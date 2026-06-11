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

#ifndef SC_ACHIEVEMENT_CRITERIA_SCRIPT_H
#define SC_ACHIEVEMENT_CRITERIA_SCRIPT_H

#include "ScriptObject.h"

class Player;
class Unit;

// AchievementCriteriaScript is database bound: the script is assigned
// through the `achievement_criteria_data` table in the world database.
class TC_GAME_API AchievementCriteriaScript : public ScriptObject
{
    protected:

        AchievementCriteriaScript(char const* name);

    public:

        // Called when an additional criteria is checked.
        virtual bool OnCheck(Player* source, Unit* target) = 0;
};

#endif // SC_ACHIEVEMENT_CRITERIA_SCRIPT_H
