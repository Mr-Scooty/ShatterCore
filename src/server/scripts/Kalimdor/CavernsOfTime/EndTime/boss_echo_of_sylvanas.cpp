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
#include "InstanceScript.h"
#include "Map.h"
#include "MapRefManager.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "PassiveAI.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "TemporarySummon.h"

#include <array>

namespace EndTime::EchoOfSylvanas
{
enum Spells
{
    // Echo of Sylvanas
    SPELL_CALLING_OF_THE_HIGHBORNE_AMBIENT  = 102603, // pre-combat arrow show, summons Brittle Ghouls
    SPELL_CALLING_OF_THE_HIGHBORNE_VISUAL   = 102581, // pre-combat visual (creature_template_addon)
    SPELL_UNHOLY_SHOT                       = 101411,
    SPELL_SHRIEK_OF_THE_HIGHBORNE           = 101412,
    SPELL_JUMP_TO_CENTER                    = 101398, // teleport to the arena center (spell_target_position)
    SPELL_CALLING_OF_THE_HIGHBORNE          = 100686, // damage immunity while airborne
    SPELL_SUMMON_GHOUL_RING                 = 101198,
    SPELL_DEATH_GRIP                        = 101397,
    SPELL_DEATH_GRIP_JUMP                   = 101987,
    SPELL_SACRIFICE                         = 101348,
    SPELL_BLIGHTED_ARROWS_SUMMON            = 101567,
    SPELL_BLIGHTED_ARROWS                   = 101401,

    // Ghoul (ring anchor)
    SPELL_GHOUL_EMERGE_VISUAL               = 100867,
    SPELL_SUMMON_RISEN_GHOUL                = 101200,
    SPELL_SHRINK_PERIODIC                   = 101318,
    SPELL_CALLING_VISUAL_ANCHOR             = 105766,

    // Risen Ghoul
    SPELL_CALLING_LINK_VISUAL               = 100862,
    SPELL_DWINDLE                           = 101259,

    // Blighted Arrows stalker
    SPELL_BLIGHTED_ARROWS_PERIODIC          = 101552,

    // Brittle Ghoul
    SPELL_PERMANENT_FEIGN_DEATH             = 96733,

    // Players
    SPELL_WRACKING_PAIN                     = 101258  // self-applied contact aura while touching the ghoul chain
};

enum Events
{
    // Echo of Sylvanas
    EVENT_UNHOLY_SHOT = 1,
    EVENT_SHRIEK_OF_THE_HIGHBORNE,
    EVENT_BLIGHTED_ARROWS,
    EVENT_CALLING_OF_THE_HIGHBORNE,

    // Ghoul (ring anchor)
    EVENT_EMERGE_VISUAL,
    EVENT_SUMMON_RISEN_GHOUL,
    EVENT_SHRINK,
    EVENT_WALK_TO_CENTER,

    // Risen Ghoul
    EVENT_LINK_VISUAL,
    EVENT_WALK_TO_SYLVANAS,

    // Brittle Ghoul
    EVENT_COLLAPSE
};

enum Actions
{
    ACTION_GHOUL_DIED       = 1,
    ACTION_GHOUL_ARRIVED    = 2,
    ACTION_SACRIFICED       = 3
};

enum Texts
{
    SAY_AGGRO   = 0,
    SAY_CALLING = 1,
    SAY_SLAY    = 2,
    SAY_DEATH   = 3
};

enum MovePoints
{
    POINT_AMBIENT_HOVER = 1,
    POINT_CALLING_HOVER = 2,
    POINT_SYLVANAS      = 3
};

enum Misc
{
    TASK_GROUP_CALLING_PHASE    = 1,
    RING_GHOUL_COUNT            = 8
};

Position const CallingCenterPosition    = { 3840.03f,  914.043f, 56.0167f };
Position const CallingHoverPosition     = { 3840.03f,  914.043f, 63.10f   };
constexpr float AmbientHoverHeight      = 7.08f;
constexpr float RingRadius              = 30.f;
constexpr float GhoulArrivalRadius      = 3.7f;
constexpr float ChainContactRange       = 2.f;

struct boss_echo_of_sylvanas : public BossAI
{
    boss_echo_of_sylvanas(Creature* creature) : BossAI(creature, DATA_ECHO_OF_SYLVANAS), _phaseGhoulKills(0), _aliveRingGhouls(0), _ringActive(false), _sacrificed(false) { }

    void JustAppeared() override
    {
        StartAmbientShow();
    }

    void JustReachedHome() override
    {
        BossAI::JustReachedHome();
        StartAmbientShow();
    }

    void JustEngagedWith(Unit* who) override
    {
        BossAI::JustEngagedWith(who);
        Talk(SAY_AGGRO, who);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 1);

        // Stop the ambient arrow show and drop back to the ground
        me->RemoveAurasDueToSpell(SPELL_CALLING_OF_THE_HIGHBORNE_AMBIENT);
        me->RemoveAurasDueToSpell(SPELL_CALLING_OF_THE_HIGHBORNE_VISUAL);
        me->SetDisableGravity(false);
        me->GetMotionMaster()->MoveFall();

        events.ScheduleEvent(EVENT_UNHOLY_SHOT, 7s);
        events.ScheduleEvent(EVENT_SHRIEK_OF_THE_HIGHBORNE, 13s);
        events.ScheduleEvent(EVENT_BLIGHTED_ARROWS, 15s + 500ms);
        events.ScheduleEvent(EVENT_CALLING_OF_THE_HIGHBORNE, 40s); // DBM: first Calling at 40 s
    }

    void EnterEvadeMode(EvadeReason why) override
    {
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_WRACKING_PAIN);
        scheduler.CancelAll();
        summons.DespawnAll();
        me->RemoveAurasDueToSpell(SPELL_CALLING_OF_THE_HIGHBORNE);
        me->SetDisableGravity(false);
        me->SetReactState(REACT_AGGRESSIVE); // a wipe during Calling would otherwise leave her passive
        ScriptedAI::EnterEvadeMode(why);
    }

    void JustDied(Unit* killer) override
    {
        BossAI::JustDied(killer);
        Talk(SAY_DEATH, killer);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_WRACKING_PAIN);
    }

    void KilledUnit(Unit* victim) override
    {
        if (victim->IsPlayer())
            Talk(SAY_SLAY, victim);
    }

    void JustSummoned(Creature* summon) override
    {
        summons.Summon(summon);

        if (summon->GetEntry() == NPC_RISEN_GHOUL)
        {
            // The ring positions are exact 45 degree steps around the arena center
            float angle = Position::NormalizeOrientation(CallingCenterPosition.GetAngle(summon));
            uint8 slot = uint8(std::lround(angle / float(M_PI_4))) % RING_GHOUL_COUNT;
            _ringGhouls[slot] = summon->GetGUID();
            ++_aliveRingGhouls;
        }
    }

    void DoAction(int32 action) override
    {
        switch (action)
        {
            case ACTION_GHOUL_DIED:
                if (!_ringActive)
                    break;

                --_aliveRingGhouls;
                ++_phaseGhoulKills;
                if (_phaseGhoulKills >= 2)
                    instance->SetData(DATA_SEVERED_TIES_ELIGIBLE, 1);

                if (!_aliveRingGhouls)
                    EndCallingPhase();
                break;
            case ACTION_GHOUL_ARRIVED:
                if (!_ringActive || _sacrificed)
                    break;

                _sacrificed = true;
                DoCastAOE(SPELL_SACRIFICE, true);
                for (ObjectGuid guid : _ringGhouls)
                    if (Creature* ghoul = ObjectAccessor::GetCreature(*me, guid))
                        if (ghoul->IsAlive() && ghoul->IsAIEnabled())
                            ghoul->AI()->DoAction(ACTION_SACRIFICED);

                EndCallingPhase();
                break;
            default:
                break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        scheduler.Update(diff);

        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_UNHOLY_SHOT:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.f, true))
                        DoCast(target, SPELL_UNHOLY_SHOT);
                    events.Repeat(9s, 12s);
                    break;
                case EVENT_SHRIEK_OF_THE_HIGHBORNE:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.f, true))
                        DoCast(target, SPELL_SHRIEK_OF_THE_HIGHBORNE);
                    events.Repeat(11s);
                    break;
                case EVENT_BLIGHTED_ARROWS:
                    SummonBlightedArrowsLine();
                    // Mirror the post-Calling salvos: the AoE flips the stalkers
                    // into the falling-arrow visual.
                    scheduler.Schedule(1s + 200ms, [this](TaskContext)
                    {
                        DoCastAOE(SPELL_BLIGHTED_ARROWS);
                    });
                    break;
                case EVENT_CALLING_OF_THE_HIGHBORNE:
                    StartCallingPhase();
                    events.Repeat(65s + 500ms);
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
    void StartAmbientShow()
    {
        if (me->IsInCombat() || instance->GetBossState(DATA_ECHO_OF_SYLVANAS) == DONE)
            return;

        me->SetDisableGravity(true);
        Position hover = me->GetHomePosition();
        hover.m_positionZ += AmbientHoverHeight;
        me->GetMotionMaster()->MoveTakeoff(POINT_AMBIENT_HOVER, hover);
        DoCastSelf(SPELL_CALLING_OF_THE_HIGHBORNE_AMBIENT, true);
    }

    void StartCallingPhase()
    {
        _phaseGhoulKills = 0;
        _aliveRingGhouls = 0;
        _sacrificed = false;
        _ringGhouls.fill(ObjectGuid::Empty);

        Talk(SAY_CALLING);
        events.CancelEvent(EVENT_UNHOLY_SHOT);
        events.CancelEvent(EVENT_SHRIEK_OF_THE_HIGHBORNE);
        me->InterruptNonMeleeSpells(true);
        me->AttackStop();
        me->SetReactState(REACT_PASSIVE);
        DoCastSelf(SPELL_JUMP_TO_CENTER, true);

        scheduler
            .Schedule(2s + 400ms, TASK_GROUP_CALLING_PHASE, [this](TaskContext)
            {
                me->SetDisableGravity(true);
                me->GetMotionMaster()->MoveTakeoff(POINT_CALLING_HOVER, CallingHoverPosition);
                DoCastSelf(SPELL_CALLING_OF_THE_HIGHBORNE, true);
            })
            .Schedule(3s + 600ms, TASK_GROUP_CALLING_PHASE, [this](TaskContext)
            {
                DoCastSelf(SPELL_SUMMON_GHOUL_RING, true);
            })
            .Schedule(7s + 300ms, TASK_GROUP_CALLING_PHASE, [this](TaskContext)
            {
                DoCastAOE(SPELL_DEATH_GRIP, true);
            })
            .Schedule(12s, TASK_GROUP_CALLING_PHASE, [this](TaskContext task)
            {
                _ringActive = true;
                UpdateWrackingField();
                task.Repeat(500ms);
            })
            .Schedule(45s, TASK_GROUP_CALLING_PHASE, [this](TaskContext)
            {
                // Watchdog - the phase should have ended long ago at this point
                if (_ringActive)
                    EndCallingPhase();
            });
    }

    void EndCallingPhase()
    {
        _ringActive = false;
        scheduler.CancelGroup(TASK_GROUP_CALLING_PHASE);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_WRACKING_PAIN);

        for (ObjectGuid guid : _ringGhouls)
            if (Creature* ghoul = ObjectAccessor::GetCreature(*me, guid))
                if (ghoul->IsAlive() && ghoul->IsAIEnabled())
                    ghoul->AI()->DoAction(ACTION_SACRIFICED);

        DespawnRingAnchors();

        me->RemoveAurasDueToSpell(SPELL_CALLING_OF_THE_HIGHBORNE);
        me->SetDisableGravity(false);
        me->GetMotionMaster()->MoveFall();

        scheduler
            .Schedule(1s + 200ms, [this](TaskContext)
            {
                SummonBlightedArrowsLine();
            })
            .Schedule(2s + 400ms, [this](TaskContext)
            {
                DoCastAOE(SPELL_BLIGHTED_ARROWS);
            })
            .Schedule(5s, [this](TaskContext)
            {
                me->SetReactState(REACT_AGGRESSIVE);
                events.ScheduleEvent(EVENT_UNHOLY_SHOT, 5s);
                events.ScheduleEvent(EVENT_SHRIEK_OF_THE_HIGHBORNE, 8s);
            });
    }

    void DespawnRingAnchors()
    {
        std::list<Creature*> anchors;
        me->GetCreatureListWithEntryInGrid(anchors, NPC_GHOUL_ANCHOR, 100.f);
        for (Creature* anchor : anchors)
            anchor->DespawnOrUnsummon(1s);
    }

    void SummonBlightedArrowsLine()
    {
        Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.f, true);
        if (!target)
            return;

        DoCastAOE(SPELL_BLIGHTED_ARROWS_SUMMON, true);

        // Five stalkers form a 16 yard line between Sylvanas and the targeted player
        float angle = me->GetAngle(target);
        for (uint8 i = 0; i < 5; ++i)
        {
            float dist = 10.f + 4.f * i;
            Position pos = { me->GetPositionX() + std::cos(angle) * dist, me->GetPositionY() + std::sin(angle) * dist, me->GetPositionZ() };
            me->SummonCreature(NPC_BLIGHTED_ARROWS, pos, TEMPSUMMON_TIMED_DESPAWN, 36s);
        }
    }

    void UpdateWrackingField()
    {
        std::array<Creature*, RING_GHOUL_COUNT> ghouls = {};
        float ringRadius = 0.f;
        uint8 alive = 0;
        for (uint8 i = 0; i < RING_GHOUL_COUNT; ++i)
        {
            if (Creature* ghoul = ObjectAccessor::GetCreature(*me, _ringGhouls[i]))
            {
                if (ghoul->IsAlive())
                {
                    ghouls[i] = ghoul;
                    ringRadius += CallingCenterPosition.GetExactDist2d(ghoul);
                    ++alive;
                }
            }
        }

        if (!alive)
            return;

        ringRadius /= alive;

        for (MapReference const& ref : me->GetMap()->GetPlayers())
        {
            Player* player = ref.GetSource();
            if (!player || !player->IsAlive() || player->IsGameMaster())
                continue;

            bool contact = false;

            // Touching any chain segment between two living neighbor ghouls
            for (uint8 i = 0; i < RING_GHOUL_COUNT && !contact; ++i)
            {
                Creature* first = ghouls[i];
                Creature* second = ghouls[(i + 1) % RING_GHOUL_COUNT];
                if (first && second && DistanceToSegment2D(*player, *first, *second) < ChainContactRange)
                    contact = true;
            }

            // Standing in the shadow field outside the ring - unless the sector has been broken open
            if (!contact && player->GetExactDist2d(CallingCenterPosition) > ringRadius + ChainContactRange)
            {
                float playerAngle = Position::NormalizeOrientation(CallingCenterPosition.GetAngle(player));
                uint8 sector = uint8(playerAngle / float(M_PI_4)) % RING_GHOUL_COUNT;
                if (ghouls[sector] && ghouls[(sector + 1) % RING_GHOUL_COUNT])
                    contact = true;
            }

            if (contact)
            {
                if (!player->HasAura(SPELL_WRACKING_PAIN))
                    player->CastSpell(player, SPELL_WRACKING_PAIN, true);
            }
            else
                player->RemoveAurasDueToSpell(SPELL_WRACKING_PAIN);
        }
    }

    static float DistanceToSegment2D(Position const& point, Position const& a, Position const& b)
    {
        float abx = b.GetPositionX() - a.GetPositionX();
        float aby = b.GetPositionY() - a.GetPositionY();
        float apx = point.GetPositionX() - a.GetPositionX();
        float apy = point.GetPositionY() - a.GetPositionY();
        float lengthSquared = abx * abx + aby * aby;
        float t = lengthSquared > 0.f ? std::clamp((apx * abx + apy * aby) / lengthSquared, 0.f, 1.f) : 0.f;
        float cx = a.GetPositionX() + t * abx - point.GetPositionX();
        float cy = a.GetPositionY() + t * aby - point.GetPositionY();
        return std::sqrt(cx * cx + cy * cy);
    }

    uint8 _phaseGhoulKills;
    uint8 _aliveRingGhouls;
    bool _ringActive;
    bool _sacrificed;
    std::array<ObjectGuid, RING_GHOUL_COUNT> _ringGhouls;
};

struct npc_echo_of_sylvanas_ghoul_anchor : public ScriptedAI
{
    npc_echo_of_sylvanas_ghoul_anchor(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void JustAppeared() override
    {
        _events.ScheduleEvent(EVENT_EMERGE_VISUAL, 1s + 200ms);
        _events.ScheduleEvent(EVENT_SUMMON_RISEN_GHOUL, 3s + 600ms);
        _events.ScheduleEvent(EVENT_SHRINK, 6s);
        _events.ScheduleEvent(EVENT_WALK_TO_CENTER, 9s + 700ms);
    }

    void JustSummoned(Creature* summon) override
    {
        // Forward the Risen Ghoul to Sylvanas so she can track the ring
        if (InstanceScript* instance = me->GetInstanceScript())
            if (Creature* sylvanas = instance->GetCreature(DATA_ECHO_OF_SYLVANAS))
                if (sylvanas->IsAIEnabled())
                    sylvanas->AI()->JustSummoned(summon);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_EMERGE_VISUAL:
                    DoCastSelf(SPELL_GHOUL_EMERGE_VISUAL);
                    break;
                case EVENT_SUMMON_RISEN_GHOUL:
                    DoCastSelf(SPELL_SUMMON_RISEN_GHOUL, true);
                    break;
                case EVENT_SHRINK:
                    DoCastSelf(SPELL_SHRINK_PERIODIC, true);
                    DoCastSelf(SPELL_CALLING_VISUAL_ANCHOR, true);
                    break;
                case EVENT_WALK_TO_CENTER:
                    me->SetWalk(true);
                    me->GetMotionMaster()->MovePoint(0, CallingCenterPosition, false);
                    break;
                default:
                    break;
            }
        }
    }

private:
    EventMap _events;
};

struct npc_echo_of_sylvanas_risen_ghoul : public ScriptedAI
{
    npc_echo_of_sylvanas_risen_ghoul(Creature* creature) : ScriptedAI(creature), _finished(false)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void JustAppeared() override
    {
        _events.ScheduleEvent(EVENT_LINK_VISUAL, 4s + 900ms);
        _events.ScheduleEvent(EVENT_WALK_TO_SYLVANAS, 6s + 100ms);
    }

    void DoAction(int32 action) override
    {
        if (action == ACTION_SACRIFICED && !_finished)
        {
            _finished = true;
            CastDwindle();
            me->DespawnOrUnsummon(500ms);
        }
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != POINT_MOTION_TYPE || pointId != POINT_SYLVANAS)
            return;

        if (InstanceScript* instance = me->GetInstanceScript())
            if (Creature* sylvanas = instance->GetCreature(DATA_ECHO_OF_SYLVANAS))
                if (sylvanas->IsAIEnabled())
                    sylvanas->AI()->DoAction(ACTION_GHOUL_ARRIVED);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (_finished)
            return;

        _finished = true;
        CastDwindle();

        if (InstanceScript* instance = me->GetInstanceScript())
            if (Creature* sylvanas = instance->GetCreature(DATA_ECHO_OF_SYLVANAS))
                if (sylvanas->IsAIEnabled())
                    sylvanas->AI()->DoAction(ACTION_GHOUL_DIED);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_LINK_VISUAL:
                    DoCastSelf(SPELL_CALLING_LINK_VISUAL, true);
                    break;
                case EVENT_WALK_TO_SYLVANAS:
                {
                    me->SetWalk(true);
                    float angle = CallingCenterPosition.GetAngle(me);
                    Position dest = { CallingCenterPosition.GetPositionX() + std::cos(angle) * GhoulArrivalRadius,
                                      CallingCenterPosition.GetPositionY() + std::sin(angle) * GhoulArrivalRadius,
                                      CallingCenterPosition.GetPositionZ() };
                    me->GetMotionMaster()->MovePoint(POINT_SYLVANAS, dest);
                    break;
                }
                default:
                    break;
            }
        }
    }

private:
    void CastDwindle()
    {
        if (TempSummon* summon = me->ToTempSummon())
            if (Unit* anchor = summon->GetSummoner())
            {
                DoCast(anchor, SPELL_DWINDLE, true);
                return;
            }

        me->CastSpell(me->GetPosition(), SPELL_DWINDLE, true);
    }

    EventMap _events;
    bool _finished;
};

struct npc_echo_of_sylvanas_blighted_arrows : public NullCreatureAI
{
    npc_echo_of_sylvanas_blighted_arrows(Creature* creature) : NullCreatureAI(creature) { }

    void JustAppeared() override
    {
        DoCastSelf(SPELL_BLIGHTED_ARROWS_PERIODIC, true);
    }
};

struct npc_echo_of_sylvanas_brittle_ghoul : public NullCreatureAI
{
    npc_echo_of_sylvanas_brittle_ghoul(Creature* creature) : NullCreatureAI(creature) { }

    void JustAppeared() override
    {
        _events.ScheduleEvent(EVENT_COLLAPSE, 16s + 500ms);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_COLLAPSE:
                    DoCastSelf(SPELL_PERMANENT_FEIGN_DEATH, true);
                    break;
                default:
                    break;
            }
        }
    }

private:
    EventMap _events;
};

// 101198 - Summon Ghoul
class spell_echo_of_sylvanas_summon_ghoul_ring : public SpellScript
{
    void HandleScriptEffect(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        for (uint8 i = 0; i < RING_GHOUL_COUNT; ++i)
        {
            float angle = float(M_PI_4) * i;
            Position pos = { CallingCenterPosition.GetPositionX() + std::cos(angle) * RingRadius,
                             CallingCenterPosition.GetPositionY() + std::sin(angle) * RingRadius,
                             CallingCenterPosition.GetPositionZ(),
                             Position::NormalizeOrientation(angle + float(M_PI)) };
            caster->SummonCreature(NPC_GHOUL_ANCHOR, pos, TEMPSUMMON_MANUAL_DESPAWN);
        }
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_echo_of_sylvanas_summon_ghoul_ring::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

// 101397 - Death Grip
class spell_echo_of_sylvanas_death_grip : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DEATH_GRIP_JUMP });
    }

    void HandleScriptEffect(SpellEffIndex /*effIndex*/)
    {
        GetHitUnit()->CastSpell(CallingCenterPosition, SPELL_DEATH_GRIP_JUMP, true);
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_echo_of_sylvanas_death_grip::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};
}

void AddSC_boss_echo_of_sylvanas()
{
    using namespace EndTime;
    using namespace EndTime::EchoOfSylvanas;
    RegisterEndTimeCreatureAI(boss_echo_of_sylvanas);
    RegisterEndTimeCreatureAI(npc_echo_of_sylvanas_ghoul_anchor);
    RegisterEndTimeCreatureAI(npc_echo_of_sylvanas_risen_ghoul);
    RegisterEndTimeCreatureAI(npc_echo_of_sylvanas_blighted_arrows);
    RegisterEndTimeCreatureAI(npc_echo_of_sylvanas_brittle_ghoul);
    RegisterSpellScript(spell_echo_of_sylvanas_summon_ghoul_ring);
    RegisterSpellScript(spell_echo_of_sylvanas_death_grip);
}
