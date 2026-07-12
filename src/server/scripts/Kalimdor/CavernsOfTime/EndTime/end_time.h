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

#ifndef DEF_ENDTIME_H
#define DEF_ENDTIME_H

#include "CreatureAIImpl.h"
#include "Define.h"

class Creature;

namespace EndTime
{
constexpr char const* DataHeader = "ET";
#define ETScriptName "instance_end_time"

uint32 const EncounterCount = 5;

enum ETDataTypes
{
    // Bosses
    DATA_ECHO_OF_BAINE      = 0,
    DATA_ECHO_OF_JAINA      = 1,
    DATA_ECHO_OF_SYLVANAS   = 2,
    DATA_ECHO_OF_TYRANDE    = 3,
    DATA_MUROZOND           = 4,

    // Additional Data
    DATA_HOURGLASS_OF_TIME,
    DATA_MUROZOND_INTRO,
    DATA_NOZDORMU_BRONZE_DRAGON_SHRINE,
    DATA_ARCANE_CIRCLE,
    DATA_COLLECTED_FRAGMENT_OF_JAINAS_STAFF,

    // Random echo selection
    DATA_ACTIVE_ECHO_1,
    DATA_ACTIVE_ECHO_2,

    // Echo of Tyrande
    DATA_SHADOW_GAUNTLET,
    DATA_MOON_GUARD_ELIGIBLE,

    // Echo of Sylvanas
    DATA_SEVERED_TIES_ELIGIBLE
};

enum ETCreatures
{
    // Bosses
    BOSS_MUROZOND                   = 54432,
    BOSS_ECHO_OF_JAINA              = 54445,
    BOSS_ECHO_OF_BAINE              = 54431,
    BOSS_ECHO_OF_SYLVANAS           = 54123,
    BOSS_ECHO_OF_TYRANDE            = 54544,

    // Encounter Related Creatures
    /*Murozond*/
    NPC_INFINITE_WARDEN             = 54923,
    NPC_INFINITE_SUPRESSOR          = 54920,
    NPC_NOZDORMU_DRAGON_SHRINES     = 54751,
    NPC_MIRROR_IMAGE                = 54435,

    /*Echo of Jaina*/
    NPC_ARCANE_CIRCLE               = 54639,
    NPC_FROST_BLADE                 = 54494,
    NPC_BLINK_TARGET                = 54542,
    NPC_FLARECORE_EMBER             = 54446,

    /*Echo of Baine*/
    NPC_BAINES_TOTEM                = 54434,
    NPC_BAINES_TOTEM_VISUAL         = 54433,
    NPC_ROCK_ISLAND                 = 54496,

    /*Echo of Sylvanas*/
    NPC_RISEN_GHOUL                 = 54191,
    NPC_GHOUL_ANCHOR                = 54197,
    NPC_BLIGHTED_ARROWS             = 54403,
    NPC_BRITTLE_GHOUL               = 54952,

    /*Echo of Tyrande*/
    NPC_MOONLANCE                   = 54574,
    NPC_MOONLANCE_SPLIT_LEFT        = 54580,
    NPC_MOONLANCE_SPLIT_CENTER      = 54581,
    NPC_MOONLANCE_SPLIT_RIGHT       = 54582,
    NPC_EYE_OF_ELUNE_1              = 54594,
    NPC_EYE_OF_ELUNE_2              = 54597,
    NPC_EYE_OF_ELUNE_3              = 54939,
    NPC_EYE_OF_ELUNE_4              = 54940,
    NPC_EYE_OF_ELUNE_5              = 54941,
    NPC_EYE_OF_ELUNE_6              = 54942,
    NPC_POOL_OF_MOONLIGHT           = 54508,
    NPC_TIME_TWISTED_SENTINEL       = 54512,
    NPC_TIME_TWISTED_HUNTRESS       = 54701,
    NPC_TIME_TWISTED_NIGHTSABER_1   = 54688,
    NPC_TIME_TWISTED_NIGHTSABER_2   = 54699,
    NPC_TIME_TWISTED_NIGHTSABER_3   = 54700,

    /*Entryway*/
    NPC_NOZDORMU_ENTRANCE           = 54476,
    NPC_ALURMI                      = 57864
};

enum ETGameObjectIds
{
    GO_HOURGLASS_OF_TIME            = 209249,
    GO_MUROZONDS_TEMPORAL_CACHE     = 209547,
    GO_FRAGMENT_OF_JAINAS_STAFF     = 209318,

    // One Time Transit Device entry per shrine
    GO_TIME_TRANSIT_DEVICE_1        = 209437,
    GO_TIME_TRANSIT_DEVICE_2        = 209438,
    GO_TIME_TRANSIT_DEVICE_3        = 209439,
    GO_TIME_TRANSIT_DEVICE_4        = 209440,
    GO_TIME_TRANSIT_DEVICE_5        = 209441,
    GO_TIME_TRANSIT_DEVICE_6        = 209442,
    GO_TIME_TRANSIT_DEVICE_7        = 209443,

    // Echo of Baine platforms
    GO_PLATFORM_1                   = 209670,
    GO_PLATFORM_2                   = 209693,
    GO_PLATFORM_3                   = 209694,
    GO_PLATFORM_4                   = 209695,
    GO_FIRE_WALL                    = 209990
};

enum ETMapEvents
{
    MAP_EVENT_AZURE_DRAGONSHRINE_ENTERED    = 29225, // fired by Teleport to Blue Dragonshrine (102126)
    MAP_EVENT_MOON_GUARD_FAILED             = 29235, // fired by Tyrande Achievement Fail (102539)
    MAP_EVENT_RUBY_DRAGONSHRINE_ENTERED     = 29244, // fired by Teleport to Ruby Dragonshrine (102579)
    MAP_EVENT_OBSIDIAN_DRAGONSHRINE_ENTERED = 29405, // fired by Teleport to Black Dragonshrine (103868)
    MAP_EVENT_EMERALD_DRAGONSHRINE_ENTERED  = 29508, // fired by Teleport to Emerald Dragonshrine (104761)
    MAP_EVENT_BRONZE_DRAGONSHRINE_ENTERED   = 29510  // fired by Teleport to Bronze Dragonshrine (104764)
};

template<class AI>
AI* GetEndTimeAI(Creature* creature)
{
    return GetInstanceAI<AI>(creature, ETScriptName);
}

#define RegisterEndTimeCreatureAI(ai_name) RegisterCreatureAIWithFactory(ai_name, GetEndTimeAI)
}

#endif // DEF_ENDTIME_H
