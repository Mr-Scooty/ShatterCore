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

#ifndef SC_ALL_BATTLEGROUND_SCRIPT_H
#define SC_ALL_BATTLEGROUND_SCRIPT_H

#include "DBCEnums.h"
#include "ObjectGuid.h"
#include "ScriptObject.h"
#include <vector>

/*
 * ShatterCore notes (4.3.4 vs AzerothCore's 3.3.5a):
 *  - Winner team arguments are uint32 team values (ALLIANCE/HORDE, 0 for a
 *    draw) where AzerothCore uses its TeamId enum, following how the 4.3.4
 *    core passes the winner through Battleground::EndBattleground.
 *  - OnAddGroup always receives 0 for opponentsArenaTeamId, the 4.3.4 queue
 *    does not track the previous opponents of an arena team.
 */

class Battleground;
class BattlegroundQueue;
class Group;
class Player;

enum BattlegroundTypeId : uint32;

struct GroupQueueInfo;
struct PvPDifficultyEntry;

// AzerothCore compatible hook enum. ShatterCore dispatches through the
// script registry for every registered script - the per-hook filtering
// of AzerothCore is accepted but not implemented (see DatabaseScript.h).
enum AllBattlegroundHook : uint16
{
    ALLBATTLEGROUNDHOOK_ON_BATTLEGROUND_START,
    ALLBATTLEGROUNDHOOK_ON_BATTLEGROUND_END_REWARD,
    ALLBATTLEGROUNDHOOK_ON_BATTLEGROUND_UPDATE,
    ALLBATTLEGROUNDHOOK_ON_BATTLEGROUND_ADD_PLAYER,
    ALLBATTLEGROUNDHOOK_ON_BATTLEGROUND_BEFORE_ADD_PLAYER,
    ALLBATTLEGROUNDHOOK_ON_BATTLEGROUND_REMOVE_PLAYER_AT_LEAVE,
    ALLBATTLEGROUNDHOOK_ON_QUEUE_UPDATE,
    ALLBATTLEGROUNDHOOK_ON_QUEUE_UPDATE_VALIDITY,
    ALLBATTLEGROUNDHOOK_ON_ADD_GROUP,
    ALLBATTLEGROUNDHOOK_CAN_FILL_PLAYERS_TO_BG,
    ALLBATTLEGROUNDHOOK_IS_CHECK_NORMAL_MATCH,
    ALLBATTLEGROUNDHOOK_CAN_SEND_MESSAGE_BG_QUEUE,
    ALLBATTLEGROUNDHOOK_ON_BEFORE_SEND_JOIN_MESSAGE_ARENA_QUEUE,
    ALLBATTLEGROUNDHOOK_ON_BEFORE_SEND_EXIT_MESSAGE_ARENA_QUEUE,
    ALLBATTLEGROUNDHOOK_ON_BATTLEGROUND_END,
    ALLBATTLEGROUNDHOOK_ON_BATTLEGROUND_DESTROY,
    ALLBATTLEGROUNDHOOK_ON_BATTLEGROUND_CREATE,
    ALLBATTLEGROUNDHOOK_CAN_ADD_GROUP_TO_MATCHING_POOL,
    ALLBATTLEGROUNDHOOK_GET_PLAYER_MATCHMAKING_RATING,
    ALLBATTLEGROUNDHOOK_END
};

class TC_GAME_API AllBattlegroundScript : public ScriptObject
{
    protected:

        AllBattlegroundScript(char const* name, std::vector<uint16> enabledHooks = std::vector<uint16>());

    public:

        // Called when a battleground leaves the preparation phase and the doors open
        virtual void OnBattlegroundStart(Battleground* /*bg*/) { }

        // Called for every participant when a battleground ends, after the honor
        // and currency rewards have been handed out (winnerTeam is ALLIANCE, HORDE or 0)
        virtual void OnBattlegroundEndReward(Battleground* /*bg*/, Player* /*player*/, uint32 /*winnerTeam*/) { }

        // Called at the end of every Battleground::Update tick
        virtual void OnBattlegroundUpdate(Battleground* /*bg*/, uint32 /*diff*/) { }

        // Called at the end of Battleground::AddPlayer
        virtual void OnBattlegroundAddPlayer(Battleground* /*bg*/, Player* /*player*/) { }

        // Called at the top of Battleground::AddPlayer
        virtual void OnBattlegroundBeforeAddPlayer(Battleground* /*bg*/, Player* /*player*/) { }

        // Called when a player is removed from a battleground at leave
        virtual void OnBattlegroundRemovePlayerAtLeave(Battleground* /*bg*/, Player* /*player*/) { }

        // Called during every battleground queue update for the processed
        // queue/bracket combination
        virtual void OnQueueUpdate(BattlegroundQueue* /*queue*/, uint32 /*diff*/, BattlegroundTypeId /*bgTypeId*/, BattlegroundBracketId /*bracketId*/, uint8 /*arenaType*/, bool /*isRated*/, uint32 /*arenaRating*/) { }

        // Called during every battleground queue update, returning false skips
        // the matchmaking part of the update
        [[nodiscard]] virtual bool OnQueueUpdateValidity(BattlegroundQueue* /*queue*/, uint32 /*diff*/, BattlegroundTypeId /*bgTypeId*/, BattlegroundBracketId /*bracketId*/, uint8 /*arenaType*/, bool /*isRated*/, uint32 /*arenaRating*/) { return true; }

        // Called when a group is added to a battleground queue, after the queue
        // index has been computed (index may be changed)
        virtual void OnAddGroup(BattlegroundQueue* /*queue*/, GroupQueueInfo* /*ginfo*/, uint32& /*index*/, Player* /*leader*/, Group* /*group*/, BattlegroundTypeId /*bgTypeId*/, PvPDifficultyEntry const* /*bracketEntry*/,
            uint8 /*arenaType*/, bool /*isRated*/, bool /*isPremade*/, uint32 /*arenaRating*/, uint32 /*matchmakerRating*/, uint32 /*arenaTeamId*/, uint32 /*opponentsArenaTeamId*/) { }

        // Called at the top of BattlegroundQueue::FillPlayersToBG, returning
        // false skips filling the selection pools for the given battleground
        [[nodiscard]] virtual bool CanFillPlayersToBG(BattlegroundQueue* /*queue*/, Battleground* /*bg*/, BattlegroundBracketId /*bracketId*/) { return true; }

        // Called at the top of BattlegroundQueue::CheckNormalMatch, returning
        // true replaces the core matchmaking logic (only the final player count
        // check is performed on the already filled selection pools)
        [[nodiscard]] virtual bool IsCheckNormalMatch(BattlegroundQueue* /*queue*/, Battleground* /*bgTemplate*/, BattlegroundBracketId /*bracketId*/, uint32 /*minPlayers*/, uint32 /*maxPlayers*/) { return false; }

        // Called before the battleground queue status is announced to the
        // joining player or the world, returning false disables the message
        [[nodiscard]] virtual bool CanSendMessageBGQueue(BattlegroundQueue* /*queue*/, Player* /*leader*/, Battleground* /*bg*/, PvPDifficultyEntry const* /*bracketEntry*/) { return true; }

        // Called before the arena queue join announcement is sent,
        // returning false disables the message
        [[nodiscard]] virtual bool OnBeforeSendJoinMessageArenaQueue(BattlegroundQueue* /*queue*/, Player* /*leader*/, GroupQueueInfo* /*ginfo*/, PvPDifficultyEntry const* /*bracketEntry*/, bool /*isRated*/) { return true; }

        // Called before the arena queue exit announcement is sent,
        // returning false disables the message
        [[nodiscard]] virtual bool OnBeforeSendExitMessageArenaQueue(BattlegroundQueue* /*queue*/, GroupQueueInfo* /*ginfo*/) { return true; }

        // Called at the end of Battleground::EndBattleground
        // (winnerTeam is ALLIANCE, HORDE or 0 for a draw)
        virtual void OnBattlegroundEnd(Battleground* /*bg*/, uint32 /*winnerTeam*/) { }

        // Called at the top of the Battleground destructor
        virtual void OnBattlegroundDestroy(Battleground* /*bg*/) { }

        // Called when a battleground instance is added to the battleground store
        virtual void OnBattlegroundCreate(Battleground* /*bg*/) { }

        // Called before a group is added to a battleground matching selection
        // pool, returning false skips the group (bg may be nullptr for the
        // premade and skirmish checks)
        [[nodiscard]] virtual bool CanAddGroupToMatchingPool(BattlegroundQueue* /*queue*/, GroupQueueInfo* /*group*/, uint32 /*poolPlayerCount*/, Battleground* /*bg*/, BattlegroundBracketId /*bracketId*/) { return true; }

        // Module facing helper without core call site: modules implementing a
        // custom battleground MMR system may answer rating queries of other
        // modules through sScriptMgr->GetPlayerMatchmakingRating, returning
        // true if a rating was provided
        [[nodiscard]] virtual bool GetPlayerMatchmakingRating(ObjectGuid /*playerGuid*/, BattlegroundTypeId /*bgTypeId*/, float& /*outRating*/) { return false; }
};

// Compatibility for old scripts
using BGScript = AllBattlegroundScript;

#endif // SC_ALL_BATTLEGROUND_SCRIPT_H
