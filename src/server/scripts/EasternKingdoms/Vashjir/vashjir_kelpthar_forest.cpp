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
    NPC_SEAL_GRABBED_SECOND         = 40731
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
        _wave2TalkIndex(0), _abductionCount(0) { }

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
                    ActorTalk(NPC_CAPTAIN_TAYLOR_WAVE1, SAY_TAYLOR_W1_DESPERATE);
                    break;
                case EVENT_WAVE1_WIPE:
                    WipeWave1();
                    break;
                case EVENT_WAVE1_TAYLOR_REST:
                    ActorTalk(NPC_CAPTAIN_TAYLOR_WAVE1, SAY_TAYLOR_W1_REST);
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
                        ActorTalk(NPC_CAPTAIN_TAYLOR_WAVE2, _wave2TalkIndex++);
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
            return FindActor(NPC_CAPTAIN_TAYLOR_WAVE1);
        if (index == 1)
            return FindActor(NPC_ERUNAK_WAVE1);
        return FindActor(Wave1SealEntries[urand(0, uint32(std::size(Wave1SealEntries)) - 1)]);
    }

    void StartEvent()
    {
        _stage = STAGE_WAVE1;
        _wave2TalkIndex = 0;
        _abductionCount = 0;

        // wave-1 pressure raiders (unkillable, attack the crew)
        for (uint8 i = 0; i < std::size(Wave1RaiderPositions); ++i)
        {
            if (Creature* raider = me->SummonCreature(NPC_ZINJATAR_RAIDER_PRESSURE, Wave1RaiderPositions[i], TEMPSUMMON_TIMED_DESPAWN, 2min))
            {
                SetSummonPhase(raider, PHASE_BRINY_CUTTER_WAVE1);
                raider->SetImmuneToPC(true);
                if (Unit* target = PickWave1Target(i))
                    if (raider->IsAIEnabled())
                        raider->AI()->AttackStart(target);
            }
        }

        // decorative killable swarm circling above deck
        for (Position const& pos : SwarmPositions)
        {
            if (Creature* swarm = me->SummonCreature(NPC_ZINJATAR_RAIDER_SWARM, pos, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 2min))
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
        me->GetCreatureListWithEntryInGrid(raiders, NPC_ZINJATAR_RAIDER_PRESSURE, 120.0f);
        for (Creature* raider : raiders)
        {
            if (!raider->IsAlive())
                continue;
            raider->CastSpell(raider, SPELL_SUMMON_NAGA_DEATH_BUNNY, true);
            raider->CastSpell(raider, SPELL_PERMANENT_FEIGN_DEATH, true);
            raider->DespawnOrUnsummon(2s);
        }

        std::list<Creature*> swarm;
        me->GetCreatureListWithEntryInGrid(swarm, NPC_ZINJATAR_RAIDER_SWARM, 120.0f);
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
        Position const& pos = Wave1RaiderPositions[urand(0, uint32(std::size(Wave1RaiderPositions)) - 1)];
        if (Creature* elite = me->SummonCreature(NPC_ZINJATAR_RAIDER_ELITE, pos, TEMPSUMMON_TIMED_DESPAWN, 2min))
        {
            SetSummonPhase(elite, PHASE_BRINY_CUTTER_WAVE2);
            elite->SetImmuneToPC(true);
            if (Creature* seal = FindActor(Wave2SealEntries[urand(0, uint32(std::size(Wave2SealEntries)) - 1)]))
                if (elite->IsAIEnabled())
                    elite->AI()->AttackStart(seal);
        }
    }

    void TrickleWave2()
    {
        std::list<Creature*> raiders;
        me->GetCreatureListWithEntryInGrid(raiders, NPC_ZINJATAR_RAIDER_WAVE2, 120.0f);
        uint8 alive = 0;
        for (Creature* raider : raiders)
            if (raider->IsAlive())
                ++alive;

        if (alive >= 8)
            return;

        for (uint8 i = 0; i < 2; ++i)
        {
            Position const& pos = Wave1RaiderPositions[urand(0, uint32(std::size(Wave1RaiderPositions)) - 1)];
            if (Creature* raider = me->SummonCreature(NPC_ZINJATAR_RAIDER_WAVE2, pos, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 3min))
            {
                SetSummonPhase(raider, PHASE_BRINY_CUTTER_WAVE2);
                if (Creature* seal = FindActor(Wave2SealEntries[urand(0, uint32(std::size(Wave2SealEntries)) - 1)]))
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
            target = FindActor(NPC_SEAL_GRABBED_FIRST);
        else if (_abductionCount == 1)
            target = FindActor(NPC_SEAL_GRABBED_SECOND);

        if (!target || !target->IsAlive() || target->GetVehicle())
        {
            std::vector<Creature*> seals;
            for (uint32 entry : Wave2SealEntries)
                if (Creature* seal = FindActor(entry))
                    if (seal->IsAlive() && !seal->GetVehicle())
                        seals.push_back(seal);
            if (seals.empty())
                return;
            target = Trinity::Containers::SelectRandomContainerElement(seals);
        }

        Position const& pos = AbductorSpawnPositions[_abductionCount % std::size(AbductorSpawnPositions)];
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
                me->GetMotionMaster()->MoveSmoothPath(POINT_ABDUCTOR_ESCAPE, AbductorEscapePath, std::size(AbductorEscapePath), false, true, ABDUCTOR_VELOCITY);
        }
    }

private:
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
    npc_zinjatar_abductor_player(Creature* creature) : PassiveAI(creature) { }

    void IsSummonedBy(Unit* summoner) override
    {
        Player* player = summoner->ToPlayer();
        if (!player)
        {
            me->DespawnOrUnsummon();
            return;
        }

        _summonerGUID = player->GetGUID();
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
                        player->KilledMonsterCredit(NPC_ALL_OR_NOTHING_CREDIT);
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
    RegisterSpellScript(spell_vashjir_sea_legs_reward);
    RegisterSpellScript(spell_vashjir_force_creator_ride_abductor);
    RegisterSpellScript(spell_vashjir_forcecast_abyssal_ride);
    RegisterSpellScript(spell_vashjir_seahorse_rodeo_response);
}
