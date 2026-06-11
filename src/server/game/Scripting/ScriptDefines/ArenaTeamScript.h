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

#ifndef SC_ARENA_TEAM_SCRIPT_H
#define SC_ARENA_TEAM_SCRIPT_H

#include "ScriptObject.h"
#include "SharedDefines.h"
#include <vector>

/*
 * AzerothCore's OnGetArenaPoints hook is not available: 4.3.4 removed arena
 * points entirely (arenas reward conquest points through the currency
 * system), so ArenaTeam::GetPoints does not exist in ShatterCore.
 */

// AzerothCore compatible hook enum. ShatterCore dispatches through the
// script registry for every registered script - the per-hook filtering
// of AzerothCore is accepted but not implemented (see DatabaseScript.h).
// Only the hooks that exist in ShatterCore are listed; the order follows
// AzerothCore's where the hook exists there.
enum ArenaTeamHook : uint16
{
    ARENATEAMHOOK_ON_GET_SLOT_BY_TYPE,
    ARENATEAMHOOK_ON_TYPEID_TO_QUEUEID,
    ARENATEAMHOOK_ON_QUEUEID_TO_ARENA_TYPE,
    ARENATEAMHOOK_ON_SET_ARENA_MAX_PLAYERS_PER_TEAM,
    ARENATEAMHOOK_END
};

class TC_GAME_API ArenaTeamScript : public ScriptObject
{
    protected:

        ArenaTeamScript(char const* name, std::vector<uint16> enabledHooks = std::vector<uint16>());

    public:

        // Called when an arena team type is mapped to its team slot,
        // the slot may be changed (allows custom arena team types)
        virtual void OnGetSlotByType(uint32 /*type*/, uint8& /*slot*/) { }

        // Called when an arena battleground type and arena type are mapped to
        // a battleground queue type id, the queue type id may be changed
        virtual void OnTypeIDToQueueID(BattlegroundTypeId /*bgTypeId*/, uint8 /*arenaType*/, uint32& /*queueTypeID*/) { }

        // Called when a battleground queue type id is mapped back to its arena
        // type, the arena type may be changed
        virtual void OnQueueIdToArenaType(BattlegroundQueueTypeId /*bgQueueTypeId*/, uint8& /*arenaType*/) { }

        // Called when the maximum players per team of a new arena are
        // determined, the count may be changed
        virtual void OnSetArenaMaxPlayersPerTeam(uint8 /*arenaType*/, uint32& /*maxPlayersPerTeam*/) { }
};

#endif // SC_ARENA_TEAM_SCRIPT_H
