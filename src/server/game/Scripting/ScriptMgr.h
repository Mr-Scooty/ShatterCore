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

#ifndef SC_SCRIPTMGR_H
#define SC_SCRIPTMGR_H

#include "Common.h"
#include "ObjectGuid.h"
#include "ScriptDefines/ScriptObject.h"
#include "ScriptDefines/AccountScript.h"
#include "ScriptDefines/AchievementCriteriaScript.h"
#include "ScriptDefines/AchievementScript.h"
#include "ScriptDefines/AllBattlegroundScript.h"
#include "ScriptDefines/AllCommandScript.h"
#include "ScriptDefines/AllCreatureScript.h"
#include "ScriptDefines/AllGameObjectScript.h"
#include "ScriptDefines/AllItemScript.h"
#include "ScriptDefines/AllMapScript.h"
#include "ScriptDefines/AllSpellScript.h"
#include "ScriptDefines/AreaTriggerScript.h"
#include "ScriptDefines/ArenaScript.h"
#include "ScriptDefines/ArenaTeamScript.h"
#include "ScriptDefines/AuctionHouseScript.h"
#include "ScriptDefines/BattlefieldScript.h"
#include "ScriptDefines/BattlegroundMapScript.h"
#include "ScriptDefines/BattlegroundScript.h"
#include "ScriptDefines/CommandScript.h"
#include "ScriptDefines/ConditionScript.h"
#include "ScriptDefines/CreatureScript.h"
#include "ScriptDefines/DatabaseScript.h"
#include "ScriptDefines/DynamicObjectScript.h"
#include "ScriptDefines/FormulaScript.h"
#include "ScriptDefines/GameEventScript.h"
#include "ScriptDefines/GameObjectScript.h"
#include "ScriptDefines/GlobalScript.h"
#include "ScriptDefines/GroupScript.h"
#include "ScriptDefines/GuildScript.h"
#include "ScriptDefines/InstanceMapScript.h"
#include "ScriptDefines/ItemScript.h"
#include "ScriptDefines/LootScript.h"
#include "ScriptDefines/MailScript.h"
#include "ScriptDefines/MiscScript.h"
#include "ScriptDefines/ModuleScript.h"
#include "ScriptDefines/MovementHandlerScript.h"
#include "ScriptDefines/OutdoorPvPScript.h"
#include "ScriptDefines/PetScript.h"
#include "ScriptDefines/PlayerScript.h"
#include "ScriptDefines/PlayerbotsScript.h"
#include "ScriptDefines/ServerScript.h"
#include "ScriptDefines/SpellScriptLoader.h"
#include "ScriptDefines/TicketScript.h"
#include "ScriptDefines/TransportScript.h"
#include "ScriptDefines/UnitScript.h"
#include "ScriptDefines/VehicleScript.h"
#include "ScriptDefines/WeatherScript.h"
#include "ScriptDefines/WorldMapScript.h"
#include "ScriptDefines/WorldObjectScript.h"
#include "ScriptDefines/WorldScript.h"
#include "ScriptDefines/WorldStateScript.h"
#include <vector>

class AccountMgr;
class AuctionHouseMgr;
class AuctionHouseObject;
class Aura;
class AuraApplication;
class AuraEffect;
class AuraScript;
class Battlefield;
class Battleground;
class BattlegroundMap;
class Channel;
class ChatCommand;
class ChatHandler;
class Creature;
class GmTicket;
class CreatureAI;
class DynamicObject;
class GameObject;
class GameObjectAI;
class Guardian;
class Guild;
class Group;
class Pet;
class InstanceMap;
class InstanceSave;
class InstanceScript;
class Item;
class LootStore;
class LootTemplate;
class Object;
class MailDraft;
class MailReceiver;
class MailSender;
class Map;
class ModuleReference;
class OutdoorPvP;
class Player;
class Quest;
class ScriptMgr;
class Spell;
class SpellInfo;
class SpellScript;
class SpellCastTargets;
class Transport;
class Unit;
class Vehicle;
class Weather;
class WorldPacket;
class WorldSocket;
class WorldObject;
class WorldSession;

struct AchievementCriteriaEntry;
struct AchievementEntry;
struct AuctionEntry;
struct AreaTriggerEntry;
struct ConditionSourceInfo;
struct Condition;
struct CreatureTemplate;
struct CreatureData;
struct ItemTemplate;
struct LootStoreItem;
struct MapEntry;
struct Position;
struct VendorItem;
struct WorldStateTemplate;

enum BattlegroundTypeId : uint32;
enum ContentLevels : uint8;
enum Difficulty : uint8;
enum DuelCompleteType : uint8;
enum InventoryResult : uint8;
enum MailCheckMask : uint8;
enum QuestStatus : uint8;
enum RemoveMethod : uint8;
enum ShutdownExitCode : uint32;
enum ShutdownMask : uint32;
enum SpellEffIndex : uint8;
enum WeatherState : uint32;
enum XPColorChar : uint8;

#define VISIBLE_RANGE       166.0f                          //MAX visible range (size of grid)

/*
    Standard procedure when adding new script type classes:

    Each script type lives in its own pair of files under
    Scripting/ScriptDefines/, e.g. MyScriptType.h and MyScriptType.cpp.

    The header defines the script type class, inheriting from ScriptObject
    (see ScriptDefines/ScriptObject.h), together with an AzerothCore
    compatible hook enum when the type is not database bound:

    // MyScriptType.h
    enum MyHook : uint16
    {
        MYHOOK_ON_SOME_EVENT,
        MYHOOK_END
    };

    class TC_GAME_API MyScriptType
        : public ScriptObject
    {
        protected:

            MyScriptType(char const* name, std::vector<uint16> enabledHooks = std::vector<uint16>());

        public:

            // If a virtual function in your script type class is not necessarily
            // required to be overridden, just declare it virtual with an empty
            // body. If, on the other hand, it's logical only to override it (i.e.
            // if it's the only method in the class), make it pure virtual, by adding
            // = 0 to it.
            virtual void OnSomeEvent(uint32 someArg1, std::string& someArg2) { }
    };

    The .cpp file defines the constructor, the ScriptMgr dispatch methods and
    the registry instantiation (ScriptRegistry.h is internal to Scripting/):

    // MyScriptType.cpp
    #include "MyScriptType.h"
    #include "ScriptMgr.h"
    #include "ScriptRegistry.h"

    MyScriptType::MyScriptType(char const* name, std::vector<uint16> enabledHooks)
        : ScriptObject(name)  // enabledHooks is accepted for AzerothCore compatibility
    {
        ScriptRegistry<MyScriptType>::Instance()->AddScript(this);
    }

    void ScriptMgr::OnSomeEvent(uint32 someArg1, std::string& someArg2)
    {
        FOREACH_SCRIPT(MyScriptType)->OnSomeEvent(someArg1, someArg2);
    }

    template class TC_GAME_API ScriptRegistry<MyScriptType>;

    Veto style hooks use FOR_SCRIPTS_RET and return the default value when no
    script is registered:

    bool ScriptMgr::OnAnotherEvent(uint32 someArg)
    {
        FOR_SCRIPTS_RET(MyScriptType, itr, end, true)
            if (!itr->second->OnAnotherEvent(someArg))
                return false;

        return true;
    }

    Finally, declare the dispatch methods in the ScriptMgr class below, add
    the header to the umbrella include list at the top of this file (modules
    and scripts only include ScriptMgr.h) and call the dispatch methods from
    the core to trigger the events on all registered scripts of that type.
*/

// Manages registration, loading, and execution of scripts.
class TC_GAME_API ScriptMgr
{
    friend class ScriptObject;

    private:
        ScriptMgr();
        virtual ~ScriptMgr();

        void FillSpellSummary();
        void LoadDatabase();

        void IncreaseScriptCount() { ++_scriptCount; }
        void DecreaseScriptCount() { --_scriptCount; }

    public: /* Initialization */
        static ScriptMgr* instance();

        void Initialize();

        uint32 GetScriptCount() const { return _scriptCount; }

        typedef void(*ScriptLoaderCallbackType)();

        /// Sets the script loader callback which is invoked to load scripts
        /// (Workaround for circular dependency game <-> scripts)
        void SetScriptLoader(ScriptLoaderCallbackType script_loader_callback)
        {
            _script_loader_callback = script_loader_callback;
        }

        /// Sets the modules loader callback which is invoked to load
        /// statically linked module scripts
        /// (Workaround for circular dependency game <-> modules)
        void SetModulesLoader(ScriptLoaderCallbackType modules_loader_callback)
        {
            _modules_loader_callback = modules_loader_callback;
        }

    public: /* Script contexts */
        /// Set the current script context, which allows the ScriptMgr
        /// to accept new scripts in this context.
        /// Requires a SwapScriptContext() call afterwards to load the new scripts.
        void SetScriptContext(std::string const& context);
        /// Returns the current script context.
        std::string const& GetCurrentScriptContext() const { return _currentContext; }
        /// Releases all scripts associated with the given script context immediately.
        /// Requires a SwapScriptContext() call afterwards to finish the unloading.
        void ReleaseScriptContext(std::string const& context);
        /// Executes all changed introduced by SetScriptContext and ReleaseScriptContext.
        /// It is possible to combine multiple SetScriptContext and ReleaseScriptContext
        /// calls for better performance (bulk changes).
        void SwapScriptContext(bool initialize = false);

        /// Returns the context name of the static context provided by the worldserver
        static std::string const& GetNameOfStaticContext();

        /// Returns the context name of the statically linked modules
        /// provided by the worldserver
        static std::string const& GetNameOfModulesContext();

        /// Acquires a strong module reference to the module containing the given script name,
        /// which prevents the shared library which contains the script from unloading.
        /// The shared library is lazy unloaded as soon as all references to it are released.
        std::shared_ptr<ModuleReference> AcquireModuleReferenceOfScriptName(
            std::string const& scriptname) const;

    public: /* Unloading */

        void Unload();

    public: /* DatabaseScript */

        void OnAfterDatabasesLoaded(uint32 updateFlags);
        bool OnDatabasesLoading();
        void OnDatabasesKeepAlive();
        void OnDatabasesClosing();
        void OnDatabaseWarnAboutSyncQueries(bool apply);
        void OnDatabaseSelectIndexLogout(Player* player, uint32& statementIndex, uint32& statementParam);
        void OnDatabaseGetDBRevision(std::string& revision);

    public: /* PlayerbotScript */

        bool OnPlayerbotCheckLFGQueue(GuidList const& guidsList);
        void OnPlayerbotCheckKillTask(Player* player, Unit* victim);
        void OnPlayerbotCheckPetitionAccount(Player* player, bool& found);
        bool OnPlayerbotCheckUpdatesToSend(Player* player);
        void OnPlayerbotPacketSent(Player* player, WorldPacket const* packet);
        void OnPlayerbotUpdate(uint32 diff);
        void OnPlayerbotUpdateSessions(Player* player);
        void OnPlayerbotLogout(Player* player);
        void OnPlayerbotLogoutBots();

    public: /* SpellScriptLoader */

        void CreateSpellScripts(uint32 spellId, std::vector<SpellScript*>& scriptVector, Spell* invoker) const;
        void CreateAuraScripts(uint32 spellId, std::vector<AuraScript*>& scriptVector, Aura* invoker) const;
        SpellScriptLoader* GetSpellScriptLoader(uint32 scriptId);

    public: /* ServerScript */

        void OnNetworkStart();
        void OnNetworkStop();
        void OnSocketOpen(std::shared_ptr<WorldSocket> socket);
        void OnSocketClose(std::shared_ptr<WorldSocket> socket);
        void OnPacketReceive(WorldSession* session, WorldPacket const& packet);
        void OnPacketSend(WorldSession* session, WorldPacket const& packet);
        bool CanPacketReceive(WorldSession* session, WorldPacket const& packet);
        bool CanPacketSend(WorldSession* session, WorldPacket const& packet);
        void OnPacketReceived(WorldSession* session, WorldPacket const& packet);

    public: /* WorldScript */

        void OnOpenStateChange(bool open);
        void OnConfigLoad(bool reload);
        void OnBeforeConfigLoad(bool reload);
        void OnLoadCustomDatabaseTable();
        void OnMotdChange(std::string& newMotd);
        void OnShutdownInitiate(ShutdownExitCode code, ShutdownMask mask);
        void OnShutdownCancel();
        void OnWorldUpdate(uint32 diff);
        void OnStartup();
        void OnShutdown();
        void OnAfterUnloadAllMaps();
        void OnBeforeWorldInitialized();
        void OnBeforeFinalizePlayerWorldSession(uint32& cacheVersion);

    public: /* FormulaScript */

        void OnHonorCalculation(float& honor, uint8 level, float multiplier);
        void OnGrayLevelCalculation(uint8& grayLevel, uint8 playerLevel);
        void OnColorCodeCalculation(XPColorChar& color, uint8 playerLevel, uint8 mobLevel);
        void OnZeroDifferenceCalculation(uint8& diff, uint8 playerLevel);
        void OnBaseGainCalculation(uint32& gain, uint8 playerLevel, uint8 mobLevel, ContentLevels content);
        void OnGainCalculation(uint32& gain, Player* player, Unit* unit);
        void OnGroupRateCalculation(float& rate, uint32 count, bool isRaid);
        void OnAfterArenaRatingCalculation(Battleground* const bg, int32& winnerMatchmakerChange, int32& loserMatchmakerChange, int32& winnerChange, int32& loserChange);
        void OnBeforeUpdatingPersonalRating(int32& mod, uint32 type);

    public: /* MapScript + AllMapScript */

        void OnCreateMap(Map* map);
        void OnDestroyMap(Map* map);
        void OnPlayerEnterMap(Map* map, Player* player);
        void OnPlayerLeaveMap(Map* map, Player* player);
        void OnMapUpdate(Map* map, uint32 diff);
        void OnBeforeCreateInstanceScript(InstanceMap* instanceMap, InstanceScript** instanceData, bool load, std::string data, uint32 completedEncounterMask);

    public: /* InstanceMapScript */

        InstanceScript* CreateInstanceData(InstanceMap* map);

    public: /* ItemScript + AllItemScript */

        bool OnQuestAccept(Player* player, Item* item, Quest const* quest);
        bool OnItemUse(Player* player, Item* item, SpellCastTargets const& targets);
        bool OnItemExpire(Player* player, ItemTemplate const* proto);
        bool OnItemRemove(Player* player, Item* item);
        bool OnCastItemCombatSpell(Player* player, Unit* victim, SpellInfo const* spellInfo, Item* item);
        void OnGossipSelect(Player* player, Item* item, uint32 sender, uint32 action);
        void OnGossipSelectCode(Player* player, Item* item, uint32 sender, uint32 action, char const* code);

    public: /* CreatureScript */

        CreatureAI* GetCreatureAI(Creature* creature);

    public: /* AllCreatureScript */

        void OnCreatureAddWorld(Creature* creature);
        void OnCreatureRemoveWorld(Creature* creature);
        void OnCreatureSaveToDB(Creature* creature);
        void OnBeforeCreatureSelectLevel(CreatureTemplate const* cinfo, Creature* creature, uint8& level);
        void OnCreatureSelectLevel(CreatureTemplate const* cinfo, Creature* creature);
        void OnCreatureUpdate(Creature* creature, uint32 diff);
        bool CanCreatureGossipHello(Player* player, Creature* creature);
        bool CanCreatureGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action);
        bool CanCreatureGossipSelectCode(Player* player, Creature* creature, uint32 sender, uint32 action, char const* code);
        bool CanCreatureQuestAccept(Player* player, Creature* creature, Quest const* quest);
        bool CanCreatureQuestReward(Player* player, Creature* creature, Quest const* quest, uint32 opt);

    public: /* GameObjectScript */

        GameObjectAI* GetGameObjectAI(GameObject* go);

    public: /* AllGameObjectScript */

        void OnGameObjectAddWorld(GameObject* go);
        void OnGameObjectRemoveWorld(GameObject* go);
        void OnGameObjectSaveToDB(GameObject* go);
        void OnGameObjectUpdate(GameObject* go, uint32 diff);
        bool CanGameObjectGossipHello(Player* player, GameObject* go);
        bool CanGameObjectGossipSelect(Player* player, GameObject* go, uint32 sender, uint32 action);
        bool CanGameObjectGossipSelectCode(Player* player, GameObject* go, uint32 sender, uint32 action, char const* code);
        bool CanGameObjectQuestAccept(Player* player, GameObject* go, Quest const* quest);
        bool CanGameObjectQuestReward(Player* player, GameObject* go, Quest const* quest, uint32 opt);
        void OnGameObjectDamaged(GameObject* go, Player* player);
        void OnGameObjectDestroyed(GameObject* go, Player* player);
        void OnGameObjectModifyHealth(GameObject* go, WorldObject* attackerOrHealer, int32& change, SpellInfo const* spellInfo);
        void OnGameObjectLootStateChanged(GameObject* go, uint32 state, Unit* unit);
        void OnGameObjectStateChanged(GameObject* go, uint32 state);

    public: /* WorldObjectScript */

        void OnWorldObjectCreate(WorldObject* object);
        void OnWorldObjectDestroy(WorldObject* object);
        void OnWorldObjectSetMap(WorldObject* object, Map* map);
        void OnWorldObjectResetMap(WorldObject* object);

    public: /* GlobalScript */

        void OnGlobalItemDelFromDB(CharacterDatabaseTransaction trans, ObjectGuid::LowType itemGuid);
        void OnGlobalMirrorImageDisplayItem(Item const* item, uint32& display);
        void OnInitializeLockedDungeons(Player* player, uint8& level, uint32& lockData, lfg::LFGDungeonData const* dungeon);
        void OnAfterInitializeLockedDungeons(Player* player);
        void OnAfterUpdateEncounterState(Map* map, EncounterCreditType type, uint32 creditEntry, Unit* source, Difficulty difficulty_fixed, std::list<DungeonEncounter const*> const* encounters, uint32 dungeonCompleted, bool updated);
        void OnAfterRefCount(Player const* player, Loot& loot, bool canRate, uint16 lootMode, LootStoreItem* lootStoreItem, uint32& maxcount, LootStore const& store);
        void OnBeforeDropAddItem(Player const* player, Loot& loot, bool canRate, uint16 lootMode, LootStoreItem* lootStoreItem, LootStore const& store);
        bool OnItemRoll(Player const* player, LootStoreItem const* lootStoreItem, float& chance, Loot& loot, LootStore const& store);
        bool OnBeforeLootEqualChanced(Player const* player, std::list<LootStoreItem*> equalChanced, Loot& loot, LootStore const& store);
        bool OnIsAffectedBySpellModCheck(SpellInfo const* affectSpell, SpellInfo const* checkSpell, SpellModifier const* mod);
        bool OnSpellHealingBonusTakenNegativeModifiers(Unit const* target, Unit const* caster, SpellInfo const* spellInfo, float& val);
        void OnLoadSpellCustomAttr(SpellInfo* spell);
        bool OnAllowedToLootContainerCheck(Player const* player, ObjectGuid source);
        void OnInstanceIdRemoved(uint32 instanceId);
        void OnBeforeSetBossState(uint32 id, EncounterState newState, EncounterState oldState, Map* instance);
        void AfterInstanceGameObjectCreate(Map* instance, GameObject* go);

    public: /* AreaTriggerScript */

        bool OnAreaTrigger(Player* player, AreaTriggerEntry const* trigger);

    public: /* BattlefieldScript */

        Battlefield* CreateBattlefield(uint32 scriptId, Map* map);

    public: /* AllBattlefieldScript */

        void OnBattlefieldPlayerEnterZone(Battlefield* bf, Player* player);
        void OnBattlefieldPlayerLeaveZone(Battlefield* bf, Player* player);
        void OnBattlefieldPlayerJoinWar(Battlefield* bf, Player* player);
        void OnBattlefieldPlayerLeaveWar(Battlefield* bf, Player* player);
        void OnBattlefieldBeforeInvitePlayerToWar(Battlefield* bf, Player* player);
        void OnBattlefieldWarEnd(Battlefield* bf, bool endByTimer);

    public: /* BattlegroundScript */

        Battleground* CreateBattleground(BattlegroundTypeId typeId);

    public: /* AllBattlegroundScript */

        void OnBattlegroundStart(Battleground* bg);
        void OnBattlegroundEndReward(Battleground* bg, Player* player, uint32 winnerTeam);
        void OnBattlegroundUpdate(Battleground* bg, uint32 diff);
        void OnBattlegroundAddPlayer(Battleground* bg, Player* player);
        void OnBattlegroundBeforeAddPlayer(Battleground* bg, Player* player);
        void OnBattlegroundRemovePlayerAtLeave(Battleground* bg, Player* player);
        void OnQueueUpdate(BattlegroundQueue* queue, uint32 diff, BattlegroundTypeId bgTypeId, BattlegroundBracketId bracketId, uint8 arenaType, bool isRated, uint32 arenaRating);
        bool OnQueueUpdateValidity(BattlegroundQueue* queue, uint32 diff, BattlegroundTypeId bgTypeId, BattlegroundBracketId bracketId, uint8 arenaType, bool isRated, uint32 arenaRating);
        void OnAddGroup(BattlegroundQueue* queue, GroupQueueInfo* ginfo, uint32& index, Player* leader, Group* group, BattlegroundTypeId bgTypeId, PvPDifficultyEntry const* bracketEntry,
            uint8 arenaType, bool isRated, bool isPremade, uint32 arenaRating, uint32 matchmakerRating, uint32 arenaTeamId, uint32 opponentsArenaTeamId);
        bool CanFillPlayersToBG(BattlegroundQueue* queue, Battleground* bg, BattlegroundBracketId bracketId);
        bool IsCheckNormalMatch(BattlegroundQueue* queue, Battleground* bgTemplate, BattlegroundBracketId bracketId, uint32 minPlayers, uint32 maxPlayers);
        bool CanSendMessageBGQueue(BattlegroundQueue* queue, Player* leader, Battleground* bg, PvPDifficultyEntry const* bracketEntry);
        bool OnBeforeSendJoinMessageArenaQueue(BattlegroundQueue* queue, Player* leader, GroupQueueInfo* ginfo, PvPDifficultyEntry const* bracketEntry, bool isRated);
        bool OnBeforeSendExitMessageArenaQueue(BattlegroundQueue* queue, GroupQueueInfo* ginfo);
        void OnBattlegroundEnd(Battleground* bg, uint32 winnerTeam);
        void OnBattlegroundDestroy(Battleground* bg);
        void OnBattlegroundCreate(Battleground* bg);
        bool CanAddGroupToMatchingPool(BattlegroundQueue* queue, GroupQueueInfo* group, uint32 poolPlayerCount, Battleground* bg, BattlegroundBracketId bracketId);
        bool GetPlayerMatchmakingRating(ObjectGuid playerGuid, BattlegroundTypeId bgTypeId, float& outRating);

    public: /* ArenaScript */

        bool CanAddMember(ArenaTeam* team, ObjectGuid playerGuid);
        bool CanSaveToDB(ArenaTeam* team);
        bool OnBeforeArenaCheckWinConditions(Battleground* bg);
        void OnArenaStart(Battleground* bg);
        bool OnBeforeArenaTeamMemberUpdate(ArenaTeam* team, Player* player, bool won, uint32 opponentMatchmakerRating, int32 matchmakerChange);
        bool CanSaveArenaStatsForMember(ArenaTeam* team, ObjectGuid playerGuid);

    public: /* ArenaTeamScript */

        void OnGetSlotByType(uint32 type, uint8& slot);
        void OnArenaTypeIDToQueueID(BattlegroundTypeId bgTypeId, uint8 arenaType, uint32& queueTypeID);
        void OnArenaQueueIdToArenaType(BattlegroundQueueTypeId bgQueueTypeId, uint8& arenaType);
        void OnSetArenaMaxPlayersPerTeam(uint8 arenaType, uint32& maxPlayersPerTeam);

    public: /* OutdoorPvPScript */

        OutdoorPvP* CreateOutdoorPvP(uint32 scriptId, Map* map);

    public: /* CommandScript */

        std::vector<ChatCommand> GetChatCommands();

    public: /* WeatherScript */

        void OnWeatherChange(Weather* weather, WeatherState state, float grade);
        void OnWeatherUpdate(Weather* weather, uint32 diff);

    public: /* AuctionHouseScript */

        void OnAuctionAdd(AuctionHouseObject* ah, AuctionEntry* entry);
        void OnAuctionRemove(AuctionHouseObject* ah, AuctionEntry* entry);
        void OnAuctionSuccessful(AuctionHouseObject* ah, AuctionEntry* entry);
        void OnAuctionExpire(AuctionHouseObject* ah, AuctionEntry* entry);
        void OnBeforeAuctionHouseMgrSendAuctionWonMail(AuctionHouseMgr* auctionHouseMgr, AuctionEntry* auction, Player* bidder, uint32& bidder_accId, bool& sendNotification, bool& updateAchievementCriteria, bool& sendMail);
        void OnBeforeAuctionHouseMgrSendAuctionSalePendingMail(AuctionHouseMgr* auctionHouseMgr, AuctionEntry* auction, Player* owner, uint32& owner_accId, bool& sendMail);
        void OnBeforeAuctionHouseMgrSendAuctionSuccessfulMail(AuctionHouseMgr* auctionHouseMgr, AuctionEntry* auction, Player* owner, uint32& owner_accId, uint64& profit, bool& sendNotification, bool& updateAchievementCriteria, bool& sendMail);
        void OnBeforeAuctionHouseMgrSendAuctionExpiredMail(AuctionHouseMgr* auctionHouseMgr, AuctionEntry* auction, Player* owner, uint32& owner_accId, bool& sendNotification, bool& sendMail);
        void OnBeforeAuctionHouseMgrSendAuctionOutbiddedMail(AuctionHouseMgr* auctionHouseMgr, AuctionEntry* auction, Player* oldBidder, uint32& oldBidder_accId, Player* newBidder, uint64& newPrice, bool& sendNotification, bool& sendMail);
        void OnBeforeAuctionHouseMgrSendAuctionCancelledToBidderMail(AuctionHouseMgr* auctionHouseMgr, AuctionEntry* auction, Player* bidder, uint32& bidder_accId, bool& sendMail);
        void OnBeforeAuctionHouseMgrUpdate();

    public: /* LootScript */

        void OnLootMoney(Player* player, uint32 gold);

    public: /* MailScript */

        void OnBeforeMailDraftSendMailTo(MailDraft* mailDraft, MailReceiver const& receiver, MailSender const& sender, MailCheckMask& checked, uint32& deliver_delay, uint32& custom_expiration, bool& deleteMailItemsFromDB, bool& sendMail);

    public: /* MiscScript */

        void OnConstructObject(Object* origin);
        void OnDestructObject(Object* origin);
        void OnConstructPlayer(Player* origin);
        void OnDestructPlayer(Player* origin);
        void OnConstructGroup(Group* origin);
        void OnDestructGroup(Group* origin);
        void OnConstructInstanceSave(InstanceSave* origin);
        void OnDestructInstanceSave(InstanceSave* origin);
        void OnItemCreate(Item* item, ItemTemplate const* itemProto, Player const* owner);
        bool CanApplySoulboundFlag(Item* item, ItemTemplate const* proto);
        bool CanItemApplyEquipSpell(Player* player, Item* item);
        bool CanSendAuctionHello(WorldSession const* session, ObjectGuid guid, Creature* creature);
        void OnAfterLootTemplateProcess(Loot* loot, LootTemplate const* tab, LootStore const& store, Player* lootOwner, bool personal, bool noEmptyError, uint16 lootMode);
        void OnInstanceSave(InstanceSave* instanceSave);
        void GetDialogStatus(Player* player, Object* questgiver);

    public: /* ConditionScript */

        bool OnConditionCheck(Condition const* condition, ConditionSourceInfo& sourceInfo);

    public: /* VehicleScript */

        void OnInstall(Vehicle* veh);
        void OnUninstall(Vehicle* veh);
        void OnReset(Vehicle* veh);
        void OnInstallAccessory(Vehicle* veh, Creature* accessory);
        void OnAddPassenger(Vehicle* veh, Unit* passenger, int8 seatId);
        void OnRemovePassenger(Vehicle* veh, Unit* passenger);

    public: /* DynamicObjectScript */

        void OnDynamicObjectUpdate(DynamicObject* dynobj, uint32 diff);

    public: /* TransportScript */

        void OnAddPassenger(Transport* transport, Player* player);
        void OnAddCreaturePassenger(Transport* transport, Creature* creature);
        void OnRemovePassenger(Transport* transport, Player* player);
        void OnTransportUpdate(Transport* transport, uint32 diff);
        void OnRelocate(Transport* transport, uint32 mapId, float x, float y, float z);

    public: /* AchievementCriteriaScript */

        bool OnCriteriaCheck(uint32 scriptId, Player* source, Unit* target);

    public: /* PlayerScript */

        void OnPVPKill(Player* killer, Player* killed);
        void OnCreatureKill(Player* killer, Creature* killed);
        void OnPlayerKilledByCreature(Creature* killer, Player* killed);
        void OnPlayerJustDied(Player* player);
        void OnPlayerReleasedGhost(Player* player);
        void OnPlayerLevelChanged(Player* player, uint8 oldLevel);
        bool OnPlayerCanGiveLevel(Player* player, uint8 newLevel);
        void OnPlayerSetMaxLevel(Player* player, uint32& maxPlayerLevel);
        void OnPlayerFreeTalentPointsChanged(Player* player, uint32 newPoints);
        void OnPlayerTalentsReset(Player* player, bool noCost);
        void OnPlayerBeforeUpdate(Player* player, uint32 p_time);
        void OnPlayerUpdate(Player* player, uint32 p_time);
        void OnPlayerAfterUpdate(Player* player, uint32 p_time);
        bool OnPlayerBeforeAchievementComplete(Player* player, AchievementEntry const* achievement);
        bool OnPlayerBeforeCriteriaProgress(Player* player, AchievementCriteriaEntry const* criteria);
        void OnPlayerMoneyChanged(Player* player, int64& amount);
        void OnPlayerMoneyLimit(Player* player, int64 amount);
        void OnPlayerBeforeLootMoney(Player* player, Loot* loot);
        void OnGivePlayerXP(Player* player, uint32& amount, Unit* victim, uint8 xpSource = 0 /*XPSOURCE_KILL*/);
        bool OnPlayerShouldBeRewardedWithMoneyInsteadOfExp(Player* player);
        void OnPlayerReputationChange(Player* player, uint32 factionID, int32& standing, bool incremental);
        void OnPlayerReputationRankChange(Player* player, uint32 factionID, ReputationRank newRank, ReputationRank oldRank, bool increased);
        void OnPlayerGiveReputation(Player* player, int32 factionID, float& amount, ReputationSource repSource);
        void OnPlayerDuelRequest(Player* target, Player* challenger);
        void OnPlayerDuelStart(Player* player1, Player* player2);
        void OnPlayerDuelEnd(Player* winner, Player* loser, DuelCompleteType type);
        void OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg);
        void OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg, Player* receiver);
        void OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg, Group* group);
        void OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg, Guild* guild);
        void OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg, Channel* channel);
        void OnPlayerClearEmote(Player* player);
        void OnPlayerTextEmote(Player* player, uint32 textEmote, uint32 emoteNum, ObjectGuid guid);
        void OnPlayerSpellCast(Player* player, Spell* spell, bool skipCheck);
        void OnPlayerLoadFromDB(Player* player);
        void OnPlayerLogin(Player* player, bool firstLogin);
        void OnPlayerBeforeLogout(Player* player);
        void OnPlayerLogout(Player* player);
        void OnPlayerCreate(Player* player);
        void OnPlayerDelete(ObjectGuid guid, uint32 accountId);
        void OnPlayerFailedDelete(ObjectGuid guid, uint32 accountId);
        void OnPlayerSave(Player* player);
        void OnPlayerBindToInstance(Player* player, Difficulty difficulty, uint32 mapid, bool permanent, uint8 extendState);
        void OnPlayerUpdateZone(Player* player, uint32 newZone, uint32 newArea);
        void OnPlayerUpdateArea(Player* player, uint32 oldArea, uint32 newArea);
        bool OnPlayerBeforeTeleport(Player* player, uint32 mapid, float x, float y, float z, float orientation, uint32 options, Unit* target);
        bool OnPlayerCanRepopAtGraveyard(Player* player);
        void OnPlayerResurrect(Player* player, float restorePercent, bool& applySickness);
        void OnPlayerBeforeDurabilityRepair(Player* player, ObjectGuid npcGUID, ObjectGuid itemGUID, float& discountMod, uint8 guildBank);
        void OnPlayerCompleteQuest(Player* player, Quest const* quest);
        bool OnPlayerBeforeQuestComplete(Player* player, uint32 questId);
        void OnPlayerQuestComputeXP(Player* player, Quest const* quest, uint32& xpValue);
        void OnPlayerQuestAbandon(Player* player, uint32 questId);
        void OnQuestStatusChange(Player* player, uint32 questId);
        void OnPlayerRepop(Player* player);
        void OnPlayerPVPFlagChange(Player* player, bool state);
        void OnPlayerFfaPvpStateUpdate(Player* player, bool result);
        void OnPlayerVictimRewardBefore(Player* player, Player* victim, uint32& killerTitle, uint32& victimRank);
        void OnPlayerVictimRewardAfter(Player* player, Player* victim, uint32& killerTitle, uint32& victimRank, float& honorF);
        void OnPlayerEnterCombat(Player* player, Unit* enemy);
        void OnPlayerLeaveCombat(Player* player);
        void OnPlayerAfterSetVisibleItemSlot(Player* player, uint8 slot, Item* item);
        void OnPlayerEquip(Player* player, Item* it, uint8 bag, uint8 slot, bool update);
        void OnPlayerLootItem(Player* player, Item* item, uint32 count, ObjectGuid lootguid);
        void OnPlayerStoreNewItem(Player* player, Item* item, uint32 count);
        void OnPlayerCreateItem(Player* player, Item* item, uint32 count);
        void OnPlayerQuestRewardItem(Player* player, Item* item, uint32 count);
        void OnPlayerBeforeBuyItemFromVendor(Player* player, ObjectGuid vendorguid, uint32 vendorslot, uint32& item, uint32 count, uint8 bag, uint8 slot);
        void OnPlayerBeforeStoreOrEquipNewItem(Player* player, uint32 vendorslot, uint32& item, uint32 count, uint8 bag, uint8 slot, ItemTemplate const* pProto, Creature* pVendor, VendorItem const* crItem, bool bStore);
        void OnPlayerAfterStoreOrEquipNewItem(Player* player, uint32 vendorslot, Item* item, uint32 count, uint8 bag, uint8 slot, ItemTemplate const* pProto, Creature* pVendor, VendorItem const* crItem, bool bStore);
        bool OnPlayerCanSellItem(Player* player, Item* item, Creature* creature);
        bool OnPlayerCanEquipItem(Player* player, uint8 slot, uint16& dest, Item* pItem, bool swap, bool not_loading);
        bool OnPlayerCanUnequipItem(Player* player, uint16 pos, bool swap);
        bool OnPlayerCanUseItem(Player* player, ItemTemplate const* proto, InventoryResult& result);
        void OnPlayerBeforeSendChatMessage(Player* player, uint32& type, uint32& lang, std::string& msg);
        void OnPlayerQueueRandomDungeon(Player* player, uint32& rDungeonId);
        void OnPlayerGossipSelect(Player* player, uint32 menu_id, uint32 sender, uint32 action);
        void OnPlayerGossipSelectCode(Player* player, uint32 menu_id, uint32 sender, uint32 action, char const* code);
        bool OnPlayerCanSendMail(Player* player, ObjectGuid receiverGuid, ObjectGuid mailbox, std::string& subject, std::string& body, uint64 money, uint64 COD, Item* item);
        void OnPlayerPetitionBuy(Player* player, Creature* creature, uint32& charterid, uint32& cost, uint32& type);
        void OnPlayerPetitionShowList(Player* player, Creature* creature, uint32& CharterEntry, uint32& CharterDispayID, uint32& CharterCost);
        bool OnPlayerCanJoinLfg(Player* player, uint8 roles, lfg::LfgDungeonSet& dungeons, std::string const& comment);
        bool OnPlayerCanInitTrade(Player* player, Player* target);
        bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 language, std::string& msg);
        bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 language, std::string& msg, Player* receiver);
        bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 language, std::string& msg, Group* group);
        bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 language, std::string& msg, Guild* guild);
        bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 language, std::string& msg, Channel* channel);
        void OnPlayerBattlegroundDesertion(Player* player, BattlegroundDesertionType desertionType);
        void OnPlayerJoinBG(Player* player);
        void OnPlayerJoinArena(Player* player);
        void OnPlayerGetMaxPersonalArenaRatingRequirement(Player const* player, uint32 minSlot, uint32& maxArenaRating);
        bool OnPlayerCanJoinInBattlegroundQueue(Player* player, ObjectGuid battlemasterGuid, BattlegroundTypeId bgTypeId, uint8 joinAsGroup, GroupJoinBattlegroundResult& err);
        void OnPlayerSetServerSideVisibility(Player* player, ServerSideVisibilityType& type, AccountTypes& sec);
        void OnPlayerSetServerSideVisibilityDetect(Player* player, ServerSideVisibilityType& type, AccountTypes& sec);
        bool OnPlayerCanLearnTalent(Player* player, TalentEntry const* talent, uint32 rank);
        void OnPlayerAfterSpecSlotChanged(Player* player, uint8 newSlot);
        void OnPlayerLearnTalents(Player* player, uint32 talentId, uint32 talentRank, uint32 spellid);
        void AnticheatSetCanFlybyServer(Player* player, bool apply);
        void AnticheatSetUnderACKmount(Player* player);
        void AnticheatSetRootACKUpd(Player* player);
        void AnticheatSetJumpingbyOpcode(Player* player, bool jump);
        void AnticheatUpdateMovementInfo(Player* player, MovementInfo const& movementInfo);
        bool AnticheatHandleDoubleJump(Player* player, Unit* mover);
        bool AnticheatCheckMovementInfo(Player* player, MovementInfo const& movementInfo, Unit* mover, bool jump);

    public: /* AccountScript */

        void OnAccountLogin(uint32 accountId);
        void OnBeforeAccountDelete(uint32 accountId);
        void OnLastIpUpdate(uint32 accountId, std::string ip);
        void OnFailedAccountLogin(uint32 accountId);
        void OnEmailChange(uint32 accountId);
        void OnFailedEmailChange(uint32 accountId);
        void OnPasswordChange(uint32 accountId);
        void OnFailedPasswordChange(uint32 accountId);
        bool CanAccountCreateCharacter(uint32 accountId, uint8 charRace, uint8 charClass);

    public: /* GuildScript */

        void OnGuildAddMember(Guild* guild, Player* player, uint8& plRank);
        void OnGuildRemoveMember(Guild* guild, Player* player, bool isDisbanding, bool isKicked);
        void OnGuildMOTDChanged(Guild* guild, const std::string& newMotd);
        void OnGuildInfoChanged(Guild* guild, const std::string& newInfo);
        void OnGuildCreate(Guild* guild, Player* leader, const std::string& name);
        void OnGuildDisband(Guild* guild);
        void OnGuildMemberWitdrawMoney(Guild* guild, Player* player, uint64 &amount, bool isRepair);
        void OnGuildMemberDepositMoney(Guild* guild, Player* player, uint64 &amount);
        void OnGuildItemMove(Guild* guild, Player* player, Item* pItem, bool isSrcBank, uint8 srcContainer, uint8 srcSlotId,
            bool isDestBank, uint8 destContainer, uint8 destSlotId);
        void OnGuildEvent(Guild* guild, uint8 eventType, ObjectGuid::LowType playerGuid1, ObjectGuid::LowType playerGuid2, uint8 newRank);
        void OnGuildBankEvent(Guild* guild, uint8 eventType, uint8 tabId, ObjectGuid::LowType playerGuid, uint64 itemOrMoney, uint16 itemStackCount, uint8 destTabId);
        bool CanGuildSendBankList(Guild const* guild, WorldSession* session, uint8 tabId, bool sendAllSlots);

    public: /* GroupScript */

        void OnGroupAddMember(Group* group, ObjectGuid guid);
        void OnGroupInviteMember(Group* group, ObjectGuid guid);
        void OnGroupRemoveMember(Group* group, ObjectGuid guid, RemoveMethod method, ObjectGuid kicker, char const* reason);
        void OnGroupChangeLeader(Group* group, ObjectGuid newLeaderGuid, ObjectGuid oldLeaderGuid);
        void OnGroupDisband(Group* group);
        bool CanGroupJoinBattlegroundQueue(Group const* group, Player* member, Battleground const* bgTemplate, uint32 MinPlayerCount, bool isRated, uint32 arenaSlot);
        void OnGroupCreate(Group* group, Player* leader);

    public: /* TicketScript */

        void OnTicketCreate(GmTicket* ticket);
        void OnTicketUpdateLastChange(GmTicket* ticket);
        void OnTicketClose(GmTicket* ticket);
        void OnTicketStatusUpdate(GmTicket* ticket);
        void OnTicketResolve(GmTicket* ticket);

    public: /* GameEventScript */

        void OnGameEventStart(uint16 EventID);
        void OnGameEventStop(uint16 EventID);
        void OnGameEventCheck(uint16 EventID);

    public: /* AllCommandScript */

        void OnHandleDevCommand(Player* player, bool& enable);
        bool OnTryExecuteCommand(ChatHandler& handler, std::string_view cmdStr);

    public: /* UnitScript */

        void OnHeal(Unit* healer, Unit* reciever, uint32& gain);
        void OnDamage(Unit* attacker, Unit* victim, uint32& damage);
        void ModifyPeriodicDamageAurasTick(Unit* target, Unit* attacker, uint32& damage, SpellInfo const* spellInfo);
        void ModifyMeleeDamage(Unit* target, Unit* attacker, uint32& damage);
        void ModifySpellDamageTaken(Unit* target, Unit* attacker, int32& damage, SpellInfo const* spellInfo);
        void ModifyHealReceived(Unit* target, Unit* healer, uint32& heal, SpellInfo const* spellInfo);
        void OnAuraApply(Unit* unit, Aura* aura);
        void OnAuraRemove(Unit* unit, AuraApplication* aurApp, AuraRemoveFlags mode);
        void OnUnitUpdate(Unit* unit, uint32 diff);
        void OnDisplayIdChange(Unit* unit, uint32 displayId);
        void OnUnitEnterEvadeMode(Unit* unit, uint8 evadeReason);
        void OnUnitEnterCombat(Unit* unit, Unit* victim);
        void OnUnitDeath(Unit* unit, Unit* killer);
        void OnUnitSetShapeshiftForm(Unit* unit, uint8 form);

    public: /* AllSpellScript */

        void OnCalcMaxDuration(Aura const* aura, int32& maxDuration);
        void OnSpellCheckCast(Spell* spell, bool strict, SpellCastResult& res);
        bool CanPrepare(Spell* spell, SpellCastTargets const* targets, AuraEffect const* triggeredByAura);
        void OnDummyEffect(WorldObject* caster, uint32 spellID, SpellEffIndex effIndex, GameObject* gameObjTarget);
        void OnDummyEffect(WorldObject* caster, uint32 spellID, SpellEffIndex effIndex, Creature* creatureTarget);
        void OnDummyEffect(WorldObject* caster, uint32 spellID, SpellEffIndex effIndex, Item* itemTarget);
        void OnSpellCastCancel(Spell* spell, WorldObject* caster, SpellInfo const* spellInfo, bool bySelf);
        void OnSpellCast(Spell* spell, WorldObject* caster, SpellInfo const* spellInfo, bool skipCheck);
        void OnSpellPrepare(Spell* spell, WorldObject* caster, SpellInfo const* spellInfo);

    public: /* PetScript */

        void OnInitStatsForLevel(Guardian* guardian, uint8 petlevel);
        void OnCalculateMaxTalentPointsForLevel(Pet* pet, uint8 level, uint8& points);
        bool CanUnlearnSpellSet(Pet* pet, uint32 level, uint32 spell);
        bool CanUnlearnSpellDefault(Pet* pet, SpellInfo const* spellInfo);
        bool CanResetTalents(Pet* pet);
        void OnPetAddToWorld(Pet* pet);

    public: /* WorldStateScript */

        void OnWorldStateValueChange(WorldStateTemplate const* worldStateTemplate, int32 oldValue, int32 newValue, Map const* map);

    public: /* MovementHandlerScript */

        void OnPlayerMove(Player* player, MovementInfo movementInfo, uint32 opcode);

    public: /* AchievementScript */

        void SetRealmCompleted(AchievementEntry const* achievement);
        bool IsCompletedCriteria(AchievementMgr<Player>* mgr, AchievementCriteriaEntry const* achievementCriteria, AchievementEntry const* achievement, CriteriaProgress const* progress);
        bool IsRealmCompleted(AchievementGlobalMgr const* globalmgr, AchievementEntry const* achievement, std::chrono::system_clock::time_point completionTime);
        void OnBeforeCheckCriteria(AchievementMgr<Player>* mgr, AchievementCriteriaEntryList const* achievementCriteriaList);
        bool CanCheckCriteria(AchievementMgr<Player>* mgr, AchievementCriteriaEntry const* achievementCriteria);

    private:
        uint32 _scriptCount;

        ScriptLoaderCallbackType _script_loader_callback;
        ScriptLoaderCallbackType _modules_loader_callback;

        std::string _currentContext;
};

#define sScriptMgr ScriptMgr::instance()

#endif
