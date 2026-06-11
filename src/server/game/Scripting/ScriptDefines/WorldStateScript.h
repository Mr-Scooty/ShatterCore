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

#ifndef SC_WORLD_STATE_SCRIPT_H
#define SC_WORLD_STATE_SCRIPT_H

#include "ScriptObject.h"

class Map;

// WorldStateScript is database bound: the script is assigned through the
// `world_state` table in the world database. ShatterCore specific script
// type without an AzerothCore equivalent.
class TC_GAME_API WorldStateScript : public ScriptObject
{
    protected:

        WorldStateScript(char const* name);

    public:

        ~WorldStateScript();

        // Called when worldstate changes value, map is optional
        virtual void OnValueChange([[maybe_unused]] int32 worldStateId, [[maybe_unused]] int32 oldValue, [[maybe_unused]] int32 newValue, [[maybe_unused]] Map const* map) { }
};

#endif // SC_WORLD_STATE_SCRIPT_H
