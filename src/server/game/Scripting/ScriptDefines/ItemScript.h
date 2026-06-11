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

#ifndef SC_ITEM_SCRIPT_H
#define SC_ITEM_SCRIPT_H

#include "ScriptObject.h"

class Item;
class Player;
class Quest;
class SpellCastTargets;
class SpellInfo;
class Unit;
struct ItemTemplate;

// ItemScript is database bound: the script is assigned to a specific item
// entry through its `ScriptName` in the world database.
class TC_GAME_API ItemScript : public ScriptObject
{
    protected:

        ItemScript(char const* name);

    public:

        // Called when a player accepts a quest from the item.
        virtual bool OnQuestAccept(Player* /*player*/, Item* /*item*/, Quest const* /*quest*/) { return false; }

        // Called when a player uses the item.
        virtual bool OnUse(Player* /*player*/, Item* /*item*/, SpellCastTargets const& /*targets*/) { return false; }

        // Called when the item expires (is destroyed).
        virtual bool OnExpire(Player* /*player*/, ItemTemplate const* /*proto*/) { return false; }

        // Called when the item is destroyed.
        virtual bool OnRemove(Player* /*player*/, Item* /*item*/) { return false; }

        // Called before casting a combat spell from this item (chance on hit spells of item template, can be used to prevent cast if returning false)
        virtual bool OnCastItemCombatSpell(Player* /*player*/, Unit* /*victim*/, SpellInfo const* /*spellInfo*/, Item* /*item*/) { return true; }

        // Called when a player selects an option in an item gossip window
        virtual void OnGossipSelect(Player* /*player*/, Item* /*item*/, uint32 /*sender*/, uint32 /*action*/) { }

        // Called when a player selects an option in an item gossip window
        virtual void OnGossipSelectCode(Player* /*player*/, Item* /*item*/, uint32 /*sender*/, uint32 /*action*/, char const* /*code*/) { }
};

#endif // SC_ITEM_SCRIPT_H
