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

#ifndef SC_INSTANCE_MAP_SCRIPT_H
#define SC_INSTANCE_MAP_SCRIPT_H

#include "ScriptObject.h"

class InstanceMap;
class InstanceScript;

// InstanceMapScript is database bound: the script is assigned to the
// instance map through the `instance_template` table in the world database.
class TC_GAME_API InstanceMapScript
    : public ScriptObject, public MapScript<InstanceMap>
{
    protected:

        InstanceMapScript(char const* name, uint32 mapId);

    public:

        // Gets an InstanceScript object for this instance.
        virtual InstanceScript* GetInstanceScript(InstanceMap* /*map*/) const { return nullptr; }
};

#endif // SC_INSTANCE_MAP_SCRIPT_H
