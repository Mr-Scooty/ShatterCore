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
 * Spine of Deathwing (Dragon Soul, 7th encounter) - LFR / 10N / 25N / 10H / 25H
 *
 * There is no boss health bar: the raid rides Deathwing's back and pries off
 * three armor plates. Deathwing (53879) is a permanently spawned, unattackable
 * controller that owns the barrel-roll balance system, the breach holes and
 * the plate progression; every other unit on the spine is his summon.
 *
 * Core loop: kill a Corruption -> its breach hole opens (Grasping Tendrils
 * anchor + Corrupted Blood spawns) and a Hideous Amalgamation emerges. Bloods
 * cannot die: at 1 HP they Burst and become their own Residue (105223,
 * transform + pacify + full heal) and crawl back to an open hole. An
 * Amalgamation that vacuums 9 residues becomes Superheated; killing it then
 * triggers a Nuclear Blast which pries up the nearest armor plate, exposing
 * the Burning Tendons for a 23s Seal Armor Breach burn window.
 *
 * The whole raid standing on one side of the spine for ~5s arms a barrel
 * roll (5s fuse): everything not anchored by Grasping Tendrils or Fiery Grip
 * is thrown off and killed - the intended disposal tool for unwanted adds.
 *
 * Heroic: Degradation (-5% max HP per real Amalgamation death) and the Blood
 * Corruption dispel minigame (Death jumps on dispel and may mutate into
 * Earth; Earth expiry grants the stacking Blood of Neltharion mitigation).
 * LFR: Corruptions cast neither Searing Plasma nor Fiery Grip.
 */

#include "Containers.h"
#include "Creature.h"
#include "CreatureAIImpl.h"
#include "GameObject.h"
#include "InstanceScript.h"
#include "Map.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "TaskScheduler.h"
#include "TemporarySummon.h"
#include "dragon_soul.h"

namespace DragonSoul::Spine
{
enum SpineTexts
{
    // Deathwing (53879)
    SAY_ROLL_WARNING_LEFT   = 0, // %s feels players on his left side. He's about to roll left!
    SAY_ROLL_WARNING_RIGHT  = 1,
    SAY_ROLL_LEFT           = 2, // %s rolls left!
    SAY_ROLL_RIGHT          = 3,
    SAY_LEVEL_OUT           = 4, // %s levels out.
    SAY_TAUNT_FIRST         = 5, // flight taunt yells (groups 5-9)
    SAY_TAUNT_LAST          = 9,

    // Hideous Amalgamation (53890)
    EMOTE_NUCLEAR_BLAST       = 0, // %s is casting [Nuclear Blast]!
    EMOTE_BLAST_MISSED        = 1, // [Nuclear Blast] wasn't close enough to pry up the plate!
    EMOTE_NOT_ENOUGH_RESIDUE  = 2  // %s didn't absorb enough Corrupted Blood residue ...
};

enum SpineSpells
{
    // Corruption
    SPELL_SEARING_PLASMA_SELECTOR   = 109379, // 8s cast; the script applies the aura
    SPELL_SEARING_PLASMA            = 105479, // heal absorb (forks 109362/109363/109364)
    SPELL_FIERY_GRIP                = 105490, // 30s channel (forks 109457/109458/109459)

    // Breach holes
    SPELL_GRASPING_TENDRILS_FIELD   = 105510, // spawner self aura, pulses the anchor debuff
    SPELL_SUMMON_SLIME              = 104999, // summons 53889 at the caster

    // Corrupted Blood
    SPELL_BURST                     = 105219, // on-"death" nova (forks 109371/109372/109373)
    SPELL_RESIDUE                   = 105223, // transform + pacify + heal to full

    // Hideous Amalgamation
    SPELL_ABSORB_BLOOD_BAR          = 109329, // enable the alt power bar overlay
    SPELL_ZERO_REGEN_90_MAX         = 109121, // energy: no regen, max 90
    SPELL_ABSORB_BLOOD_VACUUM       = 105244, // periodic 105241
    SPELL_ABSORB_BLOOD_CHECK        = 105241,
    SPELL_ABSORBED_BLOOD            = 105248, // stacking buff, 9 = superheated
    SPELL_SUPERHEATED_NUCLEUS       = 105834, // pulses 106264 (forked)
    SPELL_NUCLEAR_BLAST             = 105845, // 5s cast, blast fires on completion
    SPELL_NUCLEAR_BLAST_CHECK       = 105846, // plate seam proximity check
    SPELL_DEGRADATION               = 106005, // heroic, cast by Deathwing

    // Burning Tendons
    SPELL_SEAL_ARMOR_BREACH_RIGHT   = 105847, // 56341
    SPELL_SEAL_ARMOR_BREACH_LEFT    = 105848, // 56575
    SPELL_PLATE_FLY_OFF_LEFT        = 105366,
    SPELL_PLATE_FLY_OFF_RIGHT       = 105384,
    SPELL_SEAL_ARMOR_SLOW_LFR       = 123458, // serverside +50% cast time -> 34.5s

    // Heroic: Blood Corruption
    SPELL_BLOOD_CORRUPTION_DEATH    = 106199,
    SPELL_BLOOD_CORRUPTION_EARTH    = 106200,
    SPELL_BLOOD_OF_DEATHWING        = 106201,
    SPELL_BLOOD_OF_NELTHARION       = 106213,

    // Encounter flow
    SPELL_KILL_CREDIT_MOVIE         = 104574, // movie 75; carries encounter credit + achievements
    SPELL_TELEPORT_TO_DECK          = 108263  // single teleport back to the Skyfire deck
};

uint32 const GraspingTendrilsIds[] = { 105563, 109454, 109455, 109456 };
uint32 const FieryGripIds[]        = { 105490, 109457, 109458, 109459 };
uint32 const SearingPlasmaIds[]    = { 105479, 109362, 109363, 109364 };

enum SpineActions
{
    // controller
    ACTION_START_ENCOUNTER = ACTION_START_SPINE_ENCOUNTER, // Skyfire captain gossip
    ACTION_PLATE_LIFT,          // 105846 found a Burning Tendons in range
    ACTION_PLATE_SEALED,        // a Seal Armor Breach cast completed
    ACTION_AMALGAMATION_BLAST_MISSED,

    // adds
    ACTION_THROWN_BY_ROLL,
    ACTION_HOLE_OPEN,           // spawner: enable the tendrils field
    ACTION_HOLE_CLOSED,
    ACTION_RESIDUE_CONSUMED,    // residue eaten by an Amalgamation
    ACTION_ACTIVATE_TENDONS,
    ACTION_DEACTIVATE_TENDONS,
    ACTION_DIE_SILENT           // twin tendon: die without renotifying
};

enum SpineMiscData
{
    DATA_HOLE_OPEN_FIRST        = 100, // +holeIndex: controller hole state
    DATA_ROLL_SIDE              = 110, // 1 = left (+X), 2 = right (-X)
    DATA_ABSORBED_STACKS        = 111,
    DATA_BLOOD_CORRUPTION_JUMPS = 112
};

enum SpinePoints
{
    POINT_HOLE = 1
};

// ---------------------------------------------------------------------------
// Geometry (sniffed). The spine is static terrain: centerline X = -13855,
// running from the tail (Y -13670) toward the neck (Y -13598), Z 262 -> 272.
// The +X column is Deathwing's LEFT side, -X his RIGHT.
// ---------------------------------------------------------------------------
constexpr float SpineCenterX   = -13855.0f;
constexpr float SpineHalfWidth = 45.0f;
constexpr float SpineMinY      = -13720.0f;
constexpr float SpineMaxY      = -13560.0f;
constexpr float SpineKillZ     = 250.0f;  // below this = fell off the back
constexpr float RollDeadZone   = 2.0f;    // yards around the centerline ignored by the census

Position const DeckLandingPos = SkyfireDeckLandingPos; // 108263 destination
Position const CacheSpawnPos   = { 13437.0f, -12126.5f, 151.21f, 3.1147f };
QuaternionData const CacheRotation = { 0.0f, 0.0f, 0.99991f, 0.01345f };

// Breach hole anchors: [0..3] rear section (initial Corruptions),
// [4..5] exposed with plate 1, [6..7] exposed with plate 2
Position const HolePositions[8] =
{
    { -13841.00f, -13667.00f, 262.08f, 3.1416f }, // 0 L rear
    { -13868.60f, -13667.30f, 261.92f, 0.0f    }, // 1 R rear
    { -13841.00f, -13654.10f, 263.19f, 3.1416f }, // 2 L rear
    { -13868.00f, -13654.10f, 262.86f, 0.0f    }, // 3 R rear
    { -13841.00f, -13635.00f, 265.30f, 3.1416f }, // 4 L plate 1
    { -13868.60f, -13638.30f, 264.80f, 0.0f    }, // 5 R plate 1
    { -13841.00f, -13614.40f, 266.39f, 3.1416f }, // 6 L plate 2
    { -13868.60f, -13614.30f, 266.89f, 0.0f    }  // 7 R plate 2
};
constexpr uint8 NumHoles = 8;
constexpr uint8 HolesPerStage[3] = { 4, 6, 8 }; // reachable holes while plate 0/1/2 is frontmost

Position const TendonsRightPos[3] =
{
    { -13862.80f, -13645.01f, 265.75f, 1.5708f },
    { -13862.60f, -13626.30f, 266.59f, 1.5708f },
    { -13862.60f, -13604.90f, 269.23f, 1.5708f }
};
Position const TendonsLeftPos[3] =
{
    { -13846.80f, -13644.70f, 265.79f, 1.5708f },
    { -13846.60f, -13626.00f, 266.64f, 1.5708f },
    { -13846.60f, -13604.66f, 269.17f, 1.5708f }
};
uint32 const PlateDataIds[3] = { DATA_SPINE_PLATE_1, DATA_SPINE_PLATE_2, DATA_SPINE_PLATE_3 };

// ---------------------------------------------------------------------------
// Tuning (DBM 4.3.4 timers where known, playtest knobs otherwise)
// ---------------------------------------------------------------------------
constexpr Milliseconds AmalgamationSpawnDelay = 4500ms; // DBM: 4.5-5s after the Corruption dies
constexpr Milliseconds PlugRespawnDelay       = 10s;    // "always at least one Corruption"
constexpr Milliseconds BloodCorruptionDelay   = 4s;     // after Amalgamation spawn (DBM: 8.5-10s after Corruption death)
constexpr Milliseconds BloodSpawnFloor        = 5s;
constexpr uint32 RollWarningStreak            = 5;      // one-sided census seconds before the warning
constexpr Milliseconds RollFuse               = 5s;     // DBM roll bar
constexpr Milliseconds RollDuration           = 6s;
constexpr float RollKnockSpeedXY              = 40.0f;
constexpr float RollKnockSpeedZ               = 12.0f;
constexpr float ResidueSpeedRate              = 0.4f;
constexpr float BloodCorruptionMutateBase     = 15.0f;  // % chance, +15% per Death jump

struct SpineTuning
{
    bool  SearingPlasma;
    bool  FieryGrip;
    bool  Heroic;               // Degradation + Blood Corruption
    Milliseconds GripCadence;   // DBM: 32s on 10-player, 16s on 25-player
    Milliseconds PlasmaCadence; // knob - no DBM datum
    Milliseconds BloodSpawnBase;
};

bool IsLFR(InstanceScript const* instance)
{
    return instance && instance->IsLFR();
}

SpineTuning const& GetTuning(InstanceScript const* instance, Map const* map)
{
    static SpineTuning const lfr = { false, false, false, 32s, 20s, 12s };
    static SpineTuning const n10 = { true,  true,  false, 32s, 20s, 10s };
    static SpineTuning const n25 = { true,  true,  false, 16s, 20s, 10s };
    static SpineTuning const h10 = { true,  true,  true,  32s, 20s, 8s  };
    static SpineTuning const h25 = { true,  true,  true,  16s, 20s, 8s  };

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

bool IsOnSpine(WorldObject const* who)
{
    return std::abs(who->GetPositionX() - SpineCenterX) < SpineHalfWidth
        && who->GetPositionY() > SpineMinY && who->GetPositionY() < SpineMaxY
        && who->GetPositionZ() > SpineKillZ;
}

// wider than the spine itself: catches players knocked off and falling
bool IsInSpineKillVolume(Player const* who)
{
    return std::abs(who->GetPositionX() - SpineCenterX) < 300.0f
        && who->GetPositionY() > SpineMinY - 150.0f && who->GetPositionY() < SpineMaxY + 150.0f
        && who->GetPositionZ() < SpineKillZ;
}

bool IsAnyPlayerAliveOnSpine(Map* map)
{
    for (MapReference const& ref : map->GetPlayers())
    {
        Player* player = ref.GetSource();
        if (player && player->IsAlive() && !player->IsGameMaster() && IsOnSpine(player))
            return true;
    }
    return false;
}

bool IsAnchored(Player const* player)
{
    for (uint32 spellId : GraspingTendrilsIds)
        if (player->HasAura(spellId))
            return true;
    for (uint32 spellId : FieryGripIds)
        if (player->HasAura(spellId))
            return true;
    return false;
}

// 53879 - Deathwing: unattackable encounter controller. Permanent spawn at
// the tail; owns holes, rolls, plates, victory and the wipe funnel. Every
// other spine unit is (directly or via a Spawner) his summon.
struct boss_spine_of_deathwing : public ScriptedAI
{
    enum Stage : uint8
    {
        STAGE_IDLE,
        STAGE_ACTIVE,
        STAGE_DONE
    };

    enum TaskGroups : uint32
    {
        GROUP_CHECKS    = 1,
        GROUP_ENCOUNTER = 2,
        GROUP_ROLL      = 3,
        GROUP_HOLE_BASE = 10 // + hole index
    };

    boss_spine_of_deathwing(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript()), _summons(creature) { }

    void Reset() override
    {
        me->SetReactState(REACT_PASSIVE);
        me->setActive(true);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC);

        _scheduler.CancelAll();
        _summons.DespawnAll();
        ResetEncounterState();
        _stage = _instance->GetBossState(DATA_SPINE_OF_DEATHWING) == DONE ? STAGE_DONE : STAGE_IDLE;
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

    void JustSummoned(Creature* summon) override
    {
        _summons.Summon(summon);
    }

    void DoAction(int32 action) override
    {
        switch (action)
        {
            case ACTION_START_ENCOUNTER:
                if (_stage == STAGE_IDLE && _instance->GetBossState(DATA_SPINE_OF_DEATHWING) != DONE
                    && _instance->GetBossState(DATA_SPINE_OF_DEATHWING) != IN_PROGRESS)
                    StartEncounter();
                break;
            case ACTION_PLATE_LIFT:
                if (_stage == STAGE_ACTIVE && !_plateLifted && _plateIndex < 3)
                    LiftPlate();
                break;
            case ACTION_PLATE_SEALED:
                if (_stage == STAGE_ACTIVE && _plateLifted)
                    ResealPlate();
                break;
            default:
                break;
        }
    }

    uint32 GetData(uint32 type) const override
    {
        if (type >= DATA_HOLE_OPEN_FIRST && type < DATA_HOLE_OPEN_FIRST + NumHoles)
            return _holeOpen[type - DATA_HOLE_OPEN_FIRST] ? 1 : 0;
        if (type == DATA_BLOOD_CORRUPTION_JUMPS)
            return _bloodCorruptionJumps;
        return 0;
    }

    void SetData(uint32 type, uint32 value) override
    {
        if (type == DATA_BLOOD_CORRUPTION_JUMPS)
            _bloodCorruptionJumps = value;
    }

    void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
    {
        if (_stage != STAGE_ACTIVE)
            return;

        switch (summon->GetEntry())
        {
            case NPC_SPINE_CORRUPTION:
            case NPC_SPINE_CORRUPTION_PLUG:
            case NPC_SPINE_CORRUPTION_PLATE:
                OnCorruptionDied(summon);
                break;
            case NPC_HIDEOUS_AMALGAMATION:
                // rolled-off Amalgamations despawn without dying, so every
                // death seen here is a real kill (heroic: Degradation)
                if (GetTuning(_instance, me->GetMap()).Heroic)
                    DoCastAOE(SPELL_DEGRADATION, true);
                break;
            case NPC_BURNING_TENDONS_LEFT:
            case NPC_BURNING_TENDONS_RIGHT:
                OnTendonsDied(summon);
                break;
            default:
                break;
        }
    }

private:
    void ResetEncounterState()
    {
        _plateIndex = 0;
        _plateLifted = false;
        _rollInProgress = false;
        _rollWarningArmed = false;
        _sideStreak = 0;
        _streakSide = 0;
        _elapsed = 0ms;
        _corruptionsAlive = 0;
        _bloodCorruptionJumps = 0;
        _plugRespawnPending = false;
        _holePlug.clear();
        for (bool& open : _holeOpen)
            open = false;
        _tendonLeft.Clear();
        _tendonRight.Clear();
    }

    void StartEncounter()
    {
        _stage = STAGE_ACTIVE;
        ResetEncounterState();
        _instance->SetBossState(DATA_SPINE_OF_DEATHWING, IN_PROGRESS);

        for (uint8 i = 0; i < NumHoles; ++i)
            if (Creature* spawner = me->SummonCreature(NPC_SPINE_SPAWNER, HolePositions[i], TEMPSUMMON_MANUAL_DESPAWN))
                _spawner[i] = spawner->GetGUID();

        for (uint8 i = 0; i < 4; ++i)
            SummonPlugCorruption(i, NPC_SPINE_CORRUPTION);

        SummonTendons(0);

        _scheduler.Schedule(1s, GROUP_CHECKS, [this](TaskContext context)
        {
            _elapsed += 1000ms;
            KillSweep();

            if (!IsAnyPlayerAliveOnSpine(me->GetMap()))
            {
                Fail();
                return;
            }

            UpdateRollCensus();
            context.Repeat(1s);
        });

        _scheduler.Schedule(30s, GROUP_ENCOUNTER, [this](TaskContext context)
        {
            Talk(urand(SAY_TAUNT_FIRST, SAY_TAUNT_LAST));
            context.Repeat(randtime(45s, 75s));
        });
    }

    Creature* GetSpawner(uint8 hole) const
    {
        return ObjectAccessor::GetCreature(*me, _spawner[hole]);
    }

    void SummonPlugCorruption(uint8 hole, uint32 entry)
    {
        CloseHole(hole);
        if (Creature* corruption = me->SummonCreature(entry, HolePositions[hole], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 8s))
        {
            _holePlug[corruption->GetGUID()] = hole;
            ++_corruptionsAlive;
        }
    }

    void OpenHole(uint8 hole)
    {
        if (_holeOpen[hole])
            return;
        _holeOpen[hole] = true;

        if (Creature* spawner = GetSpawner(hole))
            spawner->AI()->DoAction(ACTION_HOLE_OPEN);

        _scheduler.Schedule(2s, GROUP_HOLE_BASE + hole, [this, hole](TaskContext context)
        {
            if (Creature* spawner = GetSpawner(hole))
                spawner->CastSpell(spawner, SPELL_SUMMON_SLIME, true);
            context.Repeat(BloodSpawnPeriod());
        });
    }

    void CloseHole(uint8 hole)
    {
        _holeOpen[hole] = false;
        _scheduler.CancelGroup(GROUP_HOLE_BASE + hole);
        if (Creature* spawner = GetSpawner(hole))
            spawner->AI()->DoAction(ACTION_HOLE_CLOSED);
    }

    Milliseconds BloodSpawnPeriod() const
    {
        Milliseconds base = GetTuning(_instance, me->GetMap()).BloodSpawnBase;
        Milliseconds period = base - Milliseconds(1500 * _plateIndex)
            - Milliseconds(300 * (_elapsed.count() / 60000)); // soft enrage: -0.3s per minute
        return std::max(period, BloodSpawnFloor);
    }

    void OnCorruptionDied(Creature* corruption)
    {
        auto itr = _holePlug.find(corruption->GetGUID());
        if (itr == _holePlug.end())
            return;

        uint8 hole = itr->second;
        _holePlug.erase(itr);
        if (_corruptionsAlive > 0)
            --_corruptionsAlive;

        OpenHole(hole);

        Position pos = HolePositions[hole];
        _scheduler.Schedule(AmalgamationSpawnDelay, GROUP_ENCOUNTER, [this, pos](TaskContext /*context*/)
        {
            me->SummonCreature(NPC_HIDEOUS_AMALGAMATION, pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 8s);
        });

        // there is always at least one Corruption plugging a hole
        if (_corruptionsAlive == 0 && !_plugRespawnPending)
        {
            _plugRespawnPending = true;
            _scheduler.Schedule(PlugRespawnDelay, GROUP_ENCOUNTER, [this](TaskContext /*context*/)
            {
                _plugRespawnPending = false;
                if (_corruptionsAlive > 0)
                    return;

                std::vector<uint8> openHoles;
                for (uint8 i = 0; i < HolesPerStage[std::min<uint8>(_plateIndex, 2)]; ++i)
                    if (_holeOpen[i])
                        openHoles.push_back(i);

                if (!openHoles.empty())
                    SummonPlugCorruption(Trinity::Containers::SelectRandomContainerElement(openHoles), NPC_SPINE_CORRUPTION_PLUG);
            });
        }
    }

    // ------------------------------------------------------------------
    // Barrel rolls
    // ------------------------------------------------------------------
    void KillSweep()
    {
        for (MapReference const& ref : me->GetMap()->GetPlayers())
        {
            Player* player = ref.GetSource();
            if (player && player->IsAlive() && !player->IsGameMaster() && IsInSpineKillVolume(player))
                player->KillSelf();
        }
    }

    void UpdateRollCensus()
    {
        if (_rollInProgress)
            return;

        uint32 left = 0, right = 0;
        for (MapReference const& ref : me->GetMap()->GetPlayers())
        {
            Player* player = ref.GetSource();
            if (!player || !player->IsAlive() || player->IsGameMaster() || !IsOnSpine(player))
                continue;
            if (IsAnchored(player))
                continue;

            float dx = player->GetPositionX() - SpineCenterX;
            if (std::abs(dx) < RollDeadZone)
                continue;
            dx > 0.0f ? ++left : ++right;
        }

        uint32 total = left + right;
        int8 side = 0;
        if (total > 0 && left == total)
            side = 1;
        else if (total > 0 && right == total)
            side = -1;

        if (side != 0 && side == _streakSide)
            ++_sideStreak;
        else
        {
            _streakSide = side;
            _sideStreak = side != 0 ? 1 : 0;
            if (side == 0 && _rollWarningArmed)
            {
                // balance restored before the fuse ran out
                _scheduler.CancelGroup(GROUP_ROLL);
                _rollWarningArmed = false;
                Talk(SAY_LEVEL_OUT);
            }
        }

        if (_sideStreak >= RollWarningStreak && !_rollWarningArmed)
        {
            _rollWarningArmed = true;
            int8 rollSide = _streakSide;
            Talk(rollSide > 0 ? SAY_ROLL_WARNING_LEFT : SAY_ROLL_WARNING_RIGHT);
            _scheduler.Schedule(RollFuse, GROUP_ROLL, [this, rollSide](TaskContext /*context*/)
            {
                ExecuteRoll(rollSide);
            });
        }
    }

    void ExecuteRoll(int8 side)
    {
        _rollWarningArmed = false;
        _rollInProgress = true;
        _sideStreak = 0;
        _streakSide = 0;

        Talk(side > 0 ? SAY_ROLL_LEFT : SAY_ROLL_RIGHT);
        _instance->SetData(DATA_SPINE_ROLL_OCCURRED, 1);

        for (MapReference const& ref : me->GetMap()->GetPlayers())
        {
            Player* player = ref.GetSource();
            if (!player || !player->IsAlive() || player->IsGameMaster() || !IsOnSpine(player))
                continue;
            if (IsAnchored(player))
                continue;

            player->KnockbackFrom(SpineCenterX - side * 40.0f, player->GetPositionY(), RollKnockSpeedXY, RollKnockSpeedZ);
        }

        // every Amalgamation and Corrupted Blood (normal or residue) is thrown;
        // Corruptions and Spawners are permanently anchored
        for (ObjectGuid guid : _summons)
        {
            Creature* summon = ObjectAccessor::GetCreature(*me, guid);
            if (!summon || !summon->IsAlive())
                continue;
            if (summon->GetEntry() != NPC_HIDEOUS_AMALGAMATION && summon->GetEntry() != NPC_CORRUPTED_BLOOD)
                continue;

            summon->AI()->SetData(DATA_ROLL_SIDE, side > 0 ? 1 : 2);
            summon->AI()->DoAction(ACTION_THROWN_BY_ROLL);
        }

        _scheduler.Schedule(RollDuration, GROUP_ROLL, [this](TaskContext /*context*/)
        {
            _rollInProgress = false;
            Talk(SAY_LEVEL_OUT);
        });
    }

    // ------------------------------------------------------------------
    // Plates
    // ------------------------------------------------------------------
    void SummonTendons(uint8 plate)
    {
        if (Creature* right = me->SummonCreature(NPC_BURNING_TENDONS_RIGHT, TendonsRightPos[plate], TEMPSUMMON_MANUAL_DESPAWN))
            _tendonRight = right->GetGUID();
        if (Creature* left = me->SummonCreature(NPC_BURNING_TENDONS_LEFT, TendonsLeftPos[plate], TEMPSUMMON_MANUAL_DESPAWN))
            _tendonLeft = left->GetGUID();
    }

    Creature* GetTendon(bool left) const
    {
        return ObjectAccessor::GetCreature(*me, left ? _tendonLeft : _tendonRight);
    }

    GameObject* GetPlate(uint8 plate) const
    {
        return plate < 3 ? _instance->GetGameObject(PlateDataIds[plate]) : nullptr;
    }

    void LiftPlate()
    {
        _plateLifted = true;
        for (bool left : { true, false })
            if (Creature* tendon = GetTendon(left))
                tendon->AI()->DoAction(ACTION_ACTIVATE_TENDONS);

        if (GameObject* plate = GetPlate(_plateIndex))
            plate->SetGoState(GO_STATE_ACTIVE);
    }

    void ResealPlate()
    {
        _plateLifted = false;
        for (bool left : { true, false })
            if (Creature* tendon = GetTendon(left))
                tendon->AI()->DoAction(ACTION_DEACTIVATE_TENDONS);

        if (GameObject* plate = GetPlate(_plateIndex))
            plate->SetGoState(GO_STATE_READY);
    }

    void OnTendonsDied(Creature* tendon)
    {
        // the twin is killed silently below with _plateLifted already false
        if (!_plateLifted)
            return;
        _plateLifted = false;

        uint8 plate = _plateIndex;
        bool diedLeft = tendon->GetEntry() == NPC_BURNING_TENDONS_LEFT;
        tendon->CastSpell(tendon, diedLeft ? SPELL_PLATE_FLY_OFF_LEFT : SPELL_PLATE_FLY_OFF_RIGHT, true);

        if (Creature* twin = GetTendon(!diedLeft))
            if (twin->IsAlive())
                twin->AI()->DoAction(ACTION_DIE_SILENT);

        if (GameObject* plateGo = GetPlate(plate))
            plateGo->DespawnOrUnsummon();

        ++_plateIndex;
        if (_plateIndex >= 3)
        {
            Victory();
            return;
        }

        SummonTendons(_plateIndex);

        // two fresh Corruptions plug the newly exposed section
        uint8 firstHole = 2 + 2 * _plateIndex;
        SummonPlugCorruption(firstHole, NPC_SPINE_CORRUPTION_PLATE);
        SummonPlugCorruption(firstHole + 1, NPC_SPINE_CORRUPTION_PLATE);
    }

    // ------------------------------------------------------------------
    // Encounter end
    // ------------------------------------------------------------------
    void Victory()
    {
        _stage = STAGE_DONE;
        _scheduler.CancelAll();

        _instance->SetBossState(DATA_SPINE_OF_DEATHWING, DONE);

        // movie + dungeon-encounter credit + achievement criteria all ride 104574
        for (MapReference const& ref : me->GetMap()->GetPlayers())
            if (Player* player = ref.GetSource())
                me->CastSpell(player, SPELL_KILL_CREDIT_MOVIE, true);

        // nothing in the core fires CAST_SPELL encounter credits on its own -
        // complete DungeonEncounter 1291 (and its LFG hook) explicitly
        _instance->UpdateEncounterStateForSpellCast(SPELL_KILL_CREDIT_MOVIE, me);

        _scheduler.Schedule(15s, [this](TaskContext /*context*/)
        {
            for (MapReference const& ref : me->GetMap()->GetPlayers())
            {
                Player* player = ref.GetSource();
                if (!player || player->IsGameMaster())
                    continue;
                if (player->IsAlive())
                    player->CastSpell(player, SPELL_TELEPORT_TO_DECK, true);
                else
                    player->NearTeleportTo(DeckLandingPos);
            }
            _summons.DespawnAll();
        });

        _scheduler.Schedule(18s, [this](TaskContext /*context*/)
        {
            uint32 entry = me->GetMap()->Is25ManRaid() ? GO_GREATER_CACHE_OF_THE_ASPECTS : GO_LESSER_CACHE_OF_THE_ASPECTS;
            if (GameObject* cache = me->SummonGameObject(entry, CacheSpawnPos, CacheRotation, WEEK, GO_SUMMON_TIMED_DESPAWN))
            {
                if (IsLFR(_instance))
                    cache->SetLootMode(LOOT_MODE_HARD_MODE_1);
                else if (me->GetMap()->IsHeroic())
                    cache->SetLootMode(LOOT_MODE_HARD_MODE_2);
            }
        });
    }

    void Fail()
    {
        if (_stage != STAGE_ACTIVE)
            return;
        _stage = STAGE_IDLE;

        _scheduler.CancelAll();
        _instance->SetBossState(DATA_SPINE_OF_DEATHWING, FAIL);

        // stragglers (e.g. still falling) sail home; the dead release normally
        for (MapReference const& ref : me->GetMap()->GetPlayers())
        {
            Player* player = ref.GetSource();
            if (player && player->IsAlive() && !player->IsGameMaster()
                && (IsOnSpine(player) || IsInSpineKillVolume(player)))
                player->NearTeleportTo(DeckLandingPos);
        }

        _summons.DespawnAll();

        // restore any ripped/lifted plates
        for (uint8 i = 0; i < 3; ++i)
        {
            if (GameObject* plate = GetPlate(i))
            {
                if (!plate->isSpawned())
                    plate->Respawn();
                plate->SetGoState(GO_STATE_READY);
            }
        }

        ResetEncounterState();
    }

    InstanceScript* _instance;
    SummonList _summons;
    TaskScheduler _scheduler;
    Stage _stage = STAGE_IDLE;

    uint8 _plateIndex = 0;
    bool _plateLifted = false;
    ObjectGuid _tendonLeft;
    ObjectGuid _tendonRight;

    bool _holeOpen[NumHoles] = { };
    ObjectGuid _spawner[NumHoles];
    std::unordered_map<ObjectGuid, uint8> _holePlug;
    uint32 _corruptionsAlive = 0;
    bool _plugRespawnPending = false;

    bool _rollInProgress = false;
    bool _rollWarningArmed = false;
    uint32 _sideStreak = 0;
    int8 _streakSide = 0;

    Milliseconds _elapsed = 0ms;
    uint32 _bloodCorruptionJumps = 0;
};

// 53888 - Spawner: invisible anchor trigger on every breach hole. While its
// hole is open it pulses the Grasping Tendrils anchor debuff (105510 native
// 500ms tick) and acts as the spawn point for Corrupted Blood.
struct npc_ds_spine_spawner : public ScriptedAI
{
    npc_ds_spine_spawner(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript()) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void JustSummoned(Creature* summon) override
    {
        // Corrupted Bloods summoned via 104999 belong to the controller
        if (Creature* deathwing = _instance->GetCreature(DATA_SPINE_OF_DEATHWING))
            deathwing->AI()->JustSummoned(summon);
    }

    void DoAction(int32 action) override
    {
        if (action == ACTION_HOLE_OPEN)
            DoCastSelf(SPELL_GRASPING_TENDRILS_FIELD, true);
        else if (action == ACTION_HOLE_CLOSED)
            me->RemoveAurasDueToSpell(SPELL_GRASPING_TENDRILS_FIELD);
    }

    void UpdateAI(uint32 /*diff*/) override { }

private:
    InstanceScript* _instance;
};

// 53891 / 56161 / 56162 - Corruption: permanently anchored tentacle.
// Searing Plasma (8s cast, healing absorb) and Fiery Grip (30s channel,
// broken by dealing 20% of its max health) - both disabled on LFR.
struct npc_ds_spine_corruption : public ScriptedAI
{
    npc_ds_spine_corruption(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript()) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        ApplyLFRHealth(me, _instance, NPC_SPINE_CORRUPTION_LFR_STATS);
        me->SetControlled(true, UNIT_STATE_ROOT);
        DoZoneInCombat(me);

        SpineTuning const& tuning = GetTuning(_instance, me->GetMap());

        if (tuning.SearingPlasma)
        {
            _scheduler.Schedule(Milliseconds(urand(6000, 15000)), [this, &tuning](TaskContext context)
            {
                if (me->HasUnitState(UNIT_STATE_CASTING))
                {
                    context.Repeat(2s);
                    return;
                }
                DoCastAOE(SPELL_SEARING_PLASMA_SELECTOR);
                context.Repeat(tuning.PlasmaCadence);
            });
        }

        if (tuning.FieryGrip)
        {
            Milliseconds cadence = tuning.GripCadence;
            _scheduler.Schedule(Milliseconds(urand(uint32(cadence.count() * 6 / 10), uint32(cadence.count()))), [this, cadence](TaskContext context)
            {
                if (me->HasUnitState(UNIT_STATE_CASTING))
                {
                    context.Repeat(2s);
                    return;
                }
                _gripDamage = 0;
                DoCastAOE(SPELL_FIERY_GRIP);
                context.Repeat(cadence);
            });
        }
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage) override
    {
        // Fiery Grip breaks once the tentacle has taken 20% of its max
        // health during the channel
        if (Spell const* channel = me->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
        {
            if (IsGripSpell(channel->GetSpellInfo()->Id))
            {
                _gripDamage += damage;
                if (_gripDamage >= me->CountPctFromMaxHealth(20))
                {
                    me->InterruptNonMeleeSpells(true);
                    RemoveGripAuras();
                }
            }
        }
    }

    void JustDied(Unit* /*killer*/) override
    {
        RemoveGripAuras();
    }

    void DamageDealt(Unit* /*victim*/, uint32& damage, DamageEffectType /*damageType*/) override
    {
        ApplyLFRDamageReduction(_instance, damage);
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);

        if (!UpdateVictim())
            return;
        DoMeleeAttackIfReady();
    }

private:
    static bool IsGripSpell(uint32 spellId)
    {
        for (uint32 gripId : FieryGripIds)
            if (spellId == gripId)
                return true;
        return false;
    }

    void RemoveGripAuras()
    {
        for (uint32 gripId : FieryGripIds)
            me->RemoveOwnedAura(gripId);
    }

    InstanceScript* _instance;
    TaskScheduler _scheduler;
    uint32 _gripDamage = 0;
};

// 53889 - Corrupted Blood: can never truly die. At 1 HP it Bursts and turns
// into its own residue (105223: invisible transform + pacify + full heal),
// then crawls back to the nearest open hole and reconstitutes. Amalgamations
// vacuum residues for Absorbed Blood stacks.
struct npc_ds_corrupted_blood : public ScriptedAI
{
    npc_ds_corrupted_blood(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript()) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        ApplyLFRHealth(me, _instance, NPC_SPINE_BLOOD_LFR_STATS);
        DoZoneInCombat(me);
        if (Player* target = me->SelectNearestPlayer(100.0f))
            AttackStart(target);
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage) override
    {
        if (_residue || _thrown)
        {
            damage = 0;
            return;
        }

        if (damage >= me->GetHealth())
        {
            damage = 0;
            BecomeResidue();
        }
    }

    void DamageDealt(Unit* /*victim*/, uint32& damage, DamageEffectType /*damageType*/) override
    {
        ApplyLFRDamageReduction(_instance, damage);
    }

    void SetData(uint32 type, uint32 value) override
    {
        if (type == DATA_ROLL_SIDE)
            _rollSide = value == 1 ? 1 : -1;
    }

    void DoAction(int32 action) override
    {
        switch (action)
        {
            case ACTION_RESIDUE_CONSUMED:
                if (_residue && !_thrown)
                {
                    _thrown = true; // no further interactions
                    me->DespawnOrUnsummon(300ms);
                }
                break;
            case ACTION_THROWN_BY_ROLL:
            {
                _thrown = true;
                me->InterruptNonMeleeSpells(true);
                me->CombatStop(true);
                Position dest = { SpineCenterX + _rollSide * 70.0f, me->GetPositionY(), me->GetPositionZ() - 15.0f, 0.0f };
                me->GetMotionMaster()->MoveJump(dest, 25.0f, 12.0f);
                me->DespawnOrUnsummon(1500ms);
                break;
            }
            default:
                break;
        }
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != POINT_MOTION_TYPE || pointId != POINT_HOLE || !_residue || _thrown)
            return;

        if (_targetHole >= 0 && HoleOpen(_targetHole))
            Reconstitute();
        else
            MoveToOpenHole();
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);

        if (_residue || _thrown)
            return;

        if (!UpdateVictim())
            return;
        DoMeleeAttackIfReady();
    }

private:
    bool HoleOpen(int8 hole) const
    {
        if (hole < 0)
            return false;
        Creature* deathwing = _instance->GetCreature(DATA_SPINE_OF_DEATHWING);
        return deathwing && deathwing->AI()->GetData(DATA_HOLE_OPEN_FIRST + uint8(hole));
    }

    void BecomeResidue()
    {
        _residue = true;
        me->InterruptNonMeleeSpells(false);
        DoCastSelf(SPELL_BURST, true);
        DoCastSelf(SPELL_RESIDUE, true);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
        me->CombatStop(true);
        me->SetReactState(REACT_PASSIVE);
        me->SetSpeedRate(MOVE_RUN, ResidueSpeedRate);
        me->SetSpeedRate(MOVE_WALK, ResidueSpeedRate);
        MoveToOpenHole();

        // holes may open/close while crawling - revalidate the path
        _scheduler.Schedule(2s, [this](TaskContext context)
        {
            if (!_residue || _thrown)
                return;
            if (_targetHole < 0 || !HoleOpen(_targetHole))
                MoveToOpenHole();
            context.Repeat(2s);
        });
    }

    void MoveToOpenHole()
    {
        _targetHole = -1;
        float best = std::numeric_limits<float>::max();
        for (uint8 i = 0; i < NumHoles; ++i)
        {
            if (!HoleOpen(i))
                continue;
            float dist = me->GetExactDist2d(HolePositions[i].GetPositionX(), HolePositions[i].GetPositionY());
            if (dist < best)
            {
                best = dist;
                _targetHole = i;
            }
        }

        if (_targetHole >= 0)
            me->GetMotionMaster()->MovePoint(POINT_HOLE, HolePositions[_targetHole]);
        else
            me->StopMoving(); // no open hole: idle until one opens
    }

    void Reconstitute()
    {
        _residue = false;
        _targetHole = -1;
        me->RemoveAurasDueToSpell(SPELL_RESIDUE);
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
        me->SetSpeedRate(MOVE_RUN, 1.0f);
        me->SetSpeedRate(MOVE_WALK, 1.0f);
        me->SetReactState(REACT_AGGRESSIVE);
        DoZoneInCombat(me);
        if (Player* target = me->SelectNearestPlayer(100.0f))
            AttackStart(target);
    }

    InstanceScript* _instance;
    TaskScheduler _scheduler;
    bool _residue = false;
    bool _thrown = false;
    int8 _targetHole = -1;
    int8 _rollSide = 1;
};

// 53890 - Hideous Amalgamation: vacuums Corrupted Blood residue (105244 ->
// 105241) for Absorbed Blood stacks; at 9 it becomes Superheated. Killing it
// superheated triggers the Nuclear Blast plate check. Heroic: casts Blood
// Corruption: Death shortly after emerging.
struct npc_ds_hideous_amalgamation : public ScriptedAI
{
    npc_ds_hideous_amalgamation(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript()) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        ApplyLFRHealth(me, _instance, NPC_SPINE_AMALGAMATION_LFR_STATS);

        me->SetPowerType(POWER_ENERGY);
        me->SetMaxPower(POWER_ENERGY, 90);
        me->SetPower(POWER_ENERGY, 0);
        DoCastSelf(SPELL_ABSORB_BLOOD_BAR, true);
        DoCastSelf(SPELL_ZERO_REGEN_90_MAX, true);
        DoCastSelf(SPELL_ABSORB_BLOOD_VACUUM, true);

        DoZoneInCombat(me);
        if (Player* target = me->SelectNearestPlayer(100.0f))
            AttackStart(target);

        if (GetTuning(_instance, me->GetMap()).Heroic)
        {
            _scheduler.Schedule(BloodCorruptionDelay, [this](TaskContext /*context*/)
            {
                CastBloodCorruption();
            });
        }
    }

    void SetData(uint32 type, uint32 value) override
    {
        if (type == DATA_ROLL_SIDE)
        {
            _rollSide = value == 1 ? 1 : -1;
            return;
        }

        if (type != DATA_ABSORBED_STACKS)
            return;

        me->SetPower(POWER_ENERGY, int32(value) * 10);
        if (value >= 9 && !_superheated)
        {
            _superheated = true;
            me->RemoveAurasDueToSpell(SPELL_ABSORB_BLOOD_VACUUM);
            DoCastSelf(SPELL_SUPERHEATED_NUCLEUS, true);
        }
    }

    uint32 GetData(uint32 type) const override
    {
        if (type == DATA_ABSORBED_STACKS)
            return _superheated ? 9 : me->GetPower(POWER_ENERGY) / 10;
        return 0;
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage) override
    {
        if (_thrown)
        {
            damage = 0;
            return;
        }

        if (_detonating)
        {
            // stays at 1 HP until the blast resolves
            if (damage >= me->GetHealth())
                damage = me->GetHealth() > 1 ? uint32(me->GetHealth() - 1) : 0;
            return;
        }

        if (damage >= me->GetHealth() && _superheated)
        {
            damage = 0;
            BeginDetonation();
        }
    }

    void DamageDealt(Unit* /*victim*/, uint32& damage, DamageEffectType /*damageType*/) override
    {
        ApplyLFRDamageReduction(_instance, damage);
    }

    void DoAction(int32 action) override
    {
        switch (action)
        {
            case ACTION_THROWN_BY_ROLL:
            {
                // no Nuclear Blast, no Degradation - the roll disposed of it
                _thrown = true;
                me->InterruptNonMeleeSpells(true);
                me->CombatStop(true);
                Position dest = { SpineCenterX + _rollSide * 70.0f, me->GetPositionY(), me->GetPositionZ() - 15.0f, 0.0f };
                me->GetMotionMaster()->MoveJump(dest, 25.0f, 12.0f);
                me->DespawnOrUnsummon(1500ms);
                break;
            }
            case ACTION_AMALGAMATION_BLAST_MISSED:
                Talk(EMOTE_BLAST_MISSED);
                break;
            default:
                break;
        }
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (_thrown)
            return;
        if (!_superheated)
            Talk(EMOTE_NOT_ENOUGH_RESIDUE);
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);

        if (_thrown || _detonating)
            return;

        if (!UpdateVictim())
            return;
        DoMeleeAttackIfReady();
    }

private:
    void CastBloodCorruption()
    {
        std::vector<Player*> candidates;
        for (MapReference const& ref : me->GetMap()->GetPlayers())
        {
            Player* player = ref.GetSource();
            if (!player || !player->IsAlive() || player->IsGameMaster() || !IsOnSpine(player))
                continue;
            if (player->HasAura(SPELL_BLOOD_CORRUPTION_DEATH) || player->HasAura(SPELL_BLOOD_CORRUPTION_EARTH))
                continue;
            candidates.push_back(player);
        }

        if (!candidates.empty())
            me->CastSpell(Trinity::Containers::SelectRandomContainerElement(candidates), SPELL_BLOOD_CORRUPTION_DEATH, true);
    }

    void BeginDetonation()
    {
        _detonating = true;
        me->SetHealth(1);
        me->AttackStop();
        me->SetReactState(REACT_PASSIVE);
        me->StopMoving();
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);

        Talk(EMOTE_NUCLEAR_BLAST);
        me->CastSpell(nullptr, SPELL_NUCLEAR_BLAST, false);

        // blast damage + plate check ride the cast; death follows shortly after
        _scheduler.Schedule(5600ms, [this](TaskContext /*context*/)
        {
            if (!_thrown && me->IsAlive())
                me->KillSelf();
        });
    }

    InstanceScript* _instance;
    TaskScheduler _scheduler;
    bool _superheated = false;
    bool _detonating = false;
    bool _thrown = false;
    int8 _rollSide = 1;
};

// 56341 / 56575 - Burning Tendons: exposed while a Nuclear Blast holds the
// current plate up. The raid must kill one half before Seal Armor Breach
// completes; health resets whenever the plate reseals (retail 4.3.4).
struct npc_ds_burning_tendons : public ScriptedAI
{
    npc_ds_burning_tendons(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript()) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        ApplyLFRHealth(me, _instance, NPC_SPINE_TENDONS_LFR_STATS);
        me->SetReactState(REACT_PASSIVE);
        me->SetControlled(true, UNIT_STATE_ROOT);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage) override
    {
        if (!_active)
            damage = 0;
    }

    void DoAction(int32 action) override
    {
        switch (action)
        {
            case ACTION_ACTIVATE_TENDONS:
                if (_active)
                    break;
                _active = true;
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                DoZoneInCombat(me);
                if (IsLFR(_instance))
                    DoCastSelf(SPELL_SEAL_ARMOR_SLOW_LFR, true); // 23s -> 34.5s
                me->CastSpell(me, me->GetEntry() == NPC_BURNING_TENDONS_RIGHT
                    ? SPELL_SEAL_ARMOR_BREACH_RIGHT : SPELL_SEAL_ARMOR_BREACH_LEFT, false);
                break;
            case ACTION_DEACTIVATE_TENDONS:
                _active = false;
                me->InterruptNonMeleeSpells(true);
                me->RemoveAurasDueToSpell(SPELL_SEAL_ARMOR_SLOW_LFR);
                me->SetFullHealth();
                me->CombatStop(true);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                break;
            case ACTION_DIE_SILENT:
                if (me->IsAlive())
                    me->KillSelf();
                break;
            default:
                break;
        }
    }

    void UpdateAI(uint32 /*diff*/) override { }

private:
    InstanceScript* _instance;
    bool _active = false;
};

// 109379 - Searing Plasma (selector): 8s cast; applies the healing-absorb
// aura (basepoints = 105479, auto-forked) to 1 (10-player) or 3 (25-player)
// random raiders, preferring targets without it.
class spell_ds_spine_searing_plasma : public SpellScript
{
    void FilterTargets(std::list<WorldObject*>& targets)
    {
        targets.remove_if([](WorldObject* obj) { return obj->GetTypeId() != TYPEID_PLAYER; });

        std::list<WorldObject*> preferred = targets;
        preferred.remove_if([](WorldObject* obj)
        {
            for (uint32 spellId : SearingPlasmaIds)
                if (obj->ToPlayer()->HasAura(spellId))
                    return true;
            return false;
        });
        if (!preferred.empty())
            targets = preferred;

        uint8 count = GetCaster()->GetMap()->Is25ManRaid() ? 3 : 1;
        if (targets.size() > count)
            Trinity::Containers::RandomResize(targets, count);
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        GetCaster()->CastSpell(GetHitUnit(), uint32(GetEffectValue()), true);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect.Register(&spell_ds_spine_searing_plasma::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        OnEffectHitTarget.Register(&spell_ds_spine_searing_plasma::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 105490 / 109457 / 109458 / 109459 - Fiery Grip: the DBC carries no target
// cap; retail grips 1 player on 10-player and up to 3 on 25-player.
class spell_ds_spine_fiery_grip : public SpellScript
{
    void FilterTargets(std::list<WorldObject*>& targets)
    {
        targets.remove_if([](WorldObject* obj)
        {
            Player* player = obj->ToPlayer();
            if (!player)
                return true;
            for (uint32 spellId : FieryGripIds)
                if (player->HasAura(spellId))
                    return true;
            return false;
        });

        uint8 count = GetCaster()->GetMap()->Is25ManRaid() ? 3 : 1;
        if (targets.size() > count)
            Trinity::Containers::RandomResize(targets, count);

        _targets = targets;
    }

    void ReuseTargets(std::list<WorldObject*>& targets)
    {
        targets = _targets;
    }

    void Register() override
    {
        OnObjectAreaTargetSelect.Register(&spell_ds_spine_fiery_grip::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        OnObjectAreaTargetSelect.Register(&spell_ds_spine_fiery_grip::ReuseTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
    }

    std::list<WorldObject*> _targets;
};

// 105241 - Absorb Blood: the Amalgamation's residue vacuum. Each pulse
// consumes nearby residues for one Absorbed Blood stack each (cap 9).
class spell_ds_spine_absorb_blood : public SpellScript
{
    void FilterTargets(std::list<WorldObject*>& targets)
    {
        Unit* caster = GetCaster();
        if (!caster || caster->GetAuraCount(SPELL_ABSORBED_BLOOD) >= 9)
        {
            targets.clear();
            return;
        }

        targets.remove_if([](WorldObject* obj)
        {
            Creature* creature = obj->ToCreature();
            return !creature || !creature->IsAlive() || !creature->HasAura(SPELL_RESIDUE);
        });
    }

    void HandleConsume(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Creature* residue = GetHitCreature();
        if (!caster || !residue)
            return;

        if (caster->GetAuraCount(SPELL_ABSORBED_BLOOD) >= 9)
            return;

        caster->CastSpell(caster, SPELL_ABSORBED_BLOOD, true);
        residue->AI()->DoAction(ACTION_RESIDUE_CONSUMED);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect.Register(&spell_ds_spine_absorb_blood::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        OnEffectHitTarget.Register(&spell_ds_spine_absorb_blood::HandleConsume, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 105248 - Absorbed Blood: keeps the Amalgamation's blood bar and the
// 9-stack Superheated Nucleus trigger in sync with the stack count.
class spell_ds_spine_absorbed_blood : public AuraScript
{
    void OnStackChange(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Creature* owner = GetTarget()->ToCreature())
            if (owner->IsAIEnabled())
                owner->AI()->SetData(DATA_ABSORBED_STACKS, GetStackAmount());
    }

    void Register() override
    {
        AfterEffectApply.Register(&spell_ds_spine_absorbed_blood::OnStackChange, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
    }
};

// 105845 - Nuclear Blast: on cast completion the blast damage fires natively;
// chain the plate seam check.
class spell_ds_spine_nuclear_blast : public SpellScript
{
    void HandleLaunch(SpellEffIndex /*effIndex*/)
    {
        GetCaster()->CastSpell(nullptr, SPELL_NUCLEAR_BLAST_CHECK, true);
    }

    void Register() override
    {
        OnEffectLaunch.Register(&spell_ds_spine_nuclear_blast::HandleLaunch, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 105846 - Nuclear Blast (seam check): did the explosion happen close enough
// to the frontmost plate? Only the current plate's Tendons exist, so any hit
// lifts the right plate.
class spell_ds_spine_nuclear_blast_check : public SpellScript
{
    void FilterTargets(std::list<WorldObject*>& targets)
    {
        targets.remove_if([](WorldObject* obj)
        {
            Creature* creature = obj->ToCreature();
            return !creature || !creature->IsAlive();
        });

        if (targets.size() > 1)
        {
            WorldObject* caster = GetCaster();
            targets.sort([caster](WorldObject* a, WorldObject* b)
            {
                return caster->GetExactDist2d(a) < caster->GetExactDist2d(b);
            });
            targets.resize(1);
        }

        _found = !targets.empty();
    }

    void HandleScript(SpellEffIndex /*effIndex*/)
    {
        if (InstanceScript* instance = GetCaster()->GetInstanceScript())
            if (Creature* deathwing = instance->GetCreature(DATA_SPINE_OF_DEATHWING))
                deathwing->AI()->DoAction(ACTION_PLATE_LIFT);
    }

    void HandleAfterCast()
    {
        if (!_found)
            if (Creature* caster = GetCaster()->ToCreature())
                if (caster->IsAIEnabled())
                    caster->AI()->DoAction(ACTION_AMALGAMATION_BLAST_MISSED);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect.Register(&spell_ds_spine_nuclear_blast_check::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        OnEffectHitTarget.Register(&spell_ds_spine_nuclear_blast_check::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        AfterCast.Register(&spell_ds_spine_nuclear_blast_check::HandleAfterCast);
    }

    bool _found = false;
};

// 105847 / 105848 - Seal Armor Breach: cast completion slams the plate shut.
class spell_ds_spine_seal_armor_breach : public SpellScript
{
    void HandleSealed(SpellEffIndex /*effIndex*/)
    {
        if (InstanceScript* instance = GetCaster()->GetInstanceScript())
            if (Creature* deathwing = instance->GetCreature(DATA_SPINE_OF_DEATHWING))
                deathwing->AI()->DoAction(ACTION_PLATE_SEALED);
    }

    void Register() override
    {
        OnEffectLaunch.Register(&spell_ds_spine_seal_armor_breach::HandleSealed, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

// 106199 / 106200 - Blood Corruption: Death / Earth (heroic). Dispelling
// makes it jump to a new raider - Death mutates into Earth with a chance
// that grows per jump. Natural expiry: Death nukes the raid (Blood of
// Deathwing), Earth pays out the stacking Blood of Neltharion mitigation.
class spell_ds_spine_blood_corruption : public AuraScript
{
    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* owner = GetTarget();
        InstanceScript* instance = owner->GetInstanceScript();
        if (!instance)
            return;

        if (GetTargetApplication()->GetRemoveMode() == AuraRemoveFlags::Expired)
        {
            if (GetId() == SPELL_BLOOD_CORRUPTION_DEATH)
                owner->CastSpell(owner, SPELL_BLOOD_OF_DEATHWING, true);
            else
                owner->CastSpell(owner, SPELL_BLOOD_OF_NELTHARION, true);
            return;
        }

        if (GetTargetApplication()->GetRemoveMode() != AuraRemoveFlags::ByEnemySpell)
            return;

        // dispelled: jump to a new target
        if (instance->GetBossState(DATA_SPINE_OF_DEATHWING) != IN_PROGRESS)
            return;

        Creature* deathwing = instance->GetCreature(DATA_SPINE_OF_DEATHWING);
        if (!deathwing)
            return;

        std::vector<Player*> candidates;
        for (MapReference const& ref : owner->GetMap()->GetPlayers())
        {
            Player* player = ref.GetSource();
            if (!player || !player->IsAlive() || player->IsGameMaster() || player == owner || !IsOnSpine(player))
                continue;
            if (player->HasAura(SPELL_BLOOD_CORRUPTION_DEATH) || player->HasAura(SPELL_BLOOD_CORRUPTION_EARTH))
                continue;
            candidates.push_back(player);
        }
        if (candidates.empty())
            return;

        Player* next = Trinity::Containers::SelectRandomContainerElement(candidates);
        uint32 spellId = SPELL_BLOOD_CORRUPTION_EARTH;
        if (GetId() == SPELL_BLOOD_CORRUPTION_DEATH)
        {
            uint32 jumps = deathwing->AI()->GetData(DATA_BLOOD_CORRUPTION_JUMPS);
            if (roll_chance_f(BloodCorruptionMutateBase + BloodCorruptionMutateBase * jumps))
                deathwing->AI()->SetData(DATA_BLOOD_CORRUPTION_JUMPS, 0);
            else
            {
                spellId = SPELL_BLOOD_CORRUPTION_DEATH;
                deathwing->AI()->SetData(DATA_BLOOD_CORRUPTION_JUMPS, jumps + 1);
            }
        }

        deathwing->CastSpell(next, spellId, true);
    }

    void Register() override
    {
        AfterEffectRemove.Register(&spell_ds_spine_blood_corruption::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 104574 - Play Movie - Deathwing 3: victory. The dungeon-encounter credit
// (instance_encounters 1291) and the kill/Dizzy achievement criteria all key
// on this spell hitting the raid.
class spell_ds_spine_kill_credit : public SpellScript
{
    void HandleMovie(SpellEffIndex /*effIndex*/)
    {
        if (Player* player = GetHitPlayer())
            player->SendMovieStart(uint32(GetEffectValue()));
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_ds_spine_kill_credit::HandleMovie, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};
}

void AddSC_boss_spine_of_deathwing()
{
    using namespace DragonSoul;
    using namespace DragonSoul::Spine;

    RegisterDragonSoulCreatureAI(boss_spine_of_deathwing);
    RegisterDragonSoulCreatureAI(npc_ds_spine_spawner);
    RegisterDragonSoulCreatureAI(npc_ds_spine_corruption);
    RegisterDragonSoulCreatureAI(npc_ds_corrupted_blood);
    RegisterDragonSoulCreatureAI(npc_ds_hideous_amalgamation);
    RegisterDragonSoulCreatureAI(npc_ds_burning_tendons);

    RegisterSpellScript(spell_ds_spine_searing_plasma);
    RegisterSpellScript(spell_ds_spine_fiery_grip);
    RegisterSpellScript(spell_ds_spine_absorb_blood);
    RegisterSpellScript(spell_ds_spine_absorbed_blood);
    RegisterSpellScript(spell_ds_spine_nuclear_blast);
    RegisterSpellScript(spell_ds_spine_nuclear_blast_check);
    RegisterSpellScript(spell_ds_spine_seal_armor_breach);
    RegisterSpellScript(spell_ds_spine_blood_corruption);
    RegisterSpellScript(spell_ds_spine_kill_credit);
}
