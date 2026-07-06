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
 * Shannox, Firelands (10/25 Normal, 10/25 Heroic)
 *
 * Difficulty model (verified against 4.3.4 JournalEncounterSection.dbc and SpellDifficulty.dbc):
 *  - Damage scaling: the core resolves difficulty spell variants at cast time via
 *    SpellDifficulty.dbc (Arcing Slash, Jagged Tear, Immolation Trap, Face Rage aura,
 *    Wary, Magma Rupture eruption, Frenzy). Scripts always cast the 10N base ID.
 *  - Heroic: Frenzied Devotion never triggers; both dogs carry Feeding Frenzy (DBC proc
 *    aura, stacks on landed melee only). Riplimb cannot be permanently slain while
 *    Shannox lives - at zero health he collapses (feign death) for 30 seconds, then
 *    reanimates at full health. Rageface dies permanently on ALL difficulties.
 *  - Frenzy is granted whenever a dog is defeated (death or collapse) and is never
 *    removed; the aura itself caps at 2 stacks.
 *  - Magma Rupture replaces Hurl Spear while Riplimb is dead or collapsed, on all
 *    difficulties; on Heroic it stops when Riplimb reanimates.
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
#include "ThreatManager.h"
#include <cmath>

namespace Firelands::Shannox
{
enum Spells
{
    // Shannox
    SPELL_BERSERK                   = 26662,
    SPELL_ARCING_SLASH              = 99931,  // Frontal cone, triggers Jagged Tear (99937) itself. Difficulty chained.
    SPELL_HURL_SPEAR                = 100002, // Dummy + physical damage around the impact destination
    SPELL_MAGMA_FLARE               = 100495, // Raid-wide fire damage on spear impact
    SPELL_SPEAR_VISUAL              = 100035, // Visual for spear stuck in the ground
    SPELL_MAGMA_RUPTURE_CAST        = 99840,  // Raid damage + stacking +40% fire damage taken. Riplimb-down replacement for Hurl Spear.
    SPELL_MAGMA_RUPTURE_VISUAL      = 99841,  // Eruption missile, triggers 99842 (difficulty chained) on impact
    SPELL_FRENZY                    = 100522, // On dog defeat. Max 2 stacks (DBC). Difficulty chained.

    // Riplimb & Rageface
    SPELL_SEPARATION_ANXIETY        = 99835,  // +100% damage/attack speed while >60yd from Shannox
    SPELL_FRENZIED_DEVOTION         = 100064, // Normal only: dog enrage at Shannox 30%
    SPELL_FEEDING_FRENZY            = 100655, // Heroic only: proc aura, stacks 100656 on landed melee
    SPELL_FEEDING_FRENZY_STACK      = 100656,
    SPELL_WARY                      = 100167, // 15s trap immunity after a dog triggers a trap. Difficulty chained.

    // Riplimb
    SPELL_LIMB_RIP                  = 99832,  // Triggers Jagged Tear (99937) itself
    SPELL_DOGGED_DETERMINATION      = 101111, // Minimum-speed floor while fetching the spear

    // Rageface
    SPELL_FACE_RAGE                 = 99945,  // Jump + stun, starts the maul
    SPELL_FACE_RAGE_AURA            = 99947,  // Victim aura: stun + ramping periodic (scripted ramp below)
    SPELL_FACE_RAGE_SELF            = 100129, // Rageface self aura: +100% crit taken; eff1 holds the break threshold. Chained.

    // Traps
    SPELL_THROW_IMMOLATION_TRAP     = 99839,  // Summons NPC 53724 at destination
    SPELL_IMMOLATION_TRAP_DAMAGE    = 99838,  // Fire hit + DoT + debuff. Difficulty chained.
    SPELL_THROW_CRYSTAL_TRAP        = 99836,  // Summons NPC 53713 at destination
    SPELL_CRYSTAL_PRISON_EFFECT     = 99837,  // Stun + school immunity on the prisoner
};

// Face Rage self-aura difficulty variants (100129 chain) - needed for manual removal
uint32 const FaceRageSelfSpells[4] = { 100129, 101212, 101213, 101214 };
// Wary difficulty variants (100167 chain) - trap scan must see all of them
uint32 const WarySpells[4] = { 100167, 101215, 101216, 101217 };

enum Events
{
    // Shannox
    EVENT_ARCING_SLASH = 1,
    EVENT_HURL_SPEAR,
    EVENT_IMMOLATION_TRAP,
    EVENT_CRYSTAL_PRISON_TRAP,
    EVENT_MAGMA_RUPTURE,
    EVENT_BERSERK,

    // Riplimb & Rageface
    EVENT_CHECK_SEPARATION,

    // Riplimb
    EVENT_LIMB_RIP,
    EVENT_RETURN_SPEAR,
    EVENT_UPDATE_RETURN_PATH,
    EVENT_RIPLIMB_REANIMATE,

    // Rageface
    EVENT_CHANGE_TARGET,
    EVENT_FACE_RAGE,

    // Spear of Shannox
    EVENT_SPEAR_LAND,
    EVENT_SIGNAL_RIPLIMB,

    // Spiral Flame
    EVENT_SPIRAL_FLAME_ERUPT,

    // Traps
    EVENT_CHECK_TRAP_TRIGGER,

    // Crystal Prison
    EVENT_PRISON_AUTO_BREAK,
    EVENT_CHECK_PRISONER,
};

enum Texts
{
    SAY_SPAWN                   = 0,  // Patrol/spawn text
    SAY_AGGRO                   = 1,
    SAY_SLAY                    = 2,
    SAY_RIPLIMB_DEAD            = 3,
    SAY_RAGEFACE_DEAD           = 4,
    SAY_DEATH                   = 5,
    SAY_HURL_SPEAR              = 6,
};

enum Creatures
{
    NPC_SPEAR_OF_SHANNOX        = 53752,  // Invisible trigger for spear mechanics
    NPC_SPEAR_OF_SHANNOX_VISUAL = 54112,  // Visible spear model on ground
    NPC_SPIRAL_FLAME            = 54276,
    NPC_IMMOLATION_TRAP         = 53724,
    NPC_CRYSTAL_PRISON_TRAP     = 53713,
    NPC_CRYSTAL_PRISON          = 53819,  // The actual prison that encases players and dogs
};

enum Misc
{
    // AI-to-AI data queries
    DATA_HOUND_COLLAPSED        = 1,
    DATA_RIPLIMB_FETCHING       = 2,

    // Movement points
    POINT_SPEAR                 = 1,
    POINT_SHANNOX               = 2,

    SEPARATION_DISTANCE         = 60,       // yards
    SEPARATION_CHECK_TIMER      = 1000,

    ARCING_SLASH_TIMER          = 12000,
    HURL_SPEAR_FIRST            = 20500,    // first throw ~20s into the fight
    HURL_SPEAR_AFTER_RETURN     = 20000,    // re-armed only once Riplimb returns the spear
    HURL_SPEAR_RETRY            = 2000,     // Riplimb unavailable (prisoned/fetching/collapsed)
    BERSERK_TIMER               = 600000,   // 10 minutes, all difficulties

    SPEAR_LAND_DELAY            = 3000,     // spear travel time
    ERUPTION_LINES              = 4,        // radiating lines of molten eruptions
    ERUPTION_STEPS              = 8,        // eruptions per line, 5yd apart
    ERUPTION_STEP_DISTANCE      = 5,        // yards
    ERUPTION_TELEGRAPH          = 2000,     // delay before the first eruption
    ERUPTION_STEP_DELAY         = 500,      // additional delay per step outward

    MAGMA_RUPTURE_FIRST         = 15000,
    MAGMA_RUPTURE_REPEAT        = 30000,

    IMMOLATION_TRAP_TIMER       = 10000,
    CRYSTAL_PRISON_TRAP_TIMER   = 25000,
    TRAP_TRIGGER_RANGE          = 2,        // yards
    TRAP_ARM_DELAY              = 2000,
    TRAP_LIFETIME               = 300000,   // untriggered traps despawn after 5 minutes
    TRAP_DOG_POSITION_CHANCE    = 20,       // % chance an Immolation Trap drops under a dog

    LIMB_RIP_TIMER              = 9000,
    RIPLIMB_REANIMATE_TIMER     = 30000,    // Heroic collapse duration

    RAGEFACE_TARGET_CHANGE_MIN  = 10000,
    RAGEFACE_TARGET_CHANGE_MAX  = 20000,
    FACE_RAGE_TIMER             = 30000,
    PRISON_DOG_AUTO_BREAK       = 10000,    // trapped dogs break free after 10 seconds
};

enum Actions
{
    ACTION_FETCH_SPEAR          = 1,
    ACTION_SPEAR_RETRIEVED      = 2,
    ACTION_RIPLIMB_DEFEATED     = 3,    // Riplimb died (Normal) or collapsed (Heroic)
    ACTION_RIPLIMB_REVIVED      = 4,    // Heroic: Riplimb reanimated at full health
    ACTION_RAGEFACE_DEAD        = 5,
    ACTION_FRENZIED_DEVOTION    = 6,    // Normal: Shannox dropped below 30%
    ACTION_PRISON_BROKEN        = 7,    // Crystal Prison released its prisoner
    ACTION_FACE_RAGE_END        = 8,    // Face Rage victim aura removed (any reason)
};

inline bool HasWary(Unit const* unit)
{
    for (uint32 spellId : WarySpells)
        if (unit->HasAura(spellId))
            return true;
    return false;
}

inline uint32 FaceRageBreakThreshold(Creature const* rageface)
{
    return rageface->GetMap()->Is25ManRaid() ? 45000 : 30000;
}

// Spawns the radiating molten eruption pattern used by both the Hurl Spear impact
// and Magma Rupture. Lines of Spiral Flames erupt outward from the owner.
void SpawnEruptionCascade(Unit* owner)
{
    float baseAngle = frand(0.0f, 2.0f * static_cast<float>(M_PI));
    for (uint8 line = 0; line < ERUPTION_LINES; ++line)
    {
        float angle = baseAngle + line * (2.0f * static_cast<float>(M_PI) / static_cast<float>(ERUPTION_LINES));
        for (uint8 step = 1; step <= ERUPTION_STEPS; ++step)
        {
            float dist = static_cast<float>(step * ERUPTION_STEP_DISTANCE);
            float x = owner->GetPositionX() + dist * std::cos(angle);
            float y = owner->GetPositionY() + dist * std::sin(angle);
            float z = owner->GetPositionZ();
            uint32 eruptDelay = ERUPTION_TELEGRAPH + step * ERUPTION_STEP_DELAY;
            if (TempSummon* flame = owner->SummonCreature(NPC_SPIRAL_FLAME, x, y, z, 0.0f, TEMPSUMMON_TIMED_DESPAWN, eruptDelay + 4000))
                if (flame->IsAIEnabled())
                    flame->AI()->SetData(0, eruptDelay);
        }
    }
}

// 53691 - Shannox
class boss_shannox : public CreatureScript
{
    public:
        boss_shannox() : CreatureScript("boss_shannox") { }

        struct boss_shannoxAI : public BossAI
        {
            boss_shannoxAI(Creature* creature) : BossAI(creature, DATA_SHANNOX),
                _hasSpear(true), _devotionTriggered(false) { }

            void Reset() override
            {
                _Reset();
                _spearGUID.Clear();
                _spearVisualGUID.Clear();
                _hasSpear = true;
                _devotionTriggered = false;
                // Shannox cannot be taunted
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, true);
                // Restore weapon visual on reset
                me->LoadEquipment();
            }

            void JustEngagedWith(Unit* who) override
            {
                BossAI::JustEngagedWith(who);
                Talk(SAY_AGGRO);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

                events.ScheduleEvent(EVENT_ARCING_SLASH, ARCING_SLASH_TIMER);
                events.ScheduleEvent(EVENT_HURL_SPEAR, HURL_SPEAR_FIRST);
                events.ScheduleEvent(EVENT_IMMOLATION_TRAP, IMMOLATION_TRAP_TIMER);
                events.ScheduleEvent(EVENT_CRYSTAL_PRISON_TRAP, CRYSTAL_PRISON_TRAP_TIMER);
                events.ScheduleEvent(EVENT_BERSERK, BERSERK_TIMER);

                // Engaging Shannox engages both hounds
                for (uint32 data : { DATA_RIPLIMB, DATA_RAGEFACE })
                    if (Creature* dog = instance->GetCreature(data))
                        if (dog->IsAlive() && !dog->IsInCombat() && dog->IsAIEnabled())
                            dog->AI()->AttackStart(who);
            }

            void DamageTaken(Unit* /*attacker*/, uint32& damage) override
            {
                // Frenzied Devotion: Normal only, once, on any living dog when Shannox reaches 30%
                if (!me->GetMap()->IsHeroic() && !_devotionTriggered && me->HealthBelowPctDamaged(30, damage))
                {
                    _devotionTriggered = true;
                    for (uint32 data : { DATA_RIPLIMB, DATA_RAGEFACE })
                        if (Creature* dog = instance->GetCreature(data))
                            if (dog->IsAlive() && dog->IsAIEnabled())
                                dog->AI()->DoAction(ACTION_FRENZIED_DEVOTION);
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

                // The hounds do not survive their master
                for (uint32 data : { DATA_RIPLIMB, DATA_RAGEFACE })
                {
                    if (Creature* dog = instance->GetCreature(data))
                    {
                        if (!dog->IsAlive())
                            continue;
                        if (dog->IsAIEnabled() && dog->AI()->GetData(DATA_HOUND_COLLAPSED))
                            dog->DespawnOrUnsummon();
                        else
                            dog->KillSelf();
                    }
                }
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
                        if (_hasSpear)
                            break;
                        ReclaimSpear();
                        events.ScheduleEvent(EVENT_HURL_SPEAR, HURL_SPEAR_AFTER_RETURN);
                        break;
                    case ACTION_RIPLIMB_DEFEATED:
                        // Riplimb died (Normal) or collapsed (Heroic): Frenzy, conjure the
                        // spear back and switch to Magma Rupture until he returns (if ever).
                        Talk(SAY_RIPLIMB_DEAD);
                        DoCastSelf(SPELL_FRENZY, true);
                        ReclaimSpear();
                        events.CancelEvent(EVENT_HURL_SPEAR);
                        events.ScheduleEvent(EVENT_MAGMA_RUPTURE, MAGMA_RUPTURE_FIRST);
                        break;
                    case ACTION_RIPLIMB_REVIVED:
                        // Heroic: Riplimb reanimated - resume the Hurl Spear cycle
                        events.CancelEvent(EVENT_MAGMA_RUPTURE);
                        events.ScheduleEvent(EVENT_HURL_SPEAR, HURL_SPEAR_AFTER_RETURN);
                        break;
                    case ACTION_RAGEFACE_DEAD:
                        Talk(SAY_RAGEFACE_DEAD);
                        DoCastSelf(SPELL_FRENZY, true);
                        break;
                    default:
                        break;
                }
            }

            ObjectGuid GetGUID(int32 type) const override
            {
                if (type == NPC_SPEAR_OF_SHANNOX)
                    return _spearGUID;
                return ObjectGuid::Empty;
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
                            // Only usable while holding the spear
                            if (_hasSpear)
                                DoCastVictim(SPELL_ARCING_SLASH);
                            events.ScheduleEvent(EVENT_ARCING_SLASH, ARCING_SLASH_TIMER);
                            break;
                        case EVENT_HURL_SPEAR:
                            if (RiplimbCanFetch())
                                DoHurlSpear();
                            else
                                events.ScheduleEvent(EVENT_HURL_SPEAR, HURL_SPEAR_RETRY);
                            break;
                        case EVENT_IMMOLATION_TRAP:
                            DoThrowImmolationTrap();
                            events.ScheduleEvent(EVENT_IMMOLATION_TRAP, IMMOLATION_TRAP_TIMER);
                            break;
                        case EVENT_CRYSTAL_PRISON_TRAP:
                            DoThrowCrystalPrisonTrap();
                            events.ScheduleEvent(EVENT_CRYSTAL_PRISON_TRAP, CRYSTAL_PRISON_TRAP_TIMER);
                            break;
                        case EVENT_MAGMA_RUPTURE:
                            me->CastSpell(me->GetPosition(), SPELL_MAGMA_RUPTURE_CAST, false);
                            SpawnEruptionCascade(me);
                            events.ScheduleEvent(EVENT_MAGMA_RUPTURE, MAGMA_RUPTURE_REPEAT);
                            break;
                        case EVENT_BERSERK:
                            DoCastSelf(SPELL_BERSERK, true);
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
            ObjectGuid _spearGUID;
            ObjectGuid _spearVisualGUID;
            bool _hasSpear;
            bool _devotionTriggered;

            bool RiplimbCanFetch() const
            {
                Creature* riplimb = instance->GetCreature(DATA_RIPLIMB);
                return riplimb && riplimb->IsAlive() && riplimb->IsAIEnabled()
                    && !riplimb->AI()->GetData(DATA_HOUND_COLLAPSED)
                    && !riplimb->AI()->GetData(DATA_RIPLIMB_FETCHING)
                    && !riplimb->HasAura(SPELL_CRYSTAL_PRISON_EFFECT);
            }

            void ReclaimSpear()
            {
                if (Creature* spear = ObjectAccessor::GetCreature(*me, _spearGUID))
                    spear->DespawnOrUnsummon();
                _spearGUID.Clear();
                if (Creature* visibleSpear = ObjectAccessor::GetCreature(*me, _spearVisualGUID))
                    visibleSpear->DespawnOrUnsummon();
                _spearVisualGUID.Clear();

                if (!_hasSpear)
                {
                    _hasSpear = true;
                    me->LoadEquipment();
                    events.ScheduleEvent(EVENT_ARCING_SLASH, 6000);
                }
            }

            void DoHurlSpear()
            {
                Creature* riplimb = instance->GetCreature(DATA_RIPLIMB);
                if (!riplimb)
                    return;

                // The spear lands near Riplimb's current position - tank positioning matters
                Position dest = riplimb->GetRandomNearPosition(5.0f);

                Talk(SAY_HURL_SPEAR);

                // Shannox no longer has his spear - remove weapon from his hand
                _hasSpear = false;
                me->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID, 0);
                events.CancelEvent(EVENT_ARCING_SLASH);

                if (TempSummon* spear = me->SummonCreature(NPC_SPEAR_OF_SHANNOX, dest, TEMPSUMMON_MANUAL_DESPAWN))
                {
                    _spearGUID = spear->GetGUID();
                    spear->AI()->SetGUID(me->GetGUID(), 0);
                    spear->AI()->SetGUID(riplimb->GetGUID(), 1);

                    if (TempSummon* visibleSpear = me->SummonCreature(NPC_SPEAR_OF_SHANNOX_VISUAL, dest, TEMPSUMMON_MANUAL_DESPAWN))
                    {
                        _spearVisualGUID = visibleSpear->GetGUID();
                        visibleSpear->SetReactState(REACT_PASSIVE);
                        visibleSpear->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                    }
                }
            }

            void DoThrowImmolationTrap()
            {
                // Occasionally the trap is dropped right under one of the dogs
                if (roll_chance_i(TRAP_DOG_POSITION_CHANCE))
                {
                    uint32 data = urand(0, 1) ? DATA_RIPLIMB : DATA_RAGEFACE;
                    if (Creature* dog = instance->GetCreature(data))
                        if (dog->IsAlive() && dog->IsAIEnabled() && !dog->AI()->GetData(DATA_HOUND_COLLAPSED))
                        {
                            me->CastSpell(dog->GetPosition(), SPELL_THROW_IMMOLATION_TRAP, true);
                            return;
                        }
                }

                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    me->CastSpell(target->GetPosition(), SPELL_THROW_IMMOLATION_TRAP, true);
            }

            void DoThrowCrystalPrisonTrap()
            {
                // Heroic: dropped at the current tank so Riplimb can be baited into it
                if (me->GetMap()->IsHeroic())
                {
                    if (Unit* victim = me->GetVictim())
                    {
                        me->CastSpell(victim->GetPosition(), SPELL_THROW_CRYSTAL_TRAP, true);
                        return;
                    }
                }

                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    me->CastSpell(target->GetPosition(), SPELL_THROW_CRYSTAL_TRAP, true);
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
    npc_shannox_houndAI(Creature* creature, uint32 otherDogData) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript()), _otherDogData(otherDogData)
    {
    }

    void Reset() override
    {
        _events.Reset();
        // Clear any leftover collapse state from a wipe
        me->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        me->SetImmuneToAll(false);
        me->SetReactState(REACT_AGGRESSIVE);
        // Neither hound can be taunted
        me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
        me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, true);
    }

    void JustEngagedWith(Unit* who) override
    {
        _events.ScheduleEvent(EVENT_CHECK_SEPARATION, SEPARATION_CHECK_TIMER);

        // Heroic: successful melee attacks stack Feeding Frenzy (DBC proc aura)
        if (me->GetMap()->IsHeroic())
            DoCastSelf(SPELL_FEEDING_FRENZY, true);

        // Engaging a hound engages Shannox (and through him, the other hound)
        if (Creature* shannox = _instance->GetCreature(DATA_SHANNOX))
            if (shannox->IsAlive() && !shannox->IsInCombat() && shannox->IsAIEnabled())
                shannox->AI()->AttackStart(who);
    }

    void EnterEvadeMode(EvadeReason why) override
    {
        // Only evade if Shannox is not in combat
        if (Creature* shannox = _instance->GetCreature(DATA_SHANNOX))
            if (shannox->IsInCombat())
                return;

        ScriptedAI::EnterEvadeMode(why);
    }

protected:
    void CheckSeparationAnxiety()
    {
        Creature* shannox = _instance->GetCreature(DATA_SHANNOX);
        if (!shannox || !shannox->IsAlive())
            return;

        if (me->GetDistance(shannox) > static_cast<float>(SEPARATION_DISTANCE))
        {
            // Both the dog and Shannox enrage until the distance closes
            ApplyOrRefreshSeparationAnxiety(me);
            ApplyOrRefreshSeparationAnxiety(shannox);
        }
        else
        {
            me->RemoveAurasDueToSpell(SPELL_SEPARATION_ANXIETY);

            // Only calm Shannox down if the other dog is not out of range either
            bool otherFar = false;
            if (Creature* other = _instance->GetCreature(_otherDogData))
                otherFar = other->IsAlive() && other->GetDistance(shannox) > static_cast<float>(SEPARATION_DISTANCE);
            if (!otherFar)
                shannox->RemoveAurasDueToSpell(SPELL_SEPARATION_ANXIETY);
        }
    }

    static void ApplyOrRefreshSeparationAnxiety(Unit* unit)
    {
        if (Aura* aura = unit->GetAura(SPELL_SEPARATION_ANXIETY))
            aura->RefreshDuration();
        else
            unit->CastSpell(unit, SPELL_SEPARATION_ANXIETY, true);
    }

    InstanceScript* _instance;
    EventMap _events;
    uint32 _otherDogData;
};

// 53694 - Riplimb
class npc_riplimb : public CreatureScript
{
    public:
        npc_riplimb() : CreatureScript("npc_riplimb") { }

        struct npc_riplimbAI : public npc_shannox_houndAI
        {
            npc_riplimbAI(Creature* creature) : npc_shannox_houndAI(creature, DATA_RAGEFACE),
                _fetchLeg(LEG_NONE), _collapsed(false) { }

            void Reset() override
            {
                npc_shannox_houndAI::Reset();
                _fetchLeg = LEG_NONE;
                _collapsed = false;
                _spearGUID.Clear();
            }

            void JustEngagedWith(Unit* who) override
            {
                npc_shannox_houndAI::JustEngagedWith(who);
                _events.ScheduleEvent(EVENT_LIMB_RIP, LIMB_RIP_TIMER);
            }

            void DamageTaken(Unit* /*attacker*/, uint32& damage) override
            {
                // Heroic: Riplimb cannot be permanently slain while his master lives.
                // At zero health he collapses for 30 seconds, then reanimates.
                if (me->GetMap()->IsHeroic() && damage >= me->GetHealth())
                {
                    Creature* shannox = _instance->GetCreature(DATA_SHANNOX);
                    if (shannox && shannox->IsAlive())
                    {
                        damage = me->GetHealth() - 1;
                        if (!_collapsed)
                            EnterCollapse();
                    }
                }
            }

            void DoAction(int32 action) override
            {
                switch (action)
                {
                    case ACTION_FETCH_SPEAR:
                    {
                        if (_collapsed || !me->IsAlive())
                            break;
                        Creature* shannox = _instance->GetCreature(DATA_SHANNOX);
                        if (!shannox || !shannox->IsAIEnabled())
                            break;
                        _spearGUID = shannox->AI()->GetGUID(NPC_SPEAR_OF_SHANNOX);
                        if (_spearGUID.IsEmpty())
                            break;
                        Creature* spear = ObjectAccessor::GetCreature(*me, _spearGUID);
                        if (!spear)
                            break;
                        _fetchLeg = LEG_TO_SPEAR;
                        me->AttackStop();
                        me->SetReactState(REACT_PASSIVE);
                        DoCastSelf(SPELL_DOGGED_DETERMINATION, true);
                        me->GetMotionMaster()->MovePoint(POINT_SPEAR, spear->GetPosition(), true);
                        break;
                    }
                    case ACTION_PRISON_BROKEN:
                        // Released from a Crystal Prison mid-fetch: resume the current leg
                        ResumeFetchLeg();
                        break;
                    case ACTION_FRENZIED_DEVOTION:
                        if (!_collapsed && me->IsAlive())
                            DoCastSelf(SPELL_FRENZIED_DEVOTION, true);
                        break;
                    default:
                        break;
                }
            }

            void MovementInform(uint32 type, uint32 id) override
            {
                if (type != POINT_MOTION_TYPE)
                    return;

                if (id == POINT_SPEAR && _fetchLeg == LEG_TO_SPEAR)
                {
                    // Grab the spear, then carry it back
                    _events.ScheduleEvent(EVENT_RETURN_SPEAR, 500);
                }
                else if (id == POINT_SHANNOX && _fetchLeg == LEG_TO_SHANNOX)
                    DeliverSpear();
            }

            void JustDied(Unit* /*killer*/) override
            {
                if (Creature* shannox = _instance->GetCreature(DATA_SHANNOX))
                    if (shannox->IsAlive() && shannox->IsAIEnabled())
                        shannox->AI()->DoAction(ACTION_RIPLIMB_DEFEATED);
            }

            uint32 GetData(uint32 type) const override
            {
                switch (type)
                {
                    case DATA_HOUND_COLLAPSED:
                        return _collapsed ? 1 : 0;
                    case DATA_RIPLIMB_FETCHING:
                        return _fetchLeg != LEG_NONE ? 1 : 0;
                    default:
                        return 0;
                }
            }

            void UpdateAI(uint32 diff) override
            {
                _events.Update(diff);

                if (_collapsed)
                {
                    while (uint32 eventId = _events.ExecuteEvent())
                        if (eventId == EVENT_RIPLIMB_REANIMATE)
                            Reanimate();
                    return;
                }

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_CHECK_SEPARATION:
                            CheckSeparationAnxiety();
                            _events.ScheduleEvent(EVENT_CHECK_SEPARATION, SEPARATION_CHECK_TIMER);
                            break;
                        case EVENT_RETURN_SPEAR:
                            _fetchLeg = LEG_TO_SHANNOX;
                            MoveTowardsShannox();
                            _events.ScheduleEvent(EVENT_UPDATE_RETURN_PATH, 1000);
                            break;
                        case EVENT_UPDATE_RETURN_PATH:
                            // Shannox moves - keep the return leg pointed at him
                            if (_fetchLeg == LEG_TO_SHANNOX)
                            {
                                if (Creature* shannox = _instance->GetCreature(DATA_SHANNOX))
                                {
                                    if (me->GetDistance(shannox) < 6.0f)
                                    {
                                        DeliverSpear();
                                        break;
                                    }
                                }
                                MoveTowardsShannox();
                                _events.ScheduleEvent(EVENT_UPDATE_RETURN_PATH, 1000);
                            }
                            break;
                        case EVENT_LIMB_RIP:
                            // Limb Rip applies Jagged Tear itself
                            if (_fetchLeg == LEG_NONE)
                                if (Unit* victim = me->GetVictim())
                                    DoCast(victim, SPELL_LIMB_RIP);
                            _events.ScheduleEvent(EVENT_LIMB_RIP, LIMB_RIP_TIMER);
                            break;
                        default:
                            break;
                    }
                }

                if (_fetchLeg != LEG_NONE)
                    return;

                if (!UpdateVictim())
                    return;

                DoMeleeAttackIfReady();
            }

        private:
            enum FetchLeg : uint8
            {
                LEG_NONE = 0,
                LEG_TO_SPEAR,
                LEG_TO_SHANNOX,
            };

            FetchLeg _fetchLeg;
            bool _collapsed;
            ObjectGuid _spearGUID;

            void MoveTowardsShannox()
            {
                if (Creature* shannox = _instance->GetCreature(DATA_SHANNOX))
                    me->GetMotionMaster()->MovePoint(POINT_SHANNOX, shannox->GetPosition(), true);
            }

            void ResumeFetchLeg()
            {
                switch (_fetchLeg)
                {
                    case LEG_TO_SPEAR:
                        if (Creature* spear = ObjectAccessor::GetCreature(*me, _spearGUID))
                            me->GetMotionMaster()->MovePoint(POINT_SPEAR, spear->GetPosition(), true);
                        else
                            AbortFetch(); // spear despawned while we were locked up
                        break;
                    case LEG_TO_SHANNOX:
                        MoveTowardsShannox();
                        break;
                    default:
                        break;
                }
            }

            void DeliverSpear()
            {
                if (_fetchLeg == LEG_NONE)
                    return;
                _fetchLeg = LEG_NONE;
                _spearGUID.Clear();
                _events.CancelEvent(EVENT_UPDATE_RETURN_PATH);
                me->RemoveAurasDueToSpell(SPELL_DOGGED_DETERMINATION);
                me->SetReactState(REACT_AGGRESSIVE);

                if (Creature* shannox = _instance->GetCreature(DATA_SHANNOX))
                {
                    if (shannox->IsAIEnabled())
                        shannox->AI()->DoAction(ACTION_SPEAR_RETRIEVED);
                    if (Unit* victim = shannox->GetVictim())
                        AttackStart(victim);
                }
            }

            void AbortFetch()
            {
                _fetchLeg = LEG_NONE;
                _spearGUID.Clear();
                _events.CancelEvent(EVENT_UPDATE_RETURN_PATH);
                me->RemoveAurasDueToSpell(SPELL_DOGGED_DETERMINATION);
                me->SetReactState(REACT_AGGRESSIVE);
            }

            void EnterCollapse()
            {
                _collapsed = true;
                AbortFetch();
                me->AttackStop();
                me->CastStop();
                me->SetReactState(REACT_PASSIVE);
                me->RemoveAurasDueToSpell(SPELL_FEEDING_FRENZY);
                me->RemoveAurasDueToSpell(SPELL_FEEDING_FRENZY_STACK);
                me->RemoveAurasDueToSpell(SPELL_SEPARATION_ANXIETY);
                me->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->SetImmuneToAll(true);
                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MoveIdle();
                _events.CancelEvent(EVENT_LIMB_RIP);
                _events.CancelEvent(EVENT_CHECK_SEPARATION);
                _events.CancelEvent(EVENT_RETURN_SPEAR);
                _events.CancelEvent(EVENT_UPDATE_RETURN_PATH);
                _events.ScheduleEvent(EVENT_RIPLIMB_REANIMATE, RIPLIMB_REANIMATE_TIMER);

                if (Creature* shannox = _instance->GetCreature(DATA_SHANNOX))
                    if (shannox->IsAlive() && shannox->IsAIEnabled())
                        shannox->AI()->DoAction(ACTION_RIPLIMB_DEFEATED);
            }

            void Reanimate()
            {
                _collapsed = false;
                me->SetFullHealth();
                me->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->SetImmuneToAll(false);
                me->SetReactState(REACT_AGGRESSIVE);
                if (me->GetMap()->IsHeroic())
                    DoCastSelf(SPELL_FEEDING_FRENZY, true);
                _events.ScheduleEvent(EVENT_LIMB_RIP, LIMB_RIP_TIMER);
                _events.ScheduleEvent(EVENT_CHECK_SEPARATION, SEPARATION_CHECK_TIMER);
                DoZoneInCombat();

                if (Creature* shannox = _instance->GetCreature(DATA_SHANNOX))
                    if (shannox->IsAlive() && shannox->IsAIEnabled())
                        shannox->AI()->DoAction(ACTION_RIPLIMB_REVIVED);
            }
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
            npc_ragefaceAI(Creature* creature) : npc_shannox_houndAI(creature, DATA_RIPLIMB),
                _faceRageActive(false) { }

            void Reset() override
            {
                npc_shannox_houndAI::Reset();
                _faceRageActive = false;
                _faceRageTargetGUID.Clear();
            }

            void JustEngagedWith(Unit* who) override
            {
                npc_shannox_houndAI::JustEngagedWith(who);

                _events.ScheduleEvent(EVENT_CHANGE_TARGET, 100);
                _events.ScheduleEvent(EVENT_FACE_RAGE, FACE_RAGE_TIMER);
            }

            void DamageTaken(Unit* attacker, uint32& damage) override
            {
                // Face Rage breaks on a single hit above the threshold (30k/45k by raid size)
                if (_faceRageActive && damage >= FaceRageBreakThreshold(me))
                    EndFaceRage(attacker);
            }

            void DoAction(int32 action) override
            {
                switch (action)
                {
                    case ACTION_FACE_RAGE_END:
                        // Victim aura removed: death, prison, expiry or break
                        EndFaceRage(nullptr);
                        break;
                    case ACTION_FRENZIED_DEVOTION:
                        if (me->IsAlive())
                            DoCastSelf(SPELL_FRENZIED_DEVOTION, true);
                        break;
                    default:
                        break;
                }
            }

            void JustDied(Unit* /*killer*/) override
            {
                if (_faceRageActive)
                    EndFaceRage(nullptr);

                if (Creature* shannox = _instance->GetCreature(DATA_SHANNOX))
                    if (shannox->IsAlive() && shannox->IsAIEnabled())
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
                            if (!_faceRageActive)
                                SelectNewRandomTarget();
                            _events.ScheduleEvent(EVENT_CHANGE_TARGET, urand(RAGEFACE_TARGET_CHANGE_MIN, RAGEFACE_TARGET_CHANGE_MAX));
                            break;
                        case EVENT_FACE_RAGE:
                            if (!_faceRageActive)
                                StartFaceRage();
                            _events.ScheduleEvent(EVENT_FACE_RAGE, FACE_RAGE_TIMER);
                            break;
                        default:
                            break;
                    }
                }

                // While mauling, the Face Rage aura does the damage - no autoattacks
                if (_faceRageActive)
                    return;

                if (!UpdateVictim())
                    return;

                DoMeleeAttackIfReady();
            }

        private:
            bool _faceRageActive;
            ObjectGuid _faceRageTargetGUID;

            bool IsTank(Player const* player) const
            {
                for (uint32 data : { DATA_SHANNOX, DATA_RIPLIMB })
                    if (Creature* creature = _instance->GetCreature(data))
                        if (creature->GetVictim() == player)
                            return true;
                return false;
            }

            Player* SelectRandomPlayer(bool excludeTanks, float minDist) const
            {
                std::vector<Player*> pool;
                Map::PlayerList const& players = me->GetMap()->GetPlayers();
                for (auto const& ref : players)
                {
                    Player* player = ref.GetSource();
                    if (!player || !player->IsAlive() || player->IsGameMaster())
                        continue;
                    if (excludeTanks && IsTank(player))
                        continue;
                    if (minDist > 0.0f && me->GetDistance(player) < minDist)
                        continue;
                    pool.push_back(player);
                }

                if (pool.empty())
                    return nullptr;

                return pool[urand(0, pool.size() - 1)];
            }

            void FixateOn(Unit* target)
            {
                me->GetThreatManager().AddThreat(target, 1.0f, nullptr, true, true);
                me->GetThreatManager().FixateTarget(target);
                AttackStart(target);
            }

            void SelectNewRandomTarget()
            {
                // Rageface darts between random non-tank targets
                Player* target = SelectRandomPlayer(true, 0.0f);
                if (!target)
                    target = SelectRandomPlayer(false, 0.0f);
                if (target)
                    FixateOn(target);
            }

            void StartFaceRage()
            {
                // Prefer a ranged, non-tank victim; fall back to anyone eligible
                Player* target = SelectRandomPlayer(true, 10.0f);
                if (!target)
                    target = SelectRandomPlayer(true, 0.0f);
                if (!target)
                    target = SelectRandomPlayer(false, 0.0f);
                if (!target)
                    return;

                _faceRageTargetGUID = target->GetGUID();
                _faceRageActive = true;

                me->GetThreatManager().ClearFixate();
                me->AttackStop();
                me->SetReactState(REACT_PASSIVE);

                // No Feeding Frenzy stacks while channeling Face Rage
                if (me->GetMap()->IsHeroic())
                    me->RemoveAurasDueToSpell(SPELL_FEEDING_FRENZY);

                me->CastSpell(target, SPELL_FACE_RAGE, false);           // leap + knockdown
                me->CastSpell(target, SPELL_FACE_RAGE_AURA, true);       // stun + ramping maul
                DoCastSelf(SPELL_FACE_RAGE_SELF, true);                  // all attacks against Rageface crit
            }

            void EndFaceRage(Unit* breaker)
            {
                if (!_faceRageActive)
                    return;
                _faceRageActive = false;

                if (Unit* victim = ObjectAccessor::GetUnit(*me, _faceRageTargetGUID))
                    victim->RemoveAurasDueToSpell(SPELL_FACE_RAGE_AURA);
                _faceRageTargetGUID.Clear();

                for (uint32 spellId : FaceRageSelfSpells)
                    me->RemoveAurasDueToSpell(spellId);

                me->SetReactState(REACT_AGGRESSIVE);

                if (me->GetMap()->IsHeroic())
                    DoCastSelf(SPELL_FEEDING_FRENZY, true);

                // Whoever broke the maul gets mauled next
                if (breaker && breaker->IsAlive())
                    FixateOn(breaker);
                else
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
                        // Start the landing sequence once thrown
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
                            _events.ScheduleEvent(EVENT_SIGNAL_RIPLIMB, 1000);
                            break;
                        case EVENT_SIGNAL_RIPLIMB:
                            if (Creature* riplimb = ObjectAccessor::GetCreature(*me, _riplimbGUID))
                                if (riplimb->IsAIEnabled())
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
                // Spear stuck in the ground
                DoCastSelf(SPELL_SPEAR_VISUAL);

                // Impact damage around the landing point (Shannox is the caster so
                // the hit is attributed and scaled correctly)
                if (Creature* shannox = ObjectAccessor::GetCreature(*me, _shannoxGUID))
                    shannox->CastSpell(me->GetPosition(), SPELL_HURL_SPEAR, true);

                // Raid-wide fire damage
                DoCastAOE(SPELL_MAGMA_FLARE);

                // Radiating molten eruptions
                SpawnEruptionCascade(me);
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

            void SetData(uint32 /*type*/, uint32 eruptDelay) override
            {
                _events.ScheduleEvent(EVENT_SPIRAL_FLAME_ERUPT, eruptDelay);
            }

            void UpdateAI(uint32 diff) override
            {
                _events.Update(diff);

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_SPIRAL_FLAME_ERUPT:
                            // Eruption missile - triggers the (difficulty chained) damage on impact
                            me->CastSpell(me->GetPosition(), SPELL_MAGMA_RUPTURE_VISUAL, true);
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

// Shared trap behavior: arm after 2 seconds, then trigger on contact by a player or a dog
struct npc_shannox_trapAI : public ScriptedAI
{
    npc_shannox_trapAI(Creature* creature) : ScriptedAI(creature), _instance(creature->GetInstanceScript()), _armed(false), _triggered(false)
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
        _events.ScheduleEvent(EVENT_CHECK_TRAP_TRIGGER, TRAP_ARM_DELAY);
        // Untriggered traps persist for a long while, independent of boss events
        me->DespawnOrUnsummon(TRAP_LIFETIME);
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
                    _armed = true;
                    if (Unit* victim = FindTrapVictim())
                    {
                        _triggered = true;
                        TriggerTrap(victim);
                        // Dogs that trip a trap become Wary of them for a while
                        if (victim->GetTypeId() == TYPEID_UNIT)
                            me->CastSpell(victim, SPELL_WARY, true);
                        me->DespawnOrUnsummon(500);
                        return;
                    }
                    _events.ScheduleEvent(EVENT_CHECK_TRAP_TRIGGER, 200);
                    break;
                default:
                    break;
            }
        }
    }

protected:
    virtual void TriggerTrap(Unit* victim) = 0;

    Unit* FindTrapVictim()
    {
        // Players first
        Map::PlayerList const& players = me->GetMap()->GetPlayers();
        for (auto const& ref : players)
        {
            Player* player = ref.GetSource();
            if (!player || !player->IsAlive() || player->IsGameMaster())
                continue;
            if (me->IsWithinDistInMap(player, static_cast<float>(TRAP_TRIGGER_RANGE)))
                return player;
        }

        // Then the dogs
        if (_instance)
        {
            for (uint32 data : { DATA_RIPLIMB, DATA_RAGEFACE })
            {
                Creature* dog = _instance->GetCreature(data);
                if (!dog || !dog->IsAlive() || !dog->IsAIEnabled())
                    continue;
                if (dog->AI()->GetData(DATA_HOUND_COLLAPSED))
                    continue;
                if (HasWary(dog) || dog->HasAura(SPELL_CRYSTAL_PRISON_EFFECT))
                    continue;
                if (me->IsWithinDistInMap(dog, static_cast<float>(TRAP_TRIGGER_RANGE)))
                    return dog;
            }
        }

        return nullptr;
    }

    InstanceScript* _instance;
    EventMap _events;
    bool _armed;
    bool _triggered;
};

// 53724 - Immolation Trap
class npc_immolation_trap : public CreatureScript
{
    public:
        npc_immolation_trap() : CreatureScript("npc_immolation_trap") { }

        struct npc_immolation_trapAI : public npc_shannox_trapAI
        {
            npc_immolation_trapAI(Creature* creature) : npc_shannox_trapAI(creature) { }

            void TriggerTrap(Unit* victim) override
            {
                // Fire hit + burn + debuff in a single (difficulty chained) spell
                me->CastSpell(victim, SPELL_IMMOLATION_TRAP_DAMAGE, true);
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

        struct npc_crystal_prison_trapAI : public npc_shannox_trapAI
        {
            npc_crystal_prison_trapAI(Creature* creature) : npc_shannox_trapAI(creature) { }

            void TriggerTrap(Unit* victim) override
            {
                if (TempSummon* prison = me->SummonCreature(NPC_CRYSTAL_PRISON, victim->GetPosition(), TEMPSUMMON_MANUAL_DESPAWN))
                    if (prison->IsAIEnabled())
                        prison->AI()->SetGUID(victim->GetGUID(), 0);
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetFirelandsAI<npc_crystal_prison_trapAI>(creature);
        }
};

// 53819 - Crystal Prison (encases players; dogs break free after 10 seconds)
class npc_crystal_prison : public CreatureScript
{
    public:
        npc_crystal_prison() : CreatureScript("npc_crystal_prison") { }

        struct npc_crystal_prisonAI : public ScriptedAI
        {
            npc_crystal_prisonAI(Creature* creature) : ScriptedAI(creature), _instance(creature->GetInstanceScript()), _released(false)
            {
                me->SetReactState(REACT_PASSIVE);
            }

            void Reset() override
            {
                _events.Reset();
                _prisonerGUID.Clear();
                _released = false;
            }

            void SetGUID(ObjectGuid const& guid, int32 /*id*/) override
            {
                _prisonerGUID = guid;

                Unit* prisoner = ObjectAccessor::GetUnit(*me, _prisonerGUID);
                if (!prisoner)
                {
                    me->DespawnOrUnsummon();
                    return;
                }

                // Encase: stun + immunity while inside
                me->CastSpell(prisoner, SPELL_CRYSTAL_PRISON_EFFECT, true);

                if (Creature* dog = prisoner->ToCreature())
                {
                    // An encased dog cannot be attacked and breaks free after 10 seconds
                    dog->SetImmuneToPC(true);
                    _events.ScheduleEvent(EVENT_PRISON_AUTO_BREAK, PRISON_DOG_AUTO_BREAK);
                }
                else if (prisoner->HasAura(SPELL_FACE_RAGE_AURA))
                {
                    // Encasing the Face Rage victim ends the maul
                    if (Creature* rageface = _instance->GetCreature(DATA_RAGEFACE))
                        if (rageface->IsAIEnabled())
                            rageface->AI()->DoAction(ACTION_FACE_RAGE_END);
                }

                _events.ScheduleEvent(EVENT_CHECK_PRISONER, 1000);
            }

            void JustDied(Unit* /*killer*/) override
            {
                ReleasePrisoner();
            }

            void UpdateAI(uint32 diff) override
            {
                _events.Update(diff);

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_PRISON_AUTO_BREAK:
                            me->KillSelf();
                            break;
                        case EVENT_CHECK_PRISONER:
                        {
                            // Free the prisoner and vanish if they died or the encounter ended
                            Unit* prisoner = ObjectAccessor::GetUnit(*me, _prisonerGUID);
                            if (!prisoner || !prisoner->IsAlive() || _instance->GetBossState(DATA_SHANNOX) != IN_PROGRESS)
                            {
                                ReleasePrisoner();
                                me->DespawnOrUnsummon();
                                return;
                            }
                            _events.ScheduleEvent(EVENT_CHECK_PRISONER, 1000);
                            break;
                        }
                        default:
                            break;
                    }
                }
            }

        private:
            InstanceScript* _instance;
            EventMap _events;
            ObjectGuid _prisonerGUID;
            bool _released;

            void ReleasePrisoner()
            {
                if (_released)
                    return;
                _released = true;

                if (Unit* prisoner = ObjectAccessor::GetUnit(*me, _prisonerGUID))
                {
                    prisoner->RemoveAurasDueToSpell(SPELL_CRYSTAL_PRISON_EFFECT);
                    if (Creature* dog = prisoner->ToCreature())
                    {
                        dog->SetImmuneToPC(false);
                        if (dog->IsAIEnabled())
                            dog->AI()->DoAction(ACTION_PRISON_BROKEN);
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetFirelandsAI<npc_crystal_prisonAI>(creature);
        }
};

// 99947 - Face Rage (victim aura): ramping maul damage + cleanup notification
class spell_shannox_face_rage : public AuraScript
{
    void HandlePeriodic(AuraEffect const* aurEff)
    {
        // Each tick hits harder than the last (8k -> 16k -> 24k ...)
        int32 base = GetSpellInfo()->Effects[EFFECT_1].CalcValue();
        const_cast<AuraEffect*>(aurEff)->SetAmount(aurEff->GetAmount() + base);
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        // Victim died, got encased, the stun expired or the maul was broken -
        // route every ending through Rageface's cleanup
        if (Unit* caster = GetCaster())
            if (Creature* rageface = caster->ToCreature())
                if (rageface->IsAIEnabled())
                    rageface->AI()->DoAction(ACTION_FACE_RAGE_END);
    }

    void Register() override
    {
        OnEffectPeriodic.Register(&spell_shannox_face_rage::HandlePeriodic, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE);
        AfterEffectRemove.Register(&spell_shannox_face_rage::OnRemove, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE, AURA_EFFECT_HANDLE_REAL);
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
    RegisterSpellScript(spell_shannox_face_rage);
}
