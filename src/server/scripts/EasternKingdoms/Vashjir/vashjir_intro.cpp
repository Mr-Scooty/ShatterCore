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
 * Vashj'ir intro ship event ("Call of Duty" 14482 Alliance / 25924 Horde).
 *
 * The mercenary ships (GO 197195 Alliance / TaxiPath 1368, GO 203466 Horde /
 * TaxiPath 2204) are permanent MOTransports driven by the transports table.
 * Both paths share the wreck-site stop at -4632.93, 3863.71 (map 0, 45 s stop);
 * the Alliance dock stop is Stormwind Harbor node 4 (-8290.86, 1423.76, 35 s),
 * the Horde dock stop is Bladefist Bay node 12 (map 1, 1440.22, -5035.89, 40 s).
 *
 * Timeline reconstructed from a retail sniff (see scratchpad batchA reports):
 *  - quest accept phases the player into 171 + 170 (harbor + staging merged),
 *  - the dock conversation loop (~318 s == transport period) is anchored to the
 *    ship's dock arrivals/departures,
 *  - at the wreck stop every quest player runs a personal grab chain:
 *    69459 summon tentacle -> 69460 ride -> 69522 summon submerge bunny ->
 *    69523 ride / 69524 phase 179 -> seat shuffles 75647/75711 -> Erunak
 *    rescue (75726/75680/75733/75746/74345/75751) -> 75757/73734 blackout ->
 *    73716 + eject + 74427/75759/75777/75974 + teleport 73727 (A) / Horde cave.
 */

#include "ScriptMgr.h"
#include "CreatureTextMgr.h"
#include "GameObject.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "PassiveAI.h"
#include "PhasingHandler.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "TemporarySummon.h"
#include "Transport.h"
#include "Vehicle.h"
#include "vashjir.h"
#include <cmath>
#include <limits>
#include <type_traits>

using namespace Vashjir;

namespace Vashjir::Intro
{

enum IntroCreatureIds
{
    // Ship crew (summoned as transport passengers)
    NPC_CAPTAIN_SAMIR               = 39447,
    NPC_ADARRAH                     = 39460,
    NPC_MACK_FEARSEN                = 39478,
    NPC_BUDD                        = 39480,
    NPC_SHIP_CONTROLLER             = 40559, // "Ship to Vashj'ir Phase Caster Bunny"
    NPC_CAPTAIN_GREMBUL             = 36818,
    NPC_CREWMAN_BANNON              = 36819,
    NPC_BILLYCLUB_BILLY             = 36820,
    NPC_BELINDAH                    = 36821,
    NPC_BUDDS_VEHICLE_BUNNY         = 42202, // water-splash landing pad

    // Stormwind harbor (world-spawned, phases 171/170)
    NPC_STORMWIND_SOLDIER           = 42021,
    NPC_STORMWIND_RECRUIT_A         = 42022,
    NPC_STORMWIND_RECRUIT_B         = 42059,
    NPC_STAGING_RECRUIT             = 42095,
    NPC_STAGING_SOLDIER             = 42096,
    NPC_CAPTAIN_TAYLOR              = 42103,

    // Wreck-site event
    NPC_TENTACLE_GREMBUL            = 36826, // grabs Grembul
    NPC_TENTACLE_BELINDAH           = 36835, // grabs Belindah
    NPC_TENTACLE_KNOCKBACK          = 36846, // knocks Billy/Bannon around
    NPC_TENTACLE_SAMIR              = 39620, // grabs Captain Samir
    NPC_TENTACLE_ADARRAH            = 39652, // grabs Adarrah
    NPC_TENTACLE_SCENERY_1          = 39661,
    NPC_TENTACLE_SCENERY_2          = 42208,
    NPC_INTRO_TENTACLE              = 36878, // per-player grab tentacle (vehicle 559)
    NPC_SUBMERGE_BUNNY              = 36901, // per-player controller (vehicle 561)
    NPC_ERUNAK_RESCUER              = 40601,
    NPC_NAGA_ASSAILANT              = 40587,
    NPC_NAGA_DEATH_BUNNY            = 40605
};

enum IntroSpells
{
    // Ship phase upkeep
    SPELL_PHASE_1_INTRO_AURA        = 75633, // A261 PhaseId 170, 20 s, pulsed on players aboard

    // Budd / crew gags
    SPELL_RIDE_VEHICLE_BUDD         = 78739, // TargA=38 -> 42202 (conditions)
    SPELL_WATER_SPLASH              = 36005,
    SPELL_RIDE_VEHICLE_SOLDIER_1    = 78749, // seat 2
    SPELL_RIDE_VEHICLE_RECRUIT      = 78752, // seat 2
    SPELL_RIDE_VEHICLE_SOLDIER_2    = 78762, // seat 3
    SPELL_RIDE_VEHICLE_SOLDIER_3    = 78760, // seat 3

    // Wreck chaos (cosmetic)
    SPELL_TENTACLE_VS_GREMBUL       = 69394, // force-cast 69396 on 36818
    SPELL_TENTACLE_VS_BELINDAH      = 69408, // force-cast 69407 on 36821
    SPELL_TENTACLE_VS_SAMIR         = 74067, // force-cast 74068 on 39447
    SPELL_TENTACLE_VS_ADARRAH       = 74131, // force-cast 74130 on 39460
    SPELL_TENTACLE_KNOCKBACK        = 69414, // knockback on 36820/36819 (conditions)
    SPELL_SUBMERGE_VISUAL           = 28819,
    SPELL_CAMERA_SHAKE_MED          = 44762,
    SPELL_VOMIT                     = 43391,

    // Wreck approach detection auras (self-casts on quest players)
    SPELL_SEE_SAMIR                 = 76326,
    SPELL_SEE_ADARRAH               = 74481,
    SPELL_SEE_DROWNING_SOLDIERS     = 77823,
    SPELL_KELP_FOREST_QUEST_INVIS_1 = 74145,

    // Personal grab chain
    SPELL_SUMMON_TENTACLE           = 69459, // 36878, spell_target_position
    SPELL_TENTACLE_ENTRY            = 69518,
    SPELL_PLAYER_RIDE_TENTACLE      = 69460,
    SPELL_SUMMON_SUBMERGE_BUNNY     = 69522, // 36901, spell_target_position
    SPELL_RIDE_SUBMERGE_BUNNY       = 69523,
    SPELL_SUBMERGE_BUNNY_PHASE      = 69524, // A261 PhaseId 179 + stun (faction-neutral effects)
    SPELL_RIDE_SUBMERGE_BUNNY_SEAT2 = 75647,
    SPELL_RIDE_SUBMERGE_BUNNY_SEAT3 = 75711,
    SPELL_SUMMON_ERUNAK             = 75726, // 40601, spell_target_position
    SPELL_SUMMON_NAGA_ASSAILANT     = 75680, // 40587, spell_target_position
    SPELL_INTRO_DAZED               = 75869, // TargA=92 (summoner)
    SPELL_LAVA_BOLT                 = 75733,
    SPELL_SUMMON_NAGA_DEATH_BUNNY   = 75743,
    SPELL_SUICIDE_NO_LOG            = 51744,
    SPELL_TURTLE_PARTS_00           = 77310,
    SPELL_TURTLE_PARTS_01           = 75375,
    SPELL_TURTLE_PARTS_02           = 75376,
    SPELL_RED_RADIATION             = 52679,
    SPELL_BLOW_BUBBLE               = 75746, // TargA=92
    SPELL_INVISIBLE_CHANNEL_BEAM    = 74345,
    SPELL_BUBBLE_SELF_INTRO         = 75751,
    SPELL_BLACKOUT_TIMER            = 75757, // TargA=92
    SPELL_DROWNED_SCREEN_EFFECT     = 73734,
    SPELL_LEAVE_SUBMERGE_BUNNY      = 73716, // removes 69524 + 75751 on summoner

    // Wake-up
    SPELL_POST_SEA_MONSTER_BINDING  = 74427, // A261 PhaseId 169 (default)
    SPELL_WAKE_UP_DEAD              = 75759,
    SPELL_INTRO_INVIS               = 75777,
    SPELL_QUEST_INVIS_DETECT_ERUNAK = 75974,
    SPELL_TELEPORT_ALLIANCE_PLAYER  = 73727  // spell_target_position -> Alliance cave
};

enum IntroTexts
{
    // 39460 Adarrah
    TEXT_ADARRAH_BUDD_NO            = 0,
    TEXT_ADARRAH_SHIPWRECK          = 1,
    // 39478 Mack Fearsen
    TEXT_MACK_BOARDING_0            = 0, // "Come aboard, now. Step right up!"
    TEXT_MACK_BOARDING_1            = 1,
    TEXT_MACK_BOARDING_2            = 2,
    TEXT_MACK_BOARDING_3            = 3,
    TEXT_MACK_BUDD_REACT            = 4,
    TEXT_MACK_WHAT_THE              = 5,
    // 39480 Budd
    TEXT_BUDD_AHOY                  = 0,
    TEXT_BUDD_SAILING               = 1,
    TEXT_BUDD_OUT_ON_A_BOAT         = 2,
    TEXT_BUDD_INTO_THE_WIND         = 3,
    TEXT_BUDD_SHINY                 = 4,
    // 42103 Captain Taylor
    TEXT_TAYLOR_THATS_ENOUGH        = 0,
    TEXT_TAYLOR_MINDS_RIGHT         = 1,
    TEXT_TAYLOR_LISTEN_UP           = 2,
    TEXT_TAYLOR_FALL_IN             = 3,
    TEXT_TAYLOR_MOVE_OUT            = 4,
    TEXT_TAYLOR_CIVILIANS           = 5,
    TEXT_TAYLOR_FALL_IN_MEN         = 6,
    TEXT_TAYLOR_SPEECH_FIRST        = 7,  // 7..13 voyage speech
    TEXT_TAYLOR_SPEECH_LAST         = 13,
    // 42022 Stormwind Recruit
    TEXT_RECRUIT_A_WHY_MERCS        = 0,
    TEXT_RECRUIT_A_MAKES_SENSE      = 1,
    // 42059 Stormwind Recruit
    TEXT_RECRUIT_B_EARTHQUAKES      = 0,
    TEXT_RECRUIT_B_YESSIR           = 1,
    // 42095 Staging Recruit
    TEXT_RECRUIT_95_MAKE_WHAT_OUT   = 0,
    TEXT_RECRUIT_95_BEEN_THERE      = 1,
    TEXT_RECRUIT_95_SEE_IT_TOO      = 2,
    // 40601 Erunak Stonespeaker (rescuer)
    TEXT_ERUNAK_TO_THE_DEPTHS       = 0,
    TEXT_ERUNAK_HOLD_ON             = 1
};

enum IntroEvents
{
    // ship controller (40559)
    EVENT_PHASE_PULSE               = 1,
    EVENT_MACK_BOARDING,
    EVENT_BUDD_SAILING,
    EVENT_TAYLOR_SPEECH,
    EVENT_CHAOS_START,
    EVENT_CHAOS_GRABS_PIRATES,
    EVENT_CHAOS_SHAKE,
    EVENT_CHAOS_KNOCKBACK,
    EVENT_CHAOS_SOLDIER_1,
    EVENT_CHAOS_MACK_VOMIT,
    EVENT_CHAOS_SAMIR,
    EVENT_CHAOS_SOLDIER_2,
    EVENT_CHAOS_ADARRAH,
    EVENT_CHAOS_SOLDIER_3,
    EVENT_CHAOS_SOLDIER_4,
    EVENT_CHAOS_CLEANUP,

    // dock Taylor
    EVENT_DOCK_LINE,
    EVENT_DOCK_FALL_IN,

    // grab tentacle
    EVENT_TENTACLE_GRAB,
    EVENT_TENTACLE_SUBMERGE,
    EVENT_TENTACLE_VANISH,

    // personal tentacle (36878)
    EVENT_INTRO_TENTACLE_ENTRY,
    EVENT_INTRO_TENTACLE_RIDE,
    EVENT_INTRO_TENTACLE_HANDOFF,

    // submerge bunny (36901)
    EVENT_BUNNY_RIDE,
    EVENT_BUNNY_DESCEND,
    EVENT_BUNNY_SEAT_2,
    EVENT_BUNNY_SUMMON_SCENE,
    EVENT_BUNNY_NAGA_MOVE,
    EVENT_BUNNY_SEAT_3,
    EVENT_BUNNY_LAVA_BOLT,
    EVENT_BUNNY_TALK_DEPTHS,
    EVENT_BUNNY_NAGA_DIE,
    EVENT_BUNNY_TURTLE_PARTS,
    EVENT_BUNNY_BUBBLE,
    EVENT_BUNNY_HOLD_ON,
    EVENT_BUNNY_BUBBLE_SELF,
    EVENT_BUNNY_BLACKOUT,
    EVENT_BUNNY_SCREEN_EFFECT,
    EVENT_BUNNY_FINALE,
    EVENT_BUNNY_VALIDATE
};

enum IntroData
{
    DATA_SHIP_EVENT                 = 1,

    // values for DATA_SHIP_EVENT (controller 40559)
    SHIP_EVENT_DOCKED               = 1,
    SHIP_EVENT_VOYAGE               = 2,
    SHIP_EVENT_WRECK_APPROACH       = 3,
    SHIP_EVENT_WRECK_CHAOS          = 4,

    // values for DATA_SHIP_EVENT (dock Taylor 42103)
    DOCK_EVENT_SHIP_APPROACH        = 5,
    DOCK_EVENT_SHIP_DOCKED          = 6,
    DOCK_EVENT_SHIP_DEPARTED        = 7
};

enum IntroActions
{
    ACTION_BUDD_JUMP                = 1
};

enum IntroPoints
{
    POINT_BUNNY_DESCENT             = 1,
    POINT_NAGA_SCENE                = 2
};

enum IntroPhases
{
    PHASE_HARBOR                    = 171,
    PHASE_SHIP                      = 170,
    PHASE_WRECK                     = 179
};

Position const AllianceDockPos    = { -8290.86f, 1423.76f,  0.0f, 0.0f };
Position const HordeDockPos       = {  1440.22f, -5035.89f, 0.0f, 0.0f };
Position const WreckStopPos       = { -4632.93f, 3863.71f,  7.58f, 0.0f };
Position const HordeCaveArrival   = { -4608.16f, 3981.21f, -70.8f, 2.217f }; // 73728 spell_target_position

// Submerge bunny descent (from its spell_target_position spawn -4650.89, 3794.72, 1.08
// down to the rescue scene in front of Erunak's spawn -4628.94, 3788.61, -77.37).
Position const BunnyDescentPath[] =
{
    { -4646.0f, 3792.5f, -15.0f },
    { -4639.0f, 3790.5f, -40.0f },
    { -4633.0f, 3789.5f, -60.0f },
    { -4629.4f, 3788.9f, -73.5f }
};

Position const NagaScenePos       = { -4634.0f, 3792.0f, -70.0f, 5.6f };
Position const SceneryTentacle1   = { -4659.65f, 3841.26f, -1.0f, 1.2f };
Position const SceneryTentacle2   = { -4607.89f, 3846.32f, -1.0f, 3.9f };

struct CrewSlot
{
    uint32 Entry;
    Position Offset; // transport-relative
};

// Transport-relative offsets from the sniffed crew manifest; the deck extras
// (Taylor, staging troops, pirates) use plausible deck offsets (main deck z ~5.2-5.7).
CrewSlot const AllianceCrew[] =
{
    { NPC_SHIP_CONTROLLER,   {   7.73f, -0.81f,  5.26f, 0.00f } },
    { NPC_ADARRAH,           { -14.88f,  4.36f,  5.62f, 2.13f } },
    { NPC_MACK_FEARSEN,      {  15.46f, -3.40f,  5.15f, 3.11f } },
    { NPC_BUDD,              { -13.18f, -0.03f,  5.66f, 0.00f } },
    { NPC_CAPTAIN_SAMIR,     {  36.46f,  0.05f, 12.04f, 3.11f } },
    { NPC_CAPTAIN_GREMBUL,   {  25.00f,  3.50f,  5.15f, 3.50f } },
    { NPC_BELINDAH,          {  -8.00f,  3.50f,  5.66f, 4.00f } },
    { NPC_BILLYCLUB_BILLY,   {   5.00f, -4.00f,  5.15f, 1.80f } },
    { NPC_CREWMAN_BANNON,    {   8.00f,  4.00f,  5.15f, 4.50f } },
    // Alliance-only deck passengers (voyage speech + wreck chaos victims)
    { NPC_CAPTAIN_TAYLOR,    {  -2.00f,  0.50f,  5.66f, 3.14f } },
    { NPC_STAGING_SOLDIER,   {  -4.00f, -2.50f,  5.66f, 3.14f } },
    { NPC_STAGING_SOLDIER,   {   0.50f, -3.50f,  5.66f, 2.80f } },
    { NPC_STAGING_SOLDIER,   {   2.50f,  2.00f,  5.66f, 3.50f } },
    { NPC_STAGING_RECRUIT,   {  -1.00f,  3.00f,  5.66f, 2.90f } }
};

// Horde ship: same mercenary crew, no Stormwind troops (no sniff data for a
// Horde escort - simplification).
CrewSlot const HordeCrew[] =
{
    { NPC_SHIP_CONTROLLER,   {   7.73f, -0.81f,  5.26f, 0.00f } },
    { NPC_ADARRAH,           { -14.88f,  4.36f,  5.62f, 2.13f } },
    { NPC_MACK_FEARSEN,      {  15.46f, -3.40f,  5.15f, 3.11f } },
    { NPC_BUDD,              { -13.18f, -0.03f,  5.66f, 0.00f } },
    { NPC_CAPTAIN_SAMIR,     {  36.46f,  0.05f, 12.04f, 3.11f } },
    { NPC_CAPTAIN_GREMBUL,   {  25.00f,  3.50f,  5.15f, 3.50f } },
    { NPC_BELINDAH,          {  -8.00f,  3.50f,  5.66f, 4.00f } },
    { NPC_BILLYCLUB_BILLY,   {   5.00f, -4.00f,  5.15f, 1.80f } },
    { NPC_CREWMAN_BANNON,    {   8.00f,  4.00f,  5.15f, 4.50f } }
};

// Stormwind dock conversation loop, offsets in seconds after the ship departs
// (sniffed cycle period == transport period ~318 s).
enum DockSpeaker : uint8
{
    SPEAK_TAYLOR,
    SPEAK_RECRUIT_22,       // 42022
    SPEAK_RECRUIT_59,       // 42059
    SPEAK_RECRUIT_95,       // 42095
    SPEAK_SOLDIER_0,        // 42021 by spawn-guid order
    SPEAK_SOLDIER_1,
    SPEAK_SOLDIER_2,
    SPEAK_SOLDIER_3,
    SPEAK_HORIZON_96        // 42096 closest to the 42095 pair spot
};

struct DockLine
{
    uint16 Offset; // seconds after dock departure
    DockSpeaker Speaker;
    uint8 Group;
};

DockLine const DockScript[] =
{
    // "Why must we ride to battle on a mercenary ship?"
    {  34, SPEAK_RECRUIT_22, 0 },
    {  42, SPEAK_SOLDIER_0,  0 },
    {  46, SPEAK_SOLDIER_0,  1 },
    {  52, SPEAK_SOLDIER_0,  2 },
    {  57, SPEAK_SOLDIER_0,  3 },
    {  64, SPEAK_SOLDIER_0,  4 },
    {  69, SPEAK_RECRUIT_22, 1 },
    // "Earthquakes every other day..."
    {  77, SPEAK_RECRUIT_59, 0 },
    {  83, SPEAK_SOLDIER_1,  6 },
    {  89, SPEAK_SOLDIER_1,  5 },
    {  95, SPEAK_SOLDIER_1,  7 },
    { 100, SPEAK_RECRUIT_59, 1 },
    // war story ("park district" -> "the beast is insane")
    { 116, SPEAK_SOLDIER_2,  8 },
    { 121, SPEAK_SOLDIER_3,  9 },
    { 126, SPEAK_SOLDIER_3, 10 },
    { 131, SPEAK_SOLDIER_2, 11 },
    { 136, SPEAK_SOLDIER_3, 12 },
    { 141, SPEAK_SOLDIER_3, 13 },
    { 146, SPEAK_SOLDIER_2, 14 },
    // "I think I can just make it out from here..."
    { 163, SPEAK_HORIZON_96, 0 },
    { 168, SPEAK_RECRUIT_95, 0 },
    { 173, SPEAK_HORIZON_96, 1 },
    { 179, SPEAK_HORIZON_96, 2 },
    { 184, SPEAK_RECRUIT_95, 1 },
    { 189, SPEAK_HORIZON_96, 3 },
    { 195, SPEAK_HORIZON_96, 4 },
    { 201, SPEAK_HORIZON_96, 5 },
    // Taylor shuts it down
    { 218, SPEAK_TAYLOR,     TEXT_TAYLOR_THATS_ENOUGH },
    { 221, SPEAK_TAYLOR,     TEXT_TAYLOR_MINDS_RIGHT },
    { 233, SPEAK_RECRUIT_95, TEXT_RECRUIT_95_SEE_IT_TOO }
};

uint32 GetIntroQuestId(Player const* player)
{
    return player->GetTeam() == ALLIANCE ? QUEST_CALL_OF_DUTY_A : QUEST_CALL_OF_DUTY_H;
}

bool IsOnIntroQuest(Player const* player)
{
    QuestStatus status = player->GetQuestStatus(GetIntroQuestId(player));
    return status == QUEST_STATUS_INCOMPLETE || status == QUEST_STATUS_COMPLETE;
}

bool IsEligibleForGrab(Player const* player)
{
    if (!player->IsAlive() || !IsOnIntroQuest(player))
        return false;
    if (player->GetVehicle() || player->HasAura(SPELL_SUBMERGE_BUNNY_PHASE))
        return false;
    return true;
}

void FinishIntroArrival(Player* player)
{
    player->RemoveAurasDueToSpell(SPELL_INTRO_DAZED);
    player->CastSpell(player, SPELL_POST_SEA_MONSTER_BINDING, true);
    player->CastSpell(player, SPELL_WAKE_UP_DEAD, true);
    player->CastSpell(player, SPELL_INTRO_INVIS, true);
    player->CastSpell(player, SPELL_QUEST_INVIS_DETECT_ERUNAK, true);

    PhasingHandler::RemovePhase(player, PHASE_HARBOR, false);
    PhasingHandler::RemovePhase(player, PHASE_SHIP, true);

    if (player->GetTeam() == ALLIANCE)
        player->CastSpell(player, SPELL_TELEPORT_ALLIANCE_PLAYER, true);
    else
        // 73728's teleport effect targets TARGET_DEST_CHANNEL_CASTER and cannot
        // consume its spell_target_position row - teleport directly instead.
        player->TeleportTo(0, HordeCaveArrival.GetPositionX(), HordeCaveArrival.GetPositionY(),
            HordeCaveArrival.GetPositionZ(), HordeCaveArrival.GetOrientation());

    // Retail strips 75777 via 73735 (cast by the cave rescue bunny ~20 s after
    // Sea Legs is accepted). Self-contained approximation: drop it after 90 s.
    player->m_Events.AddEventAtOffset([player]()
    {
        player->RemoveAurasDueToSpell(SPELL_INTRO_INVIS);
    }, 90s);
}

/*######
## PlayerScript - phase management for Call of Duty (14482/25924)
######*/

class player_vashjir_intro : public PlayerScript
{
public:
    player_vashjir_intro() : PlayerScript("player_vashjir_intro") { }

    void OnPlayerQuestStatusChange(Player* player, uint32 questId) override
    {
        if (questId != QUEST_CALL_OF_DUTY_A && questId != QUEST_CALL_OF_DUTY_H)
            return;

        switch (player->GetQuestStatus(questId))
        {
            case QUEST_STATUS_INCOMPLETE:
            case QUEST_STATUS_COMPLETE:
                // Harbor (171) and staging/ship (170) merged - retail swaps
                // 171 -> 170 when the player walks onto the pier (proximity
                // trigger not captured in the sniff). Acceptable simplification.
                PhasingHandler::AddPhase(player, PHASE_HARBOR, false);
                PhasingHandler::AddPhase(player, PHASE_SHIP, true);
                break;
            default: // NONE (abandoned) or REWARDED
                PhasingHandler::RemovePhase(player, PHASE_HARBOR, false);
                PhasingHandler::RemovePhase(player, PHASE_SHIP, true);
                player->RemoveAurasDueToSpell(SPELL_PHASE_1_INTRO_AURA);
                break;
        }
    }

    void OnPlayerLogin(Player* player) override
    {
        // Logged out mid-grab: the infinite 69524 aura (phase 179 + stun) is
        // restored on login but its controller chain is gone - finish gracefully.
        if (player->HasAura(SPELL_SUBMERGE_BUNNY_PHASE) && !player->GetVehicle())
        {
            player->RemoveAurasDueToSpell(SPELL_SUBMERGE_BUNNY_PHASE);
            player->RemoveAurasDueToSpell(SPELL_BUBBLE_SELF_INTRO);
            FinishIntroArrival(player);
            return;
        }

        if (IsOnIntroQuest(player))
        {
            PhasingHandler::AddPhase(player, PHASE_HARBOR, false);
            PhasingHandler::AddPhase(player, PHASE_SHIP, true);
        }
    }
};

/*######
## TransportScript - "Ship to Vashj'ir" (197195 Alliance / 203466 Horde)
######*/

class transport_vashjir_ship : public TransportScript
{
protected:
    transport_vashjir_ship(char const* scriptName, uint32 dockMapId, Position const& dockPos,
        uint32 questId, CrewSlot const* crew, size_t crewSize)
        : TransportScript(scriptName), _dockMapId(dockMapId), _dockPos(dockPos), _questId(questId),
        _crew(crew), _crewSize(crewSize), _crewGuids(crewSize), _state(SHIP_STATE_SAILING), _checkTimer(0) { }

    enum ShipState
    {
        SHIP_STATE_SAILING,
        SHIP_STATE_DOCK_APPROACH,
        SHIP_STATE_DOCKED,
        SHIP_STATE_DOCK_LEAVING,
        SHIP_STATE_WRECK_APPROACH,
        SHIP_STATE_AT_WRECK,
        SHIP_STATE_WRECK_LEAVING
    };

public:
    void OnUpdate(Transport* transport, uint32 diff) override
    {
        if (!transport->FindMap())
            return;

        _checkTimer += diff;
        if (_checkTimer < 500)
            return;
        _checkTimer = 0;

        float distDock = DistanceTo(transport, _dockMapId, _dockPos);
        float distWreck = DistanceTo(transport, 0, WreckStopPos);

        switch (_state)
        {
            case SHIP_STATE_SAILING:
                if (distDock < 150.0f)
                {
                    _state = SHIP_STATE_DOCK_APPROACH;
                    EnsureCrew(transport);
                    if (Creature* taylor = FindDockTaylor(transport))
                        taylor->AI()->SetData(DATA_SHIP_EVENT, DOCK_EVENT_SHIP_APPROACH);
                }
                else if (distWreck < 420.0f)
                {
                    _state = SHIP_STATE_WRECK_APPROACH;
                    OnWreckApproach(transport);
                }
                break;
            case SHIP_STATE_DOCK_APPROACH:
                if (distDock < 10.0f)
                {
                    _state = SHIP_STATE_DOCKED;
                    EnsureCrew(transport);
                    if (Creature* controller = GetController(transport))
                        controller->AI()->SetData(DATA_SHIP_EVENT, SHIP_EVENT_DOCKED);
                    if (Creature* taylor = FindDockTaylor(transport))
                        taylor->AI()->SetData(DATA_SHIP_EVENT, DOCK_EVENT_SHIP_DOCKED);
                }
                break;
            case SHIP_STATE_DOCKED:
                if (distDock > 30.0f)
                {
                    _state = SHIP_STATE_DOCK_LEAVING;
                    if (Creature* controller = GetController(transport))
                        controller->AI()->SetData(DATA_SHIP_EVENT, SHIP_EVENT_VOYAGE);
                    if (Creature* taylor = FindDockTaylor(transport))
                        taylor->AI()->SetData(DATA_SHIP_EVENT, DOCK_EVENT_SHIP_DEPARTED);
                }
                break;
            case SHIP_STATE_DOCK_LEAVING:
                if (distDock > 200.0f)
                    _state = SHIP_STATE_SAILING;
                break;
            case SHIP_STATE_WRECK_APPROACH:
                if (distWreck < 10.0f)
                {
                    _state = SHIP_STATE_AT_WRECK;
                    OnWreckArrive(transport);
                }
                break;
            case SHIP_STATE_AT_WRECK:
                if (distWreck > 30.0f)
                    _state = SHIP_STATE_WRECK_LEAVING;
                break;
            case SHIP_STATE_WRECK_LEAVING:
                if (distWreck > 450.0f)
                    _state = SHIP_STATE_SAILING;
                break;
            default:
                break;
        }
    }

    void OnAddPassenger(Transport* /*transport*/, Player* player) override
    {
        // Safety net: guarantee ship-phase visibility for quest players who
        // board (covers relogs and players sharing another player's ship view).
        if (IsOnIntroQuest(player))
        {
            PhasingHandler::AddPhase(player, PHASE_HARBOR, false);
            PhasingHandler::AddPhase(player, PHASE_SHIP, true);
        }
    }

private:
    static float DistanceTo(Transport* transport, uint32 mapId, Position const& pos)
    {
        if (transport->GetMapId() != mapId)
            return std::numeric_limits<float>::max();
        return transport->GetExactDist2d(&pos);
    }

    void EnsureCrew(Transport* transport)
    {
        for (size_t i = 0; i < _crewSize; ++i)
        {
            if (!_crewGuids[i].IsEmpty() && ObjectAccessor::GetCreature(*transport, _crewGuids[i]))
                continue;

            if (TempSummon* summon = transport->SummonPassenger(_crew[i].Entry, _crew[i].Offset, TEMPSUMMON_MANUAL_DESPAWN))
            {
                _crewGuids[i] = summon->GetGUID();
                if (_crew[i].Entry == NPC_SHIP_CONTROLLER)
                    _controllerGUID = summon->GetGUID();
            }
        }
    }

    Creature* GetController(Transport* transport) const
    {
        return ObjectAccessor::GetCreature(*transport, _controllerGUID);
    }

    // The dock captain is a world spawn (not aboard); only exists at Stormwind.
    Creature* FindDockTaylor(Transport* transport) const
    {
        std::list<Creature*> taylors;
        transport->GetCreatureListWithEntryInGrid(taylors, NPC_CAPTAIN_TAYLOR, 250.0f);
        for (Creature* taylor : taylors)
            if (!taylor->GetTransport())
                return taylor;
        return nullptr;
    }

    void OnWreckApproach(Transport* transport)
    {
        if (Creature* controller = GetController(transport))
            controller->AI()->SetData(DATA_SHIP_EVENT, SHIP_EVENT_WRECK_APPROACH);

        for (WorldObject* passenger : transport->GetPassengers())
        {
            Player* player = passenger->ToPlayer();
            if (!player || GetIntroQuestId(player) != _questId)
                continue;

            if (player->GetQuestStatus(_questId) == QUEST_STATUS_INCOMPLETE)
                player->AreaExploredOrEventHappens(_questId);

            if (IsEligibleForGrab(player))
            {
                player->CastSpell(player, SPELL_SEE_SAMIR, true);
                player->CastSpell(player, SPELL_SEE_ADARRAH, true);
                player->CastSpell(player, SPELL_SEE_DROWNING_SOLDIERS, true);
                player->CastSpell(player, SPELL_KELP_FOREST_QUEST_INVIS_1, true);
            }
        }
    }

    void OnWreckArrive(Transport* transport)
    {
        bool anyEligible = false;
        for (WorldObject* passenger : transport->GetPassengers())
        {
            Player* player = passenger->ToPlayer();
            if (!player || GetIntroQuestId(player) != _questId || !IsEligibleForGrab(player))
                continue;

            anyEligible = true;
            // Personal grab chain: 69459 summons the player's tentacle at the
            // sniffed spot (spell_target_position); its AI runs the hand-off.
            player->CastSpell(player, SPELL_SUMMON_TENTACLE, true);
        }

        // Deck chaos is cosmetic and runs once per stop (not per player).
        if (anyEligible)
            if (Creature* controller = GetController(transport))
                controller->AI()->SetData(DATA_SHIP_EVENT, SHIP_EVENT_WRECK_CHAOS);
    }

    uint32 _dockMapId;
    Position _dockPos;
    uint32 _questId;
    CrewSlot const* _crew;
    size_t _crewSize;
    std::vector<ObjectGuid> _crewGuids;
    ObjectGuid _controllerGUID;
    ShipState _state;
    uint32 _checkTimer;
};

class transport_vashjir_ship_a : public transport_vashjir_ship
{
public:
    transport_vashjir_ship_a() : transport_vashjir_ship("transport_vashjir_ship_a", 0, AllianceDockPos,
        QUEST_CALL_OF_DUTY_A, AllianceCrew, std::extent<decltype(AllianceCrew)>::value) { }
};

class transport_vashjir_ship_h : public transport_vashjir_ship
{
public:
    transport_vashjir_ship_h() : transport_vashjir_ship("transport_vashjir_ship_h", 1, HordeDockPos,
        QUEST_CALL_OF_DUTY_H, HordeCrew, std::extent<decltype(HordeCrew)>::value) { }
};

/*######
## npc_vashjir_ship_controller - 40559 "Ship to Vashj'ir Phase Caster Bunny"
## Keeps riders in phase 170 (75633 pulse) and runs all shipboard timelines.
######*/

struct npc_vashjir_ship_controller : public PassiveAI
{
    npc_vashjir_ship_controller(Creature* creature) : PassiveAI(creature), _mackLine(0), _buddLine(0), _taylorLine(0), _shakeCount(0) { }

    void Reset() override
    {
        _events.ScheduleEvent(EVENT_PHASE_PULSE, 2s);
    }

    void SetData(uint32 id, uint32 value) override
    {
        if (id != DATA_SHIP_EVENT)
            return;

        switch (value)
        {
            case SHIP_EVENT_DOCKED:
                _mackLine = 0;
                _events.ScheduleEvent(EVENT_MACK_BOARDING, 6s);
                break;
            case SHIP_EVENT_VOYAGE:
                _buddLine = 0;
                _taylorLine = 0;
                _events.ScheduleEvent(EVENT_BUDD_SAILING, 8s);
                _events.ScheduleEvent(EVENT_TAYLOR_SPEECH, 66s); // no-op on the Horde ship (no Taylor aboard)
                break;
            case SHIP_EVENT_WRECK_APPROACH:
                if (Creature* adarrah = FindCrew(NPC_ADARRAH))
                    sCreatureTextMgr->SendChat(adarrah, TEXT_ADARRAH_SHIPWRECK);
                break;
            case SHIP_EVENT_WRECK_CHAOS:
                _shakeCount = 0;
                _overboardGuids.clear();
                _events.ScheduleEvent(EVENT_CHAOS_START, 1s);
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
            switch (eventId)
            {
                case EVENT_PHASE_PULSE:
                    // A261 PhaseId 170, 20 s - keeps everyone aboard in the ship phase
                    // (conditions restrict the implicit entry target to players).
                    me->CastSpell(nullptr, SPELL_PHASE_1_INTRO_AURA, true);
                    _events.ScheduleEvent(EVENT_PHASE_PULSE, Milliseconds(4500));
                    break;
                case EVENT_MACK_BOARDING:
                    if (Creature* mack = FindCrew(NPC_MACK_FEARSEN))
                        sCreatureTextMgr->SendChat(mack, TEXT_MACK_BOARDING_0 + _mackLine);
                    if (++_mackLine < 4)
                        _events.ScheduleEvent(EVENT_MACK_BOARDING, _mackLine == 2 ? 7s : 6s);
                    break;
                case EVENT_BUDD_SAILING:
                {
                    if (Creature* budd = FindCrew(NPC_BUDD))
                    {
                        sCreatureTextMgr->SendChat(budd, TEXT_BUDD_AHOY + _buddLine);
                        if (_buddLine == 4) // "Oooh... shiny!" -> jumps overboard
                        {
                            budd->AI()->DoAction(ACTION_BUDD_JUMP);
                            if (Creature* adarrah = FindCrew(NPC_ADARRAH))
                                adarrah->m_Events.AddEventAtOffset([adarrah]()
                                {
                                    sCreatureTextMgr->SendChat(adarrah, TEXT_ADARRAH_BUDD_NO);
                                }, 4s);
                            if (Creature* mack = FindCrew(NPC_MACK_FEARSEN))
                                mack->m_Events.AddEventAtOffset([mack]()
                                {
                                    sCreatureTextMgr->SendChat(mack, TEXT_MACK_BUDD_REACT);
                                }, 6s);
                        }
                    }
                    static uint32 const buddGaps[] = { 10, 9, 8, 12 };
                    if (_buddLine < 4)
                        _events.ScheduleEvent(EVENT_BUDD_SAILING, Seconds(buddGaps[_buddLine]));
                    ++_buddLine;
                    break;
                }
                case EVENT_TAYLOR_SPEECH:
                {
                    if (Creature* taylor = FindCrew(NPC_CAPTAIN_TAYLOR))
                        sCreatureTextMgr->SendChat(taylor, TEXT_TAYLOR_CIVILIANS + _taylorLine);
                    else
                        break; // Horde ship
                    static uint32 const taylorGaps[] = { 10, 5, 7, 8, 7, 7, 7, 7 };
                    if (_taylorLine < 8)
                        _events.ScheduleEvent(EVENT_TAYLOR_SPEECH, Seconds(taylorGaps[_taylorLine]));
                    ++_taylorLine;
                    break;
                }

                // ---- wreck-site deck chaos (cosmetic, once per stop) ----
                case EVENT_CHAOS_START:
                    if (Creature* mack = FindCrew(NPC_MACK_FEARSEN))
                        sCreatureTextMgr->SendChat(mack, TEXT_MACK_WHAT_THE);
                    me->SummonCreature(NPC_TENTACLE_SCENERY_1, SceneryTentacle1, TEMPSUMMON_TIMED_DESPAWN, 20s);
                    me->SummonCreature(NPC_TENTACLE_SCENERY_2, SceneryTentacle2, TEMPSUMMON_TIMED_DESPAWN, 20s);
                    SummonPads();
                    _events.ScheduleEvent(EVENT_CHAOS_GRABS_PIRATES, 2s);
                    _events.ScheduleEvent(EVENT_CHAOS_SHAKE, 3s);
                    _events.ScheduleEvent(EVENT_CHAOS_KNOCKBACK, 5s);
                    _events.ScheduleEvent(EVENT_CHAOS_SOLDIER_1, 6s);
                    _events.ScheduleEvent(EVENT_CHAOS_MACK_VOMIT, 8s);
                    _events.ScheduleEvent(EVENT_CHAOS_SAMIR, 9s);
                    _events.ScheduleEvent(EVENT_CHAOS_SOLDIER_2, 9s + 500ms);
                    _events.ScheduleEvent(EVENT_CHAOS_ADARRAH, 11s);
                    _events.ScheduleEvent(EVENT_CHAOS_SOLDIER_3, 12s);
                    _events.ScheduleEvent(EVENT_CHAOS_SOLDIER_4, 15s);
                    _events.ScheduleEvent(EVENT_CHAOS_CLEANUP, 25s);
                    break;
                case EVENT_CHAOS_GRABS_PIRATES:
                    SummonGrabTentacle(NPC_TENTACLE_GREMBUL, NPC_CAPTAIN_GREMBUL);
                    SummonGrabTentacle(NPC_TENTACLE_BELINDAH, NPC_BELINDAH);
                    break;
                case EVENT_CHAOS_SHAKE:
                    me->CastSpell(nullptr, SPELL_CAMERA_SHAKE_MED, true);
                    if (++_shakeCount < 3)
                        _events.ScheduleEvent(EVENT_CHAOS_SHAKE, 3s);
                    break;
                case EVENT_CHAOS_KNOCKBACK:
                    // No pacer bunny staged: the tentacle knocks Billy/Bannon around itself.
                    SummonGrabTentacle(NPC_TENTACLE_KNOCKBACK, NPC_BILLYCLUB_BILLY);
                    break;
                case EVENT_CHAOS_SOLDIER_1:
                    SoldierOverboard(NPC_STAGING_SOLDIER, 0, SPELL_RIDE_VEHICLE_SOLDIER_1);
                    break;
                case EVENT_CHAOS_MACK_VOMIT:
                    if (Creature* mack = FindCrew(NPC_MACK_FEARSEN))
                        mack->CastSpell(mack, SPELL_VOMIT, true);
                    break;
                case EVENT_CHAOS_SAMIR:
                    SummonGrabTentacle(NPC_TENTACLE_SAMIR, NPC_CAPTAIN_SAMIR);
                    break;
                case EVENT_CHAOS_SOLDIER_2:
                    SoldierOverboard(NPC_STAGING_RECRUIT, 0, SPELL_RIDE_VEHICLE_RECRUIT);
                    break;
                case EVENT_CHAOS_ADARRAH:
                    SummonGrabTentacle(NPC_TENTACLE_ADARRAH, NPC_ADARRAH);
                    break;
                case EVENT_CHAOS_SOLDIER_3:
                    SoldierOverboard(NPC_STAGING_SOLDIER, 1, SPELL_RIDE_VEHICLE_SOLDIER_2);
                    break;
                case EVENT_CHAOS_SOLDIER_4:
                    SoldierOverboard(NPC_STAGING_SOLDIER, 2, SPELL_RIDE_VEHICLE_SOLDIER_3);
                    SoldierOverboard(NPC_CAPTAIN_TAYLOR, 0, SPELL_RIDE_VEHICLE_SOLDIER_2);
                    break;
                case EVENT_CHAOS_CLEANUP:
                    for (ObjectGuid guid : _overboardGuids)
                        if (Creature* crew = ObjectAccessor::GetCreature(*me, guid))
                            crew->DespawnOrUnsummon();
                    _overboardGuids.clear();
                    break;
                default:
                    break;
            }
        }
    }

private:
    Creature* FindCrew(uint32 entry, uint8 index = 0) const
    {
        std::list<Creature*> list;
        me->GetCreatureListWithEntryInGrid(list, entry, 150.0f);
        list.remove_if([&](Creature* creature)
        {
            return creature->GetTransGUID() != me->GetTransGUID();
        });
        list.sort([](Creature* left, Creature* right) { return left->GetGUID() < right->GetGUID(); });
        for (Creature* creature : list)
            if (index-- == 0)
                return creature;
        return nullptr;
    }

    float GetShipOrientation() const
    {
        if (GameObject* ship = ObjectAccessor::GetGameObject(*me, me->GetTransGUID()))
            return ship->GetOrientation();
        return me->GetOrientation();
    }

    // Two splash pads in the water beside the ship for the jump-overboard gags.
    void SummonPads()
    {
        float o = GetShipOrientation();
        for (int8 side = -1; side <= 1; side += 2)
        {
            Position pos = { me->GetPositionX() + 18.0f * std::cos(o + float(M_PI) / 2.0f) * float(side),
                             me->GetPositionY() + 18.0f * std::sin(o + float(M_PI) / 2.0f) * float(side), 0.0f, 0.0f };
            me->SummonCreature(NPC_BUDDS_VEHICLE_BUNNY, pos, TEMPSUMMON_TIMED_DESPAWN, 60s);
        }
    }

    void SummonGrabTentacle(uint32 tentacleEntry, uint32 victimEntry)
    {
        Creature* victim = FindCrew(victimEntry);
        if (!victim)
            return; // Horde ship / victim already gone

        float o = GetShipOrientation();
        Position pos = { victim->GetPositionX() + 6.0f * std::cos(o + float(M_PI) / 2.0f),
                         victim->GetPositionY() + 6.0f * std::sin(o + float(M_PI) / 2.0f),
                         victim->GetPositionZ() - 2.0f, victim->GetOrientation() };
        me->SummonCreature(tentacleEntry, pos, TEMPSUMMON_TIMED_DESPAWN, 20s);
    }

    void SoldierOverboard(uint32 entry, uint8 index, uint32 rideSpell)
    {
        if (Creature* crew = FindCrew(entry, index))
        {
            _overboardGuids.push_back(crew->GetGUID());
            crew->CastSpell(nullptr, rideSpell, true); // TargA=38 -> nearest 42202 pad (conditions)
        }
    }

    EventMap _events;
    uint8 _mackLine;
    uint8 _buddLine;
    uint8 _taylorLine;
    uint8 _shakeCount;
    std::vector<ObjectGuid> _overboardGuids;
};

/*######
## npc_vashjir_captain_taylor - 42103
## World spawn at Stormwind Harbor: runs the dock conversation loop, anchored
## to the ship's arrivals/departures. Ship-summoned copies stay passive here
## (their voyage lines are driven by the ship controller).
######*/

struct npc_vashjir_captain_taylor : public PassiveAI
{
    npc_vashjir_captain_taylor(Creature* creature) : PassiveAI(creature), _lineIndex(0) { }

    void SetData(uint32 id, uint32 value) override
    {
        if (id != DATA_SHIP_EVENT || me->GetTransport())
            return;

        switch (value)
        {
            case DOCK_EVENT_SHIP_APPROACH:
                Talk(TEXT_TAYLOR_LISTEN_UP);
                _events.ScheduleEvent(EVENT_DOCK_FALL_IN, 4s);
                break;
            case DOCK_EVENT_SHIP_DOCKED:
                Talk(TEXT_TAYLOR_MOVE_OUT);
                // Simplification: the troops do not physically march aboard;
                // the ship carries its own summoned escort.
                break;
            case DOCK_EVENT_SHIP_DEPARTED:
                _events.Reset();
                _lineIndex = 0;
                _events.ScheduleEvent(EVENT_DOCK_LINE, Seconds(DockScript[0].Offset));
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
            switch (eventId)
            {
                case EVENT_DOCK_FALL_IN:
                    Talk(TEXT_TAYLOR_FALL_IN);
                    break;
                case EVENT_DOCK_LINE:
                {
                    DockLine const& line = DockScript[_lineIndex];
                    if (Creature* speaker = GetSpeaker(line.Speaker))
                        sCreatureTextMgr->SendChat(speaker, line.Group);
                    if (++_lineIndex < std::extent<decltype(DockScript)>::value)
                        _events.ScheduleEvent(EVENT_DOCK_LINE, Seconds(DockScript[_lineIndex].Offset - line.Offset));
                    break;
                }
                default:
                    break;
            }
        }
    }

private:
    Creature* GetSpeaker(DockSpeaker speaker)
    {
        switch (speaker)
        {
            case SPEAK_TAYLOR:
                return me;
            case SPEAK_RECRUIT_22:
                return me->FindNearestCreature(NPC_STORMWIND_RECRUIT_A, 100.0f);
            case SPEAK_RECRUIT_59:
                return me->FindNearestCreature(NPC_STORMWIND_RECRUIT_B, 100.0f);
            case SPEAK_RECRUIT_95:
                return me->FindNearestCreature(NPC_STAGING_RECRUIT, 100.0f);
            case SPEAK_SOLDIER_0:
            case SPEAK_SOLDIER_1:
            case SPEAK_SOLDIER_2:
            case SPEAK_SOLDIER_3:
            {
                std::list<Creature*> soldiers;
                me->GetCreatureListWithEntryInGrid(soldiers, NPC_STORMWIND_SOLDIER, 100.0f);
                soldiers.remove_if([](Creature* creature) { return creature->GetTransport() != nullptr; });
                soldiers.sort([](Creature* left, Creature* right) { return left->GetSpawnId() < right->GetSpawnId(); });
                uint8 index = uint8(speaker - SPEAK_SOLDIER_0);
                for (Creature* soldier : soldiers)
                    if (index-- == 0)
                        return soldier;
                return nullptr;
            }
            case SPEAK_HORIZON_96:
            {
                // The "I can see it" soldier stands next to the 42095 recruit.
                Creature* recruit = me->FindNearestCreature(NPC_STAGING_RECRUIT, 100.0f);
                WorldObject* anchor = recruit ? static_cast<WorldObject*>(recruit) : static_cast<WorldObject*>(me);
                return anchor->FindNearestCreature(NPC_STAGING_SOLDIER, 100.0f);
            }
            default:
                return nullptr;
        }
    }

    EventMap _events;
    uint8 _lineIndex;
};

/*######
## npc_vashjir_budd - 39480
## Sailing lines are driven by the ship controller; this AI owns the
## jump-overboard gag (78739 ride onto a splash pad, then "lost at sea").
######*/

struct npc_vashjir_budd : public PassiveAI
{
    npc_vashjir_budd(Creature* creature) : PassiveAI(creature) { }

    void DoAction(int32 action) override
    {
        if (action != ACTION_BUDD_JUMP)
            return;

        float o = me->GetOrientation();
        if (GameObject* ship = ObjectAccessor::GetGameObject(*me, me->GetTransGUID()))
            o = ship->GetOrientation();

        Position pos = { me->GetPositionX() + 15.0f * std::cos(o - float(M_PI) / 2.0f),
                         me->GetPositionY() + 15.0f * std::sin(o - float(M_PI) / 2.0f), 0.0f, 0.0f };
        if (Creature* pad = me->SummonCreature(NPC_BUDDS_VEHICLE_BUNNY, pos, TEMPSUMMON_TIMED_DESPAWN, 30s))
            me->CastSpell(pad, SPELL_RIDE_VEHICLE_BUDD, true);

        // He fell overboard - the crew respawn at the next dock stop brings him back.
        me->DespawnOrUnsummon(12s);
    }
};

/*######
## npc_vashjir_vehicle_pad - 42202 "Budd's Vehicle Bunny"
## Water-splash landing pad: splashes whenever someone lands on it.
######*/

struct npc_vashjir_vehicle_pad : public PassiveAI
{
    npc_vashjir_vehicle_pad(Creature* creature) : PassiveAI(creature) { }

    void PassengerBoarded(Unit* /*passenger*/, int8 /*seatId*/, bool apply) override
    {
        if (apply)
            me->CastSpell(me, SPELL_WATER_SPLASH, true);
    }
};

/*######
## npc_vashjir_grab_tentacle - 36826/36835/36846/39620/39652/39661/42208
## Cosmetic deck-chaos tentacles: emerge beside a victim, grab (force-cast the
## matching ride spell), submerge and drag the victim under.
######*/

struct npc_vashjir_grab_tentacle : public PassiveAI
{
    npc_vashjir_grab_tentacle(Creature* creature) : PassiveAI(creature) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        _events.ScheduleEvent(EVENT_TENTACLE_GRAB, 2s);
        _events.ScheduleEvent(EVENT_TENTACLE_SUBMERGE, 8s);
        _events.ScheduleEvent(EVENT_TENTACLE_VANISH, 10s);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_TENTACLE_GRAB:
                    if (uint32 spellId = GetGrabSpell())
                        me->CastSpell(nullptr, spellId, true);
                    break;
                case EVENT_TENTACLE_SUBMERGE:
                    me->CastSpell(me, SPELL_SUBMERGE_VISUAL, true);
                    break;
                case EVENT_TENTACLE_VANISH:
                {
                    // Drag grabbed crew down with us.
                    if (Vehicle* kit = me->GetVehicleKit())
                    {
                        std::vector<Creature*> riders;
                        for (SeatMap::const_iterator itr = kit->Seats.begin(); itr != kit->Seats.end(); ++itr)
                            if (Unit* passenger = ObjectAccessor::GetUnit(*me, itr->second.Passenger.Guid))
                                if (Creature* creature = passenger->ToCreature())
                                    riders.push_back(creature);
                        for (Creature* rider : riders)
                        {
                            rider->ExitVehicle();
                            rider->DespawnOrUnsummon();
                        }
                    }
                    me->DespawnOrUnsummon();
                    break;
                }
                default:
                    break;
            }
        }
    }

private:
    uint32 GetGrabSpell() const
    {
        switch (me->GetEntry())
        {
            case NPC_TENTACLE_GREMBUL:   return SPELL_TENTACLE_VS_GREMBUL;
            case NPC_TENTACLE_BELINDAH:  return SPELL_TENTACLE_VS_BELINDAH;
            case NPC_TENTACLE_SAMIR:     return SPELL_TENTACLE_VS_SAMIR;
            case NPC_TENTACLE_ADARRAH:   return SPELL_TENTACLE_VS_ADARRAH;
            case NPC_TENTACLE_KNOCKBACK: return SPELL_TENTACLE_KNOCKBACK;
            default:                     return 0; // scenery tentacles just emerge/submerge
        }
    }

    EventMap _events;
};

/*######
## npc_vashjir_intro_tentacle - 36878 (vehicle 559), per-player grab
######*/

struct npc_vashjir_intro_tentacle : public PassiveAI
{
    npc_vashjir_intro_tentacle(Creature* creature) : PassiveAI(creature) { }

    void IsSummonedBy(Unit* summoner) override
    {
        Player* player = summoner->ToPlayer();
        if (!player)
        {
            me->DespawnOrUnsummon();
            return;
        }

        _playerGUID = player->GetGUID();
        _events.ScheduleEvent(EVENT_INTRO_TENTACLE_ENTRY, 5s);
    }

    void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
    {
        // The player transfers from us onto the submerge bunny: sink away.
        if (!apply && passenger->GetGUID() == _playerGUID)
        {
            me->CastSpell(me, SPELL_SUBMERGE_VISUAL, true);
            me->DespawnOrUnsummon(3s);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID);
            if (!player)
            {
                me->DespawnOrUnsummon();
                return;
            }

            switch (eventId)
            {
                case EVENT_INTRO_TENTACLE_ENTRY:
                    me->CastSpell(me, SPELL_TENTACLE_ENTRY, true);
                    _events.ScheduleEvent(EVENT_INTRO_TENTACLE_RIDE, 2s);
                    break;
                case EVENT_INTRO_TENTACLE_RIDE:
                    if (!IsEligibleForGrab(player))
                    {
                        me->DespawnOrUnsummon();
                        return;
                    }
                    player->CastSpell(me, SPELL_PLAYER_RIDE_TENTACLE, true);
                    _events.ScheduleEvent(EVENT_INTRO_TENTACLE_HANDOFF, Milliseconds(2500));
                    break;
                case EVENT_INTRO_TENTACLE_HANDOFF:
                    // Summons the 36901 submerge bunny (spell_target_position);
                    // its AI takes over the rest of the chain.
                    player->CastSpell(player, SPELL_SUMMON_SUBMERGE_BUNNY, true);
                    break;
                default:
                    break;
            }
        }
    }

private:
    EventMap _events;
    ObjectGuid _playerGUID;
};

/*######
## npc_vashjir_submerge_bunny - 36901 (vehicle 561), per-player controller
## Owns the drag-down, the Erunak rescue scene and the wake-up teleport.
######*/

struct npc_vashjir_submerge_bunny : public PassiveAI
{
    npc_vashjir_submerge_bunny(Creature* creature) : PassiveAI(creature) { }

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

        _events.ScheduleEvent(EVENT_BUNNY_RIDE, 2s);
        _events.ScheduleEvent(EVENT_BUNNY_VALIDATE, 5s);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID);
            if (!player)
            {
                Cleanup();
                return;
            }

            switch (eventId)
            {
                case EVENT_BUNNY_VALIDATE:
                    // Player must be riding us until the finale ejects them.
                    if (player->GetVehicleBase() != me)
                    {
                        if (!_finished)
                        {
                            player->RemoveAurasDueToSpell(SPELL_SUBMERGE_BUNNY_PHASE);
                            player->RemoveAurasDueToSpell(SPELL_BUBBLE_SELF_INTRO);
                            Cleanup();
                            return;
                        }
                        break;
                    }
                    _events.ScheduleEvent(EVENT_BUNNY_VALIDATE, 5s);
                    break;
                case EVENT_BUNNY_RIDE:
                    if (!player->IsAlive())
                    {
                        Cleanup();
                        return;
                    }
                    // We were summoned while the player was still in phase 170;
                    // join the wreck phase so the ride stays visible to them.
                    PhasingHandler::AddPhase(me, PHASE_WRECK, true);
                    player->CastSpell(me, SPELL_RIDE_SUBMERGE_BUNNY, true);
                    // A261 MiscValueB 179 + stun; faction-neutral effects, used for both sides
                    // (no "Submerge Bunny Phase - Horde" spell exists in 4.3.4).
                    player->CastSpell(player, SPELL_SUBMERGE_BUNNY_PHASE, true);
                    player->RemoveAurasDueToSpell(SPELL_PHASE_1_INTRO_AURA);
                    PhasingHandler::RemovePhase(player, PHASE_HARBOR, false);
                    PhasingHandler::RemovePhase(player, PHASE_SHIP, true);
                    _events.ScheduleEvent(EVENT_BUNNY_DESCEND, 4s);
                    break;
                case EVENT_BUNNY_DESCEND:
                    me->GetMotionMaster()->MoveSmoothPath(POINT_BUNNY_DESCENT, BunnyDescentPath,
                        std::extent<decltype(BunnyDescentPath)>::value, false, true);
                    _events.ScheduleEvent(EVENT_BUNNY_SEAT_2, 5s);
                    _events.ScheduleEvent(EVENT_BUNNY_SUMMON_SCENE, 14s);
                    break;
                case EVENT_BUNNY_SEAT_2:
                    player->CastSpell(me, SPELL_RIDE_SUBMERGE_BUNNY_SEAT2, true);
                    break;
                case EVENT_BUNNY_SUMMON_SCENE:
                    // Both summoned by the PLAYER so that Erunak's summoner-targeted
                    // casts (75746 etc.) resolve to the player; positions come from
                    // spell_target_position (shared wreck site for both factions).
                    player->CastSpell(player, SPELL_SUMMON_ERUNAK, true);
                    player->CastSpell(player, SPELL_SUMMON_NAGA_ASSAILANT, true);
                    _events.ScheduleEvent(EVENT_BUNNY_NAGA_MOVE, Milliseconds(1500));
                    _events.ScheduleEvent(EVENT_BUNNY_SEAT_3, 7s);
                    break;
                case EVENT_BUNNY_NAGA_MOVE:
                    if (Creature* naga = FindPlayerSummon(NPC_NAGA_ASSAILANT))
                    {
                        naga->SetDisableGravity(true);
                        naga->GetMotionMaster()->MovePoint(POINT_NAGA_SCENE, NagaScenePos, false);
                    }
                    break;
                case EVENT_BUNNY_SEAT_3:
                    player->CastSpell(me, SPELL_RIDE_SUBMERGE_BUNNY_SEAT3, true);
                    me->CastSpell(nullptr, SPELL_INTRO_DAZED, true); // TargA=92 -> our summoner
                    if (Creature* naga = FindPlayerSummon(NPC_NAGA_ASSAILANT))
                        naga->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK_UNARMED);
                    _events.ScheduleEvent(EVENT_BUNNY_LAVA_BOLT, 2s);
                    break;
                case EVENT_BUNNY_LAVA_BOLT:
                    if (Creature* erunak = FindPlayerSummon(NPC_ERUNAK_RESCUER))
                        if (Creature* naga = FindPlayerSummon(NPC_NAGA_ASSAILANT))
                            erunak->CastSpell(naga, SPELL_LAVA_BOLT, true);
                    _events.ScheduleEvent(EVENT_BUNNY_TALK_DEPTHS, Milliseconds(1500));
                    break;
                case EVENT_BUNNY_TALK_DEPTHS:
                    if (Creature* erunak = FindPlayerSummon(NPC_ERUNAK_RESCUER))
                        sCreatureTextMgr->SendChat(erunak, TEXT_ERUNAK_TO_THE_DEPTHS);
                    _events.ScheduleEvent(EVENT_BUNNY_NAGA_DIE, 1s);
                    break;
                case EVENT_BUNNY_NAGA_DIE:
                    if (Creature* naga = FindPlayerSummon(NPC_NAGA_ASSAILANT))
                    {
                        naga->CastSpell(naga, SPELL_SUMMON_NAGA_DEATH_BUNNY, true);
                        naga->CastSpell(naga, SPELL_SUICIDE_NO_LOG, true);
                    }
                    _events.ScheduleEvent(EVENT_BUNNY_TURTLE_PARTS, 1s);
                    break;
                case EVENT_BUNNY_TURTLE_PARTS:
                    if (Creature* debris = me->FindNearestCreature(NPC_NAGA_DEATH_BUNNY, 60.0f))
                    {
                        debris->CastSpell(debris, SPELL_TURTLE_PARTS_00, true);
                        debris->CastSpell(debris, SPELL_TURTLE_PARTS_01, true);
                        debris->CastSpell(debris, SPELL_TURTLE_PARTS_02, true);
                        debris->CastSpell(debris, SPELL_RED_RADIATION, true);
                    }
                    _events.ScheduleEvent(EVENT_BUNNY_BUBBLE, Milliseconds(1500));
                    break;
                case EVENT_BUNNY_BUBBLE:
                    if (Creature* erunak = FindPlayerSummon(NPC_ERUNAK_RESCUER))
                    {
                        erunak->CastSpell(nullptr, SPELL_BLOW_BUBBLE, true); // TargA=92 -> player
                        erunak->CastSpell(player, SPELL_INVISIBLE_CHANNEL_BEAM, true);
                    }
                    _events.ScheduleEvent(EVENT_BUNNY_HOLD_ON, Milliseconds(2500));
                    break;
                case EVENT_BUNNY_HOLD_ON:
                    if (Creature* erunak = FindPlayerSummon(NPC_ERUNAK_RESCUER))
                        sCreatureTextMgr->SendChat(erunak, TEXT_ERUNAK_HOLD_ON, player);
                    _events.ScheduleEvent(EVENT_BUNNY_BUBBLE_SELF, Milliseconds(500));
                    break;
                case EVENT_BUNNY_BUBBLE_SELF:
                    player->CastSpell(player, SPELL_BUBBLE_SELF_INTRO, true);
                    _events.ScheduleEvent(EVENT_BUNNY_BLACKOUT, 3s);
                    break;
                case EVENT_BUNNY_BLACKOUT:
                    me->CastSpell(nullptr, SPELL_BLACKOUT_TIMER, true); // TargA=92 -> player
                    _events.ScheduleEvent(EVENT_BUNNY_SCREEN_EFFECT, 3s);
                    break;
                case EVENT_BUNNY_SCREEN_EFFECT:
                    player->CastSpell(player, SPELL_DROWNED_SCREEN_EFFECT, true);
                    _events.ScheduleEvent(EVENT_BUNNY_FINALE, 3s);
                    break;
                case EVENT_BUNNY_FINALE:
                    _finished = true;
                    // Removes 69524 (phase 179 + stun) and 75751 on our summoner.
                    me->CastSpell(nullptr, SPELL_LEAVE_SUBMERGE_BUNNY, true);
                    player->ExitVehicle();
                    FinishIntroArrival(player);
                    if (Creature* erunak = FindPlayerSummon(NPC_ERUNAK_RESCUER))
                        erunak->DespawnOrUnsummon(3s);
                    if (Creature* naga = FindPlayerSummon(NPC_NAGA_ASSAILANT))
                        naga->DespawnOrUnsummon(3s);
                    me->DespawnOrUnsummon(2s);
                    break;
                default:
                    break;
            }
        }
    }

private:
    // Finds the scene actor that THIS player summoned (multiple players may run
    // the event concurrently in the shared 179 phase).
    Creature* FindPlayerSummon(uint32 entry) const
    {
        std::list<Creature*> list;
        me->GetCreatureListWithEntryInGrid(list, entry, 120.0f);
        for (Creature* creature : list)
            if (TempSummon* summon = creature->ToTempSummon())
                if (summon->GetSummonerGUID() == _playerGUID)
                    return creature;
        return nullptr;
    }

    void Cleanup()
    {
        if (Creature* erunak = FindPlayerSummon(NPC_ERUNAK_RESCUER))
            erunak->DespawnOrUnsummon();
        if (Creature* naga = FindPlayerSummon(NPC_NAGA_ASSAILANT))
            naga->DespawnOrUnsummon();
        me->DespawnOrUnsummon();
    }

    EventMap _events;
    ObjectGuid _playerGUID;
    bool _finished = false;
};

} // namespace Vashjir::Intro

void AddSC_vashjir_intro()
{
    using namespace Vashjir::Intro;

    new player_vashjir_intro();
    new transport_vashjir_ship_a();
    new transport_vashjir_ship_h();
    RegisterCreatureAI(npc_vashjir_ship_controller);
    RegisterCreatureAI(npc_vashjir_captain_taylor);
    RegisterCreatureAI(npc_vashjir_budd);
    RegisterCreatureAI(npc_vashjir_vehicle_pad);
    RegisterCreatureAI(npc_vashjir_grab_tentacle);
    RegisterCreatureAI(npc_vashjir_intro_tentacle);
    RegisterCreatureAI(npc_vashjir_submerge_bunny);
}
