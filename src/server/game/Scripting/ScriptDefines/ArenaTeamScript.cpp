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

#include "ArenaTeamScript.h"
#include "ScriptMgr.h"
#include "ScriptRegistry.h"

ArenaTeamScript::ArenaTeamScript(char const* name, std::vector<uint16> /*enabledHooks*/)
    : ScriptObject(name)
{
    ScriptRegistry<ArenaTeamScript>::Instance()->AddScript(this);
}

void ScriptMgr::OnGetSlotByType(uint32 type, uint8& slot)
{
    FOREACH_SCRIPT(ArenaTeamScript)->OnGetSlotByType(type, slot);
}

void ScriptMgr::OnArenaTypeIDToQueueID(BattlegroundTypeId bgTypeId, uint8 arenaType, uint32& queueTypeID)
{
    FOREACH_SCRIPT(ArenaTeamScript)->OnTypeIDToQueueID(bgTypeId, arenaType, queueTypeID);
}

void ScriptMgr::OnArenaQueueIdToArenaType(BattlegroundQueueTypeId bgQueueTypeId, uint8& arenaType)
{
    FOREACH_SCRIPT(ArenaTeamScript)->OnQueueIdToArenaType(bgQueueTypeId, arenaType);
}

void ScriptMgr::OnSetArenaMaxPlayersPerTeam(uint8 arenaType, uint32& maxPlayersPerTeam)
{
    FOREACH_SCRIPT(ArenaTeamScript)->OnSetArenaMaxPlayersPerTeam(arenaType, maxPlayersPerTeam);
}

template class TC_GAME_API ScriptRegistry<ArenaTeamScript>;
