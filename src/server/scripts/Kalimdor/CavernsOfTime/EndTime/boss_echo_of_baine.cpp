/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "end_time.h"
#include "Containers.h"
#include "GameObject.h"
#include "InstanceScript.h"
#include "ObjectAccessor.h"
#include "Map.h"
#include "MapRefManager.h"
#include "MotionMaster.h"
#include "PassiveAI.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "TemporarySummon.h"

namespace EndTime::EchoOfBaine
{
enum Spells
{
    // Echo of Baine
    SPELL_MOLTEN_AXE            = 101836, // 10s self buff, constantly refreshed while Baine stands in magma
    SPELL_MOLTEN_BLAST          = 101840, // triggered by Molten Axe on melee attacks (spell_proc)
    SPELL_THROW_TOTEM           = 101615, // target picker, triggers the totem summon missile
    SPELL_PULVERIZE             = 101625, // parent cast, selects the jump destination
    SPELL_PULVERIZE_JUMP        = 101626,
    SPELL_PULVERIZE_BLAST       = 101627, // blast at the jump destination, triggers the platform destruction (101815)
    SPELL_TOTEM_BACK            = 101602, // player throw at Baine: damage + 6s +50% damage taken + stun

    // Baine's Totem
    SPELL_BAINES_TOTEM_VISUAL   = 101594,
    SPELL_THROW_TOTEM_CARRY     = 107837, // spellclick spell
    SPELL_THROW_TOTEM_OVERLAY   = 101601, // triggered by 107837

    // Players
    SPELL_MAGMA                 = 101619, // lava damage aura while swimming in magma
    SPELL_MOLTEN_FISTS          = 101866  // 20s player buff, constantly refreshed while the player is in magma
};

enum Events
{
    EVENT_THROW_TOTEM = 1,
    EVENT_PULVERIZE
};

enum Actions
{
    ACTION_INTRO = 1
};

enum Texts
{
    SAY_INTRO       = 0,
    SAY_AGGRO       = 1,
    SAY_PULVERIZE   = 2,
    EMOTE_PULVERIZE = 3,
    SAY_SLAY        = 4,
    SAY_DEATH       = 5
};

uint32 const PlatformGameObjectEntries[] = { GO_PLATFORM_1, GO_PLATFORM_2, GO_PLATFORM_3, GO_PLATFORM_4 };

// The platforms' walking surface sits at z 131-132.4, the magma surface below at z ~129
constexpr float MagmaSurfaceZ = 130.75f;

bool IsInMagma(Unit const* who)
{
    if (who->GetPositionZ() > MagmaSurfaceZ)
        return false;

    ZLiquidStatus status = who->GetMap()->GetLiquidStatus(who->GetPhaseShift(), who->GetPositionX(), who->GetPositionY(), who->GetPositionZ(), map_liquidHeaderTypeFlags::Magma);
    if (status & MAP_LIQUID_STATUS_SWIMMING)
        return true;

    // Fallback for incomplete liquid data in the map extracts: arena bounding box + height check
    return who->GetPositionX() > 4320.f && who->GetPositionX() < 4430.f
        && who->GetPositionY() > 1395.f && who->GetPositionY() < 1505.f;
}

struct boss_echo_of_baine : public BossAI
{
    boss_echo_of_baine(Creature* creature) : BossAI(creature, DATA_ECHO_OF_BAINE), _magmaTimer(0), _pulverizePending(false), _introDone(false) { }

    void Reset() override
    {
        _Reset();
        _pulverizePending = false;
        RestorePlatforms();
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_MOLTEN_FISTS);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_MAGMA);
    }

    void JustEngagedWith(Unit* who) override
    {
        BossAI::JustEngagedWith(who);
        Talk(SAY_AGGRO, who);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 1);
        events.ScheduleEvent(EVENT_THROW_TOTEM, 10s);
        events.ScheduleEvent(EVENT_PULVERIZE, 30s);
    }

    void EnterEvadeMode(EvadeReason why) override
    {
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        summons.DespawnAll();
        ScriptedAI::EnterEvadeMode(why);
    }

    void JustDied(Unit* killer) override
    {
        BossAI::JustDied(killer);
        Talk(SAY_DEATH, killer);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_MOLTEN_FISTS);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_MAGMA);
    }

    void KilledUnit(Unit* victim) override
    {
        if (victim->IsPlayer())
            Talk(SAY_SLAY, victim);
    }

    void DoAction(int32 action) override
    {
        switch (action)
        {
            case ACTION_INTRO:
                if (!_introDone)
                {
                    _introDone = true;
                    Talk(SAY_INTRO);
                }
                break;
            default:
                break;
        }
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != EFFECT_MOTION_TYPE || pointId != EVENT_JUMP || !_pulverizePending)
            return;

        _pulverizePending = false;
        me->CastSpell(me->GetPosition(), SPELL_PULVERIZE_BLAST, true);
    }

    void SetData(uint32 type, uint32 /*data*/) override
    {
        if (type == EVENT_PULVERIZE)
            _pulverizePending = true;
    }

    void UpdateAI(uint32 diff) override
    {
        UpdateMagma(diff);

        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_THROW_TOTEM:
                    // The totem is never thrown at the current tank
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 100.f, true))
                        DoCast(target, SPELL_THROW_TOTEM);
                    else if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.f, true))
                        DoCast(target, SPELL_THROW_TOTEM);
                    events.Repeat(25s);
                    break;
                case EVENT_PULVERIZE:
                    Talk(SAY_PULVERIZE);
                    Talk(EMOTE_PULVERIZE);
                    DoCastAOE(SPELL_PULVERIZE);
                    events.Repeat(40s);
                    break;
                default:
                    break;
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
        }

        DoMeleeAttackIfReady();
    }

private:
    void UpdateMagma(uint32 diff)
    {
        if (_magmaTimer > diff)
        {
            _magmaTimer -= diff;
            return;
        }

        _magmaTimer = 250;

        if (me->IsAlive() && IsInMagma(me))
            DoCastSelf(SPELL_MOLTEN_AXE, true);

        for (MapReference const& ref : me->GetMap()->GetPlayers())
        {
            Player* player = ref.GetSource();
            if (!player || !player->IsAlive() || player->IsGameMaster())
                continue;

            if (IsInMagma(player))
            {
                player->CastSpell(player, SPELL_MOLTEN_FISTS, true);
                if (!player->HasAura(SPELL_MAGMA))
                    player->CastSpell(player, SPELL_MAGMA, true);
            }
            else
                player->RemoveAurasDueToSpell(SPELL_MAGMA);
        }
    }

    void RestorePlatforms()
    {
        for (uint32 entry : PlatformGameObjectEntries)
            if (GameObject* platform = me->FindNearestGameObject(entry, 200.f))
                platform->SetDestructibleState(GO_DESTRUCTIBLE_INTACT);
    }

    uint32 _magmaTimer;
    bool _pulverizePending;
    bool _introDone;
};

struct npc_baines_totem : public NullCreatureAI
{
    npc_baines_totem(Creature* creature) : NullCreatureAI(creature) { }

    void JustAppeared() override
    {
        DoCastSelf(SPELL_BAINES_TOTEM_VISUAL);

        // The totem visual creature rides the clickable base on retail. We place it directly instead.
        Position visualPos = me->GetPosition();
        visualPos.m_positionX += 0.3f;
        visualPos.m_positionZ -= 1.5f;
        if (Creature* visual = me->SummonCreature(NPC_BAINES_TOTEM_VISUAL, visualPos, TEMPSUMMON_TIMED_DESPAWN, 20s))
            _visualGuid = visual->GetGUID();
    }

    void OnSpellClick(Unit* /*clicker*/, bool& result) override
    {
        if (!result || _clicked)
            return;

        _clicked = true;
        me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);

        // npc_spellclick_spells applies 107837 to the clicker. Its periodic 101601 aura asks the
        // client to initialize the destination-targeted 101603 throw, which in turn hits Baine
        // with 101602. Keep that native pickup/aim/throw interaction instead of auto-throwing.

        if (Creature* visual = ObjectAccessor::GetCreature(*me, _visualGuid))
            visual->DespawnOrUnsummon(400ms);

        me->DespawnOrUnsummon(400ms);
    }

private:
    bool _clicked = false;
    ObjectGuid _visualGuid;
};

// 101815 - Pulverize (platform destruction)
class spell_echo_of_baine_pulverize_platform : public SpellScript
{
    void FilterTargets(std::list<WorldObject*>& targets)
    {
        // Only the rock platform at the blast location may sink
        targets.remove_if([](WorldObject const* target)
        {
            return std::find(std::begin(PlatformGameObjectEntries), std::end(PlatformGameObjectEntries), target->GetEntry()) == std::end(PlatformGameObjectEntries);
        });

        if (targets.size() > 1)
        {
            targets.sort(Trinity::ObjectDistanceOrderPred(GetCaster()));
            targets.resize(1);
        }
    }

    void Register() override
    {
        OnObjectAreaTargetSelect.Register(&spell_echo_of_baine_pulverize_platform::FilterTargets, EFFECT_0, TARGET_GAMEOBJECT_DEST_AREA);
    }
};

// 101625 - Pulverize
class spell_echo_of_baine_pulverize : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PULVERIZE_JUMP });
    }

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        if (targets.empty())
            return;

        Trinity::Containers::RandomResize(targets, 1);
    }

    void HandleDummyEffect(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (Creature* creature = caster->ToCreature())
            if (creature->IsAIEnabled())
                creature->AI()->SetData(EVENT_PULVERIZE, 1);

        caster->CastSpell(GetHitUnit(), SPELL_PULVERIZE_JUMP, true);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect.Register(&spell_echo_of_baine_pulverize::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        OnEffectHitTarget.Register(&spell_echo_of_baine_pulverize::HandleDummyEffect, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};
}

void AddSC_boss_echo_of_baine()
{
    using namespace EndTime;
    using namespace EndTime::EchoOfBaine;
    RegisterEndTimeCreatureAI(boss_echo_of_baine);
    RegisterEndTimeCreatureAI(npc_baines_totem);
    RegisterSpellScript(spell_echo_of_baine_pulverize);
    RegisterSpellScript(spell_echo_of_baine_pulverize_platform);
}
