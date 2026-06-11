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

#include "AllBattlegroundScript.h"
#include "ScriptMgr.h"
#include "ScriptRegistry.h"

AllBattlegroundScript::AllBattlegroundScript(char const* name, std::vector<uint16> /*enabledHooks*/)
    : ScriptObject(name)
{
    ScriptRegistry<AllBattlegroundScript>::Instance()->AddScript(this);
}

void ScriptMgr::OnBattlegroundStart(Battleground* bg)
{
    ASSERT(bg);
    FOREACH_SCRIPT(AllBattlegroundScript)->OnBattlegroundStart(bg);
}

void ScriptMgr::OnBattlegroundEndReward(Battleground* bg, Player* player, uint32 winnerTeam)
{
    ASSERT(bg);
    ASSERT(player);
    FOREACH_SCRIPT(AllBattlegroundScript)->OnBattlegroundEndReward(bg, player, winnerTeam);
}

void ScriptMgr::OnBattlegroundUpdate(Battleground* bg, uint32 diff)
{
    ASSERT(bg);
    FOREACH_SCRIPT(AllBattlegroundScript)->OnBattlegroundUpdate(bg, diff);
}

void ScriptMgr::OnBattlegroundAddPlayer(Battleground* bg, Player* player)
{
    ASSERT(bg);
    ASSERT(player);
    FOREACH_SCRIPT(AllBattlegroundScript)->OnBattlegroundAddPlayer(bg, player);
}

void ScriptMgr::OnBattlegroundBeforeAddPlayer(Battleground* bg, Player* player)
{
    ASSERT(bg);
    ASSERT(player);
    FOREACH_SCRIPT(AllBattlegroundScript)->OnBattlegroundBeforeAddPlayer(bg, player);
}

void ScriptMgr::OnBattlegroundRemovePlayerAtLeave(Battleground* bg, Player* player)
{
    ASSERT(bg);
    ASSERT(player);
    FOREACH_SCRIPT(AllBattlegroundScript)->OnBattlegroundRemovePlayerAtLeave(bg, player);
}

void ScriptMgr::OnQueueUpdate(BattlegroundQueue* queue, uint32 diff, BattlegroundTypeId bgTypeId, BattlegroundBracketId bracketId, uint8 arenaType, bool isRated, uint32 arenaRating)
{
    ASSERT(queue);
    FOREACH_SCRIPT(AllBattlegroundScript)->OnQueueUpdate(queue, diff, bgTypeId, bracketId, arenaType, isRated, arenaRating);
}

bool ScriptMgr::OnQueueUpdateValidity(BattlegroundQueue* queue, uint32 diff, BattlegroundTypeId bgTypeId, BattlegroundBracketId bracketId, uint8 arenaType, bool isRated, uint32 arenaRating)
{
    FOR_SCRIPTS_RET(AllBattlegroundScript, itr, end, true)
        if (!itr->second->OnQueueUpdateValidity(queue, diff, bgTypeId, bracketId, arenaType, isRated, arenaRating))
            return false;

    return true;
}

void ScriptMgr::OnAddGroup(BattlegroundQueue* queue, GroupQueueInfo* ginfo, uint32& index, Player* leader, Group* group, BattlegroundTypeId bgTypeId, PvPDifficultyEntry const* bracketEntry,
    uint8 arenaType, bool isRated, bool isPremade, uint32 arenaRating, uint32 matchmakerRating, uint32 arenaTeamId, uint32 opponentsArenaTeamId)
{
    ASSERT(queue);
    ASSERT(ginfo);
    FOREACH_SCRIPT(AllBattlegroundScript)->OnAddGroup(queue, ginfo, index, leader, group, bgTypeId, bracketEntry, arenaType, isRated, isPremade, arenaRating, matchmakerRating, arenaTeamId, opponentsArenaTeamId);
}

bool ScriptMgr::CanFillPlayersToBG(BattlegroundQueue* queue, Battleground* bg, BattlegroundBracketId bracketId)
{
    FOR_SCRIPTS_RET(AllBattlegroundScript, itr, end, true)
        if (!itr->second->CanFillPlayersToBG(queue, bg, bracketId))
            return false;

    return true;
}

bool ScriptMgr::IsCheckNormalMatch(BattlegroundQueue* queue, Battleground* bgTemplate, BattlegroundBracketId bracketId, uint32 minPlayers, uint32 maxPlayers)
{
    FOR_SCRIPTS_RET(AllBattlegroundScript, itr, end, false)
        if (itr->second->IsCheckNormalMatch(queue, bgTemplate, bracketId, minPlayers, maxPlayers))
            return true;

    return false;
}

bool ScriptMgr::CanSendMessageBGQueue(BattlegroundQueue* queue, Player* leader, Battleground* bg, PvPDifficultyEntry const* bracketEntry)
{
    FOR_SCRIPTS_RET(AllBattlegroundScript, itr, end, true)
        if (!itr->second->CanSendMessageBGQueue(queue, leader, bg, bracketEntry))
            return false;

    return true;
}

bool ScriptMgr::OnBeforeSendJoinMessageArenaQueue(BattlegroundQueue* queue, Player* leader, GroupQueueInfo* ginfo, PvPDifficultyEntry const* bracketEntry, bool isRated)
{
    FOR_SCRIPTS_RET(AllBattlegroundScript, itr, end, true)
        if (!itr->second->OnBeforeSendJoinMessageArenaQueue(queue, leader, ginfo, bracketEntry, isRated))
            return false;

    return true;
}

bool ScriptMgr::OnBeforeSendExitMessageArenaQueue(BattlegroundQueue* queue, GroupQueueInfo* ginfo)
{
    FOR_SCRIPTS_RET(AllBattlegroundScript, itr, end, true)
        if (!itr->second->OnBeforeSendExitMessageArenaQueue(queue, ginfo))
            return false;

    return true;
}

void ScriptMgr::OnBattlegroundEnd(Battleground* bg, uint32 winnerTeam)
{
    ASSERT(bg);
    FOREACH_SCRIPT(AllBattlegroundScript)->OnBattlegroundEnd(bg, winnerTeam);
}

void ScriptMgr::OnBattlegroundDestroy(Battleground* bg)
{
    ASSERT(bg);
    FOREACH_SCRIPT(AllBattlegroundScript)->OnBattlegroundDestroy(bg);
}

void ScriptMgr::OnBattlegroundCreate(Battleground* bg)
{
    ASSERT(bg);
    FOREACH_SCRIPT(AllBattlegroundScript)->OnBattlegroundCreate(bg);
}

bool ScriptMgr::CanAddGroupToMatchingPool(BattlegroundQueue* queue, GroupQueueInfo* group, uint32 poolPlayerCount, Battleground* bg, BattlegroundBracketId bracketId)
{
    FOR_SCRIPTS_RET(AllBattlegroundScript, itr, end, true)
        if (!itr->second->CanAddGroupToMatchingPool(queue, group, poolPlayerCount, bg, bracketId))
            return false;

    return true;
}

bool ScriptMgr::GetPlayerMatchmakingRating(ObjectGuid playerGuid, BattlegroundTypeId bgTypeId, float& outRating)
{
    FOR_SCRIPTS_RET(AllBattlegroundScript, itr, end, false)
        if (itr->second->GetPlayerMatchmakingRating(playerGuid, bgTypeId, outRating))
            return true;

    return false;
}

template class TC_GAME_API ScriptRegistry<AllBattlegroundScript>;
