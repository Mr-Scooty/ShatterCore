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

#include "end_time.h"
#include "Containers.h"
#include "EventMap.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "InstanceScript.h"
#include "Map.h"
#include "ScriptMgr.h"
#include "TemporarySummon.h"

#include <algorithm>
#include <array>
#include <sstream>

namespace EndTime
{
ObjectData const creatureData[] =
{
    { BOSS_MUROZOND,            DATA_MUROZOND           },
    { BOSS_ECHO_OF_JAINA,       DATA_ECHO_OF_JAINA      },
    { BOSS_ECHO_OF_BAINE,       DATA_ECHO_OF_BAINE      },
    { BOSS_ECHO_OF_SYLVANAS,    DATA_ECHO_OF_SYLVANAS   },
    { BOSS_ECHO_OF_TYRANDE,     DATA_ECHO_OF_TYRANDE    },
    { NPC_ARCANE_CIRCLE,        DATA_ARCANE_CIRCLE      },
    { 0,                        0                       } // END
};

ObjectData const gameobjectData[] =
{
    { GO_HOURGLASS_OF_TIME, DATA_HOURGLASS_OF_TIME } // END
};

DoorData const doorData[] =
{
    { GO_FIRE_WALL, DATA_ECHO_OF_BAINE, DOOR_TYPE_ROOM },
    { 0,            0,                  DOOR_TYPE_ROOM } // END
};

enum ETEvents
{
    EVENT_RESPAWN_MUROZOND = 1
};

enum ETSpawnGroups
{
    SPAWN_GROUP_ID_MUROZOND_CHEST   = 437,
    SPAWN_GROUP_ID_ECHO_OF_JAINA    = 463
};

enum ETAreaIds
{
    AREA_ID_BRONZE_DRAGON_SHRINE = 5795
};

enum ETWorldStates
{
    WORLD_STATE_ID_SHOW_COLLECTED_STAVE_FRAGMENTS   = 6046,
    WORLD_STATE_ID_COLLECTED_STAVE_FRAGMENTS        = 6025
};

enum ETSpells
{
    SPELL_SUMMON_PHANTOM            = 102200,
    SPELL_FIRST_ECHO_KILL_CREDIT    = 110163, // Serverside Spell
    SPELL_SECOND_ECHO_KILL_CREDIT   = 110164  // Serverside Spell
};

enum ETAchievementCriteria
{
    CRITERIA_ID_SEVERED_TIES = 18499
};

std::array<Position const, 2> MurozondSpawnPositions =
{
    Position(4288.125f, -456.40277f, 160.4989f,  2.98451f), // Initial spawn position
    Position(4181.117f, -420.21933f, 138.38057f, 3.10668f)  // Respawn position
};

class instance_end_time : public InstanceMapScript
{
public:
    instance_end_time() : InstanceMapScript(ETScriptName, 938) { }

    struct instance_end_time_InstanceMapScript : public InstanceScript
    {
        instance_end_time_InstanceMapScript(InstanceMap* map) : InstanceScript(map),
            _killedInfiniteDragonkins(0), _collectedStaffFragments(0), _killedEchoes(0),
            _moonGuardEligible(true), _severedTiesEligible(false), _loadInProgress(false),
            _shadowGauntletState(NOT_STARTED)
        {
            SetHeaders(DataHeader);
            SetBossNumber(EncounterCount);
            LoadDoorData(doorData);
            LoadObjectData(creatureData, gameobjectData);
            _activeEchoes = { DATA_ECHO_OF_BAINE, DATA_ECHO_OF_JAINA };
        }

        void ProcessEvent(WorldObject* /*obj*/, uint32 eventId, WorldObject* /*invoker*/) override
        {
            switch (eventId)
            {
                case MAP_EVENT_AZURE_DRAGONSHRINE_ENTERED:
                    if (_executedMapEvents.find(eventId) != _executedMapEvents.end())
                        return;
                    if (GetBossState(DATA_ECHO_OF_JAINA) != DONE)
                        DoUpdateWorldState(WORLD_STATE_ID_SHOW_COLLECTED_STAVE_FRAGMENTS, 1);
                    break;
                case MAP_EVENT_MOON_GUARD_FAILED:
                    SetData(DATA_MOON_GUARD_ELIGIBLE, 0);
                    return;
                case MAP_EVENT_OBSIDIAN_DRAGONSHRINE_ENTERED:
                    if (_executedMapEvents.find(eventId) != _executedMapEvents.end())
                        return;
                    if (Creature* baine = GetCreature(DATA_ECHO_OF_BAINE))
                        if (baine->IsAIEnabled())
                            baine->AI()->DoAction(1 /*ACTION_INTRO*/);
                    break;
                case MAP_EVENT_EMERALD_DRAGONSHRINE_ENTERED:
                    // Not fire-once on purpose - the gauntlet re-arms itself after a wipe and restarts on the next arrival
                    if (Creature* tyrande = GetCreature(DATA_ECHO_OF_TYRANDE))
                        if (tyrande->IsAIEnabled())
                            tyrande->AI()->DoAction(1 /*ACTION_START_GAUNTLET*/);
                    return;
                default:
                    return;
            }

            _executedMapEvents.insert(eventId);
        }

        void Create() override
        {
            InstanceScript::Create();

            SelectActiveEchoes();

            // The instance has been created without save data, just spawn Murozond at his initial position.
            instance->SummonCreature(BOSS_MUROZOND, MurozondSpawnPositions[0]);
        }

        void Load(char const* data) override
        {
            _loadInProgress = true;
            InstanceScript::Load(data);
            _loadInProgress = false;

            // The instance has been created from existing save data, but Murozond has not been defeated yet. Spawn him at his initial position.
            if (GetBossState(DATA_MUROZOND) != DONE)
            {
                if (Creature* murozond = instance->SummonCreature(BOSS_MUROZOND, MurozondSpawnPositions[0]))
                    if (_killedInfiniteDragonkins >= 8 && murozond->IsAIEnabled())
                        murozond->AI()->SetData(DATA_MUROZOND_INTRO, DONE);
            }
            else
                instance->SpawnGroupSpawn(SPAWN_GROUP_ID_MUROZOND_CHEST);

            // Manual spawn groups are not restored automatically with instance save data.
            if (_collectedStaffFragments >= 16 && GetBossState(DATA_ECHO_OF_JAINA) != DONE)
                instance->SpawnGroupSpawn(SPAWN_GROUP_ID_ECHO_OF_JAINA);

            DoUpdateWorldState(WORLD_STATE_ID_COLLECTED_STAVE_FRAGMENTS, _collectedStaffFragments);

            for (uint8 echo : _activeEchoes)
                if (GetBossState(echo) == DONE)
                    ++_killedEchoes;
        }

        void WriteSaveDataMore(std::ostringstream& data) override
        {
            data << ' ' << uint32(_activeEchoes[0]) << ' ' << uint32(_activeEchoes[1])
                 << ' ' << uint32(_shadowGauntletState == DONE ? DONE : NOT_STARTED)
                 << ' ' << uint32(_moonGuardEligible)
                 << ' ' << uint32(_collectedStaffFragments)
                 << ' ' << uint32(_killedInfiniteDragonkins);
        }

        void ReadSaveDataMore(std::istringstream& data) override
        {
            uint32 first = DATA_ECHO_OF_BAINE, second = DATA_ECHO_OF_JAINA, gauntlet = NOT_STARTED;
            uint32 moonGuardEligible = 1, collectedStaffFragments = 0, killedInfiniteDragonkins = 0;
            data >> first >> second >> gauntlet;
            data >> moonGuardEligible >> collectedStaffFragments >> killedInfiniteDragonkins;
            if (first > DATA_ECHO_OF_TYRANDE || second > DATA_ECHO_OF_TYRANDE || first == second)
            {
                // Corrupt save data, fall back to a fixed pair.
                first = DATA_ECHO_OF_BAINE;
                second = DATA_ECHO_OF_JAINA;
            }

            _activeEchoes = { uint8(first), uint8(second) };
            _shadowGauntletState = gauntlet == DONE ? DONE : NOT_STARTED;
            _moonGuardEligible = moonGuardEligible != 0;
            _collectedStaffFragments = uint8(std::min<uint32>(collectedStaffFragments, 16));
            _killedInfiniteDragonkins = uint8(std::min<uint32>(killedInfiniteDragonkins, 8));
        }

        void OnUnitDeath(Unit* unit) override
        {
            if (!unit->IsCreature())
                return;

            switch (unit->GetEntry())
            {
                case NPC_INFINITE_SUPRESSOR:
                case NPC_INFINITE_WARDEN:
                    if (_killedInfiniteDragonkins >= 8 || GetBossState(DATA_MUROZOND) == DONE)
                        break;

                    ++_killedInfiniteDragonkins;
                    if (_killedInfiniteDragonkins == 4 || _killedInfiniteDragonkins == 8)
                        if (Creature* murozond = GetCreature(DATA_MUROZOND))
                            if (murozond->IsAIEnabled())
                                murozond->AI()->SetData(DATA_MUROZOND_INTRO, _killedInfiniteDragonkins == 4 ? IN_PROGRESS : DONE);
                    SaveToDB();
                    break;
                default:
                    break;
            }
        }

        void OnCreatureCreate(Creature* creature) override
        {
            InstanceScript::OnCreatureCreate(creature);

            switch (creature->GetEntry())
            {
                case NPC_NOZDORMU_DRAGON_SHRINES:
                    if (creature->GetAreaId() == AREA_ID_BRONZE_DRAGON_SHRINE)
                        AddObject(creature, DATA_NOZDORMU_BRONZE_DRAGON_SHRINE, true);
                    break;
                default:
                    break;
            }
        }

        void OnCreatureRemove(Creature* creature) override
        {
            InstanceScript::OnCreatureRemove(creature);

            switch (creature->GetEntry())
            {
                case NPC_NOZDORMU_DRAGON_SHRINES:
                    if (creature->GetAreaId() == AREA_ID_BRONZE_DRAGON_SHRINE)
                        AddObject(creature, DATA_NOZDORMU_BRONZE_DRAGON_SHRINE, false);
                    break;
                default:
                    break;
            }
        }

        bool SetBossState(uint32 type, EncounterState state) override
        {
            if (!InstanceScript::SetBossState(type, state))
                return false;

            switch (type)
            {
                case DATA_MUROZOND:
                    if (state == FAIL)
                        _events.ScheduleEvent(EVENT_RESPAWN_MUROZOND, 30s);
                    else if (state == DONE)
                        instance->SpawnGroupSpawn(SPAWN_GROUP_ID_MUROZOND_CHEST);
                    break;
                case DATA_ECHO_OF_JAINA:
                    if (state == IN_PROGRESS)
                    {
                        DoUpdateWorldState(WORLD_STATE_ID_SHOW_COLLECTED_STAVE_FRAGMENTS, 0);
                        if (Creature* circle = GetCreature(DATA_ARCANE_CIRCLE))
                            circle->DespawnOrUnsummon();
                    }
                    break;
                case DATA_ECHO_OF_SYLVANAS:
                    if (state == IN_PROGRESS)
                        _severedTiesEligible = false; // re-armed by the boss AI when two ghouls die during one Calling of the Highborne
                    break;
                default:
                    break;
            }

            // The dungeon encounters are generic 'First Echo' and 'Second Echo' entries, credited in kill order.
            // Boss states replayed from save data are counted in Load() instead.
            if (type <= DATA_ECHO_OF_TYRANDE && state == DONE && !_loadInProgress
                && IsActiveEcho(type) && _killedEchoes < 2)
            {
                ++_killedEchoes;
                DoCastSpellOnPlayers(_killedEchoes == 1 ? SPELL_FIRST_ECHO_KILL_CREDIT : SPELL_SECOND_ECHO_KILL_CREDIT);
            }

            return true;
        }

        void SetData(uint32 type, uint32 value) override
        {
            switch (type)
            {
                case DATA_COLLECTED_FRAGMENT_OF_JAINAS_STAFF:
                    if (GetBossState(DATA_ECHO_OF_JAINA) == DONE || _collectedStaffFragments >= 16)
                        break;

                    ++_collectedStaffFragments;
                    DoUpdateWorldState(WORLD_STATE_ID_COLLECTED_STAVE_FRAGMENTS, _collectedStaffFragments);

                    if (_collectedStaffFragments < 16)
                    {
                        if (Creature* circle = GetCreature(DATA_ARCANE_CIRCLE))
                            circle->CastSpell(nullptr, SPELL_SUMMON_PHANTOM);
                    }
                    else
                        instance->SpawnGroupSpawn(SPAWN_GROUP_ID_ECHO_OF_JAINA);
                    SaveToDB();
                    break;
                case DATA_SEVERED_TIES_ELIGIBLE:
                    _severedTiesEligible = value != 0;
                    break;
                case DATA_MOON_GUARD_ELIGIBLE:
                {
                    bool eligible = value != 0;
                    if (_moonGuardEligible != eligible)
                    {
                        _moonGuardEligible = eligible;
                        SaveToDB();
                    }
                    break;
                }
                case DATA_SHADOW_GAUNTLET:
                    _shadowGauntletState = EncounterState(value);
                    if (value == DONE)
                        SaveToDB();
                    break;
                default:
                    break;
            }
        }

        uint32 GetData(uint32 type) const override
        {
            switch (type)
            {
                case DATA_ACTIVE_ECHO_1:
                    return _activeEchoes[0];
                case DATA_ACTIVE_ECHO_2:
                    return _activeEchoes[1];
                case DATA_MOON_GUARD_ELIGIBLE:
                    return _moonGuardEligible ? 1 : 0;
                case DATA_SEVERED_TIES_ELIGIBLE:
                    return _severedTiesEligible ? 1 : 0;
                case DATA_SHADOW_GAUNTLET:
                    return _shadowGauntletState;
                default:
                    return 0;
            }
        }

        bool CheckAchievementCriteriaMeet(uint32 criteriaId, Player const* /*source*/, Unit const* /*target*/, uint32 /*miscValue1*/) override
        {
            switch (criteriaId)
            {
                case CRITERIA_ID_SEVERED_TIES:
                    return _severedTiesEligible;
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
                    case EVENT_RESPAWN_MUROZOND:
                        instance->SummonCreature(BOSS_MUROZOND, MurozondSpawnPositions[1]);
                        break;
                    default:
                        break;
                }
            }
        }

    private:
        bool IsActiveEcho(uint32 type) const
        {
            return type == _activeEchoes[0] || type == _activeEchoes[1];
        }

        void SelectActiveEchoes()
        {
            std::array<uint8, 4> pool = { DATA_ECHO_OF_BAINE, DATA_ECHO_OF_JAINA, DATA_ECHO_OF_SYLVANAS, DATA_ECHO_OF_TYRANDE };
            Trinity::Containers::RandomShuffle(pool);
            _activeEchoes = { pool[0], pool[1] };
        }

        EventMap _events;
        uint8 _killedInfiniteDragonkins;
        uint8 _collectedStaffFragments;
        uint8 _killedEchoes;
        bool _moonGuardEligible;
        bool _severedTiesEligible;
        bool _loadInProgress;
        EncounterState _shadowGauntletState;
        std::array<uint8, 2> _activeEchoes;

        std::unordered_set<uint32> _executedMapEvents;
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_end_time_InstanceMapScript(map);
    }
};
}

void AddSC_instance_end_time()
{
    using namespace EndTime;
    new instance_end_time();
}
