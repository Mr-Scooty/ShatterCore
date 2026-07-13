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

/*
 * Asira Dawnslayer (54968) - second boss of Hour of Twilight.
 *
 * Choreography from retail sniffs + era DBM:
 *  - She rides the incoming Life Warden down (killing it - the drake crash-lands
 *    and lies wounded), leaps off at the crash and banters with Thrall for ~21s,
 *    then opens combat herself ("Let's get to work, shall we?").
 *  - Mark of Silence every 8.5s (metronomic); Choking Smoke Bomb first at ~10s
 *    then every 23.1s (with her combat bark); Throw Knife is the era mechanic:
 *    a body-blockable projectile that silences a marked target it strikes
 *    (rider spell 103587, wired through the knife's dummy effect).
 *  - Blade Barrier at 30%: hits under 40k are reduced to 1; a bigger hit
 *    shatters it. It cascades into Lesser Blade Barrier (30k) when removed -
 *    sniffs show the cascade at exactly the 12s aura expiry too.
 *  - Thrall fights alongside: Lava Burst filler and a Rising Fire Totem every
 *    ~23s (handled in hour_of_twilight.cpp).
 */

#include "ScriptMgr.h"
#include "Containers.h"
#include "DynamicObject.h"
#include "InstanceScript.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "SpellAuraEffects.h"
#include "SpellScript.h"
#include "TemporarySummon.h"
#include "hour_of_twilight.h"

namespace HourOfTwilight
{
namespace AsiraDawnslayer
{
enum Spells
{
    SPELL_ASIRA_DISMOUNT            = 103720, // leap off the crashing drake
    SPELL_MUTILATE                  = 103655, // finishing the downed drake (intro flavor)
    SPELL_MARK_OF_SILENCE           = 102726,
    SPELL_THROW_KNIFE               = 103597,
    SPELL_SILENCED                  = 103587, // knife rider on marked targets
    SPELL_CHOKING_SMOKE_BOMB        = 103558, // persistent area cloud (20s)
    SPELL_CHOKING_SMOKE_BOMB_AURA   = 103790, // interfere-targetting + tick, cast into the cloud
    SPELL_BLADE_BARRIER             = 103419, // 40k threshold, 12s
    SPELL_LESSER_BLADE_BARRIER      = 103562  // 30k threshold, 6s
};

enum Events
{
    EVENT_INTRO_BANTER_1 = 1,   // "...and with that out of the way..."
    EVENT_INTRO_THRALL_REPLY,   // Thrall: "I haven't come to be stopped..."
    EVENT_INTRO_ENGAGE,         // "Let's get to work, shall we?"
    EVENT_MARK_OF_SILENCE,
    EVENT_THROW_KNIFE,
    EVENT_SMOKE_BOMB
};

enum Texts
{
    // Asira
    SAY_INTRO               = 0, // 53825 "Where do you think you're going, little lizard?"
    SAY_BANTER              = 1, // 53751 "...and with that out of the way..."
    SAY_ENGAGE              = 2, // 53268 "Let's get to work, shall we?"
    SAY_BARK                = 3, // 54826/54827/54828 pool
    SAY_DEATH               = 4, // 54821

    // Thrall 54972 groups poked during the RP (defined in hour_of_twilight.cpp / SQL)
    SAY_THRALL_ASSASSIN     = 5, // 53913 "An assassin! Quickly..."
    SAY_THRALL_NOT_STOPPED  = 6  // 53968 "I haven't come to be stopped by the likes of you."
};

// Crash site of the Life Warden she rides in on.
Position const AsiraJumpPosition = { 4286.0f, 602.9f, -6.67f, 4.35f };

struct boss_asira_dawnslayer : public BossAI
{
    boss_asira_dawnslayer(Creature* creature) : BossAI(creature, DATA_ASIRA_DAWNSLAYER), _barrierTriggered(false), _arrived(false)
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetImmuneToAll(true);
    }

    void JustAppeared() override
    {
        // Grid reload after the arrival RP: stand at the crash site, open for pulls.
        if (instance->GetData(DATA_ESCORT_STAGE) >= STAGE_ASIRA_READY && instance->GetBossState(DATA_ASIRA_DAWNSLAYER) != DONE && me->IsVisible())
        {
            _arrived = true;
            OpenForPulls();
        }
    }

    // Fired by the crashing Life Warden the moment it hits the ground.
    void DoAction(int32 action) override
    {
        if (action != ACTION_ASIRA_ARRIVES || _arrived)
            return;
        _arrived = true;
        me->SetVisible(true);
        me->NearTeleportTo(AsiraJumpPosition.GetPositionX() - 3.0f, AsiraJumpPosition.GetPositionY() + 3.0f, AsiraJumpPosition.GetPositionZ() + 18.0f, AsiraJumpPosition.GetOrientation());
        DoCastSelf(SPELL_ASIRA_DISMOUNT, true);
        me->GetMotionMaster()->MoveJump(AsiraJumpPosition, 15.0f, 20.0f);
        Talk(SAY_INTRO);
        if (Creature* thrall = instance->GetCreature(DATA_THRALL_GALAKROND))
        {
            if (Creature* warden = instance->GetCreature(DATA_LIFE_WARDEN_THRALLS))
                me->CastSpell(warden, SPELL_MUTILATE, true);
            if (thrall->IsAIEnabled())
                thrall->AI()->Talk(SAY_THRALL_ASSASSIN); // +4.9s on retail; acceptable up front
        }
        events.ScheduleEvent(EVENT_INTRO_BANTER_1, 8s);
        events.ScheduleEvent(EVENT_INTRO_THRALL_REPLY, 20s);
        events.ScheduleEvent(EVENT_INTRO_ENGAGE, 31s);
    }

    void OpenForPulls()
    {
        me->SetImmuneToAll(false);
        me->SetReactState(REACT_AGGRESSIVE);
    }

    void Reset() override
    {
        _Reset();
        _barrierTriggered = false;
        if (_arrived)
            OpenForPulls();
    }

    void JustEngagedWith(Unit* who) override
    {
        BossAI::JustEngagedWith(who);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 1);
        events.ScheduleEvent(EVENT_MARK_OF_SILENCE, 1s);
        events.ScheduleEvent(EVENT_THROW_KNIFE, 6s);
        events.ScheduleEvent(EVENT_SMOKE_BOMB, 10s);
        if (Creature* thrall = instance->GetCreature(DATA_THRALL_GALAKROND))
            if (thrall->IsAIEnabled())
                thrall->AI()->DoAction(ACTION_ASIRA_ENGAGED);
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage) override
    {
        if (_barrierTriggered || !me->HealthBelowPctDamaged(30, damage))
            return;
        _barrierTriggered = true;
        DoCastSelf(SPELL_BLADE_BARRIER, true);
    }

    void KilledUnit(Unit* victim) override
    {
        if (victim->GetTypeId() == TYPEID_PLAYER)
            Talk(SAY_BARK);
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
        Talk(SAY_DEATH);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        if (Creature* thrall = instance->GetCreature(DATA_THRALL_GALAKROND))
            if (thrall->IsAIEnabled())
                thrall->AI()->DoAction(ACTION_ASIRA_DEAD);
    }

    void EnterEvadeMode(EvadeReason why) override
    {
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        BossAI::EnterEvadeMode(why);
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        // Intro banter runs outside of combat.
        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_INTRO_BANTER_1:
                    Talk(SAY_BANTER);
                    continue;
                case EVENT_INTRO_THRALL_REPLY:
                    if (Creature* thrall = instance->GetCreature(DATA_THRALL_GALAKROND))
                        if (thrall->IsAIEnabled())
                            thrall->AI()->Talk(SAY_THRALL_NOT_STOPPED);
                    continue;
                case EVENT_INTRO_ENGAGE:
                    Talk(SAY_ENGAGE);
                    OpenForPulls();
                    DoZoneInCombat();
                    continue;
                default:
                    break;
            }

            if (!UpdateVictim())
                return;
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (eventId)
            {
                case EVENT_MARK_OF_SILENCE:
                    DoCastAOE(SPELL_MARK_OF_SILENCE);
                    events.Repeat(8500ms);
                    break;
                case EVENT_THROW_KNIFE:
                {
                    // Prefer the marked target - forcing the party to body-block.
                    Unit* target = nullptr;
                    Map::PlayerList const& players = me->GetMap()->GetPlayers();
                    for (auto const& ref : players)
                        if (Player* player = ref.GetSource())
                            if (player->IsAlive() && player->HasAura(SPELL_MARK_OF_SILENCE))
                            {
                                target = player;
                                break;
                            }
                    if (!target)
                        target = SelectTarget(SELECT_TARGET_RANDOM, 0, 80.0f, true);
                    if (target)
                        DoCast(target, SPELL_THROW_KNIFE);
                    events.Repeat(10s);
                    break;
                }
                case EVENT_SMOKE_BOMB:
                    Talk(SAY_BARK);
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, [this](Unit* who)
                        {
                            return who->GetTypeId() == TYPEID_PLAYER && me->GetDistance(who) > 10.0f;
                        }))
                        me->CastSpell(target->GetPosition(), SPELL_CHOKING_SMOKE_BOMB, false);
                    else if (Unit* anyTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 60.0f, true))
                        me->CastSpell(anyTarget->GetPosition(), SPELL_CHOKING_SMOKE_BOMB, false);
                    events.Repeat(23100ms);
                    break;
                default:
                    break;
            }
        }

        if (UpdateVictim())
            DoMeleeAttackIfReady();
    }

private:
    bool _barrierTriggered;
    bool _arrived;
};

// Mark of Silence (102726) - the DBC effect is a map-wide enemy-area aura;
// retail marks a single (caster-type) player.
class spell_asira_mark_of_silence : public SpellScript
{
    void SelectMark(std::list<WorldObject*>& targets)
    {
        std::list<WorldObject*> manaUsers;
        for (WorldObject* target : targets)
            if (Unit* unit = target->ToUnit())
                if (unit->GetTypeId() == TYPEID_PLAYER && unit->GetPowerType() == POWER_MANA)
                    manaUsers.push_back(target);

        std::list<WorldObject*>& pool = manaUsers.empty() ? targets : manaUsers;
        if (pool.empty())
        {
            targets.clear();
            return;
        }
        WorldObject* mark = Trinity::Containers::SelectRandomContainerElement(pool);
        targets.clear();
        targets.push_back(mark);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect.Register(&spell_asira_mark_of_silence::SelectMark, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
    }
};

// Throw Knife (103597) - the damage effect is a cone; retail resolves it against
// the FIRST hittable player along the throw line (body-block), and silences the
// struck target if it bears Mark of Silence.
class spell_asira_throw_knife : public SpellScript
{
    void SelectBlocker(std::list<WorldObject*>& targets)
    {
        Unit* caster = GetCaster();
        WorldObject* intended = GetExplTargetWorldObject();
        if (!caster || !intended)
        {
            targets.clear();
            return;
        }

        float const lineWidth = 2.0f;
        float intendedDist = caster->GetExactDist2d(intended);
        float dirX = (intended->GetPositionX() - caster->GetPositionX()) / intendedDist;
        float dirY = (intended->GetPositionY() - caster->GetPositionY()) / intendedDist;

        WorldObject* hit = intended;
        float best = intendedDist;
        for (WorldObject* target : targets)
        {
            if (target == intended || !target->IsUnit() || !target->ToUnit()->IsAlive())
                continue;
            float relX = target->GetPositionX() - caster->GetPositionX();
            float relY = target->GetPositionY() - caster->GetPositionY();
            float along = relX * dirX + relY * dirY;         // distance along the throw line
            float across = std::abs(relX * dirY - relY * dirX); // distance off the line
            if (along > 0.0f && along < best && across <= lineWidth)
            {
                best = along;
                hit = target;
            }
        }

        targets.clear();
        targets.push_back(hit);
        _hitGUID = hit->GetGUID();
    }

    void HandleSilenceRider()
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;
        if (Unit* hit = ObjectAccessor::GetUnit(*caster, _hitGUID))
            if (hit->HasAura(SPELL_MARK_OF_SILENCE))
                caster->CastSpell(hit, SPELL_SILENCED, true);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect.Register(&spell_asira_throw_knife::SelectBlocker, EFFECT_1, TARGET_UNIT_CONE_ENEMY_24);
        AfterCast.Register(&spell_asira_throw_knife::HandleSilenceRider);
    }

private:
    ObjectGuid _hitGUID;
};

// Choking Smoke Bomb (103558) - the periodic dummy drives the cloud: each tick
// re-applies 103790 (interfere-targetting + damage) to everyone inside the
// persistent-area dynamic object.
class spell_asira_choking_smoke_bomb : public AuraScript
{
    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        Unit* caster = GetTarget();
        if (DynamicObject* cloud = caster->GetDynObject(SPELL_CHOKING_SMOKE_BOMB))
            caster->CastSpell(cloud->GetPosition(), SPELL_CHOKING_SMOKE_BOMB_AURA, true);
    }

    void Register() override
    {
        OnEffectPeriodic.Register(&spell_asira_choking_smoke_bomb::HandlePeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// Blade Barrier (103419 / 103562) - hits under the threshold are reduced to 1;
// a hit at or above it shatters the barrier. The big barrier cascades into the
// lesser one when it goes away for any reason (shatter or 12s expiry - sniffed).
class spell_asira_blade_barrier : public AuraScript
{
    void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
    {
        amount = -1; // absorb everything; Absorb() decides how much actually gets through
    }

    void Absorb(AuraEffect* /*aurEff*/, DamageInfo& dmgInfo, uint32& absorbAmount)
    {
        uint32 threshold = GetId() == SPELL_BLADE_BARRIER ? 40000 : 30000;
        if (dmgInfo.GetDamage() >= threshold)
        {
            absorbAmount = 0; // the full hit lands and shatters the barrier
            Remove();
        }
        else
            absorbAmount = dmgInfo.GetDamage() > 0 ? dmgInfo.GetDamage() - 1 : 0;
    }

    void AfterRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (GetId() != SPELL_BLADE_BARRIER)
            return;
        Unit* target = GetTarget();
        if (target->IsAlive() && target->IsInCombat())
            target->CastSpell(target, SPELL_LESSER_BLADE_BARRIER, true);
    }

    void Register() override
    {
        DoEffectCalcAmount.Register(&spell_asira_blade_barrier::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
        OnEffectAbsorb.Register(&spell_asira_blade_barrier::Absorb, EFFECT_0);
        AfterEffectRemove.Register(&spell_asira_blade_barrier::AfterRemove, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
    }
};
} // namespace AsiraDawnslayer
} // namespace HourOfTwilight

void AddSC_boss_asira_dawnslayer()
{
    using namespace HourOfTwilight;
    using namespace HourOfTwilight::AsiraDawnslayer;
    RegisterHourOfTwilightCreatureAI(boss_asira_dawnslayer);
    RegisterSpellScript(spell_asira_mark_of_silence);
    RegisterSpellScript(spell_asira_throw_knife);
    RegisterSpellScript(spell_asira_choking_smoke_bomb);
    RegisterSpellScript(spell_asira_blade_barrier);
}
