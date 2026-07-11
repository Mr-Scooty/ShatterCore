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

#include "dragon_soul.h"
#include "Containers.h"
#include "GameObject.h"
#include "InstanceScript.h"
#include "Map.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellScript.h"
#include "TaskScheduler.h"
#include "TemporarySummon.h"

namespace DragonSoul::Morchok
{
enum Texts
{
    // Morchok
    SAY_INTRO_1                 = 0,  // "No mortal shall turn me from my task." (out of combat siege RP)
    SAY_INTRO_2                 = 1,  // "Wyrmrest will fall. All will be dust."
    SAY_AGGRO                   = 2,  // "You seek to halt an avalanche. I will bury you."
    SAY_SUMMON_KOHCROM          = 3,  // "You thought to fight me alone? The earth splits to swallow and crush you."
    EMOTE_SUMMON_CRYSTAL        = 4,  // "%s summons a Resonating Crystal!"
    SAY_SUMMON_CRYSTAL          = 5,  // "Flee, and die." / "Run, and perish."
    SAY_BLACK_BLOOD_OMEN_A      = 6,  // "The rocks tremble..."
    SAY_BLACK_BLOOD_A           = 7,  // "...and the rage of the true gods follows."
    SAY_BLACK_BLOOD_OMEN_B      = 8,  // "The stone calls..."
    SAY_BLACK_BLOOD_B           = 9,  // "...and the black blood of the earth consumes you."
    SAY_BLACK_BLOOD_OMEN_C      = 10, // "The ground shakes..."
    SAY_BLACK_BLOOD_C           = 11, // "...and there is no escape from the Old Gods."
    SAY_BLACK_BLOOD_OMEN_D      = 12, // "The surface quakes..."
    SAY_BLACK_BLOOD_D           = 13, // "...and you drown in the hate of The Master."
    WHISPER_BLACK_BLOOD         = 14, // "$n! Get out of the black ooze on the ground!"
    SAY_DEATH                   = 15, // "Impossible. This cannot be. The tower... must... fall..."
    SAY_SLAY                    = 16
};

enum Spells
{
    // Morchok & Kohcrom
    SPELL_CRUSH_ARMOR                       = 103687,
    SPELL_STOMP                             = 103414, // forks: 108571 / 109033 / 109034
    SPELL_SUMMON_RESONATING_CRYSTAL         = 103640, // missile -> 103639 (summon 55346)
    SPELL_EARTHEN_VORTEX                    = 103821, // forks: 110047 / 110046 / 110045
    SPELL_FALLING_FRAGMENTS                 = 103176, // 500ms periodic (5s), trigger scripted -> 103177
    SPELL_FALLING_FRAGMENT_MISSILE          = 103177, // missile -> 103178 (damage, knockback, rock spike GO)
    SPELL_BLACK_BLOOD_OF_THE_EARTH          = 103851, // 15s channel, triggers 103785 each second
    SPELL_BLACK_BLOOD_DAMAGE                = 103785, // forks: 108570 / 110288 / 110287
    SPELL_FURIOUS                           = 103846,
    SPELL_SUMMON_KOHCROM                    = 109017,
    SPELL_BERSERK                           = 26662,

    // Resonating Crystal
    SPELL_RESONATING_CRYSTAL_AURA           = 103494,
    SPELL_TARGET_SELECTION_DANGER           = 103534, // red beam - far
    SPELL_TARGET_SELECTION_WARNING          = 103536, // yellow beam - mid
    SPELL_TARGET_SELECTION_SAFE             = 103541, // blue beam - close
    SPELL_RESONATING_CRYSTAL_DETONATE       = 103545, // forks: 108572 / 110041 / 110040
    SPELL_RESONATING_CRYSTAL_SELF_DESTRUCT  = 103673
};

enum Events
{
    // Morchok
    EVENT_CRUSH_ARMOR = 1,
    EVENT_STOMP,
    EVENT_SUMMON_RESONATING_CRYSTAL,
    EVENT_EARTHEN_VORTEX,
    EVENT_BLACK_BLOOD_OMEN,
    EVENT_BLACK_BLOOD,
    EVENT_BLACK_BLOOD_ENDED,
    EVENT_BERSERK,

    // Kohcrom
    EVENT_KOHCROM_STOMP,
    EVENT_KOHCROM_SUMMON_CRYSTAL
};

enum Actions
{
    ACTION_ECHO_STOMP = 1,
    ACTION_ECHO_CRYSTAL,
    ACTION_CAST_EARTHEN_VORTEX,
    ACTION_CAST_BLACK_BLOOD,
    ACTION_RESUME_COMBAT,
    ACTION_CAST_FURIOUS
};

enum MiscData
{
    DATA_BLACK_BLOOD_WHISPERED = 1
};

namespace
{
// Beam color / detonation tier distances (yards from the crystal)
constexpr float CrystalSafeDistance     = 10.0f;
constexpr float CrystalWarningDistance  = 25.0f;

// Black Blood wave expansion (yards) - starts at the boss and rolls outwards
constexpr float BlackBloodInitialRadius = 10.0f;
constexpr float BlackBloodGrowthPerSec  = 4.5f;

constexpr uint32 AchievementProximityRange = 5; // "Don't Stand So Close to Me"

// Unnerfed 4.3.4 encounter health. On heroic Morchok starts with the combined
// pool, then both max and current health are divided between the twins at 90%.
constexpr uint32 MorchokHealth10Normal  = 36'000'197;
constexpr uint32 MorchokHealth25Normal  = 102'000'000;
constexpr uint32 MorchokHealth10Heroic  = 42'946'000;
constexpr uint32 MorchokHealth25Heroic  = 180'404'194;
constexpr uint32 KohcromHealth10Heroic  = 21'473'000;
constexpr uint32 KohcromHealth25Heroic  = 90'202'097;

Position const KohcromSplitPosition = { -2016.3400f, -2391.2900f, 70.8304f, 5.8110f };

// Rock spike impact points captured from retail sniffs (ring around the arena)
Position const RockSpikePositions[] =
{
    { -1960.7151f, -2379.5444f, 67.7530f, 0.7854f },
    { -1960.7151f, -2399.5444f, 68.5175f, 0.3218f },
    { -1961.4730f, -2376.9983f, 67.3822f, 0.7854f },
    { -1961.4730f, -2396.9983f, 68.4750f, 0.3218f },
    { -1965.7151f, -2394.5444f, 68.4248f, 0.5404f },
    { -1966.4730f, -2391.9983f, 68.3136f, 0.5404f },
    { -1966.4730f, -2411.9983f, 68.6694f, 6.0858f },
    { -1966.4730f, -2416.9983f, 68.6792f, 5.9027f },
    { -1967.9128f, -2378.0002f, 67.5101f, 0.8761f },
    { -1967.9128f, -2393.0002f, 68.3922f, 0.5404f },
    { -1967.9128f, -2398.0002f, 68.5284f, 0.3805f },
    { -1967.9128f, -2418.0002f, 68.7268f, 5.9027f },
    { -1970.7151f, -2429.5444f, 68.1958f, 5.4978f },
    { -1975.7151f, -2429.5444f, 68.6211f, 5.3559f },
    { -1980.7151f, -2379.5444f, 68.7302f, 1.2490f },
    { -1985.7151f, -2424.5444f, 69.2627f, 5.0341f },
    { -1986.4730f, -2431.9983f, 68.6004f, 4.9098f },
    { -1987.9128f, -2368.0002f, 67.5038f, 1.4464f },
    { -1987.9128f, -2433.0002f, 68.4862f, 4.9098f },
    { -1990.7151f, -2434.5444f, 68.0691f, 4.7124f },
    { -1991.4730f, -2436.9983f, 67.1407f, 4.7124f },
    { -1992.9128f, -2368.0002f, 67.3751f, 1.5708f },
    { -1992.9128f, -2438.0002f, 67.0255f, 4.7124f },
    { -2000.7151f, -2434.5444f, 68.3597f, 4.3319f },
    { -2001.4730f, -2371.9983f, 67.6945f, 1.8491f },
    { -2001.4730f, -2431.9983f, 68.9373f, 4.3319f },
    { -2006.4730f, -2376.9983f, 68.1844f, 2.0344f },
    { -2006.4730f, -2431.9983f, 68.8950f, 4.1720f },
    { -2010.7151f, -2379.5444f, 68.7920f, 2.1588f },
    { -2012.9128f, -2378.0002f, 69.2347f, 2.1588f },
    { -2015.7151f, -2419.5444f, 70.1696f, 3.5221f },
    { -2017.9128f, -2433.0002f, 69.1625f, 3.9270f },
    { -2020.7151f, -2414.5444f, 70.5795f, 3.3067f },
    { -2020.7151f, -2429.5444f, 70.5734f, 3.7296f },
    { -2021.4730f, -2371.9983f, 71.2609f, 2.2794f },
    { -2021.4730f, -2391.9983f, 71.2266f, 2.6779f },
    { -2021.4730f, -2431.9983f, 70.6890f, 3.8363f },
    { -2022.9128f, -2413.0002f, 70.9301f, 3.3067f },
    { -2022.9128f, -2418.0002f, 70.6520f, 3.4633f },
    { -2027.9128f, -2428.0002f, 71.0568f, 3.6607f },
    { -2030.7151f, -2394.5444f, 71.9646f, 2.7828f },
    { -2031.4730f, -2406.9983f, 72.2316f, 3.1416f },
    { -2031.4730f, -2411.9983f, 71.4964f, 3.2660f },
    { -2032.9128f, -2393.0002f, 72.0993f, 2.7828f }
};

bool IsHeroicMorchok(Map const* map)
{
    return map->GetDifficulty() == RAID_DIFFICULTY_10MAN_HEROIC
        || map->GetDifficulty() == RAID_DIFFICULTY_25MAN_HEROIC;
}

Milliseconds GetKohcromEchoDelay(Map const* map)
{
    return map->GetDifficulty() == RAID_DIFFICULTY_25MAN_HEROIC ? 5s : 6s;
}

// Both bosses cast Earthen Vortex and Black Blood of the Earth simultaneously.
// Players are always handled by whichever boss is closest to them so the two
// raid groups get pulled to - and hide from - their own boss.
Unit* GetResponsibleBoss(Unit* caster, WorldObject const* target)
{
    InstanceScript* instance = caster->GetInstanceScript();
    if (!instance)
        return caster;

    Creature* morchok = instance->GetCreature(DATA_MORCHOK);
    Creature* other = instance->GetCreature(caster == morchok ? DATA_KOHCROM : DATA_MORCHOK);
    if (!other || !other->IsAlive() || !other->IsInCombat())
        return caster;

    return target->GetExactDist2d(caster) <= target->GetExactDist2d(other) ? caster : other;
}

uint32 GetMorchokHealth(Map const* map, InstanceScript const* instance)
{
    if (instance && instance->IsLFR())
        return MORCHOK_LFR_HEALTH;

    switch (map->GetDifficulty())
    {
        case RAID_DIFFICULTY_25MAN_NORMAL:
            return MorchokHealth25Normal;
        case RAID_DIFFICULTY_10MAN_HEROIC:
            return MorchokHealth10Heroic;
        case RAID_DIFFICULTY_25MAN_HEROIC:
            return MorchokHealth25Heroic;
        default:
            return MorchokHealth10Normal;
    }
}

uint32 GetKohcromHealth(Map const* map)
{
    return map->GetDifficulty() == RAID_DIFFICULTY_25MAN_HEROIC
        ? KohcromHealth25Heroic : KohcromHealth10Heroic;
}

// creature_template uses a float health multiplier, which cannot represent all
// of the retail integer values exactly. Set the encounter value explicitly.
void ApplyMorchokHealth(Creature* creature, InstanceScript const* instance)
{
    creature->SetMaxHealth(GetMorchokHealth(creature->GetMap(), instance));
    creature->SetFullHealth();
}

void ApplyLFRDamageReduction(InstanceScript const* instance, uint32& damage)
{
    if (instance && instance->IsLFR())
        damage = damage * LFR_DAMAGE_PCT / 100;
}

void SummonResonatingCrystal(Creature* caster)
{
    Unit* target = caster->AI()->SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true, false);
    if (!target)
        target = caster->AI()->SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true);
    if (!target)
        return;

    caster->CastSpell(target->GetPosition(), SPELL_SUMMON_RESONATING_CRYSTAL, true);
}

// On heroic, damage is divided across the two displayed health bars. Their
// combined remaining health is authoritative, so odd damage is not lost.
void ShareHeroicTwinDamage(Creature* creature, InstanceScript* instance, uint32& damage)
{
    if (!damage || !instance || !IsHeroicMorchok(creature->GetMap()))
        return;

    uint32 const twinData = creature == instance->GetCreature(DATA_MORCHOK) ? DATA_KOHCROM : DATA_MORCHOK;
    Creature* twin = instance->GetCreature(twinData);
    if (!twin || !twin->IsAlive() || !twin->IsInCombat())
        return;

    uint64 const combinedHealth = uint64(creature->GetHealth()) + twin->GetHealth();
    uint64 const remainingHealth = damage < combinedHealth ? combinedHealth - damage : 0;

    uint32 const creatureHealthAfter = uint32(remainingHealth / 2);
    uint32 twinHealthAfter = uint32(remainingHealth - creatureHealthAfter);

    // With one combined point left, finish the pair through the normal death
    // path so Morchok still owns encounter completion and loot generation.
    if (!creatureHealthAfter)
    {
        twin->SetHealth(1);
        damage = creature->GetHealth();
        return;
    }

    twinHealthAfter = std::min(twinHealthAfter, twin->GetMaxHealth());
    twin->SetHealth(twinHealthAfter);
    damage = creature->GetHealth() - creatureHealthAfter;

    if (twin->HealthBelowPct(20) && twin->IsAIEnabled())
        twin->AI()->DoAction(ACTION_CAST_FURIOUS);
}
}

struct boss_morchok : public BossAI
{
    boss_morchok(Creature* creature) : BossAI(creature, DATA_MORCHOK),
        _kohcromSummoned(false), _furious(false), _stompCount(0), _crystalCount(0), _firstCycle(true),
        _kohcromSkipAction(ACTION_ECHO_STOMP), _kohcromSpawnHealth(0), _blackBloodWhispered(false) { }

    void Reset() override
    {
        _Reset();
        _kohcromSummoned = false;
        _furious = false;
        _stompCount = 0;
        _crystalCount = 0;
        _firstCycle = true;
        _kohcromSkipAction = ACTION_ECHO_STOMP;
        _kohcromSpawnHealth = 0;
        _blackBloodWhispered = false;
        _transitionActive = false;
        _scheduler.CancelAll();
        me->SetReactState(REACT_AGGRESSIVE);
        ApplyMorchokHealth(me, instance);
    }

    void DamageDealt(Unit* /*victim*/, uint32& damage, DamageEffectType /*damageType*/) override
    {
        ApplyLFRDamageReduction(instance, damage);
    }

    void JustEngagedWith(Unit* who) override
    {
        // LFR is detected when the first queued player enters, after creatures
        // may already have reset with their normal difficulty statistics.
        ApplyMorchokHealth(me, instance);
        BossAI::JustEngagedWith(who);
        Talk(SAY_AGGRO);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

        if (!IsHeroicMorchok(me->GetMap()))
            events.ScheduleEvent(EVENT_CRUSH_ARMOR, 6s);
        events.ScheduleEvent(EVENT_STOMP, 12s);
        events.ScheduleEvent(EVENT_SUMMON_RESONATING_CRYSTAL, 19s);
        events.ScheduleEvent(EVENT_EARTHEN_VORTEX, 56s);
        if (IsHeroicMorchok(me->GetMap()))
            events.ScheduleEvent(EVENT_BERSERK, 7min);
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage) override
    {
        ShareHeroicTwinDamage(me, instance, damage);

        if (!_kohcromSummoned && IsHeroicMorchok(me->GetMap()) && me->HealthBelowPctDamaged(90, damage))
        {
            _kohcromSummoned = true;

            // The pre-split bar is the combined encounter pool. Divide both
            // that pool and this threshold-crossing hit between the new bars.
            uint32 const combinedHealth = me->GetHealth();
            uint32 const combinedHealthAfter = combinedHealth > damage ? combinedHealth - damage : 1;
            uint32 const morchokHealthBefore = (combinedHealth + 1) / 2;
            uint32 const morchokHealthAfter = combinedHealthAfter / 2;

            me->SetMaxHealth(GetKohcromHealth(me->GetMap()));
            me->SetHealth(morchokHealthBefore);
            _kohcromSpawnHealth = combinedHealthAfter - morchokHealthAfter;
            damage = morchokHealthBefore - morchokHealthAfter;

            Talk(SAY_SUMMON_KOHCROM);
            DoCastSelf(SPELL_SUMMON_KOHCROM, true);

            // Retail restarts both ability timers at the split. Whichever
            // ability would have come next is the first one Kohcrom skips.
            if (!_transitionActive)
            {
                if (_kohcromSkipAction == ACTION_ECHO_CRYSTAL)
                {
                    events.RescheduleEvent(EVENT_SUMMON_RESONATING_CRYSTAL, 5500ms);
                    events.RescheduleEvent(EVENT_STOMP, 12s);
                }
                else
                {
                    events.RescheduleEvent(EVENT_STOMP, 6s);
                    events.RescheduleEvent(EVENT_SUMMON_RESONATING_CRYSTAL, 15s);
                }
            }
        }

        if (!_furious && me->HealthBelowPctDamaged(20, damage))
        {
            _furious = true;
            DoCastSelf(SPELL_FURIOUS, true);
        }
    }

    void DoAction(int32 action) override
    {
        if (action == ACTION_CAST_FURIOUS && !_furious)
        {
            _furious = true;
            DoCastSelf(SPELL_FURIOUS, true);
        }
    }

    void JustSummoned(Creature* summon) override
    {
        summons.Summon(summon);

        if (summon->GetEntry() == NPC_KOHCROM)
        {
            // Both heroic bars use the supplied per-creature value and begin
            // after the hit which crossed 90%, not at the pre-damage value
            // exposed while DamageTaken is running.
            summon->SetMaxHealth(me->GetMaxHealth());
            summon->SetHealth(_kohcromSpawnHealth ? _kohcromSpawnHealth : me->GetHealth());
            _kohcromSpawnHealth = 0;
        }
    }

    void KilledUnit(Unit* victim) override
    {
        if (victim->GetTypeId() == TYPEID_PLAYER)
            Talk(SAY_SLAY);
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
        Talk(SAY_DEATH);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

        if (Creature* kohcrom = instance->GetCreature(DATA_KOHCROM))
            if (kohcrom->IsAlive())
                kohcrom->KillSelf();

        CleanupEncounter();
    }

    void EnterEvadeMode(EvadeReason /*why*/) override
    {
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        CleanupEncounter();
        _EnterEvadeMode();
        summons.DespawnAll();
        me->GetMotionMaster()->MoveTargetedHome();
        _DespawnAtEvade();
    }

    uint32 GetData(uint32 type) const override
    {
        if (type == DATA_BLACK_BLOOD_WHISPERED)
            return _blackBloodWhispered ? 1 : 0;
        return 0;
    }

    void SetData(uint32 type, uint32 data) override
    {
        if (type == DATA_BLACK_BLOOD_WHISPERED)
            _blackBloodWhispered = data != 0;
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);

        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_CRUSH_ARMOR:
                    DoCastVictim(SPELL_CRUSH_ARMOR);
                    events.Repeat(6s, 7s);
                    break;
                case EVENT_STOMP:
                    DoCastAOE(SPELL_STOMP);
                    NotifyKohcrom(ACTION_ECHO_STOMP);
                    if (++_stompCount < (_firstCycle ? 3 : 4))
                        events.Repeat(12s, 14s);
                    break;
                case EVENT_SUMMON_RESONATING_CRYSTAL:
                    Talk(EMOTE_SUMMON_CRYSTAL);
                    Talk(SAY_SUMMON_CRYSTAL);
                    SummonResonatingCrystal(me);
                    NotifyKohcrom(ACTION_ECHO_CRYSTAL);
                    if (++_crystalCount < (_firstCycle ? 2 : 3))
                        events.Repeat(12s, 15s);
                    break;
                case EVENT_EARTHEN_VORTEX:
                {
                    _transitionActive = true;
                    events.CancelEvent(EVENT_CRUSH_ARMOR);
                    events.CancelEvent(EVENT_STOMP);
                    events.CancelEvent(EVENT_SUMMON_RESONATING_CRYSTAL);
                    me->AttackStop();
                    me->SetReactState(REACT_PASSIVE);

                    _blackBloodTextVariant = urand(0, 3);
                    Talk(SAY_BLACK_BLOOD_OMEN_A + _blackBloodTextVariant * 2);

                    DoCastAOE(SPELL_EARTHEN_VORTEX);
                    DoCastSelf(SPELL_FALLING_FRAGMENTS, true);
                    NotifyKohcrom(ACTION_CAST_EARTHEN_VORTEX);

                    events.ScheduleEvent(EVENT_BLACK_BLOOD_OMEN, 5s);
                    events.ScheduleEvent(EVENT_BLACK_BLOOD, 5s);
                    break;
                }
                case EVENT_BLACK_BLOOD_OMEN:
                    Talk(SAY_BLACK_BLOOD_A + _blackBloodTextVariant * 2);
                    break;
                case EVENT_BLACK_BLOOD:
                    _blackBloodWhispered = false;
                    DoCastSelf(SPELL_BLACK_BLOOD_OF_THE_EARTH);
                    NotifyKohcrom(ACTION_CAST_BLACK_BLOOD);
                    StartAchievementProximityChecks();
                    events.ScheduleEvent(EVENT_BLACK_BLOOD_ENDED, 17s);
                    break;
                case EVENT_BLACK_BLOOD_ENDED:
                    _transitionActive = false;
                    _scheduler.CancelAll();
                    DespawnRockSpikes();
                    me->SetReactState(REACT_AGGRESSIVE);
                    if (Unit* victim = me->GetVictim())
                        AttackStart(victim);
                    NotifyKohcrom(ACTION_RESUME_COMBAT);

                    _firstCycle = false;
                    _stompCount = 0;
                    _crystalCount = 0;
                    _kohcromSkipAction = ACTION_ECHO_CRYSTAL;
                    if (!IsHeroicMorchok(me->GetMap()))
                        events.ScheduleEvent(EVENT_CRUSH_ARMOR, 6s);
                    events.ScheduleEvent(EVENT_STOMP, 19s);
                    events.ScheduleEvent(EVENT_SUMMON_RESONATING_CRYSTAL, 26s);
                    events.ScheduleEvent(EVENT_EARTHEN_VORTEX, 74s);
                    break;
                case EVENT_BERSERK:
                    DoCastSelf(SPELL_BERSERK, true);
                    if (Creature* kohcrom = instance->GetCreature(DATA_KOHCROM))
                        kohcrom->CastSpell(kohcrom, SPELL_BERSERK, true);
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
    void NotifyKohcrom(int32 action)
    {
        if (!_kohcromSummoned)
        {
            // Before the split this tracks which of the alternating abilities
            // is due next, matching DBM's retail kohcromSkip state machine.
            if (action == ACTION_ECHO_STOMP)
                _kohcromSkipAction = ACTION_ECHO_CRYSTAL;
            else if (action == ACTION_ECHO_CRYSTAL)
                _kohcromSkipAction = ACTION_ECHO_STOMP;
            return;
        }

        if ((action == ACTION_ECHO_STOMP || action == ACTION_ECHO_CRYSTAL) && action == _kohcromSkipAction)
        {
            _kohcromSkipAction = 0;
            return;
        }

        if (Creature* kohcrom = instance->GetCreature(DATA_KOHCROM))
            if (kohcrom->IsAlive() && kohcrom->IsAIEnabled())
                kohcrom->AI()->DoAction(action);
    }

    void StartAchievementProximityChecks()
    {
        // Don't Stand So Close to Me: no two (10 player) / three (25 player) players
        // may ever be within 5 yards of each other while Black Blood is channeled.
        uint8 const clusterLimit = me->GetMap()->Is25ManRaid() ? 3 : 2;
        // The spell has a two-second cast before the 15-second channel begins.
        _scheduler.Schedule(2s, [this, clusterLimit](TaskContext context)
        {
            std::vector<Player*> players;
            for (MapReference const& ref : me->GetMap()->GetPlayers())
                if (Player* player = ref.GetSource())
                    if (player->IsAlive() && !player->IsGameMaster())
                        players.push_back(player);

            for (Player* player : players)
            {
                uint8 clusterSize = 0;
                for (Player* other : players)
                    if (player->GetExactDist2d(other) <= float(AchievementProximityRange))
                        ++clusterSize;

                if (clusterSize >= clusterLimit)
                {
                    instance->SetData(DATA_MORCHOK_ACHIEVEMENT_FAILED, 1);
                    return;
                }
            }

            context.Repeat(1s);
        });
    }

    void DespawnRockSpikes()
    {
        std::list<GameObject*> spikes;
        me->GetGameObjectListWithEntryInGrid(spikes, GO_MORCHOK_ROCK_SPIKE, 250.0f);
        for (GameObject* spike : spikes)
            spike->DespawnOrUnsummon(2s);
    }

    void CleanupEncounter()
    {
        _scheduler.CancelAll();
        DespawnRockSpikes();

        std::list<Creature*> crystals;
        me->GetCreatureListWithEntryInGrid(crystals, NPC_RESONATING_CRYSTAL, 250.0f);
        for (Creature* crystal : crystals)
            crystal->DespawnOrUnsummon();

        for (uint32 spellId : { SPELL_TARGET_SELECTION_DANGER, SPELL_TARGET_SELECTION_WARNING, SPELL_TARGET_SELECTION_SAFE })
            instance->DoRemoveAurasDueToSpellOnPlayers(spellId);
    }

    TaskScheduler _scheduler;
    bool _kohcromSummoned;
    bool _furious;
    uint8 _stompCount;
    uint8 _crystalCount;
    bool _firstCycle;
    int32 _kohcromSkipAction;
    uint32 _kohcromSpawnHealth;
    uint8 _blackBloodTextVariant = 0;
    bool _blackBloodWhispered;
    bool _transitionActive = false;
};

struct npc_kohcrom : public ScriptedAI
{
    npc_kohcrom(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript()), _summons(creature), _furious(false), _frozen(false) { }

    void JustAppeared() override
    {
        me->SetReactState(REACT_PASSIVE);
        DoZoneInCombat();
        if (_instance)
            _instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

        // Sniffed split jump: ~39.5 yards in 794 ms (50 yd/s, 25 yd/s Z).
        me->GetMotionMaster()->MoveJump(KohcromSplitPosition, 50.0f, 25.0f);
        _scheduler.Schedule(7s, [this](TaskContext /*context*/)
        {
            if (!_frozen)
            {
                me->SetReactState(REACT_AGGRESSIVE);
                if (Unit* victim = me->SelectVictim())
                    AttackStart(victim);
            }
        });
    }

    void DoAction(int32 action) override
    {
        Milliseconds delay = GetKohcromEchoDelay(me->GetMap());
        switch (action)
        {
            case ACTION_ECHO_STOMP:
                if (!_frozen)
                    _events.ScheduleEvent(EVENT_KOHCROM_STOMP, delay);
                break;
            case ACTION_ECHO_CRYSTAL:
                if (!_frozen)
                    _events.ScheduleEvent(EVENT_KOHCROM_SUMMON_CRYSTAL, delay);
                break;
            case ACTION_CAST_EARTHEN_VORTEX:
                _frozen = true;
                _events.CancelEvent(EVENT_KOHCROM_STOMP);
                _events.CancelEvent(EVENT_KOHCROM_SUMMON_CRYSTAL);
                me->AttackStop();
                me->SetReactState(REACT_PASSIVE);
                DoCastAOE(SPELL_EARTHEN_VORTEX);
                DoCastSelf(SPELL_FALLING_FRAGMENTS, true);
                break;
            case ACTION_CAST_BLACK_BLOOD:
                DoCastSelf(SPELL_BLACK_BLOOD_OF_THE_EARTH);
                break;
            case ACTION_RESUME_COMBAT:
                if (!_frozen)
                    break;
                _frozen = false;
                me->SetReactState(REACT_AGGRESSIVE);
                if (Unit* victim = me->GetVictim())
                    AttackStart(victim);
                break;
            case ACTION_CAST_FURIOUS:
                if (!_furious)
                {
                    _furious = true;
                    DoCastSelf(SPELL_FURIOUS, true);
                }
                break;
            default:
                break;
        }
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage) override
    {
        ShareHeroicTwinDamage(me, _instance, damage);

        if (!_furious && me->HealthBelowPctDamaged(20, damage))
        {
            _furious = true;
            DoCastSelf(SPELL_FURIOUS, true);
        }
    }

    void JustSummoned(Creature* summon) override
    {
        _summons.Summon(summon);
    }

    void JustDied(Unit* /*killer*/) override
    {
        _summons.DespawnAll();
        if (_instance)
        {
            _instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            if (Creature* morchok = _instance->GetCreature(DATA_MORCHOK))
                if (morchok->IsAlive())
                    morchok->KillSelf();
        }
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);

        if (!UpdateVictim())
            return;

        _events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_KOHCROM_STOMP:
                    DoCastAOE(SPELL_STOMP);
                    break;
                case EVENT_KOHCROM_SUMMON_CRYSTAL:
                    SummonResonatingCrystal(me);
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
    InstanceScript* _instance;
    EventMap _events;
    SummonList _summons;
    TaskScheduler _scheduler;
    bool _furious;
    bool _frozen;
};

struct npc_morchok_resonating_crystal : public ScriptedAI
{
    npc_morchok_resonating_crystal(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript())
    {
        SetCombatMovement(false);
        me->SetReactState(REACT_PASSIVE);
    }

    void DamageDealt(Unit* /*victim*/, uint32& damage, DamageEffectType /*damageType*/) override
    {
        ApplyLFRDamageReduction(_instance, damage);
    }

    void JustAppeared() override
    {
        me->SetControlled(true, UNIT_STATE_ROOT);
        DoCastSelf(SPELL_RESONATING_CRYSTAL_AURA, true);
        SelectBeamTargets();

        _scheduler.Schedule(1s, [this](TaskContext context)
        {
            SelectBeamTargets();
            context.Repeat(1s);
        });

        _scheduler.Schedule(12s, [this](TaskContext /*context*/)
        {
            Detonate();
        });
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

private:
    void SelectBeamTargets()
    {
        uint8 const count = me->GetMap()->Is25ManRaid() ? 7 : 3;

        std::vector<Player*> players;
        for (MapReference const& ref : me->GetMap()->GetPlayers())
            if (Player* player = ref.GetSource())
                if (player->IsAlive() && !player->IsGameMaster() && me->IsWithinDist(player, 200.0f))
                    players.push_back(player);

        std::sort(players.begin(), players.end(), [this](Player const* left, Player const* right)
        {
            return me->GetExactDist2d(left) < me->GetExactDist2d(right);
        });

        if (players.size() > count)
            players.resize(count);

        std::vector<ObjectGuid> selectedPlayers;
        for (Player* player : players)
            selectedPlayers.push_back(player->GetGUID());

        for (ObjectGuid guid : _linkedPlayers)
            if (std::find(selectedPlayers.begin(), selectedPlayers.end(), guid) == selectedPlayers.end())
                if (Player* player = ObjectAccessor::GetPlayer(*me, guid))
                    RemoveBeams(player);

        _linkedPlayers = std::move(selectedPlayers);
        for (Player* player : players)
            ApplyBeam(player);
    }

    void ApplyBeam(Player* player)
    {
        float const dist = me->GetExactDist2d(player);
        uint32 const beam = dist < CrystalSafeDistance ? SPELL_TARGET_SELECTION_SAFE
            : dist < CrystalWarningDistance ? SPELL_TARGET_SELECTION_WARNING
            : SPELL_TARGET_SELECTION_DANGER;

        for (uint32 spellId : { SPELL_TARGET_SELECTION_SAFE, SPELL_TARGET_SELECTION_WARNING, SPELL_TARGET_SELECTION_DANGER })
            if (spellId != beam)
                player->RemoveAurasDueToSpell(spellId, me->GetGUID());

        if (!player->HasAura(beam, me->GetGUID()))
            me->CastSpell(player, beam, true);
    }

    void RemoveBeams(Player* player)
    {
        for (uint32 spellId : { SPELL_TARGET_SELECTION_SAFE, SPELL_TARGET_SELECTION_WARNING, SPELL_TARGET_SELECTION_DANGER })
            player->RemoveAurasDueToSpell(spellId, me->GetGUID());
    }

    void Detonate()
    {
        // Movement during the final second can change the closest linked set.
        SelectBeamTargets();
        DoCastSelf(SPELL_RESONATING_CRYSTAL_SELF_DESTRUCT, true);
        DoCastAOE(SPELL_RESONATING_CRYSTAL_DETONATE);

        for (ObjectGuid guid : _linkedPlayers)
            if (Player* player = ObjectAccessor::GetPlayer(*me, guid))
                RemoveBeams(player);

        _linkedPlayers.clear();
        me->DespawnOrUnsummon(2s);
    }

    InstanceScript* _instance;
    TaskScheduler _scheduler;
    std::vector<ObjectGuid> _linkedPlayers;
};

// 103414, 108571, 109033, 109034 - Stomp
class spell_morchok_stomp : public SpellScript
{
    void FilterTargets(std::list<WorldObject*>& targets)
    {
        _soakerGuids.clear();

        targets.remove_if([](WorldObject* target)
        {
            Player* player = target->ToPlayer();
            return !player || !player->IsAlive() || player->IsGameMaster();
        });
        _targetCount = uint32(targets.size());

        if (targets.empty())
            return;

        // The current target and the player closest to that target take a
        // double share in every raid size. Pets never enter the split pool.
        Player* primarySoaker = nullptr;
        if (Unit* victim = GetCaster()->GetVictim())
            primarySoaker = victim->ToPlayer();

        if (primarySoaker)
        {
            if (std::find_if(targets.begin(), targets.end(), [primarySoaker](WorldObject const* target)
                { return target->GetGUID() == primarySoaker->GetGUID(); }) != targets.end())
                _soakerGuids.push_back(primarySoaker->GetGUID());
            else
                primarySoaker = nullptr;
        }

        std::list<WorldObject*> sorted = targets;
        WorldObject* distanceOrigin = primarySoaker ? static_cast<WorldObject*>(primarySoaker) : GetCaster();
        sorted.sort([distanceOrigin](WorldObject* left, WorldObject* right)
        {
            return distanceOrigin->GetExactDist2d(left) < distanceOrigin->GetExactDist2d(right);
        });

        for (WorldObject* target : sorted)
        {
            if (_soakerGuids.size() >= 2)
                break;
            if (std::find(_soakerGuids.begin(), _soakerGuids.end(), target->GetGUID()) == _soakerGuids.end())
                _soakerGuids.push_back(target->GetGUID());
        }
    }

    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        if (!_targetCount)
            return;

        uint32 const soakers = uint32(_soakerGuids.size());
        int32 const share = GetEffectValue() / int32(_targetCount + soakers);

        bool const isSoaker = std::find(_soakerGuids.begin(), _soakerGuids.end(), GetHitUnit()->GetGUID()) != _soakerGuids.end();
        SetHitDamage(isSoaker ? share * 2 : share);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect.Register(&spell_morchok_stomp::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
        OnEffectHitTarget.Register(&spell_morchok_stomp::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }

    uint32 _targetCount = 0;
    std::vector<ObjectGuid> _soakerGuids;
};

// 103545, 108572, 110041, 110040 - Resonating Crystal (detonation)
class spell_morchok_resonating_crystal_detonate : public SpellScript
{
    bool HasBeamFromCaster(WorldObject const* target) const
    {
        Unit const* unit = target->ToUnit();
        if (!unit)
            return false;

        for (uint32 spellId : { SPELL_TARGET_SELECTION_SAFE, SPELL_TARGET_SELECTION_WARNING, SPELL_TARGET_SELECTION_DANGER })
            if (unit->HasAura(spellId, GetCaster()->GetGUID()))
                return true;

        return false;
    }

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        targets.remove_if([this](WorldObject* target)
        {
            return !HasBeamFromCaster(target);
        });
        _targetCount = uint32(targets.size());
    }

    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        Unit* target = GetHitUnit();
        if (!_targetCount)
            return;

        int32 multiplier = 3;
        if (target->HasAura(SPELL_TARGET_SELECTION_SAFE, GetCaster()->GetGUID()))
            multiplier = 1;
        else if (target->HasAura(SPELL_TARGET_SELECTION_WARNING, GetCaster()->GetGUID()))
            multiplier = 2;

        // The DBC value is the raid-size damage pool. Safe, warning and danger
        // links take one, two and three shares respectively.
        SetHitDamage(GetHitDamage() / int32(_targetCount) * multiplier);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect.Register(&spell_morchok_resonating_crystal_detonate::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
        OnObjectAreaTargetSelect.Register(&spell_morchok_resonating_crystal_detonate::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ENEMY);
        OnObjectAreaTargetSelect.Register(&spell_morchok_resonating_crystal_detonate::FilterTargets, EFFECT_2, TARGET_UNIT_DEST_AREA_ENEMY);
        OnEffectHitTarget.Register(&spell_morchok_resonating_crystal_detonate::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }

    uint32 _targetCount = 0;
};

// 103821, 110047, 110046, 110045 - Earthen Vortex
class spell_morchok_earthen_vortex : public SpellScript
{
    void FilterTargets(std::list<WorldObject*>& targets)
    {
        Unit* caster = GetCaster();
        targets.remove_if([caster](WorldObject* target)
        {
            return !target->ToPlayer() || GetResponsibleBoss(caster, target) != caster;
        });
    }

    void HandleForceCast(SpellEffIndex effIndex)
    {
        // The forced vehicle ride (104512 -> NPC 55723) has no serverside vehicle
        // data - emulate the swallow by yanking the target into the caster instead.
        PreventHitDefaultEffect(effIndex);

        Unit* target = GetHitUnit();
        target->GetMotionMaster()->MoveJump(GetCaster()->GetPosition(), 25.0f, 10.0f);
    }

    void HandleTeleport(SpellEffIndex effIndex)
    {
        // The client teleport would overwrite the scripted parabolic pull.
        PreventHitDefaultEffect(effIndex);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect.Register(&spell_morchok_earthen_vortex::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
        OnObjectAreaTargetSelect.Register(&spell_morchok_earthen_vortex::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ENEMY);
        OnObjectAreaTargetSelect.Register(&spell_morchok_earthen_vortex::FilterTargets, EFFECT_2, TARGET_UNIT_DEST_AREA_ENEMY);
        OnEffectHitTarget.Register(&spell_morchok_earthen_vortex::HandleForceCast, EFFECT_0, SPELL_EFFECT_FORCE_CAST);
        OnEffectHitTarget.Register(&spell_morchok_earthen_vortex::HandleTeleport, EFFECT_1, SPELL_EFFECT_TELEPORT_UNITS);
    }
};

// Effect 0 normally puts the player in an unavailable serverside vehicle for
// five seconds. Hold the player through that window while preserving the DBC
// effect 2 periodic-percent damage aura.
class aura_morchok_earthen_vortex : public AuraScript
{
    void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        GetTarget()->SetControlled(true, UNIT_STATE_STUNNED);
    }

    void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        GetTarget()->SetControlled(false, UNIT_STATE_STUNNED);
    }

    void Register() override
    {
        AfterEffectApply.Register(&aura_morchok_earthen_vortex::HandleApply, EFFECT_2, SPELL_AURA_PERIODIC_DAMAGE_PERCENT, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove.Register(&aura_morchok_earthen_vortex::HandleRemove, EFFECT_2, SPELL_AURA_PERIODIC_DAMAGE_PERCENT, AURA_EFFECT_HANDLE_REAL);
    }
};

// 103785, 108570, 110288, 110287 - Black Blood of the Earth (damage)
class spell_morchok_black_blood_damage : public SpellScript
{
    void FilterTargets(std::list<WorldObject*>& targets)
    {
        Unit* caster = GetCaster();

        // The wave of black blood expands outwards from the boss over the channel
        float waveRadius = BlackBloodInitialRadius;
        if (Aura const* channel = caster->GetAura(SPELL_BLACK_BLOOD_OF_THE_EARTH))
            waveRadius += BlackBloodGrowthPerSec * float(channel->GetMaxDuration() - channel->GetDuration()) / float(IN_MILLISECONDS);

        targets.remove_if([&](WorldObject* target)
        {
            if (GetResponsibleBoss(caster, target) != caster)
                return true;

            if (caster->GetExactDist2d(target) > waveRadius)
                return true;

            // Rock spikes block the blood - explicit check because the spell
            // system skips M2 models during its own line of sight checks.
            return !caster->IsWithinLOSInMap(target);
        });
    }

    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        Creature* caster = GetCaster()->ToCreature();
        if (!caster || caster->GetEntry() != BOSS_MORCHOK || !caster->IsAIEnabled())
            return;

        if (Player* player = GetHitUnit()->ToPlayer())
        {
            if (!caster->AI()->GetData(DATA_BLACK_BLOOD_WHISPERED))
            {
                caster->AI()->Talk(WHISPER_BLACK_BLOOD, player);
                caster->AI()->SetData(DATA_BLACK_BLOOD_WHISPERED, 1);
            }
        }
    }

    void Register() override
    {
        OnObjectAreaTargetSelect.Register(&spell_morchok_black_blood_damage::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
        OnObjectAreaTargetSelect.Register(&spell_morchok_black_blood_damage::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ENEMY);
        OnEffectHitTarget.Register(&spell_morchok_black_blood_damage::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 103176 - Falling Fragments
class aura_morchok_falling_fragments : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_FALLING_FRAGMENT_MISSILE });
    }

    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        Unit* caster = GetTarget();

        // Retail sends two distinct fragment missiles on each 500 ms tick.
        for (uint8 fragment = 0; fragment < 2; ++fragment)
        {
            std::vector<uint8> candidates;
            for (uint8 i = 0; i < uint8(std::size(RockSpikePositions)); ++i)
                if (_usedPositions.find(i) == _usedPositions.end() && caster->GetExactDist2d(RockSpikePositions[i]) < 55.0f)
                    candidates.push_back(i);

            if (candidates.empty())
                return;

            uint8 const index = Trinity::Containers::SelectRandomContainerElement(candidates);
            _usedPositions.insert(index);
            caster->CastSpell(RockSpikePositions[index], SPELL_FALLING_FRAGMENT_MISSILE, true);
        }
    }

    void Register() override
    {
        OnEffectPeriodic.Register(&aura_morchok_falling_fragments::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }

    std::set<uint8> _usedPositions;
};
}

void AddSC_boss_morchok()
{
    using namespace DragonSoul;
    using namespace DragonSoul::Morchok;

    RegisterDragonSoulCreatureAI(boss_morchok);
    RegisterDragonSoulCreatureAI(npc_kohcrom);
    RegisterDragonSoulCreatureAI(npc_morchok_resonating_crystal);

    RegisterSpellScript(spell_morchok_stomp);
    RegisterSpellScript(spell_morchok_resonating_crystal_detonate);
    RegisterSpellScript(spell_morchok_earthen_vortex);
    RegisterSpellScript(aura_morchok_earthen_vortex);
    RegisterSpellScript(spell_morchok_black_blood_damage);
    RegisterSpellScript(aura_morchok_falling_fragments);
}
