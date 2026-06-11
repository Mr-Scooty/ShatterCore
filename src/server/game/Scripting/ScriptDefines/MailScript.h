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

#ifndef SC_MAIL_SCRIPT_H
#define SC_MAIL_SCRIPT_H

#include "ScriptObject.h"
#include <vector>

class MailDraft;
class MailReceiver;
class MailSender;

enum MailCheckMask : uint8;

// AzerothCore compatible hook enum. ShatterCore dispatches through the
// script registry for every registered script - the per-hook filtering
// of AzerothCore is accepted but not implemented (see DatabaseScript.h).
enum MailHook : uint16
{
    MAILHOOK_ON_BEFORE_MAIL_DRAFT_SEND_MAIL_TO,
    MAILHOOK_END
};

class TC_GAME_API MailScript : public ScriptObject
{
    protected:

        MailScript(char const* name, std::vector<uint16> enabledHooks = std::vector<uint16>());

    public:

        // Called before mail is sent. Setting deleteMailItemsFromDB deletes
        // the attached items from the database, clearing sendMail skips the
        // mail, custom_expiration overrides the expiration time (in days).
        virtual void OnBeforeMailDraftSendMailTo(MailDraft* /*mailDraft*/, MailReceiver const& /*receiver*/, MailSender const& /*sender*/, MailCheckMask& /*checked*/, uint32& /*deliver_delay*/, uint32& /*custom_expiration*/, bool& /*deleteMailItemsFromDB*/, bool& /*sendMail*/) { }
};

#endif // SC_MAIL_SCRIPT_H
