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

#ifndef SC_ALL_ITEM_SCRIPT_H
#define SC_ALL_ITEM_SCRIPT_H

#include "ScriptObject.h"

/*
 * AllItemScript hooks run for every item, before the database bound
 * ItemScript of the item (if any) is executed. The virtuals follow their
 * AzerothCore equivalents for module source compatibility.
 */

class Item;
class Player;
class Quest;
class SpellCastTargets;
struct ItemTemplate;

class TC_GAME_API AllItemScript : public ScriptObject
{
    protected:

        AllItemScript(char const* name);

    public:

        // Called when a player accepts a quest from the item, returning false skips the quest accept
        [[nodiscard]] virtual bool CanItemQuestAccept(Player* /*player*/, Item* /*item*/, Quest const* /*quest*/) { return true; }

        // Called when a player uses the item, returning true marks the use as handled
        [[nodiscard]] virtual bool CanItemUse(Player* /*player*/, Item* /*item*/, SpellCastTargets const& /*targets*/) { return false; }

        // Called when the item is destroyed, returning false marks the removal as unhandled
        [[nodiscard]] virtual bool CanItemRemove(Player* /*player*/, Item* /*item*/) { return true; }

        // Called when the item expires (is destroyed), returning false marks the expire as unhandled
        [[nodiscard]] virtual bool CanItemExpire(Player* /*player*/, ItemTemplate const* /*proto*/) { return true; }

        // Called when a player selects an option in an item gossip window
        virtual void OnItemGossipSelect(Player* /*player*/, Item* /*item*/, uint32 /*sender*/, uint32 /*action*/) { }

        // Called when a player selects an option in an item gossip window
        virtual void OnItemGossipSelectCode(Player* /*player*/, Item* /*item*/, uint32 /*sender*/, uint32 /*action*/, char const* /*code*/) { }
};

#endif // SC_ALL_ITEM_SCRIPT_H
