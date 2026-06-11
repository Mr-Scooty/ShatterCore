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

#ifndef SC_PLAYER_SCRIPT_H
#define SC_PLAYER_SCRIPT_H

#include "Common.h"
#include "ObjectGuid.h"
#include "ScriptObject.h"
#include "SharedDefines.h"
#include <set>
#include <string>
#include <vector>

/*
 * The PlayerScript virtuals were renamed to their AzerothCore equivalents
 * for module source compatibility. When rebasing on upstream TrinityCore,
 * map the old TC virtual names to the new ones as follows:
 *
 *   OnPVPKill                 -> OnPlayerPVPKill
 *   OnCreatureKill            -> OnPlayerCreatureKill
 *   OnPlayerKilledByCreature  -> OnPlayerKilledByCreature (unchanged)
 *   OnLevelChanged            -> OnPlayerLevelChanged
 *   OnFreeTalentPointsChanged -> OnPlayerFreeTalentPointsChanged
 *   OnTalentsReset            -> OnPlayerTalentsReset
 *   OnMoneyChanged            -> OnPlayerMoneyChanged
 *   OnMoneyLimit              -> OnPlayerMoneyLimit
 *   OnGiveXP                  -> OnPlayerGiveXP (gained an xpSource argument, see PlayerXPSource)
 *   OnReputationChange        -> OnPlayerReputationChange
 *   OnDuelRequest             -> OnPlayerDuelRequest
 *   OnDuelStart               -> OnPlayerDuelStart
 *   OnDuelEnd                 -> OnPlayerDuelEnd
 *   OnChat (5 overloads)      -> OnPlayerChat
 *   OnClearEmote              -> OnPlayerClearEmote
 *   OnTextEmote               -> OnPlayerTextEmote
 *   OnSpellCast               -> OnPlayerSpellCast
 *   OnLogin(player, firstLogin) -> OnPlayerLogin(player) + OnPlayerFirstLogin(player)
 *   OnLogout                  -> OnPlayerLogout
 *   OnCreate                  -> OnPlayerCreate
 *   OnDelete                  -> OnPlayerDelete
 *   OnFailedDelete            -> OnPlayerFailedDelete
 *   OnSave                    -> OnPlayerSave
 *   OnBindToInstance          -> OnPlayerBindToInstance (keeps the extendState argument)
 *   OnUpdateZone              -> OnPlayerUpdateZone
 *   OnMapChanged              -> OnPlayerMapChanged
 *   OnQuestStatusChange       -> OnPlayerQuestStatusChange
 *   OnPlayerRepop             -> OnPlayerRepop (unchanged)
 *
 * ShatterCore keeps its 4.3.4 signature types where they are richer than
 * AzerothCore's 3.3.5a ones (money is int64, OnPlayerBindToInstance carries
 * the extendState).
 */

class Channel;
class Creature;
class Group;
class Guild;
class Item;
class Player;
class Quest;
class Spell;
class Unit;
struct ItemTemplate;
struct Loot;
struct MovementInfo;
struct TalentEntry;
struct VendorItem;

namespace lfg
{
    typedef std::set<uint32> LfgDungeonSet;
}

enum BattlegroundDesertionType : uint8;
enum Difficulty : uint8;
enum InventoryResult : uint8;
enum QuestStatus : uint8;
enum ReputationSource : uint8;

// AzerothCore compatible hook enum. ShatterCore dispatches through the
// script registry for every registered script - the per-hook filtering
// of AzerothCore is accepted but not implemented (see DatabaseScript.h).
// Only the hooks that exist in ShatterCore are listed; the order follows
// AzerothCore's where the hook exists there.
enum PlayerHook : uint16
{
    PLAYERHOOK_ON_PLAYER_JUST_DIED,
    PLAYERHOOK_ON_PLAYER_RELEASED_GHOST,
    PLAYERHOOK_ON_PLAYER_COMPLETE_QUEST,
    PLAYERHOOK_ON_PVP_KILL,
    PLAYERHOOK_ON_CREATURE_KILL,
    PLAYERHOOK_ON_PLAYER_KILLED_BY_CREATURE,
    PLAYERHOOK_ON_LEVEL_CHANGED,
    PLAYERHOOK_ON_FREE_TALENT_POINTS_CHANGED,
    PLAYERHOOK_ON_TALENTS_RESET,
    PLAYERHOOK_ON_BEFORE_UPDATE,
    PLAYERHOOK_ON_UPDATE,
    PLAYERHOOK_ON_MONEY_CHANGED,
    PLAYERHOOK_ON_BEFORE_LOOT_MONEY,
    PLAYERHOOK_ON_GIVE_EXP,
    PLAYERHOOK_ON_REPUTATION_CHANGE,
    PLAYERHOOK_ON_REPUTATION_RANK_CHANGE,
    PLAYERHOOK_ON_DUEL_REQUEST,
    PLAYERHOOK_ON_DUEL_START,
    PLAYERHOOK_ON_DUEL_END,
    PLAYERHOOK_ON_TEXT_EMOTE,
    PLAYERHOOK_ON_SPELL_CAST,
    PLAYERHOOK_ON_LOAD_FROM_DB,
    PLAYERHOOK_ON_LOGIN,
    PLAYERHOOK_ON_BEFORE_LOGOUT,
    PLAYERHOOK_ON_LOGOUT,
    PLAYERHOOK_ON_CREATE,
    PLAYERHOOK_ON_DELETE,
    PLAYERHOOK_ON_FAILED_DELETE,
    PLAYERHOOK_ON_SAVE,
    PLAYERHOOK_ON_BIND_TO_INSTANCE,
    PLAYERHOOK_ON_UPDATE_ZONE,
    PLAYERHOOK_ON_UPDATE_AREA,
    PLAYERHOOK_ON_MAP_CHANGED,
    PLAYERHOOK_ON_BEFORE_TELEPORT,
    PLAYERHOOK_ON_BEFORE_QUEST_COMPLETE,
    PLAYERHOOK_ON_QUEST_COMPUTE_EXP,
    PLAYERHOOK_ON_BEFORE_DURABILITY_REPAIR,
    PLAYERHOOK_ON_FIRST_LOGIN,
    PLAYERHOOK_ON_SET_MAX_LEVEL,
    PLAYERHOOK_SHOULD_BE_REWARDED_WITH_MONEY_INSTEAD_OF_EXP,
    PLAYERHOOK_CAN_REPOP_AT_GRAVEYARD,
    PLAYERHOOK_ON_PLAYER_RESURRECT,
    PLAYERHOOK_ON_QUEST_ABANDON,
    PLAYERHOOK_ON_CAN_GIVE_LEVEL,
    PLAYERHOOK_ON_GIVE_REPUTATION,
    PLAYERHOOK_ON_PLAYER_PVP_FLAG_CHANGE,
    PLAYERHOOK_ON_FFA_PVP_STATE_UPDATE,
    PLAYERHOOK_ON_VICTIM_REWARD_BEFORE,
    PLAYERHOOK_ON_VICTIM_REWARD_AFTER,
    PLAYERHOOK_ON_PLAYER_ENTER_COMBAT,
    PLAYERHOOK_ON_PLAYER_LEAVE_COMBAT,
    PLAYERHOOK_ON_AFTER_SET_VISIBLE_ITEM_SLOT,
    PLAYERHOOK_ON_EQUIP,
    PLAYERHOOK_ON_LOOT_ITEM,
    PLAYERHOOK_ON_STORE_NEW_ITEM,
    PLAYERHOOK_ON_CREATE_ITEM,
    PLAYERHOOK_ON_QUEST_REWARD_ITEM,
    PLAYERHOOK_ON_BEFORE_BUY_ITEM_FROM_VENDOR,
    PLAYERHOOK_ON_BEFORE_STORE_OR_EQUIP_NEW_ITEM,
    PLAYERHOOK_ON_AFTER_STORE_OR_EQUIP_NEW_ITEM,
    PLAYERHOOK_CAN_SELL_ITEM,
    PLAYERHOOK_CAN_EQUIP_ITEM,
    PLAYERHOOK_CAN_UNEQUIP_ITEM,
    PLAYERHOOK_CAN_USE_ITEM,
    PLAYERHOOK_ON_BEFORE_SEND_CHAT_MESSAGE,
    PLAYERHOOK_ON_QUEUE_RANDOM_DUNGEON,
    PLAYERHOOK_ON_GOSSIP_SELECT,
    PLAYERHOOK_ON_GOSSIP_SELECT_CODE,
    PLAYERHOOK_CAN_SEND_MAIL,
    PLAYERHOOK_PETITION_BUY,
    PLAYERHOOK_PETITION_SHOW_LIST,
    PLAYERHOOK_CAN_JOIN_LFG,
    PLAYERHOOK_CAN_INIT_TRADE,
    PLAYERHOOK_CAN_PLAYER_USE_CHAT,
    PLAYERHOOK_CAN_PLAYER_USE_PRIVATE_CHAT,
    PLAYERHOOK_CAN_PLAYER_USE_GROUP_CHAT,
    PLAYERHOOK_CAN_PLAYER_USE_GUILD_CHAT,
    PLAYERHOOK_CAN_PLAYER_USE_CHANNEL_CHAT,
    PLAYERHOOK_ON_SET_SERVER_SIDE_VISIBILITY,
    PLAYERHOOK_ON_SET_SERVER_SIDE_VISIBILITY_DETECT,
    PLAYERHOOK_CAN_LEARN_TALENT,
    PLAYERHOOK_ON_AFTER_SPEC_SLOT_CHANGED,
    PLAYERHOOK_ON_PLAYER_LEARN_TALENTS,
    PLAYERHOOK_ANTICHEAT_SET_CAN_FLY_BY_SERVER,
    PLAYERHOOK_ANTICHEAT_SET_UNDER_ACK_MOUNT,
    PLAYERHOOK_ANTICHEAT_SET_ROOT_ACK_UPD,
    PLAYERHOOK_ANTICHEAT_SET_JUMPING_BY_OPCODE,
    PLAYERHOOK_ANTICHEAT_UPDATE_MOVEMENT_INFO,
    PLAYERHOOK_ANTICHEAT_HANDLE_DOUBLE_JUMP,
    PLAYERHOOK_ANTICHEAT_CHECK_MOVEMENT_INFO,
    // ShatterCore specific hooks without an AzerothCore equivalent
    PLAYERHOOK_ON_CHAT,
    PLAYERHOOK_ON_CLEAR_EMOTE,
    PLAYERHOOK_ON_MONEY_LIMIT,
    PLAYERHOOK_ON_QUEST_STATUS_CHANGE,
    PLAYERHOOK_ON_PLAYER_REPOP,
    PLAYERHOOK_ON_BATTLEGROUND_DESERTION,
    PLAYERHOOK_ON_PLAYER_JOIN_BG,
    PLAYERHOOK_ON_PLAYER_JOIN_ARENA,
    PLAYERHOOK_ON_GET_MAX_PERSONAL_ARENA_RATING_REQUIREMENT,
    PLAYERHOOK_CAN_JOIN_IN_BATTLEGROUND_QUEUE,
    PLAYERHOOK_END
};

class TC_GAME_API PlayerScript : public ScriptObject
{
    protected:

        PlayerScript(char const* name, std::vector<uint16> enabledHooks = std::vector<uint16>());

    public:

        // Called when a player dies
        virtual void OnPlayerJustDied(Player* /*player*/) { }

        // Called when clicking the release button
        virtual void OnPlayerReleasedGhost(Player* /*player*/) { }

        // Called when a player completes a quest
        virtual void OnPlayerCompleteQuest(Player* /*player*/, Quest const* /*quest*/) { }

        // Called when a player kills another player
        virtual void OnPlayerPVPKill(Player* /*killer*/, Player* /*killed*/) { }

        // Called when a player kills a creature
        virtual void OnPlayerCreatureKill(Player* /*killer*/, Creature* /*killed*/) { }

        // Called when a player is killed by a creature
        virtual void OnPlayerKilledByCreature(Creature* /*killer*/, Player* /*killed*/) { }

        // Called when a player's level changes (after the level is applied)
        virtual void OnPlayerLevelChanged(Player* /*player*/, uint8 /*oldLevel*/) { }

        // Called when a player's free talent points change (right before the change is applied)
        virtual void OnPlayerFreeTalentPointsChanged(Player* /*player*/, uint32 /*points*/) { }

        // Called when a player's talent points are reset (right before the reset is done)
        virtual void OnPlayerTalentsReset(Player* /*player*/, bool /*noCost*/) { }

        // Called for player::update
        virtual void OnPlayerBeforeUpdate(Player* /*player*/, uint32 /*p_time*/) { }
        virtual void OnPlayerUpdate(Player* /*player*/, uint32 /*p_time*/) { }

        // Called when a player's money is modified (before the modification is done)
        virtual void OnPlayerMoneyChanged(Player* /*player*/, int64& /*amount*/) { }

        // Called when a player's money is at limit (amount = money tried to add)
        virtual void OnPlayerMoneyLimit(Player* /*player*/, int64 /*amount*/) { }

        // Called before looted money is added to a player
        virtual void OnPlayerBeforeLootMoney(Player* /*player*/, Loot* /*loot*/) { }

        // Called when a player gains XP (before anything is given), xpSource is a PlayerXPSource value
        virtual void OnPlayerGiveXP(Player* /*player*/, uint32& /*amount*/, Unit* /*victim*/, uint8 /*xpSource*/) { }

        // Called when a player's reputation changes (before it is actually changed)
        virtual void OnPlayerReputationChange(Player* /*player*/, uint32 /*factionId*/, int32& /*standing*/, bool /*incremental*/) { }

        // Called when a player's reputation rank changes (before it is actually changed)
        virtual void OnPlayerReputationRankChange(Player* /*player*/, uint32 /*factionID*/, ReputationRank /*newRank*/, ReputationRank /*oldRank*/, bool /*increased*/) { }

        // Called when a player gains reputation (before anything is given)
        virtual void OnPlayerGiveReputation(Player* /*player*/, int32 /*factionID*/, float& /*amount*/, ReputationSource /*repSource*/) { }

        // Called when a duel is requested
        virtual void OnPlayerDuelRequest(Player* /*target*/, Player* /*challenger*/) { }

        // Called when a duel starts (after 3s countdown)
        virtual void OnPlayerDuelStart(Player* /*player1*/, Player* /*player2*/) { }

        // Called when a duel ends
        virtual void OnPlayerDuelEnd(Player* /*winner*/, Player* /*loser*/, DuelCompleteType /*type*/) { }

        // The following methods are called when a player sends a chat message.
        virtual void OnPlayerChat(Player* /*player*/, uint32 /*type*/, uint32 /*lang*/, std::string& /*msg*/) { }

        virtual void OnPlayerChat(Player* /*player*/, uint32 /*type*/, uint32 /*lang*/, std::string& /*msg*/, Player* /*receiver*/) { }

        virtual void OnPlayerChat(Player* /*player*/, uint32 /*type*/, uint32 /*lang*/, std::string& /*msg*/, Group* /*group*/) { }

        virtual void OnPlayerChat(Player* /*player*/, uint32 /*type*/, uint32 /*lang*/, std::string& /*msg*/, Guild* /*guild*/) { }

        virtual void OnPlayerChat(Player* /*player*/, uint32 /*type*/, uint32 /*lang*/, std::string& /*msg*/, Channel* /*channel*/) { }

        // Both of the below are called on emote opcodes.
        virtual void OnPlayerClearEmote(Player* /*player*/) { }

        virtual void OnPlayerTextEmote(Player* /*player*/, uint32 /*textEmote*/, uint32 /*emoteNum*/, ObjectGuid /*guid*/) { }

        // Called in Spell::Cast.
        virtual void OnPlayerSpellCast(Player* /*player*/, Spell* /*spell*/, bool /*skipCheck*/) { }

        // Called during data loading
        virtual void OnPlayerLoadFromDB(Player* /*player*/) { }

        // Called when a player logs in.
        virtual void OnPlayerLogin(Player* /*player*/) { }

        // Called when a player logs in for the first time
        virtual void OnPlayerFirstLogin(Player* /*player*/) { }

        // Called before the player is logged out
        virtual void OnPlayerBeforeLogout(Player* /*player*/) { }

        // Called when a player logs out.
        virtual void OnPlayerLogout(Player* /*player*/) { }

        // Called when a player is created.
        virtual void OnPlayerCreate(Player* /*player*/) { }

        // Called when a player is deleted.
        virtual void OnPlayerDelete(ObjectGuid /*guid*/, uint32 /*accountId*/) { }

        // Called when a player delete failed
        virtual void OnPlayerFailedDelete(ObjectGuid /*guid*/, uint32 /*accountId*/) { }

        // Called when a player is about to be saved.
        virtual void OnPlayerSave(Player* /*player*/) { }

        // Called when a player is bound to an instance
        virtual void OnPlayerBindToInstance(Player* /*player*/, Difficulty /*difficulty*/, uint32 /*mapId*/, bool /*permanent*/, uint8 /*extendState*/) { }

        // Called when a player switches to a new zone
        virtual void OnPlayerUpdateZone(Player* /*player*/, uint32 /*newZone*/, uint32 /*newArea*/) { }

        // Called when a player switches to a new area (more accurate than UpdateZone)
        virtual void OnPlayerUpdateArea(Player* /*player*/, uint32 /*oldArea*/, uint32 /*newArea*/) { }

        // Called when a player changes to a new map (after moving to new map)
        virtual void OnPlayerMapChanged(Player* /*player*/) { }

        // Called before a player is being teleported to new coords
        [[nodiscard]] virtual bool OnPlayerBeforeTeleport(Player* /*player*/, uint32 /*mapid*/, float /*x*/, float /*y*/, float /*z*/, float /*orientation*/, uint32 /*options*/, Unit* /*target*/) { return true; }

        // Called before a quest is completed, can be used to deny completion
        [[nodiscard]] virtual bool OnPlayerBeforeQuestComplete(Player* /*player*/, uint32 /*questId*/) { return true; }

        // Called after computing the XP reward value for a quest
        virtual void OnPlayerQuestComputeXP(Player* /*player*/, Quest const* /*quest*/, uint32& /*xpValue*/) { }

        // Before durability repair action, you can even modify the discount value
        virtual void OnPlayerBeforeDurabilityRepair(Player* /*player*/, ObjectGuid /*npcGUID*/, ObjectGuid /*itemGUID*/, float& /*discountMod*/, uint8 /*guildBank*/) { }

        // Called when the player's maximum level is set (before it is applied)
        virtual void OnPlayerSetMaxLevel(Player* /*player*/, uint32& /*maxPlayerLevel*/) { }

        // Called when rewarding a quest, true rewards money instead of experience
        virtual bool OnPlayerShouldBeRewardedWithMoneyInsteadOfExp(Player* /*player*/) { return false; }

        // Called before a player repops at a graveyard, can be used to deny the repop
        [[nodiscard]] virtual bool OnPlayerCanRepopAtGraveyard(Player* /*player*/) { return true; }

        // Called when a player resurrects
        virtual void OnPlayerResurrect(Player* /*player*/, float /*restorePercent*/, bool& /*applySickness*/) { }

        // Called after a player abandons a quest
        virtual void OnPlayerQuestAbandon(Player* /*player*/, uint32 /*questId*/) { }

        // Called before a player is given a new level, can be used to cancel the level up
        virtual bool OnPlayerCanGiveLevel(Player* /*player*/, uint8 /*newLevel*/) { return true; }

        // Called after a player's quest status has been changed
        virtual void OnPlayerQuestStatusChange(Player* /*player*/, uint32 /*questId*/) { }

        // Called when a player presses release when he died
        virtual void OnPlayerRepop(Player* /*player*/) { }

        // Called when the PvP flag of a player changes (Player::UpdatePvP)
        virtual void OnPlayerPVPFlagChange(Player* /*player*/, bool /*state*/) { }

        // Called when the free-for-all PvP state of a player is updated
        virtual void OnPlayerFfaPvpStateUpdate(Player* /*player*/, bool /*result*/) { }

        // Called before the honorable kill victim reward is calculated.
        // ShatterCore note: victimRank is uint32 (derived from the victim's
        // chosen title in 4.3.4) where AzerothCore uses int32.
        virtual void OnPlayerVictimRewardBefore(Player* /*player*/, Player* /*victim*/, uint32& /*killerTitle*/, uint32& /*victimRank*/) { }

        // Called after the honorable kill victim reward is calculated, honorF can still be modified
        virtual void OnPlayerVictimRewardAfter(Player* /*player*/, Player* /*victim*/, uint32& /*killerTitle*/, uint32& /*victimRank*/, float& /*honorF*/) { }

        // Called when a player enters combat
        virtual void OnPlayerEnterCombat(Player* /*player*/, Unit* /*enemy*/) { }

        // Called when a player leaves combat
        virtual void OnPlayerLeaveCombat(Player* /*player*/) { }

        // Called after a visible item slot of a player has been changed
        virtual void OnPlayerAfterSetVisibleItemSlot(Player* /*player*/, uint8 /*slot*/, Item* /*item*/) { }

        // Called after a player equips an item (also fires when an item is added to a stack in the slot)
        virtual void OnPlayerEquip(Player* /*player*/, Item* /*it*/, uint8 /*bag*/, uint8 /*slot*/, bool /*update*/) { }

        // Called after a player loots an item into his inventory
        virtual void OnPlayerLootItem(Player* /*player*/, Item* /*item*/, uint32 /*count*/, ObjectGuid /*lootguid*/) { }

        // Called after a new item is stored in the player's inventory
        virtual void OnPlayerStoreNewItem(Player* /*player*/, Item* /*item*/, uint32 /*count*/) { }

        // Called after an item is created for a player (crafting and other create-item spells)
        virtual void OnPlayerCreateItem(Player* /*player*/, Item* /*item*/, uint32 /*count*/) { }

        // Called after a player receives an item as a quest reward
        virtual void OnPlayerQuestRewardItem(Player* /*player*/, Item* /*item*/, uint32 /*count*/) { }

        // Called at the top of Player::BuyItemFromVendorSlot, item can be changed.
        // ShatterCore note: count is uint32 (32 bit wide in the 4.3.4 protocol) where AzerothCore uses uint8.
        virtual void OnPlayerBeforeBuyItemFromVendor(Player* /*player*/, ObjectGuid /*vendorguid*/, uint32 /*vendorslot*/, uint32& /*item*/, uint32 /*count*/, uint8 /*bag*/, uint8 /*slot*/) { }

        // Called before a bought item is stored or equipped, after the price has been paid. item can be changed.
        // ShatterCore note: count is uint32 (see OnPlayerBeforeBuyItemFromVendor).
        virtual void OnPlayerBeforeStoreOrEquipNewItem(Player* /*player*/, uint32 /*vendorslot*/, uint32& /*item*/, uint32 /*count*/, uint8 /*bag*/, uint8 /*slot*/, ItemTemplate const* /*pProto*/, Creature* /*pVendor*/, VendorItem const* /*crItem*/, bool /*bStore*/) { }

        // Called after a bought item has been stored or equipped, item may be nullptr if storing failed.
        // ShatterCore note: count is uint32 (see OnPlayerBeforeBuyItemFromVendor).
        virtual void OnPlayerAfterStoreOrEquipNewItem(Player* /*player*/, uint32 /*vendorslot*/, Item* /*item*/, uint32 /*count*/, uint8 /*bag*/, uint8 /*slot*/, ItemTemplate const* /*pProto*/, Creature* /*pVendor*/, VendorItem const* /*crItem*/, bool /*bStore*/) { }

        // Called before a player sells an item to a vendor, returning false denies the sale
        [[nodiscard]] virtual bool OnPlayerCanSellItem(Player* /*player*/, Item* /*item*/, Creature* /*creature*/) { return true; }

        // Called in Player::CanEquipItem, returning false denies the equip
        [[nodiscard]] virtual bool OnPlayerCanEquipItem(Player* /*player*/, uint8 /*slot*/, uint16& /*dest*/, Item* /*pItem*/, bool /*swap*/, bool /*not_loading*/) { return true; }

        // Called in Player::CanUnequipItem, returning false denies the unequip
        [[nodiscard]] virtual bool OnPlayerCanUnequipItem(Player* /*player*/, uint16 /*pos*/, bool /*swap*/) { return true; }

        // Called in Player::CanUseItem, returning false denies the use, the deny reason can be set through result
        [[nodiscard]] virtual bool OnPlayerCanUseItem(Player* /*player*/, ItemTemplate const* /*proto*/, InventoryResult& /*result*/) { return true; }

        // Called in WorldSession::HandleMessagechatOpcode after all validations and before the
        // message is routed, type, lang and msg may be modified
        virtual void OnPlayerBeforeSendChatMessage(Player* /*player*/, uint32& /*type*/, uint32& /*lang*/, std::string& /*msg*/) { }

        // Called when a single random dungeon is selected in LFGMgr::JoinLfg, the dungeon id may be modified
        virtual void OnPlayerQueueRandomDungeon(Player* /*player*/, uint32& /*rDungeonId*/) { }

        // Called when a player selects an option in a player gossip menu
        virtual void OnPlayerGossipSelect(Player* /*player*/, uint32 /*menu_id*/, uint32 /*sender*/, uint32 /*action*/) { }

        // Called when a player selects an option in a player gossip menu with a code box
        virtual void OnPlayerGossipSelectCode(Player* /*player*/, uint32 /*menu_id*/, uint32 /*sender*/, uint32 /*action*/, char const* /*code*/) { }

        // Called before mail is sent or returned to its sender, returning false blocks the mail
        // (ShatterCore keeps its 4.3.4 uint64 money types where AzerothCore uses uint32)
        [[nodiscard]] virtual bool OnPlayerCanSendMail(Player* /*player*/, ObjectGuid /*receiverGuid*/, ObjectGuid /*mailbox*/, std::string& /*subject*/, std::string& /*body*/, uint64 /*money*/, uint64 /*COD*/, Item* /*item*/) { return true; }

        // Called in WorldSession::HandlePetitionBuyOpcode after the charter is selected,
        // charter id, cost and type may be modified
        virtual void OnPlayerPetitionBuy(Player* /*player*/, Creature* /*creature*/, uint32& /*charterid*/, uint32& /*cost*/, uint32& /*type*/) { }

        // Called for every entry of the petition vendor list, the charter entry,
        // display id and cost may be modified
        virtual void OnPlayerPetitionShowList(Player* /*player*/, Creature* /*creature*/, uint32& /*CharterEntry*/, uint32& /*CharterDispayID*/, uint32& /*CharterCost*/) { }

        // Called at the top of LFGMgr::JoinLfg, returning false blocks the join attempt
        [[nodiscard]] virtual bool OnPlayerCanJoinLfg(Player* /*player*/, uint8 /*roles*/, lfg::LfgDungeonSet& /*dungeons*/, std::string const& /*comment*/) { return true; }

        // Called in WorldSession::HandleInitiateTradeOpcode, returning false blocks the trade
        [[nodiscard]] virtual bool OnPlayerCanInitTrade(Player* /*player*/, Player* /*target*/) { return true; }

        // Called before a say/yell/emote/AFK/DND message is sent, returning false blocks it
        [[nodiscard]] virtual bool OnPlayerCanUseChat(Player* /*player*/, uint32 /*type*/, uint32 /*language*/, std::string& /*msg*/) { return true; }

        // Called before a whisper is sent, returning false blocks it
        [[nodiscard]] virtual bool OnPlayerCanUseChat(Player* /*player*/, uint32 /*type*/, uint32 /*language*/, std::string& /*msg*/, Player* /*receiver*/) { return true; }

        // Called before a group/raid/battleground message is sent, returning false blocks it
        [[nodiscard]] virtual bool OnPlayerCanUseChat(Player* /*player*/, uint32 /*type*/, uint32 /*language*/, std::string& /*msg*/, Group* /*group*/) { return true; }

        // Called before a guild/officer message is sent, returning false blocks it
        [[nodiscard]] virtual bool OnPlayerCanUseChat(Player* /*player*/, uint32 /*type*/, uint32 /*language*/, std::string& /*msg*/, Guild* /*guild*/) { return true; }

        // Called before a channel message is sent, returning false blocks it
        [[nodiscard]] virtual bool OnPlayerCanUseChat(Player* /*player*/, uint32 /*type*/, uint32 /*language*/, std::string& /*msg*/, Channel* /*channel*/) { return true; }

        // Called when a player deserts a battleground or arena
        // (see BattlegroundDesertionType in Battleground.h)
        virtual void OnPlayerBattlegroundDesertion(Player* /*player*/, BattlegroundDesertionType /*desertionType*/) { }

        // Called when a player joins a battleground queue,
        // after the queue status packet has been sent
        virtual void OnPlayerJoinBG(Player* /*player*/) { }

        // Called when a player joins an arena queue,
        // after the queue status packet has been sent
        virtual void OnPlayerJoinArena(Player* /*player*/) { }

        // Called at the end of Player::GetMaxPersonalArenaRatingRequirement,
        // the rating cap used for item purchase requirements may be changed
        virtual void OnPlayerGetMaxPersonalArenaRatingRequirement(Player const* /*player*/, uint32 /*minSlot*/, uint32& /*maxArenaRating*/) { }

        // Called at the top of WorldSession::HandleBattlemasterJoinOpcode, returning false
        // denies the join; err may be set to the failure reason shown to the player.
        // ShatterCore note: 4.3.4 join results use 0 (ERR_BATTLEGROUND_NONE) for success and
        // positive error codes, unlike AzerothCore's 3.3.5a negative error codes. When the
        // join is denied and err is left at ERR_BATTLEGROUND_NONE, the core sends
        // ERR_BATTLEGROUND_JOIN_FAILED instead.
        [[nodiscard]] virtual bool OnPlayerCanJoinInBattlegroundQueue(Player* /*player*/, ObjectGuid /*battlemasterGuid*/, BattlegroundTypeId /*bgTypeId*/, uint8 /*joinAsGroup*/, GroupJoinBattlegroundResult& /*err*/) { return true; }

        // Called before the GM visibility of a player is changed, type and security may be modified
        virtual void OnPlayerSetServerSideVisibility(Player* /*player*/, ServerSideVisibilityType& /*type*/, AccountTypes& /*sec*/) { }

        // Called before the GM detection visibility of a player is changed, type and security may be modified
        virtual void OnPlayerSetServerSideVisibilityDetect(Player* /*player*/, ServerSideVisibilityType& /*type*/, AccountTypes& /*sec*/) { }

        // Called at the top of Player::LearnTalent, returning false denies the talent
        [[nodiscard]] virtual bool OnPlayerCanLearnTalent(Player* /*player*/, TalentEntry const* /*talent*/, uint32 /*rank*/) { return true; }

        // Called at the end of Player::ActivateSpec after the new spec has been applied
        virtual void OnPlayerAfterSpecSlotChanged(Player* /*player*/, uint8 /*newSlot*/) { }

        // Called at the end of Player::LearnTalent after the talent spell has been learned
        virtual void OnPlayerLearnTalents(Player* /*player*/, uint32 /*talentId*/, uint32 /*talentRank*/, uint32 /*spellid*/) { }

        // Passive Anticheat System
        virtual void AnticheatSetCanFlybyServer(Player* /*player*/, bool /*apply*/) { }
        virtual void AnticheatSetUnderACKmount(Player* /*player*/) { }
        virtual void AnticheatSetRootACKUpd(Player* /*player*/) { }
        virtual void AnticheatSetJumpingbyOpcode(Player* /*player*/, bool /*jump*/) { }
        virtual void AnticheatUpdateMovementInfo(Player* /*player*/, MovementInfo const& /*movementInfo*/) { }
        [[nodiscard]] virtual bool AnticheatHandleDoubleJump(Player* /*player*/, Unit* /*mover*/) { return true; }
        [[nodiscard]] virtual bool AnticheatCheckMovementInfo(Player* /*player*/, MovementInfo const& /*movementInfo*/, Unit* /*mover*/, bool /*jump*/) { return true; }
};

#endif // SC_PLAYER_SCRIPT_H
