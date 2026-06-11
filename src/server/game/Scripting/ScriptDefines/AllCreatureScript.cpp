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

#include "AllCreatureScript.h"
#include "ScriptMgr.h"
#include "ScriptRegistry.h"

AllCreatureScript::AllCreatureScript(char const* name)
    : ScriptObject(name)
{
    ScriptRegistry<AllCreatureScript>::Instance()->AddScript(this);
}

void ScriptMgr::OnCreatureAddWorld(Creature* creature)
{
    ASSERT(creature);

    FOREACH_SCRIPT(AllCreatureScript)->OnCreatureAddWorld(creature);
}

void ScriptMgr::OnCreatureRemoveWorld(Creature* creature)
{
    ASSERT(creature);

    FOREACH_SCRIPT(AllCreatureScript)->OnCreatureRemoveWorld(creature);
}

void ScriptMgr::OnCreatureSaveToDB(Creature* creature)
{
    ASSERT(creature);

    FOREACH_SCRIPT(AllCreatureScript)->OnCreatureSaveToDB(creature);
}

void ScriptMgr::OnBeforeCreatureSelectLevel(CreatureTemplate const* cinfo, Creature* creature, uint8& level)
{
    FOREACH_SCRIPT(AllCreatureScript)->OnBeforeCreatureSelectLevel(cinfo, creature, level);
}

void ScriptMgr::OnCreatureSelectLevel(CreatureTemplate const* cinfo, Creature* creature)
{
    FOREACH_SCRIPT(AllCreatureScript)->OnCreatureSelectLevel(cinfo, creature);
}

void ScriptMgr::OnCreatureUpdate(Creature* creature, uint32 diff)
{
    ASSERT(creature);

    FOREACH_SCRIPT(AllCreatureScript)->OnAllCreatureUpdate(creature, diff);
}

bool ScriptMgr::CanCreatureGossipHello(Player* player, Creature* creature)
{
    ASSERT(player);
    ASSERT(creature);

    FOR_SCRIPTS_RET(AllCreatureScript, itr, end, false)
        if (itr->second->CanCreatureGossipHello(player, creature))
            return true;

    return false;
}

bool ScriptMgr::CanCreatureGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)
{
    ASSERT(player);
    ASSERT(creature);

    FOR_SCRIPTS_RET(AllCreatureScript, itr, end, false)
        if (itr->second->CanCreatureGossipSelect(player, creature, sender, action))
            return true;

    return false;
}

bool ScriptMgr::CanCreatureGossipSelectCode(Player* player, Creature* creature, uint32 sender, uint32 action, char const* code)
{
    ASSERT(player);
    ASSERT(creature);
    ASSERT(code);

    FOR_SCRIPTS_RET(AllCreatureScript, itr, end, false)
        if (itr->second->CanCreatureGossipSelectCode(player, creature, sender, action, code))
            return true;

    return false;
}

bool ScriptMgr::CanCreatureQuestAccept(Player* player, Creature* creature, Quest const* quest)
{
    ASSERT(player);
    ASSERT(creature);
    ASSERT(quest);

    FOR_SCRIPTS_RET(AllCreatureScript, itr, end, false)
        if (itr->second->CanCreatureQuestAccept(player, creature, quest))
            return true;

    return false;
}

bool ScriptMgr::CanCreatureQuestReward(Player* player, Creature* creature, Quest const* quest, uint32 opt)
{
    ASSERT(player);
    ASSERT(creature);
    ASSERT(quest);

    FOR_SCRIPTS_RET(AllCreatureScript, itr, end, false)
        if (itr->second->CanCreatureQuestReward(player, creature, quest, opt))
            return true;

    return false;
}

template class TC_GAME_API ScriptRegistry<AllCreatureScript>;
