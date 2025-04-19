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

#include "ScriptMgr.h"
#include "Player.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "Vehicle.h"
#include "Log.h"

enum HotRodSpells
{
    SPELL_KEYS_TO_HOT_ROD = 91551
};

enum HotRodData
{
    NPC_HOT_ROD = 49132,
    QUEST_ROLLING_WITH_MY_HOMIES = 14071,
    ITEM_KEYS_TO_HOT_ROD = 46856
};

// 91551 - Keys to the Hot Rod
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

                    // Find the nearest Hot Rod
                    if (Creature* hotRod = player->FindNearestCreature(NPC_HOT_ROD, 20.0f, true))
                    {
                        if (!hotRod->GetVehicleKit())
                            return;

                        // Force player to enter vehicle
                        player->EnterVehicle(hotRod);
                    }
                    
                    // Make sure player has the item
                    if (!player->HasItemCount(ITEM_KEYS_TO_HOT_ROD, 1))
                        player->AddItem(ITEM_KEYS_TO_HOT_ROD, 1);
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

// Script to ensure "Keys to the Hot Rod" persists between logout/login
class player_script_rolling_with_homies : public PlayerScript
{
public:
    player_script_rolling_with_homies() : PlayerScript("player_script_rolling_with_homies") { }

    void OnLogin(Player* player, bool firstLogin) override
    {
        TC_LOG_ERROR("scripts.players", "player_script_rolling_with_homies: OnLogin triggered for player %s (GUID: %u).", player->GetName().c_str(), player->GetGUID().GetCounter());
        (void)firstLogin; // Mark as unused

        QuestStatus status = player->GetQuestStatus(QUEST_ROLLING_WITH_MY_HOMIES);
        TC_LOG_ERROR("scripts.players", "player_script_rolling_with_homies: Player %u Quest %u Status: %u", player->GetGUID().GetCounter(), QUEST_ROLLING_WITH_MY_HOMIES, status);

        // Check if player has the quest and is in the right state (not completed)
        if (status == QUEST_STATUS_INCOMPLETE)
        {
            TC_LOG_ERROR("scripts.players", "player_script_rolling_with_homies: Player %u has quest %u incomplete.", player->GetGUID().GetCounter(), QUEST_ROLLING_WITH_MY_HOMIES);

            bool needsBuff = !player->HasAura(SPELL_KEYS_TO_HOT_ROD);

            TC_LOG_ERROR("scripts.players", "player_script_rolling_with_homies: Player %u - Needs Buff: %d", player->GetGUID().GetCounter(), needsBuff);
            
            // Add the buff if missing
            if (needsBuff)
            {
                 TC_LOG_ERROR("scripts.players", "player_script_rolling_with_homies: Player %u attempting to cast spell %u.", player->GetGUID().GetCounter(), SPELL_KEYS_TO_HOT_ROD);
                player->CastSpell(player, SPELL_KEYS_TO_HOT_ROD, true);
                 TC_LOG_ERROR("scripts.players", "player_script_rolling_with_homies: Player %u finished attempting spell cast.", player->GetGUID().GetCounter());
            }
        }
    }
    
    // Check for the item when the buff is applied
    void OnSpellCast(Player* player, Spell* spell, bool /*skipCheck*/) override
    {
        if (spell->GetSpellInfo()->Id == SPELL_KEYS_TO_HOT_ROD)
        {
            if (!player->HasItemCount(ITEM_KEYS_TO_HOT_ROD, 1))
                player->AddItem(ITEM_KEYS_TO_HOT_ROD, 1);
        }
    }

    // Remove the buff if the quest is abandoned or status changes
    void OnQuestStatusChange(Player* player, uint32 questId) override
    {
        if (questId == QUEST_ROLLING_WITH_MY_HOMIES)
        {
            QuestStatus status = player->GetQuestStatus(questId);
            TC_LOG_ERROR("scripts.players", "player_script_rolling_with_homies: Player %u Quest %u Status Changed: %u", player->GetGUID().GetCounter(), questId, status);

            // If the quest is no longer active (abandoned, completed, failed etc.), remove the buff.
            if (status == QUEST_STATUS_NONE || status == QUEST_STATUS_COMPLETE || status == QUEST_STATUS_FAILED)
            {
                 TC_LOG_ERROR("scripts.players", "player_script_rolling_with_homies: Player %u quest %u no longer active, removing buff %u.", player->GetGUID().GetCounter(), questId, SPELL_KEYS_TO_HOT_ROD);
                player->RemoveAurasDueToSpell(SPELL_KEYS_TO_HOT_ROD);
            }
        }
    }
};

void AddSC_kezan()
{
    new spell_item_keys_to_the_hot_rod();
    new player_script_rolling_with_homies();
}