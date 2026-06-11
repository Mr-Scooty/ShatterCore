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
    DATABASEHOOK_END
};

class TC_GAME_API DatabaseScript : public ScriptObject
{
    protected:

        DatabaseScript(char const* name, std::vector<uint16> enabledHooks = std::vector<uint16>());

    public:

        // Called after all databases are loaded
        virtual void OnAfterDatabasesLoaded(uint32 /*updateFlags*/) { }
};

#endif // SC_DATABASE_SCRIPT_H
