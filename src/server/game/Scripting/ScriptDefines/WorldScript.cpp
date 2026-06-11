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

#include "WorldScript.h"
#include "ScriptMgr.h"
#include "ScriptRegistry.h"

WorldScript::WorldScript(char const* name, std::vector<uint16> /*enabledHooks*/)
    : ScriptObject(name)
{
    ScriptRegistry<WorldScript>::Instance()->AddScript(this);
}

void ScriptMgr::OnOpenStateChange(bool open)
{
    FOREACH_SCRIPT(WorldScript)->OnOpenStateChange(open);
}

void ScriptMgr::OnConfigLoad(bool reload)
{
    FOREACH_SCRIPT(WorldScript)->OnAfterConfigLoad(reload);
}

void ScriptMgr::OnBeforeConfigLoad(bool reload)
{
    FOREACH_SCRIPT(WorldScript)->OnBeforeConfigLoad(reload);
}

void ScriptMgr::OnLoadCustomDatabaseTable()
{
    FOREACH_SCRIPT(WorldScript)->OnLoadCustomDatabaseTable();
}

void ScriptMgr::OnMotdChange(std::string& newMotd)
{
    FOREACH_SCRIPT(WorldScript)->OnMotdChange(newMotd);
}

void ScriptMgr::OnShutdownInitiate(ShutdownExitCode code, ShutdownMask mask)
{
    FOREACH_SCRIPT(WorldScript)->OnShutdownInitiate(code, mask);
}

void ScriptMgr::OnShutdownCancel()
{
    FOREACH_SCRIPT(WorldScript)->OnShutdownCancel();
}

void ScriptMgr::OnWorldUpdate(uint32 diff)
{
    FOREACH_SCRIPT(WorldScript)->OnUpdate(diff);
}

void ScriptMgr::OnStartup()
{
    FOREACH_SCRIPT(WorldScript)->OnStartup();
}

void ScriptMgr::OnShutdown()
{
    FOREACH_SCRIPT(WorldScript)->OnShutdown();
}

void ScriptMgr::OnAfterUnloadAllMaps()
{
    FOREACH_SCRIPT(WorldScript)->OnAfterUnloadAllMaps();
}

void ScriptMgr::OnBeforeWorldInitialized()
{
    FOREACH_SCRIPT(WorldScript)->OnBeforeWorldInitialized();
}

void ScriptMgr::OnBeforeFinalizePlayerWorldSession(uint32& cacheVersion)
{
    FOREACH_SCRIPT(WorldScript)->OnBeforeFinalizePlayerWorldSession(cacheVersion);
}

template class TC_GAME_API ScriptRegistry<WorldScript>;
