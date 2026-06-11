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

#ifndef SC_TRANSPORT_SCRIPT_H
#define SC_TRANSPORT_SCRIPT_H

#include "ScriptObject.h"

class Creature;
class Player;
class Transport;

// TransportScript is database bound: the script is assigned to the transport
// gameobject entry through its `ScriptName` in the world database.
class TC_GAME_API TransportScript : public ScriptObject, public UpdatableScript<Transport>
{
    protected:

        TransportScript(char const* name);

    public:

        // Called when a player boards the transport.
        virtual void OnAddPassenger(Transport* /*transport*/, Player* /*player*/) { }

        // Called when a creature boards the transport.
        virtual void OnAddCreaturePassenger(Transport* /*transport*/, Creature* /*creature*/) { }

        // Called when a player exits the transport.
        virtual void OnRemovePassenger(Transport* /*transport*/, Player* /*player*/) { }

        // Called when a transport moves.
        virtual void OnRelocate(Transport* /*transport*/, uint32 /*mapId*/, float /*x*/, float /*y*/, float /*z*/) { }
};

#endif // SC_TRANSPORT_SCRIPT_H
