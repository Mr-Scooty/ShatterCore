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
#include "hour_of_twilight.h"

namespace HourOfTwilight
{
enum HOTInstanceSpells
{
    SPELL_CANCEL_ECLIPSE = 110260 // Serverside - fail event for criteria 18669 (Eclipse spark counter)
};

enum HOTInstanceEvents
{
    EVENT_OPEN_ICEWALLS = 1,        // both walls open 10s after Arcurion dies (sniffed)
    EVENT_REVEAL_GALAKROND_THRALL   // fallback in case the frozen Thrall's wolf-run RP is interrupted
};

ObjectData const creatureData[] =
{
    { NPC_ARCURION,               DATA_ARCURION               },
    { NPC_ASIRA_DAWNSLAYER,       DATA_ASIRA_DAWNSLAYER       },
    { NPC_ARCHBISHOP_BENEDICTUS,  DATA_ARCHBISHOP_BENEDICTUS  },
    { NPC_THRALL_ENTRANCE,        DATA_THRALL_ENTRANCE        },
    { NPC_THRALL_FROZEN,          DATA_THRALL_FROZEN          },
    { NPC_THRALL_GALAKROND,       DATA_THRALL_GALAKROND       },
    { NPC_THRALL_TITANS,          DATA_THRALL_TITANS          },
    { NPC_THRALL_EPILOGUE,        DATA_THRALL_EPILOGUE        },
    { NPC_LIFE_WARDEN_THRALL,     DATA_LIFE_WARDEN_THRALLS    },
    { 0,                          0                           } // END
};

ObjectData const gameobjectData[] =
{
    { GO_ICEWALL_ARENA,           DATA_ICEWALL_ARENA          },
    { GO_ICEWALL_EXIT,            DATA_ICEWALL_EXIT           },
    { GO_EXIT_PORTAL,             DATA_EXIT_PORTAL            },
    { 0,                          0                           } // END
};

DoorData const doorData[] =
{
    { 0, 0, DOOR_TYPE_ROOM } // END - no encounter doors in this instance
};

class instance_hour_of_twilight : public InstanceMapScript
{
public:
    instance_hour_of_twilight() : InstanceMapScript(HOTScriptName, 940) { }

    struct instance_hour_of_twilight_InstanceMapScript : public InstanceScript
    {
        instance_hour_of_twilight_InstanceMapScript(InstanceMap* map) : InstanceScript(map),
            _escortStage(STAGE_NONE)
        {
            SetHeaders(DataHeader);
            SetBossNumber(EncounterCount);
            LoadDoorData(doorData);
            LoadObjectData(creatureData, gameobjectData);
        }

        void OnCreatureCreate(Creature* creature) override
        {
            InstanceScript::OnCreatureCreate(creature);

            switch (creature->GetEntry())
            {
                // Only the Thrall that matches the current escort stage stays visible.
                case NPC_THRALL_ENTRANCE:
                    creature->SetVisible(_escortStage < STAGE_ARCURION_READY);
                    break;
                case NPC_THRALL_FROZEN:
                    creature->SetVisible(_escortStage == STAGE_ARCURION_READY);
                    break;
                case NPC_THRALL_GALAKROND:
                    creature->SetVisible(_escortStage >= STAGE_ARCURION_DONE && _escortStage < STAGE_ASIRA_DONE);
                    break;
                case NPC_THRALL_TITANS:
                    creature->SetVisible(_escortStage >= STAGE_ASIRA_DONE && _escortStage < STAGE_BENEDICTUS_READY);
                    break;
                case NPC_THRALL_EPILOGUE:
                    // 54971 is both the Benedictus fight ally and the epilogue quest ender.
                    creature->SetVisible(_escortStage >= STAGE_BENEDICTUS_READY);
                    break;
                case NPC_ARCURION:
                    // Revealed by the frozen-leg Thrall's ready check on a live run; a grid
                    // reload at that stage skips the materialize RP.
                    creature->SetVisible(_escortStage >= STAGE_ARCURION_READY && GetBossState(DATA_ARCURION) != DONE);
                    break;
                case NPC_ASIRA_DAWNSLAYER:
                    creature->SetVisible(_escortStage >= STAGE_ASIRA_READY && GetBossState(DATA_ASIRA_DAWNSLAYER) != DONE);
                    break;
                case NPC_LIFE_WARDEN_THRALL:
                    creature->SetVisible(_escortStage >= STAGE_ASIRA_READY && _escortStage < STAGE_ASIRA_DONE);
                    break;
                case NPC_LIFE_WARDEN_TAXI:
                    creature->SetVisible(_escortStage >= STAGE_ASIRA_DONE);
                    break;
                // Canyon ambushers reveal at their scripted beats during leg 1.
                case NPC_FROZEN_SERVITOR:
                case NPC_CRYSTALLINE_ELEMENTAL:
                case NPC_FROZEN_SHARD:
                    if (creature->IsAlive())
                        creature->SetVisible(false);
                    break;
                default:
                    break;
            }
        }

        void OnGameObjectCreate(GameObject* go) override
        {
            InstanceScript::OnGameObjectCreate(go);

            switch (go->GetEntry())
            {
                case GO_ICEWALL_ARENA:
                    // Open by default; the Arcurion script closes it for the duration of the fight.
                    go->SetGoState(GO_STATE_ACTIVE);
                    break;
                case GO_ICEWALL_EXIT:
                    go->SetGoState(_escortStage >= STAGE_ARCURION_DONE ? GO_STATE_ACTIVE : GO_STATE_READY);
                    break;
                case GO_EXIT_PORTAL:
                    if (_escortStage < STAGE_BENEDICTUS_DONE)
                        go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    break;
                default:
                    break;
            }
        }

        void SetData(uint32 type, uint32 data) override
        {
            switch (type)
            {
                case DATA_ESCORT_STAGE:
                    SetEscortStage(EscortStage(data));
                    break;
                default:
                    break;
            }
        }

        uint32 GetData(uint32 type) const override
        {
            switch (type)
            {
                case DATA_ESCORT_STAGE:
                    return _escortStage;
                default:
                    return 0;
            }
        }

        void SetEscortStage(EscortStage stage)
        {
            if (stage <= _escortStage)
                return; // monotonic - never regress a checkpoint

            _escortStage = stage;

            switch (stage)
            {
                case STAGE_ARCURION_READY:
                    if (Creature* thrall = GetCreature(DATA_THRALL_ENTRANCE))
                        thrall->SetVisible(false);
                    if (Creature* frozen = GetCreature(DATA_THRALL_FROZEN))
                        frozen->SetVisible(true);
                    break;
                case STAGE_ARCURION_DONE:
                    _events.ScheduleEvent(EVENT_OPEN_ICEWALLS, 10s);
                    _events.ScheduleEvent(EVENT_REVEAL_GALAKROND_THRALL, 90s);
                    break;
                case STAGE_ASIRA_READY:
                    if (Creature* warden = GetCreature(DATA_LIFE_WARDEN_THRALLS))
                    {
                        warden->SetVisible(true);
                        if (warden->IsAIEnabled())
                            warden->AI()->DoAction(ACTION_ASIRA_ARRIVES);
                    }
                    break;
                case STAGE_ASIRA_DONE:
                {
                    if (Creature* thrall = GetCreature(DATA_THRALL_TITANS))
                        thrall->SetVisible(true);
                    for (auto const& [spawnId, creature] : instance->GetCreatureBySpawnIdStore())
                        if (creature->GetEntry() == NPC_LIFE_WARDEN_TAXI)
                            creature->SetVisible(true);
                    break;
                }
                case STAGE_BENEDICTUS_READY:
                    if (Creature* benedictus = GetCreature(DATA_ARCHBISHOP_BENEDICTUS))
                        if (benedictus->IsAIEnabled())
                            benedictus->AI()->DoAction(ACTION_BENEDICTUS_MEET_PARTY);
                    break;
                case STAGE_BENEDICTUS_DONE:
                    if (GameObject* portal = GetGameObject(DATA_EXIT_PORTAL))
                        portal->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    break;
                default:
                    break;
            }

            SaveToDB();
        }

        bool SetBossState(uint32 type, EncounterState state) override
        {
            if (!InstanceScript::SetBossState(type, state))
                return false;

            if (state == DONE)
            {
                switch (type)
                {
                    case DATA_ARCURION:
                        SetEscortStage(STAGE_ARCURION_DONE);
                        break;
                    case DATA_ASIRA_DAWNSLAYER:
                        SetEscortStage(STAGE_ASIRA_DONE);
                        break;
                    case DATA_ARCHBISHOP_BENEDICTUS:
                        // Reset every player's Eclipse spark counter (fail event for criteria
                        // 18669) so spark kills after the boss died never roll into a later run.
                        DoCastSpellOnPlayers(SPELL_CANCEL_ECLIPSE);
                        SetEscortStage(STAGE_BENEDICTUS_DONE);
                        break;
                    default:
                        break;
                }
            }
            return true;
        }

        void Update(uint32 diff) override
        {
            _events.Update(diff);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_OPEN_ICEWALLS:
                        if (GameObject* wall = GetGameObject(DATA_ICEWALL_ARENA))
                            wall->SetGoState(GO_STATE_ACTIVE);
                        if (GameObject* wall = GetGameObject(DATA_ICEWALL_EXIT))
                            wall->SetGoState(GO_STATE_ACTIVE);
                        break;
                    case EVENT_REVEAL_GALAKROND_THRALL:
                        // Normally revealed when the frozen-leg Thrall finishes his wolf-run;
                        // this is the safety net if that RP got interrupted.
                        if (Creature* thrall = GetCreature(DATA_THRALL_GALAKROND))
                            if (_escortStage >= STAGE_ARCURION_DONE && _escortStage < STAGE_ASIRA_DONE)
                                thrall->SetVisible(true);
                        break;
                    default:
                        break;
                }
            }
        }

        void WriteSaveDataMore(std::ostringstream& data) override
        {
            data << ' ' << uint32(_escortStage);
        }

        void ReadSaveDataMore(std::istringstream& data) override
        {
            uint32 stage = 0;
            data >> stage;

            // Snap to the nearest stable checkpoint - mid-leg progress rewinds to the leg start.
            _escortStage = EscortStage(stage);
            if (_escortStage == STAGE_CANYON_ESCORT)
                _escortStage = STAGE_NONE;
            else if (_escortStage == STAGE_GALAKROND_ESCORT || _escortStage == STAGE_ASIRA_READY)
                _escortStage = STAGE_ARCURION_DONE;
            else if (_escortStage == STAGE_TITANS_ESCORT)
                _escortStage = STAGE_ASIRA_DONE;
            else if (_escortStage > STAGE_BENEDICTUS_DONE)
                _escortStage = STAGE_NONE; // corrupt save data

            // Boss states already persisted by the core take precedence.
            if (GetBossState(DATA_ARCHBISHOP_BENEDICTUS) == DONE)
                _escortStage = STAGE_BENEDICTUS_DONE;
            else if (GetBossState(DATA_ASIRA_DAWNSLAYER) == DONE && _escortStage < STAGE_ASIRA_DONE)
                _escortStage = STAGE_ASIRA_DONE;
            else if (GetBossState(DATA_ARCURION) == DONE && _escortStage < STAGE_ARCURION_DONE)
                _escortStage = STAGE_ARCURION_DONE;
        }

    private:
        EventMap _events;
        EscortStage _escortStage;
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_hour_of_twilight_InstanceMapScript(map);
    }
};
}

void AddSC_instance_hour_of_twilight()
{
    using namespace HourOfTwilight;
    new instance_hour_of_twilight();
}
