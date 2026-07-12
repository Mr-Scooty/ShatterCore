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
#include "GameObject.h"
#include "GameObjectAI.h"
#include "InstanceScript.h"
#include "PassiveAI.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"

namespace EndTime
{
enum Events
{
    // Nozdormu
    EVENT_TALK_ENCOUNTER_INTRO = 1,
    EVENT_TALK_ENCOUNTER_OUTRO_1,
    EVENT_TALK_ENCOUNTER_OUTRO_2,
    EVENT_TALK_ENCOUNTER_OUTRO_3,
    EVENT_TALK_ENCOUNTER_OUTRO_4
};

enum Actions
{
    // Nozdormu
    ACTION_ENCOUNTER_INTRO = 1,
    ACTION_ENCOUNTER_OUTRO = 2
};

enum Texts
{
    // Nozdormu
    SAY_ENCOUNTER_INTRO     = 0,
    SAY_ENCOUNTER_OUTRO_1   = 1,
    SAY_ENCOUNTER_OUTRO_2   = 2,
    SAY_ENCOUNTER_OUTRO_3   = 3,
    SAY_ENCOUNTER_OUTRO_4   = 4
};

enum GossipMenuIds
{
    GOSSIP_MENU_ID_NOZDORMU                 = 13360,
    GOSSIP_MENU_OPTION_ID_WELL_OF_ETERNITY  = 0
};

Position const NozdormuEncounterTeleportPosition    = { 4033.37f, -294.457f, 181.613f, 5.8119464f };
Position const NozdormuDefeatTeleportPosition       = { 4138.48f, -429.436f, 122.259f, 5.8119464f };

struct npc_end_time_nozdormu : public NullCreatureAI
{
    npc_end_time_nozdormu(Creature* creature) : NullCreatureAI(creature),  _instance(nullptr), _introDone(false) { }

    void InitializeAI() override
    {
        _instance = me->GetInstanceScript();
    }

    bool GossipHello(Player* player) override
    {
        if (!_instance)
            return true;

        if (_instance->GetBossState(DATA_MUROZOND) == DONE)
            AddGossipItemFor(player, GOSSIP_MENU_ID_NOZDORMU, GOSSIP_MENU_OPTION_ID_WELL_OF_ETERNITY, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

        SendGossipMenuFor(player, player->GetGossipTextId(GOSSIP_MENU_ID_NOZDORMU, me), me->GetGUID());

        return true;
    }

    bool GossipSelect(Player* /*player*/, uint32 /*menuId*/, uint32 /*gossipListId*/) override
    {
        // Todo: Well of Eternity teleport

        return true;
    }

    void DoAction(int32 action) override
    {
        switch (action)
        {
            case ACTION_ENCOUNTER_INTRO:
                if (!_introDone)
                {
                    _events.ScheduleEvent(EVENT_TALK_ENCOUNTER_INTRO, 8s + 400ms);
                    _introDone = true;
                }
                break;
            case ACTION_ENCOUNTER_OUTRO:
                me->setActive(true);
                me->NearTeleportTo(NozdormuDefeatTeleportPosition);
                _events.ScheduleEvent(EVENT_TALK_ENCOUNTER_OUTRO_1, 8s + 700ms);
                me->setActive(false);
                break;
            default:
                break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_TALK_ENCOUNTER_INTRO:
                    Talk(SAY_ENCOUNTER_INTRO);
                    me->setActive(true);
                    me->NearTeleportTo(NozdormuEncounterTeleportPosition);
                    me->setActive(false);
                    break;
                case EVENT_TALK_ENCOUNTER_OUTRO_1:
                    Talk(SAY_ENCOUNTER_OUTRO_1);
                    _events.ScheduleEvent(EVENT_TALK_ENCOUNTER_OUTRO_2, 17s);
                    break;
                case EVENT_TALK_ENCOUNTER_OUTRO_2:
                    Talk(SAY_ENCOUNTER_OUTRO_2);
                    _events.ScheduleEvent(EVENT_TALK_ENCOUNTER_OUTRO_3, 17s);
                    break;
                case EVENT_TALK_ENCOUNTER_OUTRO_3:
                    Talk(SAY_ENCOUNTER_OUTRO_3);
                    _events.ScheduleEvent(EVENT_TALK_ENCOUNTER_OUTRO_4, 14s);
                    break;
                case EVENT_TALK_ENCOUNTER_OUTRO_4:
                    Talk(SAY_ENCOUNTER_OUTRO_4);
                    me->SetFacingTo(3.1765f);
                    break;
                default:
                    break;
            }
        }
    }

private:
    EventMap _events;
    InstanceScript* _instance;
    bool _introDone;
};

enum TimeTransitDeviceAreaIds
{
    AREA_ID_RUBY_DRAGONSHRINE       = 5790,
    AREA_ID_OBSIDIAN_DRAGONSHRINE   = 5792,
    AREA_ID_AZURE_DRAGONSHRINE      = 5793,
    AREA_ID_EMERALD_DRAGONSHRINE    = 5794,
    AREA_ID_BRONZE_DRAGONSHRINE     = 5795,
    AREA_ID_ENTRYWAY_OF_TIME        = 5796
};

enum TimeTransitDeviceSpells
{
    SPELL_TELEPORT_TO_ENTRANCE              = 102564,
    SPELL_TELEPORT_TO_BLUE_DRAGONSHRINE     = 102126,
    SPELL_TELEPORT_TO_RUBY_DRAGONSHRINE     = 102579,
    SPELL_TELEPORT_TO_OBSIDIAN_DRAGONSHRINE = 103868,
    SPELL_TELEPORT_TO_EMERALD_DRAGONSHRINE  = 104761,
    SPELL_TELEPORT_TO_BRONZE_DRAGONSHRINE   = 104764
};

enum TimeTransitGossipMenuIds
{
    GOSSIP_MENU_ID_SELECT_YOUR_DESTINATION = 13321
};

enum TimeTransitGossipIndexes
{
    GOSSIP_INDEX_TELEPORT_TO_ENTRYWAY_OF_TIME           = 0,
    GOSSIP_INDEX_TELEPORT_TO_FIRST_ECHO_RUBY            = 1,
    GOSSIP_INDEX_TELEPORT_TO_FIRST_ECHO_EMERALD         = 2,
    GOSSIP_INDEX_TELEPORT_TO_FIRST_ECHO_BLUE            = 3,
    GOSSIP_INDEX_TELEPORT_TO_FIRST_ECHO_OBSIDIAN        = 4,
    GOSSIP_INDEX_TELEPORT_TO_SECOND_ECHO_RUBY           = 5,
    GOSSIP_INDEX_TELEPORT_TO_SECOND_ECHO_EMERALD        = 6,
    GOSSIP_INDEX_TELEPORT_TO_SECOND_ECHO_BLUE           = 7,
    GOSSIP_INDEX_TELEPORT_TO_SECOND_ECHO_OBSIDIAN       = 8,
    GOSSIP_INDEX_TELEPORT_TO_BRONZE_DRAGONSHRINE        = 9
};

struct TransitDeviceEchoWing
{
    uint32 BossDataId;
    uint32 AreaId;
    uint32 FirstEchoGossipIndex;
    uint32 SecondEchoGossipIndex;
};

TransitDeviceEchoWing const TransitDeviceEchoWings[] =
{
    { DATA_ECHO_OF_SYLVANAS,    AREA_ID_RUBY_DRAGONSHRINE,      GOSSIP_INDEX_TELEPORT_TO_FIRST_ECHO_RUBY,       GOSSIP_INDEX_TELEPORT_TO_SECOND_ECHO_RUBY       },
    { DATA_ECHO_OF_TYRANDE,     AREA_ID_EMERALD_DRAGONSHRINE,   GOSSIP_INDEX_TELEPORT_TO_FIRST_ECHO_EMERALD,    GOSSIP_INDEX_TELEPORT_TO_SECOND_ECHO_EMERALD    },
    { DATA_ECHO_OF_JAINA,       AREA_ID_AZURE_DRAGONSHRINE,     GOSSIP_INDEX_TELEPORT_TO_FIRST_ECHO_BLUE,       GOSSIP_INDEX_TELEPORT_TO_SECOND_ECHO_BLUE       },
    { DATA_ECHO_OF_BAINE,       AREA_ID_OBSIDIAN_DRAGONSHRINE,  GOSSIP_INDEX_TELEPORT_TO_FIRST_ECHO_OBSIDIAN,   GOSSIP_INDEX_TELEPORT_TO_SECOND_ECHO_OBSIDIAN   }
};

static std::unordered_map<uint32 /*gossipIndex*/, uint32 /*teleportSpellId*/> TransitDeviceTeleportSpells =
{
    { GOSSIP_INDEX_TELEPORT_TO_ENTRYWAY_OF_TIME,        SPELL_TELEPORT_TO_ENTRANCE              },
    { GOSSIP_INDEX_TELEPORT_TO_FIRST_ECHO_RUBY,         SPELL_TELEPORT_TO_RUBY_DRAGONSHRINE     },
    { GOSSIP_INDEX_TELEPORT_TO_FIRST_ECHO_EMERALD,      SPELL_TELEPORT_TO_EMERALD_DRAGONSHRINE  },
    { GOSSIP_INDEX_TELEPORT_TO_FIRST_ECHO_BLUE,         SPELL_TELEPORT_TO_BLUE_DRAGONSHRINE     },
    { GOSSIP_INDEX_TELEPORT_TO_FIRST_ECHO_OBSIDIAN,     SPELL_TELEPORT_TO_OBSIDIAN_DRAGONSHRINE },
    { GOSSIP_INDEX_TELEPORT_TO_SECOND_ECHO_RUBY,        SPELL_TELEPORT_TO_RUBY_DRAGONSHRINE     },
    { GOSSIP_INDEX_TELEPORT_TO_SECOND_ECHO_EMERALD,     SPELL_TELEPORT_TO_EMERALD_DRAGONSHRINE  },
    { GOSSIP_INDEX_TELEPORT_TO_SECOND_ECHO_BLUE,        SPELL_TELEPORT_TO_BLUE_DRAGONSHRINE     },
    { GOSSIP_INDEX_TELEPORT_TO_SECOND_ECHO_OBSIDIAN,    SPELL_TELEPORT_TO_OBSIDIAN_DRAGONSHRINE },
    { GOSSIP_INDEX_TELEPORT_TO_BRONZE_DRAGONSHRINE,     SPELL_TELEPORT_TO_BRONZE_DRAGONSHRINE   }
};

struct go_end_time_time_transit_device : public GameObjectAI
{
    go_end_time_time_transit_device(GameObject* gameObject) : GameObjectAI(gameObject), _instance(nullptr) { }

    void InitializeAI() override
    {
        _instance = me->GetInstanceScript();
    }

    bool GossipHello(Player* player) override
    {
        if (!_instance)
            return false;

        if (player->GetAreaId() != AREA_ID_ENTRYWAY_OF_TIME)
            AddGossipItemFor(player, GOSSIP_MENU_ID_SELECT_YOUR_DESTINATION, GOSSIP_INDEX_TELEPORT_TO_ENTRYWAY_OF_TIME, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + AsUnderlyingType(GOSSIP_INDEX_TELEPORT_TO_ENTRYWAY_OF_TIME));

        // Each instance offers a randomly selected pair of Echo wings. The player's current shrine is never offered.
        uint32 const activeEchoes[2] = { _instance->GetData(DATA_ACTIVE_ECHO_1), _instance->GetData(DATA_ACTIVE_ECHO_2) };
        for (TransitDeviceEchoWing const& wing : TransitDeviceEchoWings)
        {
            if (player->GetAreaId() == wing.AreaId)
                continue;

            uint32 gossipIndex = 0;
            if (wing.BossDataId == activeEchoes[0])
                gossipIndex = wing.FirstEchoGossipIndex;
            else if (wing.BossDataId == activeEchoes[1])
                gossipIndex = wing.SecondEchoGossipIndex;
            else
                continue;

            AddGossipItemFor(player, GOSSIP_MENU_ID_SELECT_YOUR_DESTINATION, gossipIndex, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + gossipIndex);
        }

        // The Bronze Dragonshrine only opens up when both active Echoes have been defeated
        if (player->GetAreaId() != AREA_ID_BRONZE_DRAGONSHRINE
            && _instance->GetBossState(activeEchoes[0]) == DONE && _instance->GetBossState(activeEchoes[1]) == DONE)
            AddGossipItemFor(player, GOSSIP_MENU_ID_SELECT_YOUR_DESTINATION, GOSSIP_INDEX_TELEPORT_TO_BRONZE_DRAGONSHRINE, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + AsUnderlyingType(GOSSIP_INDEX_TELEPORT_TO_BRONZE_DRAGONSHRINE));

        SendGossipMenuFor(player, player->GetGossipTextId(GOSSIP_MENU_ID_SELECT_YOUR_DESTINATION, me), me->GetGUID());

        return true;
    }

    bool GossipSelect(Player* player, uint32 /*gossipMenuId*/, uint32 action) override
    {
        uint32 index = player->PlayerTalkClass->GetGossipOptionAction(action) - GOSSIP_ACTION_INFO_DEF;
        ClearGossipMenuFor(player);

        if (player->IsInCombat())
            return true;

        auto itr = TransitDeviceTeleportSpells.find(index);
        if (itr != TransitDeviceTeleportSpells.end())
            player->CastSpell(player, itr->second);

        return true;
    }

private:
    InstanceScript* _instance;
};

struct go_end_time_fragment_of_jainas_staff : public GameObjectAI
{
    go_end_time_fragment_of_jainas_staff(GameObject* gameObject) : GameObjectAI(gameObject), _instance(nullptr) { }

    void InitializeAI() override
    {
        _instance = me->GetInstanceScript();
    }

    bool GossipHello(Player* /*player*/) override
    {
        if (!_instance)
            return false;

        _instance->SetData(DATA_COLLECTED_FRAGMENT_OF_JAINAS_STAFF, 0);
        me->DespawnOrUnsummon();

        return true;
    }

private:
    InstanceScript* _instance;
};
}

void AddSC_end_time()
{
    using namespace EndTime;
    RegisterEndTimeCreatureAI(npc_end_time_nozdormu);
    RegisterGameObjectAI(go_end_time_time_transit_device);
    RegisterGameObjectAI(go_end_time_fragment_of_jainas_staff);
}
