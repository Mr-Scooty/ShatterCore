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

#ifndef SC_SERVER_SCRIPT_H
#define SC_SERVER_SCRIPT_H

#include "ScriptObject.h"
#include <memory>
#include <vector>

class WorldPacket;
class WorldSession;
class WorldSocket;

/*
 * ShatterCore keeps the TrinityCore void OnPacketSend/OnPacketReceive observer
 * hooks (working on a modifiable packet copy) alongside AzerothCore's
 * CanPacketSend/CanPacketReceive veto hooks.
 */

// AzerothCore compatible hook enum. ShatterCore dispatches through the
// script registry for every registered script - the per-hook filtering
// of AzerothCore is accepted but not implemented (see DatabaseScript.h).
enum ServerHook : uint16
{
    SERVERHOOK_ON_NETWORK_START,
    SERVERHOOK_ON_NETWORK_STOP,
    SERVERHOOK_ON_SOCKET_OPEN,
    SERVERHOOK_ON_SOCKET_CLOSE,
    SERVERHOOK_CAN_PACKET_SEND,
    SERVERHOOK_CAN_PACKET_RECEIVE,
    SERVERHOOK_ON_PACKET_RECEIVED,
    SERVERHOOK_END
};

class TC_GAME_API ServerScript : public ScriptObject
{
    protected:

        ServerScript(char const* name, std::vector<uint16> enabledHooks = std::vector<uint16>());

    public:

        // Called when reactive socket I/O is started (WorldTcpSessionMgr).
        virtual void OnNetworkStart() { }

        // Called when reactive I/O is stopped.
        virtual void OnNetworkStop() { }

        // Called when a remote socket establishes a connection to the server. Do not store the socket object.
        virtual void OnSocketOpen(std::shared_ptr<WorldSocket> /*socket*/) { }

        // Called when a socket is closed. Do not store the socket object, and do not rely on the connection
        // being open; it is not.
        virtual void OnSocketClose(std::shared_ptr<WorldSocket> /*socket*/) { }

        // Called when a packet is sent to a client. The packet object is a copy of the original packet, so reading
        // and modifying it is safe.
        virtual void OnPacketSend(WorldSession* /*session*/, WorldPacket& /*packet*/) { }

        // Called when a (valid) packet is received by a client. The packet object is a copy of the original packet, so
        // reading and modifying it is safe. Make sure to check WorldSession pointer before usage, it might be null in case of auth packets
        virtual void OnPacketReceive(WorldSession* /*session*/, WorldPacket& /*packet*/) { }

        // Called before a packet is sent to a client, returning false drops the packet
        [[nodiscard]] virtual bool CanPacketSend(WorldSession* /*session*/, WorldPacket const& /*packet*/) { return true; }

        // Called before a received (valid) packet is processed, returning false drops the packet.
        // Make sure to check WorldSession pointer before usage, it might be null in case of auth packets
        [[nodiscard]] virtual bool CanPacketReceive(WorldSession* /*session*/, WorldPacket const& /*packet*/) { return true; }

        // Called after a received packet's opcode handler has run (mod-playerbots observes
        // master-client actions here). The packet must not be modified.
        virtual void OnPacketReceived(WorldSession* /*session*/, WorldPacket const& /*packet*/) { }
};

#endif // SC_SERVER_SCRIPT_H
