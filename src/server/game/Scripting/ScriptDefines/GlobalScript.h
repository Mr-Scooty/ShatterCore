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

#ifndef SC_GLOBAL_SCRIPT_H
#define SC_GLOBAL_SCRIPT_H

#include "DatabaseEnvFwd.h"
#include "ObjectGuid.h"
#include "ScriptObject.h"
#include <list>
#include <vector>

/*
 * Only the AzerothCore GlobalScript hooks with a clean 4.3.4 call site are
 * available. The following hooks were NOT ported:
 *
 *   OnAfterCalculateLootGroupAmount - bound to AzerothCore's
 *         Rate.Drop.Item.GroupAmount config which has no ShatterCore
 *         equivalent (loot groups always produce a single roll here).
 *   OnBeforeUpdateArenaPoints - 4.3.4 replaced arena points with conquest.
 *   OnBeforeWorldObjectSetPhaseMask - 4.3.4 uses PhaseShift/PhasingHandler,
 *         WorldObject::SetPhaseMask does not exist anymore.
 *   OnAllowedForPlayerLootCheck - AzerothCore threads a custom `source`
 *         guid through LootItem::AllowedForPlayer to feed this hook; the
 *         4.3.4 signature has no source object.
 *
 * Notes on ported hooks:
 *
 *   OnSpellHealingBonusTakenNegativeModifiers - in 4.3.4 players take
 *         healing through PLAYER_FIELD_MOD_HEALING_PCT, so the hook only
 *         fires for non-player targets (the only path which still uses the
 *         negative SPELL_AURA_MOD_HEALING_PCT modifier).
 *   OnInstanceIdRemoved - fired from
 *         InstanceSaveManager::_ResetInstance after the save, the database
 *         entry and the instance id itself have been released (AzerothCore
 *         fires it from its DeleteInstanceSaveIfNeeded equivalent).
 */

class GameObject;
class Item;
class LootStore;
class Map;
class Player;
class SpellInfo;
class Unit;

struct DungeonEncounter;
struct Loot;
struct LootStoreItem;
struct SpellModifier;

enum Difficulty : uint8;
enum EncounterCreditType : uint8;
enum EncounterState : uint32;

namespace lfg
{
    struct LFGDungeonData;
}

// AzerothCore compatible hook enum. ShatterCore dispatches through the
// script registry for every registered script - the per-hook filtering
// of AzerothCore is accepted but not implemented (see DatabaseScript.h).
// Only the hooks that exist in ShatterCore are listed; the order follows
// AzerothCore's where the hook exists there.
enum GlobalHook : uint16
{
    GLOBALHOOK_ON_ITEM_DEL_FROM_DB,
    GLOBALHOOK_ON_MIRRORIMAGE_DISPLAY_ITEM,
    GLOBALHOOK_ON_AFTER_REF_COUNT,
    GLOBALHOOK_ON_BEFORE_DROP_ADD_ITEM,
    GLOBALHOOK_ON_ITEM_ROLL,
    GLOBALHOOK_ON_BEFORE_LOOT_EQUAL_CHANCED,
    GLOBALHOOK_ON_INITIALIZE_LOCKED_DUNGEONS,
    GLOBALHOOK_ON_AFTER_INITIALIZE_LOCKED_DUNGEONS,
    GLOBALHOOK_ON_AFTER_UPDATE_ENCOUNTER_STATE,
    GLOBALHOOK_ON_IS_AFFECTED_BY_SPELL_MOD_CHECK,
    GLOBALHOOK_ON_SPELL_HEALING_BONUS_TAKEN_NEGATIVE_MODIFIERS,
    GLOBALHOOK_ON_LOAD_SPELL_CUSTOM_ATTR,
    GLOBALHOOK_ON_ALLOWED_TO_LOOT_CONTAINER_CHECK,
    GLOBALHOOK_ON_INSTANCEID_REMOVED,
    GLOBALHOOK_ON_BEFORE_SET_BOSS_STATE,
    GLOBALHOOK_AFTER_INSTANCE_GAME_OBJECT_CREATE,
    GLOBALHOOK_END
};

// following hooks can be used anywhere and are not db bounded
class TC_GAME_API GlobalScript : public ScriptObject
{
    protected:

        GlobalScript(char const* name, std::vector<uint16> enabledHooks = std::vector<uint16>());

    public:

        // items
        virtual void OnItemDelFromDB(CharacterDatabaseTransaction /*trans*/, ObjectGuid::LowType /*itemGuid*/) { }
        virtual void OnMirrorImageDisplayItem(Item const* /*item*/, uint32& /*display*/) { }

        // loot
        // Called after the reference multiplicator of a referenced loot entry has been rolled, maxcount can be changed
        virtual void OnAfterRefCount(Player const* /*player*/, LootStoreItem* /*LootStoreItem*/, Loot& /*loot*/, bool /*canRate*/, uint16 /*lootMode*/, uint32& /*maxcount*/, LootStore const& /*store*/) { }

        // Called before a rolled loot entry is added to the loot
        virtual void OnBeforeDropAddItem(Player const* /*player*/, Loot& /*loot*/, bool /*canRate*/, uint16 /*lootMode*/, LootStoreItem* /*LootStoreItem*/, LootStore const& /*store*/) { }

        // Called when a loot entry rolls for its drop chance, chance can be changed, returning false skips the entry
        virtual bool OnItemRoll(Player const* /*player*/, LootStoreItem const* /*LootStoreItem*/, float& /*chance*/, Loot& /*loot*/, LootStore const& /*store*/) { return true; }

        // Called before an item is taken from the equal-chanced part of a loot group, returning false skips the group
        virtual bool OnBeforeLootEqualChanced(Player const* /*player*/, std::list<LootStoreItem*> /*EqualChanced*/, Loot& /*loot*/, LootStore const& /*store*/) { return true; }

        // lfg
        virtual void OnInitializeLockedDungeons(Player* /*player*/, uint8& /*level*/, uint32& /*lockData*/, lfg::LFGDungeonData const* /*dungeon*/) { }
        virtual void OnAfterInitializeLockedDungeons(Player* /*player*/) { }

        // Called when a dungeon encounter is updated.
        virtual void OnAfterUpdateEncounterState(Map* /*map*/, EncounterCreditType /*type*/, uint32 /*creditEntry*/, Unit* /*source*/, Difficulty /*difficulty_fixed*/, std::list<DungeonEncounter const*> const* /*encounters*/, uint32 /*dungeonCompleted*/, bool /*updated*/) { }

        // Called when checking if a spell is affected by a spell mod, returning false denies the mod
        [[nodiscard]] virtual bool OnIsAffectedBySpellModCheck(SpellInfo const* /*affectSpell*/, SpellInfo const* /*checkSpell*/, SpellModifier const* /*mod*/) { return true; }

        // Called before the negative healing taken modifier is computed for a non-player
        // target, returning true uses val instead of the default computation
        [[nodiscard]] virtual bool OnSpellHealingBonusTakenNegativeModifiers(Unit const* /*target*/, Unit const* /*caster*/, SpellInfo const* /*spellInfo*/, float& /*val*/) { return false; }

        // Called for every spell when the custom spell attributes are loaded
        virtual void OnLoadSpellCustomAttr(SpellInfo* /*spell*/) { }

        // Called before a loot container is opened in Player::SendLoot,
        // returning true denies the loot (the player receives a loot error)
        [[nodiscard]] virtual bool OnAllowedToLootContainerCheck(Player const* /*player*/, ObjectGuid /*source*/) { return false; }

        // Called after an instance id has been removed from the instance save manager
        virtual void OnInstanceIdRemoved(uint32 /*instanceId*/) { }

        // Called at the top of InstanceScript::SetBossState
        virtual void OnBeforeSetBossState(uint32 /*id*/, EncounterState /*newState*/, EncounterState /*oldState*/, Map* /*instance*/) { }

        // Called after a gameobject is added to an instance script
        virtual void AfterInstanceGameObjectCreate(Map* /*instance*/, GameObject* /*go*/) { }
};

#endif // SC_GLOBAL_SCRIPT_H
