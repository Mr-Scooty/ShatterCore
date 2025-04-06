-- Set up variables for frequently used entries and spells
SET @ENTRY := 34874; -- Megs Dreadshredder
SET @QUEST := 14071; -- Rolling with my Homies quest
SET @SPELL_KEYS := 91551; -- Keys to the Hot Rod spell
SET @SPELL_ACCEPT := 66394; -- Quest Accept spell
SET @SPELL_CREATE_KEYS := 66297; -- Create Keys to Hot Rod spell
SET @BROADCAST := 48504; -- Broadcast text ID for Megs instructions

-- Fix for Hot Rod vehicle issues
-- Set proper VehicleId for the Hot Rod
UPDATE `creature_template` SET `VehicleId` = 181 WHERE `entry` IN (34840, 37676, 49131);

-- Update creature flags to make it mountable
UPDATE `creature_template` SET `unit_flags` = `unit_flags` | 16777216 WHERE `entry` IN (34840, 37676, 49131);

-- Delete existing spellclick spells
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` IN (34840, 37676, 49131);

-- Add proper spellclick spells
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES 
(34840, 46598, 1, 0),
(37676, 46598, 1, 0),
(49131, 46598, 1, 0);

-- Fix for Megs Dreadshredder quest accept handling
-- Update creature_template to use SmartAI
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` = @ENTRY;

-- Delete any existing SmartAI scripts for Megs Dreadshredder
DELETE FROM `smart_scripts` WHERE `entryorguid` = @ENTRY AND `source_type` = 0;
DELETE FROM `smart_scripts` WHERE `entryorguid` = @ENTRY*100 AND `source_type` = 9;

-- Add SmartAI scripts for proper quest handling
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(@ENTRY, 0, 0, 1, 19, 0, 100, 0, @QUEST, 0, 0, 0, 85, @SPELL_KEYS, 2, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Megs Dreadshredder - On Quest Accept (Rolling with my Homies) - Cast Keys to the Hot Rod'),
(@ENTRY, 0, 1, 2, 61, 0, 100, 0, 0, 0, 0, 0, 11, @SPELL_ACCEPT, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Megs Dreadshredder - Linked - Cast Quest Accept Spell'),
(@ENTRY, 0, 2, 3, 61, 0, 100, 0, 0, 0, 0, 0, 11, @SPELL_CREATE_KEYS, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Megs Dreadshredder - Linked - Cast Create Keys to Hot Rod'),
(@ENTRY, 0, 3, 0, 61, 0, 100, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Megs Dreadshredder - Linked - Say Line 1');

-- Add the NPC talk texts
DELETE FROM `creature_text` WHERE `CreatureID` = @ENTRY AND `GroupID` IN (0, 1);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(@ENTRY, 0, 0, 'Ooh, I think you\'re gonna like this, boss!', 12, 0, 100, 1, 0, 0, 49019, 0, 'Megs Dreadshredder - On Quest Accept'),
(@ENTRY, 1, 0, 'Use the Keys to the Hot Rod |TInterface\\Icons\\inv_misc_key_12.blp:32|t to get into your car. Pick up your friends, Izzy, Ace, and Gobber.', 41, 0, 100, 1, 0, 0, @BROADCAST, 0, 'Megs Dreadshredder - After Keys');

-- Remove AUTO_ACCEPT flag from quest flags
-- Current flags: 3801088 (with AUTO_ACCEPT flag: 0x00080000 = 524288)
-- New flags: 3276800 (without AUTO_ACCEPT flag)
UPDATE `quest_template` SET `Flags` = 3276800 WHERE `ID` = @QUEST;

-- Register the spell script for Keys to the Hot Rod item
DELETE FROM `spell_script_names` WHERE `spell_id` = @SPELL_KEYS;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (@SPELL_KEYS, 'spell_item_keys_to_the_hot_rod'); 

-- Add SPELL_ATTR4_AURA_EXPIRES_OFFLINE flag to "Keys to the Hot Rod" spell (ID 91551)
-- SPELL_ATTR4_AURA_EXPIRES_OFFLINE = 0x00000004 (4)

-- Get current AttributesEx4 value
SET @CURRENT_ATTR4 = (SELECT AttributesEx4 FROM spell_dbc WHERE Id = 91551);

-- Update the spell to add SPELL_ATTR4_AURA_EXPIRES_OFFLINE attribute
UPDATE `spell_dbc` 
SET `AttributesEx4` = (@CURRENT_ATTR4 | 0x00000004) 
WHERE `Id` = 91551;