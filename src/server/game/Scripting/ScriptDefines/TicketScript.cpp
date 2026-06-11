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

#include "TicketScript.h"
#include "ScriptMgr.h"
#include "ScriptRegistry.h"

TicketScript::TicketScript(char const* name, std::vector<uint16> /*enabledHooks*/)
    : ScriptObject(name)
{
    ScriptRegistry<TicketScript>::Instance()->AddScript(this);
}

void ScriptMgr::OnTicketCreate(GmTicket* ticket)
{
    FOREACH_SCRIPT(TicketScript)->OnTicketCreate(ticket);
}

void ScriptMgr::OnTicketUpdateLastChange(GmTicket* ticket)
{
    FOREACH_SCRIPT(TicketScript)->OnTicketUpdateLastChange(ticket);
}

void ScriptMgr::OnTicketClose(GmTicket* ticket)
{
    FOREACH_SCRIPT(TicketScript)->OnTicketClose(ticket);
}

void ScriptMgr::OnTicketStatusUpdate(GmTicket* ticket)
{
    FOREACH_SCRIPT(TicketScript)->OnTicketStatusUpdate(ticket);
}

void ScriptMgr::OnTicketResolve(GmTicket* ticket)
{
    FOREACH_SCRIPT(TicketScript)->OnTicketResolve(ticket);
}

template class TC_GAME_API ScriptRegistry<TicketScript>;
