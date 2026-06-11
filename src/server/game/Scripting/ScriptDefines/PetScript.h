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

#ifndef SC_PET_SCRIPT_H
#define SC_PET_SCRIPT_H

#include "ScriptObject.h"
#include <vector>

class Guardian;
class Pet;
class SpellInfo;

// AzerothCore compatible hook enum. ShatterCore dispatches through the
// script registry for every registered script - the per-hook filtering
// of AzerothCore is accepted but not implemented (see DatabaseScript.h).
enum PetHook : uint16
{
    PETHOOK_ON_INIT_STATS_FOR_LEVEL,
    PETHOOK_ON_CALCULATE_MAX_TALENT_POINTS_FOR_LEVEL,
    PETHOOK_CAN_UNLEARN_SPELL_SET,
    PETHOOK_CAN_UNLEARN_SPELL_DEFAULT,
    PETHOOK_CAN_RESET_TALENTS,
    PETHOOK_ON_PET_ADD_TO_WORLD,
    PETHOOK_END
};

class TC_GAME_API PetScript : public ScriptObject
{
    protected:

        PetScript(char const* name, std::vector<uint16> enabledHooks = std::vector<uint16>());

    public:

        // Called in Guardian::InitStatsForLevel before the stats are finalized
        virtual void OnInitStatsForLevel(Guardian* /*guardian*/, uint8 /*petlevel*/) { }

        // Called when calculating a pet's max talent points
        virtual void OnCalculateMaxTalentPointsForLevel(Pet* /*pet*/, uint8 /*level*/, uint8& /*points*/) { }

        // Called before unlearning a levelup spell on level down, returning false keeps the spell
        [[nodiscard]] virtual bool CanUnlearnSpellSet(Pet* /*pet*/, uint32 /*level*/, uint32 /*spell*/) { return true; }

        // Called before unlearning a default spell on level down, returning false keeps the spell
        [[nodiscard]] virtual bool CanUnlearnSpellDefault(Pet* /*pet*/, SpellInfo const* /*spellInfo*/) { return true; }

        // Called before a pet's talents are reset, returning false aborts the reset
        [[nodiscard]] virtual bool CanResetTalents(Pet* /*pet*/) { return true; }

        /**
         * @brief This hook called after add pet in world
         *
         * @param pet Contains information about the Pet
         */
        virtual void OnPetAddToWorld(Pet* /*pet*/) { }
};

#endif // SC_PET_SCRIPT_H
