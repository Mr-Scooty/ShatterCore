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
#include "vashjir.h"
#include "CombatAI.h"
#include "Containers.h"
#include "EventMap.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "PassiveAI.h"
#include "PhasingHandler.h"
#include "Player.h"
#include "Random.h"
#include "ScriptedCreature.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "TemporarySummon.h"
#include "Vehicle.h"
#include <cmath>

namespace Vashjir::KelptharForest
{

/*######
## Quest 25558 - All or Nothing (The Briny Cutter battle)
## Quest 25371 - The Abyssal Ride (seahorse rodeo)
######*/

enum KelptharCreatures
{
    // 25558 battle actors (DB spawns)
    NPC_CAPTAIN_TAYLOR_WAVE1        = 40737, // phase 180
    NPC_CAPTAIN_TAYLOR_WAVE2        = 40729, // phase 181
    NPC_ERUNAK_WAVE1                = 40746, // phase 180, SAI Lava Bolt
    NPC_ERUNAK_WAVE2                = 40736, // phase 181, SAI Lava Bolt

    // 25558 summoned attackers
    NPC_ZINJATAR_RAIDER_PRESSURE    = 40759, // faction 2205, unkillable wave-1 pressure
    NPC_ZINJATAR_RAIDER_SWARM       = 40753, // faction 74, killable circling swarm
    NPC_ZINJATAR_RAIDER_WAVE2       = 40770, // faction 74, killable wave-2 trickle
    NPC_ZINJATAR_RAIDER_ELITE       = 40782, // faction 2205, unkillable wave-2 pressure
    NPC_ZINJATAR_ABDUCTOR_SEAL      = 40786, // VehicleId 569, carries seals off
    NPC_ZINJATAR_ABDUCTOR_PLAYER    = 40797, // VehicleId 569 (SQL), personal player carry
    NPC_NAGA_DEATH_BUNNY            = 40605, // gore explosion visual bunny

    // 25558 rescue
    NPC_ERUNAK_RESCUE               = 40801,
    NPC_MOANAH_RESCUE               = 41241,
    NPC_RENDEL_RESCUE               = 41244,
    NPC_ALL_OR_NOTHING_CREDIT       = 40714,

    // 25371 rodeo
    NPC_ABYSSAL_SEAHORSE            = 39996,
    NPC_ABYSSAL_LURE                = 39942,

    // wave-2 seals with abduction texts
    NPC_SEAL_GRABBED_FIRST          = 40734,
    NPC_SEAL_GRABBED_SECOND         = 40731,

    // 25949 Horde mirror battle (Honor's Tomb)
    NPC_HONORS_TOMB_BATTLE_BUNNY    = 41766, // "Immortal Coil Battle Bunny"
    NPC_NAZGRIM_TALKER_WAVE1        = 41769, // phase 180
    NPC_NAZGRIM_TALKER_WAVE2        = 41793, // phase 181
    NPC_ZINJATAR_RAIDER_HORDE       = 41764, // faction 74 wave raiders
    NPC_ZINJATAR_RAIDER_HORDE_ELITE = 41781, // faction 14 unkillable pressure
    NPC_BLOOD_AND_THUNDER_CREDIT    = 41759
};

enum KelptharSpells
{
    // phasing (native aura 261, PhaseId in MiscValueB)
    SPELL_PHASE_BRINY_CUTTER_180    = 75901, // wave 1
    SPELL_PHASE_BRINY_CUTTER_181    = 76039, // wave 2

    // wave wipe / gore chain
    SPELL_SUMMON_NAGA_DEATH_BUNNY   = 75743, // native summon 40605
    SPELL_PERMANENT_FEIGN_DEATH     = 29266,
    SPELL_TURTLE_PARTS_00           = 77310,
    SPELL_TURTLE_PARTS_01           = 75375,
    SPELL_TURTLE_PARTS_02           = 75376,
    SPELL_NAGA_EXPLOSION            = 75744,
    SPELL_RED_RADIATION             = 52679,

    // abductions
    SPELL_NAGA_STRIKE               = 73760,
    SPELL_SUMMON_ABDUCTOR           = 76122, // native summon 40797, cast by player
    SPELL_FORCE_CREATOR_RIDE        = 76123, // script effect -> summoner boards seat 0
    SPELL_SUMMON_ERUNAK_RESCUE      = 76127, // native summon 40801 (dest-db)
    SPELL_SUMMON_THUNK_RESCUE       = 77324, // native summon 41241 (dest-db)
    SPELL_SUMMON_RENDEL_RESCUE      = 77326, // native summon 41244 (dest-db)
    SPELL_LAVA_BOLT_RESCUE          = 76128, // dummy area-entry volley visual

    // seahorse rodeo
    SPELL_ABYSSAL_RAY_TRIGGER       = 74539, // lure -> seahorse: come here
    SPELL_PECK_PUFFERFISH           = 86328,
    SPELL_MOUNT_INSTRUCTION         = 86358,
    SPELL_CLICK_TO_RIDE_TRIGGER     = 86324, // spellclick spell (dummy)
    SPELL_FORCECAST_ABYSSAL_RIDE    = 74574, // script effect BP 74573 at summoner
    SPELL_RIDE_SEAT_BEHIND          = 74573, // aura 236, BP 2 -> seat 1
    SPELL_GRIP                      = 74633,
    SPELL_BUCK_LEFT_WARNING         = 86255,
    SPELL_BUCK_RIGHT_WARNING        = 86256,
    SPELL_SPEED_WARNING             = 86257,
    SPELL_LEFT_CHECK_MASTER         = 86253,
    SPELL_RIGHT_CHECK_MASTER        = 86252,
    SPELL_SPEED_CHECK_MASTER        = 86254,
    SPELL_LEAN_LEFT                 = 87217, // vehicle bar -> rider
    SPELL_LEAN_RIGHT                = 87219, // vehicle bar -> rider
    SPELL_HOLD_ON_TIGHT             = 86332, // vehicle bar -> rider
    SPELL_EVENT_SUCCESS             = 74672,
    SPELL_EVENT_FAIL                = 74673, // also triggers 74794 Grip Loss
    SPELL_VICTORY_EMOTE             = 87372,
    SPELL_ABYSSAL_RIDE_KILL_CREDIT  = 75538, // native: KC 39996 + forcecast 86372 at summoner
    SPELL_EJECT_ALL_PASSENGERS      = 50630, // no core handler - cosmetic, ejection done manually

    // Sea Legs
    SPELL_SEA_LEGS                  = 73701
};

enum KelptharPhases
{
    PHASE_BRINY_CUTTER_WAVE1        = 180,
    PHASE_BRINY_CUTTER_WAVE2        = 181,
    PHASE_KELPTHAR_DEFAULT          = 169
};

enum KelptharTexts
{
    // 40737 Captain Taylor (wave 1)
    SAY_TAYLOR_W1_DESPERATE         = 0, // "There are too many! Shaman, do something!"
    SAY_TAYLOR_W1_REST              = 1, // "Rest while you can, men. ..."

    // 40729 Captain Taylor (wave 2): groups 0-5 in timeline order

    // 40734 / 40731 grabbed seals
    SAY_SEAL_GRABBED                = 0,

    // 40797 personal abductor
    SAY_ABDUCTOR_SPECIMEN           = 0, // "What a fine ssspecimen you are."
    SAY_ABDUCTOR_GRATEFUL           = 1, // "My Lady will be mosst grateful ..."

    // 40801 Erunak rescue
    SAY_ERUNAK_FOLLOW               = 0, // "We were most fortunate to escape, $n. Follow me."

    // 39996 Abyssal Seahorse
    WHISPER_SEAHORSE_GRAB           = 0, // "Grab the Seahorse while it's distracted!"
    EMOTE_SEAHORSE_DISPLEASED       = 1, // "The Abyssal Seahorse seems very displeased."
    WHISPER_SEAHORSE_HOLD_ON        = 2,
    WHISPER_SEAHORSE_LEAN_RIGHT     = 4,
    WHISPER_SEAHORSE_LEAN_LEFT      = 5
};

enum KelptharData
{
    DATA_ABDUCTION_TARGET           = 1,
    DATA_RODEO_RESPONSE             = 1
};

enum KelptharPoints
{
    POINT_ABDUCTOR_SEAL             = 1,
    POINT_ABDUCTOR_ESCAPE           = 2,
    POINT_PLAYER_CARRY_END          = 3,
    POINT_ERUNAK_CAVE               = 4,
    POINT_SEAHORSE_LURE             = 5,
    POINT_SEAHORSE_RIDE_END         = 6
};

uint32 const Wave1SealEntries[] = { 40738, 40739, 40740, 40741, 40742, 40743, 40744 };
uint32 const Wave2SealEntries[] = { 40730, 40731, 40732, 40733, 40734, 40735 };
uint32 const HordeCrewEntries[] = { 41796, 41797, 41798, 41799, 41800 }; // Hellscream's Vanguard, both waves

// 25558 / 25949 share one battle script - the roster swaps by controller entry (40756 A / 41766 H)
struct BattleRoster
{
    uint32 TalkerWave1;
    uint32 TalkerWave2;
    uint32 ShamanWave1;
    uint32 RaiderPressure;
    uint32 RaiderSwarm;
    uint32 RaiderTrickle;
    uint32 RaiderElite;
    uint32 SealFirstGrab;
    uint32 SealSecondGrab;
    uint32 const* Wave1Seals;
    std::size_t Wave1SealCount;
    uint32 const* Wave2Seals;
    std::size_t Wave2SealCount;
};

BattleRoster const AllianceBattleRoster =
{
    NPC_CAPTAIN_TAYLOR_WAVE1, NPC_CAPTAIN_TAYLOR_WAVE2, NPC_ERUNAK_WAVE1,
    NPC_ZINJATAR_RAIDER_PRESSURE, NPC_ZINJATAR_RAIDER_SWARM, NPC_ZINJATAR_RAIDER_WAVE2, NPC_ZINJATAR_RAIDER_ELITE,
    NPC_SEAL_GRABBED_FIRST, NPC_SEAL_GRABBED_SECOND,
    Wave1SealEntries, std::size(Wave1SealEntries), Wave2SealEntries, std::size(Wave2SealEntries)
};

BattleRoster const HordeBattleRoster =
{
    NPC_NAZGRIM_TALKER_WAVE1, NPC_NAZGRIM_TALKER_WAVE2, 0, // no phase-180 shaman twin - raiders fall back to crew targets
    NPC_ZINJATAR_RAIDER_HORDE, NPC_ZINJATAR_RAIDER_HORDE, NPC_ZINJATAR_RAIDER_HORDE, NPC_ZINJATAR_RAIDER_HORDE_ELITE,
    HordeCrewEntries[0], HordeCrewEntries[1],
    HordeCrewEntries, std::size(HordeCrewEntries), HordeCrewEntries, std::size(HordeCrewEntries)
};

// Ring around the Briny Cutter deck (center ~ -4484, 3820, deck z ~ -103)
Position const Wave1RaiderPositions[] =
{
    { -4460.0f, 3820.0f, -92.0f, 3.14f },
    { -4462.9f, 3832.0f, -93.0f, 3.67f },
    { -4471.0f, 3841.0f, -94.0f, 4.19f },
    { -4484.0f, 3844.0f, -92.0f, 4.71f },
    { -4497.0f, 3841.0f, -94.0f, 5.24f },
    { -4505.1f, 3832.0f, -95.0f, 5.76f },
    { -4508.0f, 3820.0f, -93.0f, 0.00f },
    { -4505.1f, 3808.0f, -95.0f, 0.52f },
    { -4497.0f, 3799.0f, -96.0f, 1.05f },
    { -4484.0f, 3796.0f, -95.0f, 1.57f },
    { -4471.0f, 3799.0f, -93.0f, 2.09f },
    { -4462.9f, 3808.0f, -94.0f, 2.62f }
};

// Wider, higher ring for the decorative circling swarm
Position const SwarmPositions[] =
{
    { -4452.0f, 3820.0f, -84.0f, 3.14f },
    { -4454.8f, 3833.0f, -85.0f, 3.56f },
    { -4462.6f, 3843.8f, -83.0f, 3.98f },
    { -4474.1f, 3850.4f, -86.0f, 4.40f },
    { -4487.3f, 3851.8f, -84.0f, 4.82f },
    { -4500.0f, 3847.7f, -85.0f, 5.24f },
    { -4509.9f, 3838.8f, -83.0f, 5.65f },
    { -4515.3f, 3826.7f, -86.0f, 6.07f },
    { -4515.3f, 3813.3f, -84.0f, 0.21f },
    { -4509.9f, 3801.2f, -85.0f, 0.63f },
    { -4500.0f, 3792.3f, -83.0f, 1.05f },
    { -4487.3f, 3788.2f, -86.0f, 1.47f },
    { -4474.1f, 3789.6f, -84.0f, 1.88f },
    { -4462.6f, 3796.2f, -85.0f, 2.30f },
    { -4454.8f, 3807.0f, -83.0f, 2.72f }
};

// Abductors swim in from above / north-east (first sniffed at -4394.3, 3830.0, -76.7)
Position const AbductorSpawnPositions[] =
{
    { -4420.3f, 3838.0f, -70.0f, 3.60f },
    { -4448.0f, 3856.0f, -72.0f, 4.20f },
    { -4520.0f, 3852.0f, -74.0f, 5.20f },
    { -4455.0f, 3782.0f, -70.0f, 2.40f }
};

Position const AbductorEscapePath[] =
{
    { -4438.0f, 3862.0f, -58.0f },
    { -4382.0f, 3906.0f, -34.0f }
};

// Personal abductor carry route west (sniff 23:11:06 -> 23:11:26)
Position const PlayerCarryPath[] =
{
    { -4522.0f, 3813.0f, -81.0f  },
    { -4697.0f, 3805.0f, -81.0f  },
    { -4801.0f, 3785.0f, -108.0f },
    { -4828.0f, 3762.0f, -110.0f }
};

// Horde mirror carry: Honor's Tomb wreck down to the same rescue spot (76127/77324/77326 dest-db rows are shared)
Position const HordePlayerCarryPath[] =
{
    { -4650.0f, 3950.0f, -85.0f  },
    { -4750.0f, 3870.0f, -95.0f  },
    { -4801.0f, 3785.0f, -108.0f },
    { -4828.0f, 3762.0f, -110.0f }
};

// Erunak rescue walk from ~-4837, 3757, -111 into Seafarer's Tomb (-4895, 3773, -148)
Position const ErunakCavePath[] =
{
    { -4850.9f, 3760.9f, -118.0f },
    { -4869.5f, 3766.5f, -133.0f },
    { -4886.0f, 3771.0f, -145.0f },
    { -4893.0f, 3772.5f, -147.8f }
};

// Seahorse summoned ~100y NE of the raft, approaches the lure at -4889.65, 3797.50, -148.84
Position const SeahorseApproachPath[] =
{
    { -4870.0f, 3828.0f, -156.0f },
    { -4890.5f, 3799.5f, -148.3f }
};

// WPP waypoint_path @MOVID+465 (39996), single 86.6s retail spline, speed 25.69
Position const SeahorseRidePath[] =
{
    { -4896.20f, 3809.17f, -147.80f },
    { -4958.59f, 3790.93f, -158.42f },
    { -5015.35f, 3762.39f, -169.88f },
    { -5151.26f, 3682.12f, -222.53f },
    { -5322.48f, 3598.53f, -238.76f },
    { -5551.15f, 3613.67f, -239.13f },
    { -5613.00f, 3617.95f, -219.65f },
    { -5712.56f, 3548.54f, -117.20f },
    { -5704.81f, 3455.78f, -29.49f  },
    { -5625.28f, 3429.99f, -42.03f  },
    { -5542.15f, 3433.26f, -93.75f  },
    { -5459.78f, 3461.59f, -115.42f },
    { -5384.88f, 3446.84f, -134.51f },
    { -5342.00f, 3457.72f, -142.27f },
    { -5275.65f, 3423.97f, -133.05f },
    { -5220.71f, 3407.52f, -93.47f  },
    { -5139.95f, 3386.10f, -68.00f  },
    { -5108.15f, 3421.85f, -88.34f  },
    { -5104.71f, 3488.98f, -105.59f },
    { -5117.01f, 3554.82f, -107.07f },
    { -5062.70f, 3657.72f, -105.51f },
    { -4914.70f, 3752.29f, -136.23f },
    { -4887.61f, 3787.15f, -144.68f }
};

float constexpr SEAHORSE_RIDE_VELOCITY = 25.69f;
float constexpr SEAHORSE_APPROACH_VELOCITY = 13.0f;
float constexpr CARRY_VELOCITY = 16.0f;
float constexpr ABDUCTOR_VELOCITY = 10.0f;
float constexpr ERUNAK_SWIM_VELOCITY = 4.5f;

/*######
## npc_briny_cutter_battle_bunny - 40756 (25558 battle controller, phase 180 DB spawn)
##                                 41766 (25949 Horde mirror at Honor's Tomb, same AI)
######*/

enum BattleBunnyEvents
{
    EVENT_IDLE_SCAN = 1,
    EVENT_PRESENCE_CHECK,
    EVENT_WAVE1_TAYLOR_DESPERATE,
    EVENT_WAVE1_WIPE,
    EVENT_WAVE1_TAYLOR_REST,
    EVENT_PHASE_SWAP,
    EVENT_WAVE2_ELITE,
    EVENT_WAVE2_TRICKLE,
    EVENT_WAVE2_TAYLOR_TALK,
    EVENT_WAVE2_ABDUCTION,
    EVENT_WAVE2_CAPTURE,
    EVENT_EVENT_RESET
};

enum BattleBunnyStages
{
    STAGE_IDLE = 0,
    STAGE_WAVE1,
    STAGE_WAVE2,
    STAGE_CAPTURE
};

struct npc_briny_cutter_battle_bunny : public NullCreatureAI
{
    npc_briny_cutter_battle_bunny(Creature* creature) : NullCreatureAI(creature), _summons(creature), _stage(STAGE_IDLE),
        _wave2TalkIndex(0), _abductionCount(0), _roster(creature->GetEntry() == NPC_HONORS_TOMB_BATTLE_BUNNY ? HordeBattleRoster : AllianceBattleRoster),
        _horde(creature->GetEntry() == NPC_HONORS_TOMB_BATTLE_BUNNY) { }

    void InitializeAI() override
    {
        me->SetReactState(REACT_PASSIVE);
        // DB spawn is phase 180 only - add 181 so the controller can drive both stages
        PhasingHandler::AddPhase(me, PHASE_BRINY_CUTTER_WAVE2, false);
        _events.ScheduleEvent(EVENT_IDLE_SCAN, 5s);
    }

    void JustSummoned(Creature* summon) override
    {
        _summons.Summon(summon);
    }

    void SummonedCreatureDespawn(Creature* summon) override
    {
        _summons.Despawn(summon);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_IDLE_SCAN:
                    if (FindEventPlayer(SPELL_PHASE_BRINY_CUTTER_180))
                        StartEvent();
                    else
                        _events.ScheduleEvent(EVENT_IDLE_SCAN, 2s);
                    break;
                case EVENT_PRESENCE_CHECK:
                    if (_stage == STAGE_WAVE1 || _stage == STAGE_WAVE2)
                    {
                        if (!FindEventPlayer(SPELL_PHASE_BRINY_CUTTER_180) && !FindEventPlayer(SPELL_PHASE_BRINY_CUTTER_181))
                            CleanupEvent();
                        else
                            _events.ScheduleEvent(EVENT_PRESENCE_CHECK, 5s);
                    }
                    break;
                case EVENT_WAVE1_TAYLOR_DESPERATE:
                    ActorTalk(_roster.TalkerWave1, SAY_TAYLOR_W1_DESPERATE);
                    break;
                case EVENT_WAVE1_WIPE:
                    WipeWave1();
                    break;
                case EVENT_WAVE1_TAYLOR_REST:
                    ActorTalk(_roster.TalkerWave1, SAY_TAYLOR_W1_REST);
                    break;
                case EVENT_PHASE_SWAP:
                    StartWave2();
                    break;
                case EVENT_WAVE2_ELITE:
                    SummonWave2Elite();
                    break;
                case EVENT_WAVE2_TRICKLE:
                    TrickleWave2();
                    if (_stage == STAGE_WAVE2)
                        _events.ScheduleEvent(EVENT_WAVE2_TRICKLE, 12s);
                    break;
                case EVENT_WAVE2_TAYLOR_TALK:
                    if (_wave2TalkIndex <= 5)
                        ActorTalk(_roster.TalkerWave2, _wave2TalkIndex++);
                    break;
                case EVENT_WAVE2_ABDUCTION:
                    LaunchAbduction();
                    break;
                case EVENT_WAVE2_CAPTURE:
                    CapturePlayers();
                    break;
                case EVENT_EVENT_RESET:
                    CleanupEvent();
                    break;
                default:
                    break;
            }
        }
    }

private:
    Player* FindEventPlayer(uint32 auraId) const
    {
        std::list<Player*> players;
        me->GetPlayerListInGrid(players, 90.0f);
        for (Player* player : players)
            if (player->IsAlive() && player->HasAura(auraId))
                return player;
        return nullptr;
    }

    Creature* FindActor(uint32 entry) const
    {
        return me->FindNearestCreature(entry, 150.0f);
    }

    void ActorTalk(uint32 entry, uint8 group) const
    {
        if (Creature* actor = FindActor(entry))
            if (actor->IsAIEnabled())
                actor->AI()->Talk(group);
    }

    void SetSummonPhase(Creature* summon, uint32 phaseId) const
    {
        // controller carries both battle phases - narrow each summon down to its stage
        PhasingHandler::RemovePhase(summon, phaseId == PHASE_BRINY_CUTTER_WAVE1 ? PHASE_BRINY_CUTTER_WAVE2 : PHASE_BRINY_CUTTER_WAVE1, false);
        PhasingHandler::AddPhase(summon, phaseId, true);
    }

    Unit* PickWave1Target(uint8 index) const
    {
        // first raider harasses Taylor, second Erunak (keeps his IC Lava Bolt SAI running)
        if (index == 0)
            return FindActor(_roster.TalkerWave1);
        if (index == 1 && _roster.ShamanWave1)
            return FindActor(_roster.ShamanWave1);
        return FindActor(_roster.Wave1Seals[urand(0, uint32(_roster.Wave1SealCount) - 1)]);
    }

    // the Alliance wreck keeps its hand-tuned rings; the Horde mirror builds them around the controller spawn
    Position RingPosition(float radius, float zOffset, float angle) const
    {
        return Position(me->GetPositionX() + radius * std::cos(angle), me->GetPositionY() + radius * std::sin(angle),
            me->GetPositionZ() + zOffset, Position::NormalizeOrientation(angle + float(M_PI)));
    }

    Position Wave1RaiderPosition(uint8 index) const
    {
        if (!_horde)
            return Wave1RaiderPositions[index];
        return RingPosition(24.0f, 9.0f, float(index) * 2.0f * float(M_PI) / float(std::size(Wave1RaiderPositions)));
    }

    Position SwarmPosition(uint8 index) const
    {
        if (!_horde)
            return SwarmPositions[index];
        return RingPosition(32.0f, 16.0f, float(index) * 2.0f * float(M_PI) / float(std::size(SwarmPositions)));
    }

    Position AbductorSpawnPosition(uint8 index) const
    {
        if (!_horde)
            return AbductorSpawnPositions[index % std::size(AbductorSpawnPositions)];
        return RingPosition(55.0f, 25.0f, 0.6f + float(index % 4) * float(M_PI) / 2.0f);
    }

    void StartEvent()
    {
        _stage = STAGE_WAVE1;
        _wave2TalkIndex = 0;
        _abductionCount = 0;

        // wave-1 pressure raiders (unkillable, attack the crew)
        for (uint8 i = 0; i < std::size(Wave1RaiderPositions); ++i)
        {
            if (Creature* raider = me->SummonCreature(_roster.RaiderPressure, Wave1RaiderPosition(i), TEMPSUMMON_TIMED_DESPAWN, 2min))
            {
                SetSummonPhase(raider, PHASE_BRINY_CUTTER_WAVE1);
                raider->SetImmuneToPC(true);
                if (Unit* target = PickWave1Target(i))
                    if (raider->IsAIEnabled())
                        raider->AI()->AttackStart(target);
            }
        }

        // decorative killable swarm circling above deck
        for (uint8 i = 0; i < std::size(SwarmPositions); ++i)
        {
            if (Creature* swarm = me->SummonCreature(_roster.RaiderSwarm, SwarmPosition(i), TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 2min))
            {
                SetSummonPhase(swarm, PHASE_BRINY_CUTTER_WAVE1);
                swarm->GetMotionMaster()->MoveRandom(10.0f);
            }
        }

        // sniff offsets from phase-180 entry (23:08:30)
        _events.ScheduleEvent(EVENT_WAVE1_TAYLOR_DESPERATE, 41s);
        _events.ScheduleEvent(EVENT_WAVE1_WIPE, 50s + 500ms);
        _events.ScheduleEvent(EVENT_WAVE1_TAYLOR_REST, 55s + 500ms);
        _events.ScheduleEvent(EVENT_PHASE_SWAP, 66s);
        _events.ScheduleEvent(EVENT_PRESENCE_CHECK, 10s);
    }

    void WipeWave1()
    {
        std::list<Creature*> raiders;
        me->GetCreatureListWithEntryInGrid(raiders, _roster.RaiderPressure, 120.0f);
        for (Creature* raider : raiders)
        {
            if (!raider->IsAlive())
                continue;
            raider->CastSpell(raider, SPELL_SUMMON_NAGA_DEATH_BUNNY, true);
            raider->CastSpell(raider, SPELL_PERMANENT_FEIGN_DEATH, true);
            raider->DespawnOrUnsummon(2s);
        }

        if (_roster.RaiderSwarm == _roster.RaiderPressure) // Horde set shares one raider entry - already handled above
            return;

        std::list<Creature*> swarm;
        me->GetCreatureListWithEntryInGrid(swarm, _roster.RaiderSwarm, 120.0f);
        for (Creature* naga : swarm)
            naga->DespawnOrUnsummon(Milliseconds(urand(0, 1500)));
    }

    void StartWave2()
    {
        _stage = STAGE_WAVE2;

        // swap every wave-1 player to phase 181 (native aura 261, MiscValueB 181)
        std::list<Player*> players;
        me->GetPlayerListInGrid(players, 90.0f);
        for (Player* player : players)
        {
            if (!player->IsAlive() || !player->HasAura(SPELL_PHASE_BRINY_CUTTER_180))
                continue;
            player->RemoveAurasDueToSpell(SPELL_PHASE_BRINY_CUTTER_180);
            player->CastSpell(player, SPELL_PHASE_BRINY_CUTTER_181, true);
        }

        _events.ScheduleEvent(EVENT_WAVE2_ELITE, 5s);
        _events.ScheduleEvent(EVENT_WAVE2_ELITE, 18s);
        _events.ScheduleEvent(EVENT_WAVE2_TRICKLE, 8s);

        // Taylor 40729 yells (sniff: T2 +10/+16/+28/+55/+66/+79)
        _events.ScheduleEvent(EVENT_WAVE2_TAYLOR_TALK, 10s);
        _events.ScheduleEvent(EVENT_WAVE2_TAYLOR_TALK, 16s);
        _events.ScheduleEvent(EVENT_WAVE2_TAYLOR_TALK, 28s);
        _events.ScheduleEvent(EVENT_WAVE2_TAYLOR_TALK, 55s);
        _events.ScheduleEvent(EVENT_WAVE2_TAYLOR_TALK, 66s);
        _events.ScheduleEvent(EVENT_WAVE2_TAYLOR_TALK, 79s);

        // scripted seal abductions
        _events.ScheduleEvent(EVENT_WAVE2_ABDUCTION, 40s);
        _events.ScheduleEvent(EVENT_WAVE2_ABDUCTION, 56s);
        _events.ScheduleEvent(EVENT_WAVE2_ABDUCTION, 67s);
        _events.ScheduleEvent(EVENT_WAVE2_ABDUCTION, 80s);

        _events.ScheduleEvent(EVENT_WAVE2_CAPTURE, 87s);
        _events.ScheduleEvent(EVENT_EVENT_RESET, 115s);
    }

    void SummonWave2Elite()
    {
        Position pos = Wave1RaiderPosition(uint8(urand(0, uint32(std::size(Wave1RaiderPositions)) - 1)));
        if (Creature* elite = me->SummonCreature(_roster.RaiderElite, pos, TEMPSUMMON_TIMED_DESPAWN, 2min))
        {
            SetSummonPhase(elite, PHASE_BRINY_CUTTER_WAVE2);
            elite->SetImmuneToPC(true);
            if (Creature* seal = FindActor(_roster.Wave2Seals[urand(0, uint32(_roster.Wave2SealCount) - 1)]))
                if (elite->IsAIEnabled())
                    elite->AI()->AttackStart(seal);
        }
    }

    void TrickleWave2()
    {
        std::list<Creature*> raiders;
        me->GetCreatureListWithEntryInGrid(raiders, _roster.RaiderTrickle, 120.0f);
        uint8 alive = 0;
        for (Creature* raider : raiders)
            if (raider->IsAlive())
                ++alive;

        if (alive >= 8)
            return;

        for (uint8 i = 0; i < 2; ++i)
        {
            Position pos = Wave1RaiderPosition(uint8(urand(0, uint32(std::size(Wave1RaiderPositions)) - 1)));
            if (Creature* raider = me->SummonCreature(_roster.RaiderTrickle, pos, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 3min))
            {
                SetSummonPhase(raider, PHASE_BRINY_CUTTER_WAVE2);
                if (Creature* seal = FindActor(_roster.Wave2Seals[urand(0, uint32(_roster.Wave2SealCount) - 1)]))
                    if (raider->IsAIEnabled())
                        raider->AI()->AttackStart(seal);
            }
        }
    }

    void LaunchAbduction()
    {
        // first two grabs take the seals that have abduction texts
        Creature* target = nullptr;
        if (_abductionCount == 0)
            target = FindActor(_roster.SealFirstGrab);
        else if (_abductionCount == 1)
            target = FindActor(_roster.SealSecondGrab);

        if (!target || !target->IsAlive() || target->GetVehicle())
        {
            std::vector<Creature*> seals;
            for (std::size_t i = 0; i < _roster.Wave2SealCount; ++i)
                if (Creature* seal = FindActor(_roster.Wave2Seals[i]))
                    if (seal->IsAlive() && !seal->GetVehicle())
                        seals.push_back(seal);
            if (seals.empty())
                return;
            target = Trinity::Containers::SelectRandomContainerElement(seals);
        }

        Position pos = AbductorSpawnPosition(_abductionCount);
        if (Creature* abductor = me->SummonCreature(NPC_ZINJATAR_ABDUCTOR_SEAL, pos, TEMPSUMMON_TIMED_DESPAWN, 1min))
        {
            SetSummonPhase(abductor, PHASE_BRINY_CUTTER_WAVE2);
            if (abductor->IsAIEnabled())
                abductor->AI()->SetGUID(target->GetGUID(), DATA_ABDUCTION_TARGET);
        }
        ++_abductionCount;
    }

    void CapturePlayers()
    {
        _stage = STAGE_CAPTURE;

        std::list<Player*> players;
        me->GetPlayerListInGrid(players, 90.0f);
        for (Player* player : players)
            if (player->IsAlive() && player->HasAura(SPELL_PHASE_BRINY_CUTTER_181) && !player->GetVehicle())
                player->CastSpell(player, SPELL_SUMMON_ABDUCTOR, true); // personal 40797 takes over
    }

    void CleanupEvent()
    {
        // stragglers still in phase 181 are put back to phase 180 so the next loop can pick them up
        std::list<Player*> players;
        me->GetPlayerListInGrid(players, 120.0f);
        for (Player* player : players)
        {
            if (!player->HasAura(SPELL_PHASE_BRINY_CUTTER_181) || player->GetVehicle())
                continue;
            player->RemoveAurasDueToSpell(SPELL_PHASE_BRINY_CUTTER_181);
            player->CastSpell(player, SPELL_PHASE_BRINY_CUTTER_180, true);
        }

        _summons.DespawnAll();
        _events.Reset();
        _stage = STAGE_IDLE;
        _wave2TalkIndex = 0;
        _abductionCount = 0;
        _events.ScheduleEvent(EVENT_IDLE_SCAN, 5s);
    }

    EventMap _events;
    SummonList _summons;
    uint8 _stage;
    uint8 _wave2TalkIndex;
    uint8 _abductionCount;
    BattleRoster const& _roster;
    bool _horde;
};

/*######
## npc_zinjatar_abductor - 40786 (seal grabber, VehicleId 569)
######*/

enum SealAbductorEvents
{
    EVENT_ABDUCTOR_ESCAPE = 1
};

struct npc_zinjatar_abductor : public PassiveAI
{
    npc_zinjatar_abductor(Creature* creature) : PassiveAI(creature) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        me->SetDisableGravity(true);
    }

    void SetGUID(ObjectGuid const& guid, int32 id) override
    {
        if (id != DATA_ABDUCTION_TARGET)
            return;

        _sealGUID = guid;
        if (Creature* seal = ObjectAccessor::GetCreature(*me, _sealGUID))
        {
            Position pos = seal->GetPosition();
            pos.m_positionZ += 2.0f;
            me->GetMotionMaster()->MovePoint(POINT_ABDUCTOR_SEAL, pos, false, ABDUCTOR_VELOCITY);
        }
        else
            me->DespawnOrUnsummon();
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type != POINT_MOTION_TYPE && type != EFFECT_MOTION_TYPE)
            return;

        switch (id)
        {
            case POINT_ABDUCTOR_SEAL:
                if (Creature* seal = ObjectAccessor::GetCreature(*me, _sealGUID))
                {
                    if (seal->IsAlive())
                    {
                        me->CastSpell(seal, SPELL_NAGA_STRIKE, true);
                        if (seal->GetEntry() == NPC_SEAL_GRABBED_FIRST || seal->GetEntry() == NPC_SEAL_GRABBED_SECOND)
                            if (seal->IsAIEnabled())
                                seal->AI()->Talk(SAY_SEAL_GRABBED);
                        seal->EnterVehicle(me, 0);
                    }
                }
                _events.ScheduleEvent(EVENT_ABDUCTOR_ESCAPE, 1s + 500ms);
                break;
            case POINT_ABDUCTOR_ESCAPE:
                if (Vehicle* kit = me->GetVehicleKit())
                    if (Unit* passenger = kit->GetPassenger(0))
                        if (Creature* seal = passenger->ToCreature())
                            seal->DespawnOrUnsummon(0s, 120s); // DB seal - bring it back for the next run
                me->DespawnOrUnsummon(500ms);
                break;
            default:
                break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            if (eventId == EVENT_ABDUCTOR_ESCAPE)
            {
                if (IsHordeEvent())
                {
                    // Honor's Tomb has no hand-tuned escape lane - rise away along the current facing
                    float angle = me->GetOrientation();
                    Position path[2] =
                    {
                        { me->GetPositionX() + 30.0f * std::cos(angle), me->GetPositionY() + 30.0f * std::sin(angle), me->GetPositionZ() + 20.0f },
                        { me->GetPositionX() + 90.0f * std::cos(angle), me->GetPositionY() + 90.0f * std::sin(angle), me->GetPositionZ() + 55.0f }
                    };
                    me->GetMotionMaster()->MoveSmoothPath(POINT_ABDUCTOR_ESCAPE, path, std::size(path), false, true, ABDUCTOR_VELOCITY);
                }
                else
                    me->GetMotionMaster()->MoveSmoothPath(POINT_ABDUCTOR_ESCAPE, AbductorEscapePath, std::size(AbductorEscapePath), false, true, ABDUCTOR_VELOCITY);
            }
        }
    }

private:
    bool IsHordeEvent()
    {
        if (TempSummon* summon = me->ToTempSummon())
            if (Unit* summoner = summon->GetSummoner())
                return summoner->GetEntry() == NPC_HONORS_TOMB_BATTLE_BUNNY;
        return false;
    }

    EventMap _events;
    ObjectGuid _sealGUID;
};

/*######
## npc_zinjatar_abductor_player - 40797 (personal carry, summoned by 76122)
######*/

enum PlayerAbductorEvents
{
    EVENT_CARRY_START = 1,
    EVENT_CARRY_TALK_SPECIMEN,
    EVENT_CARRY_UNPHASE,
    EVENT_CARRY_TALK_GRATEFUL,
    EVENT_RESCUE_VOLLEY_1,
    EVENT_RESCUE_VOLLEY_2,
    EVENT_RESCUE_GORE_DEATH
};

struct npc_zinjatar_abductor_player : public PassiveAI
{
    npc_zinjatar_abductor_player(Creature* creature) : PassiveAI(creature), _horde(false) { }

    void IsSummonedBy(Unit* summoner) override
    {
        Player* player = summoner->ToPlayer();
        if (!player)
        {
            me->DespawnOrUnsummon();
            return;
        }

        _summonerGUID = player->GetGUID();
        _horde = player->GetTeam() == HORDE;
        me->SetDisableGravity(true);
        me->CastSpell(player, SPELL_FORCE_CREATOR_RIDE, true); // script effect: summoner boards seat 0

        _events.ScheduleEvent(EVENT_CARRY_START, 2s);
        _events.ScheduleEvent(EVENT_CARRY_TALK_SPECIMEN, 2s + 500ms);
        _events.ScheduleEvent(EVENT_CARRY_UNPHASE, 15s);      // sniff: phase 181 dropped mid-flight
        _events.ScheduleEvent(EVENT_CARRY_TALK_GRATEFUL, 16s);
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type != EFFECT_MOTION_TYPE || id != POINT_PLAYER_CARRY_END)
            return;

        // summon the rescue trio (native dest-db summons at the sniffed coords)
        if (Player* player = ObjectAccessor::GetPlayer(*me, _summonerGUID))
        {
            player->CastSpell(player, SPELL_SUMMON_ERUNAK_RESCUE, true);
            player->CastSpell(player, SPELL_SUMMON_THUNK_RESCUE, true);
            player->CastSpell(player, SPELL_SUMMON_RENDEL_RESCUE, true);
        }

        _events.ScheduleEvent(EVENT_RESCUE_VOLLEY_1, 1s + 500ms);
        _events.ScheduleEvent(EVENT_RESCUE_VOLLEY_2, 3s + 500ms);
        _events.ScheduleEvent(EVENT_RESCUE_GORE_DEATH, 5s);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_CARRY_START:
                    if (_horde)
                        me->GetMotionMaster()->MoveSmoothPath(POINT_PLAYER_CARRY_END, HordePlayerCarryPath, std::size(HordePlayerCarryPath), false, true, CARRY_VELOCITY);
                    else
                        me->GetMotionMaster()->MoveSmoothPath(POINT_PLAYER_CARRY_END, PlayerCarryPath, std::size(PlayerCarryPath), false, true, CARRY_VELOCITY);
                    break;
                case EVENT_CARRY_TALK_SPECIMEN:
                    if (Player* player = ObjectAccessor::GetPlayer(*me, _summonerGUID))
                        Talk(SAY_ABDUCTOR_SPECIMEN, player);
                    break;
                case EVENT_CARRY_UNPHASE:
                    if (Player* player = ObjectAccessor::GetPlayer(*me, _summonerGUID))
                        player->RemoveAurasDueToSpell(SPELL_PHASE_BRINY_CUTTER_181);
                    // keep carrying the now-default-phase passenger
                    PhasingHandler::AddPhase(me, PHASE_KELPTHAR_DEFAULT, true);
                    break;
                case EVENT_CARRY_TALK_GRATEFUL:
                    if (Player* player = ObjectAccessor::GetPlayer(*me, _summonerGUID))
                        Talk(SAY_ABDUCTOR_GRATEFUL, player);
                    break;
                case EVENT_RESCUE_VOLLEY_1:
                case EVENT_RESCUE_VOLLEY_2:
                    RescueVolley();
                    break;
                case EVENT_RESCUE_GORE_DEATH:
                    me->CastSpell(me, SPELL_SUMMON_NAGA_DEATH_BUNNY, true); // 40605 fires the gore chain
                    if (Vehicle* kit = me->GetVehicleKit())
                        kit->RemoveAllPassengers();
                    me->DespawnOrUnsummon(1s);
                    break;
                default:
                    break;
            }
        }
    }

private:
    void RescueVolley() const
    {
        // trio volleys Lava Bolt 76128 (area-entry targeting hits me via conditions)
        for (uint32 entry : { uint32(NPC_ERUNAK_RESCUE), uint32(NPC_MOANAH_RESCUE), uint32(NPC_RENDEL_RESCUE) })
        {
            std::list<Creature*> rescuers;
            me->GetCreatureListWithEntryInGrid(rescuers, entry, 60.0f);
            for (Creature* rescuer : rescuers)
            {
                TempSummon* summon = rescuer->ToTempSummon();
                if (!summon || summon->GetSummonerGUID() != _summonerGUID)
                    continue;
                rescuer->CastSpell(nullptr, SPELL_LAVA_BOLT_RESCUE, false);
            }
        }
    }

    EventMap _events;
    ObjectGuid _summonerGUID;
    bool _horde;
};

/*######
## npc_erunak_rescue - 40801 (summoned by 76127 after the carry)
######*/

enum ErunakRescueEvents
{
    EVENT_ERUNAK_TALK_FOLLOW = 1,
    EVENT_ERUNAK_MOVE_CAVE,
    EVENT_ERUNAK_CREDIT,
    EVENT_ERUNAK_DESPAWN
};

struct npc_erunak_rescue : public PassiveAI
{
    npc_erunak_rescue(Creature* creature) : PassiveAI(creature) { }

    void IsSummonedBy(Unit* summoner) override
    {
        if (Player* player = summoner->ToPlayer())
            _summonerGUID = player->GetGUID();

        _events.ScheduleEvent(EVENT_ERUNAK_TALK_FOLLOW, 6s + 500ms);
        _events.ScheduleEvent(EVENT_ERUNAK_MOVE_CAVE, 8s + 500ms);
        _events.ScheduleEvent(EVENT_ERUNAK_CREDIT, 20s);  // sniff: credit 20.3s after rescue summons, no proximity gate
        _events.ScheduleEvent(EVENT_ERUNAK_DESPAWN, 29s);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_ERUNAK_TALK_FOLLOW:
                    if (Player* player = ObjectAccessor::GetPlayer(*me, _summonerGUID))
                        Talk(SAY_ERUNAK_FOLLOW, player);
                    break;
                case EVENT_ERUNAK_MOVE_CAVE:
                {
                    uint8 angleIndex = 0;
                    ForEachCompanion([&](Creature* companion)
                    {
                        companion->GetMotionMaster()->MoveFollow(me, 2.5f, 1.3f + 3.5f * float(angleIndex++));
                    });
                    me->GetMotionMaster()->MoveSmoothPath(POINT_ERUNAK_CAVE, ErunakCavePath, std::size(ErunakCavePath), false, true, ERUNAK_SWIM_VELOCITY);
                    break;
                }
                case EVENT_ERUNAK_CREDIT:
                    if (Player* player = ObjectAccessor::GetPlayer(*me, _summonerGUID))
                        player->KilledMonsterCredit(player->GetTeam() == HORDE ? NPC_BLOOD_AND_THUNDER_CREDIT : NPC_ALL_OR_NOTHING_CREDIT);
                    break;
                case EVENT_ERUNAK_DESPAWN:
                    ForEachCompanion([](Creature* companion)
                    {
                        companion->DespawnOrUnsummon();
                    });
                    me->DespawnOrUnsummon();
                    break;
                default:
                    break;
            }
        }
    }

private:
    template<typename Action>
    void ForEachCompanion(Action&& action) const
    {
        for (uint32 entry : { uint32(NPC_MOANAH_RESCUE), uint32(NPC_RENDEL_RESCUE) })
        {
            std::list<Creature*> companions;
            me->GetCreatureListWithEntryInGrid(companions, entry, 60.0f);
            for (Creature* companion : companions)
            {
                TempSummon* summon = companion->ToTempSummon();
                if (!summon || summon->GetSummonerGUID() != _summonerGUID)
                    continue;
                action(companion);
            }
        }
    }

    EventMap _events;
    ObjectGuid _summonerGUID;
};

/*######
## npc_naga_death_bunny - 40605 (gore explosion, summoned by 75743)
######*/

struct npc_naga_death_bunny : public NullCreatureAI
{
    npc_naga_death_bunny(Creature* creature) : NullCreatureAI(creature) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        me->CastSpell(me, SPELL_TURTLE_PARTS_00, true);
        me->CastSpell(me, SPELL_TURTLE_PARTS_01, true);
        me->CastSpell(me, SPELL_TURTLE_PARTS_02, true);
        me->CastSpell(me, SPELL_NAGA_EXPLOSION, true);
        me->CastSpell(me, SPELL_RED_RADIATION, true);
        me->DespawnOrUnsummon(6s);
    }
};

/*######
## npc_abyssal_seahorse - 39996 (25371 rodeo vehicle, summoned by 74609)
######*/

enum SeahorseEvents
{
    EVENT_SEAHORSE_APPROACH_LURE = 1,
    EVENT_SEAHORSE_START_RUN,
    EVENT_SEAHORSE_PROMPT,
    EVENT_SEAHORSE_CHECK
};

enum RodeoRoundTypes
{
    RODEO_ROUND_LEFT  = 0,
    RODEO_ROUND_RIGHT = 1,
    RODEO_ROUND_SPEED = 2
};

struct RodeoRound
{
    uint32 WarningSpellId;
    uint32 CheckSpellId;
    uint32 ResponseSpellId;
    uint8 TextGroup;
};

RodeoRound const RodeoRounds[3] =
{
    { SPELL_BUCK_LEFT_WARNING,  SPELL_LEFT_CHECK_MASTER,  SPELL_LEAN_LEFT,     WHISPER_SEAHORSE_LEAN_LEFT  },
    { SPELL_BUCK_RIGHT_WARNING, SPELL_RIGHT_CHECK_MASTER, SPELL_LEAN_RIGHT,    WHISPER_SEAHORSE_LEAN_RIGHT },
    { SPELL_SPEED_WARNING,      SPELL_SPEED_CHECK_MASTER, SPELL_HOLD_ON_TIGHT, WHISPER_SEAHORSE_HOLD_ON    }
};

// success/fail whispers are player-sender whispers in the sniff (not creature_text) - sent script-side
char const* const RodeoPraiseWhispers[6] =
{
    "Good!",
    "Good work!",
    "Well done!",
    "Excellent!",
    "Unshakable!",
    "Smooth moves!"
};

char const* const RodeoGripWhisper = "You're losing grip.  Be careful!"; // double space is retail

uint8 constexpr RODEO_MAX_FAILS = 3;

struct npc_abyssal_seahorse : public VehicleAI
{
    npc_abyssal_seahorse(Creature* creature) : VehicleAI(creature), _approachStarted(false), _peckReady(false),
        _rideStarted(false), _rideEnded(false), _awaitingResponse(false), _round(RODEO_ROUND_LEFT), _responseSpell(0), _fails(0) { }

    void IsSummonedBy(Unit* summoner) override
    {
        _summonerGUID = summoner->GetGUID();
        me->SetReactState(REACT_PASSIVE);
        me->SetDisableGravity(true);
        // fallback in case the lure's 74539 ping never arrives
        _events.ScheduleEvent(EVENT_SEAHORSE_APPROACH_LURE, 12s);
    }

    void SpellHit(WorldObject* /*caster*/, SpellInfo const* spellInfo) override
    {
        switch (spellInfo->Id)
        {
            case SPELL_ABYSSAL_RAY_TRIGGER:
                if (!_approachStarted)
                    _events.RescheduleEvent(EVENT_SEAHORSE_APPROACH_LURE, 1ms);
                break;
            case SPELL_LEAN_LEFT:
            case SPELL_LEAN_RIGHT:
            case SPELL_HOLD_ON_TIGHT:
                // rider-cast path of the vehicle-bar redirect
                RecordResponse(spellInfo->Id);
                break;
            default:
                break;
        }
    }

    void SpellHitTarget(WorldObject* target, SpellInfo const* spellInfo) override
    {
        // vehicle-cast path: bar spell cast by me at the rider (TARGET_UNIT_PASSENGER_1)
        if (target != me)
            switch (spellInfo->Id)
            {
                case SPELL_LEAN_LEFT:
                case SPELL_LEAN_RIGHT:
                case SPELL_HOLD_ON_TIGHT:
                    RecordResponse(spellInfo->Id);
                    break;
                default:
                    break;
            }
    }

    void SetData(uint32 id, uint32 value) override
    {
        if (id == DATA_RODEO_RESPONSE)
            RecordResponse(value);
    }

    void OnSpellClick(Unit* clicker, bool& result) override
    {
        if (!result || !_peckReady || _rideStarted)
            return;
        if (clicker->GetGUID() != _summonerGUID) // personal seahorse
            return;

        // script effect at summoner -> summoner casts 74573 -> boards seat 1
        DoCastSelf(SPELL_FORCECAST_ABYSSAL_RIDE);
    }

    void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
    {
        if (passenger->GetTypeId() != TYPEID_PLAYER) // the lure also attaches itself (57346)
            return;

        if (!apply)
        {
            // rider left early (or was ejected) - clean up if the run is not already over
            if (_rideStarted && !_rideEnded)
            {
                _rideEnded = true;
                _events.Reset();
                me->DespawnOrUnsummon(5s);
            }
            return;
        }

        if (_rideStarted)
            return;

        _riderGUID = passenger->GetGUID();
        _rideStarted = true;
        _peckReady = false;
        Talk(EMOTE_SEAHORSE_DISPLEASED);
        _events.ScheduleEvent(EVENT_SEAHORSE_START_RUN, 5s);
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type != EFFECT_MOTION_TYPE)
            return;

        switch (id)
        {
            case POINT_SEAHORSE_LURE:
            {
                _peckReady = true;
                DoCastSelf(SPELL_PECK_PUFFERFISH);         // conditions row targets the lure
                DoCastSelf(SPELL_MOUNT_INSTRUCTION, true);
                if (Player* summoner = ObjectAccessor::GetPlayer(*me, _summonerGUID))
                    Talk(WHISPER_SEAHORSE_GRAB, summoner);
                break;
            }
            case POINT_SEAHORSE_RIDE_END:
            {
                if (_rideEnded)
                    break;
                _rideEnded = true;
                _events.Reset();
                DoCastSelf(SPELL_VICTORY_EMOTE, true);
                DoCastSelf(SPELL_ABYSSAL_RIDE_KILL_CREDIT, true); // native: KC 39996 + Erunak ping at summoner
                DoCastSelf(SPELL_EJECT_ALL_PASSENGERS, true);
                if (Vehicle* kit = me->GetVehicleKit())
                    kit->RemoveAllPassengers();
                me->DespawnOrUnsummon(6s);
                break;
            }
            default:
                break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        VehicleAI::UpdateAI(diff);
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_SEAHORSE_APPROACH_LURE:
                    if (!_approachStarted)
                    {
                        _approachStarted = true;
                        me->GetMotionMaster()->MoveSmoothPath(POINT_SEAHORSE_LURE, SeahorseApproachPath, std::size(SeahorseApproachPath), false, true, SEAHORSE_APPROACH_VELOCITY);
                    }
                    break;
                case EVENT_SEAHORSE_START_RUN:
                    DoCastSelf(SPELL_GRIP, true);
                    me->GetMotionMaster()->MoveSmoothPath(POINT_SEAHORSE_RIDE_END, SeahorseRidePath, std::size(SeahorseRidePath), false, true, SEAHORSE_RIDE_VELOCITY);
                    _events.ScheduleEvent(EVENT_SEAHORSE_PROMPT, 6s);
                    break;
                case EVENT_SEAHORSE_PROMPT:
                {
                    if (_rideEnded)
                        break;
                    _round = uint8(urand(RODEO_ROUND_LEFT, RODEO_ROUND_SPEED));
                    _responseSpell = 0;
                    _awaitingResponse = true;
                    RodeoRound const& round = RodeoRounds[_round];
                    DoCastSelf(round.WarningSpellId, true);
                    if (Player* rider = GetRider())
                        Talk(round.TextGroup, rider);
                    _events.ScheduleEvent(EVENT_SEAHORSE_CHECK, 2s + 500ms);
                    _events.ScheduleEvent(EVENT_SEAHORSE_PROMPT, 4s);
                    break;
                }
                case EVENT_SEAHORSE_CHECK:
                    EvaluateRound();
                    break;
                default:
                    break;
            }
        }
    }

private:
    Player* GetRider() const
    {
        return ObjectAccessor::GetPlayer(*me, _riderGUID);
    }

    void RecordResponse(uint32 spellId)
    {
        if (_awaitingResponse)
            _responseSpell = spellId;
    }

    void EvaluateRound()
    {
        if (_rideEnded || !_awaitingResponse)
            return;

        _awaitingResponse = false;
        RodeoRound const& round = RodeoRounds[_round];
        DoCastSelf(round.CheckSpellId, true); // retail check spell at the rider (cosmetic here)

        Player* rider = GetRider();
        if (!rider)
            return;

        if (_responseSpell == round.ResponseSpellId)
        {
            rider->CastSpell(rider, SPELL_EVENT_SUCCESS, true);
            me->Whisper(RodeoPraiseWhispers[urand(0, 5)], LANG_UNIVERSAL, rider, true);
        }
        else
        {
            rider->CastSpell(rider, SPELL_EVENT_FAIL, true); // triggers 74794 Grip Loss
            me->Whisper(RodeoGripWhisper, LANG_UNIVERSAL, rider, true);
            if (++_fails >= RODEO_MAX_FAILS)
            {
                // thrown off - no credit
                _rideEnded = true;
                _events.Reset();
                DoCastSelf(SPELL_EJECT_ALL_PASSENGERS, true);
                if (Vehicle* kit = me->GetVehicleKit())
                    kit->RemoveAllPassengers();
                me->DespawnOrUnsummon(4s);
            }
        }
    }

    EventMap _events;
    ObjectGuid _summonerGUID;
    ObjectGuid _riderGUID;
    bool _approachStarted;
    bool _peckReady;
    bool _rideStarted;
    bool _rideEnded;
    bool _awaitingResponse;
    uint8 _round;
    uint32 _responseSpell;
    uint8 _fails;
};

/*######
## spell_vashjir_sea_legs_reward - 86672 (quest reward dummy -> apply 73701 immediately)
######*/

class spell_vashjir_sea_legs_reward : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SEA_LEGS });
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Unit* target = GetHitUnit();
        target->CastSpell(target, SPELL_SEA_LEGS, true);
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_vashjir_sea_legs_reward::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

/*######
## spell_vashjir_force_creator_ride_abductor - 76123 (script effect at summoner -> board seat 0)
######*/

class spell_vashjir_force_creator_ride_abductor : public SpellScript
{
    void HandleScriptEffect(SpellEffIndex /*effIndex*/)
    {
        if (Unit* caster = GetCaster())
            GetHitUnit()->EnterVehicle(caster, 0);
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_vashjir_force_creator_ride_abductor::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

/*######
## spell_vashjir_forcecast_abyssal_ride - 74574 (script effect BP 74573 at summoner)
######*/

class spell_vashjir_forcecast_abyssal_ride : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_RIDE_SEAT_BEHIND });
    }

    void HandleScriptEffect(SpellEffIndex /*effIndex*/)
    {
        if (Unit* caster = GetCaster())
            GetHitUnit()->CastSpell(caster, SPELL_RIDE_SEAT_BEHIND, true);
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_vashjir_forcecast_abyssal_ride::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

/*######
## spell_vashjir_seahorse_rodeo_response - 87217 / 87219 / 86332 (notify the rodeo AI)
######*/

class spell_vashjir_seahorse_rodeo_response : public SpellScript
{
    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        // the fork's vehicle-bar redirect may make either the rider or the seahorse the caster
        Creature* seahorse = nullptr;
        if (caster->GetEntry() == NPC_ABYSSAL_SEAHORSE)
            seahorse = caster->ToCreature();
        else if (Unit* base = caster->GetVehicleBase())
            if (base->GetEntry() == NPC_ABYSSAL_SEAHORSE)
                seahorse = base->ToCreature();

        if (seahorse && seahorse->IsAIEnabled())
            seahorse->AI()->SetData(DATA_RODEO_RESPONSE, GetSpellInfo()->Id);
    }

    void Register() override
    {
        AfterHit.Register(&spell_vashjir_seahorse_rodeo_response::HandleAfterHit);
    }
};

/*######
## Quest 25657 - Dah, Nunt... Dah, Nunt... (Gnaws bait event)
## Quest 25670 - DUN-dun-DUN-dun-DUN-dun (Gnaws-II harpoon event)
## Quest 25743 - Decisions, Decisions (Pewter Prophet / Budd farewell)
######*/

enum GnawsCreatures
{
    NPC_BAIT_BUNNY                  = 41051, // Vehicle 803, per-player event director
    NPC_BOOBY_TRAPPED_BAIT          = 41048,
    NPC_CLONED_IMAGE                = 41085, // mirror-image stand-in body
    NPC_GIANT_SEA_GRUB              = 41042,
    NPC_GNAWS                       = 41057, // Vehicle 804
    NPC_MOUTH_BLOOD_BUNNY           = 46403,
    NPC_PLAYER_BAIT                 = 41093, // Vehicle 807
    NPC_HARPOON_CHAIN_BUNNY         = 46460,
    NPC_GNAWS_II                    = 41098, // Vehicle 808
    NPC_GNAWS_BLOOD_BUNNY           = 41150,
    NPC_GNAWS_HARPOON_BUNNY         = 41154,
    NPC_THE_PEWTER_PROPHET          = 41192
};

enum GnawsSpells
{
    // 25657 bait event
    SPELL_CLONE_ME                  = 45204,
    SPELL_GNAWS_SUMMON_TIMER        = 76697, // dummy aura on summoner (flavor)
    SPELL_REVERSE_CAST_RIDE_BAIT    = 76744, // script BP 76745 at summoner
    SPELL_SUMMON_GNAWS              = 76707, // native: 41057 at dest-db
    SPELL_INVISIBLE_BEAM            = 86424,
    SPELL_RIDE_VEHICLE_HARDCODED    = 46598,
    SPELL_RIDE_VEHICLE_BAIT         = 43671, // bait into Gnaws' mouth
    SPELL_BAIT_BUNNY_EXPLOSION      = 76731,
    SPELL_GNAWS_WINCE               = 86433, // dummy at own vehicle
    SPELL_GNAWS_TOOTH_00            = 76736,
    SPELL_GNAWS_TOOTH_01            = 76738,
    SPELL_GNAWS_TOOTH_02            = 76739,
    SPELL_BLOODY_EXPLOSION_LARGE    = 86438,
    SPELL_GNAWS_KILL_CREDIT         = 76747, // eff0 script -> 76761, eff1 KC 41057, eff2 teleport dest-db
    SPELL_SEE_GNAWS_TEETH           = 76761,

    // 25670 harpoon event
    SPELL_SUMMON_HARPOON_CHAIN      = 86542, // native: 46460 at dest-db
    SPELL_RIDE_BAIT_BUNNY           = 76853,
    SPELL_FORCE_RIDE_PLAYER_BAIT    = 76799, // script BP 76853 at summoner
    SPELL_GNAWS_II_TIMER            = 86509, // dummy aura on summoner (flavor)
    SPELL_GNAWS_BAIT_PULSE          = 75394,
    SPELL_SUMMON_GNAWS_II           = 76819, // native: 41098 at dest-db
    SPELL_TRACK_GNAWS_CHANNEL       = 86520,
    SPELL_CHOMP_WARMUP              = 76878,
    SPELL_FORCE_RIDE_GNAWS_II       = 76854, // script BP 86521 at summoner
    SPELL_INVIS_PASSENGER           = 76884,
    SPELL_GNAWS_CHOMP_PERIODIC      = 76842, // ticks 76844 (native 10% max-hp chomp)
    SPELL_HARPOON_TO_GNAWS          = 76882,
    SPELL_CANCEL_CHOMP_PERIODIC     = 76996, // native: strips 76842
    SPELL_FORCE_CAMERA_SEAT         = 82581, // script BP 82582 at summoner
    SPELL_BLOODY_EXPLOSION_HUGE     = 76989,
    SPELL_RED_RADIATION_LARGE       = 75280,
    SPELL_SPEAR_BUNNY_TRANSFORM     = 76991,
    SPELL_GNAWS_WRAP_UP             = 77000,
    SPELL_GNAWS_DEATH_ANIM          = 77499,
    SPELL_GNAWS_DEATH_SENTENCE      = 86523,
    SPELL_FEIGN_DEATH_DROWNED       = 58806,
    SPELL_GNAWS_II_KILL_CREDIT      = 77004, // native: KC 41098
    SPELL_SEE_GNAWS_CORPSE          = 77005,

    // 25743 Pewter Prophet
    SPELL_POUND_PEWTER              = 77275,
    SPELL_PEWTER_QUEST_COMPLETION   = 77281, // native: KC 41192 + strip 77207
    SPELL_SEE_HARRISON_FINAL        = 77282,
    SPELL_PING_BUDD                 = 86599
};

enum GnawsTexts
{
    SAY_FIRE_HARPOON_GUN            = 0, // 41098 boss whisper "|cFFFF2222Fire Harpoon Gun!|r"
    SAY_BUDD_FAREWELL               = 0  // 46463 "Farewell, friend! Good treasure hunting!"
};

enum GnawsPoints
{
    POINT_GNAWS_DIVE                = 7,
    POINT_GNAWS_EXIT,
    POINT_GNAWS_II_APPROACH,
    POINT_GNAWS_II_DEATH,
    POINT_BUDD_FAREWELL
};

enum GnawsData
{
    DATA_GNAWS_ARRIVED              = 1,
    DATA_GNAWS_EXIT                 = 2
};

// 41057/41098 arc from the dest-db summon point up over the reef and down onto the bait (sniff X/Z, Y interpolated)
Position const GnawsDivePath[] =
{
    { -5010.00f, 3407.00f, -104.00f },
    { -4995.90f, 3393.30f, -86.00f  },
    { -4981.00f, 3396.50f, -68.00f  },
    { -4968.00f, 3400.00f, -62.00f  },
    { -4944.00f, 3404.50f, -67.00f  },
    { -4931.00f, 3408.00f, -88.00f  },
    { -4926.00f, 3410.00f, -96.00f  },
    { -4928.00f, 3412.00f, -102.00f },
    { -4929.20f, 3413.70f, -104.30f },
    { -4931.50f, 3432.40f, -103.50f }
};

Position const GnawsExitPath[] =
{
    { -4946.0f, 3456.0f, -102.0f }
};

// harpooned Gnaws death drag to the corpse-scene spot at Gurboggle's Ledge (sniff 20s)
Position const GnawsDeathDragPath[] =
{
    { -5010.00f, 3402.00f, -95.00f  },
    { -5079.00f, 3371.00f, -94.00f  },
    { -5127.18f, 3382.56f, -111.25f }
};

Position const BuddFarewellPos = { -4979.5f, 3463.6f, -110.4f };

float constexpr GNAWS_DIVE_VELOCITY = 24.0f;
float constexpr GNAWS_EXIT_VELOCITY = 14.0f;
float constexpr GNAWS_DRAG_VELOCITY = 10.5f;

// all shark-arc actors are per-player summons - resolve them through their summoner
Creature* FindPlayerSummon(Creature* source, uint32 entry, ObjectGuid summonerGUID)
{
    std::list<Creature*> summons;
    source->GetCreatureListWithEntryInGrid(summons, entry, 150.0f);
    for (Creature* creature : summons)
        if (TempSummon* summon = creature->ToTempSummon())
            if (summon->GetSummonerGUID() == summonerGUID)
                return creature;
    return nullptr;
}

/*######
## npc_gnaws_bait_bunny - 41051 (25657 event director, summoned by 76694)
######*/

enum GnawsBaitEvents
{
    EVENT_BAIT_CAMERA = 1,
    EVENT_BAIT_SUMMON_GNAWS,
    EVENT_BAIT_RELEASE_PLAYER,
    EVENT_BAIT_EAT,
    EVENT_BAIT_TEETH,
    EVENT_BAIT_IMAGE_DESPAWN,
    EVENT_BAIT_FINISH,
    EVENT_BAIT_FAILSAFE
};

struct npc_gnaws_bait_bunny : public NullCreatureAI
{
    npc_gnaws_bait_bunny(Creature* creature) : NullCreatureAI(creature), _arrived(false) { }

    void IsSummonedBy(Unit* summoner) override
    {
        Player* player = summoner->ToPlayer();
        if (!player)
        {
            me->DespawnOrUnsummon();
            return;
        }

        _playerGUID = player->GetGUID();
        me->SetDisableGravity(true);

        // bait rides my platform seat, the explosive grub crawls next to it
        if (Creature* bait = me->SummonCreature(NPC_BOOBY_TRAPPED_BAIT, me->GetPosition(), TEMPSUMMON_TIMED_DESPAWN, 1min))
        {
            _baitGUID = bait->GetGUID();
            bait->EnterVehicle(me, 0);
        }
        if (Creature* grub = me->SummonCreature(NPC_GIANT_SEA_GRUB, me->GetNearPosition(4.0f, 1.5f), TEMPSUMMON_TIMED_DESPAWN, 1min))
            _grubGUID = grub->GetGUID();

        // sniff offsets from the 76694 summons
        _events.ScheduleEvent(EVENT_BAIT_CAMERA, 1s + 200ms);
        _events.ScheduleEvent(EVENT_BAIT_SUMMON_GNAWS, 5s + 200ms);
        _events.ScheduleEvent(EVENT_BAIT_RELEASE_PLAYER, 6s + 300ms);
        _events.ScheduleEvent(EVENT_BAIT_FAILSAFE, 45s);
    }

    void SetData(uint32 id, uint32 /*value*/) override
    {
        if (id != DATA_GNAWS_ARRIVED || _arrived)
            return;
        _arrived = true;

        if (Creature* grub = ObjectAccessor::GetCreature(*me, _grubGUID))
            grub->DespawnOrUnsummon(); // slurped up first

        _events.ScheduleEvent(EVENT_BAIT_EAT, 1s + 200ms);
        _events.ScheduleEvent(EVENT_BAIT_TEETH, 2s + 800ms);
        _events.ScheduleEvent(EVENT_BAIT_IMAGE_DESPAWN, 4s);
        _events.ScheduleEvent(EVENT_BAIT_FINISH, 12s);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_BAIT_CAMERA:
                    me->CastSpell(nullptr, SPELL_GNAWS_SUMMON_TIMER, true);     // flavor timer aura on the summoner
                    me->CastSpell(nullptr, SPELL_REVERSE_CAST_RIDE_BAIT, true); // -> player casts 76745, boards camera seat
                    break;
                case EVENT_BAIT_SUMMON_GNAWS:
                    if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID))
                    {
                        player->CastSpell(nullptr, SPELL_SUMMON_GNAWS, true); // dest-db, the player owns the shark
                        if (Creature* gnaws = FindPlayerSummon(me, NPC_GNAWS, _playerGUID))
                        {
                            _gnawsGUID = gnaws->GetGUID();
                            me->CastSpell(gnaws, SPELL_INVISIBLE_BEAM, true);
                        }
                    }
                    break;
                case EVENT_BAIT_RELEASE_PLAYER:
                    // retail: the camera ends right after Gnaws appears - teleport back + credit + tooth vision
                    if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID))
                    {
                        player->ExitVehicle();
                        me->CastSpell(player, SPELL_GNAWS_KILL_CREDIT, true);
                        player->SetControlled(false, UNIT_STATE_ROOT);
                    }
                    break;
                case EVENT_BAIT_EAT:
                    if (Creature* bait = ObjectAccessor::GetCreature(*me, _baitGUID))
                    {
                        if (Creature* gnaws = ObjectAccessor::GetCreature(*me, _gnawsGUID))
                        {
                            bait->ExitVehicle();
                            bait->CastSpell(gnaws, SPELL_RIDE_VEHICLE_BAIT, true);
                            bait->CastSpell(nullptr, SPELL_GNAWS_WINCE, true);
                        }
                        bait->DespawnOrUnsummon(600ms);
                    }
                    DoCastSelf(SPELL_BAIT_BUNNY_EXPLOSION, true);
                    if (Creature* gnaws = ObjectAccessor::GetCreature(*me, _gnawsGUID))
                        if (gnaws->IsAIEnabled())
                            gnaws->AI()->SetData(DATA_GNAWS_EXIT, 1);
                    break;
                case EVENT_BAIT_TEETH:
                    for (uint8 i = 0; i < 2; ++i)
                    {
                        DoCastSelf(SPELL_GNAWS_TOOTH_00, true);
                        DoCastSelf(SPELL_GNAWS_TOOTH_01, true);
                        DoCastSelf(SPELL_GNAWS_TOOTH_02, true);
                    }
                    DoCastSelf(SPELL_BAIT_BUNNY_EXPLOSION, true);
                    break;
                case EVENT_BAIT_IMAGE_DESPAWN:
                    if (Creature* image = FindPlayerSummon(me, NPC_CLONED_IMAGE, _playerGUID))
                        image->DespawnOrUnsummon();
                    break;
                case EVENT_BAIT_FINISH:
                case EVENT_BAIT_FAILSAFE:
                    Cleanup();
                    break;
                default:
                    break;
            }
        }
    }

private:
    void Cleanup()
    {
        if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID))
            player->SetControlled(false, UNIT_STATE_ROOT);
        for (ObjectGuid guid : { _baitGUID, _grubGUID })
            if (Creature* summon = ObjectAccessor::GetCreature(*me, guid))
                summon->DespawnOrUnsummon();
        if (Creature* image = FindPlayerSummon(me, NPC_CLONED_IMAGE, _playerGUID))
            image->DespawnOrUnsummon();
        me->DespawnOrUnsummon();
    }

    EventMap _events;
    ObjectGuid _playerGUID;
    ObjectGuid _baitGUID;
    ObjectGuid _grubGUID;
    ObjectGuid _gnawsGUID;
    bool _arrived;
};

/*######
## npc_gnaws_cloned_image - 41085 (stand-in body, summoned by 76694)
######*/

struct npc_gnaws_cloned_image : public NullCreatureAI
{
    npc_gnaws_cloned_image(Creature* creature) : NullCreatureAI(creature) { }

    void IsSummonedBy(Unit* summoner) override
    {
        if (Player* player = summoner->ToPlayer())
            player->CastSpell(me, SPELL_CLONE_ME, true); // wear the player's looks while he rides the camera
        me->DespawnOrUnsummon(1min); // the director despawns me earlier
    }
};

/*######
## npc_gnaws - 41057 (25657 shark, summoned by 76707)
######*/

enum GnawsEvents
{
    EVENT_GNAWS_BUNNY_BOOM = 1,
    EVENT_GNAWS_DESPAWN
};

struct npc_gnaws : public PassiveAI
{
    npc_gnaws(Creature* creature) : PassiveAI(creature) { }

    void IsSummonedBy(Unit* summoner) override
    {
        Player* player = summoner->ToPlayer();
        if (!player)
        {
            me->DespawnOrUnsummon();
            return;
        }

        _playerGUID = player->GetGUID();
        me->SetReactState(REACT_PASSIVE);
        me->SetDisableGravity(true);

        // mouth blood bunnies ride along inside the maw
        for (ObjectGuid& bunnyGUID : _bunnyGUIDs)
        {
            if (Creature* bunny = me->SummonCreature(NPC_MOUTH_BLOOD_BUNNY, me->GetPosition(), TEMPSUMMON_TIMED_DESPAWN, 1min))
            {
                bunnyGUID = bunny->GetGUID();
                bunny->CastSpell(me, SPELL_RIDE_VEHICLE_HARDCODED, true);
            }
        }

        me->GetMotionMaster()->MoveSmoothPath(POINT_GNAWS_DIVE, GnawsDivePath, std::size(GnawsDivePath), false, true, GNAWS_DIVE_VELOCITY);
        _events.ScheduleEvent(EVENT_GNAWS_DESPAWN, 40s); // failsafe
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type != EFFECT_MOTION_TYPE || id != POINT_GNAWS_DIVE)
            return;

        if (Creature* director = FindPlayerSummon(me, NPC_BAIT_BUNNY, _playerGUID))
            if (director->IsAIEnabled())
                director->AI()->SetData(DATA_GNAWS_ARRIVED, 1);
    }

    void SetData(uint32 id, uint32 /*value*/) override
    {
        if (id != DATA_GNAWS_EXIT)
            return;

        me->GetMotionMaster()->MoveSmoothPath(POINT_GNAWS_EXIT, GnawsExitPath, std::size(GnawsExitPath), false, true, GNAWS_EXIT_VELOCITY);
        _events.ScheduleEvent(EVENT_GNAWS_BUNNY_BOOM, 2s);
        _events.RescheduleEvent(EVENT_GNAWS_DESPAWN, 14s);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_GNAWS_BUNNY_BOOM:
                    for (ObjectGuid guid : _bunnyGUIDs)
                    {
                        if (Creature* bunny = ObjectAccessor::GetCreature(*me, guid))
                        {
                            bunny->CastSpell(bunny, SPELL_BLOODY_EXPLOSION_LARGE, true);
                            bunny->DespawnOrUnsummon(5s);
                        }
                    }
                    break;
                case EVENT_GNAWS_DESPAWN:
                    me->DespawnOrUnsummon();
                    break;
                default:
                    break;
            }
        }
    }

private:
    EventMap _events;
    ObjectGuid _playerGUID;
    ObjectGuid _bunnyGUIDs[3];
};

/*######
## npc_player_bait_bunny - 41093 (25670 buoy rig, summoned by 76795)
######*/

enum PlayerBaitEvents
{
    EVENT_PLAYER_BAIT_SUMMON_GNAWS = 1
};

struct npc_player_bait_bunny : public NullCreatureAI
{
    npc_player_bait_bunny(Creature* creature) : NullCreatureAI(creature) { }

    void IsSummonedBy(Unit* summoner) override
    {
        Player* player = summoner->ToPlayer();
        if (!player)
        {
            me->DespawnOrUnsummon();
            return;
        }

        _playerGUID = player->GetGUID();
        me->SetDisableGravity(true);
        me->CastSpell(nullptr, SPELL_FORCE_RIDE_PLAYER_BAIT, true); // -> player casts 76853, chains himself to the rig
        me->CastSpell(nullptr, SPELL_GNAWS_II_TIMER, true);         // flavor 10s timer aura

        _events.ScheduleEvent(EVENT_PLAYER_BAIT_SUMMON_GNAWS, 10s);
        me->DespawnOrUnsummon(4min); // failsafe - the Gnaws II script despawns me at the death drag
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            if (eventId == EVENT_PLAYER_BAIT_SUMMON_GNAWS)
            {
                if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID))
                {
                    player->CastSpell(nullptr, SPELL_SUMMON_GNAWS_II, true); // dest-db
                    if (Creature* gnaws = FindPlayerSummon(me, NPC_GNAWS_II, _playerGUID))
                        me->CastSpell(gnaws, SPELL_TRACK_GNAWS_CHANNEL, true);
                }
            }
        }
    }

private:
    EventMap _events;
    ObjectGuid _playerGUID;
};

/*######
## npc_harpoon_chain_bunny - 46460 (25670 chain anchor, summoned by 86542)
######*/

enum ChainBunnyEvents
{
    EVENT_CHAIN_PULSE = 1
};

struct npc_harpoon_chain_bunny : public NullCreatureAI
{
    npc_harpoon_chain_bunny(Creature* creature) : NullCreatureAI(creature) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        _events.ScheduleEvent(EVENT_CHAIN_PULSE, 2s);
        me->DespawnOrUnsummon(4min); // failsafe - the Gnaws II script despawns me at the death drag
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            if (eventId == EVENT_CHAIN_PULSE)
            {
                me->CastSpell(nullptr, SPELL_GNAWS_BAIT_PULSE, true); // dummy ping at the summoner
                _events.ScheduleEvent(EVENT_CHAIN_PULSE, 4s + 500ms);
            }
        }
    }

private:
    EventMap _events;
};

/*######
## npc_gnaws_ii - 41098 (25670 shark, summoned by 76819; 76859 sits on its vehicle bar)
######*/

enum GnawsIIEvents
{
    EVENT_GNAWS2_MOUTH = 1,
    EVENT_GNAWS2_CHOMP,
    EVENT_GNAWS2_WHISPER,
    EVENT_GNAWS2_DRAG,
    EVENT_GNAWS2_FEIGN,
    EVENT_GNAWS2_CREDIT,
    EVENT_GNAWS2_DESPAWN
};

struct npc_gnaws_ii : public PassiveAI
{
    npc_gnaws_ii(Creature* creature) : PassiveAI(creature), _dead(false) { }

    void IsSummonedBy(Unit* summoner) override
    {
        Player* player = summoner->ToPlayer();
        if (!player)
        {
            me->DespawnOrUnsummon();
            return;
        }

        _playerGUID = player->GetGUID();
        me->SetReactState(REACT_PASSIVE);
        me->SetDisableGravity(true);

        if (Creature* blood = me->SummonCreature(NPC_GNAWS_BLOOD_BUNNY, me->GetPosition(), TEMPSUMMON_TIMED_DESPAWN, 5min))
        {
            _bloodBunnyGUID = blood->GetGUID();
            blood->CastSpell(me, SPELL_RIDE_VEHICLE_HARDCODED, true);
        }
        if (Creature* harpoon = me->SummonCreature(NPC_GNAWS_HARPOON_BUNNY, me->GetPosition(), TEMPSUMMON_TIMED_DESPAWN, 5min))
        {
            _harpoonBunnyGUID = harpoon->GetGUID();
            harpoon->CastSpell(me, SPELL_RIDE_VEHICLE_HARDCODED, true);
        }

        me->GetMotionMaster()->MoveSmoothPath(POINT_GNAWS_II_APPROACH, GnawsDivePath, std::size(GnawsDivePath), false, true, GNAWS_DIVE_VELOCITY);
        me->DespawnOrUnsummon(4min); // failsafe
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type != EFFECT_MOTION_TYPE)
            return;

        switch (id)
        {
            case POINT_GNAWS_II_APPROACH:
                DoCastSelf(SPELL_CHOMP_WARMUP, true);
                _events.ScheduleEvent(EVENT_GNAWS2_MOUTH, 2s);
                break;
            case POINT_GNAWS_II_DEATH:
                me->CastSpell(nullptr, SPELL_GNAWS_WRAP_UP, true); // dummy at the summoner
                DoCastSelf(SPELL_GNAWS_DEATH_ANIM, true);
                DoCastSelf(SPELL_GNAWS_DEATH_SENTENCE, true);
                _events.ScheduleEvent(EVENT_GNAWS2_FEIGN, 2s + 500ms);
                _events.ScheduleEvent(EVENT_GNAWS2_CREDIT, 4s + 500ms);
                _events.ScheduleEvent(EVENT_GNAWS2_DESPAWN, 7s);
                break;
            default:
                break;
        }
    }

    void SpellHit(WorldObject* /*caster*/, SpellInfo const* spellInfo) override
    {
        if (spellInfo->Id != SPELL_HARPOON_TO_GNAWS || _dead)
            return;

        _dead = true;
        _events.Reset();

        DoCastSelf(SPELL_CANCEL_CHOMP_PERIODIC, true);         // native: strips 76842
        me->CastSpell(nullptr, SPELL_FORCE_CAMERA_SEAT, true); // -> player casts 82582, camera seat
        if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID))
            player->RemoveAurasDueToSpell(SPELL_INVIS_PASSENGER);

        if (Creature* blood = ObjectAccessor::GetCreature(*me, _bloodBunnyGUID))
        {
            blood->CastSpell(blood, SPELL_BLOODY_EXPLOSION_HUGE, true);
            blood->CastSpell(blood, SPELL_RED_RADIATION_LARGE, true);
        }
        if (Creature* harpoon = ObjectAccessor::GetCreature(*me, _harpoonBunnyGUID))
            harpoon->CastSpell(harpoon, SPELL_SPEAR_BUNNY_TRANSFORM, true);

        // the buoy rig is done for
        if (Creature* playerBait = FindPlayerSummon(me, NPC_PLAYER_BAIT, _playerGUID))
            playerBait->DespawnOrUnsummon(2s);
        if (Creature* chain = FindPlayerSummon(me, NPC_HARPOON_CHAIN_BUNNY, _playerGUID))
            chain->DespawnOrUnsummon(2s);

        _events.ScheduleEvent(EVENT_GNAWS2_DRAG, 1s + 500ms);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_GNAWS2_MOUTH:
                    me->CastSpell(nullptr, SPELL_FORCE_RIDE_GNAWS_II, true); // -> player casts 86521, into the maw
                    me->CastSpell(nullptr, SPELL_INVIS_PASSENGER, true);
                    _events.ScheduleEvent(EVENT_GNAWS2_CHOMP, 5s);
                    _events.ScheduleEvent(EVENT_GNAWS2_WHISPER, 12s);
                    break;
                case EVENT_GNAWS2_CHOMP:
                    DoCastSelf(SPELL_GNAWS_CHOMP_PERIODIC, true);
                    break;
                case EVENT_GNAWS2_WHISPER:
                    if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID))
                        Talk(SAY_FIRE_HARPOON_GUN, player);
                    _events.ScheduleEvent(EVENT_GNAWS2_WHISPER, 20s); // nag until the button is pressed
                    break;
                case EVENT_GNAWS2_DRAG:
                    me->GetMotionMaster()->MoveSmoothPath(POINT_GNAWS_II_DEATH, GnawsDeathDragPath, std::size(GnawsDeathDragPath), false, true, GNAWS_DRAG_VELOCITY);
                    break;
                case EVENT_GNAWS2_FEIGN:
                    DoCastSelf(SPELL_FEIGN_DEATH_DROWNED, true);
                    break;
                case EVENT_GNAWS2_CREDIT:
                    if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID))
                    {
                        player->CastSpell(player, SPELL_GNAWS_II_KILL_CREDIT, true);
                        player->CastSpell(player, SPELL_SEE_GNAWS_CORPSE, true);
                        player->ExitVehicle();
                    }
                    break;
                case EVENT_GNAWS2_DESPAWN:
                    for (ObjectGuid guid : { _bloodBunnyGUID, _harpoonBunnyGUID })
                        if (Creature* bunny = ObjectAccessor::GetCreature(*me, guid))
                            bunny->DespawnOrUnsummon();
                    me->DespawnOrUnsummon();
                    break;
                default:
                    break;
            }
        }
    }

private:
    EventMap _events;
    ObjectGuid _playerGUID;
    ObjectGuid _bloodBunnyGUID;
    ObjectGuid _harpoonBunnyGUID;
    bool _dead;
};

/*######
## npc_pewter_prophet - 41192 (25743, DB spawn - pounded with 77275)
######*/

struct npc_pewter_prophet : public NullCreatureAI
{
    npc_pewter_prophet(Creature* creature) : NullCreatureAI(creature) { }

    void SpellHit(WorldObject* caster, SpellInfo const* spellInfo) override
    {
        if (spellInfo->Id != SPELL_POUND_PEWTER)
            return;

        if (Unit* unitCaster = caster->ToUnit())
            me->CastSpell(unitCaster, SPELL_PEWTER_QUEST_COMPLETION, true); // native KC + 77207 strip (77282 via spell script)
    }
};

/*######
## npc_budd_farewell - 46463 (25743 turn-in Budd, permanent phased spawn - pinged by reward spell 86599)
######*/

enum BuddFarewellEvents
{
    EVENT_BUDD_SWIM_OFF = 1
};

struct npc_budd_farewell : public PassiveAI
{
    npc_budd_farewell(Creature* creature) : PassiveAI(creature), _leaving(false) { }

    void JustAppeared() override
    {
        PassiveAI::JustAppeared();
        _leaving = false;
    }

    void SpellHit(WorldObject* /*caster*/, SpellInfo const* spellInfo) override
    {
        if (spellInfo->Id != SPELL_PING_BUDD || _leaving)
            return;

        _leaving = true;
        Talk(SAY_BUDD_FAREWELL);
        _events.ScheduleEvent(EVENT_BUDD_SWIM_OFF, 1s + 400ms);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            if (eventId == EVENT_BUDD_SWIM_OFF)
            {
                me->GetMotionMaster()->MovePoint(POINT_BUDD_FAREWELL, BuddFarewellPos);
                me->DespawnOrUnsummon(11s, 60s); // back for the next treasure hunter
            }
        }
    }

private:
    EventMap _events;
    bool _leaving;
};

/*######
## Quest 25794 - Undersea Sanctuary (Watery Vision tour)
## Quest 25812 - Spelunking (arrival RP trigger)
## Quest 25887 - Wake of Destruction (Dominated Great Shark)
######*/

enum SanctuaryCreatures
{
    NPC_WATERY_VISION               = 41294, // Vehicle 818
    NPC_FAMISHED_GREAT_SHARK        = 41998,
    NPC_DOMINATED_GREAT_SHARK       = 42013, // Vehicle 744
    NPC_ZINJATAR_PEN_GUARDIAN       = 41996  // phase-170 flavor set, SAI gores on 78303
};

enum SanctuarySpells
{
    // 25794 vision
    SPELL_SUMMON_WATERY_VISION      = 77379,
    SPELL_FORCE_MASTER_RIDE_VISION  = 77418, // script -> summoner casts 77419
    SPELL_RIDE_VISION               = 77419,
    SPELL_SCRYING_SCREEN            = 77375, // screen effect + vision limbo phase
    SPELL_VISION_TELEPORT           = 77376, // native dest-db (tour start)
    SPELL_VISION_TELEPORT_RETURN    = 77377, // native dest-db (back to the raft)
    SPELL_SEE_IMPRISONED_SOLDIERS   = 78118,

    // 25812 reward
    SPELL_SUMMON_ERUNAK_DEEPMIST    = 77429, // native dest-db
    SPELL_SUMMON_MOANAH_DEEPMIST    = 77430, // native dest-db
    SPELL_SUMMON_RENDEL_DEEPMIST    = 77432, // native dest-db

    // 25887 shark
    SPELL_DOMINATE_CREATURE         = 78287, // item spell, periodic-triggers 78288
    SPELL_FORCE_RIDE_SHARK          = 78289, // flavor dummy on the summoner
    SPELL_RIDE_FAMISHED_SHARK       = 78290,
    SPELL_SHARK_QUEST_CHECK         = 78307,
    SPELL_EAT_ME                    = 78302,
    SPELL_EAT_NAGA                  = 78296,
    SPELL_EAT_NAGA_SPELL            = 78303,
    SPELL_SHARK_KILL_CREDIT         = 78304,
    SPELL_RIDER_CHEER               = 78314,
    SPELL_ENABLE_RETURN             = 78308,
    SPELL_CLEAR_SHARK_AURAS         = 78311, // native: strips 78307 + pens phase 78271
    SPELL_SWIM_SPEED_BOOST          = 83159,
    SPELL_POOF                      = 75029,
    SPELL_PERMANENT_FEIGN_GORE      = 58951  // guardian SAI gore state - scan filter
};

enum SanctuaryTexts
{
    WHISPER_SHARK_DEVOUR            = 0 // 42013 "Ride into the Holding Pens and DEVOUR naga!"
};

enum SanctuaryPoints
{
    POINT_VISION_TOUR               = 12
};

uint8 constexpr SHARK_REQUIRED_KILLS = 12;

// 25794 vision flight (sniff spline, 40.5s retail move time)
Position const VisionFlightPath[] =
{
    { -5237.20f, 3481.59f, -130.06f },
    { -5248.90f, 3482.00f, -130.06f },
    { -5273.89f, 3479.94f, -126.17f },
    { -5298.89f, 3467.63f, -120.43f },
    { -5325.23f, 3437.45f, -118.37f },
    { -5324.89f, 3406.11f, -116.20f },
    { -5298.71f, 3375.92f, -122.48f },
    { -5249.27f, 3370.59f, -130.12f },
    { -5218.66f, 3363.98f, -136.57f },
    { -5192.74f, 3336.51f, -127.16f },
    { -5163.84f, 3309.21f, -119.39f },
    { -5148.89f, 3294.63f, -114.72f }
};

float constexpr VISION_FLIGHT_VELOCITY = 9.4f;

/*######
## npc_watery_vision - 41294 (25794 camera vehicle, summoned by 77379)
######*/

enum WateryVisionEvents
{
    EVENT_VISION_TELEPORT = 1,
    EVENT_VISION_FLIGHT,
    EVENT_VISION_DETECT,
    EVENT_VISION_FINISH
};

struct npc_watery_vision : public PassiveAI
{
    npc_watery_vision(Creature* creature) : PassiveAI(creature), _finished(false) { }

    void IsSummonedBy(Unit* summoner) override
    {
        Player* player = summoner->ToPlayer();
        if (!player)
        {
            me->DespawnOrUnsummon();
            return;
        }

        _playerGUID = player->GetGUID();
        me->SetReactState(REACT_PASSIVE);
        me->SetDisableGravity(true);
        me->CastSpell(nullptr, SPELL_FORCE_MASTER_RIDE_VISION, true); // -> player casts 77419 and boards
        me->DespawnOrUnsummon(2min); // failsafe
    }

    void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
    {
        if (passenger->GetTypeId() != TYPEID_PLAYER)
            return;

        if (apply)
            _events.ScheduleEvent(EVENT_VISION_TELEPORT, 1s);
        else if (!_finished)
            me->DespawnOrUnsummon(2s); // bailed out early - no credit
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type != EFFECT_MOTION_TYPE || id != POINT_VISION_TOUR)
            return;

        _finished = true;
        DoCastSelf(SPELL_VISION_TELEPORT_RETURN, true); // vehicle + rider back to the raft
        _events.ScheduleEvent(EVENT_VISION_FINISH, 1s);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_VISION_TELEPORT:
                    if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID))
                        me->CastSpell(player, SPELL_SCRYING_SCREEN, true);
                    DoCastSelf(SPELL_VISION_TELEPORT, true); // vehicle + rider to the tour start
                    _events.ScheduleEvent(EVENT_VISION_FLIGHT, 1s + 500ms);
                    break;
                case EVENT_VISION_FLIGHT:
                    me->GetMotionMaster()->MoveSmoothPath(POINT_VISION_TOUR, VisionFlightPath, std::size(VisionFlightPath), false, true, VISION_FLIGHT_VELOCITY);
                    _events.ScheduleEvent(EVENT_VISION_DETECT, 4s);
                    break;
                case EVENT_VISION_DETECT:
                    if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID))
                        player->CastSpell(player, SPELL_SEE_IMPRISONED_SOLDIERS, true); // caged 41548 scenes along the route
                    break;
                case EVENT_VISION_FINISH:
                    if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID))
                    {
                        player->KilledMonsterCredit(NPC_WATERY_VISION);
                        player->RemoveAurasDueToSpell(SPELL_SCRYING_SCREEN);
                        player->ExitVehicle();
                    }
                    me->DespawnOrUnsummon(5s);
                    break;
                default:
                    break;
            }
        }
    }

private:
    EventMap _events;
    ObjectGuid _playerGUID;
    bool _finished;
};

/*######
## npc_dominated_great_shark - 42013 (25887 ride, summoned by 78288)
######*/

enum SharkEvents
{
    EVENT_SHARK_SCAN = 1,
    EVENT_SHARK_EAT,
    EVENT_SHARK_GORE,
    EVENT_SHARK_COMPLETE
};

struct npc_dominated_great_shark : public PassiveAI
{
    npc_dominated_great_shark(Creature* creature) : PassiveAI(creature), _kills(0), _dismissed(false) { }

    void IsSummonedBy(Unit* summoner) override
    {
        Player* player = summoner->ToPlayer();
        if (!player)
        {
            me->DespawnOrUnsummon();
            return;
        }

        _playerGUID = player->GetGUID();
        _homePos = me->GetPosition();
        me->SetReactState(REACT_PASSIVE);
        me->SetDisableGravity(true);
        DoCastSelf(SPELL_POOF, true);
        DoCastSelf(SPELL_SWIM_SPEED_BOOST, true);
        me->CastSpell(nullptr, SPELL_FORCE_RIDE_SHARK, true); // flavor dummy on the summoner
        player->CastSpell(me, SPELL_RIDE_FAMISHED_SHARK, true);
        me->DespawnOrUnsummon(10min); // failsafe
    }

    void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
    {
        Player* player = passenger->ToPlayer();
        if (!player)
            return;

        if (apply)
        {
            Talk(WHISPER_SHARK_DEVOUR, player);
            me->CastSpell(player, SPELL_SHARK_QUEST_CHECK, true);
            _events.ScheduleEvent(EVENT_SHARK_SCAN, 2s);
        }
        else
            Dismiss();
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_SHARK_SCAN:
                    if (Creature* guardian = FindGuardianSnack())
                    {
                        _victimGUID = guardian->GetGUID();
                        guardian->CastSpell(me, SPELL_EAT_ME, true);
                        _events.ScheduleEvent(EVENT_SHARK_EAT, 800ms);
                    }
                    else
                        _events.ScheduleEvent(EVENT_SHARK_SCAN, 2s);
                    break;
                case EVENT_SHARK_EAT:
                    if (Creature* guardian = ObjectAccessor::GetCreature(*me, _victimGUID))
                        me->CastSpell(guardian, SPELL_EAT_NAGA, true);
                    _events.ScheduleEvent(EVENT_SHARK_GORE, 700ms);
                    break;
                case EVENT_SHARK_GORE:
                {
                    if (Creature* guardian = ObjectAccessor::GetCreature(*me, _victimGUID))
                    {
                        _eatenGUIDs.insert(guardian->GetGUID());
                        me->CastSpell(guardian, SPELL_EAT_NAGA_SPELL, true); // guardian SAI gores itself
                    }
                    if (Player* rider = ObjectAccessor::GetPlayer(*me, _playerGUID))
                    {
                        me->CastSpell(rider, SPELL_SHARK_KILL_CREDIT, true);
                        me->CastSpell(rider, SPELL_RIDER_CHEER, true);
                        if (++_kills >= SHARK_REQUIRED_KILLS)
                        {
                            rider->CastSpell(nullptr, SPELL_ENABLE_RETURN, true); // lands on his vehicle - me
                            _events.ScheduleEvent(EVENT_SHARK_COMPLETE, 4s);
                            break;
                        }
                    }
                    _events.ScheduleEvent(EVENT_SHARK_SCAN, 2s);
                    break;
                }
                case EVENT_SHARK_COMPLETE:
                    Dismiss();
                    break;
                default:
                    break;
            }
        }
    }

private:
    Creature* FindGuardianSnack() const
    {
        std::list<Creature*> guardians;
        me->GetCreatureListWithEntryInGrid(guardians, NPC_ZINJATAR_PEN_GUARDIAN, 8.0f);
        for (Creature* guardian : guardians)
            if (guardian->IsAlive() && !guardian->HasAura(SPELL_PERMANENT_FEIGN_GORE) && _eatenGUIDs.find(guardian->GetGUID()) == _eatenGUIDs.end())
                return guardian;
        return nullptr;
    }

    void Dismiss()
    {
        if (_dismissed)
            return;
        _dismissed = true;
        _events.Reset();

        if (Player* rider = ObjectAccessor::GetPlayer(*me, _playerGUID))
            me->CastSpell(rider, SPELL_CLEAR_SHARK_AURAS, true); // native: quest-check + pens phase drop
        if (Vehicle* kit = me->GetVehicleKit())
            kit->RemoveAllPassengers();

        // hand the spot back to a famished shark for the next rider
        me->SummonCreature(NPC_FAMISHED_GREAT_SHARK, _homePos, TEMPSUMMON_TIMED_DESPAWN, 5min);
        me->DespawnOrUnsummon(2s);
    }

    EventMap _events;
    ObjectGuid _playerGUID;
    ObjectGuid _victimGUID;
    GuidSet _eatenGUIDs;
    Position _homePos;
    uint8 _kills;
    bool _dismissed;
};

/*######
## spell_vashjir_release_bait - 76694 (native summons 41085 + 41051; script roots the fisherman)
######*/

class spell_vashjir_release_bait : public SpellScript
{
    void HandleAfterCast()
    {
        if (Unit* caster = GetCaster())
            caster->SetControlled(true, UNIT_STATE_ROOT); // released by the bait bunny director
    }

    void Register() override
    {
        AfterCast.Register(&spell_vashjir_release_bait::HandleAfterCast);
    }
};

/*######
## spell_vashjir_forcecast_bp_at_caster - 76744 / 76799 / 76854 / 82581 (hit unit casts the BP spell at the caster)
######*/

class spell_vashjir_forcecast_bp_at_caster : public SpellScript
{
    void HandleScriptEffect(SpellEffIndex /*effIndex*/)
    {
        if (Unit* caster = GetCaster())
            GetHitUnit()->CastSpell(caster, uint32(GetEffectValue()), true);
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_vashjir_forcecast_bp_at_caster::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

/*######
## spell_vashjir_gnaws_kill_credit - 76747 (eff0 script -> self-cast 76761; KC + teleport are native)
######*/

class spell_vashjir_gnaws_kill_credit : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SEE_GNAWS_TEETH });
    }

    void HandleScriptEffect(SpellEffIndex /*effIndex*/)
    {
        Unit* target = GetHitUnit();
        target->CastSpell(target, SPELL_SEE_GNAWS_TEETH, true);
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_vashjir_gnaws_kill_credit::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

/*######
## spell_vashjir_fastening_chain - 76795 (native summon 41093; script adds the dest-db chain bunny)
######*/

class spell_vashjir_fastening_chain : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SUMMON_HARPOON_CHAIN });
    }

    void HandleAfterCast()
    {
        if (Unit* caster = GetCaster())
            caster->CastSpell(nullptr, SPELL_SUMMON_HARPOON_CHAIN, true);
    }

    void Register() override
    {
        AfterCast.Register(&spell_vashjir_fastening_chain::HandleAfterCast);
    }
};

/*######
## spell_vashjir_fire_harpoon_gun - 76859 (vehicle bar; the gun bunny 41094 shoots 76882 back at Gnaws)
######*/

class spell_vashjir_fire_harpoon_gun : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_HARPOON_TO_GNAWS });
    }

    void HandleScriptEffect(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        // the fork's vehicle-bar redirect may make either the rider or Gnaws the caster
        Unit* gnaws = nullptr;
        if (caster->GetEntry() == NPC_GNAWS_II)
            gnaws = caster;
        else if (Unit* base = caster->GetVehicleBase())
            if (base->GetEntry() == NPC_GNAWS_II)
                gnaws = base;

        if (gnaws)
            GetHitUnit()->CastSpell(gnaws, SPELL_HARPOON_TO_GNAWS, true);
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_vashjir_fire_harpoon_gun::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

/*######
## spell_vashjir_pewter_pounder_completion - 77281 (adds the 77282 Harrison-vision grant)
######*/

class spell_vashjir_pewter_pounder_completion : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SEE_HARRISON_FINAL });
    }

    void HandleAfterHit()
    {
        if (Unit* target = GetHitUnit())
            target->CastSpell(target, SPELL_SEE_HARRISON_FINAL, true);
    }

    void Register() override
    {
        AfterHit.Register(&spell_vashjir_pewter_pounder_completion::HandleAfterHit);
    }
};

/*######
## spell_vashjir_scrying - 86382 (Erunak's Scrying Orb -> summon the Watery Vision)
######*/

class spell_vashjir_scrying : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SUMMON_WATERY_VISION });
    }

    void HandleAfterCast()
    {
        if (Unit* caster = GetCaster())
            caster->CastSpell(nullptr, SPELL_SUMMON_WATERY_VISION, true);
    }

    void Register() override
    {
        AfterCast.Register(&spell_vashjir_scrying::HandleAfterCast);
    }
};

/*######
## spell_vashjir_force_master_ride_vision - 77418 (script -> summoner casts 77419 at the vision)
######*/

class spell_vashjir_force_master_ride_vision : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_RIDE_VISION });
    }

    void HandleScriptEffect(SpellEffIndex /*effIndex*/)
    {
        if (Unit* caster = GetCaster())
            GetHitUnit()->CastSpell(caster, SPELL_RIDE_VISION, true);
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_vashjir_force_master_ride_vision::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

/*######
## spell_vashjir_spelunking_completion - 77433 (quest reward -> Deepmist arrival RP trio, dest-db summons)
######*/

class spell_vashjir_spelunking_completion : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SUMMON_ERUNAK_DEEPMIST, SPELL_SUMMON_MOANAH_DEEPMIST, SPELL_SUMMON_RENDEL_DEEPMIST });
    }

    void HandleScriptEffect(SpellEffIndex /*effIndex*/)
    {
        Unit* target = GetHitUnit();
        target->CastSpell(nullptr, SPELL_SUMMON_ERUNAK_DEEPMIST, true);
        target->CastSpell(nullptr, SPELL_SUMMON_MOANAH_DEEPMIST, true);
        target->CastSpell(nullptr, SPELL_SUMMON_RENDEL_DEEPMIST, true);
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_vashjir_spelunking_completion::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

/*######
## spell_vashjir_dominate_great_shark - 78288 (replace the famished shark; eff1 native summon spawns 42013)
######*/

class spell_vashjir_dominate_great_shark : public SpellScript
{
    void HandleApplyAura(SpellEffIndex /*effIndex*/)
    {
        Creature* shark = GetHitCreature();
        if (!shark || shark->GetEntry() != NPC_FAMISHED_GREAT_SHARK)
            return;

        shark->RemoveAurasDueToSpell(SPELL_DOMINATE_CREATURE); // stop further ticks - one dominated shark only
        shark->DespawnOrUnsummon(500ms, 5min);
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_vashjir_dominate_great_shark::HandleApplyAura, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
    }
};

} // namespace Vashjir::KelptharForest

void AddSC_vashjir_kelpthar_forest()
{
    using namespace Vashjir::KelptharForest;
    RegisterCreatureAI(npc_briny_cutter_battle_bunny);
    RegisterCreatureAI(npc_zinjatar_abductor);
    RegisterCreatureAI(npc_zinjatar_abductor_player);
    RegisterCreatureAI(npc_erunak_rescue);
    RegisterCreatureAI(npc_naga_death_bunny);
    RegisterCreatureAI(npc_abyssal_seahorse);
    RegisterCreatureAI(npc_gnaws_bait_bunny);
    RegisterCreatureAI(npc_gnaws_cloned_image);
    RegisterCreatureAI(npc_gnaws);
    RegisterCreatureAI(npc_player_bait_bunny);
    RegisterCreatureAI(npc_harpoon_chain_bunny);
    RegisterCreatureAI(npc_gnaws_ii);
    RegisterCreatureAI(npc_pewter_prophet);
    RegisterCreatureAI(npc_budd_farewell);
    RegisterCreatureAI(npc_watery_vision);
    RegisterCreatureAI(npc_dominated_great_shark);
    RegisterSpellScript(spell_vashjir_sea_legs_reward);
    RegisterSpellScript(spell_vashjir_force_creator_ride_abductor);
    RegisterSpellScript(spell_vashjir_forcecast_abyssal_ride);
    RegisterSpellScript(spell_vashjir_seahorse_rodeo_response);
    RegisterSpellScript(spell_vashjir_release_bait);
    RegisterSpellScript(spell_vashjir_forcecast_bp_at_caster);
    RegisterSpellScript(spell_vashjir_gnaws_kill_credit);
    RegisterSpellScript(spell_vashjir_fastening_chain);
    RegisterSpellScript(spell_vashjir_fire_harpoon_gun);
    RegisterSpellScript(spell_vashjir_pewter_pounder_completion);
    RegisterSpellScript(spell_vashjir_scrying);
    RegisterSpellScript(spell_vashjir_force_master_ride_vision);
    RegisterSpellScript(spell_vashjir_spelunking_completion);
    RegisterSpellScript(spell_vashjir_dominate_great_shark);
}
