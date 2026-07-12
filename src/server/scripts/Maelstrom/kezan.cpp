/*
 * This file is part of the ShatterCore and TrinityCore Projects. See AUTHORS file for Copyright information
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

#include "ScriptMgr.h"
#include "Player.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "SpellMgr.h"
#include "SpellDefines.h"
#include "Vehicle.h"
#include "VehicleDefines.h"
#include "SharedDefines.h"
#include "ScriptedCreature.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "EventMap.h"
#include "TemporarySummon.h"
#include "CombatAI.h"
#include "DB2Stores.h"
#include "Item.h"
#include <algorithm>
#include <cmath>
#include <chrono>
#include <set>
#include <array>
#include <vector>

using namespace std::chrono_literals;

enum HotRodSpells
{
    SPELL_KEYS_TO_HOT_ROD = 91551,
    SPELL_RIDE_VEHICLE = 46598,
    SPELL_HOT_ROD_KNOCK_BACK = 66301, // frontal cone, driver casts it while the rod is moving
    SPELL_STOLEN_LOOT = 67041         // looter-cast on the driver, creates item 47530
};

enum HotRodData
{
    NPC_HOT_ROD_1 = 34840,
    NPC_HOT_ROD_2 = 37676,

    QUEST_ROLLING_WITH_MY_HOMIES = 14071,
    ITEM_KEYS_TO_HOT_ROD = 46856,

    NPC_KEYS_USED_CREDIT = 48323,
    NPC_IZZY_CREDIT = 34959,
    NPC_ACE_CREDIT = 34957,
    NPC_GOBBER_CREDIT = 34958,
    QUEST_LIFE_SAVINGS = 14126,

    NPC_IZZY = 34890,
    NPC_ACE = 34892,
    NPC_GOBBER = 34954,

    SPELL_SUMMON_IZZY = 66600,
    SPELL_SUMMON_GOBBER = 66599,
    SPELL_SUMMON_ACE = 66597,

    SPELL_RESUMMON_IZZY = 66646,
    SPELL_RESUMMON_ACE = 66644,
    SPELL_RESUMMON_GOBBER = 66645,

    QUEST_ROBBING_HOODS = 14121,
    NPC_HIRED_LOOTER = 35234,
    NPC_KEZAN_CITIZEN_1 = 35063,
    NPC_KEZAN_CITIZEN_2 = 35075
};

enum HotRodTexts
{
    SAY_CITIZEN_RUN_OVER = 0
};

static bool ShouldKeepRollingWithHomiesCompanions(Player const* player)
{
    if (!player)
        return false;

    QuestStatus rollingStatus = player->GetQuestStatus(QUEST_ROLLING_WITH_MY_HOMIES);
    if (rollingStatus != QUEST_STATUS_COMPLETE && rollingStatus != QUEST_STATUS_REWARDED)
        return false;

    return player->GetQuestStatus(QUEST_LIFE_SAVINGS) != QUEST_STATUS_REWARDED;
}

enum HotRodEvents
{
    EVENT_MOVE_TO_VEHICLE = 1,
    EVENT_BOARD_VEHICLE = 2,
    EVENT_GRANT_CREDIT = 3,
    EVENT_DESPAWN_FAILSAFE = 4,
    EVENT_REQUEUE_BOARDERS = 5,
    EVENT_RUN_OVER = 6
};

enum HotRodActions
{
    ACTION_BOARD_VEHICLE = 1
};

class spell_item_keys_to_the_hot_rod : public SpellScriptLoader
{
    public:
        spell_item_keys_to_the_hot_rod() : SpellScriptLoader("spell_item_keys_to_the_hot_rod") { }

        class spell_item_keys_to_the_hot_rod_SpellScript : public SpellScript
        {
            public:
                spell_item_keys_to_the_hot_rod_SpellScript() : SpellScript() { }

                void HandleAfterCast()
                {
                    Player* player = GetCaster()->ToPlayer();
                    if (!player)
                        return;

                    Item* castItem = GetCastItem();
                    if (!castItem || castItem->GetEntry() != ITEM_KEYS_TO_HOT_ROD)
                        return;

                    if (player->GetQuestStatus(QUEST_ROLLING_WITH_MY_HOMIES) == QUEST_STATUS_INCOMPLETE)
                    {
                        player->KilledMonsterCredit(NPC_KEYS_USED_CREDIT);
                    }

                    if (Unit* vehicleBase = player->GetVehicleBase())
                        if (vehicleBase->GetEntry() == NPC_HOT_ROD_1 || vehicleBase->GetEntry() == NPC_HOT_ROD_2)
                            return;

                    // The parked Hot Rods/Trikes around Kezan are decoration - the keys summon
                    // the player's own Hot Rod, which despawns again when the driver gets out.
                    Creature* hotRod = player->SummonCreature(NPC_HOT_ROD_1, player->GetPosition(), TEMPSUMMON_MANUAL_DESPAWN);
                    if (hotRod)
                    {
                        if (!hotRod->GetVehicleKit())
                        {
                            hotRod->DespawnOrUnsummon();
                            return;
                        }

                        player->EnterVehicle(hotRod, 0);

                        if (!player->HasItemCount(ITEM_KEYS_TO_HOT_ROD, 1))
                            player->AddItem(ITEM_KEYS_TO_HOT_ROD, 1);
                    }
                }

                void Register() override
                {
                    AfterCast.Register(&spell_item_keys_to_the_hot_rod_SpellScript::HandleAfterCast);
                }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_keys_to_the_hot_rod_SpellScript();
        }
};

class npc_hot_rod_vehicle : public CreatureScript
{
public:
    npc_hot_rod_vehicle() : CreatureScript("npc_hot_rod_vehicle") { }

    struct npc_hot_rod_vehicleAI : public VehicleAI
    {
        npc_hot_rod_vehicleAI(Creature* creature) : VehicleAI(creature), _driverHadNoJump(false) { }

        void Reset() override
        {
            pickedUpNPCs.clear();
            _events.Reset();
            _driverGUID.Clear();
            _driverHadNoJump = false;
            me->RemoveExtraUnitMovementFlag(MOVEMENTFLAG2_NO_JUMPING);
        }

        void SetGUID(ObjectGuid const& /*guid*/, int32 id) override
        {
            if (id > 0)
            {
                pickedUpNPCs.insert(id);
            }
        }

        void PassengerBoarded(Unit* passenger, int8 seatId, bool apply) override
        {
            if (seatId == 0)
            {
                Player* player = passenger->ToPlayer();
                if (!player)
                    return;

                if (apply)
                {
                    _driverGUID = player->GetGUID();
                    _driverHadNoJump = player->HasExtraUnitMovementFlag(MOVEMENTFLAG2_NO_JUMPING);
                    player->RemoveExtraUnitMovementFlag(MOVEMENTFLAG2_NO_JUMPING);
                    me->RemoveExtraUnitMovementFlag(MOVEMENTFLAG2_NO_JUMPING);
                    _events.ScheduleEvent(EVENT_REQUEUE_BOARDERS, 1s);
                    _events.ScheduleEvent(EVENT_RUN_OVER, 100ms);
                }
                else
                {
                    _events.Reset();
                    _driverGUID.Clear();
                    if (_driverHadNoJump)
                        player->AddExtraUnitMovementFlag(MOVEMENTFLAG2_NO_JUMPING);
                    else
                        player->RemoveExtraUnitMovementFlag(MOVEMENTFLAG2_NO_JUMPING);
                    _driverHadNoJump = false;
                    me->AddExtraUnitMovementFlag(MOVEMENTFLAG2_NO_JUMPING);

                    // Summoned Hot Rods despawn as soon as the driver gets out (sniffed)
                    if (me->ToTempSummon())
                        me->DespawnOrUnsummon(2000);

                    if (player->GetQuestStatus(QUEST_ROLLING_WITH_MY_HOMIES) != QUEST_STATUS_INCOMPLETE &&
                        !ShouldKeepRollingWithHomiesCompanions(player))
                        return;

                    for (uint32 npcEntry : pickedUpNPCs)
                    {
                        uint32 summonSpell = 0;

                        if (npcEntry == NPC_IZZY)
                            summonSpell = SPELL_RESUMMON_IZZY;
                        else if (npcEntry == NPC_ACE)
                            summonSpell = SPELL_RESUMMON_ACE;
                        else if (npcEntry == NPC_GOBBER)
                            summonSpell = SPELL_RESUMMON_GOBBER;

                        if (summonSpell)
                        {
                            player->CastSpell(player, summonSpell, true);
                        }
                    }

                    pickedUpNPCs.clear();
                }
            }
        }

    private:
        void UpdateAI(uint32 diff) override
        {
            VehicleAI::UpdateAI(diff);
            _events.Update(diff);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                if (eventId == EVENT_REQUEUE_BOARDERS)
                {
                    if (_driverGUID.IsEmpty())
                        continue;

                    if (Player* player = ObjectAccessor::GetPlayer(*me, _driverGUID))
                        EnsureSummonedCompanionsBoard(player);
                }
                else if (eventId == EVENT_RUN_OVER)
                {
                    if (_driverGUID.IsEmpty())
                        continue;

                    // Retail: the driver spams the run-over cone (~100ms cadence in the
                    // Goblin_P2 sniff) for as long as the Hot Rod is being driven.
                    if (Player* driver = ObjectAccessor::GetPlayer(*me, _driverGUID))
                    {
                        if (me->isMoving())
                            driver->CastSpell(driver, SPELL_HOT_ROD_KNOCK_BACK, true);
                        _events.ScheduleEvent(EVENT_RUN_OVER, 100ms);
                    }
                }
            }
        }

        void EnsureSummonedCompanionsBoard(Player* player)
        {
            static uint32 const kCompanionEntries[] = { NPC_IZZY_CREDIT, NPC_GOBBER_CREDIT, NPC_ACE_CREDIT, NPC_IZZY, NPC_GOBBER, NPC_ACE };
            bool requested = false;

            for (uint32 companionEntry : kCompanionEntries)
            {
                std::list<Creature*> companions;
                player->GetCreatureListWithEntryInGrid(companions, companionEntry, 100.0f);

                for (Creature* companion : companions)
                {
                    if (companion->GetOwnerGUID() != player->GetGUID())
                        continue;

                    if (companion->GetVehicle())
                        continue;

                    if (CreatureAI* ai = companion->AI())
                    {
                        ai->SetGUID(me->GetGUID(), ACTION_BOARD_VEHICLE);
                        requested = true;
                    }
                }
            }

            if (requested)
                _events.ScheduleEvent(EVENT_REQUEUE_BOARDERS, 1500ms);
        }

        std::set<uint32> pickedUpNPCs;
        EventMap _events;
        ObjectGuid _driverGUID;
        bool _driverHadNoJump;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_hot_rod_vehicleAI(creature);
    }
};

// 66301 - Hot Rod Knock Back: frontal cone the driver spams while moving. Retail
// (Goblin_P2 sniff): a run-over Hired Looter casts 67041 on the driver (creates
// Stolen Loot 47530 for Robbing Hoods) and drops dead on the spot; Kezan Citizens
// are launched (native knockback, BP/MiscValue 90) and yell at the driver.
class spell_kezan_hot_rod_run_over : public SpellScriptLoader
{
public:
    spell_kezan_hot_rod_run_over() : SpellScriptLoader("spell_kezan_hot_rod_run_over") { }

    class spell_kezan_hot_rod_run_over_SpellScript : public SpellScript
    {
    public:
        // The full 12yd DBC cone is used on purpose: an aggroed Hired Looter chases
        // the rod and slips past a short "bumper" range between two 100ms ticks -
        // retail kills them on approach (the sniff shows the same citizen hit twice
        // 206ms apart, so the retail cone is deep, not contact-sized).

        void HandleRunOver(SpellEffIndex effIndex)
        {
            Creature* target = GetHitCreature();
            if (!target || !target->IsAlive())
                return;

            Player* driver = GetCaster()->ToPlayer();

            if (target->GetEntry() == NPC_HIRED_LOOTER)
            {
                // The looter hands over its Stolen Loot and dies where it stands (no
                // knockback in the sniff). KillSelf leaves the corpse untapped so the
                // run-over reward cannot be double-dipped from the corpse loot.
                PreventHitDefaultEffect(effIndex);
                if (driver && driver->GetQuestStatus(QUEST_ROBBING_HOODS) == QUEST_STATUS_INCOMPLETE)
                    target->CastSpell(driver, SPELL_STOLEN_LOOT, true);
                target->KillSelf();
                return;
            }

            // Anyone already sailing through the air from a previous tick is skipped -
            // the 100ms cone would otherwise re-launch them mid-flight.
            if (target->GetMotionMaster()->GetCurrentMovementGeneratorType() == EFFECT_MOTION_TYPE)
            {
                PreventHitDefaultEffect(effIndex);
                return;
            }

            if (driver && (target->GetEntry() == NPC_KEZAN_CITIZEN_1 || target->GetEntry() == NPC_KEZAN_CITIZEN_2))
                target->AI()->Talk(SAY_CITIZEN_RUN_OVER, driver);
        }

        void Register() override
        {
            OnEffectHitTarget.Register(&spell_kezan_hot_rod_run_over_SpellScript::HandleRunOver, EFFECT_0, SPELL_EFFECT_KNOCK_BACK);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_kezan_hot_rod_run_over_SpellScript();
    }
};

class npc_hot_rod_follower : public CreatureScript
{
public:
    npc_hot_rod_follower() : CreatureScript("npc_hot_rod_follower") { }

    struct npc_hot_rod_followerAI : public ScriptedAI
    {
        npc_hot_rod_followerAI(Creature* creature) : ScriptedAI(creature), checkTimer(2000), hasSpawned(false)
        {
        }

        void Reset() override
        {
            hasSpawned = false;
        }

        void UpdateAI(uint32 diff) override
        {
            if (checkTimer <= diff)
            {
                checkTimer = 2000;
                CheckForPlayerWithHotRod();
            }
            else
                checkTimer -= diff;
        }

    private:
        uint32 checkTimer;
        bool hasSpawned;

        void CheckForPlayerWithHotRod()
        {
            if (hasSpawned)
                return;

            Player* player = nullptr;

            for (uint32 hotRodEntry : {NPC_HOT_ROD_1, NPC_HOT_ROD_2})
            {
                if (Creature* hotRod = me->FindNearestCreature(hotRodEntry, 20.0f, true))
                {
                    if (Unit* charmer = hotRod->GetCharmer())
                    {
                        if (charmer->GetTypeId() == TYPEID_PLAYER)
                        {
                            player = charmer->ToPlayer();
                            break;
                        }
                    }
                }
            }

            if (!player)
            {
                player = me->SelectNearestPlayer(20.0f);
            }

            if (!player)
            {
                return;
            }

            if (player->GetQuestStatus(QUEST_ROLLING_WITH_MY_HOMIES) != QUEST_STATUS_INCOMPLETE)
                return;

            Unit* vehicle = player->GetVehicleBase();

            if (!vehicle)
            {
                ObjectGuid charmedGuid = player->GetCharmedGUID();
                if (!charmedGuid.IsEmpty())
                {
                    if (Unit* charmed = ObjectAccessor::GetUnit(*player, charmedGuid))
                    {
                        if (IsHotRod(charmed->GetEntry()))
                        {
                            vehicle = charmed;
                        }
                    }
                }
            }

            if (!vehicle || !IsHotRod(vehicle->GetEntry()))
            {
                return;
            }

            hasSpawned = true;

            uint32 summonSpell = 0;
            if (me->GetEntry() == NPC_IZZY)
                summonSpell = SPELL_SUMMON_IZZY;
            else if (me->GetEntry() == NPC_ACE)
                summonSpell = SPELL_SUMMON_ACE;
            else if (me->GetEntry() == NPC_GOBBER)
                summonSpell = SPELL_SUMMON_GOBBER;

            if (summonSpell == 0)
            {
                return;
            }

            player->CastSpell({ me->GetPositionX(), me->GetPositionY(), me->GetPositionZ() }, summonSpell, true);

            me->DespawnOrUnsummon(1000);
        }

        bool IsHotRod(uint32 entry)
        {
            return (entry == NPC_HOT_ROD_1 || entry == NPC_HOT_ROD_2);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_hot_rod_followerAI(creature);
    }
};

class player_script_rolling_with_homies : public PlayerScript
{
public:
    player_script_rolling_with_homies() : PlayerScript("player_script_rolling_with_homies") { }

    void OnLogin(Player* player, bool /*firstLogin*/) override
    {
        QuestStatus status = player->GetQuestStatus(QUEST_ROLLING_WITH_MY_HOMIES);
        if (status == QUEST_STATUS_INCOMPLETE || status == QUEST_STATUS_COMPLETE || ShouldKeepRollingWithHomiesCompanions(player))
        {
            if (!player->HasAura(SPELL_KEYS_TO_HOT_ROD))
                player->CastSpell(player, SPELL_KEYS_TO_HOT_ROD, true);
        }

        if (status == QUEST_STATUS_COMPLETE || ShouldKeepRollingWithHomiesCompanions(player))
        {
            ObjectGuid playerGUID = player->GetGUID();
            player->m_Events.AddEventAtOffset([playerGUID]()
            {
                Player* player = ObjectAccessor::FindPlayer(playerGUID);
                if (!ShouldKeepRollingWithHomiesCompanions(player))
                    return;

                player->CastSpell(player, SPELL_RESUMMON_IZZY, true);
                player->CastSpell(player, SPELL_RESUMMON_ACE, true);
                player->CastSpell(player, SPELL_RESUMMON_GOBBER, true);
            }, 2s);
        }
    }

    void OnSpellCast(Player* player, Spell* spell, bool /*skipCheck*/) override
    {
        if (spell->GetSpellInfo()->Id == SPELL_KEYS_TO_HOT_ROD)
        {
            if (!player->HasItemCount(ITEM_KEYS_TO_HOT_ROD, 1))
                player->AddItem(ITEM_KEYS_TO_HOT_ROD, 1);
        }
    }

    void OnQuestStatusChange(Player* player, uint32 questId) override
    {
        if (questId == QUEST_ROLLING_WITH_MY_HOMIES)
        {
            QuestStatus status = player->GetQuestStatus(questId);
            if (status == QUEST_STATUS_NONE || status == QUEST_STATUS_FAILED)
            {
                player->RemoveAurasDueToSpell(SPELL_KEYS_TO_HOT_ROD);
                CleanupQuestFollowers(player);
            }
        }
        else if (questId == QUEST_LIFE_SAVINGS)
        {
            QuestStatus status = player->GetQuestStatus(questId);
            if (status == QUEST_STATUS_REWARDED)
            {
                player->RemoveAurasDueToSpell(SPELL_KEYS_TO_HOT_ROD);
                CleanupQuestFollowers(player);
            }
        }
    }

    void CleanupQuestFollowers(Player* player)
    {
        static constexpr std::array<uint32, 6> kFollowerEntries =
        {
            NPC_IZZY, NPC_ACE, NPC_GOBBER,
            NPC_IZZY_CREDIT, NPC_ACE_CREDIT, NPC_GOBBER_CREDIT
        };

        std::list<Creature*> followers;
        for (uint32 entry : kFollowerEntries)
            player->GetCreatureListWithEntryInGrid(followers, entry, 200.0f);

        std::set<ObjectGuid> processed;

        for (Creature* follower : followers)
        {
            if (!follower)
                continue;

            if (!processed.insert(follower->GetGUID()).second)
                continue;

            TempSummon* tempSummon = follower->ToTempSummon();
            bool ownedByPlayer = follower->GetOwnerGUID() == player->GetGUID();

            if (tempSummon && tempSummon->GetSummonerGUID() == player->GetGUID())
                ownedByPlayer = true;

            if (!ownedByPlayer)
                continue;

            follower->CombatStop(true);
            follower->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
            follower->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);

            if (follower->GetVehicleBase())
                follower->ExitVehicle();

            if (tempSummon)
                tempSummon->DespawnOrUnsummon(500);
            else
                follower->DespawnOrUnsummon(500);
        }
    }
};

struct npc_rolling_with_homies_gossipAI : public ScriptedAI
{
    npc_rolling_with_homies_gossipAI(Creature* creature) : ScriptedAI(creature), checkTimer(2000), isSummoned(false), boardingInProgress(false)
    {
        _playerGUID.Clear();
        _vehicleGUID.Clear();
    }

    void IsSummonedBy(Unit* summoner) override
    {

        if (!summoner)
        {
            return;
        }

        Player* player = summoner->ToPlayer();

        if (!player)
        {
            if (Creature* summonerCreature = summoner->ToCreature())
            {
                player = summonerCreature->SelectNearestPlayer(30.0f);
            }
        }

        if (!player)
        {
            me->DespawnOrUnsummon(1000);
            return;
        }

        if (player->GetQuestStatus(QUEST_ROLLING_WITH_MY_HOMIES) != QUEST_STATUS_INCOMPLETE &&
            !ShouldKeepRollingWithHomiesCompanions(player))
        {
            me->DespawnOrUnsummon(1000);
            return;
        }

        std::list<Creature*> existing;
        player->GetCreatureListWithEntryInGrid(existing, me->GetEntry(), 50.0f);

        for (Creature* other : existing) {
            if (other != me && other->GetOwnerGUID() == player->GetGUID()) {
                me->DespawnOrUnsummon(100);
                return;
            }
        }

        boardingInProgress = false;
        isSummoned = true;
        _playerGUID = player->GetGUID();

        me->SetOwnerGUID(player->GetGUID());
        me->SetCreatorGUID(player->GetGUID());
        me->SetFaction(player->GetFaction());
        me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        me->SetReactState(REACT_PASSIVE);

        me->SetVisible(true);
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);

        me->RemoveAllAuras();
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
        me->SetUInt32Value(UNIT_FIELD_BYTES_1, 0);
        me->SetUInt32Value(UNIT_NPC_EMOTESTATE, 0);

        const CreatureTemplate* cTemplate = sObjectMgr->GetCreatureTemplate(me->GetEntry());
        if (cTemplate && cTemplate->Models.size() > 0)
        {
        }

        uint32 displayId = 0;
        uint32 wrongDisplayId = 0;

        if (me->GetEntry() == 34959)
        {
            displayId = 29482;
        }
        else if (me->GetEntry() == 34957)
        {
            displayId = 29495;
            wrongDisplayId = 29481;
        }
        else if (me->GetEntry() == 34958)
        {
            displayId = 32385;
            wrongDisplayId = 29483;
        }

        if (displayId != 0)
        {
            if (wrongDisplayId != 0 && me->GetDisplayId() == wrongDisplayId)
            {
                me->SetDisplayId(0);
            }

            me->SetDisplayId(displayId);

            uint32 actualDisplay = me->GetDisplayId();
            if (actualDisplay != displayId)
            {
                me->SetUInt32Value(UNIT_FIELD_DISPLAYID, displayId);
                actualDisplay = me->GetDisplayId();

                if (actualDisplay != displayId)
                {
                    me->SetObjectScale(0.99f);
                    me->SetObjectScale(1.0f);
                    me->SetDisplayId(displayId);
                    actualDisplay = me->GetDisplayId();
                }
            }
        }
        else
        {
        }

        me->SetFloatValue(OBJECT_FIELD_SCALE_X, 1.0f);

        me->GetMotionMaster()->MoveFollow(player, 3.0f, float(rand_norm() * 2 * M_PI));

        Unit* vehicle = FindPlayerHotRod(player);
        if (vehicle)
        {
            _vehicleGUID = vehicle->GetGUID();
        }

        _events.ScheduleEvent(EVENT_MOVE_TO_VEHICLE, 500);

        player->UpdateObjectVisibility();
        me->UpdateObjectVisibility();

        me->SendUpdateToPlayer(player);

    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type == POINT_MOTION_TYPE && id == 1)
        {
            if (_events.GetNextEventTime(EVENT_BOARD_VEHICLE) == 0)
                _events.ScheduleEvent(EVENT_BOARD_VEHICLE, 100);
        }
    }

    void Reset() override
    {
        visibilityFixed = false;
        visibilityCheckTimer = 500;
        visibilityCheckCount = 0;
        checkTimer = 3000;
    }

    void SetGUID(ObjectGuid const& guid, int32 action) override
    {
        if (action == ACTION_BOARD_VEHICLE)
        {
            _vehicleGUID = guid;

            _events.CancelEvent(EVENT_MOVE_TO_VEHICLE);
            _events.CancelEvent(EVENT_BOARD_VEHICLE);

            boardingInProgress = false;

            _events.ScheduleEvent(EVENT_BOARD_VEHICLE, 100);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        if (!visibilityFixed && me->IsAlive())
        {
            if (visibilityCheckTimer <= diff)
            {
                if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID))
                {
                    me->SetVisible(true);
                    uint32 currentDisplay = me->GetDisplayId();
                    if (currentDisplay == 0 || currentDisplay == 11686)
                    {
                        uint32 displayId = 29482;
                        if (me->GetEntry() == 34957)
                            displayId = 29481;
                        else if (me->GetEntry() == 34958)
                            displayId = 29483;

                        me->SetDisplayId(displayId);
                    }

                    me->SendUpdateToPlayer(player);
                    player->UpdateObjectVisibility();

                    visibilityCheckCount++;
                    if (visibilityCheckCount >= 5)
                        visibilityFixed = true;
                }
                visibilityCheckTimer = 1000;
            }
            else
                visibilityCheckTimer -= diff;
        }

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_MOVE_TO_VEHICLE:
                {
                    Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID);
                    if (!player)
                    {
                        me->DespawnOrUnsummon(1000);
                        return;
                    }

                    Unit* vehicle = ObjectAccessor::GetUnit(*me, _vehicleGUID);
                    if (!vehicle || !vehicle->GetVehicleKit())
                    {
                        if (Unit* refreshed = FindPlayerHotRod(player))
                        {
                            vehicle = refreshed;
                            _vehicleGUID = vehicle->GetGUID();
                        }
                    }

                    if (!vehicle || !vehicle->GetVehicleKit())
                    {
                        _events.ScheduleEvent(EVENT_MOVE_TO_VEHICLE, 1000);
                        break;
                    }

                    float dist = me->GetExactDist2d(vehicle);

                    if (dist > 5.0f)
                    {
                        Position dest = vehicle->GetPosition();
                        me->MovePosition(dest, -3.0f, 0.0f);
                        me->GetMotionMaster()->MovePoint(1, dest, false);
                        _events.ScheduleEvent(EVENT_MOVE_TO_VEHICLE, 400);
                        break;
                    }

                    _events.ScheduleEvent(EVENT_BOARD_VEHICLE, 200);
                    break;
                }

                case EVENT_BOARD_VEHICLE:
                {
                    if (boardingInProgress)
                        break;

                    boardingInProgress = true;

                    Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID);
                    if (!player)
                    {
                        boardingInProgress = false;
                        break;
                    }

                    Unit* vehicle = ObjectAccessor::GetUnit(*me, _vehicleGUID);
                    if (!vehicle || !vehicle->GetVehicleKit())
                    {
                        if (Unit* refreshed = FindPlayerHotRod(player))
                        {
                            vehicle = refreshed;
                            _vehicleGUID = vehicle->GetGUID();
                        }
                    }

                    if (!vehicle || !vehicle->GetVehicleKit())
                    {
                        boardingInProgress = false;
                        _events.ScheduleEvent(EVENT_MOVE_TO_VEHICLE, 500);
                        break;
                    }

                    if (me->isDead())
                    {
                        boardingInProgress = false;
                        break;
                    }

                    if (me->GetVehicleBase() == vehicle)
                    {
                        boardingInProgress = false;
                        _events.ScheduleEvent(EVENT_GRANT_CREDIT, 200);
                        break;
                    }

                    Vehicle* veh = vehicle->GetVehicleKit();
                    if (!veh)
                    {
                        boardingInProgress = false;
                        _events.ScheduleEvent(EVENT_MOVE_TO_VEHICLE, 500);
                        break;
                    }

                    int8 seat = GetSeatForNPC(me->GetEntry(), veh);
                    if (seat < 0)
                    {
                        boardingInProgress = false;
                        me->DespawnOrUnsummon(1000);
                        break;
                    }

                    if (veh->GetPassenger(seat))
                    {
                        boardingInProgress = false;
                        _events.ScheduleEvent(EVENT_BOARD_VEHICLE, 750);
                        break;
                    }

                    me->GetMotionMaster()->Clear();
                    me->StopMoving();
                    me->SetFacingToObject(vehicle);

                    me->EnterVehicle(vehicle, seat);

                    boardingInProgress = false;
                    _events.ScheduleEvent(EVENT_GRANT_CREDIT, 800);
                    break;
                }

                case EVENT_GRANT_CREDIT:
                {
                    if (me->isDead())
                    {
                        return;
                    }

                    Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID);
                    if (!player)
                        return;

                    Unit* vehicle = ObjectAccessor::GetUnit(*me, _vehicleGUID);
                    if (!vehicle || me->GetVehicleBase() != vehicle)
                    {
                        _events.ScheduleEvent(EVENT_BOARD_VEHICLE, 500);
                        break;
                    }

                    GiveQuestCreditForNPC(player, me);

                    if (Creature* vehicleCreature = vehicle->ToCreature())
                    {
                        if (CreatureAI* vehicleAI = vehicleCreature->AI())
                        {
                            uint32 companionEntry = GetCompanionEntryForCredit(me->GetEntry());
                            vehicleAI->SetGUID(me->GetGUID(), companionEntry ? int32(companionEntry) : me->GetEntry());
                        }
                    }
                    break;
                }

                case EVENT_DESPAWN_FAILSAFE:
                    if (!me->GetVehicle())
                    {
                        me->DespawnOrUnsummon(1000);
                    }
                    break;
            }
        }

        if (!isSummoned)
        {
            if (checkTimer <= diff)
            {
                checkTimer = 3000;
                CheckForPlayerWithVehicle();
            }
            else
                checkTimer -= diff;
        }

        ScriptedAI::UpdateAI(diff);
    }

    void CheckForPlayerWithVehicle()
    {
        Player* player = me->SelectNearestPlayer(15.0f);
        if (!player)
            return;

        if (player->GetQuestStatus(QUEST_ROLLING_WITH_MY_HOMIES) != QUEST_STATUS_INCOMPLETE)
            return;

        Unit* vehicle = FindPlayerHotRod(player);
        if (vehicle)
        {
            TryEnterPlayerVehicle(player, me);
        }
    }

    bool GossipHello(Player* player) override
    {
        if (player->GetQuestStatus(QUEST_ROLLING_WITH_MY_HOMIES) == QUEST_STATUS_INCOMPLETE)
            return false;

        return true;
    }

    bool GossipSelect(Player* player, uint32 /*menuId*/, uint32 gossipListId) override
    {
        if (gossipListId == 0)
        {
            CloseGossipMenuFor(player);
            TryEnterPlayerVehicle(player, me);
            return true;
        }

        return false;
    }

private:
    void TryEnterPlayerVehicle(Player* player, Creature* npc)
    {

        Unit* hotRod = FindPlayerHotRod(player);

        if (hotRod && hotRod->GetVehicleKit())
        {

            GiveQuestCreditForNPC(player, npc);

            if (CreatureAI* ai = npc->AI())
            {
                if (ScriptedAI* scriptedAI = dynamic_cast<ScriptedAI*>(ai))
                    scriptedAI->Talk(0);
            }

            if (Creature* vehicle = hotRod->ToCreature())
            {
                if (CreatureAI* vehicleAI = vehicle->AI())
                {
                    vehicleAI->SetGUID(npc->GetGUID(), npc->GetEntry());
                }
            }

            npc->DespawnOrUnsummon(1000);
        }
        else
        {
        }
    }

    bool IsHotRodEntry(uint32 entry)
    {
        return (entry == NPC_HOT_ROD_1 || entry == NPC_HOT_ROD_2);
    }

    Unit* FindPlayerHotRod(Player* player)
    {
        ObjectGuid charmedGuid = player->GetCharmedGUID();
        if (!charmedGuid.IsEmpty())
        {
            if (Unit* possessed = ObjectAccessor::GetUnit(*player, charmedGuid))
            {
                if (IsHotRodEntry(possessed->GetEntry()))
                {
                    return possessed;
                }
            }
        }

        Unit* vehicle = player->GetVehicleBase();
        if (vehicle && IsHotRodEntry(vehicle->GetEntry()))
        {
            return vehicle;
        }

        return nullptr;
    }

    void GiveQuestCreditForNPC(Player* player, Creature* npc)
    {
        if (npc->GetEntry() == NPC_IZZY_CREDIT)
        {
            player->KilledMonsterCredit(NPC_IZZY_CREDIT);
        }
        else if (npc->GetEntry() == NPC_ACE_CREDIT)
        {
            player->KilledMonsterCredit(NPC_ACE_CREDIT);
        }
        else if (npc->GetEntry() == NPC_GOBBER_CREDIT)
        {
            player->KilledMonsterCredit(NPC_GOBBER_CREDIT);
        }
    }

    int8 GetSeatForNPC(uint32 entry, Vehicle* veh)
    {
        if (!veh)
            return -1;

        static constexpr std::array<uint32, 2> kHotRodEntries = { NPC_HOT_ROD_1, NPC_HOT_ROD_2 };
        bool const isHotRod = std::find(kHotRodEntries.begin(), kHotRodEntries.end(), veh->GetCreatureEntry()) != kHotRodEntries.end();

        if (veh->Seats.size() <= 1)
        {
            return -1;
        }

        auto const seatIsFree = [veh, isHotRod](int8 seatId, bool allowLocked) -> bool
        {
            auto const itr = veh->Seats.find(seatId);
            if (itr == veh->Seats.end())
                return false;

            VehicleSeat const& seat = itr->second;
            VehicleSeatEntry const* seatInfo = seat.SeatInfo;
            if (!seatInfo)
                return false;

            if (seatId == 0 || seatInfo->HasFlag(VEHICLE_SEAT_FLAG_CAN_CONTROL))
                return false;

            bool const locked = !seatInfo->CanEnterOrExit() && !seatInfo->IsUsableByOverride();
            if (!allowLocked && locked)
                return false;

            if (!seat.Passenger.Guid.IsEmpty())
                return false;

            return true;
        };

        std::vector<int8> availableSeats;
        for (auto const& [seatId, seat] : veh->Seats)
        {
            VehicleSeatEntry const* seatInfo = seat.SeatInfo;

            if (!seatInfo)
                continue;

            bool const isDriverSeat = seatId == 0 || seatInfo->HasFlag(VEHICLE_SEAT_FLAG_CAN_CONTROL);
            bool const locked = !seatInfo->CanEnterOrExit() && !seatInfo->IsUsableByOverride();
            bool const occupied = !seat.Passenger.Guid.IsEmpty();

            if (isDriverSeat)
            {
                continue;
            }

            if (occupied)
                continue;

            if (!locked || isHotRod)
                availableSeats.push_back(seatId);
        }

        if (availableSeats.empty())
        {
            return -1;
        }

        auto const pickPreferredSeat = [&](std::array<int8, 3> const& seatOrder) -> int8
        {
            for (int8 seatId : seatOrder)
            {
                if (seatId < 0)
                    continue;

                if (seatIsFree(seatId, true))
                    return seatId;
            }
            return -1;
        };

        if (isHotRod)
        {
            switch (entry)
            {
                case NPC_ACE_CREDIT:
                case NPC_ACE:
                {
                    static constexpr std::array<int8, 3> kAceSeats = { 1, 2, 3 };
                    if (int8 seat = pickPreferredSeat(kAceSeats))
                        return seat;
                    break;
                }
                case NPC_GOBBER_CREDIT:
                case NPC_GOBBER:
                {
                    static constexpr std::array<int8, 3> kGobberSeats = { 2, 1, 3 };
                    if (int8 seat = pickPreferredSeat(kGobberSeats))
                        return seat;
                    break;
                }
                case NPC_IZZY_CREDIT:
                case NPC_IZZY:
                {
                    static constexpr std::array<int8, 3> kIzzySeats = { 3, 2, 1 };
                    if (int8 seat = pickPreferredSeat(kIzzySeats))
                        return seat;
                    break;
                }
                default:
                    break;
            }
        }

        std::sort(availableSeats.begin(), availableSeats.end());
        int8 const fallbackSeat = availableSeats.front();
        if (seatIsFree(fallbackSeat, isHotRod))
        {
            return fallbackSeat;
        }

        for (int8 seatId : availableSeats)
        {
            if (seatIsFree(seatId, isHotRod))
            {
                return seatId;
            }
        }

        return -1;
    }

    uint32 GetCompanionEntryForCredit(uint32 entry)
    {
        switch (entry)
        {
            case NPC_IZZY_CREDIT:
                return NPC_IZZY;
            case NPC_GOBBER_CREDIT:
                return NPC_GOBBER;
            case NPC_ACE_CREDIT:
                return NPC_ACE;
            default:
                return 0;
        }
    }

private:
    EventMap _events;
    ObjectGuid _playerGUID;
    ObjectGuid _vehicleGUID;
    uint32 checkTimer;
    bool isSummoned;
    bool boardingInProgress;

    bool visibilityFixed;
    uint32 visibilityCheckTimer;
    uint32 visibilityCheckCount;
};

class npc_rolling_with_homies_gossip : public CreatureScript
{
public:
    npc_rolling_with_homies_gossip() : CreatureScript("npc_rolling_with_homies_gossip") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_rolling_with_homies_gossipAI(creature);
    }
};

void AddSC_kezan()
{
    new spell_item_keys_to_the_hot_rod();
    new npc_hot_rod_vehicle();
    new spell_kezan_hot_rod_run_over();
    new npc_hot_rod_follower();
    new npc_rolling_with_homies_gossip();
    new player_script_rolling_with_homies();
}
