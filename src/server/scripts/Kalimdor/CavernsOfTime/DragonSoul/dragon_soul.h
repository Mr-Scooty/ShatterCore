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

#ifndef DEF_DRAGONSOUL_H
#define DEF_DRAGONSOUL_H

#include "Define.h"
#include "Creature.h"
#include "CreatureAIImpl.h"

namespace DragonSoul
{
constexpr char const* DataHeader = "DS";
#define DSScriptName "instance_dragon_soul"

uint32 const EncounterCount = 8;

enum DSDataTypes
{
    // Bosses
    DATA_MORCHOK                = 0,
    DATA_WARLORD_ZONOZZ         = 1,
    DATA_YORSAHJ_THE_UNSLEEPING = 2,
    DATA_HAGARA_THE_STORMBINDER = 3,
    DATA_ULTRAXION              = 4,
    DATA_WARMASTER_BLACKHORN    = 5,
    DATA_SPINE_OF_DEATHWING     = 6,
    DATA_MADNESS_OF_DEATHWING   = 7,

    // Additional Data
    DATA_DEATHWING_MADNESS_OF_DEATHWING,
    DATA_THRALL_MADNESS_OF_DEATHWING,
    DATA_YSERA_MADNESS_OF_DEATHWING,
    DATA_ALEXSTRASZA_MADNESS_OF_DEATHWING,
    DATA_NOZDORMU_MADNESS_OF_DEATHWING,
    DATA_KALECGOS_MADNESS_OF_DEATHWING,
    DATA_TAIL_TENTACLE_MADNESS_OF_DEATHWING,

    // Morchok
    DATA_KOHCROM,
    DATA_MORCHOK_ACHIEVEMENT_FAILED,

    // Warlord Zon'ozz
    DATA_ZONOZZ_PING_PONG,

    // Yor'sahj
    DATA_YORSAHJ_TASTE_THE_RAINBOW,

    // Raid Finder
    DATA_IS_LFR
};

// Taste the Rainbow! (achievement 6129) - color pairs the boss was empowered
// by on a kill, reported by the boss AI to the instance script
enum YorsahjRainbowBits
{
    RAINBOW_BIT_BLACK_YELLOW  = 0x1, // criteria 18495
    RAINBOW_BIT_RED_GREEN     = 0x2, // criteria 18496
    RAINBOW_BIT_BLACK_BLUE    = 0x4, // criteria 18497
    RAINBOW_BIT_PURPLE_YELLOW = 0x8  // criteria 18498
};

// Raid Finder tuning: LFR runs are 25 player normal instances with reduced
// boss health (stats template below) and reduced outgoing boss damage.
constexpr uint32 LFR_DAMAGE_PCT = 70;                   // playtest knob - no retail data
constexpr uint32 NPC_MORCHOK_LFR_STATS = 58226;         // HealthModifier 350 = ~30.06M

// Yor'sahj LFR stats templates (community retail values)
constexpr uint32 NPC_YORSAHJ_LFR_STATS               = 58227; // ~106.0M
constexpr uint32 NPC_YORSAHJ_GLOBULE_LFR_STATS       = 58228; // ~4.1M
constexpr uint32 NPC_YORSAHJ_MANA_VOID_LFR_STATS     = 58229; // ~3.8M
constexpr uint32 NPC_YORSAHJ_FORGOTTEN_ONE_LFR_STATS = 58230; // ~1.1M

constexpr uint32 NPC_ZONOZZ_LFR_STATS                = 58231; // ~143.0M (70% of 25N)

enum DSCreatures
{
    // Bosses
    BOSS_MORCHOK                                = 55265,
    BOSS_WARLORD_ZONOZZ                         = 55308,
    BOSS_YORSAHJ                                = 55312,
    BOSS_MADNESS_OF_DEATHWING                   = 56173,

    /*Morchok*/
    NPC_KOHCROM                                 = 57773,
    NPC_RESONATING_CRYSTAL                      = 55346,

    /*Warlord Zon'ozz*/
    NPC_VOID_OF_THE_UNMAKING                    = 55334,
    NPC_EYE_OF_GORATH                           = 55416,
    NPC_FLAIL_OF_GORATH                         = 55417,
    NPC_CLAW_OF_GORATH                          = 55418,

    /*Yor'sahj the Unsleeping*/
    NPC_ACIDIC_GLOBULE                          = 55862, // green
    NPC_SHADOWED_GLOBULE                        = 55863, // purple
    NPC_GLOWING_GLOBULE                         = 55864, // yellow
    NPC_CRIMSON_GLOBULE                         = 55865, // red
    NPC_COBALT_GLOBULE                          = 55866, // blue
    NPC_DARK_GLOBULE                            = 55867, // black
    NPC_FORGOTTEN_ONE                           = 56265,
    NPC_MANA_VOID                               = 56231,
    NPC_MAW_OF_SHUMA                            = 55544,

    /*Madness of Deathwing*/
    NPC_DEATHWING_MADNESS_OF_DEATHWING          = 57962,
    NPC_ARM_TENTACLE_1                          = 56167,
    NPC_ARM_TENTACLE_2                          = 56846,
    NPC_WING_TENTACLE                           = 56168,
    NPC_TAIL_TENTACLE                           = 56844,
    NPC_MUTATED_CORRUPTION                      = 56471,
    NPC_CRUSH_TARGET                            = 56581,
    NPC_PLATFORM                                = 56307,
    NPC_HEMORRHAGE_TARGET                       = 56359,
    NPC_ELEMENTIUM_BOLT                         = 56262,
    NPC_BLISTERING_TENTACLE                     = 56188,
    NPC_TIME_ZONE_TARGET                        = 56332,
    NPC_TIME_ZONE                               = 56311,
    NPC_COSMETIC_TENTACLE                       = 57693,
    NPC_ELEMENTIUM_FRAGMENT                     = 56724,
    NPC_ELEMENTIUM_TERROR                       = 56710,
    NPC_JUMP_PAD                                = 56699,
    NPC_THRALL_MADNESS_OF_DEATHWING             = 56103,
    NPC_YSERA_MADNESS_OF_DEATHWING              = 56100,
    NPC_ALEXSTRASZA_MADNESS_OF_DEATHWING        = 56099,
    NPC_NOZDORMU_MADNESS_OF_DEATHWING           = 56102,
    NPC_KALECGOS_MADNESS_OF_DEATHWING           = 56101
};

enum DSGameObjectIds
{
    GO_MORCHOK_ROCK_SPIKE            = 209596,
    GO_ELEMENTIUM_FRAGMENT_10_NORMAL = 210079,
    GO_ELEMENTIUM_FRAGMENT_25_NORMAL = 210218,
    GO_ELEMENTIUM_FRAGMENT_25_LFR    = 210220,
    GO_ELEMENTIUM_FRAGMENT_10_HEROIC = 210219,
    GO_ELEMENTIUM_FRAGMENT_25_HEROIC = 210217
};

enum DSSpells
{
    SPELL_CALM_MAELSTROM_SKYBOX = 109480
};

enum DSMapObjIds
{
    /*
    * Data Values:
    * 1, 0, 60,  0, 0, 0  -- Talk
    * 1, 0, 213, 0, 0, 0  -- Scream in Agony
    */
    MAP_OBJ_ID_SPINE_OF_DEATHWING_HEAD  = 6574436,
    /*
    * Data Values:
    * 1, 1, [(0 - 100)], 0, 0, 0 -- Skybox cloud speed
    */
    MAP_OBJ_ID_SKYFIRE_SKYBOX           = 6858573
};

template<class AI>
AI* GetDragonSoulAI(Creature* creature)
{
    return GetInstanceAI<AI>(creature, DSScriptName);
}

#define RegisterDragonSoulCreatureAI(ai_name) RegisterCreatureAIWithFactory(ai_name, GetDragonSoulAI)
}

#endif // DEF_DRAGONSOUL_H
