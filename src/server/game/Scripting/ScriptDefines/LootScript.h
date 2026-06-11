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

#ifndef SC_LOOT_SCRIPT_H
#define SC_LOOT_SCRIPT_H

#include "ScriptObject.h"
#include <vector>

class Player;

// AzerothCore compatible hook enum. ShatterCore dispatches through the
// script registry for every registered script - the per-hook filtering
// of AzerothCore is accepted but not implemented (see DatabaseScript.h).
enum LootHook : uint16
{
    LOOTHOOK_ON_LOOT_MONEY,
    LOOTHOOK_END
};

class TC_GAME_API LootScript : public ScriptObject
{
    protected:

        LootScript(char const* name, std::vector<uint16> enabledHooks = std::vector<uint16>());

    public:

        /**
         * @brief This hook is called when money is looted, after it has been
         * given to the player(s) (shared money is reported once with the
         * full loot amount)
         *
         * @param player Contains information about the Player
         * @param gold Contains information about money
         */
        virtual void OnLootMoney(Player* /*player*/, uint32 /*gold*/) { }
};

#endif // SC_LOOT_SCRIPT_H
