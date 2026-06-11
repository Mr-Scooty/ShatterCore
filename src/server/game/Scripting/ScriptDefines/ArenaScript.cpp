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

#include "ArenaScript.h"
#include "ScriptMgr.h"
#include "ScriptRegistry.h"

ArenaScript::ArenaScript(char const* name, std::vector<uint16> /*enabledHooks*/)
    : ScriptObject(name)
{
    ScriptRegistry<ArenaScript>::Instance()->AddScript(this);
}

bool ScriptMgr::CanAddMember(ArenaTeam* team, ObjectGuid playerGuid)
{
    ASSERT(team);
    FOR_SCRIPTS_RET(ArenaScript, itr, end, true)
        if (!itr->second->CanAddMember(team, playerGuid))
            return false;

    return true;
}

bool ScriptMgr::CanSaveToDB(ArenaTeam* team)
{
    ASSERT(team);
    FOR_SCRIPTS_RET(ArenaScript, itr, end, true)
        if (!itr->second->CanSaveToDB(team))
            return false;

    return true;
}

bool ScriptMgr::OnBeforeArenaCheckWinConditions(Battleground* bg)
{
    ASSERT(bg);
    FOR_SCRIPTS_RET(ArenaScript, itr, end, true)
        if (!itr->second->OnBeforeArenaCheckWinConditions(bg))
            return false;

    return true;
}

void ScriptMgr::OnArenaStart(Battleground* bg)
{
    ASSERT(bg);
    FOREACH_SCRIPT(ArenaScript)->OnArenaStart(bg);
}

bool ScriptMgr::OnBeforeArenaTeamMemberUpdate(ArenaTeam* team, Player* player, bool won, uint32 opponentMatchmakerRating, int32 matchmakerChange)
{
    FOR_SCRIPTS_RET(ArenaScript, itr, end, true)
        if (!itr->second->OnBeforeArenaTeamMemberUpdate(team, player, won, opponentMatchmakerRating, matchmakerChange))
            return false;

    return true;
}

bool ScriptMgr::CanSaveArenaStatsForMember(ArenaTeam* team, ObjectGuid playerGuid)
{
    ASSERT(team);
    FOR_SCRIPTS_RET(ArenaScript, itr, end, true)
        if (!itr->second->CanSaveArenaStatsForMember(team, playerGuid))
            return false;

    return true;
}

template class TC_GAME_API ScriptRegistry<ArenaScript>;
