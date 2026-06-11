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

#ifndef SC_MISC_SCRIPT_H
#define SC_MISC_SCRIPT_H

#include "ObjectGuid.h"
#include "ScriptObject.h"
#include <vector>

/*
 * The following AzerothCore MiscScript hooks were NOT ported because
 * ShatterCore's 4.3.4 core has no clean equivalent call site:
 *
 *   ValidateSpellAtCastSpell, ValidateSpellAtCastSpellResult - the 4.3.4
 *         WorldSession::HandleCastSpellOpcode was rewritten around the
 *         pending cast request system; the 3.3.5a oldSpellId/castFlags
 *         rewrite point no longer exists.
 *   OnPlayerSetPhase - bound to AzerothCore's phase mask aura handler;
 *         4.3.4 phasing works through PhaseShift/PhasingHandler (and the
 *         hook has no caller in AzerothCore itself).
 */

class Creature;
class Group;
class InstanceSave;
class Item;
class LootStore;
class LootTemplate;
class Object;
class Player;
class WorldSession;
struct ItemTemplate;
struct Loot;

// AzerothCore compatible hook enum. ShatterCore dispatches through the
// script registry for every registered script - the per-hook filtering
// of AzerothCore is accepted but not implemented (see DatabaseScript.h).
// Only the hooks that exist in ShatterCore are listed; the order follows
// AzerothCore's where the hook exists there.
enum MiscHook : uint16
{
    MISCHOOK_ON_CONSTRUCT_OBJECT,
    MISCHOOK_ON_DESTRUCT_OBJECT,
    MISCHOOK_ON_CONSTRUCT_PLAYER,
    MISCHOOK_ON_DESTRUCT_PLAYER,
    MISCHOOK_ON_CONSTRUCT_GROUP,
    MISCHOOK_ON_DESTRUCT_GROUP,
    MISCHOOK_ON_CONSTRUCT_INSTANCE_SAVE,
    MISCHOOK_ON_DESTRUCT_INSTANCE_SAVE,
    MISCHOOK_ON_ITEM_CREATE,
    MISCHOOK_CAN_APPLY_SOULBOUND_FLAG,
    MISCHOOK_CAN_ITEM_APPLY_EQUIP_SPELL,
    MISCHOOK_CAN_SEND_AUCTIONHELLO,
    MISCHOOK_ON_AFTER_LOOT_TEMPLATE_PROCESS,
    MISCHOOK_ON_INSTANCE_SAVE,
    MISCHOOK_GET_DIALOG_STATUS,
    MISCHOOK_END
};

class TC_GAME_API MiscScript : public ScriptObject
{
    protected:

        MiscScript(char const* name, std::vector<uint16> enabledHooks = std::vector<uint16>());

    public:

        // Called when an Object is constructed
        virtual void OnConstructObject(Object* /*origin*/) { }

        // Called when an Object is destructed
        virtual void OnDestructObject(Object* /*origin*/) { }

        // Called when a Player is constructed
        virtual void OnConstructPlayer(Player* /*origin*/) { }

        // Called when a Player is destructed
        virtual void OnDestructPlayer(Player* /*origin*/) { }

        // Called when a Group is constructed
        virtual void OnConstructGroup(Group* /*origin*/) { }

        // Called when a Group is destructed
        virtual void OnDestructGroup(Group* /*origin*/) { }

        // Called when an InstanceSave is constructed
        virtual void OnConstructInstanceSave(InstanceSave* /*origin*/) { }

        // Called when an InstanceSave is destructed
        virtual void OnDestructInstanceSave(InstanceSave* /*origin*/) { }

        // Called at the end of Item::Create
        virtual void OnItemCreate(Item* /*item*/, ItemTemplate const* /*itemProto*/, Player const* /*owner*/) { }

        // Called when loading an item from the database before removing the
        // soulbound flag from items whose template is not binding, returning
        // false keeps the soulbound flag
        [[nodiscard]] virtual bool CanApplySoulboundFlag(Item* /*item*/, ItemTemplate const* /*proto*/) { return true; }

        // Called before an item set spell is applied on equip, returning false skips the spell
        [[nodiscard]] virtual bool CanItemApplyEquipSpell(Player* /*player*/, Item* /*item*/) { return true; }

        // Called before the auction house window is opened, returning false keeps it closed
        [[nodiscard]] virtual bool CanSendAuctionHello(WorldSession const* /*session*/, ObjectGuid /*guid*/, Creature* /*creature*/) { return true; }

        // Called after a loot template has been processed into the loot
        virtual void OnAfterLootTemplateProcess(Loot* /*loot*/, LootTemplate const* /*tab*/, LootStore const& /*store*/, Player* /*lootOwner*/, bool /*personal*/, bool /*noEmptyError*/, uint16 /*lootMode*/) { }

        // Called after an instance save has been written to the database
        virtual void OnInstanceSave(InstanceSave* /*instanceSave*/) { }

        // Called at the top of Player::GetQuestDialogStatus
        virtual void GetDialogStatus(Player* /*player*/, Object* /*questgiver*/) { }
};

#endif // SC_MISC_SCRIPT_H
