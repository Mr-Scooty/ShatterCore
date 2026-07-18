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

#ifndef VASHJIR_H_
#define VASHJIR_H_

#include "Position.h"

namespace Vashjir
{
enum VashjirZones
{
    ZONE_VASHJIR                    = 5146,
    ZONE_KELPTHAR_FOREST            = 4815,
    ZONE_SHIMMERING_EXPANSE         = 5144,
    ZONE_ABYSSAL_DEPTHS             = 5145
};

enum IntroQuests
{
    // Alliance
    QUEST_A_PERSONAL_SUMMONS        = 28825,
    QUEST_EYE_OF_THE_STORM_A        = 28826,
    QUEST_TO_THE_DEPTHS_A           = 28827,
    QUEST_HEROS_CALL_VASHJIR        = 27724,
    QUEST_CALL_OF_DUTY_A            = 14482,
    QUEST_SEA_LEGS_A                = 24432,
    // Horde
    QUEST_EYE_OF_THE_STORM_H        = 28805,
    QUEST_TO_THE_DEPTHS_H           = 28816,
    QUEST_WARCHIEFS_COMMAND_VASHJIR = 27718,
    QUEST_CALL_OF_DUTY_H            = 25924,
    QUEST_SEA_LEGS_H                = 25929
};

enum IntroCreatures
{
    NPC_RECRUITER_BURNS             = 45226,
    NPC_ERUNAK_STONESPEAKER         = 36915
};
}

#endif // VASHJIR_H_
