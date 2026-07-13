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

#ifndef DEF_WELLOFETERNITY_H
#define DEF_WELLOFETERNITY_H

#include "CreatureAIImpl.h"
#include "Define.h"

class Creature;

namespace WellOfEternity
{
constexpr char const* DataHeader = "WOE";
#define WOEScriptName "instance_well_of_eternity"

    uint32 const EncounterCount = 3;

    enum WOEDataTypes
    {
        // Bosses
        BOSS_PEROTHARN = 0,
        BOSS_QUEEN_AZSHARA = 1,
        BOSS_MANNOROTH_AND_VAROTHEN = 2,

        // Misc
        DATA_WOE_COURTYARD_DOOR01,

        // Courtyard gauntlet progression
        DATA_LEGION_DEMON_FIRST_PULL,   // door-guard Legion Demon killed -> Illidan reveal (persisted)
        DATA_PORTALS_SHUT_DOWN,         // 0..3 Portal Energy Focus crystals destroyed (persisted)
        DATA_GAUNTLET_ILLIDAN,          // GUID accessor for the active escort Illidan

        // Peroth'arn
        DATA_PLAYER_CAUGHT_BY_EYE,      // volatile, voids Lazy Eye for the attempt

        // Mannoroth
        DATA_FEL_DRAIN_TRIGGERED,       // volatile, arms That's Not Canon!

        // GUID accessors (GetCreature via ObjectData)
        DATA_NOZDORMU_ENTRANCE,
        DATA_ILLIDAN_FINALE,
        DATA_TYRANDE,
        DATA_MALFURION,
        DATA_CAPTAIN_VAROTHEN,
        DATA_EMBEDDED_BLADE
    };

    enum WOECreatures
    {
        // Bosses
        NPC_PEROTHARN                   = 55085,
        NPC_QUEEN_AZSHARA               = 54853,
        NPC_MANNOROTH                   = 54969,
        NPC_CAPTAIN_VAROTHEN            = 55419,

        // Entrance
        NPC_NOZDORMU_ENTRANCE           = 55624,
        NPC_ALURMI                      = 57864,

        // Courtyard gauntlet
        NPC_ILLIDAN_GAUNTLET            = 55500, // vehicle; re-summoned once per escort stage
        NPC_SHADOWCLOAK_HELPER          = 55154, // per-player cloak visual stalker
        NPC_SHADOWCLOAK_ILLIDAN_HELPER  = 56389,
        NPC_DISTRACT_DEMON_STALKER      = 58200,
        NPC_LEGION_DEMON_DOOR_GUARD     = 55503, // the lone first-pull felguard
        NPC_LEGION_DEMON_MARCHING       = 54500, // endless palace reinforcements
        NPC_GUARDIAN_DEMON              = 54927,
        NPC_LEGION_PORTAL               = 54513,
        NPC_FEL_CRYSTAL_STALKER         = 55965, // sits on each Portal Energy Focus GO
        NPC_FEL_CRYSTAL                 = 55917,
        NPC_PORTAL_CONNECTOR_1          = 55541,
        NPC_PORTAL_CONNECTOR_2          = 55542,
        NPC_PORTAL_CONNECTOR_3          = 55543,
        NPC_WELL_OF_ETERNITY_STALKER    = 54506,
        NPC_FIRE_WALL_STALKER           = 56096,
        NPC_CORRUPTED_ARCANIST          = 55654,
        NPC_DREADLORD_DEFENDER          = 55656,

        // Peroth'arn encounter
        NPC_EYE_OF_PEROTHARN_1          = 55868,
        NPC_EYE_OF_PEROTHARN_2          = 55879,
        NPC_HUNTING_SUMMON_CIRCLE       = 56182,
        NPC_HUNTING_SUMMON_STALKER      = 56248, // ring center, owns the search raid-emote
        NPC_HUNTING_STALKERS            = 56189,
        NPC_EASY_PREY_STALKER           = 56308,
        NPC_FEL_FLAMES_STALKER          = 57329,
        NPC_FEL_FLAMES                  = 55502, // ground-fire trail (also Mannoroth's firestorm)

        // Palace trash
        NPC_EYE_OF_THE_LEGION           = 54747,
        NPC_ENCHANTED_HIGHMISTRESS_1    = 54589,
        NPC_ENCHANTED_HIGHMISTRESS_2    = 56579,
        NPC_ENCHANTED_REFLECTION        = 54695,
        NPC_ETERNAL_CHAMPION            = 54612,
        NPC_ROYAL_HANDMAIDEN            = 54645,

        // Queen Azshara encounter
        NPC_ENCHANTED_MAGUS_FIRE        = 54882,
        NPC_ENCHANTED_MAGUS_FROST       = 54883,
        NPC_ENCHANTED_MAGUS_ARCANE      = 54884,
        NPC_HAND_OF_THE_QUEEN           = 54728,
        NPC_ARCANE_BOMB_AIR             = 54864, // "Hammer of Divinity" orb pair
        NPC_ARCANE_BOMB_GROUND          = 54865,
        NPC_VAROTHEN_SHADOWBAT_CAMEO    = 57117, // bat-mounted Varo'then, departure RP
        NPC_VAROTHEN_CAMEO              = 57118,
        NPC_BRONZE_DRAKE_VEHICLE        = 57107,

        // Shores / Mannoroth encounter
        NPC_ILLIDAN_FINALE              = 55532,
        NPC_TYRANDE                     = 55524,
        NPC_MALFURION                   = 55570,
        NPC_VORACIOUS_FELHOUND          = 56073,
        NPC_DOOMGUARD_ANNIHILATOR       = 55519,
        NPC_ABYSSAL_DOOMBRINGER         = 55510,
        NPC_ABYSSAL_DOOMBRINGER_FAR     = 56078,
        NPC_HIGHGUARD_ELITE             = 55426,
        NPC_SHADOWBAT_VEHICLE           = 55453,
        NPC_SHADOWBAT_MIRROR            = 55465,
        NPC_SHADOWBAT_AMBIENT           = 57458,
        NPC_DREADLORD_DEBILITATOR       = 55762,
        NPC_DOOMGUARD_DEVASTATOR        = 55739,
        NPC_FELHOUND_WAVE               = 56001,
        NPC_FELGUARD_WAVE               = 56002,
        NPC_INFERNAL                    = 56036,
        NPC_VAROTHENS_MAGICAL_BLADE     = 55837, // lootable sword (spellclick 104818)
        NPC_EMBEDDED_BLADE              = 55838, // Mannoroth vehicle seat 0
        NPC_MANNOROTH_STRIKE_POINT      = 55839, // Mannoroth vehicle seats 1-6
        NPC_PORTAL_TO_TWISTING_NETHER   = 56087,
        NPC_GENERAL_PURPOSE_BUNNY_JMF   = 45979,
        NPC_GP_BUNNY_JMF_FLYING_HUGE    = 54020,
        NPC_LEGION_ARMY_DOOMGUARD       = 55700, // ambient army streaming out of the well

        // Finale RP
        NPC_THE_DRAGON_SOUL             = 55078,
        NPC_NOZDORMU_FINALE             = 56102,
        NPC_CHROMIE                     = 57913,
        NPC_YSERA                       = 55393,
        NPC_ALEXSTRASZA                 = 55394,
        NPC_SORIDORMI                   = 55395,
        NPC_NELTHARION                  = 55400,
        NPC_AN_UNKNOWN_EVIL             = 57201
    };

    enum WOEGameObjectIds
    {
        GO_WOE_COURTYARD_DOOR01     = 210084,
        GO_LARGE_FIREWALL_DOOR      = 210234,
        GO_SMALL_FIREWALL_DOOR      = 210130,
        GO_INVISIBLE_FIREWALL       = 210097, // courtyard -> palace stair gate
        GO_PALACE_DOORS             = 209937,
        GO_PORTAL_ENERGY_FOCUS_1    = 209366, // west
        GO_PORTAL_ENERGY_FOCUS_2    = 209447, // center-east
        GO_PORTAL_ENERGY_FOCUS_3    = 209448, // upper terrace
        GO_TIME_TRANSIT_DEVICE_1    = 209998, // entrance
        GO_TIME_TRANSIT_DEVICE_2    = 209997, // palace floor
        GO_TIME_TRANSIT_DEVICE_3    = 210000, // palace terrace
        GO_TIME_TRANSIT_DEVICE_4    = 209999, // shores of the well
        GO_ROYAL_CACHE              = 210025, // Azshara loot
        GO_MINOR_CACHE_OF_ASPECTS   = 209541  // post-Mannoroth
    };

    enum WOESharedSpells
    {
        SPELL_NIGHT_ELF_ILLUSION            = 108424, // dungeon-wide disguise, dummy -> 108463 transform
        SPELL_NIGHT_ELF_TRANSFORM           = 108463,

        // Illidan's stealth package (gauntlet + Peroth'arn hide phase)
        SPELL_SHADOWCLOAK                   = 103004, // player driver aura (proc + 500ms periodic)
        SPELL_SHADOW_WALK                   = 102994, // the stealth itself
        SPELL_SHADOW_WALK_PULSE             = 103020,
        SPELL_SHADOW_AMBUSHER_STEALTH       = 103420,
        SPELL_SHADOW_AMBUSHER               = 103018, // damage bonus window
        SPELL_SHADOWCLOAK_ILLIDAN           = 105915,

        SPELL_RIDE_VEHICLE_HARDCODED        = 46598,

        // Encounter credits
        SPELL_AZSHARA_EVENT_CREDIT          = 94981,  // serverside killcredit 51314
        SPELL_MANNOROTH_ACHIEVEMENT_SPELL   = 105576  // encounter + achievement marker on players
    };

    enum WOEAchievementCriteria
    {
        CRITERIA_ID_LAZY_EYE        = 18618, // achievement 6127
        CRITERIA_ID_THATS_NOT_CANON = 18363  // achievement 6070
    };

    // Actions crossing translation units. File-local actions must not collide (start at 10).
    enum WOESharedActions
    {
        ACTION_PEROTHARN_INTRO          = 1, // gauntlet complete -> Peroth'arn entrance RP
        ACTION_ILLIDAN_DRAIN_ESSENCE    = 2, // Peroth'arn -> escort Illidan: break the channel, re-stealth party
        ACTION_ILLIDAN_HIDE_ENDED       = 3, // Peroth'arn -> escort Illidan: hide phase resolved, recover
        ACTION_ILLIDAN_PEROTHARN_DEAD   = 4, // Peroth'arn -> escort Illidan: outro + departure
        ACTION_PEROTHARN_LEDGE_RP       = 5  // first-pull demon died -> Peroth'arn ledge dialogue
    };

    template<class AI>
    AI* GetWellOfEternityAI(Creature* creature)
    {
        return GetInstanceAI<AI>(creature, WOEScriptName);
    }

#define RegisterWellOfEternityCreatureAI(ai_name) RegisterCreatureAIWithFactory(ai_name, GetWellOfEternityAI)
}

#endif // DEF_WELLOFETERNITY_H
