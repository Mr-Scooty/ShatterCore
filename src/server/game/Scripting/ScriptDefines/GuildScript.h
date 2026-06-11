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

#ifndef SC_GUILD_SCRIPT_H
#define SC_GUILD_SCRIPT_H

#include "ObjectGuid.h"
#include "ScriptObject.h"
#include <string>
#include <vector>

class Guild;
class Item;
class Player;
class WorldSession;

/*
 * ShatterCore keeps its 4.3.4 signature types where they are richer than
 * AzerothCore's 3.3.5a ones (guild bank money amounts are uint64).
 */

// AzerothCore compatible hook enum. ShatterCore dispatches through the
// script registry for every registered script - the per-hook filtering
// of AzerothCore is accepted but not implemented (see DatabaseScript.h).
enum GuildHook : uint16
{
    GUILDHOOK_ON_ADD_MEMBER,
    GUILDHOOK_ON_REMOVE_MEMBER,
    GUILDHOOK_ON_MOTD_CHANGED,
    GUILDHOOK_ON_INFO_CHANGED,
    GUILDHOOK_ON_CREATE,
    GUILDHOOK_ON_DISBAND,
    GUILDHOOK_ON_MEMBER_WITDRAW_MONEY,
    GUILDHOOK_ON_MEMBER_DEPOSIT_MONEY,
    GUILDHOOK_ON_ITEM_MOVE,
    GUILDHOOK_ON_EVENT,
    GUILDHOOK_ON_BANK_EVENT,
    GUILDHOOK_CAN_GUILD_SEND_BANK_LIST,
    GUILDHOOK_END
};

class TC_GAME_API GuildScript : public ScriptObject
{
    protected:

        GuildScript(char const* name, std::vector<uint16> enabledHooks = std::vector<uint16>());

    public:

        // Called when a member is added to the guild.
        virtual void OnAddMember(Guild* /*guild*/, Player* /*player*/, uint8& /*plRank*/) { }

        // Called when a member is removed from the guild.
        virtual void OnRemoveMember(Guild* /*guild*/, Player* /*player*/, bool /*isDisbanding*/, bool /*isKicked*/) { }

        // Called when the guild MOTD (message of the day) changes.
        virtual void OnMOTDChanged(Guild* /*guild*/, const std::string& /*newMotd*/) { }

        // Called when the guild info is altered.
        virtual void OnInfoChanged(Guild* /*guild*/, const std::string& /*newInfo*/) { }

        // Called when a guild is created.
        virtual void OnCreate(Guild* /*guild*/, Player* /*leader*/, const std::string& /*name*/) { }

        // Called when a guild is disbanded.
        virtual void OnDisband(Guild* /*guild*/) { }

        // Called when a guild member withdraws money from a guild bank.
        virtual void OnMemberWitdrawMoney(Guild* /*guild*/, Player* /*player*/, uint64& /*amount*/, bool /*isRepair*/) { }

        // Called when a guild member deposits money in a guild bank.
        virtual void OnMemberDepositMoney(Guild* /*guild*/, Player* /*player*/, uint64& /*amount*/) { }

        // Called when a guild member moves an item in a guild bank.
        virtual void OnItemMove(Guild* /*guild*/, Player* /*player*/, Item* /*pItem*/, bool /*isSrcBank*/, uint8 /*srcContainer*/, uint8 /*srcSlotId*/,
            bool /*isDestBank*/, uint8 /*destContainer*/, uint8 /*destSlotId*/) { }

        virtual void OnEvent(Guild* /*guild*/, uint8 /*eventType*/, ObjectGuid::LowType /*playerGuid1*/, ObjectGuid::LowType /*playerGuid2*/, uint8 /*newRank*/) { }

        virtual void OnBankEvent(Guild* /*guild*/, uint8 /*eventType*/, uint8 /*tabId*/, ObjectGuid::LowType /*playerGuid*/, uint64 /*itemOrMoney*/, uint16 /*itemStackCount*/, uint8 /*destTabId*/) { }

        // Called before the guild bank tab content is sent to a client, returning false blocks the packet
        [[nodiscard]] virtual bool CanGuildSendBankList(Guild const* /*guild*/, WorldSession* /*session*/, uint8 /*tabId*/, bool /*sendAllSlots*/) { return true; }
};

#endif // SC_GUILD_SCRIPT_H
