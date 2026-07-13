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

#ifndef DEF_HOUROFTWILIGHT_H
#define DEF_HOUROFTWILIGHT_H

#include "CreatureAIImpl.h"
#include "Define.h"

namespace HourOfTwilight
{
constexpr char const* DataHeader = "HOT";
#define HOTScriptName "instance_hour_of_twilight"

uint32 const EncounterCount = 3;

enum HOTDataTypes
{
    // Bosses
    DATA_ARCURION                   = 0,
    DATA_ASIRA_DAWNSLAYER           = 1,
    DATA_ARCHBISHOP_BENEDICTUS      = 2,

    // ObjectData accessors
    DATA_THRALL_ENTRANCE            = 3,   // 54548
    DATA_THRALL_FROZEN              = 4,   // 55779
    DATA_THRALL_GALAKROND           = 5,   // 54972
    DATA_THRALL_TITANS              = 6,   // 54634
    DATA_THRALL_EPILOGUE            = 7,   // 54971 - Benedictus ally + quest ender
    DATA_ICEWALL_ARENA              = 8,   // GO 210049 - open by default, closes at Arcurion's reveal
    DATA_ICEWALL_EXIT               = 9,   // GO 210048 - closed until Arcurion dies
    DATA_EXIT_PORTAL                = 10,  // GO 210026
    DATA_LIFE_WARDEN_THRALLS        = 11,  // 55415 - the crashed drake

    // SetData / GetData channels
    DATA_ESCORT_STAGE               = 20   // persisted, see EscortStage
};

// Escort checkpoints - persisted across wipes and server restarts.
enum EscortStage : uint32
{
    STAGE_NONE                      = 0,   // fresh instance, Thrall waiting at the entrance
    STAGE_CANYON_ESCORT             = 1,   // leg 1 running (canyon ambush waves)
    STAGE_ARCURION_READY            = 2,   // leg 1 done, frozen-leg Thrall offers the ready check
    STAGE_ARCURION_DONE             = 3,   // exit wall open, leg 2 Thrall available
    STAGE_GALAKROND_ESCORT          = 4,   // leg 2 running (assassin ambushes)
    STAGE_ASIRA_READY               = 5,   // Asira arrived on the crashing Life Warden
    STAGE_ASIRA_DONE                = 6,   // taxis available, leg 3 Thrall waiting across the wastes
    STAGE_TITANS_ESCORT             = 7,   // leg 3 running (faceless gauntlet)
    STAGE_BENEDICTUS_READY          = 8,   // gauntlet done, reveal pending at the chamber
    STAGE_BENEDICTUS_DONE           = 9    // epilogue gossip + exit portal
};

enum HOTCreatures
{
    // Bosses
    NPC_ARCURION                    = 54590,
    NPC_ASIRA_DAWNSLAYER            = 54968,
    NPC_ARCHBISHOP_BENEDICTUS       = 54938,

    // Thrall (one entry per escort leg)
    NPC_THRALL_ENTRANCE             = 54548,
    NPC_THRALL_FROZEN               = 55779,
    NPC_THRALL_GALAKROND            = 54972,
    NPC_THRALL_TITANS               = 54634,
    NPC_THRALL_EPILOGUE             = 54971,

    // Canyon leg / Arcurion encounter
    NPC_FROZEN_SERVITOR             = 54555, // pre-placed canyon ambushers
    NPC_FROZEN_SERVITOR_SUMMON      = 54600, // arena rim adds
    NPC_CRYSTALLINE_ELEMENTAL       = 55559,
    NPC_FROZEN_SHARD                = 55563,
    NPC_ICY_TOMB                    = 54995,
    NPC_SERVITOR_SPAWN_POINT        = 54598, // x20 arena-rim spawn markers
    NPC_ARCURION_SPAWN_VISUAL       = 57197,
    NPC_ICE_WALL_EXIT_STALKER       = 55728,

    // Galakrond leg / Asira encounter
    NPC_TWILIGHT_ASSASSIN           = 55106,
    NPC_TWILIGHT_RANGER             = 55107,
    NPC_TWILIGHT_SHADOW_WALKER      = 55109,
    NPC_TWILIGHT_THUG               = 55111,
    NPC_TWILIGHT_BRUISER            = 55112,
    NPC_RISING_FIRE_TOTEM           = 55474,

    // Life Warden flight
    NPC_LIFE_WARDEN_THRALL          = 55415, // crashed drake: Asira arrives on it, Thrall departs on it
    NPC_LIFE_WARDEN_TAXI            = 55549, // x5 player taxis, spell-click 103989

    // Path of the Titans gauntlet
    NPC_FACELESS_BRUTE              = 54632,
    NPC_FACELESS_SHADOW_WEAVER      = 54633,
    NPC_SHADOW_BORER                = 54686,
    NPC_DARK_HAZE                   = 54628,
    NPC_CORRUPTED_SLIME             = 54646, // rains from the corrupted temple during the gauntlet

    // Benedictus encounter
    NPC_PURIFYING_LIGHT             = 55377, // P1 orb
    NPC_CORRUPTING_TWILIGHT         = 55467, // P2 orb (attackable)
    NPC_PURIFYING_BLAST             = 55427, // P1 ground pool (103653 ticks)
    NPC_TWILIGHT_BLAST              = 55468, // P2 ground pool (103775 ticks)
    NPC_WAVE_OF_VIRTUE              = 55441, // P1 wave rider
    NPC_WAVE_OF_TWILIGHT            = 55469, // P2 wave rider
    NPC_WATER_SHELL                 = 55447, // Thrall's protective bubble (P1)
    NPC_HOLY_SHIELD                 = 54955, // ramp-seal stalker (Holy Wall 102629)
    NPC_EARTHEN_SHELL_TARGET        = 55445, // pool-state controller at the platform
    NPC_TWILIGHT_SPARK              = 55466  // Eclipse targets
};

enum HOTGameObjectIds
{
    GO_ICEWALL_ARENA                = 210049,
    GO_ICEWALL_EXIT                 = 210048,
    GO_EXIT_PORTAL                  = 210026
};

enum HOTActions
{
    // Instance -> creature signals
    ACTION_START_ESCORT_INTRO       = 1,  // Arcurion: rim reinforcements announce (relayed to Thrall)
    ACTION_THRALL_FREED             = 2,  // Arcurion at 30% - tomb shatters for good
    ACTION_TOMB_DESTROYED           = 3,  // players broke the Icy Tomb
    ACTION_ARCURION_DEAD            = 4,  // post-fight RP for the frozen-leg Thrall
    ACTION_ASIRA_ARRIVES            = 5,  // Life Warden crash + Asira drop-in
    ACTION_ASIRA_ENGAGED            = 6,  // Thrall enters ally mode
    ACTION_ASIRA_DEAD               = 7,  // Thrall heals the warden and flies off
    ACTION_BENEDICTUS_MEET_PARTY    = 8,  // "Get inside, quickly!" ramp RP
    ACTION_BENEDICTUS_ENGAGED       = 9,  // Thrall enters P1 support mode
    ACTION_THRALL_IMPRISONED        = 10, // P2: support off
    ACTION_THRALL_RELEASED          = 11, // Benedictus dead
    ACTION_CONTROLLER_ENGAGE        = 12, // Earthen Shell Target: P1 water state
    ACTION_CONTROLLER_TWILIGHT      = 13, // Earthen Shell Target: P2 water state
    ACTION_CONTROLLER_RESET         = 14,
    ACTION_THRALL_ENTER_CHAMBER     = 15, // Benedictus returned to his anchor - gauntlet Thrall walks in
    ACTION_BENEDICTUS_REVEAL        = 16  // fight-Thrall in position - the betrayal dialogue begins
};

template<class AI>
AI* GetHourOfTwilightAI(Creature* creature)
{
    return GetInstanceAI<AI>(creature, HOTScriptName);
}

#define RegisterHourOfTwilightCreatureAI(ai_name) RegisterCreatureAIWithFactory(ai_name, GetHourOfTwilightAI)
}

#endif // DEF_HOUROFTWILIGHT_H
