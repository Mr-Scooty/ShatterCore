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

#include "ServerScript.h"
#include "ScriptMgr.h"
#include "ScriptRegistry.h"
#include "WorldPacket.h"

ServerScript::ServerScript(char const* name, std::vector<uint16> /*enabledHooks*/)
    : ScriptObject(name)
{
    ScriptRegistry<ServerScript>::Instance()->AddScript(this);
}

void ScriptMgr::OnNetworkStart()
{
    FOREACH_SCRIPT(ServerScript)->OnNetworkStart();
}

void ScriptMgr::OnNetworkStop()
{
    FOREACH_SCRIPT(ServerScript)->OnNetworkStop();
}

void ScriptMgr::OnSocketOpen(std::shared_ptr<WorldSocket> socket)
{
    ASSERT(socket);

    FOREACH_SCRIPT(ServerScript)->OnSocketOpen(socket);
}

void ScriptMgr::OnSocketClose(std::shared_ptr<WorldSocket> socket)
{
    ASSERT(socket);

    FOREACH_SCRIPT(ServerScript)->OnSocketClose(socket);
}

void ScriptMgr::OnPacketReceive(WorldSession* session, WorldPacket const& packet)
{
    if (SCR_REG_LST(ServerScript).empty())
        return;

    WorldPacket copy(packet);
    FOREACH_SCRIPT(ServerScript)->OnPacketReceive(session, copy);
}

void ScriptMgr::OnPacketSend(WorldSession* session, WorldPacket const& packet)
{
    ASSERT(session);

    if (SCR_REG_LST(ServerScript).empty())
        return;

    WorldPacket copy(packet);
    FOREACH_SCRIPT(ServerScript)->OnPacketSend(session, copy);
}

bool ScriptMgr::CanPacketSend(WorldSession* session, WorldPacket const& packet)
{
    ASSERT(session);

    FOR_SCRIPTS_RET(ServerScript, itr, end, true)
        if (!itr->second->CanPacketSend(session, packet))
            return false;

    return true;
}

bool ScriptMgr::CanPacketReceive(WorldSession* session, WorldPacket const& packet)
{
    FOR_SCRIPTS_RET(ServerScript, itr, end, true)
        if (!itr->second->CanPacketReceive(session, packet))
            return false;

    return true;
}

void ScriptMgr::OnPacketReceived(WorldSession* session, WorldPacket const& packet)
{
    FOREACH_SCRIPT(ServerScript)->OnPacketReceived(session, packet);
}

template class TC_GAME_API ScriptRegistry<ServerScript>;
