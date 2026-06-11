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

#include "GameObjectScript.h"
#include "AllGameObjectScript.h"
#include "GameObject.h"
#include "ScriptMgr.h"
#include "ScriptRegistry.h"

GameObjectScript::GameObjectScript(char const* name)
    : ScriptObject(name)
{
    ScriptRegistry<GameObjectScript>::Instance()->AddScript(this);
}

GameObjectAI* ScriptMgr::GetGameObjectAI(GameObject* gameobject)
{
    ASSERT(gameobject);

    // AllGameObjectScripts may provide an AI for any gameobject,
    // the first registered script which returns one wins.
    FOR_SCRIPTS(AllGameObjectScript, itr, end)
        if (GameObjectAI* ai = itr->second->GetGameObjectAI(gameobject))
            return ai;

    GET_SCRIPT_RET(GameObjectScript, gameobject->GetScriptId(), tmpscript, nullptr);
    return tmpscript->GetAI(gameobject);
}

template class TC_GAME_API ScriptRegistry<GameObjectScript>;
