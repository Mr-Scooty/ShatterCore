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

#include "DatabaseScript.h"
#include "ScriptMgr.h"
#include "ScriptRegistry.h"

DatabaseScript::DatabaseScript(char const* name, std::vector<uint16> /*enabledHooks*/)
    : ScriptObject(name)
{
    ScriptRegistry<DatabaseScript>::Instance()->AddScript(this);
}

void ScriptMgr::OnAfterDatabasesLoaded(uint32 updateFlags)
{
    FOREACH_SCRIPT(DatabaseScript)->OnAfterDatabasesLoaded(updateFlags);
}

bool ScriptMgr::OnDatabasesLoading()
{
    FOR_SCRIPTS_RET(DatabaseScript, itr, end, true)
        if (!itr->second->OnDatabasesLoading())
            return false;

    return true;
}

void ScriptMgr::OnDatabasesKeepAlive()
{
    FOREACH_SCRIPT(DatabaseScript)->OnDatabasesKeepAlive();
}

void ScriptMgr::OnDatabasesClosing()
{
    FOREACH_SCRIPT(DatabaseScript)->OnDatabasesClosing();
}

void ScriptMgr::OnDatabaseWarnAboutSyncQueries(bool apply)
{
    FOREACH_SCRIPT(DatabaseScript)->OnDatabaseWarnAboutSyncQueries(apply);
}

void ScriptMgr::OnDatabaseSelectIndexLogout(Player* player, uint32& statementIndex, uint32& statementParam)
{
    FOREACH_SCRIPT(DatabaseScript)->OnDatabaseSelectIndexLogout(player, statementIndex, statementParam);
}

void ScriptMgr::OnDatabaseGetDBRevision(std::string& revision)
{
    FOREACH_SCRIPT(DatabaseScript)->OnDatabaseGetDBRevision(revision);
}

template class TC_GAME_API ScriptRegistry<DatabaseScript>;
