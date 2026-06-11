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

#include "PlayerScript.h"
#include "ScriptMgr.h"
#include "ScriptRegistry.h"
#include "Player.h"

PlayerScript::PlayerScript(char const* name, std::vector<uint16> /*enabledHooks*/)
    : ScriptObject(name)
{
    ScriptRegistry<PlayerScript>::Instance()->AddScript(this);
}

void ScriptMgr::OnPlayerJustDied(Player* player)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerJustDied(player);
}

void ScriptMgr::OnPlayerReleasedGhost(Player* player)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerReleasedGhost(player);
}

void ScriptMgr::OnPlayerCompleteQuest(Player* player, Quest const* quest)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerCompleteQuest(player, quest);
}

void ScriptMgr::OnPVPKill(Player* killer, Player* killed)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerPVPKill(killer, killed);
}

void ScriptMgr::OnCreatureKill(Player* killer, Creature* killed)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerCreatureKill(killer, killed);
}

void ScriptMgr::OnPlayerKilledByCreature(Creature* killer, Player* killed)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerKilledByCreature(killer, killed);
}

void ScriptMgr::OnPlayerLevelChanged(Player* player, uint8 oldLevel)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerLevelChanged(player, oldLevel);
}

void ScriptMgr::OnPlayerFreeTalentPointsChanged(Player* player, uint32 points)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerFreeTalentPointsChanged(player, points);
}

void ScriptMgr::OnPlayerTalentsReset(Player* player, bool noCost)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerTalentsReset(player, noCost);
}

void ScriptMgr::OnPlayerBeforeUpdate(Player* player, uint32 p_time)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerBeforeUpdate(player, p_time);
}

void ScriptMgr::OnPlayerUpdate(Player* player, uint32 p_time)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerUpdate(player, p_time);
}

void ScriptMgr::OnPlayerMoneyChanged(Player* player, int64& amount)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerMoneyChanged(player, amount);
}

void ScriptMgr::OnPlayerMoneyLimit(Player* player, int64 amount)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerMoneyLimit(player, amount);
}

void ScriptMgr::OnPlayerBeforeLootMoney(Player* player, Loot* loot)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerBeforeLootMoney(player, loot);
}

void ScriptMgr::OnGivePlayerXP(Player* player, uint32& amount, Unit* victim, uint8 xpSource)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerGiveXP(player, amount, victim, xpSource);
}

void ScriptMgr::OnPlayerReputationChange(Player* player, uint32 factionID, int32& standing, bool incremental)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerReputationChange(player, factionID, standing, incremental);
}

void ScriptMgr::OnPlayerReputationRankChange(Player* player, uint32 factionID, ReputationRank newRank, ReputationRank oldRank, bool increased)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerReputationRankChange(player, factionID, newRank, oldRank, increased);
}

void ScriptMgr::OnPlayerGiveReputation(Player* player, int32 factionID, float& amount, ReputationSource repSource)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerGiveReputation(player, factionID, amount, repSource);
}

void ScriptMgr::OnPlayerDuelRequest(Player* target, Player* challenger)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerDuelRequest(target, challenger);
}

void ScriptMgr::OnPlayerDuelStart(Player* player1, Player* player2)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerDuelStart(player1, player2);
}

void ScriptMgr::OnPlayerDuelEnd(Player* winner, Player* loser, DuelCompleteType type)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerDuelEnd(winner, loser, type);
}

void ScriptMgr::OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerChat(player, type, lang, msg);
}

void ScriptMgr::OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg, Player* receiver)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerChat(player, type, lang, msg, receiver);
}

void ScriptMgr::OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg, Group* group)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerChat(player, type, lang, msg, group);
}

void ScriptMgr::OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg, Guild* guild)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerChat(player, type, lang, msg, guild);
}

void ScriptMgr::OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg, Channel* channel)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerChat(player, type, lang, msg, channel);
}

void ScriptMgr::OnPlayerClearEmote(Player* player)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerClearEmote(player);
}

void ScriptMgr::OnPlayerTextEmote(Player* player, uint32 textEmote, uint32 emoteNum, ObjectGuid guid)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerTextEmote(player, textEmote, emoteNum, guid);
}

void ScriptMgr::OnPlayerSpellCast(Player* player, Spell* spell, bool skipCheck)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerSpellCast(player, spell, skipCheck);
}

void ScriptMgr::OnPlayerLoadFromDB(Player* player)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerLoadFromDB(player);
}

void ScriptMgr::OnPlayerLogin(Player* player, bool firstLogin)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerLogin(player);

    if (firstLogin)
    {
        FOREACH_SCRIPT(PlayerScript)->OnPlayerFirstLogin(player);
    }
}

void ScriptMgr::OnPlayerBeforeLogout(Player* player)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerBeforeLogout(player);
}

void ScriptMgr::OnPlayerLogout(Player* player)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerLogout(player);
}

void ScriptMgr::OnPlayerCreate(Player* player)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerCreate(player);
}

void ScriptMgr::OnPlayerDelete(ObjectGuid guid, uint32 accountId)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerDelete(guid, accountId);
}

void ScriptMgr::OnPlayerFailedDelete(ObjectGuid guid, uint32 accountId)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerFailedDelete(guid, accountId);
}

void ScriptMgr::OnPlayerSave(Player* player)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerSave(player);
}

void ScriptMgr::OnPlayerBindToInstance(Player* player, Difficulty difficulty, uint32 mapid, bool permanent, uint8 extendState)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerBindToInstance(player, difficulty, mapid, permanent, extendState);
}

void ScriptMgr::OnPlayerUpdateZone(Player* player, uint32 newZone, uint32 newArea)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerUpdateZone(player, newZone, newArea);
}

void ScriptMgr::OnPlayerUpdateArea(Player* player, uint32 oldArea, uint32 newArea)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerUpdateArea(player, oldArea, newArea);
}

bool ScriptMgr::OnPlayerBeforeTeleport(Player* player, uint32 mapid, float x, float y, float z, float orientation, uint32 options, Unit* target)
{
    FOR_SCRIPTS_RET(PlayerScript, itr, end, true)
        if (!itr->second->OnPlayerBeforeTeleport(player, mapid, x, y, z, orientation, options, target))
            return false;

    return true;
}

bool ScriptMgr::OnPlayerBeforeQuestComplete(Player* player, uint32 questId)
{
    FOR_SCRIPTS_RET(PlayerScript, itr, end, true)
        if (!itr->second->OnPlayerBeforeQuestComplete(player, questId))
            return false;

    return true;
}

void ScriptMgr::OnPlayerQuestComputeXP(Player* player, Quest const* quest, uint32& xpValue)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerQuestComputeXP(player, quest, xpValue);
}

void ScriptMgr::OnPlayerBeforeDurabilityRepair(Player* player, ObjectGuid npcGUID, ObjectGuid itemGUID, float& discountMod, uint8 guildBank)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerBeforeDurabilityRepair(player, npcGUID, itemGUID, discountMod, guildBank);
}

void ScriptMgr::OnPlayerSetMaxLevel(Player* player, uint32& maxPlayerLevel)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerSetMaxLevel(player, maxPlayerLevel);
}

bool ScriptMgr::OnPlayerShouldBeRewardedWithMoneyInsteadOfExp(Player* player)
{
    FOR_SCRIPTS_RET(PlayerScript, itr, end, false)
        if (itr->second->OnPlayerShouldBeRewardedWithMoneyInsteadOfExp(player))
            return true;

    return false;
}

bool ScriptMgr::OnPlayerCanRepopAtGraveyard(Player* player)
{
    FOR_SCRIPTS_RET(PlayerScript, itr, end, true)
        if (!itr->second->OnPlayerCanRepopAtGraveyard(player))
            return false;

    return true;
}

void ScriptMgr::OnPlayerResurrect(Player* player, float restorePercent, bool& applySickness)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerResurrect(player, restorePercent, applySickness);
}

void ScriptMgr::OnPlayerQuestAbandon(Player* player, uint32 questId)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerQuestAbandon(player, questId);
}

bool ScriptMgr::OnPlayerCanGiveLevel(Player* player, uint8 newLevel)
{
    FOR_SCRIPTS_RET(PlayerScript, itr, end, true)
        if (!itr->second->OnPlayerCanGiveLevel(player, newLevel))
            return false;

    return true;
}

void ScriptMgr::OnQuestStatusChange(Player* player, uint32 questId)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerQuestStatusChange(player, questId);
}

void ScriptMgr::OnPlayerRepop(Player* player)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerRepop(player);
}

void ScriptMgr::OnPlayerPVPFlagChange(Player* player, bool state)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerPVPFlagChange(player, state);
}

void ScriptMgr::OnPlayerFfaPvpStateUpdate(Player* player, bool result)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerFfaPvpStateUpdate(player, result);
}

void ScriptMgr::OnPlayerVictimRewardBefore(Player* player, Player* victim, uint32& killerTitle, uint32& victimRank)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerVictimRewardBefore(player, victim, killerTitle, victimRank);
}

void ScriptMgr::OnPlayerVictimRewardAfter(Player* player, Player* victim, uint32& killerTitle, uint32& victimRank, float& honorF)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerVictimRewardAfter(player, victim, killerTitle, victimRank, honorF);
}

void ScriptMgr::OnPlayerEnterCombat(Player* player, Unit* enemy)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerEnterCombat(player, enemy);
}

void ScriptMgr::OnPlayerLeaveCombat(Player* player)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerLeaveCombat(player);
}

void ScriptMgr::OnPlayerAfterSetVisibleItemSlot(Player* player, uint8 slot, Item* item)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerAfterSetVisibleItemSlot(player, slot, item);
}

void ScriptMgr::OnPlayerEquip(Player* player, Item* it, uint8 bag, uint8 slot, bool update)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerEquip(player, it, bag, slot, update);
}

void ScriptMgr::OnPlayerLootItem(Player* player, Item* item, uint32 count, ObjectGuid lootguid)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerLootItem(player, item, count, lootguid);
}

void ScriptMgr::OnPlayerStoreNewItem(Player* player, Item* item, uint32 count)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerStoreNewItem(player, item, count);
}

void ScriptMgr::OnPlayerCreateItem(Player* player, Item* item, uint32 count)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerCreateItem(player, item, count);
}

void ScriptMgr::OnPlayerQuestRewardItem(Player* player, Item* item, uint32 count)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerQuestRewardItem(player, item, count);
}

void ScriptMgr::OnPlayerBeforeBuyItemFromVendor(Player* player, ObjectGuid vendorguid, uint32 vendorslot, uint32& item, uint32 count, uint8 bag, uint8 slot)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerBeforeBuyItemFromVendor(player, vendorguid, vendorslot, item, count, bag, slot);
}

void ScriptMgr::OnPlayerBeforeStoreOrEquipNewItem(Player* player, uint32 vendorslot, uint32& item, uint32 count, uint8 bag, uint8 slot, ItemTemplate const* pProto, Creature* pVendor, VendorItem const* crItem, bool bStore)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerBeforeStoreOrEquipNewItem(player, vendorslot, item, count, bag, slot, pProto, pVendor, crItem, bStore);
}

void ScriptMgr::OnPlayerAfterStoreOrEquipNewItem(Player* player, uint32 vendorslot, Item* item, uint32 count, uint8 bag, uint8 slot, ItemTemplate const* pProto, Creature* pVendor, VendorItem const* crItem, bool bStore)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerAfterStoreOrEquipNewItem(player, vendorslot, item, count, bag, slot, pProto, pVendor, crItem, bStore);
}

bool ScriptMgr::OnPlayerCanSellItem(Player* player, Item* item, Creature* creature)
{
    FOR_SCRIPTS_RET(PlayerScript, itr, end, true)
        if (!itr->second->OnPlayerCanSellItem(player, item, creature))
            return false;

    return true;
}

bool ScriptMgr::OnPlayerCanEquipItem(Player* player, uint8 slot, uint16& dest, Item* pItem, bool swap, bool not_loading)
{
    FOR_SCRIPTS_RET(PlayerScript, itr, end, true)
        if (!itr->second->OnPlayerCanEquipItem(player, slot, dest, pItem, swap, not_loading))
            return false;

    return true;
}

bool ScriptMgr::OnPlayerCanUnequipItem(Player* player, uint16 pos, bool swap)
{
    FOR_SCRIPTS_RET(PlayerScript, itr, end, true)
        if (!itr->second->OnPlayerCanUnequipItem(player, pos, swap))
            return false;

    return true;
}

bool ScriptMgr::OnPlayerCanUseItem(Player* player, ItemTemplate const* proto, InventoryResult& result)
{
    FOR_SCRIPTS_RET(PlayerScript, itr, end, true)
        if (!itr->second->OnPlayerCanUseItem(player, proto, result))
            return false;

    return true;
}

void ScriptMgr::OnPlayerBeforeSendChatMessage(Player* player, uint32& type, uint32& lang, std::string& msg)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerBeforeSendChatMessage(player, type, lang, msg);
}

void ScriptMgr::OnPlayerQueueRandomDungeon(Player* player, uint32& rDungeonId)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerQueueRandomDungeon(player, rDungeonId);
}

void ScriptMgr::OnPlayerGossipSelect(Player* player, uint32 menu_id, uint32 sender, uint32 action)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerGossipSelect(player, menu_id, sender, action);
}

void ScriptMgr::OnPlayerGossipSelectCode(Player* player, uint32 menu_id, uint32 sender, uint32 action, char const* code)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerGossipSelectCode(player, menu_id, sender, action, code);
}

bool ScriptMgr::OnPlayerCanSendMail(Player* player, ObjectGuid receiverGuid, ObjectGuid mailbox, std::string& subject, std::string& body, uint64 money, uint64 COD, Item* item)
{
    FOR_SCRIPTS_RET(PlayerScript, itr, end, true)
        if (!itr->second->OnPlayerCanSendMail(player, receiverGuid, mailbox, subject, body, money, COD, item))
            return false;

    return true;
}

void ScriptMgr::OnPlayerPetitionBuy(Player* player, Creature* creature, uint32& charterid, uint32& cost, uint32& type)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerPetitionBuy(player, creature, charterid, cost, type);
}

void ScriptMgr::OnPlayerPetitionShowList(Player* player, Creature* creature, uint32& CharterEntry, uint32& CharterDispayID, uint32& CharterCost)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerPetitionShowList(player, creature, CharterEntry, CharterDispayID, CharterCost);
}

bool ScriptMgr::OnPlayerCanJoinLfg(Player* player, uint8 roles, lfg::LfgDungeonSet& dungeons, std::string const& comment)
{
    FOR_SCRIPTS_RET(PlayerScript, itr, end, true)
        if (!itr->second->OnPlayerCanJoinLfg(player, roles, dungeons, comment))
            return false;

    return true;
}

bool ScriptMgr::OnPlayerCanInitTrade(Player* player, Player* target)
{
    FOR_SCRIPTS_RET(PlayerScript, itr, end, true)
        if (!itr->second->OnPlayerCanInitTrade(player, target))
            return false;

    return true;
}

bool ScriptMgr::OnPlayerCanUseChat(Player* player, uint32 type, uint32 language, std::string& msg)
{
    FOR_SCRIPTS_RET(PlayerScript, itr, end, true)
        if (!itr->second->OnPlayerCanUseChat(player, type, language, msg))
            return false;

    return true;
}

bool ScriptMgr::OnPlayerCanUseChat(Player* player, uint32 type, uint32 language, std::string& msg, Player* receiver)
{
    FOR_SCRIPTS_RET(PlayerScript, itr, end, true)
        if (!itr->second->OnPlayerCanUseChat(player, type, language, msg, receiver))
            return false;

    return true;
}

bool ScriptMgr::OnPlayerCanUseChat(Player* player, uint32 type, uint32 language, std::string& msg, Group* group)
{
    FOR_SCRIPTS_RET(PlayerScript, itr, end, true)
        if (!itr->second->OnPlayerCanUseChat(player, type, language, msg, group))
            return false;

    return true;
}

bool ScriptMgr::OnPlayerCanUseChat(Player* player, uint32 type, uint32 language, std::string& msg, Guild* guild)
{
    FOR_SCRIPTS_RET(PlayerScript, itr, end, true)
        if (!itr->second->OnPlayerCanUseChat(player, type, language, msg, guild))
            return false;

    return true;
}

bool ScriptMgr::OnPlayerCanUseChat(Player* player, uint32 type, uint32 language, std::string& msg, Channel* channel)
{
    FOR_SCRIPTS_RET(PlayerScript, itr, end, true)
        if (!itr->second->OnPlayerCanUseChat(player, type, language, msg, channel))
            return false;

    return true;
}

void ScriptMgr::OnPlayerBattlegroundDesertion(Player* player, BattlegroundDesertionType desertionType)
{
    ASSERT(player);
    FOREACH_SCRIPT(PlayerScript)->OnPlayerBattlegroundDesertion(player, desertionType);
}

void ScriptMgr::OnPlayerJoinBG(Player* player)
{
    ASSERT(player);
    FOREACH_SCRIPT(PlayerScript)->OnPlayerJoinBG(player);
}

void ScriptMgr::OnPlayerJoinArena(Player* player)
{
    ASSERT(player);
    FOREACH_SCRIPT(PlayerScript)->OnPlayerJoinArena(player);
}

void ScriptMgr::OnPlayerGetMaxPersonalArenaRatingRequirement(Player const* player, uint32 minSlot, uint32& maxArenaRating)
{
    ASSERT(player);
    FOREACH_SCRIPT(PlayerScript)->OnPlayerGetMaxPersonalArenaRatingRequirement(player, minSlot, maxArenaRating);
}

bool ScriptMgr::OnPlayerCanJoinInBattlegroundQueue(Player* player, ObjectGuid battlemasterGuid, BattlegroundTypeId bgTypeId, uint8 joinAsGroup, GroupJoinBattlegroundResult& err)
{
    ASSERT(player);
    FOR_SCRIPTS_RET(PlayerScript, itr, end, true)
        if (!itr->second->OnPlayerCanJoinInBattlegroundQueue(player, battlemasterGuid, bgTypeId, joinAsGroup, err))
            return false;

    return true;
}

void ScriptMgr::OnPlayerSetServerSideVisibility(Player* player, ServerSideVisibilityType& type, AccountTypes& sec)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerSetServerSideVisibility(player, type, sec);
}

void ScriptMgr::OnPlayerSetServerSideVisibilityDetect(Player* player, ServerSideVisibilityType& type, AccountTypes& sec)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerSetServerSideVisibilityDetect(player, type, sec);
}

bool ScriptMgr::OnPlayerCanLearnTalent(Player* player, TalentEntry const* talent, uint32 rank)
{
    ASSERT(player);
    FOR_SCRIPTS_RET(PlayerScript, itr, end, true)
        if (!itr->second->OnPlayerCanLearnTalent(player, talent, rank))
            return false;

    return true;
}

void ScriptMgr::OnPlayerAfterSpecSlotChanged(Player* player, uint8 newSlot)
{
    ASSERT(player);
    FOREACH_SCRIPT(PlayerScript)->OnPlayerAfterSpecSlotChanged(player, newSlot);
}

void ScriptMgr::OnPlayerLearnTalents(Player* player, uint32 talentId, uint32 talentRank, uint32 spellid)
{
    ASSERT(player);
    FOREACH_SCRIPT(PlayerScript)->OnPlayerLearnTalents(player, talentId, talentRank, spellid);
}

void ScriptMgr::AnticheatSetCanFlybyServer(Player* player, bool apply)
{
    FOREACH_SCRIPT(PlayerScript)->AnticheatSetCanFlybyServer(player, apply);
}

void ScriptMgr::AnticheatSetUnderACKmount(Player* player)
{
    FOREACH_SCRIPT(PlayerScript)->AnticheatSetUnderACKmount(player);
}

void ScriptMgr::AnticheatSetRootACKUpd(Player* player)
{
    FOREACH_SCRIPT(PlayerScript)->AnticheatSetRootACKUpd(player);
}

void ScriptMgr::AnticheatSetJumpingbyOpcode(Player* player, bool jump)
{
    FOREACH_SCRIPT(PlayerScript)->AnticheatSetJumpingbyOpcode(player, jump);
}

void ScriptMgr::AnticheatUpdateMovementInfo(Player* player, MovementInfo const& movementInfo)
{
    FOREACH_SCRIPT(PlayerScript)->AnticheatUpdateMovementInfo(player, movementInfo);
}

bool ScriptMgr::AnticheatHandleDoubleJump(Player* player, Unit* mover)
{
    FOR_SCRIPTS_RET(PlayerScript, itr, end, true)
        if (!itr->second->AnticheatHandleDoubleJump(player, mover))
            return false;

    return true;
}

bool ScriptMgr::AnticheatCheckMovementInfo(Player* player, MovementInfo const& movementInfo, Unit* mover, bool jump)
{
    FOR_SCRIPTS_RET(PlayerScript, itr, end, true)
        if (!itr->second->AnticheatCheckMovementInfo(player, movementInfo, mover, jump))
            return false;

    return true;
}

template class TC_GAME_API ScriptRegistry<PlayerScript>;
