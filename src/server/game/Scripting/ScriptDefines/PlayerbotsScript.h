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

#ifndef SC_PLAYERBOTS_SCRIPT_H
#define SC_PLAYERBOTS_SCRIPT_H

#include "ObjectGuid.h"
#include "ScriptObject.h"
#include <vector>

class Player;
class Unit;
class WorldPacket;

/*
 * Hook surface for the mod-playerbots module, ported from the
 * liyunfan1223/azerothcore-wotlk Playerbot fork. The AzerothCore fork passes
 * lfg::Lfg5Guids to OnPlayerbotCheckLFGQueue; ShatterCore's LFG queue works on
 * the global GuidList, so that is used here instead.
 */

// AzerothCore compatible hook enum. ShatterCore dispatches through the
// script registry for every registered script - the per-hook filtering
// of AzerothCore is accepted but not implemented (see DatabaseScript.h).
enum PlayerbotHook : uint16
{
    PLAYERBOTHOOK_ON_PLAYERBOT_UPDATE,
    PLAYERBOTHOOK_ON_PLAYERBOT_UPDATE_SESSIONS,
    PLAYERBOTHOOK_ON_PLAYERBOT_LOGOUT,
    PLAYERBOTHOOK_ON_PLAYERBOT_LOGOUT_BOTS,
    PLAYERBOTHOOK_ON_PLAYERBOT_PACKET_SENT,
    PLAYERBOTHOOK_ON_PLAYERBOT_CHECK_LFG_QUEUE,
    PLAYERBOTHOOK_ON_PLAYERBOT_CHECK_KILL_TASK,
    PLAYERBOTHOOK_ON_PLAYERBOT_CHECK_PETITION_ACCOUNT,
    PLAYERBOTHOOK_ON_PLAYERBOT_CHECK_UPDATES_TO_SEND,
    PLAYERBOTHOOK_END
};

class TC_GAME_API PlayerbotScript : public ScriptObject
{
    protected:

        PlayerbotScript(char const* name, std::vector<uint16> enabledHooks = std::vector<uint16>());

    public:

        // Called from LFGQueue before a proposal is created; returning false rejects the group composition.
        [[nodiscard]] virtual bool OnPlayerbotCheckLFGQueue(GuidList const& /*guidsList*/) { return true; }

        // Called after a player got kill credit for a victim (guild task bookkeeping).
        virtual void OnPlayerbotCheckKillTask(Player* /*player*/, Unit* /*victim*/) { }

        // Called when petition signatures are checked per-account so linked bot accounts can be allowed.
        virtual void OnPlayerbotCheckPetitionAccount(Player* /*player*/, bool& /*found*/) { }

        // Called from Map::SendObjectUpdates; returning false skips building update packets for this player.
        [[nodiscard]] virtual bool OnPlayerbotCheckUpdatesToSend(Player* /*player*/) { return true; }

        // Called from WorldSession::SendPacket for every outgoing packet (bots have no socket; they sniff here).
        virtual void OnPlayerbotPacketSent(Player* /*player*/, WorldPacket const* /*packet*/) { }

        // Called once per World::Update tick.
        virtual void OnPlayerbotUpdate(uint32 /*diff*/) { }

        // Called when a session's packets are processed so the bot AI can pump its own queue.
        virtual void OnPlayerbotUpdateSessions(Player* /*player*/) { }

        // Called near the start of WorldSession::LogoutPlayer.
        virtual void OnPlayerbotLogout(Player* /*player*/) { }

        // Called from World::KickAll so all bots get logged out on shutdown.
        virtual void OnPlayerbotLogoutBots() { }
};

#endif // SC_PLAYERBOTS_SCRIPT_H
