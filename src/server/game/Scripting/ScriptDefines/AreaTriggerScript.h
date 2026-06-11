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

#ifndef SC_AREA_TRIGGER_SCRIPT_H
#define SC_AREA_TRIGGER_SCRIPT_H

#include "ScriptObject.h"

class InstanceScript;
class Player;
struct AreaTriggerEntry;

// AreaTriggerScript is database bound: the script is assigned to an area
// trigger ID through the `areatrigger_scripts` table in the world database.
class TC_GAME_API AreaTriggerScript : public ScriptObject
{
    protected:

        AreaTriggerScript(char const* name);

    public:

        // Called when the area trigger is activated by a player.
        virtual bool OnTrigger(Player* /*player*/, AreaTriggerEntry const* /*trigger*/) { return false; }
};

class TC_GAME_API OnlyOnceAreaTriggerScript : public AreaTriggerScript
{
    using AreaTriggerScript::AreaTriggerScript;

    public:
        bool OnTrigger(Player* /*player*/, AreaTriggerEntry const* /*trigger*/) final override;

    protected:
        virtual bool _OnTrigger(Player* /*player*/, AreaTriggerEntry const* /*trigger*/) = 0;
        void ResetAreaTriggerDone(InstanceScript* /*instance*/, uint32 /*triggerId*/);
        void ResetAreaTriggerDone(Player const* /*player*/, AreaTriggerEntry const* /*trigger*/);
};

#endif // SC_AREA_TRIGGER_SCRIPT_H
