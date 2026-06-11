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

#include "InstanceMapScript.h"
#include "DBCStores.h"
#include "Log.h"
#include "Map.h"
#include "ScriptMgr.h"
#include "ScriptRegistry.h"

InstanceMapScript::InstanceMapScript(char const* name, uint32 mapId)
    : ScriptObject(name), MapScript<InstanceMap>(sMapStore.LookupEntry(mapId))
{
    if (!GetEntry())
        TC_LOG_ERROR("scripts", "Invalid InstanceMapScript for %u; no such map ID.", mapId);

    if (GetEntry() && !GetEntry()->IsDungeon())
        TC_LOG_ERROR("scripts", "InstanceMapScript for map %u is invalid.", mapId);

    ScriptRegistry<InstanceMapScript>::Instance()->AddScript(this);
}

InstanceScript* ScriptMgr::CreateInstanceData(InstanceMap* map)
{
    ASSERT(map);

    GET_SCRIPT_RET(InstanceMapScript, map->GetScriptId(), tmpscript, nullptr);
    return tmpscript->GetInstanceScript(map);
}

template class TC_GAME_API ScriptRegistry<InstanceMapScript>;
