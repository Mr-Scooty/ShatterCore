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

#ifndef SC_ARENA_SCRIPT_H
#define SC_ARENA_SCRIPT_H

#include "ObjectGuid.h"
#include "ScriptObject.h"
#include <vector>

/*
 * AzerothCore's OnGetPoints hook is not available: 4.3.4 removed arena
 * points entirely (arenas reward conquest points through the currency
 * system), so ArenaTeam::GetPoints does not exist in ShatterCore.
 *
 * OnBeforeArenaTeamMemberUpdate defaults to true (allow the update) where
 * AzerothCore defaults to false. AzerothCore relies on its per-hook enable
 * filtering to skip scripts that do not override the hook; ShatterCore
 * dispatches every registered script, so a true default is required to
 * keep the vanilla rating updates working.
 */

class ArenaTeam;
class Battleground;
class Player;

// AzerothCore compatible hook enum. ShatterCore dispatches through the
// script registry for every registered script - the per-hook filtering
// of AzerothCore is accepted but not implemented (see DatabaseScript.h).
// Only the hooks that exist in ShatterCore are listed; the order follows
// AzerothCore's where the hook exists there.
enum ArenaHook : uint16
{
    ARENAHOOK_CAN_ADD_MEMBER,
    ARENAHOOK_CAN_SAVE_TO_DB,
    ARENAHOOK_ON_BEFORE_CHECK_WIN_CONDITION,
    ARENAHOOK_ON_ARENA_START,
    ARENAHOOK_ON_BEFORE_TEAM_MEMBER_UPDATE,
    ARENAHOOK_CAN_SAVE_ARENA_STATS_FOR_MEMBER,
    ARENAHOOK_END
};

class TC_GAME_API ArenaScript : public ScriptObject
{
    protected:

        ArenaScript(char const* name, std::vector<uint16> enabledHooks = std::vector<uint16>());

    public:

        // Called when a member is about to be added to an arena team,
        // returning false denies the join
        [[nodiscard]] virtual bool CanAddMember(ArenaTeam* /*team*/, ObjectGuid /*playerGuid*/) { return true; }

        // Called at the top of ArenaTeam::SaveToDB, returning false skips saving
        [[nodiscard]] virtual bool CanSaveToDB(ArenaTeam* /*team*/) { return true; }

        // Called at the top of Arena::CheckWinConditions, returning false
        // skips the core win condition check
        [[nodiscard]] virtual bool OnBeforeArenaCheckWinConditions(Battleground* /*bg*/) { return true; }

        // Called when the arena doors open
        virtual void OnArenaStart(Battleground* /*bg*/) { }

        // Called before the rating of an arena team member is updated at the
        // end of a rated match, returning false skips the update
        [[nodiscard]] virtual bool OnBeforeArenaTeamMemberUpdate(ArenaTeam* /*team*/, Player* /*player*/, bool /*won*/, uint32 /*opponentMatchmakerRating*/, int32 /*matchmakerChange*/) { return true; }

        // Called before the matchmaker rating of an arena team member is saved,
        // returning false skips saving the character arena stats
        [[nodiscard]] virtual bool CanSaveArenaStatsForMember(ArenaTeam* /*team*/, ObjectGuid /*playerGuid*/) { return true; }
};

#endif // SC_ARENA_SCRIPT_H
