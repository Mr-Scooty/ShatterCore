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
    { DATA_MORCHOK, new CircleBoundary(Position(-1981.03, -2409.30), 95.0) }
};

enum DSAchievementCriteria
{
    CRITERIA_DONT_STAND_SO_CLOSE_TO_ME = 18607
};

// Boss creature entry per encounter data index, used for Raid Finder loot lockouts
uint32 GetBossEntryForData(uint32 bossId)
{
    switch (bossId)
    {
        case DATA_MORCHOK:
            return BOSS_MORCHOK;
        default:
            return 0;
    }
}

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

            if (state == DONE && IsLFR())
                if (uint32 bossEntry = GetBossEntryForData(type))
                    RegisterLFRLootLockouts(type, bossEntry);

            return true;
        }

        void SetData(uint32 type, uint32 /*data*/) override
        {
            if (type == DATA_MORCHOK_ACHIEVEMENT_FAILED)
                _morchokAchievementFailed = true;
        }

        uint32 GetData(uint32 type) const override
        {
            if (type == DATA_IS_LFR)
                return IsLFR() ? 1 : 0;
            return 0;
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
            data << uint32(IsLFR() ? 1 : 0);
        }

        void ReadSaveDataMore(std::istringstream& data) override
        {
            uint32 isLfr = 0;
            data >> isLfr;
            SetLFR(isLfr != 0);
        }

        void OnCreatureCreate(Creature* creature) override
        {
            InstanceScript::OnCreatureCreate(creature);

            // Raid Finder bosses use the LFR loot rows (LootMode 2). Must be
            // set at create time - loot is filled before JustDied fires.
            if (IsLFR() && creature->GetEntry() == BOSS_MORCHOK)
                creature->SetLootMode(LOOT_MODE_HARD_MODE_1);

            switch (creature->GetEntry())
            {
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
