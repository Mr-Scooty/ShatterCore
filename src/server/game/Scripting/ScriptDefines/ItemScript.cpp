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

#include "ItemScript.h"
#include "AllItemScript.h"
#include "GossipDef.h"
#include "Item.h"
#include "ItemTemplate.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptRegistry.h"

ItemScript::ItemScript(char const* name)
    : ScriptObject(name)
{
    ScriptRegistry<ItemScript>::Instance()->AddScript(this);
}

bool ScriptMgr::OnQuestAccept(Player* player, Item* item, Quest const* quest)
{
    ASSERT(player);
    ASSERT(item);
    ASSERT(quest);

    // AllItemScripts may veto the quest accept for any item
    FOR_SCRIPTS(AllItemScript, itr, end)
        if (!itr->second->CanItemQuestAccept(player, item, quest))
            return false;

    GET_SCRIPT_RET(ItemScript, item->GetScriptId(), tmpscript, false);
    player->PlayerTalkClass->ClearMenus();
    return tmpscript->OnQuestAccept(player, item, quest);
}

bool ScriptMgr::OnItemUse(Player* player, Item* item, SpellCastTargets const& targets)
{
    ASSERT(player);
    ASSERT(item);

    // AllItemScripts may handle the use of any item
    FOR_SCRIPTS(AllItemScript, itr, end)
        if (itr->second->CanItemUse(player, item, targets))
            return true;

    GET_SCRIPT_RET(ItemScript, item->GetScriptId(), tmpscript, false);
    return tmpscript->OnUse(player, item, targets);
}

bool ScriptMgr::OnItemExpire(Player* player, ItemTemplate const* proto)
{
    ASSERT(player);
    ASSERT(proto);

    // AllItemScripts may veto the expire handling for any item
    FOR_SCRIPTS(AllItemScript, itr, end)
        if (!itr->second->CanItemExpire(player, proto))
            return false;

    GET_SCRIPT_RET(ItemScript, proto->ScriptId, tmpscript, false);
    return tmpscript->OnExpire(player, proto);
}

bool ScriptMgr::OnItemRemove(Player* player, Item* item)
{
    ASSERT(player);
    ASSERT(item);

    // AllItemScripts may veto the remove handling for any item
    FOR_SCRIPTS(AllItemScript, itr, end)
        if (!itr->second->CanItemRemove(player, item))
            return false;

    GET_SCRIPT_RET(ItemScript, item->GetScriptId(), tmpscript, false);
    return tmpscript->OnRemove(player, item);
}

bool ScriptMgr::OnCastItemCombatSpell(Player* player, Unit* victim, SpellInfo const* spellInfo, Item* item)
{
    ASSERT(player);
    ASSERT(victim);
    ASSERT(spellInfo);
    ASSERT(item);

    GET_SCRIPT_RET(ItemScript, item->GetScriptId(), tmpscript, true);
    return tmpscript->OnCastItemCombatSpell(player, victim, spellInfo, item);
}

void ScriptMgr::OnGossipSelect(Player* player, Item* item, uint32 sender, uint32 action)
{
    ASSERT(player);
    ASSERT(item);

    FOREACH_SCRIPT(AllItemScript)->OnItemGossipSelect(player, item, sender, action);

    if (ItemScript* tmpscript = ScriptRegistry<ItemScript>::Instance()->GetScriptById(item->GetScriptId()))
        tmpscript->OnGossipSelect(player, item, sender, action);
}

void ScriptMgr::OnGossipSelectCode(Player* player, Item* item, uint32 sender, uint32 action, char const* code)
{
    ASSERT(player);
    ASSERT(item);

    FOREACH_SCRIPT(AllItemScript)->OnItemGossipSelectCode(player, item, sender, action, code);

    if (ItemScript* tmpscript = ScriptRegistry<ItemScript>::Instance()->GetScriptById(item->GetScriptId()))
        tmpscript->OnGossipSelectCode(player, item, sender, action, code);
}

template class TC_GAME_API ScriptRegistry<ItemScript>;
