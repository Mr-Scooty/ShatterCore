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

#ifndef SC_TICKET_SCRIPT_H
#define SC_TICKET_SCRIPT_H

#include "ScriptObject.h"
#include <vector>

class GmTicket;

// AzerothCore compatible hook enum. ShatterCore dispatches through the
// script registry for every registered script - the per-hook filtering
// of AzerothCore is accepted but not implemented (see DatabaseScript.h).
enum TicketHook : uint16
{
    TICKETHOOK_ON_TICKET_CREATE,
    TICKETHOOK_ON_TICKET_UPDATE_LAST_CHANGE,
    TICKETHOOK_ON_TICKET_CLOSE,
    TICKETHOOK_ON_TICKET_STATUS_UPDATE,
    TICKETHOOK_ON_TICKET_RESOLVE,
    TICKETHOOK_END
};

class TC_GAME_API TicketScript : public ScriptObject
{
    protected:

        TicketScript(char const* name, std::vector<uint16> enabledHooks = std::vector<uint16>());

    public:

        // Called in TicketMgr::AddTicket when a new ticket is created
        virtual void OnTicketCreate(GmTicket* /*ticket*/) { }

        // Called whenever a ticket modification updates the ticket mgr last change time
        virtual void OnTicketUpdateLastChange(GmTicket* /*ticket*/) { }

        // Called in TicketMgr::CloseTicket
        virtual void OnTicketClose(GmTicket* /*ticket*/) { }

        // Reserved by AzerothCore, currently never called
        virtual void OnTicketStatusUpdate(GmTicket* /*ticket*/) { }

        // Called in TicketMgr::ResolveAndCloseTicket
        virtual void OnTicketResolve(GmTicket* /*ticket*/) { }
};

#endif // SC_TICKET_SCRIPT_H
