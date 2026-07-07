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
#include "InstanceScript.h"
#include "Map.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "TaskScheduler.h"
#include "TemporarySummon.h"

namespace DragonSoul::Hagara
{
enum Texts
{
    SAY_INTRO_START       = 0,  // "Even with the Aspect of Time on your side..."
    SAY_INTRO_WAVE        = 1,  // two variants
    SAY_INTRO_FINAL_WAVE  = 2,  // "Not one of you will live to see the final cataclysm!"
    SAY_INTRO_DONE        = 3,  // "Swagger all you like..."
    SAY_AGGRO             = 4,
    SAY_ICE_TOMB          = 5,  // two variants
    SAY_ICE_PHASE         = 6,  // two variants
    SAY_LIGHTNING_PHASE   = 7,  // two variants
    SAY_FOCUSED_ASSAULT   = 8,  // three variants
    SAY_CRYSTAL_DIED      = 9,  // seven variants
    SAY_LAST_CRYSTAL      = 10, // "The one remaining is still enough to finish you."
    SAY_CONDUCTOR_CHARGED = 11, // four variants
    SAY_LIGHTNING_END     = 12, // two variants
    SAY_FEEDBACK_END      = 13, // "I'll finish you now, pups!"
    SAY_SLAY              = 14, // four variants
    SAY_DEATH             = 15
};

enum Spells
{
    SPELL_BERSERK                       = 26662,

    // Assault phase
    SPELL_FOCUSED_ASSAULT               = 107851, // forks: 110900 / 110899 / 110898 (aura on Hagara, 500ms strikes)
    SPELL_FOCUSED_ASSAULT_STRIKE        = 107850, // 50% weapon damage on her target, melee range gated in-script
    SPELL_SHATTERED_ICE                 = 105289, // forks: 108567 / 110888 / 110887
    SPELL_ICE_TOMB_TARGETING            = 104448, // dummy on src area, target count scripted
    SPELL_ICE_TOMB_AURA                 = 104451, // stun + 5% max health per second suffocation
    SPELL_ICE_LANCE_MARKER              = 105269, // target marker visual
    SPELL_ICE_LANCE_MISSILE             = 105313, // missile -> 105316 impact
    SPELL_ICE_LANCE_IMPACT              = 105316, // forks: 107061 / 107062 / 107063 (damage + stacking debuff)
    SPELL_FROSTFLAKE                    = 109325, // heroic: stacking snare, drops a patch when removed
    SPELL_FROSTFLAKE_SNARE_SLOW         = 123457, // serverside: the patch's -50% snare (109337 is an areatrigger)
    SPELL_SNOWDRIFT_VISUAL              = 62463,  // ground frost visual for the snare patch

    // Ice phase
    SPELL_FROZEN_TEMPEST                = 105256, // forks: 109552 / 109553 / 109554 (immunity + knockback)
    SPELL_WATERY_ENTRENCHMENT           = 110317, // 12% max health per second, applied while inside the ring
    SPELL_ICE_WAVE_VISUAL               = 105265, // wall segment visual on the wave stalkers
    SPELL_ICE_WAVE_DAMAGE               = 105314, // ~190k frost, cast on swept players
    SPELL_ICICLE_TELEGRAPH              = 69426,  // 1.7s telegraph, ticks 69425 + Snowdrift
    SPELL_ICICLE_DAMAGE                 = 69425,  // damage overridden from the tuning table
    SPELL_CRYSTALLINE_TETHER_FROST      = 105311, // on each living Binding Crystal
    SPELL_CRYSTALLINE_OVERLOAD          = 105312, // crystal death burst
    SPELL_SIMPLE_TELEPORT               = 70618,  // crystal spawn flash

    // Lightning phase
    SPELL_WATER_SHIELD                  = 105409, // forks: 109560 / 109561 / 109562 (immunity + 2.1s Lightning Storm pulse)
    SPELL_LIGHTNING_STORM               = 105465, // forks: 108568 / 110893 / 110892 (raid pulse from the shield)
    SPELL_LIGHTNING_CONDUIT             = 105369, // forks: 108569 / 109201 / 109202 (10k nature per second chain aura)
    SPELL_CRYSTALLINE_TETHER_LIGHTNING  = 105482, // on each inert Crystal Conductor
    SPELL_LIGHTNING_ROD                 = 105343, // charged conductor visual
    SPELL_CONDUCTOR_OVERLOAD            = 105487, // conductor burst when the phase ends
    SPELL_STORM_PILLAR                  = 109541, // heroic: missile -> 109563 forks (35k, 5yd)

    // Feedback
    SPELL_FEEDBACK                      = 108934, // +100% damage taken + self-stun, 15s

    // Holding Hands (serverside credit, criteria 18608)
    SPELL_HOLDING_HANDS_CREDIT          = 110520,

    // Intro assault event
    SPELL_FROSTBOLT                     = 109334, // Twilight Frost Evoker
    SPELL_SHACKLES_OF_ICE               = 109423, // Twilight Frost Evoker
    SPELL_FROST_CORRUPTION              = 109333, // Twilight Frost Evoker
    SPELL_CHAIN_LIGHTNING               = 109348, // Stormborn Myrmidon
    SPELL_SPARK                         = 109368, // Stormborn Myrmidon
    SPELL_CHAIN_LIGHTNING_ADEPT         = 109427, // Stormbinder Adept
    SPELL_ADEPT_TELEPORT                = 109424  // Stormbinder Adept blink visual
};

enum Phases : uint8
{
    PHASE_NONE = 0,
    PHASE_ASSAULT,
    PHASE_ICE,
    PHASE_LIGHTNING,
    PHASE_FEEDBACK
};

enum Events
{
    // Assault
    EVENT_FOCUSED_ASSAULT = 1,
    EVENT_SHATTERED_ICE,
    EVENT_ICE_TOMB,
    EVENT_ICE_LANCE,
    EVENT_FROSTFLAKE,
    EVENT_SPECIAL_PHASE,
    // Shared
    EVENT_FEEDBACK_END,
    EVENT_PHASE_WATCHDOG,
    EVENT_BERSERK
};

enum Actions
{
    // Crystal Conductor visual states (boss AI owns the charge bookkeeping)
    ACTION_CONDUCTOR_INERT = 1,
    ACTION_CONDUCTOR_CHARGE,
    ACTION_CONDUCTOR_RESET
};

enum Points
{
    POINT_CENTER = 1,
    POINT_INTRO_DESCENT
};

enum MiscConst
{
    NPC_WORLD_TRIGGER  = 22515, // Frostflake Snare patch stalker
    GUID_TOMB_VICTIM   = 1,
    GUID_LANCE_TARGET  = 2
};

namespace
{
constexpr uint32 SchedulerGroupIntro     = 1;
constexpr uint32 SchedulerGroupCombat    = 2;
constexpr uint32 SchedulerGroupIce       = 3;
constexpr uint32 SchedulerGroupLightning = 4;

Position const PlatformCenter = { 13587.29f, 13611.83f, 122.503f, 0.0f };
Position const GroundHome     = { 13579.00f, 13612.00f, 122.55f,  0.0f };

// The four permanent Crystal Conductors sit on the platform cardinals; on
// heroic, four extra conductors are summoned on the intercardinals (the
// Binding Crystal spots) for the lightning phase
Position const ExtraConductorPositions[4] =
{
    { 13557.42f, 13643.13f, 123.567f, 5.480f }, // NW
    { 13617.32f, 13643.45f, 123.567f, 3.944f }, // NE
    { 13557.73f, 13580.65f, 123.567f, 0.803f }, // SW
    { 13617.50f, 13580.92f, 123.567f, 2.356f }  // SE
};

Position const& BindingCrystalPosition(uint8 index) { return ExtraConductorPositions[index]; }

Position const TwilightPortalPositions[6] =
{
    { 13558.0f, 13581.0f, 123.6f, 0.79f },
    { 13588.0f, 13652.0f, 123.6f, 4.71f },
    { 13547.0f, 13613.0f, 123.6f, 0.00f },
    { 13557.0f, 13642.0f, 123.6f, 5.50f },
    { 13587.0f, 13570.0f, 123.6f, 1.57f },
    { 13569.0f, 13612.0f, 122.5f, 0.00f }
};

// Ice Wave geometry: four wall arms at 90 degree offsets sweep the platform;
// damage comes from a polar-coordinate check, the stalkers are visuals only
constexpr float WaveAngularSpeed  = 0.3927f; // rad/s, ~16s per revolution
constexpr float WaveInnerRadius   = 13.0f;
constexpr float WaveOuterRadius   = 58.0f;
constexpr float WaveHalfThickness = 2.5f;    // yards, converted to an angle per player radius
constexpr uint32 WaveGraceMs      = 1500;    // per-player re-hit protection
float const WaveStalkerRadii[6] = { 15.0f, 23.0f, 31.0f, 39.0f, 47.0f, 55.0f };

constexpr float EntrenchmentRadius = 12.0f;
constexpr float ConduitChainRange  = 10.0f;
constexpr float ConductorChargeRange = 10.0f;
constexpr float SnarePatchRadius   = 5.0f;
constexpr float IceLanceRadius     = 52.0f;  // platform rim
constexpr float PlatformFallZ      = 100.0f; // below this you fell off

struct HagaraTuning
{
    uint8  IceTombTargets;
    uint32 IceTombHealth;
    uint8  IceLanceCount;
    uint8  ConductorCharges;
    int32  IcicleDamage;
    bool   HeroicAbilities;
};

bool IsHeroicHagara(Map const* map)
{
    return map->GetDifficulty() == RAID_DIFFICULTY_10MAN_HEROIC
        || map->GetDifficulty() == RAID_DIFFICULTY_25MAN_HEROIC;
}

bool IsLFR(InstanceScript const* instance)
{
    return instance && instance->IsLFR();
}

// Health from user-supplied retail values; icicle damage is a playtest knob
HagaraTuning const& GetTuning(InstanceScript const* instance, Map const* map)
{
    static HagaraTuning const lfr    = { 5,  976374, 5, 4, 60000, false };
    static HagaraTuning const n10    = { 2,  542430, 3, 4, 60000, false };
    static HagaraTuning const n25    = { 5, 1394820, 5, 4, 60000, false };
    static HagaraTuning const h10    = { 2,  488187, 3, 8, 85000, true  };
    static HagaraTuning const h25    = { 6, 1937250, 5, 8, 85000, true  };

    if (IsLFR(instance))
        return lfr;

    switch (map->GetDifficulty())
    {
        case RAID_DIFFICULTY_25MAN_NORMAL: return n25;
        case RAID_DIFFICULTY_10MAN_HEROIC: return h10;
        case RAID_DIFFICULTY_25MAN_HEROIC: return h25;
        default:                           return n10;
    }
}

// Raid Finder tuning: reduced health (from the LFR stats templates) and
// reduced outgoing damage, applied on top of the 25 player normal profile
void ApplyLFRHealth(Creature* creature, InstanceScript const* instance, uint32 statsEntry)
{
    if (!IsLFR(instance))
        return;

    CreatureTemplate const* lfrStats = sObjectMgr->GetCreatureTemplate(statsEntry);
    if (!lfrStats)
        return;

    if (CreatureBaseStats const* baseStats = sObjectMgr->GetCreatureBaseStats(creature->getLevel(), lfrStats->unit_class))
    {
        creature->SetMaxHealth(baseStats->GenerateHealth(lfrStats));
        creature->SetFullHealth();
    }
}

void ApplyLFRDamageReduction(InstanceScript const* instance, uint32& damage)
{
    if (IsLFR(instance))
        damage = damage * LFR_DAMAGE_PCT / 100;
}

// Intro assault event: six waves from the Twilight Portals. 25 player raids
// use the dedicated (entry + 1) templates at higher counts.
struct IntroWaveEntry
{
    uint32 Entry;
    uint8 Count10;
    uint8 Count25;
};

std::vector<IntroWaveEntry> const IntroWaves[6] =
{
    { { NPC_TWILIGHT_FROST_EVOKER, 2, 3 } },
    { { NPC_STORMBORN_MYRMIDON,    2, 3 } },
    { { NPC_STORMBINDER_ADEPT,     2, 3 }, { NPC_CORRUPTED_FRAGMENT, 4, 8 } },
    { { NPC_TWILIGHT_FROST_EVOKER, 2, 3 }, { NPC_STORMBORN_MYRMIDON, 1, 2 } },
    { { NPC_STORMBINDER_ADEPT,     2, 3 }, { NPC_TWILIGHT_FROST_EVOKER, 2, 3 } },
    { { NPC_TWILIGHT_FROST_EVOKER, 1, 2 }, { NPC_STORMBORN_MYRMIDON, 1, 2 }, { NPC_STORMBINDER_ADEPT, 1, 2 }, { NPC_CORRUPTED_FRAGMENT, 6, 10 } }
};

bool IsIntroAddEntry(uint32 entry)
{
    switch (entry)
    {
        case NPC_TWILIGHT_FROST_EVOKER:
        case NPC_TWILIGHT_FROST_EVOKER + 1:
        case NPC_STORMBORN_MYRMIDON:
        case NPC_STORMBORN_MYRMIDON + 1:
        case NPC_STORMBINDER_ADEPT:
        case NPC_STORMBINDER_ADEPT + 1:
        case NPC_CORRUPTED_FRAGMENT:
        case NPC_CORRUPTED_FRAGMENT + 1:
            return true;
        default:
            return false;
    }
}
}

struct boss_hagara : public BossAI
{
    boss_hagara(Creature* creature) : BossAI(creature, DATA_HAGARA_THE_STORMBINDER) { }

    void Reset() override
    {
        _Reset();
        _scheduler.CancelAll();
        _phase = PHASE_NONE;
        _nextSpecial = PHASE_ICE;
        _crystalsDead = 0;
        _chargedCount = 0;
        _chainIntact = true;
        _chainWindow = false;
        _conductorCharged.clear();
        _waveStalkers.clear();
        _waveGrace.clear();
        _introRunning = false;
        _introWave = 0;
        _introAlive = 0;

        ResetConductors();

        if (instance->GetData(DATA_HAGARA_INTRO_DONE))
            SetGroundStance();
        else
            SetPerchStance();

        ApplyLFRHealth(me, instance, NPC_HAGARA_LFR_STATS);
    }

    void DamageDealt(Unit* /*victim*/, uint32& damage, DamageEffectType damageType) override
    {
        // Direct spell hits carry their own Raid Finder cuts in the spell
        // scripts; this handles melee and periodic ticks
        if (damageType != SPELL_DIRECT_DAMAGE)
            ApplyLFRDamageReduction(instance, damage);
    }

    void MoveInLineOfSight(Unit* who) override
    {
        if (!_introRunning && !instance->GetData(DATA_HAGARA_INTRO_DONE)
            && who->GetTypeId() == TYPEID_PLAYER && !who->ToPlayer()->IsGameMaster()
            && me->IsWithinDistInMap(who, 60.0f))
            StartIntroEvent();

        BossAI::MoveInLineOfSight(who);
    }

    void JustEngagedWith(Unit* who) override
    {
        BossAI::JustEngagedWith(who);
        Talk(SAY_AGGRO);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

        _nextSpecial = PHASE_ICE;
        StartAssaultPhase(true);

        if (!IsLFR(instance))
            events.ScheduleEvent(EVENT_BERSERK, 8min);

        // Players knocked or jumping off the platform fall into the abyss
        _scheduler.Schedule(2s, SchedulerGroupCombat, [this](TaskContext context)
        {
            std::list<Player*> players;
            me->GetPlayerListInGrid(players, 200.0f);
            for (Player* player : players)
                if (player->IsAlive() && !player->IsGameMaster() && player->GetPositionZ() < PlatformFallZ)
                    Unit::Kill(me, player);
            context.Repeat(2s);
        });

        // Frostflake Snare patches (heroic): refresh the snare on anyone inside
        if (GetTuning(instance, me->GetMap()).HeroicAbilities)
        {
            _scheduler.Schedule(1s, SchedulerGroupCombat, [this](TaskContext context)
            {
                for (ObjectGuid guid : summons)
                {
                    Creature* patch = ObjectAccessor::GetCreature(*me, guid);
                    if (!patch || patch->GetEntry() != NPC_WORLD_TRIGGER)
                        continue;

                    std::list<Player*> players;
                    patch->GetPlayerListInGrid(players, SnarePatchRadius);
                    for (Player* player : players)
                        if (player->IsAlive())
                            if (Aura* snare = patch->AddAura(SPELL_FROSTFLAKE_SNARE_SLOW, player))
                                snare->SetDuration(3000);
                }
                context.Repeat(1s);
            });
        }
    }

    void KilledUnit(Unit* victim) override
    {
        if (victim->GetTypeId() == TYPEID_PLAYER && roll_chance_i(50))
            Talk(SAY_SLAY);
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
        Talk(SAY_DEATH);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
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

    void DoAction(int32 /*action*/) override { }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != POINT_MOTION_TYPE && type != EFFECT_MOTION_TYPE)
            return;

        if (pointId == POINT_CENTER)
        {
            me->SetControlled(true, UNIT_STATE_ROOT);
            if (_phase == PHASE_ICE)
                StartIcePhase();
            else if (_phase == PHASE_LIGHTNING)
                StartLightningPhase();
        }
        else if (pointId == POINT_INTRO_DESCENT)
        {
            me->SetHomePosition(GroundHome);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
            me->SetReactState(REACT_AGGRESSIVE);
        }
    }

    void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
    {
        if (IsIntroAddEntry(summon->GetEntry()))
        {
            if (_introRunning && _introAlive)
                --_introAlive;
            return;
        }

        switch (summon->GetEntry())
        {
            case NPC_FROZEN_BINDING_CRYSTAL:
                OnBindingCrystalDied(summon);
                break;
            case NPC_BOUND_LIGHTNING_ELEMENTAL:
                OnElementalDied(summon);
                break;
            default:
                break;
        }
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
                case EVENT_FOCUSED_ASSAULT:
                    if (roll_chance_i(30))
                        Talk(SAY_FOCUSED_ASSAULT);
                    DoCastVictim(SPELL_FOCUSED_ASSAULT);
                    events.Repeat(15s);
                    break;
                case EVENT_SHATTERED_ICE:
                {
                    Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 100.0f, true);
                    if (!target)
                        target = me->GetVictim();
                    if (target)
                        DoCast(target, SPELL_SHATTERED_ICE);
                    events.Repeat(10500ms, 15s);
                    break;
                }
                case EVENT_ICE_TOMB:
                    Talk(SAY_ICE_TOMB);
                    DoCastAOE(SPELL_ICE_TOMB_TARGETING);
                    break;
                case EVENT_ICE_LANCE:
                    SummonIceLances();
                    events.Repeat(30s);
                    break;
                case EVENT_FROSTFLAKE:
                {
                    // Prefers ranged: anyone outside melee range of the boss
                    Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, [this](Unit* unit)
                    {
                        return unit->GetTypeId() == TYPEID_PLAYER && !me->IsWithinMeleeRange(unit) && !unit->HasAura(SPELL_FROSTFLAKE);
                    });
                    if (!target)
                        target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true);
                    if (target)
                        DoCast(target, SPELL_FROSTFLAKE, true);
                    events.Repeat(5s);
                    break;
                }
                case EVENT_SPECIAL_PHASE:
                    BeginSpecialPhase(_nextSpecial);
                    _nextSpecial = (_nextSpecial == PHASE_ICE) ? PHASE_LIGHTNING : PHASE_ICE;
                    break;
                case EVENT_FEEDBACK_END:
                    Talk(SAY_FEEDBACK_END);
                    StartAssaultPhase(false);
                    break;
                case EVENT_PHASE_WATCHDOG:
                    TC_LOG_ERROR("scripts", "boss_hagara: special phase watchdog fired in phase {}, forcing phase end", _phase);
                    EndSpecialPhase();
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

        if (_phase == PHASE_ASSAULT)
            DoMeleeAttackIfReady();
    }

private:
    /* ------------------------------------------------------------------ */
    /* Intro assault event                                                */
    /* ------------------------------------------------------------------ */

    void SetPerchStance()
    {
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
        me->SetReactState(REACT_PASSIVE);
    }

    void SetGroundStance()
    {
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
        me->SetReactState(REACT_AGGRESSIVE);
        me->SetHomePosition(GroundHome);
        if (me->GetExactDist2d(GroundHome) > 5.0f && !me->IsInCombat())
            me->NearTeleportTo(GroundHome);
    }

    void StartIntroEvent()
    {
        _introRunning = true;
        _introWave = 0;
        _introAlive = 0;
        Talk(SAY_INTRO_START);

        for (Position const& pos : TwilightPortalPositions)
            me->SummonCreature(NPC_HAGARA_TWILIGHT_PORTAL, pos, TEMPSUMMON_MANUAL_DESPAWN);

        _scheduler.Schedule(5s, SchedulerGroupIntro, [this](TaskContext /*context*/)
        {
            SpawnIntroWave();
        });

        // Wipe check: reset the whole event if everyone died or left
        _scheduler.Schedule(10s, SchedulerGroupIntro, [this](TaskContext context)
        {
            std::list<Player*> players;
            me->GetPlayerListInGrid(players, 120.0f);
            players.remove_if([](Player* player) { return !player->IsAlive() || player->IsGameMaster(); });
            if (players.empty())
            {
                ResetIntroEvent();
                return;
            }
            context.Repeat(5s);
        });
    }

    void ResetIntroEvent()
    {
        _scheduler.CancelGroup(SchedulerGroupIntro);
        _introRunning = false;
        _introWave = 0;
        _introAlive = 0;
        summons.DespawnEntry(NPC_HAGARA_TWILIGHT_PORTAL);
        for (uint32 entry : { NPC_TWILIGHT_FROST_EVOKER, NPC_STORMBORN_MYRMIDON, NPC_STORMBINDER_ADEPT, NPC_CORRUPTED_FRAGMENT })
        {
            summons.DespawnEntry(entry);
            summons.DespawnEntry(entry + 1); // 25 player templates
        }
    }

    void SpawnIntroWave()
    {
        if (_introWave >= std::size(IntroWaves))
            return;

        bool const is25 = me->GetMap()->Is25ManRaid();
        uint8 portal = urand(0, std::size(TwilightPortalPositions) - 1);

        for (IntroWaveEntry const& waveEntry : IntroWaves[_introWave])
        {
            uint8 const count = is25 ? waveEntry.Count25 : waveEntry.Count10;
            for (uint8 i = 0; i < count; ++i)
            {
                Position pos = TwilightPortalPositions[portal];
                pos.RelocateOffset({ frand(-3.0f, 3.0f), frand(-3.0f, 3.0f), 0.0f, 0.0f });
                if (me->SummonCreature(waveEntry.Entry + (is25 ? 1 : 0), pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 4s))
                    ++_introAlive;
                portal = (portal + 1) % std::size(TwilightPortalPositions);
            }
        }

        ++_introWave;

        if (_introWave == std::size(IntroWaves))
            Talk(SAY_INTRO_FINAL_WAVE);
        else if (_introWave == 1 || _introWave == 3)
            Talk(SAY_INTRO_WAVE);

        // Next wave when this one is cleared, or after 25 seconds
        uint8 const waveWhenScheduled = _introWave;
        _scheduler.Schedule(1s, SchedulerGroupIntro, [this, waveWhenScheduled](TaskContext context)
        {
            if (!_introRunning || _introWave != waveWhenScheduled)
                return;

            bool const cleared = _introAlive == 0;
            bool const timedOut = context.GetRepeatCounter() >= 24 && _introWave < std::size(IntroWaves);

            if (cleared && _introWave >= std::size(IntroWaves))
            {
                CompleteIntroEvent();
                return;
            }

            if (cleared || timedOut)
            {
                SpawnIntroWave();
                return;
            }

            context.Repeat(1s);
        });
    }

    void CompleteIntroEvent()
    {
        _scheduler.CancelGroup(SchedulerGroupIntro);
        _introRunning = false;
        summons.DespawnEntry(NPC_HAGARA_TWILIGHT_PORTAL);
        Talk(SAY_INTRO_DONE);
        instance->SetData(DATA_HAGARA_INTRO_DONE, 1);
        me->GetMotionMaster()->MoveJump(GroundHome, 20.0f, 15.0f, POINT_INTRO_DESCENT);
    }

    /* ------------------------------------------------------------------ */
    /* Assault phase                                                      */
    /* ------------------------------------------------------------------ */

    void StartAssaultPhase(bool first)
    {
        _phase = PHASE_ASSAULT;
        events.SetPhase(PHASE_ASSAULT);
        me->SetReactState(REACT_AGGRESSIVE);
        if (Unit* victim = me->GetVictim())
            AttackStart(victim);

        events.ScheduleEvent(EVENT_FOCUSED_ASSAULT, first ? 4s : 1s, 0, PHASE_ASSAULT);
        events.ScheduleEvent(EVENT_SHATTERED_ICE, 10500ms, 0, PHASE_ASSAULT);
        events.ScheduleEvent(EVENT_ICE_LANCE, first ? 10s : 12s, 0, PHASE_ASSAULT);
        if (!first)
            events.ScheduleEvent(EVENT_ICE_TOMB, 20s, 0, PHASE_ASSAULT);
        if (GetTuning(instance, me->GetMap()).HeroicAbilities)
            events.ScheduleEvent(EVENT_FROSTFLAKE, 5s, 0, PHASE_ASSAULT);

        // The special phase cycle runs 62s shield-break to shield-break;
        // Feedback eats 15s of that after the first special
        events.ScheduleEvent(EVENT_SPECIAL_PHASE, first ? 30s : 47s, 0, PHASE_ASSAULT);
    }

    void SummonIceLances()
    {
        uint8 const count = GetTuning(instance, me->GetMap()).IceLanceCount;

        std::list<Player*> players;
        me->GetPlayerListInGrid(players, 100.0f);
        players.remove_if([this](Player* player) { return !player->IsAlive() || player->IsGameMaster() || player->HasAura(SPELL_ICE_TOMB_AURA); });
        if (players.empty())
            return;

        float const startAngle = frand(0.0f, 2.0f * float(M_PI));
        for (uint8 i = 0; i < count; ++i)
        {
            float const angle = Position::NormalizeOrientation(startAngle + i * 2.0f * float(M_PI) / count);
            Position pos =
            {
                PlatformCenter.GetPositionX() + IceLanceRadius * std::cos(angle),
                PlatformCenter.GetPositionY() + IceLanceRadius * std::sin(angle),
                PlatformCenter.GetPositionZ(), Position::NormalizeOrientation(angle + float(M_PI))
            };
            pos.m_positionZ = std::max(me->GetMapHeight(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ() + 5.0f), PlatformCenter.GetPositionZ());

            Creature* lance = me->SummonCreature(NPC_ICE_LANCE, pos, TEMPSUMMON_TIMED_DESPAWN, 16s);
            if (!lance)
                continue;

            // Each lance channels at its own player, unique while there are
            // enough candidates
            Player* target = Trinity::Containers::SelectRandomContainerElement(players);
            if (players.size() > 1)
                players.remove(target);
            lance->AI()->SetGUID(target->GetGUID(), GUID_LANCE_TARGET);
        }
    }

    /* ------------------------------------------------------------------ */
    /* Special phase framework                                            */
    /* ------------------------------------------------------------------ */

    void BeginSpecialPhase(uint8 phase)
    {
        _phase = phase;
        events.SetPhase(phase);
        me->SetReactState(REACT_PASSIVE);
        me->AttackStop();
        me->InterruptNonMeleeSpells(true);
        RemoveAuraFromSelfWithForks(SPELL_FOCUSED_ASSAULT);

        // Assault leftovers do not cross into the special phase
        summons.DespawnEntry(NPC_ICE_LANCE);
        summons.DespawnEntry(NPC_ICE_TOMB);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_ICE_TOMB_AURA);

        Talk(phase == PHASE_ICE ? SAY_ICE_PHASE : SAY_LIGHTNING_PHASE);

        events.ScheduleEvent(EVENT_PHASE_WATCHDOG, 5min, 0, phase);
        me->GetMotionMaster()->MovePoint(POINT_CENTER, PlatformCenter);
    }

    void StartIcePhase()
    {
        _crystalsDead = 0;
        DoCastSelf(SPELL_FROZEN_TEMPEST);

        // Watery Entrenchment: the ring around her hits for 12% max health
        // per second (105259 is an areatrigger the core cannot run)
        _scheduler.Schedule(1s, SchedulerGroupIce, [this](TaskContext context)
        {
            std::list<Player*> players;
            me->GetPlayerListInGrid(players, 60.0f);
            for (Player* player : players)
            {
                bool const inside = player->IsAlive() && player->GetExactDist2d(PlatformCenter) <= EntrenchmentRadius;
                if (inside && !player->HasAura(SPELL_WATERY_ENTRENCHMENT))
                    me->AddAura(SPELL_WATERY_ENTRENCHMENT, player);
                else if (!inside && player->HasAura(SPELL_WATERY_ENTRENCHMENT))
                    player->RemoveAurasDueToSpell(SPELL_WATERY_ENTRENCHMENT);
            }
            context.Repeat(1s);
        });

        // Four Frozen Binding Crystals on the intercardinals
        for (uint8 i = 0; i < 4; ++i)
            me->SummonCreature(NPC_FROZEN_BINDING_CRYSTAL, BindingCrystalPosition(i), TEMPSUMMON_MANUAL_DESPAWN);

        StartIceWaves();

        // Collapsing Icicles crash down near random players
        _scheduler.Schedule(4s, SchedulerGroupIce, [this](TaskContext context)
        {
            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
            {
                Position pos = target->GetPosition();
                pos.RelocateOffset({ frand(-3.0f, 3.0f), frand(-3.0f, 3.0f), 0.0f, 0.0f });
                me->SummonCreature(NPC_COLLAPSING_ICICLE, pos, TEMPSUMMON_TIMED_DESPAWN, 3s);
            }
            context.Repeat(3s, 5s);
        });
    }

    void StartIceWaves()
    {
        _waveAngle = frand(0.0f, 2.0f * float(M_PI));
        _waveDirection = urand(0, 1) ? 1.0f : -1.0f;
        _waveStalkers.clear();
        _waveGrace.clear();

        for (uint8 arm = 0; arm < 4; ++arm)
        {
            float const armAngle = _waveAngle + arm * float(M_PI) / 2.0f;
            for (float radius : WaveStalkerRadii)
            {
                Position pos = WavePosition(armAngle, radius);
                if (Creature* stalker = me->SummonCreature(NPC_ICE_WAVE, pos, TEMPSUMMON_MANUAL_DESPAWN))
                    _waveStalkers.push_back(stalker->GetGUID());
            }
        }

        _scheduler.Schedule(250ms, SchedulerGroupIce, [this](TaskContext context)
        {
            UpdateIceWaves(250);
            context.Repeat(250ms);
        });
    }

    Position WavePosition(float angle, float radius) const
    {
        return { PlatformCenter.GetPositionX() + radius * std::cos(angle),
                 PlatformCenter.GetPositionY() + radius * std::sin(angle),
                 PlatformCenter.GetPositionZ() + 1.0f, Position::NormalizeOrientation(angle + float(M_PI) / 2.0f) };
    }

    void UpdateIceWaves(uint32 diffMs)
    {
        _waveAngle = Position::NormalizeOrientation(_waveAngle + _waveDirection * WaveAngularSpeed * diffMs / 1000.0f);

        // Reposition the visual stalkers (6 per arm, 4 arms)
        for (size_t i = 0; i < _waveStalkers.size(); ++i)
        {
            Creature* stalker = ObjectAccessor::GetCreature(*me, _waveStalkers[i]);
            if (!stalker)
                continue;

            float const armAngle = _waveAngle + float(i / std::size(WaveStalkerRadii)) * float(M_PI) / 2.0f;
            Position const pos = WavePosition(armAngle, WaveStalkerRadii[i % std::size(WaveStalkerRadii)]);
            stalker->NearTeleportTo(pos);
        }

        // Damage from geometry: a player is hit when their polar angle falls
        // within a wall arm's angular footprint at their distance from center
        for (auto itr = _waveGrace.begin(); itr != _waveGrace.end();)
        {
            if (itr->second <= diffMs)
                itr = _waveGrace.erase(itr);
            else
            {
                itr->second -= diffMs;
                ++itr;
            }
        }

        std::list<Player*> players;
        me->GetPlayerListInGrid(players, 70.0f);
        for (Player* player : players)
        {
            if (!player->IsAlive() || player->IsGameMaster() || _waveGrace.count(player->GetGUID()))
                continue;

            float const radius = player->GetExactDist2d(PlatformCenter);
            if (radius < WaveInnerRadius || radius > WaveOuterRadius)
                continue;

            float const playerAngle = PlatformCenter.GetAngle(player);
            float const halfWidth = std::atan2(WaveHalfThickness, radius);

            for (uint8 arm = 0; arm < 4; ++arm)
            {
                float const armAngle = Position::NormalizeOrientation(_waveAngle + arm * float(M_PI) / 2.0f);
                float delta = std::fabs(Position::NormalizeOrientation(playerAngle - armAngle));
                if (delta > float(M_PI))
                    delta = 2.0f * float(M_PI) - delta;

                if (delta <= halfWidth)
                {
                    _waveGrace[player->GetGUID()] = WaveGraceMs;
                    if (Creature* stalker = NearestWaveStalker(arm, radius))
                        stalker->CastSpell(player, SPELL_ICE_WAVE_DAMAGE, true);
                    break;
                }
            }
        }
    }

    Creature* NearestWaveStalker(uint8 arm, float radius)
    {
        size_t best = 0;
        float bestDelta = std::numeric_limits<float>::max();
        for (size_t i = 0; i < std::size(WaveStalkerRadii); ++i)
        {
            float const delta = std::fabs(WaveStalkerRadii[i] - radius);
            if (delta < bestDelta)
            {
                bestDelta = delta;
                best = i;
            }
        }

        size_t const index = size_t(arm) * std::size(WaveStalkerRadii) + best;
        if (index >= _waveStalkers.size())
            return nullptr;
        return ObjectAccessor::GetCreature(*me, _waveStalkers[index]);
    }

    void OnBindingCrystalDied(Creature* crystal)
    {
        crystal->CastSpell(crystal, SPELL_CRYSTALLINE_OVERLOAD, true);
        ++_crystalsDead;

        if (_crystalsDead >= 4)
            EndSpecialPhase();
        else if (_crystalsDead == 3)
            Talk(SAY_LAST_CRYSTAL);
        else
            Talk(SAY_CRYSTAL_DIED);
    }

    /* ------------------------------------------------------------------ */
    /* Lightning phase                                                    */
    /* ------------------------------------------------------------------ */

    void StartLightningPhase()
    {
        _chargedCount = 0;
        _chainIntact = true;
        _chainWindow = false;
        DoCastSelf(SPELL_WATER_SHIELD);

        // Heroic: four extra conductors join the four permanent ones
        if (GetTuning(instance, me->GetMap()).ConductorCharges > 4)
            for (Position const& pos : ExtraConductorPositions)
                me->SummonCreature(NPC_CRYSTAL_CONDUCTOR, pos, TEMPSUMMON_MANUAL_DESPAWN);

        CollectConductors();
        for (ObjectGuid guid : _conductors)
            if (Creature* conductor = ObjectAccessor::GetCreature(*me, guid))
                conductor->AI()->DoAction(ACTION_CONDUCTOR_INERT);

        SummonLightningElemental();

        // Nothing alive to charge a conductor? Summon a replacement
        _scheduler.Schedule(5s, SchedulerGroupLightning, [this](TaskContext context)
        {
            if (!HasLivingElemental() && _chargedCount < GetTuning(instance, me->GetMap()).ConductorCharges)
                SummonLightningElemental();
            context.Repeat(5s);
        });

        // The conduit chain: electricity arcs from charged conductors through
        // players standing within 10 yards of each other
        _scheduler.Schedule(1s, SchedulerGroupLightning, [this](TaskContext context)
        {
            ConduitChainPulse();
            context.Repeat(1s);
        });

        if (GetTuning(instance, me->GetMap()).HeroicAbilities)
        {
            _scheduler.Schedule(5s, SchedulerGroupLightning, [this](TaskContext context)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    DoCast(target, SPELL_STORM_PILLAR, true);
                context.Repeat(5s);
            });
        }
    }

    void CollectConductors()
    {
        _conductors.clear();
        _conductorCharged.clear();

        std::list<Creature*> conductors;
        me->GetCreatureListWithEntryInGrid(conductors, NPC_CRYSTAL_CONDUCTOR, 100.0f);
        for (Creature* conductor : conductors)
        {
            _conductors.push_back(conductor->GetGUID());
            _conductorCharged[conductor->GetGUID()] = false;
        }
    }

    void ResetConductors()
    {
        std::list<Creature*> conductors;
        me->GetCreatureListWithEntryInGrid(conductors, NPC_CRYSTAL_CONDUCTOR, 100.0f);
        for (Creature* conductor : conductors)
            if (conductor->IsAIEnabled())
                conductor->AI()->DoAction(ACTION_CONDUCTOR_RESET);
    }

    void SummonLightningElemental()
    {
        float const angle = frand(0.0f, 2.0f * float(M_PI));
        Position pos = { PlatformCenter.GetPositionX() + 8.0f * std::cos(angle),
                         PlatformCenter.GetPositionY() + 8.0f * std::sin(angle),
                         PlatformCenter.GetPositionZ(), 0.0f };
        me->SummonCreature(NPC_BOUND_LIGHTNING_ELEMENTAL, pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5s);
    }

    bool HasLivingElemental()
    {
        for (ObjectGuid guid : summons)
            if (Creature* summon = ObjectAccessor::GetCreature(*me, guid))
                if (summon->GetEntry() == NPC_BOUND_LIGHTNING_ELEMENTAL && summon->IsAlive())
                    return true;
        return false;
    }

    void OnElementalDied(Creature* elemental)
    {
        // Dying next to an inert conductor charges it; anywhere else the
        // elemental was wasted and the watchdog summons a replacement
        Creature* nearest = nullptr;
        float nearestDist = ConductorChargeRange;
        for (ObjectGuid guid : _conductors)
        {
            if (_conductorCharged[guid])
                continue;
            Creature* conductor = ObjectAccessor::GetCreature(*me, guid);
            if (!conductor)
                continue;
            float const dist = elemental->GetExactDist2d(conductor);
            if (dist <= nearestDist)
            {
                nearest = conductor;
                nearestDist = dist;
            }
        }

        if (nearest)
            ChargeConductor(nearest);
    }

    void ChargeConductor(Creature* conductor)
    {
        if (_conductorCharged[conductor->GetGUID()])
            return;

        _conductorCharged[conductor->GetGUID()] = true;
        conductor->AI()->DoAction(ACTION_CONDUCTOR_CHARGE);
        ++_chargedCount;
        _chainWindow = true;
        Talk(SAY_CONDUCTOR_CHARGED);

        if (_chargedCount >= GetTuning(instance, me->GetMap()).ConductorCharges)
        {
            Talk(SAY_LIGHTNING_END);

            for (ObjectGuid guid : _conductors)
                if (Creature* charged = ObjectAccessor::GetCreature(*me, guid))
                    charged->CastSpell(charged, SPELL_CONDUCTOR_OVERLOAD, true);

            if (_chainIntact && !IsLFR(instance))
            {
                std::list<Player*> players;
                me->GetPlayerListInGrid(players, 100.0f);
                for (Player* player : players)
                    if (player->IsAlive() && !player->IsGameMaster())
                        me->CastSpell(player, SPELL_HOLDING_HANDS_CREDIT, true);
                instance->SetData(DATA_HAGARA_HOLDING_HANDS, 1);
            }

            EndSpecialPhase();
        }
    }

    // Breadth-first conduction: charged conductors energize players within 10
    // yards, players relay to other players, and only players can charge an
    // inert conductor (conductors never arc to each other directly)
    void ConduitChainPulse()
    {
        std::vector<Creature*> charged;
        std::vector<Creature*> inert;
        for (ObjectGuid guid : _conductors)
        {
            Creature* conductor = ObjectAccessor::GetCreature(*me, guid);
            if (!conductor)
                continue;
            if (_conductorCharged[guid])
                charged.push_back(conductor);
            else
                inert.push_back(conductor);
        }

        if (charged.empty())
            return;

        std::list<Player*> allPlayers;
        me->GetPlayerListInGrid(allPlayers, 100.0f);
        allPlayers.remove_if([](Player* player) { return !player->IsAlive() || player->IsGameMaster(); });

        std::vector<Player*> connected;
        std::vector<Player*> frontier;
        std::unordered_set<ObjectGuid> visited;

        auto visit = [&](Player* player)
        {
            if (visited.insert(player->GetGUID()).second)
            {
                connected.push_back(player);
                frontier.push_back(player);
            }
        };

        for (Player* player : allPlayers)
            for (Creature* conductor : charged)
                if (player->GetExactDist2d(conductor) <= ConduitChainRange)
                {
                    visit(player);
                    break;
                }

        while (!frontier.empty())
        {
            std::vector<Player*> next = std::move(frontier);
            frontier.clear();
            for (Player* node : next)
                for (Player* other : allPlayers)
                    if (!visited.count(other->GetGUID()) && node->GetExactDist2d(other) <= ConduitChainRange)
                        visit(other);
        }

        // Holding Hands: once the first conductor is charged, the chain must
        // stay connected until the last one closes the circuit
        if (_chainWindow && _chargedCount < GetTuning(instance, me->GetMap()).ConductorCharges && connected.empty())
            _chainIntact = false;

        uint32 const conduitSpellId = sSpellMgr->GetSpellIdForDifficulty(SPELL_LIGHTNING_CONDUIT, me);
        for (Player* player : connected)
            if (Aura* conduit = me->AddAura(conduitSpellId, player))
                conduit->SetDuration(2000);

        for (Creature* conductor : inert)
            for (Player* player : connected)
                if (player->GetExactDist2d(conductor) <= ConduitChainRange)
                {
                    ChargeConductor(conductor);
                    break;
                }
    }

    /* ------------------------------------------------------------------ */
    /* Phase end / cleanup                                                */
    /* ------------------------------------------------------------------ */

    void RemoveAuraFromSelfWithForks(uint32 spellId)
    {
        me->RemoveAurasDueToSpell(sSpellMgr->GetSpellIdForDifficulty(spellId, me));
    }

    void CleanupSpecialPhase()
    {
        _scheduler.CancelGroup(SchedulerGroupIce);
        _scheduler.CancelGroup(SchedulerGroupLightning);
        events.CancelEvent(EVENT_PHASE_WATCHDOG);

        RemoveAuraFromSelfWithForks(SPELL_FROZEN_TEMPEST);
        RemoveAuraFromSelfWithForks(SPELL_WATER_SHIELD);

        summons.DespawnEntry(NPC_ICE_WAVE);
        summons.DespawnEntry(NPC_COLLAPSING_ICICLE);
        summons.DespawnEntry(NPC_FROZEN_BINDING_CRYSTAL);
        summons.DespawnEntry(NPC_BOUND_LIGHTNING_ELEMENTAL);
        summons.DespawnEntry(NPC_CRYSTAL_CONDUCTOR); // heroic extras only - the permanent four are world spawns
        _waveStalkers.clear();
        _waveGrace.clear();

        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_WATERY_ENTRENCHMENT);
        for (uint32 conduitId : { 105369u, 108569u, 109201u, 109202u })
            instance->DoRemoveAurasDueToSpellOnPlayers(conduitId);

        ResetConductors();
        _conductors.clear();
        _conductorCharged.clear();
    }

    void EndSpecialPhase()
    {
        CleanupSpecialPhase();

        me->SetControlled(false, UNIT_STATE_ROOT);
        DoCastSelf(SPELL_FEEDBACK, true);

        _phase = PHASE_FEEDBACK;
        events.SetPhase(PHASE_FEEDBACK);
        events.ScheduleEvent(EVENT_FEEDBACK_END, 15s, 0, PHASE_FEEDBACK);
    }

    void CleanupEncounter()
    {
        CleanupSpecialPhase();
        _scheduler.CancelAll();
        me->SetControlled(false, UNIT_STATE_ROOT);

        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_ICE_TOMB_AURA);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_FROSTFLAKE);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_FROSTFLAKE_SNARE_SLOW);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_ICE_LANCE_MARKER);
    }

    TaskScheduler _scheduler;
    uint8 _phase = PHASE_NONE;
    uint8 _nextSpecial = PHASE_ICE;

    // Ice phase
    uint8 _crystalsDead = 0;
    float _waveAngle = 0.0f;
    float _waveDirection = 1.0f;
    std::vector<ObjectGuid> _waveStalkers;
    std::unordered_map<ObjectGuid, uint32> _waveGrace;

    // Lightning phase
    uint8 _chargedCount = 0;
    bool _chainIntact = true;
    bool _chainWindow = false;
    std::vector<ObjectGuid> _conductors;
    std::unordered_map<ObjectGuid, bool> _conductorCharged;

    // Intro event
    bool _introRunning = false;
    uint8 _introWave = 0;
    uint32 _introAlive = 0;
};

struct npc_hagara_ice_tomb : public ScriptedAI
{
    npc_hagara_ice_tomb(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript())
    {
        SetCombatMovement(false);
        me->SetReactState(REACT_PASSIVE);
    }

    void JustAppeared() override
    {
        HagaraTuning const& tuning = GetTuning(_instance, me->GetMap());
        me->SetMaxHealth(tuning.IceTombHealth);
        me->SetFullHealth();
        DoZoneInCombat();

        // Release the victim if they died or the aura was otherwise removed
        _scheduler.Schedule(1s, [this](TaskContext context)
        {
            Player* victim = ObjectAccessor::GetPlayer(*me, _victimGUID);
            if (!victim || !victim->IsAlive() || !victim->HasAura(SPELL_ICE_TOMB_AURA))
            {
                ReleaseVictim();
                me->DespawnOrUnsummon(500ms);
                return;
            }
            context.Repeat(1s);
        });
    }

    void SetGUID(ObjectGuid const& guid, int32 id) override
    {
        if (id == GUID_TOMB_VICTIM)
            _victimGUID = guid;
    }

    void JustDied(Unit* /*killer*/) override
    {
        ReleaseVictim();
        me->DespawnOrUnsummon(3s);
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

private:
    void ReleaseVictim()
    {
        if (Player* victim = ObjectAccessor::GetPlayer(*me, _victimGUID))
            victim->RemoveAurasDueToSpell(SPELL_ICE_TOMB_AURA);
    }

    InstanceScript* _instance;
    TaskScheduler _scheduler;
    ObjectGuid _victimGUID;
};

struct npc_hagara_ice_lance : public ScriptedAI
{
    npc_hagara_ice_lance(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript())
    {
        SetCombatMovement(false);
        me->SetReactState(REACT_PASSIVE);
    }

    void SetGUID(ObjectGuid const& guid, int32 id) override
    {
        if (id != GUID_LANCE_TARGET)
            return;

        _targetGUID = guid;
        if (Player* target = ObjectAccessor::GetPlayer(*me, _targetGUID))
        {
            me->SetFacingToObject(target);
            me->AddAura(SPELL_ICE_LANCE_MARKER, target);
        }

        // Every 2 seconds a frost projectile flies at the marked player; the
        // first body-blocker along the flight line intercepts it
        _scheduler.Schedule(2s, [this](TaskContext context)
        {
            FireProjectile();
            context.Repeat(2s);
        });
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

private:
    void FireProjectile()
    {
        Player* marked = ObjectAccessor::GetPlayer(*me, _targetGUID);
        if (!marked || !marked->IsAlive())
        {
            me->DespawnOrUnsummon(1s);
            return;
        }

        Unit* target = marked;

        float const lanceToMark = me->GetExactDist2d(marked);
        float bestDist = lanceToMark;

        std::list<Player*> players;
        me->GetPlayerListInGrid(players, lanceToMark + 5.0f);
        for (Player* blocker : players)
        {
            if (blocker == marked || !blocker->IsAlive() || blocker->IsGameMaster() || blocker->HasAura(SPELL_ICE_TOMB_AURA))
                continue;

            float const distFromLance = me->GetExactDist2d(blocker);
            if (distFromLance >= bestDist)
                continue;

            // Distance from the lance->mark line segment
            float const angleToMark = me->GetAngle(marked);
            float const angleToBlocker = me->GetAngle(blocker);
            float const lateral = std::fabs(std::sin(angleToBlocker - angleToMark)) * distFromLance;
            float const forward = std::cos(angleToBlocker - angleToMark) * distFromLance;

            if (forward > 0.0f && lateral <= 2.0f)
            {
                target = blocker;
                bestDist = distFromLance;
            }
        }

        me->CastSpell(target, SPELL_ICE_LANCE_MISSILE, true);
    }

    InstanceScript* _instance;
    TaskScheduler _scheduler;
    ObjectGuid _targetGUID;
};

struct npc_hagara_ice_wave : public ScriptedAI
{
    npc_hagara_ice_wave(Creature* creature) : ScriptedAI(creature)
    {
        SetCombatMovement(false);
        me->SetReactState(REACT_PASSIVE);
    }

    void JustAppeared() override
    {
        DoCastSelf(SPELL_ICE_WAVE_VISUAL, true);
    }

    void UpdateAI(uint32 /*diff*/) override { }
};

struct npc_hagara_binding_crystal : public ScriptedAI
{
    npc_hagara_binding_crystal(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript())
    {
        SetCombatMovement(false);
        me->SetReactState(REACT_PASSIVE);
    }

    void JustAppeared() override
    {
        ApplyLFRHealth(me, _instance, NPC_HAGARA_CRYSTAL_LFR_STATS);
        DoCastSelf(SPELL_SIMPLE_TELEPORT, true);
        DoCastSelf(SPELL_CRYSTALLINE_TETHER_FROST, true);
        DoZoneInCombat();
    }

    void UpdateAI(uint32 /*diff*/) override { }

private:
    InstanceScript* _instance;
};

struct npc_hagara_crystal_conductor : public ScriptedAI
{
    npc_hagara_crystal_conductor(Creature* creature) : ScriptedAI(creature)
    {
        SetCombatMovement(false);
        me->SetReactState(REACT_PASSIVE);
    }

    void DoAction(int32 action) override
    {
        switch (action)
        {
            case ACTION_CONDUCTOR_INERT:
                me->RemoveAurasDueToSpell(SPELL_LIGHTNING_ROD);
                DoCastSelf(SPELL_CRYSTALLINE_TETHER_LIGHTNING, true);
                break;
            case ACTION_CONDUCTOR_CHARGE:
                me->RemoveAurasDueToSpell(SPELL_CRYSTALLINE_TETHER_LIGHTNING);
                DoCastSelf(SPELL_LIGHTNING_ROD, true);
                break;
            case ACTION_CONDUCTOR_RESET:
                me->RemoveAurasDueToSpell(SPELL_CRYSTALLINE_TETHER_LIGHTNING);
                me->RemoveAurasDueToSpell(SPELL_LIGHTNING_ROD);
                break;
            default:
                break;
        }
    }

    void UpdateAI(uint32 /*diff*/) override { }
};

struct npc_hagara_bound_lightning_elemental : public ScriptedAI
{
    npc_hagara_bound_lightning_elemental(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript()) { }

    void JustAppeared() override
    {
        ApplyLFRHealth(me, _instance, NPC_HAGARA_ELEMENTAL_LFR_STATS);
        me->SetReactState(REACT_AGGRESSIVE);
        DoZoneInCombat();
    }

    void DamageDealt(Unit* /*victim*/, uint32& damage, DamageEffectType /*damageType*/) override
    {
        ApplyLFRDamageReduction(_instance, damage);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }

private:
    InstanceScript* _instance;
};

struct npc_hagara_collapsing_icicle : public ScriptedAI
{
    npc_hagara_collapsing_icicle(Creature* creature) : ScriptedAI(creature)
    {
        SetCombatMovement(false);
        me->SetReactState(REACT_PASSIVE);
    }

    void JustAppeared() override
    {
        // 1.7s telegraph (Snowdrift swirl), then the icicle crashes down
        DoCastSelf(SPELL_ICICLE_TELEGRAPH, true);
    }

    void UpdateAI(uint32 /*diff*/) override { }
};

struct npc_hagara_intro_add : public ScriptedAI
{
    npc_hagara_intro_add(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript()) { }

    void JustAppeared() override
    {
        DoZoneInCombat();

        switch (me->GetEntry())
        {
            case NPC_TWILIGHT_FROST_EVOKER:
            case NPC_TWILIGHT_FROST_EVOKER + 1:
                _scheduler.Schedule(2s, 4s, [this](TaskContext context)
                {
                    if (Unit* victim = me->GetVictim())
                        DoCast(victim, SPELL_FROSTBOLT);
                    context.Repeat(3s, 4s);
                });
                _scheduler.Schedule(8s, 12s, [this](TaskContext context)
                {
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 60.0f, true))
                        DoCast(target, SPELL_SHACKLES_OF_ICE);
                    context.Repeat(12s, 16s);
                });
                _scheduler.Schedule(6s, 10s, [this](TaskContext context)
                {
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 60.0f, true))
                        DoCast(target, SPELL_FROST_CORRUPTION);
                    context.Repeat(14s, 18s);
                });
                break;
            case NPC_STORMBORN_MYRMIDON:
            case NPC_STORMBORN_MYRMIDON + 1:
                _scheduler.Schedule(6s, 9s, [this](TaskContext context)
                {
                    if (Unit* victim = me->GetVictim())
                        DoCast(victim, SPELL_CHAIN_LIGHTNING);
                    context.Repeat(8s, 12s);
                });
                _scheduler.Schedule(9s, 13s, [this](TaskContext context)
                {
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 60.0f, true))
                        DoCast(target, SPELL_SPARK);
                    context.Repeat(10s, 14s);
                });
                break;
            case NPC_STORMBINDER_ADEPT:
            case NPC_STORMBINDER_ADEPT + 1:
                _scheduler.Schedule(3s, 6s, [this](TaskContext context)
                {
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 60.0f, true))
                        DoCast(target, SPELL_CHAIN_LIGHTNING_ADEPT);
                    context.Repeat(6s, 9s);
                });
                _scheduler.Schedule(15s, 20s, [this](TaskContext context)
                {
                    // Blink to a random spot on the platform
                    DoCastSelf(SPELL_ADEPT_TELEPORT, true);
                    float const angle = frand(0.0f, 2.0f * float(M_PI));
                    float const radius = frand(10.0f, 30.0f);
                    me->NearTeleportTo({ PlatformCenter.GetPositionX() + radius * std::cos(angle),
                                         PlatformCenter.GetPositionY() + radius * std::sin(angle),
                                         PlatformCenter.GetPositionZ(), angle });
                    context.Repeat(15s, 20s);
                });
                break;
            case NPC_CORRUPTED_FRAGMENT:
            case NPC_CORRUPTED_FRAGMENT + 1:
                // Fixates on a random raid member
                _scheduler.Schedule(1500ms, [this](TaskContext /*context*/)
                {
                    Fixate();
                });
                break;
            default:
                break;
        }
    }

    void DamageDealt(Unit* /*victim*/, uint32& damage, DamageEffectType /*damageType*/) override
    {
        // The intro waves hit noticeably harder on heroic
        if (IsHeroicHagara(me->GetMap()))
            damage = damage * 140 / 100;
        ApplyLFRDamageReduction(_instance, damage);
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);

        if (!UpdateVictim())
            return;

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        if (Unit* victim = me->GetVictim())
            if (_fixateGUID && victim->GetGUID() != _fixateGUID)
                Fixate();

        DoMeleeAttackIfReady();
    }

private:
    void Fixate()
    {
        Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 150.0f, true);
        if (!target)
            return;

        _fixateGUID = target->GetGUID();
        ResetThreatList();
        AddThreat(target, 50000000.0f);
        AttackStart(target);
    }

    InstanceScript* _instance;
    TaskScheduler _scheduler;
    ObjectGuid _fixateGUID;
};

// 107851, 110900, 110899, 110898 - Focused Assault
class spell_hagara_focused_assault : public AuraScript
{
    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        // She strikes from where she stands - the tank outranging the strikes
        // is the intended counterplay
        GetTarget()->SetControlled(true, UNIT_STATE_ROOT);
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        GetTarget()->SetControlled(false, UNIT_STATE_ROOT);
    }

    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        PreventDefaultAction();

        Unit* hagara = GetTarget();
        Unit* victim = hagara->GetVictim();
        if (victim && hagara->IsWithinMeleeRange(victim))
            hagara->CastSpell(victim, SPELL_FOCUSED_ASSAULT_STRIKE, true);
    }

    void Register() override
    {
        AfterEffectApply.Register(&spell_hagara_focused_assault::OnApply, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove.Register(&spell_hagara_focused_assault::OnRemove, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
        OnEffectPeriodic.Register(&spell_hagara_focused_assault::HandlePeriodic, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

// 107850 - Focused Assault (strike)
class spell_hagara_focused_assault_strike : public SpellScript
{
    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        uint32 damage = GetHitDamage();
        ApplyLFRDamageReduction(GetCaster()->GetInstanceScript(), damage);
        SetHitDamage(damage);
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_hagara_focused_assault_strike::HandleDamage, EFFECT_0, SPELL_EFFECT_WEAPON_PERCENT_DAMAGE);
    }
};

// 105289, 108567, 110888, 110887 - Shattered Ice
class spell_hagara_shattered_ice : public SpellScript
{
    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        uint32 damage = GetHitDamage();
        ApplyLFRDamageReduction(GetCaster()->GetInstanceScript(), damage);
        SetHitDamage(damage);
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_hagara_shattered_ice::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 104448 - Ice Tomb (target selection)
class spell_hagara_ice_tomb : public SpellScript
{
    void FilterTargets(std::list<WorldObject*>& targets)
    {
        Unit* caster = GetCaster();
        InstanceScript const* instance = caster->GetInstanceScript();

        Unit* tank = caster->GetVictim();
        targets.remove_if([caster, tank](WorldObject* target)
        {
            Player* player = target->ToPlayer();
            if (!player || !player->IsAlive() || player->IsGameMaster())
                return true;
            if (player == tank)
                return true;
            return player->HasAura(SPELL_ICE_TOMB_AURA);
        });

        if (targets.empty())
            return;

        // Never entomb the last player standing
        uint8 maxTargets = GetTuning(instance, caster->GetMap()).IceTombTargets;
        bool const tankFree = tank && tank->IsAlive() && tank->GetTypeId() == TYPEID_PLAYER;
        if (!tankFree && targets.size() <= maxTargets)
            maxTargets = uint8(targets.size()) - 1;

        if (!maxTargets)
        {
            targets.clear();
            return;
        }

        if (targets.size() > maxTargets)
            Trinity::Containers::RandomResize(targets, maxTargets);
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        caster->AddAura(SPELL_ICE_TOMB_AURA, target);
        if (Creature* tomb = caster->SummonCreature(NPC_ICE_TOMB, target->GetPosition(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 4s))
            tomb->AI()->SetGUID(target->GetGUID(), GUID_TOMB_VICTIM);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect.Register(&spell_hagara_ice_tomb::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        OnEffectHitTarget.Register(&spell_hagara_ice_tomb::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 105316, 107061, 107062, 107063 - Ice Lance (impact)
class spell_hagara_ice_lance : public SpellScript
{
    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        uint32 damage = GetHitDamage();
        ApplyLFRDamageReduction(GetCaster()->GetInstanceScript(), damage);
        SetHitDamage(damage);
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_hagara_ice_lance::HandleDamage, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 109325 - Frostflake
class spell_hagara_frostflake : public SpellScript
{
    void FilterTargets(std::list<WorldObject*>& targets)
    {
        // The boss picks the target; the DBC targeting is a 200yd area
        WorldObject* primary = GetExplTargetUnit();
        targets.clear();
        if (primary)
            targets.push_back(primary);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect.Register(&spell_hagara_frostflake::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
        OnObjectAreaTargetSelect.Register(&spell_hagara_frostflake::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ENEMY);
    }
};

class spell_hagara_frostflake_AuraScript : public AuraScript
{
    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        // The chill deepens every second until it is dispelled
        PreventDefaultAction();
        if (Aura* aura = GetAura())
            if (aura->GetStackAmount() < 20)
                aura->ModStackAmount(1);
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (GetTargetApplication()->GetRemoveMode() == AuraRemoveFlags::ByDeath)
            return;

        // Dispelling (or expiry) freezes the ground under the victim
        Unit* caster = GetCaster();
        Unit* target = GetTarget();
        if (!caster)
            return;

        if (Creature* patch = caster->SummonCreature(NPC_WORLD_TRIGGER, target->GetPosition(), TEMPSUMMON_TIMED_DESPAWN, 25s))
            patch->CastSpell(patch, SPELL_SNOWDRIFT_VISUAL, true);
    }

    void Register() override
    {
        OnEffectPeriodic.Register(&spell_hagara_frostflake_AuraScript::HandlePeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
        AfterEffectRemove.Register(&spell_hagara_frostflake_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_DECREASE_SPEED, AURA_EFFECT_HANDLE_REAL);
    }
};

// 105369, 108569, 109201, 109202 - Lightning Conduit
class spell_hagara_lightning_conduit : public AuraScript
{
    void CalcAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        uint32 damage = uint32(amount);
        ApplyLFRDamageReduction(caster->GetInstanceScript(), damage);
        amount = int32(damage);
    }

    void Register() override
    {
        DoEffectCalcAmount.Register(&spell_hagara_lightning_conduit::CalcAmount, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
    }
};

// 105465, 108568, 110893, 110892 - Lightning Storm
class spell_hagara_lightning_storm : public SpellScript
{
    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        uint32 damage = GetHitDamage();
        ApplyLFRDamageReduction(GetCaster()->GetInstanceScript(), damage);
        SetHitDamage(damage);
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_hagara_lightning_storm::HandleDamage, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 105314 - Ice Wave
class spell_hagara_ice_wave : public SpellScript
{
    void FilterTargets(std::list<WorldObject*>& targets)
    {
        // The boss AI already resolved who the wall swept over
        WorldObject* primary = GetExplTargetUnit();
        targets.clear();
        if (primary)
            targets.push_back(primary);
    }

    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        uint32 damage = GetHitDamage();
        ApplyLFRDamageReduction(GetCaster()->GetInstanceScript(), damage);
        SetHitDamage(damage);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect.Register(&spell_hagara_ice_wave::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
        OnObjectAreaTargetSelect.Register(&spell_hagara_ice_wave::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ENEMY);
        OnEffectHitTarget.Register(&spell_hagara_ice_wave::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 69425 - Collapsing Icicle (impact)
class spell_hagara_icicle : public SpellScript
{
    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        // The DBC values are WotLK-era; damage comes from the tuning table
        Unit* caster = GetCaster();
        InstanceScript const* instance = caster->GetInstanceScript();

        uint32 damage = uint32(GetTuning(instance, caster->GetMap()).IcicleDamage);
        ApplyLFRDamageReduction(instance, damage);
        SetHitDamage(int32(damage));
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_hagara_icicle::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};
}

void AddSC_boss_hagara()
{
    using namespace DragonSoul;
    using namespace DragonSoul::Hagara;

    RegisterDragonSoulCreatureAI(boss_hagara);
    RegisterDragonSoulCreatureAI(npc_hagara_ice_tomb);
    RegisterDragonSoulCreatureAI(npc_hagara_ice_lance);
    RegisterDragonSoulCreatureAI(npc_hagara_ice_wave);
    RegisterDragonSoulCreatureAI(npc_hagara_binding_crystal);
    RegisterDragonSoulCreatureAI(npc_hagara_crystal_conductor);
    RegisterDragonSoulCreatureAI(npc_hagara_bound_lightning_elemental);
    RegisterDragonSoulCreatureAI(npc_hagara_collapsing_icicle);
    RegisterDragonSoulCreatureAI(npc_hagara_intro_add);

    RegisterSpellScript(spell_hagara_focused_assault);
    RegisterSpellScript(spell_hagara_focused_assault_strike);
    RegisterSpellScript(spell_hagara_shattered_ice);
    RegisterSpellScript(spell_hagara_ice_tomb);
    RegisterSpellScript(spell_hagara_ice_lance);
    RegisterSpellAndAuraScriptPair(spell_hagara_frostflake, spell_hagara_frostflake_AuraScript);
    RegisterSpellScript(spell_hagara_lightning_conduit);
    RegisterSpellScript(spell_hagara_lightning_storm);
    RegisterSpellScript(spell_hagara_ice_wave);
    RegisterSpellScript(spell_hagara_icicle);
}
