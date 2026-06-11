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

#ifndef SC_WEATHER_SCRIPT_H
#define SC_WEATHER_SCRIPT_H

#include "ScriptObject.h"

class Weather;

enum WeatherState : uint32;

// WeatherScript is database bound: the script is assigned to a zone through
// the `game_weather` table in the world database.
class TC_GAME_API WeatherScript : public ScriptObject, public UpdatableScript<Weather>
{
    protected:

        WeatherScript(char const* name);

    public:

        // Called when the weather changes in the zone this script is associated with.
        virtual void OnChange(Weather* /*weather*/, WeatherState /*state*/, float /*grade*/) { }
};

#endif // SC_WEATHER_SCRIPT_H
