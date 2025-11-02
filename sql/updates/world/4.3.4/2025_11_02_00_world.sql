SET @ENTRY := 34874;
SET @QUEST := 14071;
SET @SPELL_KEYS := 91551;
SET @SPELL_ACCEPT := 66394;
SET @SPELL_CREATE_KEYS := 66297;
SET @BROADCAST := 48504;
SET @NPC_KEYS_CREDIT := 48323;
SET @NPC_IZZY_CREDIT := 34959;
SET @NPC_ACE_CREDIT := 34957;
SET @NPC_GOBBER_CREDIT := 34958;
SET @NPC_IZZY := 34890;
SET @NPC_ACE := 34892;
SET @NPC_GOBBER := 34954;
UPDATE `creature_template` SET `VehicleId` = 181 WHERE `entry` IN (34840, 37676, 49131, 49132);
UPDATE `creature_template` SET `unit_flags` = `unit_flags` | 16777216 WHERE `entry` IN (34840, 37676, 49131, 49132);
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` IN (34840, 37676, 49131, 49132);
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(34840, 46598, 1, 0),
(37676, 46598, 1, 0),
(49131, 46598, 1, 0),
(49132, 46598, 1, 0);
UPDATE `creature_template` SET `AIName` = 'SmartAI', `ScriptName` = 'npc_hot_rod_follower'
WHERE `entry` IN (@NPC_IZZY, @NPC_ACE, @NPC_GOBBER);
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` = @ENTRY;
DELETE FROM `smart_scripts` WHERE `entryorguid` = @ENTRY AND `source_type` = 0;
DELETE FROM `smart_scripts` WHERE `entryorguid` = @ENTRY*100 AND `source_type` = 9;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(@ENTRY, 0, 0, 1, 19, 0, 100, 0, @QUEST, 0, 0, 0, 85, @SPELL_KEYS, 2, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Megs Dreadshredder - On Quest Accept (Rolling with my Homies) - Cast Keys to the Hot Rod'),
(@ENTRY, 0, 1, 2, 61, 0, 100, 0, 0, 0, 0, 0, 11, @SPELL_ACCEPT, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Megs Dreadshredder - Linked - Cast Quest Accept Spell'),
(@ENTRY, 0, 2, 3, 61, 0, 100, 0, 0, 0, 0, 0, 11, @SPELL_CREATE_KEYS, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Megs Dreadshredder - Linked - Cast Create Keys to Hot Rod'),
(@ENTRY, 0, 3, 0, 61, 0, 100, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Megs Dreadshredder - Linked - Say Line 1');
DELETE FROM `creature_text` WHERE `CreatureID` = @ENTRY AND `GroupID` IN (0, 1);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(@ENTRY, 0, 0, 'Ooh, I think you\'re gonna like this, boss!', 12, 0, 100, 1, 0, 0, 49019, 0, 'Megs Dreadshredder - On Quest Accept'),
(@ENTRY, 1, 0, 'Use the Keys to the Hot Rod |TInterface\\Icons\\inv_misc_key_12.blp:32|t to get into your car. Pick up your friends, Izzy, Ace, and Gobber.', 41, 0, 100, 1, 0, 0, @BROADCAST, 0, 'Megs Dreadshredder - After Keys');
DELETE FROM `creature_text` WHERE `CreatureID` IN (@NPC_IZZY, @NPC_ACE, @NPC_GOBBER);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(@NPC_IZZY, 0, 0, 'Woohoo! Let\'s ride!', 12, 0, 100, 1, 0, 0, 0, 0, 'Izzy - Enter Hot Rod'),
(@NPC_ACE, 0, 0, 'This is gonna be great!', 12, 0, 100, 1, 0, 0, 0, 0, 'Ace - Enter Hot Rod'),
(@NPC_GOBBER, 0, 0, 'Finally, some action!', 12, 0, 100, 1, 0, 0, 0, 0, 'Gobber - Enter Hot Rod');
UPDATE `quest_template` SET `Flags` = 3276800 WHERE `ID` = @QUEST;
DELETE FROM `spell_script_names` WHERE `spell_id` = @SPELL_KEYS;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (@SPELL_KEYS, 'spell_item_keys_to_the_hot_rod');
INSERT INTO vehicle_template_accessory (entry, accessory_entry, seat_id, minion, description, summontype, summontimer) VALUES
(34840, 0, 1, 0, 'Hot Rod Passenger Seat 1', 6, 30000),
(34840, 0, 2, 0, 'Hot Rod Passenger Seat 2', 6, 30000),
(34840, 0, 3, 0, 'Hot Rod Passenger Seat 3', 6, 30000),
(37676, 0, 1, 0, 'Hot Rod Passenger Seat 1', 6, 30000),
(37676, 0, 2, 0, 'Hot Rod Passenger Seat 2', 6, 30000),
(37676, 0, 3, 0, 'Hot Rod Passenger Seat 3', 6, 30000),
(49131, 0, 1, 0, 'Goblin Hot Rod Passenger Seat 1', 6, 30000),
(49131, 0, 2, 0, 'Goblin Hot Rod Passenger Seat 2', 6, 30000),
(49131, 0, 3, 0, 'Goblin Hot Rod Passenger Seat 3', 6, 30000);

UPDATE `creature_template` SET `npcflag` = `npcflag` | 16777216 WHERE `entry` IN (34890, 34892, 34954);
UPDATE `creature_template` SET `npcflag` = `npcflag` | 16777216 WHERE `entry` IN (34840, 37676, 49131, 49132);
DELETE FROM `gossip_menu_option` WHERE `MenuID` IN (11358, 11360) AND `OptionID` = 0;
INSERT INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES
(11358, 0, 0, 'I need to pick up my friends for the party.', 0, 1, 1, 0, 0, 0, 0, '', 0, 0),
(11360, 0, 0, 'I need to pick up my friends for the party.', 0, 1, 1, 0, 0, 0, 0, '', 0, 0);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 15 AND `SourceGroup` IN (11358, 11360) AND `SourceEntry` = 0;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(15, 11358, 0, 0, 0, 9, 0, 14071, 0, 0, 0, 0, 0, '', 'Show gossip option only if player has quest 14071'),
(15, 11360, 0, 0, 0, 9, 0, 14071, 0, 0, 0, 0, 0, '', 'Show gossip option only if player has quest 14071');
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (34957, 34959) AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(34957, 0, 0, 0, 62, 0, 100, 0, 11358, 0, 0, 0, 0, 11, 91551, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Ace - On Gossip Option Select - Cast Keys to the Hot Rod'),
(34959, 0, 0, 0, 62, 0, 100, 0, 11360, 0, 0, 0, 0, 11, 91551, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Izzy - On Gossip Option Select - Cast Keys to the Hot Rod');
DELETE FROM `quest_offer_reward` WHERE `ID` = 14071;
INSERT INTO `quest_offer_reward` (`ID`, `Emote1`, `Emote2`, `Emote3`, `Emote4`, `EmoteDelay1`, `EmoteDelay2`, `EmoteDelay3`, `EmoteDelay4`, `RewardText`, `VerifiedBuild`) VALUES
(14071, 0, 0, 0, 0, 0, 0, 0, 0, 'There they are! Okay, you three make sure that you help $g him:her; out today. $g He''s:She''s; got a lot of stuff to take care of before the party!$B$BWhoohoo! I can''t wait!', 15595);
DELETE FROM `quest_request_items` WHERE `ID` = 14071;
INSERT INTO `quest_request_items` (`ID`, `EmoteOnComplete`, `EmoteOnIncomplete`, `CompletionText`, `VerifiedBuild`) VALUES
(14071, 0, 0, 'Did you pick up all of your friends?', 15595);

SET @ACE_CREDIT := 34957;
SET @GOBBER_CREDIT := 34958;
SET @IZZY_CREDIT := 34959;
SET @MENU_ACE := 11358;
SET @MENU_IZZY := 11360;
SET @MENU_GOBBER := 11359;
SET @TEXT_GOBBER := 15830;
DELETE FROM `gossip_menu` WHERE `MenuID` = @MENU_GOBBER;
INSERT INTO `gossip_menu` (`MenuID`, `TextID`) VALUES (@MENU_GOBBER, @TEXT_GOBBER);
DELETE FROM `gossip_menu_option` WHERE `MenuID` = @MENU_GOBBER;
INSERT INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES
(@MENU_GOBBER, 0, 0, 'I need to pick up my friends for the party.', 0, 1, 1, 0, 0, 0, 0, '', 0, 0);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 15 AND `SourceGroup` = @MENU_GOBBER AND `SourceEntry` = 0;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(15, @MENU_GOBBER, 0, 0, 0, 9, 0, 14071, 0, 0, 0, 0, 0, '', 'Show gossip option only if player has quest 14071');
UPDATE `creature_template` SET `gossip_menu_id` = @MENU_GOBBER WHERE `entry` = @GOBBER_CREDIT;
DELETE FROM `smart_scripts` WHERE `entryorguid` = @GOBBER_CREDIT AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(@GOBBER_CREDIT, 0, 0, 0, 62, 0, 100, 0, @MENU_GOBBER, 0, 0, 0, 0, 11, 91551, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Gobber - On Gossip Option Select - Cast Keys to the Hot Rod');
UPDATE `creature_template` SET `ScriptName` = 'npc_rolling_with_homies_gossip', `AIName` = '' WHERE `entry` IN (@ACE_CREDIT, @IZZY_CREDIT, @GOBBER_CREDIT);
UPDATE `creature_template` SET `npcflag` = `npcflag` | 1 WHERE `entry` IN (@ACE_CREDIT, @IZZY_CREDIT, @GOBBER_CREDIT);
UPDATE `creature_template` SET `ScriptName` = '' WHERE `entry` IN (34890, 34892, 34954);
UPDATE `creature_template` SET `npcflag` = `npcflag` | 16777216 WHERE `entry` IN (34840, 37676, 49131, 49132);
UPDATE `creature_template` SET `unit_flags` = `unit_flags` | 16777216 WHERE `entry` IN (34840, 37676, 49131, 49132);
DELETE FROM `creature_text` WHERE `CreatureID` IN (@ACE_CREDIT, @IZZY_CREDIT, @GOBBER_CREDIT);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(@ACE_CREDIT, 0, 0, 'This is gonna be great! Let''s go!', 12, 0, 100, 1, 0, 0, 0, 0, 'Ace - Quest Complete'),
(@IZZY_CREDIT, 0, 0, 'Woohoo! I love parties!', 12, 0, 100, 1, 0, 0, 0, 0, 'Izzy - Quest Complete'),
(@GOBBER_CREDIT, 0, 0, 'Finally, some action! Let''s ride!', 12, 0, 100, 1, 0, 0, 0, 0, 'Gobber - Quest Complete');
DELETE FROM `quest_offer_reward` WHERE `ID` = 14071;
INSERT INTO `quest_offer_reward` (`ID`, `Emote1`, `Emote2`, `Emote3`, `Emote4`, `EmoteDelay1`, `EmoteDelay2`, `EmoteDelay3`, `EmoteDelay4`, `RewardText`, `VerifiedBuild`) VALUES
(14071, 0, 0, 0, 0, 0, 0, 0, 0, 'There they are! Okay, you three make sure that you help $g him:her; out today. $g He''s:She''s; got a lot of stuff to take care of before the party!$B$BWhoohoo! I can''t wait!', 15595);
DELETE FROM `quest_request_items` WHERE `ID` = 14071;
INSERT INTO `quest_request_items` (`ID`, `EmoteOnComplete`, `EmoteOnIncomplete`, `CompletionText`, `VerifiedBuild`) VALUES
(14071, 0, 0, 'Did you pick up all of your friends?', 15595);

DELETE FROM `smart_scripts` WHERE `entryorguid` IN (34957, 34958, 34959) AND `source_type` = 0;
UPDATE `creature_template` SET `ScriptName` = 'npc_rolling_with_homies_gossip', `AIName` = '' WHERE `entry` IN (34957, 34958, 34959);
UPDATE `creature_template` SET `npcflag` = `npcflag` | 1 WHERE `entry` IN (34957, 34958, 34959);

UPDATE `creature_template` SET `ScriptName` = 'npc_izzy_test', `AIName` = '' WHERE `entry` = 34959;

UPDATE `creature_template` SET `VehicleId` = 181 WHERE `entry` IN (34840, 37676, 49131, 49132);
UPDATE `creature_template` SET `unit_flags` = 524288 WHERE `entry` IN (34840, 37676);
UPDATE `creature_template` SET `unit_flags` = (`unit_flags` | 524288) WHERE `entry` IN (49131, 49132);
UPDATE `creature_template` SET `npcflag` = (`npcflag` | 16777216) WHERE `entry` IN (34840, 37676, 49131, 49132);
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` IN (34840, 37676, 49131, 49132);
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(34840, 46598, 1, 0),
(37676, 46598, 1, 0),
(49131, 46598, 1, 0),
(49132, 46598, 1, 0);
DELETE FROM `vehicle_template_accessory` WHERE `entry` IN (34840, 37676, 49131, 49132);
INSERT INTO `vehicle_template_accessory` (`entry`, `accessory_entry`, `seat_id`, `minion`, `description`, `summontype`, `summontimer`) VALUES
(34840, 0, 1, 0, 'Hot Rod Passenger Seat 1', 6, 30000),
(34840, 0, 2, 0, 'Hot Rod Passenger Seat 2', 6, 30000),
(34840, 0, 3, 0, 'Hot Rod Passenger Seat 3', 6, 30000),
(37676, 0, 1, 0, 'Hot Rod Passenger Seat 1', 6, 30000),
(37676, 0, 2, 0, 'Hot Rod Passenger Seat 2', 6, 30000),
(37676, 0, 3, 0, 'Hot Rod Passenger Seat 3', 6, 30000),
(49131, 0, 1, 0, 'Goblin Hot Rod Passenger Seat 1', 6, 30000),
(49131, 0, 2, 0, 'Goblin Hot Rod Passenger Seat 2', 6, 30000),
(49131, 0, 3, 0, 'Goblin Hot Rod Passenger Seat 3', 6, 30000),
(49132, 0, 1, 0, 'Goblin Hot Rod Passenger Seat 1', 6, 30000),
(49132, 0, 2, 0, 'Goblin Hot Rod Passenger Seat 2', 6, 30000),
(49132, 0, 3, 0, 'Goblin Hot Rod Passenger Seat 3', 6, 30000);
UPDATE `creature_template` SET
`ScriptName` = 'npc_hot_rod_follower',
`AIName` = ''
WHERE `entry` IN (34890, 34892, 34954);
UPDATE `creature_template` SET
`ScriptName` = 'npc_rolling_with_homies_gossip',
`AIName` = ''
WHERE `entry` IN (34957, 34958, 34959);
UPDATE `creature_template` SET `npcflag` = (`npcflag` | 1) WHERE `entry` IN (34890, 34892, 34954);
UPDATE `creature_template` SET `npcflag` = (`npcflag` | 1) WHERE `entry` IN (34957, 34958, 34959);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (34890, 34892, 34954, 34957, 34958, 34959) AND `source_type` = 0;

UPDATE `creature` SET `spawntimesecs` = 30 WHERE `id` IN (34890, 34892, 34954);
UPDATE `creature_template` SET `flags_extra` = (`flags_extra` & ~1) WHERE `entry` IN (34890, 34892, 34954, 34957, 34958, 34959);
UPDATE `creature_template` SET `flags_extra` = (`flags_extra` & ~2) WHERE `entry` IN (34890, 34892, 34954, 34957, 34958, 34959);
DELETE FROM `creature_text` WHERE `CreatureID` IN (34890, 34892, 34954);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(34892, 0, 0, 'Alright! Let''s get this party started!', 12, 0, 100, 1, 0, 0, 0, 0, 'Ace - Picked Up'),
(34954, 0, 0, 'Finally, some action! Let''s ride!', 12, 0, 100, 1, 0, 0, 0, 0, 'Gobber - Picked Up'),
(34890, 0, 0, 'Woohoo! I love parties! Let''s go!', 12, 0, 100, 1, 0, 0, 0, 0, 'Izzy - Picked Up');

UPDATE `creature_template` SET `VehicleId` = 181 WHERE `entry` IN (34840, 37676, 49131, 49132);
DELETE FROM `vehicle_template_accessory` WHERE `entry` IN (34840, 37676, 49131, 49132);
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` IN (34840, 37676, 49131, 49132);
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(34840, 46598, 1, 0),
(37676, 46598, 1, 0),
(49131, 46598, 1, 0),
(49132, 46598, 1, 0);
UPDATE `creature_template` SET
`unit_flags` = `unit_flags` | 524288,
`unit_flags2` = `unit_flags2` | 2048
WHERE `entry` IN (34840, 37676, 49131, 49132);
UPDATE `creature_template` SET `npcflag` = `npcflag` | 16777216 WHERE `entry` IN (34840, 37676, 49131, 49132);
UPDATE `creature_template` SET
`unit_flags` = `unit_flags` & ~768,
`unit_flags2` = `unit_flags2` | 2048
WHERE `entry` IN (34890, 34892, 34954);

DELETE FROM `vehicle_template_accessory` WHERE `entry` IN (34840, 37676, 49131, 49132);
UPDATE `creature_template` SET `VehicleId` = 181 WHERE `entry` IN (34840, 37676, 49131, 49132);
UPDATE `creature_template` SET
`unit_flags` = 524288,
`unit_flags2` = 2048
WHERE `entry` IN (34840, 37676);
UPDATE `creature_template` SET
`unit_flags` = `unit_flags` | 524288,
`unit_flags2` = 2048
WHERE `entry` IN (49131, 49132);
UPDATE `creature_template` SET
`npcflag` = `npcflag` | 16777216
WHERE `entry` IN (34840, 37676, 49131, 49132);
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` IN (34840, 37676, 49131, 49132);
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(34840, 46598, 1, 0),
(37676, 46598, 1, 0),
(49131, 46598, 1, 0),
(49132, 46598, 1, 0);
UPDATE `creature_template` SET
`ScriptName` = 'npc_hot_rod_follower',
`AIName` = '',
`npcflag` = `npcflag` | 1
WHERE `entry` IN (34890, 34892, 34954);
UPDATE `creature_template` SET
`ScriptName` = 'npc_rolling_with_homies_gossip',
`AIName` = '',
`npcflag` = `npcflag` | 1
WHERE `entry` IN (34957, 34958, 34959);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (34890, 34892, 34954, 34957, 34958, 34959) AND `source_type` = 0;
DELETE FROM `gossip_menu_option` WHERE `MenuID` IN (11358, 11359, 11360) AND `OptionID` = 0;
INSERT INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES
(11358, 0, 0, 'Hop in!', 34905, 1, 1, 0, 0, 0, 0, '', 0, 0),
(11359, 0, 0, 'Hop in!', 34905, 1, 1, 0, 0, 0, 0, '', 0, 0),
(11360, 0, 0, 'Hop in!', 34905, 1, 1, 0, 0, 0, 0, '', 0, 0);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 15 AND `SourceGroup` IN (11358, 11359, 11360) AND `SourceEntry` = 0;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(15, 11358, 0, 0, 0, 9, 0, 14071, 0, 0, 0, 0, 0, '', 'Show gossip only if player has quest 14071'),
(15, 11359, 0, 0, 0, 9, 0, 14071, 0, 0, 0, 0, 0, '', 'Show gossip only if player has quest 14071'),
(15, 11360, 0, 0, 0, 9, 0, 14071, 0, 0, 0, 0, 0, '', 'Show gossip only if player has quest 14071');

DELETE FROM `vehicle_template_accessory` WHERE `entry` IN (34840, 37676, 49131, 49132);
UPDATE `creature_template` SET
`VehicleId` = 181,
`ScriptName` = 'npc_hot_rod_vehicle',
`AIName` = ''
WHERE `entry` IN (34840, 37676, 49131, 49132);
UPDATE `creature_template` SET
`unit_flags` = 524288,
`unit_flags2` = 2048
WHERE `entry` IN (34840, 37676, 49131, 49132);
UPDATE `creature_template` SET
`npcflag` = `npcflag` | 16777216
WHERE `entry` IN (34840, 37676, 49131, 49132);
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` IN (34840, 37676, 49131, 49132);
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(34840, 46598, 1, 0),
(37676, 46598, 1, 0),
(49131, 46598, 1, 0),
(49132, 46598, 1, 0);
UPDATE `creature_template` SET
`ScriptName` = 'npc_hot_rod_follower',
`AIName` = '',
`npcflag` = `npcflag` | 1
WHERE `entry` IN (34890, 34892, 34954);
DELETE FROM `creature_template_addon` WHERE `entry` IN (34890, 34892, 34954);
INSERT INTO `creature_template_addon` (`entry`, `auras`) VALUES
(34890, '66405'),
(34892, '66403'),
(34954, '66404');
UPDATE `creature_template` SET
`ScriptName` = 'npc_rolling_with_homies_gossip',
`AIName` = '',
`npcflag` = `npcflag` | 1
WHERE `entry` IN (34957, 34958, 34959);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (34890, 34892, 34954, 34957, 34958, 34959) AND `source_type` = 0;
DELETE FROM `gossip_menu_option` WHERE `MenuID` IN (11358, 11359, 11360) AND `OptionID` = 0;
INSERT INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES
(11358, 0, 0, 'Hop in!', 34905, 1, 1, 0, 0, 0, 0, '', 0, 0),
(11359, 0, 0, 'Hop in!', 34905, 1, 1, 0, 0, 0, 0, '', 0, 0),
(11360, 0, 0, 'Hop in!', 34905, 1, 1, 0, 0, 0, 0, '', 0, 0);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 15 AND `SourceGroup` IN (11358, 11359, 11360) AND `SourceEntry` = 0;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(15, 11358, 0, 0, 0, 9, 0, 14071, 0, 0, 0, 0, 0, '', 'Show gossip only if player has quest 14071'),
(15, 11359, 0, 0, 0, 9, 0, 14071, 0, 0, 0, 0, 0, '', 'Show gossip only if player has quest 14071'),
(15, 11360, 0, 0, 0, 9, 0, 14071, 0, 0, 0, 0, 0, '', 'Show gossip only if player has quest 14071');
DELETE FROM `quest_offer_reward` WHERE `ID` = 14071;
INSERT INTO `quest_offer_reward` (`ID`, `Emote1`, `Emote2`, `Emote3`, `Emote4`, `EmoteDelay1`, `EmoteDelay2`, `EmoteDelay3`, `EmoteDelay4`, `RewardText`, `VerifiedBuild`) VALUES
(14071, 0, 0, 0, 0, 0, 0, 0, 0, 'There they are! Okay, you three make sure that you help $g him:her; out today. $g He''s:She''s; got a lot of stuff to take care of before the party!$B$BWhoohoo! I can''t wait!', 15595);
DELETE FROM `quest_request_items` WHERE `ID` = 14071;
INSERT INTO `quest_request_items` (`ID`, `EmoteOnComplete`, `EmoteOnIncomplete`, `CompletionText`, `VerifiedBuild`) VALUES
(14071, 0, 0, 'Did you pick up all of your friends?', 15595);

UPDATE `creature_template` SET
`ScriptName` = 'npc_rolling_with_homies_gossip',
`AIName` = '',
`faction` = 35,
`npcflag` = `npcflag` | 1
WHERE `entry` IN (34957, 34958, 34959);
UPDATE `creature_template` SET
`unit_flags` = 768,
`flags_extra` = `flags_extra` | 128
WHERE `entry` IN (34957, 34958, 34959);
UPDATE `creature_template` SET
`ScriptName` = 'npc_hot_rod_follower',
`AIName` = '',
`npcflag` = `npcflag` | 1
WHERE `entry` IN (34890, 34892, 34954);
UPDATE `creature_template` SET
`VehicleId` = 181,
`ScriptName` = 'npc_hot_rod_vehicle',
`AIName` = ''
WHERE `entry` IN (34840, 37676, 49131, 49132);
UPDATE `creature_template` SET
`unit_flags` = 524288,
`unit_flags2` = 2048,
`npcflag` = `npcflag` | 16777216
WHERE `entry` IN (34840, 37676, 49131, 49132);
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` IN (34840, 37676, 49131, 49132);
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(34840, 46598, 1, 0),
(37676, 46598, 1, 0),
(49131, 46598, 1, 0),
(49132, 46598, 1, 0);
DELETE FROM `vehicle_template_accessory` WHERE `entry` IN (34840, 37676, 49131, 49132);
DELETE FROM `creature_template_addon` WHERE `entry` IN (34890, 34892, 34954);
INSERT INTO `creature_template_addon` (`entry`, `auras`) VALUES
(34890, '66405'),
(34892, '66403'),
(34954, '66404');
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (34890, 34892, 34954, 34957, 34958, 34959) AND `source_type` = 0;

DELETE FROM `gossip_menu` WHERE `MenuID` IN (11358, 11359, 11360) AND `TextID` = 0;
INSERT INTO `gossip_menu` (`MenuID`, `TextID`, `VerifiedBuild`) VALUES
(11358, 0, 0),
(11359, 0, 0),
(11360, 0, 0);
DELETE FROM `smart_scripts` WHERE `entryorguid` = 34890 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(34890, 0, 0, 0, 62, 0, 100, 0, 11358, 0, 0, 0, 0, 100, 66600, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Izzy - On gossip select (option 0): Cast spell 66600');
DELETE FROM `smart_scripts` WHERE `entryorguid` = 34954 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(34954, 0, 0, 0, 62, 0, 100, 0, 11359, 0, 0, 0, 0, 100, 66599, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Gobber - On gossip select (option 0): Cast spell 66599');
DELETE FROM `smart_scripts` WHERE `entryorguid` = 34892 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(34892, 0, 0, 0, 62, 0, 100, 0, 11360, 0, 0, 0, 0, 100, 66597, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Ace - On gossip select (option 0): Cast spell 66597');

UPDATE `creature_template` SET
`VehicleId` = 181,
`ScriptName` = 'npc_hot_rod_vehicle',
`AIName` = ''
WHERE `entry` IN (34840, 37676, 49131, 49132);
UPDATE `creature_template` SET
`unit_flags` = 524288,
`unit_flags2` = 2048,
`npcflag` = `npcflag` | 16777216
WHERE `entry` IN (34840, 37676, 49131, 49132);
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` IN (34840, 37676, 49131, 49132);
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(34840, 46598, 1, 0),
(37676, 46598, 1, 0),
(49131, 46598, 1, 0),
(49132, 46598, 1, 0);
DELETE FROM `vehicle_template_accessory` WHERE `entry` IN (34840, 37676, 49131, 49132);
UPDATE `creature_template` SET
`ScriptName` = 'npc_rolling_with_homies_gossip',
`AIName` = '',
`faction` = 35,
`unit_flags` = 768,
`flags_extra` = `flags_extra` | 128
WHERE `entry` IN (34957, 34958, 34959);
UPDATE `creature_template` SET
`ScriptName` = 'npc_hot_rod_follower',
`AIName` = '',
`npcflag` = `npcflag` | 1
WHERE `entry` IN (34890, 34892, 34954);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 18 AND `SourceEntry` = 46598 AND `SourceGroup` IN (34840, 37676, 49131, 49132);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(18, 34840, 46598, 0, 0, 31, 0, 4, 0, 0, 0, 0, 0, '', 'Spellclick only available to players'),
(18, 37676, 46598, 0, 0, 31, 0, 4, 0, 0, 0, 0, 0, '', 'Spellclick only available to players'),
(18, 49131, 46598, 0, 0, 31, 0, 4, 0, 0, 0, 0, 0, '', 'Spellclick only available to players'),
(18, 49132, 46598, 0, 0, 31, 0, 4, 0, 0, 0, 0, 0, '', 'Spellclick only available to players');
DELETE FROM `gossip_menu_option` WHERE `MenuID` IN (11358, 11360, 11359) AND `OptionID` = 0;
INSERT INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES
(11358, 0, 0, 'Hop in the Hot Rod, Ace!', 0, 1, 1, 0, 0, 0, 0, '', 0, 0),
(11359, 0, 0, 'Hop in the Hot Rod, Izzy!', 0, 1, 1, 0, 0, 0, 0, '', 0, 0),
(11360, 0, 0, 'Hop in the Hot Rod, Gobber!', 0, 1, 1, 0, 0, 0, 0, '', 0, 0);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 15 AND `SourceGroup` IN (11358, 11359, 11360) AND `SourceEntry` = 0;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(15, 11358, 0, 0, 0, 9, 0, 14071, 0, 0, 0, 0, 0, '', 'Show gossip only if player has quest 14071'),
(15, 11359, 0, 0, 0, 9, 0, 14071, 0, 0, 0, 0, 0, '', 'Show gossip only if player has quest 14071'),
(15, 11360, 0, 0, 0, 9, 0, 14071, 0, 0, 0, 0, 0, '', 'Show gossip only if player has quest 14071');

UPDATE `creature_template` SET
`modelid1` = 29482,
`modelid2` = 0,
`modelid3` = 0,
`modelid4` = 0,
`scale` = 1,
`unit_flags` = 768,
`unit_flags2` = 2048,
`dynamicflags` = 0,
`type_flags` = 0,
`flags_extra` = 128
WHERE `entry` = 34959;
UPDATE `creature_template` SET
`modelid1` = 29481,
`modelid2` = 0,
`modelid3` = 0,
`modelid4` = 0,
`scale` = 1,
`unit_flags` = 768,
`unit_flags2` = 2048,
`dynamicflags` = 0,
`type_flags` = 0,
`flags_extra` = 128
WHERE `entry` = 34957;
UPDATE `creature_template` SET
`modelid1` = 29483,
`modelid2` = 0,
`modelid3` = 0,
`modelid4` = 0,
`scale` = 1,
`unit_flags` = 768,
`unit_flags2` = 2048,
`dynamicflags` = 0,
`type_flags` = 0,
`flags_extra` = 128
WHERE `entry` = 34958;
DELETE FROM `spell_script_names` WHERE `spell_id` IN (66600, 66599, 66597);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(66600, ''),
(66599, ''),
(66597, '');
DELETE FROM `creature` WHERE `id` IN (34959, 34957, 34958);

DELETE FROM `creature_template_model` WHERE `CreatureID` IN (34959, 34957, 34958);
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `Probability`, `VerifiedBuild`) VALUES
(34959, 0, 29482, 1, 15595),
(34957, 0, 29481, 1, 15595),
(34958, 0, 29483, 1, 15595);
UPDATE `creature_template_addon` SET
`visibilityDistanceType` = 0,
`VisFlags` = 0,
`auras` = NULL
WHERE `entry` IN (34959, 34957, 34958);
UPDATE `creature_template` SET
`unit_flags` = 768,
`unit_flags2` = 0,
`dynamicflags` = 0,
`type_flags` = 0,
`flags_extra` = 0
WHERE `entry` IN (34959, 34957, 34958);
UPDATE `creature_template` SET
`modelid1` = 29482,
`modelid2` = 0,
`modelid3` = 0,
`modelid4` = 0
WHERE `entry` = 34959;
UPDATE `creature_template` SET
`modelid1` = 29481,
`modelid2` = 0,
`modelid3` = 0,
`modelid4` = 0
WHERE `entry` = 34957;
UPDATE `creature_template` SET
`modelid1` = 29483,
`modelid2` = 0,
`modelid3` = 0,
`modelid4` = 0
WHERE `entry` = 34958;

DELETE FROM `creature_template_model` WHERE `CreatureID` IN (34959, 34957, 34958);
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `Probability`, `VerifiedBuild`) VALUES
(34959, 0, 29482, 1, 15595),
(34957, 0, 29481, 1, 15595),
(34958, 0, 29483, 1, 15595);
UPDATE `creature_template_addon` SET
`visibilityDistanceType` = 0
WHERE `entry` IN (34959, 34957, 34958);
UPDATE `creature_template` SET
`modelid1` = 29482,
`modelid2` = 0,
`modelid3` = 0,
`modelid4` = 0,
`VerifiedBuild` = 15595
WHERE `entry` = 34959;
UPDATE `creature_template` SET
`modelid1` = 29481,
`modelid2` = 0,
`modelid3` = 0,
`modelid4` = 0,
`VerifiedBuild` = 15595
WHERE `entry` = 34957;
UPDATE `creature_template` SET
`modelid1` = 29483,
`modelid2` = 0,
`modelid3` = 0,
`modelid4` = 0,
`VerifiedBuild` = 15595
WHERE `entry` = 34958;
UPDATE `creature_template` SET
`modelid1` = 29482,
`modelid2` = 0,
`modelid3` = 0,
`modelid4` = 0
WHERE `entry` = 34890;
UPDATE `creature_template` SET
`modelid1` = 29481,
`modelid2` = 0,
`modelid3` = 0,
`modelid4` = 0
WHERE `entry` = 34892;
UPDATE `creature_template` SET
`modelid1` = 29483,
`modelid2` = 0,
`modelid3` = 0,
`modelid4` = 0
WHERE `entry` = 34954;

UPDATE `creature_template` SET `modelid1` = 29495 WHERE `entry` = 34957;
UPDATE `creature_template` SET `modelid1` = 32385 WHERE `entry` = 34958;
UPDATE `creature_template_model` SET `CreatureDisplayID` = 29495 WHERE `CreatureID` = 34957;
UPDATE `creature_template_model` SET `CreatureDisplayID` = 32385 WHERE `CreatureID` = 34958;

DELETE FROM `vehicle_template_accessory` WHERE `entry` IN (34840, 37676, 49131, 49132);
INSERT INTO `vehicle_template_accessory` (`entry`, `accessory_entry`, `seat_id`, `minion`, `description`, `summontype`, `summontimer`) VALUES
(34840, 0, 1, 0, 'Hot Rod Seat 1 (Izzy/Ace/Gobber)', 6, 30000),
(34840, 0, 2, 0, 'Hot Rod Seat 2 (Izzy/Ace/Gobber)', 6, 30000),
(34840, 0, 3, 0, 'Hot Rod Seat 3 (Izzy/Ace/Gobber)', 6, 30000),
(37676, 0, 1, 0, 'Hot Rod Variant Seat 1', 6, 30000),
(37676, 0, 2, 0, 'Hot Rod Variant Seat 2', 6, 30000),
(37676, 0, 3, 0, 'Hot Rod Variant Seat 3', 6, 30000),
(49131, 0, 1, 0, 'Goblin Hot Rod Seat 1', 6, 30000),
(49131, 0, 2, 0, 'Goblin Hot Rod Seat 2', 6, 30000),
(49131, 0, 3, 0, 'Goblin Hot Rod Seat 3', 6, 30000),
(49132, 0, 1, 0, 'Goblin Hot Rod Variant Seat 1', 6, 30000),
(49132, 0, 2, 0, 'Goblin Hot Rod Variant Seat 2', 6, 30000),
(49132, 0, 3, 0, 'Goblin Hot Rod Variant Seat 3', 6, 30000);

DELETE FROM `vehicle_seat_addon` WHERE `SeatEntry` IN (2082, 2083, 2084);
INSERT INTO `vehicle_seat_addon`
(`SeatEntry`, `SeatOffsetX`, `SeatOffsetY`, `SeatOffsetZ`,
`SeatOffsetO`, `ExitParamX`, `ExitParamY`, `ExitParamZ`,
`ExitParamO`, `ExitParamValue`)
VALUES
(2082, -1.510319, -0.630674, 0.991446, 0, 0, 0, 0, 0, 0),
(2083, -1.810319,  0.069326, 0.991446, 0, 0, 0, 0, 0, 0),
(2084, -1.660319,  0.678672, 0.991446, 0, 0, 0, 0, 0, 0);