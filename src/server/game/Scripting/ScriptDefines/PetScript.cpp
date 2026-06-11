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

#include "PetScript.h"
#include "ScriptMgr.h"
#include "ScriptRegistry.h"

PetScript::PetScript(char const* name, std::vector<uint16> /*enabledHooks*/)
    : ScriptObject(name)
{
    ScriptRegistry<PetScript>::Instance()->AddScript(this);
}

void ScriptMgr::OnInitStatsForLevel(Guardian* guardian, uint8 petlevel)
{
    FOREACH_SCRIPT(PetScript)->OnInitStatsForLevel(guardian, petlevel);
}

void ScriptMgr::OnCalculateMaxTalentPointsForLevel(Pet* pet, uint8 level, uint8& points)
{
    FOREACH_SCRIPT(PetScript)->OnCalculateMaxTalentPointsForLevel(pet, level, points);
}

bool ScriptMgr::CanUnlearnSpellSet(Pet* pet, uint32 level, uint32 spell)
{
    FOR_SCRIPTS_RET(PetScript, itr, end, true)
        if (!itr->second->CanUnlearnSpellSet(pet, level, spell))
            return false;

    return true;
}

bool ScriptMgr::CanUnlearnSpellDefault(Pet* pet, SpellInfo const* spellInfo)
{
    FOR_SCRIPTS_RET(PetScript, itr, end, true)
        if (!itr->second->CanUnlearnSpellDefault(pet, spellInfo))
            return false;

    return true;
}

bool ScriptMgr::CanResetTalents(Pet* pet)
{
    FOR_SCRIPTS_RET(PetScript, itr, end, true)
        if (!itr->second->CanResetTalents(pet))
            return false;

    return true;
}

void ScriptMgr::OnPetAddToWorld(Pet* pet)
{
    ASSERT(pet);

    FOREACH_SCRIPT(PetScript)->OnPetAddToWorld(pet);
}

template class TC_GAME_API ScriptRegistry<PetScript>;
