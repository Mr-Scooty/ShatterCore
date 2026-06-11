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

#include "AllItemScript.h"
#include "ScriptMgr.h"
#include "ScriptRegistry.h"

// The ScriptMgr item dispatch methods which compose AllItemScript and the
// database bound ItemScript live in ScriptDefines/ItemScript.cpp

AllItemScript::AllItemScript(char const* name)
    : ScriptObject(name)
{
    ScriptRegistry<AllItemScript>::Instance()->AddScript(this);
}

template class TC_GAME_API ScriptRegistry<AllItemScript>;
