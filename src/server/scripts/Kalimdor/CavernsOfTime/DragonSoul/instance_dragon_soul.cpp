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

#include "AreaBoundary.h"
#include "Creature.h"
#include "Group.h"
#include "InstanceScript.h"
#include "LFGMgr.h"
#include "Map.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "dragon_soul.h"

namespace DragonSoul
{
ObjectData const creatureData[] =
{
    { BOSS_MORCHOK,                         DATA_MORCHOK                            },
    { NPC_KOHCROM,                          DATA_KOHCROM                            },
    { BOSS_WARLORD_ZONOZZ,                  DATA_WARLORD_ZONOZZ                     },
    { BOSS_YORSAHJ,                         DATA_YORSAHJ_THE_UNSLEEPING             },
    { BOSS_HAGARA,                          DATA_HAGARA_THE_STORMBINDER             },
    { NPC_TRAVEL_TO_EYE_OF_ETERNITY,        DATA_TRAVEL_TO_EYE_OF_ETERNITY          },
    { BOSS_ULTRAXION,                       DATA_ULTRAXION                          },
    { NPC_ULTRAXION_GAUNTLET,               DATA_ULTRAXION_GAUNTLET_CONTROLLER      },
    { NPC_TRAVEL_TO_WYRMREST_SUMMIT,        DATA_TRAVEL_TO_WYRMREST_SUMMIT          },
    { NPC_THRALL_ULTRAXION,                 DATA_THRALL_ULTRAXION                   },
    { NPC_ALEXSTRASZA_ULTRAXION,            DATA_ALEXSTRASZA_ULTRAXION              },
    { NPC_YSERA_ULTRAXION,                  DATA_YSERA_ULTRAXION                    },
    { NPC_KALECGOS_ULTRAXION,               DATA_KALECGOS_ULTRAXION                 },
    { NPC_NOZDORMU_ULTRAXION,               DATA_NOZDORMU_ULTRAXION                 },
    { NPC_DEATHWING_ULTRAXION,              DATA_DEATHWING_ULTRAXION                },
    { BOSS_MADNESS_OF_DEATHWING,            DATA_MADNESS_OF_DEATHWING               },
    { NPC_DEATHWING_MADNESS_OF_DEATHWING,   DATA_DEATHWING_MADNESS_OF_DEATHWING     },
    { NPC_THRALL_MADNESS_OF_DEATHWING,      DATA_THRALL_MADNESS_OF_DEATHWING        },
    { NPC_YSERA_MADNESS_OF_DEATHWING,       DATA_YSERA_MADNESS_OF_DEATHWING         },
    { NPC_ALEXSTRASZA_MADNESS_OF_DEATHWING, DATA_ALEXSTRASZA_MADNESS_OF_DEATHWING   },
    { NPC_NOZDORMU_MADNESS_OF_DEATHWING,    DATA_NOZDORMU_MADNESS_OF_DEATHWING      },
    { NPC_KALECGOS_MADNESS_OF_DEATHWING,    DATA_KALECGOS_MADNESS_OF_DEATHWING      },
    { NPC_TAIL_TENTACLE,                    DATA_TAIL_TENTACLE_MADNESS_OF_DEATHWING },
    { 0,                                    0                                       } // END
};

ObjectData const gameobjectData[] =
{
    { 0, 0 } // END
};

DoorData const doorData[] =
{
    { 0, 0, DOOR_TYPE_ROOM } // END
};

BossBoundaryData const boundaries =
{
    { DATA_MORCHOK,                 new CircleBoundary(Position(-1981.03, -2409.30), 95.0) },
    { DATA_WARLORD_ZONOZZ,          new CircleBoundary(Position(-1765.00, -1915.00), 65.0) },
    { DATA_YORSAHJ_THE_UNSLEEPING,  new CircleBoundary(Position(-1765.65, -3034.35), 115.0) },
    { DATA_HAGARA_THE_STORMBINDER,  new CircleBoundary(Position(13587.29, 13611.83), 60.0) },
    { DATA_ULTRAXION,               new CircleBoundary(Position(-1786.00, -2393.00), 100.0) }
};

enum DSAchievementCriteria
{
    CRITERIA_DONT_STAND_SO_CLOSE_TO_ME = 18607,

    // Ping Pong Champion: the Void of the Unmaking bounced 10+ times before
    // detonating on the boss, on any single sphere during the kill attempt
    CRITERIA_PING_PONG_CHAMPION        = 18494,

    // Taste the Rainbow!
    CRITERIA_RAINBOW_BLACK_YELLOW      = 18495,
    CRITERIA_RAINBOW_RED_GREEN         = 18496,
    CRITERIA_RAINBOW_BLACK_BLUE        = 18497,
    CRITERIA_RAINBOW_PURPLE_YELLOW     = 18498,

    // Holding Hands: all conductors charged by one unbroken conduit chain
    CRITERIA_HOLDING_HANDS             = 18608,

    // Minutes to Midnight: no raid member hit by Hour of Twilight more than once
    CRITERIA_MINUTES_TO_MIDNIGHT       = 18391
};

// Boss creature entry per encounter data index, used for Raid Finder loot lockouts
uint32 GetBossEntryForData(uint32 bossId)
{
    switch (bossId)
    {
        case DATA_MORCHOK:
            return BOSS_MORCHOK;
        case DATA_WARLORD_ZONOZZ:
            return BOSS_WARLORD_ZONOZZ;
        case DATA_YORSAHJ_THE_UNSLEEPING:
            return BOSS_YORSAHJ;
        case DATA_HAGARA_THE_STORMBINDER:
            return BOSS_HAGARA;
        case DATA_ULTRAXION:
            return BOSS_ULTRAXION;
        default:
            return 0;
    }
}

// Infinite-duration encounter auras that must never leak out of the fight
// (saved player auras survive relogs; see OnPlayerEnter)
uint32 const UltraxionPersistentAuras[] =
{
    106368, // Twilight Shift
    105554, // Heroic Will (extra action button grant)
    106108, // Heroic Will (button aura)
    105984, // Timeloop
    105896, // Gift of Life
    109340, // Gift of Life (heroic)
    105900, // Essence of Dreams
    109342, // Essence of Dreams (heroic)
    105903, // Source of Magic
    109346, // Source of Magic (heroic)
    106498  // Looming Darkness
};

class instance_dragon_soul : public InstanceMapScript
{
public:
    instance_dragon_soul() : InstanceMapScript(DSScriptName, 967) { }

    struct instance_dragon_soul_InstanceMapScript : public InstanceScript
    {
        instance_dragon_soul_InstanceMapScript(InstanceMap* map) : InstanceScript(map)
        {
            SetHeaders(DataHeader);
            SetBossNumber(EncounterCount);
            LoadBossBoundaries(boundaries);
            LoadDoorData(doorData);
            LoadObjectData(creatureData, gameobjectData);
        }

        bool SetBossState(uint32 type, EncounterState state) override
        {
            if (!InstanceScript::SetBossState(type, state))
                return false;

            if (type == DATA_MORCHOK && state == IN_PROGRESS)
                _morchokAchievementFailed = false;

            if (type == DATA_WARLORD_ZONOZZ && state == IN_PROGRESS)
                _zonozzPingPong = false;

            if (type == DATA_YORSAHJ_THE_UNSLEEPING && state == IN_PROGRESS)
                _yorsahjRainbowMask = 0;

            if (type == DATA_HAGARA_THE_STORMBINDER && state == IN_PROGRESS)
                _hagaraHoldingHands = false;

            if (type == DATA_ULTRAXION && state == IN_PROGRESS)
                _ultraxionAchievementFailed = false;

            // The Eye of Eternity teleporter activates once both preceding
            // bosses are down
            if ((type == DATA_WARLORD_ZONOZZ || type == DATA_YORSAHJ_THE_UNSLEEPING) && state == DONE)
                if (Creature* teleporter = GetCreature(DATA_TRAVEL_TO_EYE_OF_ETERNITY))
                    teleporter->SetVisible(IsEyeTeleporterActive());

            // The Wyrmrest Summit teleporter activates once Hagara is down
            if (type == DATA_HAGARA_THE_STORMBINDER && state == DONE)
                if (Creature* teleporter = GetCreature(DATA_TRAVEL_TO_WYRMREST_SUMMIT))
                    teleporter->SetVisible(true);

            if (state == DONE && IsLFR())
                if (uint32 bossEntry = GetBossEntryForData(type))
                    RegisterLFRLootLockouts(type, bossEntry);

            return true;
        }

        void SetData(uint32 type, uint32 data) override
        {
            if (type == DATA_MORCHOK_ACHIEVEMENT_FAILED)
                _morchokAchievementFailed = true;
            else if (type == DATA_ZONOZZ_PING_PONG)
                _zonozzPingPong = true;
            else if (type == DATA_YORSAHJ_TASTE_THE_RAINBOW)
                _yorsahjRainbowMask |= data;
            else if (type == DATA_HAGARA_HOLDING_HANDS)
                _hagaraHoldingHands = true;
            else if (type == DATA_HAGARA_INTRO_DONE)
            {
                _hagaraIntroDone = data != 0;
                SaveToDB();
            }
            else if (type == DATA_ULTRAXION_ACHIEVEMENT_FAILED)
                _ultraxionAchievementFailed = true;
            else if (type == DATA_ULTRAXION_GAUNTLET_DONE)
            {
                _ultraxionGauntletDone = data != 0;
                SaveToDB();
            }
        }

        uint32 GetData(uint32 type) const override
        {
            if (type == DATA_IS_LFR)
                return IsLFR() ? 1 : 0;
            if (type == DATA_HAGARA_INTRO_DONE)
                return _hagaraIntroDone ? 1 : 0;
            if (type == DATA_ULTRAXION_GAUNTLET_DONE)
                return _ultraxionGauntletDone ? 1 : 0;
            return 0;
        }

        bool IsEyeTeleporterActive() const
        {
            return GetBossState(DATA_WARLORD_ZONOZZ) == DONE && GetBossState(DATA_YORSAHJ_THE_UNSLEEPING) == DONE;
        }

        bool CheckRequiredBosses(uint32 bossId, Player const* /*player*/ = nullptr) const override
        {
            // Raid Finder wing 1 ("The Siege of Wyrmrest Temple") covers
            // Morchok through Hagara; wing 2 content is not queueable yet
            if (IsLFR())
                return bossId <= DATA_HAGARA_THE_STORMBINDER;

            return true;
        }

        bool CheckAchievementCriteriaMeet(uint32 criteriaId, Player const* /*source*/, Unit const* /*target*/, uint32 /*miscvalue1*/) override
        {
            switch (criteriaId)
            {
                case CRITERIA_DONT_STAND_SO_CLOSE_TO_ME:
                    return !IsLFR() && !_morchokAchievementFailed;
                case CRITERIA_PING_PONG_CHAMPION:
                    return !IsLFR() && _zonozzPingPong;
                // Glory criteria are not earnable through the Raid Finder
                case CRITERIA_RAINBOW_BLACK_YELLOW:
                    return !IsLFR() && (_yorsahjRainbowMask & RAINBOW_BIT_BLACK_YELLOW) != 0;
                case CRITERIA_RAINBOW_RED_GREEN:
                    return !IsLFR() && (_yorsahjRainbowMask & RAINBOW_BIT_RED_GREEN) != 0;
                case CRITERIA_RAINBOW_BLACK_BLUE:
                    return !IsLFR() && (_yorsahjRainbowMask & RAINBOW_BIT_BLACK_BLUE) != 0;
                case CRITERIA_RAINBOW_PURPLE_YELLOW:
                    return !IsLFR() && (_yorsahjRainbowMask & RAINBOW_BIT_PURPLE_YELLOW) != 0;
                case CRITERIA_HOLDING_HANDS:
                    return !IsLFR() && _hagaraHoldingHands;
                // "on Normal or Heroic Difficulty" per the achievement text
                case CRITERIA_MINUTES_TO_MIDNIGHT:
                    return !IsLFR() && !_ultraxionAchievementFailed;
                default:
                    break;
            }

            return false;
        }

        // Raid Finder: players who already looted this boss this week are
        // excluded from item rolls. Eligibility is snapshotted at the kill,
        // before the fresh lockouts are recorded.
        void RegisterLFRLootLockouts(uint32 bossId, uint32 bossEntry)
        {
            GuidSet& eligible = _lfrLootEligible[bossId];
            eligible.clear();

            for (MapReference const& ref : instance->GetPlayers())
            {
                Player* player = ref.GetSource();
                if (!player || player->IsGameMaster())
                    continue;

                if (!player->HasLFRLootLockout(bossEntry))
                    eligible.insert(player->GetGUID());

                player->AddLFRLootLockout(bossEntry);
            }
        }

        bool IsEligibleForLootRoll(Player const* player, WorldObject const* lootSource) const override
        {
            if (!IsLFR())
                return true;

            Creature const* creature = lootSource->ToCreature();
            if (!creature)
                return true;

            uint32 bossId = 0;
            for (; bossId < EncounterCount; ++bossId)
                if (GetBossEntryForData(bossId) == creature->GetEntry())
                    break;

            if (bossId >= EncounterCount)
                return true;

            auto itr = _lfrLootEligible.find(bossId);
            if (itr == _lfrLootEligible.end())
                return !player->HasLFRLootLockout(creature->GetEntry());

            return itr->second.find(player->GetGUID()) != itr->second.end();
        }

        void WriteSaveDataMore(std::ostringstream& data) override
        {
            data << uint32(IsLFR() ? 1 : 0) << ' ' << uint32(_hagaraIntroDone ? 1 : 0) << ' ' << uint32(_ultraxionGauntletDone ? 1 : 0);
        }

        void ReadSaveDataMore(std::istringstream& data) override
        {
            uint32 isLfr = 0;
            data >> isLfr;
            SetLFR(isLfr != 0);

            // absent in old save strings - stream failure leaves the default
            uint32 hagaraIntroDone = 0;
            data >> hagaraIntroDone;
            _hagaraIntroDone = hagaraIntroDone != 0;

            uint32 ultraxionGauntletDone = 0;
            data >> ultraxionGauntletDone;
            _ultraxionGauntletDone = ultraxionGauntletDone != 0;
        }

        void OnCreatureCreate(Creature* creature) override
        {
            InstanceScript::OnCreatureCreate(creature);

            // Raid Finder bosses use the LFR loot rows (LootMode 2). Must be
            // set at create time - loot is filled before JustDied fires.
            if (IsLFR() && (creature->GetEntry() == BOSS_MORCHOK || creature->GetEntry() == BOSS_WARLORD_ZONOZZ || creature->GetEntry() == BOSS_YORSAHJ || creature->GetEntry() == BOSS_HAGARA || creature->GetEntry() == BOSS_ULTRAXION))
                creature->SetLootMode(LOOT_MODE_HARD_MODE_1);

            switch (creature->GetEntry())
            {
                case NPC_TRAVEL_TO_EYE_OF_ETERNITY:
                    creature->SetVisible(IsEyeTeleporterActive());
                    break;
                case NPC_TRAVEL_TO_WYRMREST_SUMMIT:
                    creature->SetVisible(GetBossState(DATA_HAGARA_THE_STORMBINDER) == DONE);
                    break;
                case NPC_YSERA_MADNESS_OF_DEATHWING:
                case NPC_ALEXSTRASZA_MADNESS_OF_DEATHWING:
                case NPC_NOZDORMU_MADNESS_OF_DEATHWING:
                case NPC_KALECGOS_MADNESS_OF_DEATHWING:
                    creature->setActive(true); // Ugly as fuck but the boss area is just too big...
                    break;
                default:
                    break;
            }
        }

        void OnPlayerEnter(Player* player) override
        {
            if (GetBossState(DATA_MADNESS_OF_DEATHWING) == DONE)
                player->CastSpell(player, SPELL_CALM_MAELSTROM_SKYBOX);

            // Ultraxion's infinite encounter auras are saved with the player;
            // strip them from anyone entering outside an active attempt
            if (GetBossState(DATA_ULTRAXION) != IN_PROGRESS)
                for (uint32 spellId : UltraxionPersistentAuras)
                    player->RemoveAurasDueToSpell(spellId);

            // Raid Finder: groups formed by the Raid Finder queue flag the
            // instance automatically on first entry (25 normal mode only)
            if (!IsLFR() && instance->GetDifficulty() == RAID_DIFFICULTY_25MAN_NORMAL)
            {
                if (Group const* group = player->GetGroup())
                {
                    if (group->isLFRGroup() && sLFGMgr->inLfgDungeonMap(group->GetGUID(), instance->GetId(), instance->GetDifficulty()))
                    {
                        SetLFR(true);
                        SaveToDB();
                    }
                }
            }
        }

    private:
        bool _morchokAchievementFailed = false;
        bool _zonozzPingPong = false;
        uint32 _yorsahjRainbowMask = 0;
        bool _hagaraHoldingHands = false;
        bool _hagaraIntroDone = false;
        bool _ultraxionAchievementFailed = false;
        bool _ultraxionGauntletDone = false;
        std::unordered_map<uint32, GuidSet> _lfrLootEligible;
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_dragon_soul_InstanceMapScript(map);
    }
};
}

void AddSC_instance_dragon_soul()
{
    using namespace DragonSoul;
    new instance_dragon_soul();
}
