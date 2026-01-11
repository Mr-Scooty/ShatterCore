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

#include "ScriptMgr.h"
#include "firelands.h"
#include "InstanceScript.h"
#include "Map.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "SpellAuraEffects.h"
#include "SpellScript.h"
#include "TemporarySummon.h"
#include <cmath>

namespace Firelands::Shannox
{
enum Spells
{
    // Shannox
    SPELL_ARCING_SLASH          = 99931,
    SPELL_JAGGED_TEAR           = 99936, // Applied by Arcing Slash
    SPELL_HURL_SPEAR            = 100002,
    SPELL_MAGMA_FLARE           = 100495,
    SPELL_SPEAR_VISUAL          = 100035, // Visual for spear landing location

    // Spear of Shannox
    SPELL_MAGMA_RUPTURE         = 99842,
    SPELL_MAGMA_RUPTURE_VISUAL  = 99841,  // Visual aura for fire patches before eruption
    SPELL_SPEAR_LAND_DAMAGE     = 100002, // Physical damage on landing

    // Riplimb & Rageface
    SPELL_SEPARATION_ANXIETY    = 99835,

    // Riplimb
    SPELL_LIMB_RIP              = 99832,  // Every 10 seconds, applies Jagged Tear
    SPELL_DOGGED_DETERMINATION  = 101111, // When fetching spear, movement speed buff
    SPELL_FRENZIED_DEVOTION     = 100064, // When Shannox below 30% HP (normal mode)

    // Rageface
    SPELL_FACE_RAGE             = 99947,  // Leap and pin target, stun
    SPELL_FACE_RAGE_DAMAGE      = 99945,  // Escalating damage while pinned

    // Traps
    SPELL_IMMOLATION_TRAP       = 99838,  // Initial 65k fire damage
    SPELL_IMMOLATION_DOT        = 99839,  // 51k fire damage over 9 seconds + 40% increased damage taken
    SPELL_CRYSTAL_PRISON_TRAP   = 99836,  // Trap trigger effect
    SPELL_CRYSTAL_PRISON_EFFECT = 99837,  // Imprisonment effect on player

    // Frenzy
    SPELL_FRENZY                = 100522, // Gained when a hound dies, stacks
};

enum Events
{
    // Shannox
    EVENT_ARCING_SLASH          = 1,
    EVENT_HURL_SPEAR            = 2,
    EVENT_THROW_TRAPS           = 3,

    // Riplimb & Rageface
    EVENT_CHECK_SEPARATION      = 4,
    EVENT_FETCH_SPEAR           = 5,
    EVENT_RETURN_SPEAR          = 6,

    // Riplimb
    EVENT_LIMB_RIP              = 12,
    EVENT_CHECK_SHANNOX_HP      = 13,

    // Rageface
    EVENT_CHANGE_TARGET         = 14,
    EVENT_FACE_RAGE             = 15,
    EVENT_FACE_RAGE_TICK        = 16,

    // Spear of Shannox
    EVENT_SPEAR_LAND            = 7,
    EVENT_MAGMA_RUPTURE         = 8,
    EVENT_SIGNAL_RIPLIMB        = 9,

    // Spiral Flame
    EVENT_SPIRAL_FLAME_ERUPT    = 10,

    // Traps
    EVENT_CHECK_TRAP_TRIGGER    = 11,
};

enum Texts
{
    SAY_SPAWN                   = 0,  // Patrol/spawn text
    SAY_AGGRO                   = 1,
    SAY_SLAY                    = 2,
    SAY_RIPLIMB_DEAD            = 3,
    SAY_RAGEFACE_DEAD           = 4,
    SAY_DEATH                   = 5,
};

enum Creatures
{
    NPC_RIPLIMB                 = 53694,
    NPC_RAGEFACE                = 53695,
    NPC_SPEAR_OF_SHANNOX        = 53752,  // Invisible trigger for spear mechanics
    NPC_SPEAR_OF_SHANNOX_VISUAL = 54112,  // Visible spear model on ground
    NPC_SPIRAL_FLAME            = 54276,
    NPC_IMMOLATION_TRAP         = 53724,
    NPC_CRYSTAL_PRISON_TRAP     = 53713,
    NPC_CRYSTAL_PRISON          = 53819,  // The actual prison that encases players
};

enum Misc
{
    SEPARATION_DISTANCE         = 60,   // yards
    SEPARATION_CHECK_TIMER      = 1000, // 1 second check interval

    ARCING_SLASH_RANGE          = 10,   // yards (melee range for frontal cone)
    ARCING_SLASH_ANGLE          = 120,  // degrees (60 degrees each side of facing)

    HURL_SPEAR_TIMER            = 45000,    // 45 seconds
    SPEAR_LAND_DELAY            = 3000,     // 3 seconds after throw for spear to land
    MAGMA_RUPTURE_DELAY         = 2000,     // 2 seconds after landing for magma rupture
    SPEAR_LAND_DAMAGE_RADIUS    = 5,        // yards
    SPEAR_LAND_DAMAGE           = 100000,   // 100k physical damage on landing
    SPIRAL_FLAME_COUNT          = 8,        // Number of fire patches spawned in circle
    SPIRAL_FLAME_RADIUS         = 8,        // yards from spear center

    THROW_TRAPS_TIMER           = 25000,    // 25 seconds
    TRAP_TRIGGER_RANGE          = 2,        // yards - distance to trigger trap
    TRAP_ARM_DELAY              = 2000,     // 2 seconds before trap becomes active
    CRYSTAL_PRISON_HEALTH       = 2800000,  // 2.8 million HP

    LIMB_RIP_TIMER              = 10000,    // 10 seconds
    SHANNOX_HP_CHECK_TIMER      = 1000,     // Check Shannox HP every second
    SHANNOX_FRENZIED_HP_PCT     = 30,       // Frenzied Devotion below 30% HP

    // Rageface
    RAGEFACE_TARGET_CHANGE_TIMER = 15000,   // Change target every 15 seconds
    FACE_RAGE_TIMER             = 45000,    // Face Rage every 45 seconds
    FACE_RAGE_TICK_TIMER        = 500,      // Damage tick every 0.5 seconds
    FACE_RAGE_BASE_DAMAGE       = 8000,     // 8k base damage, increases each tick
    FACE_RAGE_INTERRUPT_DAMAGE  = 30000,    // 30k damage to interrupt Face Rage
};

enum Actions
{
    ACTION_FETCH_SPEAR          = 1,
    ACTION_SPEAR_RETRIEVED      = 2,
    ACTION_RIPLIMB_DEAD         = 3,
    ACTION_RAGEFACE_DEAD        = 4,
};

// 53691 - Shannox
class boss_shannox : public CreatureScript
{
    public:
        boss_shannox() : CreatureScript("boss_shannox") { }

        struct boss_shannoxAI : public BossAI
        {
            boss_shannoxAI(Creature* creature) : BossAI(creature, DATA_SHANNOX),
                _hasSpear(true), _riplimbDead(false), _ragefaceDead(false) { }

            void Reset() override
            {
                _Reset();
                _riplimbGUID.Clear();
                _ragefaceGUID.Clear();
                _spearGUID.Clear();
                _spearVisualGUID.Clear();
                _hasSpear = true;
                _riplimbDead = false;
                _ragefaceDead = false;
                // Restore weapon visual on reset
                me->LoadEquipment();
            }

            void JustEngagedWith(Unit* who) override
            {
                BossAI::JustEngagedWith(who);
                Talk(SAY_AGGRO);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

                // Schedule Arcing Slash - roughly every 10 seconds
                events.ScheduleEvent(EVENT_ARCING_SLASH, 10 * IN_MILLISECONDS);

                // Schedule Hurl Spear - every 45 seconds
                events.ScheduleEvent(EVENT_HURL_SPEAR, HURL_SPEAR_TIMER);

                // Schedule Trap throwing - every 25 seconds
                events.ScheduleEvent(EVENT_THROW_TRAPS, THROW_TRAPS_TIMER);

                // Find and engage the hounds
                if (Creature* riplimb = me->FindNearestCreature(NPC_RIPLIMB, 100.0f))
                {
                    _riplimbGUID = riplimb->GetGUID();
                    riplimb->AI()->AttackStart(who);
                }

                if (Creature* rageface = me->FindNearestCreature(NPC_RAGEFACE, 100.0f))
                {
                    _ragefaceGUID = rageface->GetGUID();
                    rageface->AI()->AttackStart(who);
                }
            }

            void KilledUnit(Unit* who) override
            {
                if (who->GetTypeId() != TYPEID_PLAYER)
                    return;

                if (!(rand32() % 5))
                    Talk(SAY_SLAY);
            }

            void JustDied(Unit* /*killer*/) override
            {
                _JustDied();
                Talk(SAY_DEATH);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

                // Kill the hounds when Shannox dies
                if (Creature* riplimb = ObjectAccessor::GetCreature(*me, _riplimbGUID))
                    if (riplimb->IsAlive())
                        riplimb->KillSelf();

                if (Creature* rageface = ObjectAccessor::GetCreature(*me, _ragefaceGUID))
                    if (rageface->IsAlive())
                        rageface->KillSelf();
            }

            void EnterEvadeMode(EvadeReason /*why*/) override
            {
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                me->GetMotionMaster()->MoveTargetedHome();
                summons.DespawnAll();
                _hasSpear = true;
                // Restore weapon visual on evade
                me->LoadEquipment();
                _DespawnAtEvade();
            }

            void DoAction(int32 action) override
            {
                switch (action)
                {
                    case ACTION_SPEAR_RETRIEVED:
                        _hasSpear = true;
                        // Restore weapon visual - Shannox has his spear back
                        me->LoadEquipment();
                        // Despawn the spear trigger creature
                        if (Creature* spear = ObjectAccessor::GetCreature(*me, _spearGUID))
                            spear->DespawnOrUnsummon();
                        _spearGUID.Clear();
                        // Despawn the visible spear model
                        if (Creature* visibleSpear = ObjectAccessor::GetCreature(*me, _spearVisualGUID))
                            visibleSpear->DespawnOrUnsummon();
                        _spearVisualGUID.Clear();
                        break;
                    case ACTION_RIPLIMB_DEAD:
                        _riplimbDead = true;
                        Talk(SAY_RIPLIMB_DEAD);
                        // Gain Frenzy - stacks when both hounds die
                        DoCastSelf(SPELL_FRENZY, true);
                        break;
                    case ACTION_RAGEFACE_DEAD:
                        _ragefaceDead = true;
                        Talk(SAY_RAGEFACE_DEAD);
                        // Gain Frenzy - stacks when both hounds die
                        DoCastSelf(SPELL_FRENZY, true);
                        break;
                    default:
                        break;
                }
            }

            ObjectGuid GetGUID(int32 type) const override
            {
                switch (type)
                {
                    case NPC_RIPLIMB:
                        return _riplimbGUID;
                    case NPC_RAGEFACE:
                        return _ragefaceGUID;
                    case NPC_SPEAR_OF_SHANNOX:
                        return _spearGUID;
                    default:
                        return ObjectGuid::Empty;
                }
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_ARCING_SLASH:
                            // Can only use Arcing Slash when holding the spear
                            if (_hasSpear)
                                DoArcingSlash();
                            events.ScheduleEvent(EVENT_ARCING_SLASH, 10 * IN_MILLISECONDS);
                            break;
                        case EVENT_HURL_SPEAR:
                            DoHurlSpear();
                            events.ScheduleEvent(EVENT_HURL_SPEAR, HURL_SPEAR_TIMER);
                            break;
                        case EVENT_THROW_TRAPS:
                            DoThrowTraps();
                            events.ScheduleEvent(EVENT_THROW_TRAPS, THROW_TRAPS_TIMER);
                            break;
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

        private:
            ObjectGuid _riplimbGUID;
            ObjectGuid _ragefaceGUID;
            ObjectGuid _spearGUID;
            ObjectGuid _spearVisualGUID;
            bool _hasSpear;
            bool _riplimbDead;
            bool _ragefaceDead;

            void DoArcingSlash()
            {
                // Get all players within cone range
                std::list<Player*> targets;
                Map::PlayerList const& playerList = me->GetMap()->GetPlayers();

                for (auto const& playerRef : playerList)
                {
                    Player* player = playerRef.GetSource();
                    if (!player || !player->IsAlive())
                        continue;

                    // Check if player is within range
                    if (!me->IsWithinDistInMap(player, static_cast<float>(ARCING_SLASH_RANGE)))
                        continue;

                    // Check if player is within the 120-degree frontal cone
                    if (!me->HasInArc(static_cast<float>(M_PI) * static_cast<float>(ARCING_SLASH_ANGLE) / 180.0f, player))
                        continue;

                    targets.push_back(player);
                }

                // Cast Arcing Slash on each target in the cone and apply Jagged Tear
                for (Player* target : targets)
                {
                    me->CastSpell(target, SPELL_ARCING_SLASH, true);
                    me->CastSpell(target, SPELL_JAGGED_TEAR, true);
                }
            }

            void DoHurlSpear()
            {
                // Select a random player target
                Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true);
                if (!target)
                    return;

                // Cast Hurl Spear visual
                DoCast(target, SPELL_HURL_SPEAR);

                // Shannox no longer has his spear - remove weapon from his hand
                _hasSpear = false;
                me->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID, 0);

                // Spawn the Spear of Shannox at the target location after delay
                // The spear creature handles the landing sequence
                Position spearPos = target->GetPosition();
                if (TempSummon* spear = me->SummonCreature(NPC_SPEAR_OF_SHANNOX, spearPos, TEMPSUMMON_MANUAL_DESPAWN))
                {
                    _spearGUID = spear->GetGUID();
                    spear->AI()->SetGUID(me->GetGUID(), 0); // Pass Shannox GUID to spear
                    spear->AI()->SetGUID(_riplimbGUID, 1);  // Pass Riplimb GUID to spear

                    // Also spawn the visible spear model
                    if (TempSummon* visibleSpear = me->SummonCreature(NPC_SPEAR_OF_SHANNOX_VISUAL, spearPos, TEMPSUMMON_MANUAL_DESPAWN))
                    {
                        _spearVisualGUID = visibleSpear->GetGUID();
                        visibleSpear->SetReactState(REACT_PASSIVE);
                        visibleSpear->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                    }
                }
            }

            void DoThrowTraps()
            {
                // Throw Immolation Trap at a random player
                if (Unit* target1 = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                {
                    Position trapPos = target1->GetPosition();
                    if (TempSummon* trap = me->SummonCreature(NPC_IMMOLATION_TRAP, trapPos, TEMPSUMMON_TIMED_DESPAWN, 60000))
                    {
                        trap->SetReactState(REACT_PASSIVE);
                    }
                }

                // Throw Crystal Prison Trap at a random player (can be same or different)
                if (Unit* target2 = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                {
                    Position trapPos = target2->GetPosition();
                    if (TempSummon* trap = me->SummonCreature(NPC_CRYSTAL_PRISON_TRAP, trapPos, TEMPSUMMON_TIMED_DESPAWN, 60000))
                    {
                        trap->SetReactState(REACT_PASSIVE);
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetFirelandsAI<boss_shannoxAI>(creature);
        }
};

// Base AI for Riplimb and Rageface
struct npc_shannox_houndAI : public ScriptedAI
{
    npc_shannox_houndAI(Creature* creature) : ScriptedAI(creature)
    {
        _instance = creature->GetInstanceScript();
    }

    void Reset() override
    {
        _events.Reset();
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        _events.ScheduleEvent(EVENT_CHECK_SEPARATION, SEPARATION_CHECK_TIMER);
    }

    void EnterEvadeMode(EvadeReason why) override
    {
        // Only evade if Shannox is not in combat
        if (Creature* shannox = _instance->GetCreature(DATA_SHANNOX))
            if (shannox->IsInCombat())
                return;

        ScriptedAI::EnterEvadeMode(why);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_CHECK_SEPARATION:
                    CheckSeparationAnxiety();
                    _events.ScheduleEvent(EVENT_CHECK_SEPARATION, SEPARATION_CHECK_TIMER);
                    break;
                default:
                    break;
            }
        }

        DoMeleeAttackIfReady();
    }

private:
    void CheckSeparationAnxiety()
    {
        Creature* shannox = _instance->GetCreature(DATA_SHANNOX);
        if (!shannox || !shannox->IsAlive())
            return;

        float distance = me->GetDistance(shannox);

        if (distance > static_cast<float>(SEPARATION_DISTANCE))
        {
            // Apply or refresh Separation Anxiety if more than 60 yards from Shannox
            if (!me->HasAura(SPELL_SEPARATION_ANXIETY))
                me->CastSpell(me, SPELL_SEPARATION_ANXIETY, true);
            else
            {
                // Refresh the aura duration
                if (Aura* separationAnxiety = me->GetAura(SPELL_SEPARATION_ANXIETY))
                    separationAnxiety->RefreshDuration();
            }
        }
        else
        {
            // Remove Separation Anxiety if within 60 yards of Shannox
            me->RemoveAurasDueToSpell(SPELL_SEPARATION_ANXIETY);
        }
    }

protected:
    InstanceScript* _instance;
    EventMap _events;
};

// 53694 - Riplimb
class npc_riplimb : public CreatureScript
{
    public:
        npc_riplimb() : CreatureScript("npc_riplimb") { }

        struct npc_riplimbAI : public npc_shannox_houndAI
        {
            npc_riplimbAI(Creature* creature) : npc_shannox_houndAI(creature),
                _fetchingSpear(false), _frenziedDevotionApplied(false) { }

            void Reset() override
            {
                npc_shannox_houndAI::Reset();
                _fetchingSpear = false;
                _frenziedDevotionApplied = false;
                _spearGUID.Clear();
            }

            void JustEngagedWith(Unit* who) override
            {
                npc_shannox_houndAI::JustEngagedWith(who);

                // Riplimb is immune to taunt
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, true);

                // Schedule Limb Rip - every 10 seconds
                _events.ScheduleEvent(EVENT_LIMB_RIP, LIMB_RIP_TIMER);

                // Schedule Shannox HP check for Frenzied Devotion (normal mode only)
                if (!me->GetMap()->IsHeroic())
                    _events.ScheduleEvent(EVENT_CHECK_SHANNOX_HP, SHANNOX_HP_CHECK_TIMER);
            }

            void DoAction(int32 action) override
            {
                switch (action)
                {
                    case ACTION_FETCH_SPEAR:
                        // Get the spear GUID from Shannox
                        if (Creature* shannox = _instance->GetCreature(DATA_SHANNOX))
                        {
                            _spearGUID = shannox->AI()->GetGUID(NPC_SPEAR_OF_SHANNOX);
                            if (!_spearGUID.IsEmpty())
                            {
                                _fetchingSpear = true;
                                // Stop attacking and go fetch the spear
                                me->AttackStop();
                                me->SetReactState(REACT_PASSIVE);

                                // Apply Dogged Determination - movement speed buff while fetching
                                DoCastSelf(SPELL_DOGGED_DETERMINATION, true);

                                if (Creature* spear = ObjectAccessor::GetCreature(*me, _spearGUID))
                                    me->GetMotionMaster()->MovePoint(1, spear->GetPosition());
                            }
                        }
                        break;
                    default:
                        break;
                }
            }

            void MovementInform(uint32 type, uint32 id) override
            {
                if (type != POINT_MOTION_TYPE)
                    return;

                if (id == 1) // Reached spear
                {
                    // Schedule return to Shannox
                    _events.ScheduleEvent(EVENT_RETURN_SPEAR, 500);
                }
                else if (id == 2) // Reached Shannox
                {
                    // Return spear to Shannox
                    if (Creature* shannox = _instance->GetCreature(DATA_SHANNOX))
                        shannox->AI()->DoAction(ACTION_SPEAR_RETRIEVED);

                    _fetchingSpear = false;
                    _spearGUID.Clear();

                    // Remove Dogged Determination
                    me->RemoveAurasDueToSpell(SPELL_DOGGED_DETERMINATION);

                    // Resume combat
                    me->SetReactState(REACT_AGGRESSIVE);
                    if (Creature* shannox = _instance->GetCreature(DATA_SHANNOX))
                        if (Unit* victim = shannox->GetVictim())
                            AttackStart(victim);
                }
            }

            void JustDied(Unit* /*killer*/) override
            {
                // Notify Shannox that Riplimb has died
                if (Creature* shannox = _instance->GetCreature(DATA_SHANNOX))
                    shannox->AI()->DoAction(ACTION_RIPLIMB_DEAD);
            }

            void UpdateAI(uint32 diff) override
            {
                _events.Update(diff);

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_CHECK_SEPARATION:
                            if (!_fetchingSpear)
                                CheckSeparationAnxiety();
                            _events.ScheduleEvent(EVENT_CHECK_SEPARATION, SEPARATION_CHECK_TIMER);
                            break;
                        case EVENT_RETURN_SPEAR:
                            // Move back to Shannox with the spear
                            if (Creature* shannox = _instance->GetCreature(DATA_SHANNOX))
                                me->GetMotionMaster()->MovePoint(2, shannox->GetPosition());
                            break;
                        case EVENT_LIMB_RIP:
                            // Cast Limb Rip on current target (applies Jagged Tear)
                            if (Unit* victim = me->GetVictim())
                            {
                                DoCast(victim, SPELL_LIMB_RIP);
                                // Also apply Jagged Tear stacking debuff
                                DoCast(victim, SPELL_JAGGED_TEAR, true);
                            }
                            _events.ScheduleEvent(EVENT_LIMB_RIP, LIMB_RIP_TIMER);
                            break;
                        case EVENT_CHECK_SHANNOX_HP:
                            // Check if Shannox is below 30% HP for Frenzied Devotion
                            if (!_frenziedDevotionApplied)
                            {
                                if (Creature* shannox = _instance->GetCreature(DATA_SHANNOX))
                                {
                                    if (shannox->IsAlive() && shannox->HealthBelowPct(SHANNOX_FRENZIED_HP_PCT))
                                    {
                                        DoCastSelf(SPELL_FRENZIED_DEVOTION, true);
                                        _frenziedDevotionApplied = true;
                                        break; // Don't reschedule
                                    }
                                }
                            }
                            if (!_frenziedDevotionApplied)
                                _events.ScheduleEvent(EVENT_CHECK_SHANNOX_HP, SHANNOX_HP_CHECK_TIMER);
                            break;
                        default:
                            break;
                    }
                }

                if (_fetchingSpear)
                    return;

                if (!UpdateVictim())
                    return;

                DoMeleeAttackIfReady();
            }

        private:
            void CheckSeparationAnxiety()
            {
                Creature* shannox = _instance->GetCreature(DATA_SHANNOX);
                if (!shannox || !shannox->IsAlive())
                    return;

                float distance = me->GetDistance(shannox);

                if (distance > static_cast<float>(SEPARATION_DISTANCE))
                {
                    if (!me->HasAura(SPELL_SEPARATION_ANXIETY))
                        me->CastSpell(me, SPELL_SEPARATION_ANXIETY, true);
                    else if (Aura* separationAnxiety = me->GetAura(SPELL_SEPARATION_ANXIETY))
                        separationAnxiety->RefreshDuration();
                }
                else
                    me->RemoveAurasDueToSpell(SPELL_SEPARATION_ANXIETY);
            }

            bool _fetchingSpear;
            bool _frenziedDevotionApplied;
            ObjectGuid _spearGUID;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetFirelandsAI<npc_riplimbAI>(creature);
        }
};

// 53695 - Rageface
class npc_rageface : public CreatureScript
{
    public:
        npc_rageface() : CreatureScript("npc_rageface") { }

        struct npc_ragefaceAI : public npc_shannox_houndAI
        {
            npc_ragefaceAI(Creature* creature) : npc_shannox_houndAI(creature),
                _faceRageActive(false), _faceRageDamageMultiplier(1), _frenziedDevotionApplied(false) { }

            void Reset() override
            {
                npc_shannox_houndAI::Reset();
                _faceRageActive = false;
                _faceRageDamageMultiplier = 1;
                _frenziedDevotionApplied = false;
                _faceRageTargetGUID.Clear();
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                // Rageface has no threat table - attacks random players
                me->SetReactState(REACT_PASSIVE);

                // Schedule random target change every 15 seconds
                _events.ScheduleEvent(EVENT_CHANGE_TARGET, 100); // Initial target selection
                _events.ScheduleEvent(EVENT_CHECK_SEPARATION, SEPARATION_CHECK_TIMER);

                // Schedule Face Rage every 45 seconds
                _events.ScheduleEvent(EVENT_FACE_RAGE, FACE_RAGE_TIMER);

                // Schedule Shannox HP check for Frenzied Devotion (normal mode only)
                if (!me->GetMap()->IsHeroic())
                    _events.ScheduleEvent(EVENT_CHECK_SHANNOX_HP, SHANNOX_HP_CHECK_TIMER);
            }

            void DamageTaken(Unit* /*attacker*/, uint32& damage) override
            {
                // Check if Face Rage should be interrupted by 30k+ damage
                if (_faceRageActive && damage >= FACE_RAGE_INTERRUPT_DAMAGE)
                {
                    EndFaceRage();
                }
            }

            void JustDied(Unit* /*killer*/) override
            {
                // End Face Rage if active
                if (_faceRageActive)
                    EndFaceRage();

                // Notify Shannox that Rageface has died
                if (Creature* shannox = _instance->GetCreature(DATA_SHANNOX))
                    shannox->AI()->DoAction(ACTION_RAGEFACE_DEAD);
            }

            void UpdateAI(uint32 diff) override
            {
                _events.Update(diff);

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_CHECK_SEPARATION:
                            CheckSeparationAnxiety();
                            _events.ScheduleEvent(EVENT_CHECK_SEPARATION, SEPARATION_CHECK_TIMER);
                            break;
                        case EVENT_CHANGE_TARGET:
                            // Don't change target during Face Rage
                            if (!_faceRageActive)
                            {
                                SelectNewRandomTarget();
                            }
                            _events.ScheduleEvent(EVENT_CHANGE_TARGET, RAGEFACE_TARGET_CHANGE_TIMER);
                            break;
                        case EVENT_FACE_RAGE:
                            StartFaceRage();
                            _events.ScheduleEvent(EVENT_FACE_RAGE, FACE_RAGE_TIMER);
                            break;
                        case EVENT_FACE_RAGE_TICK:
                            DoFaceRageDamage();
                            break;
                        case EVENT_CHECK_SHANNOX_HP:
                            // Check if Shannox is below 30% HP for Frenzied Devotion
                            if (!_frenziedDevotionApplied)
                            {
                                if (Creature* shannox = _instance->GetCreature(DATA_SHANNOX))
                                {
                                    if (shannox->IsAlive() && shannox->HealthBelowPct(SHANNOX_FRENZIED_HP_PCT))
                                    {
                                        DoCastSelf(SPELL_FRENZIED_DEVOTION, true);
                                        _frenziedDevotionApplied = true;
                                        break; // Don't reschedule
                                    }
                                }
                            }
                            if (!_frenziedDevotionApplied)
                                _events.ScheduleEvent(EVENT_CHECK_SHANNOX_HP, SHANNOX_HP_CHECK_TIMER);
                            break;
                        default:
                            break;
                    }
                }

                // Only do melee if not in Face Rage (Face Rage handles its own damage)
                if (!_faceRageActive)
                {
                    if (Unit* victim = me->GetVictim())
                    {
                        if (me->IsWithinMeleeRange(victim))
                            DoMeleeAttackIfReady();
                        else
                            me->GetMotionMaster()->MoveChase(victim);
                    }
                }
            }

        private:
            bool _faceRageActive;
            uint32 _faceRageDamageMultiplier;
            bool _frenziedDevotionApplied;
            ObjectGuid _faceRageTargetGUID;

            void CheckSeparationAnxiety()
            {
                Creature* shannox = _instance->GetCreature(DATA_SHANNOX);
                if (!shannox || !shannox->IsAlive())
                    return;

                float distance = me->GetDistance(shannox);

                if (distance > static_cast<float>(SEPARATION_DISTANCE))
                {
                    if (!me->HasAura(SPELL_SEPARATION_ANXIETY))
                        me->CastSpell(me, SPELL_SEPARATION_ANXIETY, true);
                    else if (Aura* separationAnxiety = me->GetAura(SPELL_SEPARATION_ANXIETY))
                        separationAnxiety->RefreshDuration();
                }
                else
                    me->RemoveAurasDueToSpell(SPELL_SEPARATION_ANXIETY);
            }

            void SelectNewRandomTarget()
            {
                // Get all players in the instance
                std::vector<Player*> validTargets;
                Map::PlayerList const& playerList = me->GetMap()->GetPlayers();

                for (auto const& playerRef : playerList)
                {
                    Player* player = playerRef.GetSource();
                    if (!player || !player->IsAlive() || player->IsGameMaster())
                        continue;

                    validTargets.push_back(player);
                }

                if (validTargets.empty())
                    return;

                // Pick a random target
                Player* newTarget = validTargets[urand(0, validTargets.size() - 1)];
                AttackStart(newTarget);
                me->GetMotionMaster()->MoveChase(newTarget);
            }

            void StartFaceRage()
            {
                // Select a random player for Face Rage
                std::vector<Player*> validTargets;
                Map::PlayerList const& playerList = me->GetMap()->GetPlayers();

                for (auto const& playerRef : playerList)
                {
                    Player* player = playerRef.GetSource();
                    if (!player || !player->IsAlive() || player->IsGameMaster())
                        continue;

                    validTargets.push_back(player);
                }

                if (validTargets.empty())
                    return;

                Player* target = validTargets[urand(0, validTargets.size() - 1)];
                _faceRageTargetGUID = target->GetGUID();
                _faceRageActive = true;
                _faceRageDamageMultiplier = 1;

                // Cast Face Rage (leap and stun)
                DoCast(target, SPELL_FACE_RAGE);

                // Move to target and start damage ticks
                me->GetMotionMaster()->MovePoint(0, target->GetPosition());
                _events.ScheduleEvent(EVENT_FACE_RAGE_TICK, FACE_RAGE_TICK_TIMER);
            }

            void DoFaceRageDamage()
            {
                if (!_faceRageActive)
                    return;

                Player* target = ObjectAccessor::GetPlayer(*me, _faceRageTargetGUID);
                if (!target || !target->IsAlive())
                {
                    EndFaceRage();
                    return;
                }

                // Deal escalating damage: 8k * multiplier
                uint32 damage = FACE_RAGE_BASE_DAMAGE * _faceRageDamageMultiplier;
                Unit::DealDamage(me, target, damage, 0, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false);

                // Increase damage for next tick
                _faceRageDamageMultiplier++;

                // Schedule next tick
                _events.ScheduleEvent(EVENT_FACE_RAGE_TICK, FACE_RAGE_TICK_TIMER);
            }

            void EndFaceRage()
            {
                _faceRageActive = false;
                _faceRageDamageMultiplier = 1;

                // Remove Face Rage stun from target
                if (Player* target = ObjectAccessor::GetPlayer(*me, _faceRageTargetGUID))
                    target->RemoveAurasDueToSpell(SPELL_FACE_RAGE);

                _faceRageTargetGUID.Clear();

                // Cancel damage tick event
                _events.CancelEvent(EVENT_FACE_RAGE_TICK);

                // Resume normal random target behavior
                SelectNewRandomTarget();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetFirelandsAI<npc_ragefaceAI>(creature);
        }
};

// 53752 - Spear of Shannox
class npc_spear_of_shannox : public CreatureScript
{
    public:
        npc_spear_of_shannox() : CreatureScript("npc_spear_of_shannox") { }

        struct npc_spear_of_shannoxAI : public ScriptedAI
        {
            npc_spear_of_shannoxAI(Creature* creature) : ScriptedAI(creature)
            {
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
            }

            void Reset() override
            {
                _events.Reset();
                _shannoxGUID.Clear();
                _riplimbGUID.Clear();
            }

            void SetGUID(ObjectGuid const& guid, int32 id) override
            {
                switch (id)
                {
                    case 0:
                        _shannoxGUID = guid;
                        // Start the landing sequence after 3 seconds
                        _events.ScheduleEvent(EVENT_SPEAR_LAND, SPEAR_LAND_DELAY);
                        break;
                    case 1:
                        _riplimbGUID = guid;
                        break;
                }
            }

            void UpdateAI(uint32 diff) override
            {
                _events.Update(diff);

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_SPEAR_LAND:
                            DoSpearLand();
                            _events.ScheduleEvent(EVENT_MAGMA_RUPTURE, MAGMA_RUPTURE_DELAY);
                            break;
                        case EVENT_MAGMA_RUPTURE:
                            DoMagmaRupture();
                            _events.ScheduleEvent(EVENT_SIGNAL_RIPLIMB, 500);
                            break;
                        case EVENT_SIGNAL_RIPLIMB:
                            // Tell Riplimb to fetch the spear
                            if (Creature* riplimb = ObjectAccessor::GetCreature(*me, _riplimbGUID))
                                riplimb->AI()->DoAction(ACTION_FETCH_SPEAR);
                            break;
                        default:
                            break;
                    }
                }
            }

        private:
            EventMap _events;
            ObjectGuid _shannoxGUID;
            ObjectGuid _riplimbGUID;

            void DoSpearLand()
            {
                // Cast visual spell to show the spear stuck in the ground
                DoCastSelf(SPELL_SPEAR_VISUAL);

                // Deal 100k physical damage to all players within radius
                Map::PlayerList const& playerList = me->GetMap()->GetPlayers();
                for (auto const& playerRef : playerList)
                {
                    Player* player = playerRef.GetSource();
                    if (!player || !player->IsAlive())
                        continue;

                    if (me->IsWithinDistInMap(player, static_cast<float>(SPEAR_LAND_DAMAGE_RADIUS)))
                    {
                        // Deal 100k physical damage
                        Unit::DealDamage(me, player, SPEAR_LAND_DAMAGE, 0, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false);
                    }
                }

                // Cast Magma Flare - raid-wide 40k fire damage
                DoCastAOE(SPELL_MAGMA_FLARE);

                // Spawn Spiral Flames in a circular pattern
                SpawnSpiralFlames();
            }

            void SpawnSpiralFlames()
            {
                float angleStep = 2.0f * static_cast<float>(M_PI) / static_cast<float>(SPIRAL_FLAME_COUNT);
                for (uint8 i = 0; i < SPIRAL_FLAME_COUNT; ++i)
                {
                    float angle = static_cast<float>(i) * angleStep;
                    float x = me->GetPositionX() + static_cast<float>(SPIRAL_FLAME_RADIUS) * std::cos(angle);
                    float y = me->GetPositionY() + static_cast<float>(SPIRAL_FLAME_RADIUS) * std::sin(angle);
                    float z = me->GetPositionZ();

                    if (TempSummon* flame = me->SummonCreature(NPC_SPIRAL_FLAME, x, y, z, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 10000))
                    {
                        // Spiral flame will erupt after 2 seconds (same as MAGMA_RUPTURE_DELAY)
                        // The spiral flame handles its own eruption
                    }
                }
            }

            void DoMagmaRupture()
            {
                // The spiral flames erupt - this is handled by their own AI
                // We just need to ensure the timing is synchronized
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetFirelandsAI<npc_spear_of_shannoxAI>(creature);
        }
};

// 54276 - Spiral Flame
class npc_spiral_flame : public CreatureScript
{
    public:
        npc_spiral_flame() : CreatureScript("npc_spiral_flame") { }

        struct npc_spiral_flameAI : public ScriptedAI
        {
            npc_spiral_flameAI(Creature* creature) : ScriptedAI(creature)
            {
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
            }

            void Reset() override
            {
                _events.Reset();
            }

            void IsSummonedBy(Unit* /*summoner*/) override
            {
                // Cast visual spell to show fire patch on ground
                DoCastSelf(SPELL_MAGMA_RUPTURE_VISUAL);

                // Schedule eruption 2 seconds after spawn
                _events.ScheduleEvent(EVENT_SPIRAL_FLAME_ERUPT, MAGMA_RUPTURE_DELAY);
            }

            void UpdateAI(uint32 diff) override
            {
                _events.Update(diff);

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_SPIRAL_FLAME_ERUPT:
                            // Cast Magma Rupture - 70k fire damage to nearby players
                            DoCastAOE(SPELL_MAGMA_RUPTURE);
                            break;
                        default:
                            break;
                    }
                }
            }

        private:
            EventMap _events;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetFirelandsAI<npc_spiral_flameAI>(creature);
        }
};

// 53724 - Immolation Trap
class npc_immolation_trap : public CreatureScript
{
    public:
        npc_immolation_trap() : CreatureScript("npc_immolation_trap") { }

        struct npc_immolation_trapAI : public ScriptedAI
        {
            npc_immolation_trapAI(Creature* creature) : ScriptedAI(creature), _armed(false), _triggered(false)
            {
                me->SetReactState(REACT_PASSIVE);
            }

            void Reset() override
            {
                _events.Reset();
                _armed = false;
                _triggered = false;
            }

            void IsSummonedBy(Unit* /*summoner*/) override
            {
                // Trap arms after a short delay
                _events.ScheduleEvent(EVENT_CHECK_TRAP_TRIGGER, TRAP_ARM_DELAY);
            }

            void UpdateAI(uint32 diff) override
            {
                if (_triggered)
                    return;

                _events.Update(diff);

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_CHECK_TRAP_TRIGGER:
                            if (!_armed)
                            {
                                _armed = true;
                                // Continue checking for players
                            }

                            if (_armed)
                            {
                                // Check for players stepping on the trap
                                if (Player* player = me->SelectNearestPlayer(static_cast<float>(TRAP_TRIGGER_RANGE)))
                                {
                                    if (player->IsAlive() && !player->IsGameMaster())
                                    {
                                        TriggerTrap(player);
                                        return;
                                    }
                                }
                            }

                            // Keep checking every 200ms
                            _events.ScheduleEvent(EVENT_CHECK_TRAP_TRIGGER, 200);
                            break;
                        default:
                            break;
                    }
                }
            }

        private:
            EventMap _events;
            bool _armed;
            bool _triggered;

            void TriggerTrap(Player* victim)
            {
                _triggered = true;

                // Deal 65k fire damage
                me->CastSpell(victim, SPELL_IMMOLATION_TRAP, true);

                // Apply the DoT debuff (51k over 9 seconds + 40% increased damage taken)
                me->CastSpell(victim, SPELL_IMMOLATION_DOT, true);

                // Despawn after triggering
                me->DespawnOrUnsummon(500);
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetFirelandsAI<npc_immolation_trapAI>(creature);
        }
};

// 53713 - Crystal Prison Trap
class npc_crystal_prison_trap : public CreatureScript
{
    public:
        npc_crystal_prison_trap() : CreatureScript("npc_crystal_prison_trap") { }

        struct npc_crystal_prison_trapAI : public ScriptedAI
        {
            npc_crystal_prison_trapAI(Creature* creature) : ScriptedAI(creature), _armed(false), _triggered(false)
            {
                me->SetReactState(REACT_PASSIVE);
            }

            void Reset() override
            {
                _events.Reset();
                _armed = false;
                _triggered = false;
            }

            void IsSummonedBy(Unit* /*summoner*/) override
            {
                // Trap arms after a short delay
                _events.ScheduleEvent(EVENT_CHECK_TRAP_TRIGGER, TRAP_ARM_DELAY);
            }

            void UpdateAI(uint32 diff) override
            {
                if (_triggered)
                    return;

                _events.Update(diff);

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_CHECK_TRAP_TRIGGER:
                            if (!_armed)
                            {
                                _armed = true;
                            }

                            if (_armed)
                            {
                                // Check for players stepping on the trap
                                if (Player* player = me->SelectNearestPlayer(static_cast<float>(TRAP_TRIGGER_RANGE)))
                                {
                                    if (player->IsAlive() && !player->IsGameMaster())
                                    {
                                        TriggerTrap(player);
                                        return;
                                    }
                                }
                            }

                            // Keep checking every 200ms
                            _events.ScheduleEvent(EVENT_CHECK_TRAP_TRIGGER, 200);
                            break;
                        default:
                            break;
                    }
                }
            }

        private:
            EventMap _events;
            bool _armed;
            bool _triggered;

            void TriggerTrap(Player* victim)
            {
                _triggered = true;

                // Spawn the Crystal Prison on the player
                Position prisonPos = victim->GetPosition();
                if (TempSummon* prison = me->SummonCreature(NPC_CRYSTAL_PRISON, prisonPos, TEMPSUMMON_MANUAL_DESPAWN))
                {
                    // Pass the victim's GUID to the prison
                    prison->AI()->SetGUID(victim->GetGUID(), 0);
                }

                // Despawn the trap after triggering
                me->DespawnOrUnsummon(500);
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetFirelandsAI<npc_crystal_prison_trapAI>(creature);
        }
};

// 53819 - Crystal Prison (the actual prison encasing a player)
class npc_crystal_prison : public CreatureScript
{
    public:
        npc_crystal_prison() : CreatureScript("npc_crystal_prison") { }

        struct npc_crystal_prisonAI : public ScriptedAI
        {
            npc_crystal_prisonAI(Creature* creature) : ScriptedAI(creature)
            {
                me->SetReactState(REACT_PASSIVE);
            }

            void Reset() override
            {
                _prisonedPlayerGUID.Clear();
            }

            void SetGUID(ObjectGuid const& guid, int32 /*id*/) override
            {
                _prisonedPlayerGUID = guid;

                // Set the prison's health to 2.8 million
                me->SetMaxHealth(CRYSTAL_PRISON_HEALTH);
                me->SetHealth(CRYSTAL_PRISON_HEALTH);

                // Apply imprisonment effect to the player
                if (Player* player = ObjectAccessor::GetPlayer(*me, _prisonedPlayerGUID))
                {
                    // Stun/root the player
                    me->CastSpell(player, SPELL_CRYSTAL_PRISON_EFFECT, true);
                }
            }

            void JustDied(Unit* /*killer*/) override
            {
                // Free the imprisoned player
                if (Player* player = ObjectAccessor::GetPlayer(*me, _prisonedPlayerGUID))
                {
                    player->RemoveAurasDueToSpell(SPELL_CRYSTAL_PRISON_EFFECT);
                }
            }

            void UpdateAI(uint32 /*diff*/) override
            {
                // Check if the imprisoned player has died or left
                if (!_prisonedPlayerGUID.IsEmpty())
                {
                    Player* player = ObjectAccessor::GetPlayer(*me, _prisonedPlayerGUID);
                    if (!player || !player->IsAlive())
                    {
                        // Player died or left, despawn the prison
                        me->DespawnOrUnsummon();
                    }
                }
            }

        private:
            ObjectGuid _prisonedPlayerGUID;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetFirelandsAI<npc_crystal_prisonAI>(creature);
        }
};

}

void AddSC_boss_shannox()
{
    using namespace Firelands;
    using namespace Firelands::Shannox;
    new boss_shannox();
    new npc_riplimb();
    new npc_rageface();
    new npc_spear_of_shannox();
    new npc_spiral_flame();
    new npc_immolation_trap();
    new npc_crystal_prison_trap();
    new npc_crystal_prison();
}
