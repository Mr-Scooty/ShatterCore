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

#include "AreaTriggerScript.h"
#include "DBCStructure.h"
#include "InstanceScript.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptRegistry.h"

AreaTriggerScript::AreaTriggerScript(char const* name)
    : ScriptObject(name)
{
    ScriptRegistry<AreaTriggerScript>::Instance()->AddScript(this);
}

bool ScriptMgr::OnAreaTrigger(Player* player, AreaTriggerEntry const* trigger)
{
    ASSERT(player);
    ASSERT(trigger);

    GET_SCRIPT_RET(AreaTriggerScript, sObjectMgr->GetAreaTriggerScriptId(trigger->ID), tmpscript, false);
    return tmpscript->OnTrigger(player, trigger);
}

bool OnlyOnceAreaTriggerScript::OnTrigger(Player* player, AreaTriggerEntry const* trigger)
{
    uint32 const triggerId = trigger->ID;
    if (InstanceScript* instance = player->GetInstanceScript())
    {
        if (instance->IsAreaTriggerDone(triggerId))
            return true;
        else
            instance->MarkAreaTriggerDone(triggerId);
    }
    return _OnTrigger(player, trigger);
}

void OnlyOnceAreaTriggerScript::ResetAreaTriggerDone(InstanceScript* script, uint32 triggerId) { script->ResetAreaTriggerDone(triggerId); }
void OnlyOnceAreaTriggerScript::ResetAreaTriggerDone(Player const* player, AreaTriggerEntry const* trigger) { if (InstanceScript* instance = player->GetInstanceScript()) ResetAreaTriggerDone(instance, trigger->ID); }

template class TC_GAME_API ScriptRegistry<AreaTriggerScript>;
