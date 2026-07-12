-- Kezan: "The New You" (14109/14110) could not be completed - the outfit gossip options never showed.
-- The three makeover vendors (Gappy Silvertooth 35126 / Szabo 35128 / Missa Spekkies 35130) had their
-- gossip_menu_option rows stored with OptionType=0 (GOSSIP_OPTION_NONE) and OptionNpcflag=0.
-- Player::PrepareGossipMenu requires (OptionNpcflag & creature npcflags) to be non-zero, so the
-- options were silently skipped and the player could never ask for the outfit pieces.
-- Retail sniff (Goblin_P2, menu 10622) confirms the option is offered while on the quest; selecting
-- it makes the player self-cast the create-item spell (66780/66781/66782, already wired via SmartAI
-- invoker-cast) and then shows the option-less follow-up menu (10619/10621/10623) via ActionMenuID.
-- OptionType=1 (GOSSIP_OPTION_GOSSIP) + OptionNpcflag=1 (UNIT_NPC_FLAG_GOSSIP) restores all of that.
UPDATE `gossip_menu_option` SET `OptionType`=1, `OptionNpcflag`=1 WHERE `MenuID` IN (10620,10622,10624) AND `OptionID`=0;

-- Fix misleading imported comments: action 85 is an invoker (player) self-cast, not "Add Item".
UPDATE `smart_scripts` SET `comment`='Gappy Silvertooth - On Gossip Option 0 Selected - Invoker Cast ''Shiny Bling''' WHERE `entryorguid`=35126 AND `source_type`=0 AND `id`=0;
UPDATE `smart_scripts` SET `comment`='Szabo - On Gossip Option 0 Selected - Invoker Cast ''Hip New Outfit''' WHERE `entryorguid`=35128 AND `source_type`=0 AND `id`=0;
UPDATE `smart_scripts` SET `comment`='Missa Spekkies - On Gossip Option 0 Selected - Invoker Cast ''Cool Shades''' WHERE `entryorguid`=35130 AND `source_type`=0 AND `id`=0;
