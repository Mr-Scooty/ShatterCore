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

#include "GlobalScript.h"
#include "ScriptMgr.h"
#include "ScriptRegistry.h"

GlobalScript::GlobalScript(char const* name, std::vector<uint16> /*enabledHooks*/)
    : ScriptObject(name)
{
    ScriptRegistry<GlobalScript>::Instance()->AddScript(this);
}

void ScriptMgr::OnGlobalItemDelFromDB(CharacterDatabaseTransaction trans, ObjectGuid::LowType itemGuid)
{
    FOREACH_SCRIPT(GlobalScript)->OnItemDelFromDB(trans, itemGuid);
}

void ScriptMgr::OnGlobalMirrorImageDisplayItem(Item const* item, uint32& display)
{
    FOREACH_SCRIPT(GlobalScript)->OnMirrorImageDisplayItem(item, display);
}

void ScriptMgr::OnAfterRefCount(Player const* player, Loot& loot, bool canRate, uint16 lootMode, LootStoreItem* lootStoreItem, uint32& maxcount, LootStore const& store)
{
    FOREACH_SCRIPT(GlobalScript)->OnAfterRefCount(player, lootStoreItem, loot, canRate, lootMode, maxcount, store);
}

void ScriptMgr::OnBeforeDropAddItem(Player const* player, Loot& loot, bool canRate, uint16 lootMode, LootStoreItem* lootStoreItem, LootStore const& store)
{
    FOREACH_SCRIPT(GlobalScript)->OnBeforeDropAddItem(player, loot, canRate, lootMode, lootStoreItem, store);
}

bool ScriptMgr::OnItemRoll(Player const* player, LootStoreItem const* lootStoreItem, float& chance, Loot& loot, LootStore const& store)
{
    FOR_SCRIPTS_RET(GlobalScript, itr, end, true)
        if (!itr->second->OnItemRoll(player, lootStoreItem, chance, loot, store))
            return false;

    return true;
}

bool ScriptMgr::OnBeforeLootEqualChanced(Player const* player, std::list<LootStoreItem*> equalChanced, Loot& loot, LootStore const& store)
{
    FOR_SCRIPTS_RET(GlobalScript, itr, end, true)
        if (!itr->second->OnBeforeLootEqualChanced(player, equalChanced, loot, store))
            return false;

    return true;
}

void ScriptMgr::OnInitializeLockedDungeons(Player* player, uint8& level, uint32& lockData, lfg::LFGDungeonData const* dungeon)
{
    FOREACH_SCRIPT(GlobalScript)->OnInitializeLockedDungeons(player, level, lockData, dungeon);
}

void ScriptMgr::OnAfterInitializeLockedDungeons(Player* player)
{
    FOREACH_SCRIPT(GlobalScript)->OnAfterInitializeLockedDungeons(player);
}

void ScriptMgr::OnAfterUpdateEncounterState(Map* map, EncounterCreditType type, uint32 creditEntry, Unit* source, Difficulty difficulty_fixed, std::list<DungeonEncounter const*> const* encounters, uint32 dungeonCompleted, bool updated)
{
    FOREACH_SCRIPT(GlobalScript)->OnAfterUpdateEncounterState(map, type, creditEntry, source, difficulty_fixed, encounters, dungeonCompleted, updated);
}

bool ScriptMgr::OnIsAffectedBySpellModCheck(SpellInfo const* affectSpell, SpellInfo const* checkSpell, SpellModifier const* mod)
{
    FOR_SCRIPTS_RET(GlobalScript, itr, end, true)
        if (!itr->second->OnIsAffectedBySpellModCheck(affectSpell, checkSpell, mod))
            return false;

    return true;
}

bool ScriptMgr::OnSpellHealingBonusTakenNegativeModifiers(Unit const* target, Unit const* caster, SpellInfo const* spellInfo, float& val)
{
    FOR_SCRIPTS_RET(GlobalScript, itr, end, false)
        if (itr->second->OnSpellHealingBonusTakenNegativeModifiers(target, caster, spellInfo, val))
            return true;

    return false;
}

void ScriptMgr::OnLoadSpellCustomAttr(SpellInfo* spell)
{
    FOREACH_SCRIPT(GlobalScript)->OnLoadSpellCustomAttr(spell);
}

bool ScriptMgr::OnAllowedToLootContainerCheck(Player const* player, ObjectGuid source)
{
    FOR_SCRIPTS_RET(GlobalScript, itr, end, true)
        if (itr->second->OnAllowedToLootContainerCheck(player, source))
            return false;

    return true;
}

void ScriptMgr::OnInstanceIdRemoved(uint32 instanceId)
{
    FOREACH_SCRIPT(GlobalScript)->OnInstanceIdRemoved(instanceId);
}

void ScriptMgr::OnBeforeSetBossState(uint32 id, EncounterState newState, EncounterState oldState, Map* instance)
{
    FOREACH_SCRIPT(GlobalScript)->OnBeforeSetBossState(id, newState, oldState, instance);
}

void ScriptMgr::AfterInstanceGameObjectCreate(Map* instance, GameObject* go)
{
    FOREACH_SCRIPT(GlobalScript)->AfterInstanceGameObjectCreate(instance, go);
}

template class TC_GAME_API ScriptRegistry<GlobalScript>;
