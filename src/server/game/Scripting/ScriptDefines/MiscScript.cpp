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

#include "MiscScript.h"
#include "ScriptMgr.h"
#include "ScriptRegistry.h"

MiscScript::MiscScript(char const* name, std::vector<uint16> /*enabledHooks*/)
    : ScriptObject(name)
{
    ScriptRegistry<MiscScript>::Instance()->AddScript(this);
}

void ScriptMgr::OnConstructObject(Object* origin)
{
    FOREACH_SCRIPT(MiscScript)->OnConstructObject(origin);
}

void ScriptMgr::OnDestructObject(Object* origin)
{
    FOREACH_SCRIPT(MiscScript)->OnDestructObject(origin);
}

void ScriptMgr::OnConstructPlayer(Player* origin)
{
    FOREACH_SCRIPT(MiscScript)->OnConstructPlayer(origin);
}

void ScriptMgr::OnDestructPlayer(Player* origin)
{
    FOREACH_SCRIPT(MiscScript)->OnDestructPlayer(origin);
}

void ScriptMgr::OnConstructGroup(Group* origin)
{
    FOREACH_SCRIPT(MiscScript)->OnConstructGroup(origin);
}

void ScriptMgr::OnDestructGroup(Group* origin)
{
    FOREACH_SCRIPT(MiscScript)->OnDestructGroup(origin);
}

void ScriptMgr::OnConstructInstanceSave(InstanceSave* origin)
{
    FOREACH_SCRIPT(MiscScript)->OnConstructInstanceSave(origin);
}

void ScriptMgr::OnDestructInstanceSave(InstanceSave* origin)
{
    FOREACH_SCRIPT(MiscScript)->OnDestructInstanceSave(origin);
}

void ScriptMgr::OnItemCreate(Item* item, ItemTemplate const* itemProto, Player const* owner)
{
    FOREACH_SCRIPT(MiscScript)->OnItemCreate(item, itemProto, owner);
}

bool ScriptMgr::CanApplySoulboundFlag(Item* item, ItemTemplate const* proto)
{
    FOR_SCRIPTS_RET(MiscScript, itr, end, true)
        if (!itr->second->CanApplySoulboundFlag(item, proto))
            return false;

    return true;
}

bool ScriptMgr::CanItemApplyEquipSpell(Player* player, Item* item)
{
    FOR_SCRIPTS_RET(MiscScript, itr, end, true)
        if (!itr->second->CanItemApplyEquipSpell(player, item))
            return false;

    return true;
}

bool ScriptMgr::CanSendAuctionHello(WorldSession const* session, ObjectGuid guid, Creature* creature)
{
    FOR_SCRIPTS_RET(MiscScript, itr, end, true)
        if (!itr->second->CanSendAuctionHello(session, guid, creature))
            return false;

    return true;
}

void ScriptMgr::OnAfterLootTemplateProcess(Loot* loot, LootTemplate const* tab, LootStore const& store, Player* lootOwner, bool personal, bool noEmptyError, uint16 lootMode)
{
    FOREACH_SCRIPT(MiscScript)->OnAfterLootTemplateProcess(loot, tab, store, lootOwner, personal, noEmptyError, lootMode);
}

void ScriptMgr::OnInstanceSave(InstanceSave* instanceSave)
{
    FOREACH_SCRIPT(MiscScript)->OnInstanceSave(instanceSave);
}

void ScriptMgr::GetDialogStatus(Player* player, Object* questgiver)
{
    FOREACH_SCRIPT(MiscScript)->GetDialogStatus(player, questgiver);
}

template class TC_GAME_API ScriptRegistry<MiscScript>;
