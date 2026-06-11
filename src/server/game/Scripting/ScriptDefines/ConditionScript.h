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

#ifndef SC_CONDITION_SCRIPT_H
#define SC_CONDITION_SCRIPT_H

#include "ScriptObject.h"

struct Condition;
struct ConditionSourceInfo;

// ConditionScript is database bound: the script is assigned through the
// CONDITION_SCRIPT condition type in the world database.
class TC_GAME_API ConditionScript : public ScriptObject
{
    protected:

        ConditionScript(char const* name);

    public:

        // Called when a single condition is checked for a player.
        virtual bool OnConditionCheck(Condition const* /*condition*/, ConditionSourceInfo& /*sourceInfo*/) { return true; }
};

#endif // SC_CONDITION_SCRIPT_H
