-- Add creature text for Drowning Watchman
DELETE FROM `creature_text` WHERE `CreatureID`=36440;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(36440, 0, 0, 'The land gave way from under our feet... I thought I was dead. I owe you one.', 12, 0, 100, 0, 0, 0, 36418, 0, 'Drowning Watchman - On Rescue'),
(36440, 0, 1, 'Thank you... *gasp* I owe you my life.', 12, 0, 100, 0, 0, 0, 36417, 0, 'Drowning Watchman - On Rescue'),
(36440, 0, 2, 'I... I thought I was a goner. Thanks.', 12, 0, 100, 0, 0, 0, 36419, 0, 'Drowning Watchman - On Rescue');

-- Set creature template flags (combining all necessary flags)
UPDATE `creature_template` SET `npcflag`=16777216, `unit_flags`=33536 WHERE `entry`=36440;

-- Remove previous gossip-related entries
DELETE FROM `gossip_menu` WHERE `MenuID` IN (10850, 10851);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=14 AND `SourceGroup` IN (10850, 10851);

-- Remove duplicate spawns
DELETE c1 FROM creature c1
INNER JOIN creature c2 
WHERE c1.guid > c2.guid 
AND c1.id = 36440 
AND c2.id = 36440 
AND c1.position_x = c2.position_x 
AND c1.position_y = c2.position_y;

-- Clean up any existing scripts and spellclick spells
DELETE FROM `smart_scripts` WHERE `entryorguid`=36440 AND `source_type`=0;
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry`=36440;

-- Add updated SmartAI scripts
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(36440, 0, 0, 1, 73, 0, 100, 0, 0, 0, 0, 0, 11, 47020, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Drowning Watchman - On Spellclick - Cast Ride Vehicle'),
(36440, 0, 1, 2, 61, 0, 100, 0, 0, 0, 0, 0, 83, 16777216, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Drowning Watchman - Linked - Remove Spellclick flag'),
(36440, 0, 2, 0, 61, 0, 100, 0, 0, 0, 0, 0, 28, 68730, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Drowning Watchman - Linked - Remove Drowning aura'),
(36440, 0, 3, 4, 8, 0, 100, 0, 68739, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Drowning Watchman - On Spell Hit (Drowning Militia Dummy) - Say Text'),
(36440, 0, 4, 0, 61, 0, 100, 0, 0, 0, 0, 0, 41, 10000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Drowning Watchman - Linked - Despawn in 10 seconds');

-- Ensure quest requirements are set correctly
UPDATE `quest_template` SET `RequiredNpcOrGo1`=36450, `RequiredNpcOrGoCount1`=4 WHERE `Id`=14395;

-- Link area trigger to script
INSERT INTO `areatrigger_scripts` VALUES (5437, 'at_gasping_for_breath');

-- Add spellclick spell
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 36440;
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES 
(36440, 68735, 1, 0);

-- Add conditions for spellclick
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 18 AND `SourceGroup` = 36440;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(18, 36440, 68735, 0, 0, 9, 0, 14395, 0, 0, 0, 'Show spellclick if player has quest 14395 active'),
(18, 36440, 68735, 0, 0, 28, 0, 14395, 0, 0, 1, 'Hide spellclick if player has completed quest 14395');