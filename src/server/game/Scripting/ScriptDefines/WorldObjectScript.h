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

#ifndef SC_WORLD_OBJECT_SCRIPT_H
#define SC_WORLD_OBJECT_SCRIPT_H

#include "ScriptObject.h"
#include <vector>

/*
 * AzerothCore's OnWorldObjectUpdate hook is not available: ShatterCore's
 * WorldObject::Update is an empty inline virtual without a common dispatch
 * point, the per-type update hooks (OnAllCreatureUpdate, OnGameObjectUpdate,
 * OnPlayerUpdate, ...) cover the updateable object types instead.
 */

class Map;
class WorldObject;

// AzerothCore compatible hook enum. ShatterCore dispatches through the
// script registry for every registered script - the per-hook filtering
// of AzerothCore is accepted but not implemented (see DatabaseScript.h).
// Only the hooks that exist in ShatterCore are listed; the order follows
// AzerothCore's where the hook exists there.
enum WorldObjectHook : uint16
{
    WORLDOBJECTHOOK_ON_WORLD_OBJECT_DESTROY,
    WORLDOBJECTHOOK_ON_WORLD_OBJECT_CREATE,
    WORLDOBJECTHOOK_ON_WORLD_OBJECT_SET_MAP,
    WORLDOBJECTHOOK_ON_WORLD_OBJECT_RESET_MAP,
    WORLDOBJECTHOOK_END
};

class TC_GAME_API WorldObjectScript : public ScriptObject
{
    protected:

        WorldObjectScript(char const* name, std::vector<uint16> enabledHooks = std::vector<uint16>());

    public:

        /**
         * @brief This hook called before destroy world object
         *
         * @param object Contains information about the WorldObject
         */
        virtual void OnWorldObjectDestroy(WorldObject* /*object*/) { }

        /**
         * @brief This hook called after create world object
         *
         * @param object Contains information about the WorldObject
         */
        virtual void OnWorldObjectCreate(WorldObject* /*object*/) { }

        /**
         * @brief This hook called after world object set to map
         *
         * @param object Contains information about the WorldObject
         * @param map Contains information about the Map
         */
        virtual void OnWorldObjectSetMap(WorldObject* /*object*/, Map* /*map*/) { }

        /**
         * @brief This hook called before the world object leaves its map
         *
         * @param object Contains information about the WorldObject
         */
        virtual void OnWorldObjectResetMap(WorldObject* /*object*/) { }
};

#endif // SC_WORLD_OBJECT_SCRIPT_H
