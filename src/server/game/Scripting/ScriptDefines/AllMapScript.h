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

#ifndef SC_ALL_MAP_SCRIPT_H
#define SC_ALL_MAP_SCRIPT_H

#include "ScriptObject.h"
#include <string>
#include <vector>

/*
 * AzerothCore's OnDestroyInstance(MapInstanced*, Map*) hook is not available:
 * ShatterCore has no MapInstanced layer, instanced maps are owned by the
 * MapManager directly. Use OnDestroyMap instead, which fires for every map.
 */

class InstanceMap;
class InstanceScript;
class Map;
class Player;

// AzerothCore compatible hook enum. ShatterCore dispatches through the
// script registry for every registered script - the per-hook filtering
// of AzerothCore is accepted but not implemented (see DatabaseScript.h).
// Only the hooks that exist in ShatterCore are listed; the order follows
// AzerothCore's where the hook exists there.
enum AllMapHook : uint16
{
    ALLMAPHOOK_ON_PLAYER_ENTER_ALL,
    ALLMAPHOOK_ON_PLAYER_LEAVE_ALL,
    ALLMAPHOOK_ON_BEFORE_CREATE_INSTANCE_SCRIPT,
    ALLMAPHOOK_ON_CREATE_MAP,
    ALLMAPHOOK_ON_DESTROY_MAP,
    ALLMAPHOOK_ON_MAP_UPDATE,
    ALLMAPHOOK_END
};

class TC_GAME_API AllMapScript : public ScriptObject
{
    protected:

        AllMapScript(char const* name, std::vector<uint16> enabledHooks = std::vector<uint16>());

    public:

        /**
         * @brief This hook called when a player enters any Map
         *
         * @param map Contains information about the Map
         * @param player Contains information about the Player
         */
        virtual void OnPlayerEnterAll(Map* /*map*/, Player* /*player*/) { }

        /**
         * @brief This hook called when a player leaves any Map
         *
         * @param map Contains information about the Map
         * @param player Contains information about the Player
         */
        virtual void OnPlayerLeaveAll(Map* /*map*/, Player* /*player*/) { }

        /**
         * @brief This hook called before the instance script is created
         *
         * @param instanceMap Contains information about the InstanceMap
         * @param instanceData The InstanceScript pointer to fill, scripts may
         *        assign their own InstanceScript here to override the core one
         * @param load If true the instance save data is being loaded
         * @param data Contains the saved instance data
         * @param completedEncounterMask Contains the completed encounter mask
         */
        virtual void OnBeforeCreateInstanceScript(InstanceMap* /*instanceMap*/, InstanceScript** /*instanceData*/, bool /*load*/, std::string /*data*/, uint32 /*completedEncounterMask*/) { }

        /**
         * @brief This hook called when a map is created
         *
         * @param map Contains information about the Map
         */
        virtual void OnCreateMap(Map* /*map*/) { }

        /**
         * @brief This hook called before a map is destroyed
         *
         * @param map Contains information about the Map
         */
        virtual void OnDestroyMap(Map* /*map*/) { }

        /**
         * @brief This hook called on every map update
         *
         * @param map Contains information about the Map
         * @param diff Contains information about the diff time
         */
        virtual void OnMapUpdate(Map* /*map*/, uint32 /*diff*/) { }
};

#endif // SC_ALL_MAP_SCRIPT_H
