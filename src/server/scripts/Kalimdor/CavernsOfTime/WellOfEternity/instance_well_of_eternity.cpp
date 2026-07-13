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
#include "Creature.h"
#include "CreatureAI.h"
#include "EventMap.h"
#include "GameObject.h"
#include "InstanceScript.h"
#include "Map.h"
#include "Player.h"
#include "TemporarySummon.h"
#include "well_of_eternity.h"

#include <array>
#include <limits>

namespace WellOfEternity
{
ObjectData const creatureData[] =
{
    { NPC_PEROTHARN,        BOSS_PEROTHARN              },
    { NPC_QUEEN_AZSHARA,    BOSS_QUEEN_AZSHARA          },
    { NPC_MANNOROTH,        BOSS_MANNOROTH_AND_VAROTHEN },
    { NPC_CAPTAIN_VAROTHEN, DATA_CAPTAIN_VAROTHEN       },
    { NPC_ILLIDAN_GAUNTLET, DATA_GAUNTLET_ILLIDAN       },
    { NPC_ILLIDAN_FINALE,   DATA_ILLIDAN_FINALE         },
    { NPC_TYRANDE,          DATA_TYRANDE                },
    { NPC_MALFURION,        DATA_MALFURION              },
    { NPC_NOZDORMU_ENTRANCE,DATA_NOZDORMU_ENTRANCE      },
    { NPC_EMBEDDED_BLADE,   DATA_EMBEDDED_BLADE         },
    { 0,                    0                           } // END
};

ObjectData const gameobjectData[] =
{
    { GO_WOE_COURTYARD_DOOR01,  DATA_WOE_COURTYARD_DOOR01   },
    { 0,                        0                           } // END
};

DoorData const doorData[] =
{
    { GO_LARGE_FIREWALL_DOOR,   BOSS_PEROTHARN, DOOR_TYPE_ROOM    },
    { GO_SMALL_FIREWALL_DOOR,   BOSS_PEROTHARN, DOOR_TYPE_ROOM    },
    { GO_INVISIBLE_FIREWALL,    BOSS_PEROTHARN, DOOR_TYPE_PASSAGE },
    { 0,                        0,              DOOR_TYPE_ROOM    } // END
};

enum WOEInstanceEvents
{
    EVENT_MARCHING_DEMONS = 1,
    EVENT_LEGION_ARMY,
    EVENT_DRAKE_CHECK,
    EVENT_PEROTHARN_INTRO_ON_LOAD
};

enum WOEInstanceMisc
{
    SPAWN_GROUP_ID_ROYAL_CACHE      = 470,
    SPAWN_GROUP_ID_MINOR_CACHE      = 471
};

Position const IllidanGauntletPositions[4] =
{
    { 3173.675f, -4875.510f, 194.440f, 5.3582f }, // intro ledge
    { 3294.200f, -4981.970f, 181.160f, 0.8727f }, // portal 1 (west)
    { 3444.980f, -4886.340f, 181.160f, 4.0143f }, // portal 2 (center-east)
    { 3471.120f, -4839.830f, 194.215f, 2.0071f }  // portal 3 (upper terrace)
};

Position const MarchingDemonSpawnPositions[2] =
{
    { 3454.300f, -5084.906f, 213.680f, 2.1468f },
    { 3460.250f, -5080.970f, 213.648f, 2.1468f }
};

Position const BronzeDrakePerchPositions[5] =
{
    { 3429.800f, -5269.486f, 232.845f, 0.9948f },
    { 3436.187f, -5278.691f, 235.671f, 1.1868f },
    { 3446.381f, -5274.656f, 232.845f, 1.3265f },
    { 3454.650f, -5282.660f, 236.201f, 1.5533f },
    { 3465.402f, -5277.052f, 232.845f, 1.7104f }
};

Position const LegionArmySpawnPosition = { 3421.490f, -5474.340f, 17.083f, 0.0f };
Position const NetherPortalSpawnPosition = { 3424.074f, -5459.351f, 19.229f, 6.2483f };

class instance_well_of_eternity : public InstanceMapScript
{
public:
    instance_well_of_eternity() : InstanceMapScript(WOEScriptName, 939) { }

    struct instance_well_of_eternity_InstanceMapScript : public InstanceScript
    {
        instance_well_of_eternity_InstanceMapScript(InstanceMap* map) : InstanceScript(map),
            _firstPullDone(0), _portalsShutDown(0), _playerCaughtByEye(false), _felDrainTriggered(false)
        {
            SetHeaders(DataHeader);
            SetBossNumber(EncounterCount);
            LoadDoorData(doorData);
            LoadObjectData(creatureData, gameobjectData);
        }

        void OnPlayerEnter(Player* player) override
        {
            // Everyone inside the timeway is disguised as a night elf.
            if (!player->HasAura(SPELL_NIGHT_ELF_ILLUSION))
                player->CastSpell(player, SPELL_NIGHT_ELF_ILLUSION, true);
            if (!player->HasAura(SPELL_NIGHT_ELF_TRANSFORM))
                player->CastSpell(player, SPELL_NIGHT_ELF_TRANSFORM, true);
        }

        void Create() override
        {
            InstanceScript::Create();
            SummonGauntletIllidanIfNeeded();
        }

        void Load(char const* data) override
        {
            InstanceScript::Load(data);

            SummonGauntletIllidanIfNeeded();

            if (GetBossState(BOSS_PEROTHARN) != DONE)
            {
                // Courtyard still contested: resume the demon reinforcements
                // and, with all portals down, re-run the boss entrance.
                if (_firstPullDone && _portalsShutDown < 3)
                    _events.ScheduleEvent(EVENT_MARCHING_DEMONS, 10s);
                else if (_portalsShutDown >= 3)
                    _events.ScheduleEvent(EVENT_PEROTHARN_INTRO_ON_LOAD, 5s);
            }
            else
                StartWellSiege();

            if (GetBossState(BOSS_QUEEN_AZSHARA) == DONE)
            {
                if (GetBossState(BOSS_MANNOROTH_AND_VAROTHEN) != DONE)
                    instance->SpawnGroupSpawn(SPAWN_GROUP_ID_ROYAL_CACHE);
                _events.ScheduleEvent(EVENT_DRAKE_CHECK, 10s);
            }

            if (GetBossState(BOSS_MANNOROTH_AND_VAROTHEN) == DONE)
                instance->SpawnGroupSpawn(SPAWN_GROUP_ID_MINOR_CACHE);
        }

        void WriteSaveDataMore(std::ostringstream& data) override
        {
            data << ' ' << _firstPullDone << ' ' << _portalsShutDown;
        }

        void ReadSaveDataMore(std::istringstream& data) override
        {
            data >> _firstPullDone;
            data >> _portalsShutDown;
            if (_portalsShutDown > 3)
                _portalsShutDown = 3;
        }

        void SetData(uint32 type, uint32 value) override
        {
            switch (type)
            {
                case DATA_LEGION_DEMON_FIRST_PULL:
                    if (_firstPullDone)
                        break;
                    _firstPullDone = 1;
                    SummonGauntletIllidanIfNeeded();
                    if (Creature* perotharn = GetCreature(BOSS_PEROTHARN))
                        if (perotharn->IsAIEnabled())
                            perotharn->AI()->DoAction(ACTION_PEROTHARN_LEDGE_RP);
                    _events.ScheduleEvent(EVENT_MARCHING_DEMONS, 20s);
                    SaveToDB();
                    break;
                case DATA_PORTALS_SHUT_DOWN:
                    if (_portalsShutDown >= 3)
                        break;
                    ++_portalsShutDown;
                    if (_portalsShutDown == 3)
                    {
                        _events.CancelEvent(EVENT_MARCHING_DEMONS);
                        if (Creature* perotharn = GetCreature(BOSS_PEROTHARN))
                            if (perotharn->IsAIEnabled())
                                perotharn->AI()->DoAction(ACTION_PEROTHARN_INTRO);
                    }
                    SaveToDB();
                    break;
                case DATA_PLAYER_CAUGHT_BY_EYE:
                    _playerCaughtByEye = value != 0;
                    break;
                case DATA_FEL_DRAIN_TRIGGERED:
                    _felDrainTriggered = value != 0;
                    break;
                default:
                    break;
            }
        }

        uint32 GetData(uint32 type) const override
        {
            switch (type)
            {
                case DATA_LEGION_DEMON_FIRST_PULL:
                    return _firstPullDone;
                case DATA_PORTALS_SHUT_DOWN:
                    return _portalsShutDown;
                case DATA_PLAYER_CAUGHT_BY_EYE:
                    return _playerCaughtByEye ? 1 : 0;
                case DATA_FEL_DRAIN_TRIGGERED:
                    return _felDrainTriggered ? 1 : 0;
                default:
                    return 0;
            }
        }

        bool SetBossState(uint32 type, EncounterState state) override
        {
            if (!InstanceScript::SetBossState(type, state))
                return false;

            switch (type)
            {
                case BOSS_PEROTHARN:
                    if (state == NOT_STARTED)
                        _playerCaughtByEye = false;
                    else if (state == DONE)
                        StartWellSiege();
                    break;
                case BOSS_QUEEN_AZSHARA:
                    if (state == DONE)
                    {
                        instance->SpawnGroupSpawn(SPAWN_GROUP_ID_ROYAL_CACHE);
                        _events.ScheduleEvent(EVENT_DRAKE_CHECK, 8s);

                        // The Vainglorious: she survives, so the quest kill credit
                        // (RequiredNpcOrGo 54853) cannot come from a corpse.
                        for (auto const& ref : instance->GetPlayers())
                            if (Player* player = ref.GetSource())
                                player->KilledMonsterCredit(NPC_QUEEN_AZSHARA);
                    }
                    break;
                case BOSS_MANNOROTH_AND_VAROTHEN:
                    if (state == NOT_STARTED)
                        _felDrainTriggered = false;
                    else if (state == DONE)
                    {
                        instance->SpawnGroupSpawn(SPAWN_GROUP_ID_MINOR_CACHE);
                        _events.CancelEvent(EVENT_LEGION_ARMY);
                    }
                    break;
                default:
                    break;
            }
            return true;
        }

        bool CheckAchievementCriteriaMeet(uint32 criteriaId, Player const* /*source*/, Unit const* /*target*/, uint32 /*miscValue1*/) override
        {
            switch (criteriaId)
            {
                case CRITERIA_ID_LAZY_EYE:
                    return !_playerCaughtByEye;
                case CRITERIA_ID_THATS_NOT_CANON:
                    return _felDrainTriggered;
                default:
                    break;
            }
            return false;
        }

        void Update(uint32 diff) override
        {
            _events.Update(diff);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_MARCHING_DEMONS:
                        if (_portalsShutDown < 3 && GetBossState(BOSS_PEROTHARN) != DONE)
                        {
                            for (Position const& pos : MarchingDemonSpawnPositions)
                                instance->SummonCreature(NPC_LEGION_DEMON_MARCHING, pos);
                            _events.Repeat(16s, 20s);
                        }
                        break;
                    case EVENT_LEGION_ARMY:
                        if (GetBossState(BOSS_MANNOROTH_AND_VAROTHEN) != DONE && instance->HavePlayers())
                            for (uint8 i = 0; i < 4; ++i)
                                instance->SummonCreature(NPC_LEGION_ARMY_DOOMGUARD, LegionArmySpawnPosition);
                        _events.Repeat(15s);
                        break;
                    case EVENT_DRAKE_CHECK:
                        for (uint8 i = 0; i < 5; ++i)
                        {
                            if (!instance->GetCreature(_drakeGuids[i]))
                                if (Creature* drake = instance->SummonCreature(NPC_BRONZE_DRAKE_VEHICLE, BronzeDrakePerchPositions[i]))
                                    _drakeGuids[i] = drake->GetGUID();
                        }
                        _events.Repeat(30s);
                        break;
                    case EVENT_PEROTHARN_INTRO_ON_LOAD:
                        if (Creature* perotharn = GetCreature(BOSS_PEROTHARN))
                            if (perotharn->IsAIEnabled())
                                perotharn->AI()->DoAction(ACTION_PEROTHARN_INTRO);
                        break;
                    default:
                        break;
                }
            }
        }

    private:
        void SummonGauntletIllidanIfNeeded()
        {
            if (!_firstPullDone || GetBossState(BOSS_PEROTHARN) == DONE)
                return;
            if (GetCreature(DATA_GAUNTLET_ILLIDAN))
                return;
            instance->SummonCreature(NPC_ILLIDAN_GAUNTLET, IllidanGauntletPositions[std::min<uint32>(_portalsShutDown, 3)]);
        }

        // Ambient Legion siege around the Well once the palace opens.
        void StartWellSiege()
        {
            if (_events.GetTimeUntilEvent(EVENT_LEGION_ARMY) != std::numeric_limits<uint32>::max())
                return;
            if (GetBossState(BOSS_MANNOROTH_AND_VAROTHEN) == DONE)
                return;
            instance->SummonCreature(NPC_PORTAL_TO_TWISTING_NETHER, NetherPortalSpawnPosition);
            _events.ScheduleEvent(EVENT_LEGION_ARMY, 5s);
        }

        EventMap _events;
        uint32 _firstPullDone;
        uint32 _portalsShutDown;
        bool _playerCaughtByEye;
        bool _felDrainTriggered;
        std::array<ObjectGuid, 5> _drakeGuids;
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_well_of_eternity_InstanceMapScript(map);
    }
};
}

void AddSC_instance_well_of_eternity()
{
    using namespace WellOfEternity;
    new instance_well_of_eternity();
}
