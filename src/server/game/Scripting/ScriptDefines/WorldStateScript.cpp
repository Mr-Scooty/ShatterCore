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

#include "WorldStateScript.h"
#include "ScriptMgr.h"
#include "ScriptRegistry.h"
#include "WorldStateDefines.h"

WorldStateScript::WorldStateScript(char const* name)
    : ScriptObject(name)
{
    ScriptRegistry<WorldStateScript>::Instance()->AddScript(this);
}

WorldStateScript::~WorldStateScript() = default;

void ScriptMgr::OnWorldStateValueChange(WorldStateTemplate const* worldStateTemplate, int32 oldValue, int32 newValue, Map const* map)
{
    ASSERT(worldStateTemplate);

    GET_SCRIPT(WorldStateScript, worldStateTemplate->ScriptId, tmpscript);
    tmpscript->OnValueChange(worldStateTemplate->Id, oldValue, newValue, map);
}

template class TC_GAME_API ScriptRegistry<WorldStateScript>;
