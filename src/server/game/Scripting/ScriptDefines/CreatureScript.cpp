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

#include "CreatureScript.h"
#include "AllCreatureScript.h"
#include "Creature.h"
#include "ScriptMgr.h"
#include "ScriptRegistry.h"

CreatureScript::CreatureScript(char const* name)
    : ScriptObject(name)
{
    ScriptRegistry<CreatureScript>::Instance()->AddScript(this);
}

CreatureAI* ScriptMgr::GetCreatureAI(Creature* creature)
{
    ASSERT(creature);

    // AllCreatureScripts may provide an AI for any creature,
    // the first registered script which returns one wins.
    FOR_SCRIPTS(AllCreatureScript, itr, end)
        if (CreatureAI* ai = itr->second->GetCreatureAI(creature))
            return ai;

    GET_SCRIPT_RET(CreatureScript, creature->GetScriptId(), tmpscript, nullptr);
    return tmpscript->GetAI(creature);
}

template class TC_GAME_API ScriptRegistry<CreatureScript>;
