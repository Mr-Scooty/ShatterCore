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
        (void)firstLogin; // Mark as unused
        // Check if player has the quest and is in the right state (not completed)
        if (player->GetQuestStatus(QUEST_ROLLING_WITH_MY_HOMIES) == QUEST_STATUS_INCOMPLETE)
        {
            bool needsItem = !player->HasItemCount(ITEM_KEYS_TO_HOT_ROD, 1);
            bool needsBuff = !player->HasAura(SPELL_KEYS_TO_HOT_ROD);
            
            // Add the buff if missing
            if (needsBuff)
                player->CastSpell(player, SPELL_KEYS_TO_HOT_ROD, true);
            
            // Add the item if missing
            if (needsItem)
                player->AddItem(ITEM_KEYS_TO_HOT_ROD, 1);
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
};

void AddSC_kezan()
{
    new spell_item_keys_to_the_hot_rod();
    new player_script_rolling_with_homies();
}