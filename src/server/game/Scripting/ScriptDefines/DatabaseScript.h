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

#ifndef SC_DATABASE_SCRIPT_H
#define SC_DATABASE_SCRIPT_H

#include "ScriptObject.h"
#include <vector>

// AzerothCore compatible hook enum. ShatterCore dispatches through the
// script registry for every registered script - the per-hook filtering
// of AzerothCore is accepted but not implemented (see PlayerScript.h).
enum DatabaseHook : uint16
{
    DATABASEHOOK_ON_AFTER_DATABASES_LOADED,
    DATABASEHOOK_ON_DATABASES_LOADING,
    DATABASEHOOK_ON_DATABASES_KEEP_ALIVE,
    DATABASEHOOK_ON_DATABASES_CLOSING,
    DATABASEHOOK_ON_DATABASE_WARN_ABOUT_SYNC_QUERIES,
    DATABASEHOOK_ON_DATABASE_SELECT_INDEX_LOGOUT,
    DATABASEHOOK_ON_DATABASE_GET_DB_REVISION,
    DATABASEHOOK_END
};

class Player;

class TC_GAME_API DatabaseScript : public ScriptObject
{
    protected:

        DatabaseScript(char const* name, std::vector<uint16> enabledHooks = std::vector<uint16>());

    public:

        // Called after all databases are loaded
        virtual void OnAfterDatabasesLoaded(uint32 /*updateFlags*/) { }

        // Called while the worldserver opens its database pools; lets a module open
        // an additional pool (mod-playerbots). Returning false aborts server startup.
        [[nodiscard]] virtual bool OnDatabasesLoading() { return true; }

        // Called alongside the core pools' KeepAlive ping.
        virtual void OnDatabasesKeepAlive() { }

        // Called while the worldserver closes its database pools.
        virtual void OnDatabasesClosing() { }

        // Called around long synchronous stretches to toggle sync-query warnings.
        virtual void OnDatabaseWarnAboutSyncQueries(bool /*apply*/) { }

        // Lets a module override which statement marks a character offline on logout
        // (bots must not mark the whole account's characters offline).
        virtual void OnDatabaseSelectIndexLogout(Player* /*player*/, uint32& /*statementIndex*/, uint32& /*statementParam*/) { }

        // Lets a module report its database revision for server info output.
        virtual void OnDatabaseGetDBRevision(std::string& /*revision*/) { }
};

#endif // SC_DATABASE_SCRIPT_H
