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

#include "AllGameObjectScript.h"
#include "ScriptMgr.h"
#include "ScriptRegistry.h"

AllGameObjectScript::AllGameObjectScript(char const* name)
    : ScriptObject(name)
{
    ScriptRegistry<AllGameObjectScript>::Instance()->AddScript(this);
}

void ScriptMgr::OnGameObjectAddWorld(GameObject* go)
{
    ASSERT(go);

    FOREACH_SCRIPT(AllGameObjectScript)->OnGameObjectAddWorld(go);
}

void ScriptMgr::OnGameObjectRemoveWorld(GameObject* go)
{
    ASSERT(go);

    FOREACH_SCRIPT(AllGameObjectScript)->OnGameObjectRemoveWorld(go);
}

void ScriptMgr::OnGameObjectSaveToDB(GameObject* go)
{
    ASSERT(go);

    FOREACH_SCRIPT(AllGameObjectScript)->OnGameObjectSaveToDB(go);
}

void ScriptMgr::OnGameObjectUpdate(GameObject* go, uint32 diff)
{
    ASSERT(go);

    FOREACH_SCRIPT(AllGameObjectScript)->OnGameObjectUpdate(go, diff);
}

bool ScriptMgr::CanGameObjectGossipHello(Player* player, GameObject* go)
{
    ASSERT(player);
    ASSERT(go);

    FOR_SCRIPTS_RET(AllGameObjectScript, itr, end, false)
        if (itr->second->CanGameObjectGossipHello(player, go))
            return true;

    return false;
}

bool ScriptMgr::CanGameObjectGossipSelect(Player* player, GameObject* go, uint32 sender, uint32 action)
{
    ASSERT(player);
    ASSERT(go);

    FOR_SCRIPTS_RET(AllGameObjectScript, itr, end, false)
        if (itr->second->CanGameObjectGossipSelect(player, go, sender, action))
            return true;

    return false;
}

bool ScriptMgr::CanGameObjectGossipSelectCode(Player* player, GameObject* go, uint32 sender, uint32 action, char const* code)
{
    ASSERT(player);
    ASSERT(go);
    ASSERT(code);

    FOR_SCRIPTS_RET(AllGameObjectScript, itr, end, false)
        if (itr->second->CanGameObjectGossipSelectCode(player, go, sender, action, code))
            return true;

    return false;
}

bool ScriptMgr::CanGameObjectQuestAccept(Player* player, GameObject* go, Quest const* quest)
{
    ASSERT(player);
    ASSERT(go);
    ASSERT(quest);

    FOR_SCRIPTS_RET(AllGameObjectScript, itr, end, false)
        if (itr->second->CanGameObjectQuestAccept(player, go, quest))
            return true;

    return false;
}

bool ScriptMgr::CanGameObjectQuestReward(Player* player, GameObject* go, Quest const* quest, uint32 opt)
{
    ASSERT(player);
    ASSERT(go);
    ASSERT(quest);

    FOR_SCRIPTS_RET(AllGameObjectScript, itr, end, false)
        if (itr->second->CanGameObjectQuestReward(player, go, quest, opt))
            return true;

    return false;
}

void ScriptMgr::OnGameObjectDamaged(GameObject* go, Player* player)
{
    ASSERT(go);

    FOREACH_SCRIPT(AllGameObjectScript)->OnGameObjectDamaged(go, player);
}

void ScriptMgr::OnGameObjectDestroyed(GameObject* go, Player* player)
{
    ASSERT(go);

    FOREACH_SCRIPT(AllGameObjectScript)->OnGameObjectDestroyed(go, player);
}

void ScriptMgr::OnGameObjectModifyHealth(GameObject* go, WorldObject* attackerOrHealer, int32& change, SpellInfo const* spellInfo)
{
    ASSERT(go);

    FOREACH_SCRIPT(AllGameObjectScript)->OnGameObjectModifyHealth(go, attackerOrHealer, change, spellInfo);
}

void ScriptMgr::OnGameObjectLootStateChanged(GameObject* go, uint32 state, Unit* unit)
{
    ASSERT(go);

    FOREACH_SCRIPT(AllGameObjectScript)->OnGameObjectLootStateChanged(go, state, unit);
}

void ScriptMgr::OnGameObjectStateChanged(GameObject* go, uint32 state)
{
    ASSERT(go);

    FOREACH_SCRIPT(AllGameObjectScript)->OnGameObjectStateChanged(go, state);
}

template class TC_GAME_API ScriptRegistry<AllGameObjectScript>;
