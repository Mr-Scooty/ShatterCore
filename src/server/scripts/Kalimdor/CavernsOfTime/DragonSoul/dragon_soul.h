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

    // Hagara the Stormbinder
    DATA_HAGARA_INTRO_DONE,        // persisted: intro assault event completed this lockout
    DATA_HAGARA_HOLDING_HANDS,     // boss AI -> instance: unbroken-chain lightning phase
    DATA_TRAVEL_TO_EYE_OF_ETERNITY,

    // Ultraxion
    DATA_ULTRAXION_GAUNTLET_DONE,      // persisted: Twilight Assaulter gauntlet cleared this lockout
    DATA_ULTRAXION_ACHIEVEMENT_FAILED, // boss AI -> instance: someone was hit by Hour of Twilight twice
    DATA_ULTRAXION_GAUNTLET_CONTROLLER,
    DATA_TRAVEL_TO_WYRMREST_SUMMIT,
    DATA_THRALL_ULTRAXION,
    DATA_ALEXSTRASZA_ULTRAXION,
    DATA_YSERA_ULTRAXION,
    DATA_KALECGOS_ULTRAXION,
    DATA_NOZDORMU_ULTRAXION,
    DATA_DEATHWING_ULTRAXION,

    // Warmaster Blackhorn
    DATA_GORIONA,
    DATA_THE_SKYFIRE,                    // ship health proxy (encounter frame)
    DATA_GUNSHIP_PURSUIT_CONTROLLER,     // encounter state machine (56599)
    DATA_SKY_CAPTAIN_SWAYZE,
    DATA_KAANU_REEVS,
    DATA_TRAVEL_TO_SKYFIRE_DECK,
    DATA_BLACKHORN_ACHIEVEMENT_FAILED,   // spell script -> instance: unsoaked Barrage hit the ship

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

// Hagara LFR stats templates (70% of 25N, user-approved)
constexpr uint32 NPC_HAGARA_LFR_STATS                = 58242; // ~72.1M
constexpr uint32 NPC_HAGARA_CRYSTAL_LFR_STATS        = 58243; // ~814k
constexpr uint32 NPC_HAGARA_ELEMENTAL_LFR_STATS      = 58244; // ~2.4M

constexpr uint32 NPC_ULTRAXION_LFR_STATS             = 58245; // ~129.0M (70% of 25N, user-approved)

// Warmaster Blackhorn LFR stats templates (70% of 25N, user-approved).
// 58250/58251 are live Bound Lightning Elemental rows - skipped.
constexpr uint32 NPC_BLACKHORN_LFR_STATS             = 58246; // ~36.07M
constexpr uint32 NPC_GORIONA_LFR_STATS               = 58247; // ~1.28M
constexpr uint32 NPC_ASSAULT_DRAKE_LFR_STATS         = 58248; // ~165.9k
constexpr uint32 NPC_TWILIGHT_ELITE_LFR_STATS        = 58249; // ~506.8k
constexpr uint32 NPC_SKYFIRE_LFR_STATS               = 58252; // ~10.5M

// Ultraxion: the Twilight Realm phase id. Twilight Shift (106368) carries the
// legacy 4.3.4 phasemask 16, remapped to phase id 16 in LoadSpellInfoCorrections.
constexpr uint32 PHASE_TWILIGHT_REALM = 16;

// Warmaster Blackhorn: shared between the boss script and the instance script.
// The fight plays out on a second Skyfire far from Wyrmrest (the "flight
// arena"); the parked staging ship sits beside the Wyrmrest summit.
Position const SkyfireDeckCenterPos = { 13444.9f, -12133.3f, 151.21f,  3.1147f };
Position const SkyfireStagingPos    = { -1699.94f, -2364.35f, 339.85f, 1.5533f };

enum DSCreatures
{
    // Bosses
    BOSS_MORCHOK                                = 55265,
    BOSS_WARLORD_ZONOZZ                         = 55308,
    BOSS_YORSAHJ                                = 55312,
    BOSS_HAGARA                                 = 55689,
    BOSS_ULTRAXION                              = 55294,
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

    /*Hagara the Stormbinder*/
    NPC_ICE_TOMB                                = 55695,
    NPC_ICE_WAVE                                = 56104,
    NPC_ICE_LANCE                               = 56108,
    NPC_FROZEN_BINDING_CRYSTAL                  = 56136,
    NPC_CRYSTAL_CONDUCTOR                       = 56165,
    NPC_BOUND_LIGHTNING_ELEMENTAL               = 56700,
    NPC_COLLAPSING_ICICLE                       = 57867,
    NPC_TWILIGHT_FROST_EVOKER                   = 57807, // 25-man: 57808
    NPC_STORMBORN_MYRMIDON                      = 57817, // 25-man: 57818
    NPC_STORMBINDER_ADEPT                       = 57823, // 25-man: 57824
    NPC_CORRUPTED_FRAGMENT                      = 57819, // 25-man: 57820
    NPC_HAGARA_TWILIGHT_PORTAL                  = 57809,
    NPC_TRAVEL_TO_EYE_OF_ETERNITY               = 57377,
    NPC_TRAVEL_TO_WYRMREST_BASE                 = 57882,
    NPC_TRAVEL_TO_WYRMREST_SUMMIT               = 57379,

    /*Ultraxion*/
    NPC_ULTRAXION_COSMETIC                      = 56259, // normal-realm copy seen while under Heroic Will
    NPC_ULTRAXION_GAUNTLET                      = 56305, // pre-event controller
    NPC_THRALL_ULTRAXION                        = 56667,
    NPC_ALEXSTRASZA_ULTRAXION                   = 56630,
    NPC_YSERA_ULTRAXION                         = 56665,
    NPC_KALECGOS_ULTRAXION                      = 56664,
    NPC_NOZDORMU_ULTRAXION                      = 56666,
    NPC_THE_DRAGON_SOUL                         = 56668,
    NPC_DEATHWING_ULTRAXION                     = 55971, // perched RP speaker for the gauntlet
    NPC_TWILIGHT_ASSAULTER_N                    = 56249, // gauntlet drakes (S/E/W/N flights)
    NPC_TWILIGHT_ASSAULTER_S                    = 56250,
    NPC_TWILIGHT_ASSAULTER_E                    = 56251,
    NPC_TWILIGHT_ASSAULTER_W                    = 56252,

    /*Warmaster Blackhorn*/
    BOSS_WARMASTER_BLACKHORN                    = 56427,
    NPC_GORIONA                                 = 56781,
    NPC_TWILIGHT_ASSAULT_DRAKE_S                = 56587, // starboard side, VehicleId 1907, carries Slayer
    NPC_TWILIGHT_ASSAULT_DRAKE_P                = 56855, // port side, VehicleId 1908, carries Dreadblade
    NPC_TWILIGHT_ELITE_DREADBLADE               = 56854,
    NPC_TWILIGHT_ELITE_SLAYER                   = 56848,
    NPC_TWILIGHT_INFILTRATOR                    = 56922,
    NPC_TWILIGHT_SAPPER                         = 56923,
    NPC_DYNAMITE_BUNDLE                         = 57470, // sapper backpack visual (vehicle accessory)
    NPC_THE_SKYFIRE                             = 56598, // ship structural-integrity proxy
    NPC_GUNSHIP_PURSUIT_CONTROLLER              = 56599,
    NPC_SKYFIRE_HARPOON_GUN                     = 56681,
    NPC_SKYFIRE_CANNON                          = 57260,
    NPC_SKYFIRE_COMMANDO                        = 57264, // gun/cannon crew (vehicle accessory)
    NPC_SKYFIRE_DECKHAND                        = 57265,
    NPC_ONSLAUGHT_TARGET                        = 57238, // Twilight Onslaught ground marker
    NPC_TWILIGHT_FLAMES_PATCH                   = 57268, // summoned natively by 108051 EFFECT_1
    NPC_DECK_FIRE                               = 57920, // heroic deck fires
    NPC_SKY_CAPTAIN_SWAYZE                      = 55870,
    NPC_KAANU_REEVS                             = 55891,
    NPC_TRAVEL_TO_SKYFIRE_DECK                  = 57378,

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
    GO_THE_FOCUSING_IRIS             = 210132,
    GO_HAGARA_ICE_BLOCK              = 201722,
    GO_GIFT_OF_LIFE                  = 209873,
    GO_ESSENCE_OF_DREAMS             = 209874,
    GO_SOURCE_OF_MAGIC               = 209875,
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
