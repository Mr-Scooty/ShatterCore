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

#ifndef SC_ALL_COMMAND_SCRIPT_H
#define SC_ALL_COMMAND_SCRIPT_H

#include "ScriptObject.h"
#include <string_view>
#include <vector>

/*
 * AzerothCore's OnBeforeIsInvokerVisible hook is not available: its
 * Acore::Impl::ChatCommands::CommandPermissions parameter belongs to the
 * AzerothCore command system, ShatterCore still uses the old-style
 * ChatCommand tables which have no equivalent type.
 */

class ChatHandler;
class Player;

// AzerothCore compatible hook enum. ShatterCore dispatches through the
// script registry for every registered script - the per-hook filtering
// of AzerothCore is accepted but not implemented (see DatabaseScript.h).
// Only the hooks that exist in ShatterCore are listed; the order follows
// AzerothCore's where the hook exists there.
enum AllCommandHook : uint16
{
    ALLCOMMANDHOOK_ON_HANDLE_DEV_COMMAND,
    ALLCOMMANDHOOK_ON_TRY_EXECUTE_COMMAND,
    ALLCOMMANDHOOK_END
};

class TC_GAME_API AllCommandScript : public ScriptObject
{
    protected:

        AllCommandScript(char const* name, std::vector<uint16> enabledHooks = std::vector<uint16>());

    public:

        // Called when the .dev command toggles the developer state of a player
        virtual void OnHandleDevCommand(Player* /*player*/, bool& /*enable*/) { }

        // Called when a command is parsed, but before it is executed,
        // returning false blocks the execution
        [[nodiscard]] virtual bool OnTryExecuteCommand(ChatHandler& /*handler*/, std::string_view /*cmdStr*/) { return true; }
};

// Compatibility for old scripts
using CommandSC = AllCommandScript;

#endif // SC_ALL_COMMAND_SCRIPT_H
