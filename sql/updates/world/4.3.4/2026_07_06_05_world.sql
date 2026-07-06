-- Lord Rhyolith encounter completion: 10/25 Normal + 10/25 Heroic

-- Missing boss texts: armor-break transition (groups 6/7), death (group 8) and the
-- Thermal Ignition add-wave yells (new group 10), all taken from Firelands sniffs
DELETE FROM `creature_text` WHERE `CreatureID` = 52558 AND `GroupID` IN (6, 7, 8, 10);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(52558, 6, 0, '%s stumbles as his armor is shattered!', 41, 0, 100, 0, 0, 0, 0, 51766, 0, 'Lord Rhyolith - Announce Armor Broken'),
(52558, 7, 0, 'Eons I have slept undisturbed... Now this... Creatures of flesh, now you will BURN!', 14, 0, 100, 0, 0, 24558, 0, 52240, 0, 'Lord Rhyolith - Armor Broken'),
(52558, 8, 0, 'Broken. Mnngghhh... broken...', 14, 0, 100, 0, 0, 24545, 0, 52246, 0, 'Lord Rhyolith - Death'),
(52558, 10, 0, 'Sear the flesh from their tiny frames.', 14, 0, 100, 0, 0, 24562, 0, 52248, 0, 'Lord Rhyolith - Thermal Ignition'),
(52558, 10, 1, 'Succumb to living flame.', 14, 0, 100, 0, 0, 24551, 0, 52242, 0, 'Lord Rhyolith - Thermal Ignition');

-- Molten Armor (volcano empowerment): the 75%-50% transitional boss entry 54192 was
-- missing from the implicit-target conditions, unlike its siblings 52558/54199/53772
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` IN (98255, 101157, 101158, 101159) AND `ConditionValue2` = 54192;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 98255,  0, 0, 31, 0, 3, 54192, 0, 0, 0, 0, '', 'Molten Armor - Target Lord Rhyolith'),
(13, 1, 101157, 0, 0, 31, 0, 3, 54192, 0, 0, 0, 0, '', 'Molten Armor - Target Lord Rhyolith'),
(13, 1, 101158, 0, 0, 31, 0, 3, 54192, 0, 0, 0, 0, '', 'Molten Armor - Target Lord Rhyolith'),
(13, 1, 101159, 0, 0, 31, 0, 3, 54192, 0, 0, 0, 0, '', 'Molten Armor - Target Lord Rhyolith');

-- Fuse (Liquid Obsidian, Heroic): script effects target Lord Rhyolith by entry
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 99875;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 99875, 0, 0, 31, 0, 3, 52558, 0, 0, 0, 0, '', 'Fuse - Target Lord Rhyolith'),
(13, 1, 99875, 0, 0, 31, 0, 3, 54192, 0, 0, 0, 0, '', 'Fuse - Target Lord Rhyolith'),
(13, 1, 99875, 0, 0, 31, 0, 3, 54199, 0, 0, 0, 0, '', 'Fuse - Target Lord Rhyolith'),
(13, 4, 99875, 0, 0, 31, 0, 3, 52558, 0, 0, 0, 0, '', 'Fuse - Target Lord Rhyolith'),
(13, 4, 99875, 0, 0, 31, 0, 3, 54192, 0, 0, 0, 0, '', 'Fuse - Target Lord Rhyolith'),
(13, 4, 99875, 0, 0, 31, 0, 3, 54199, 0, 0, 0, 0, '', 'Fuse - Target Lord Rhyolith');

DELETE FROM `spell_script_names` WHERE `spell_id` = 99875;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(99875, 'spell_rhyolith_fuse');

-- Lord Rhyolith dies as entry 53772 (his phase-two form), so loot is drawn from that
-- chain. 53773 (25N) had no loot at all and the chain missed the legendary quest items;
-- point all four at the maintained TDB tables of the base chain and drop the stale rows
UPDATE `creature_template` SET `lootid` = 52558 WHERE `entry` = 53772; -- 10N
UPDATE `creature_template` SET `lootid` = 52559 WHERE `entry` = 53773; -- 25N
UPDATE `creature_template` SET `lootid` = 52560 WHERE `entry` = 53774; -- 10H
UPDATE `creature_template` SET `lootid` = 52561 WHERE `entry` = 53775; -- 25H
DELETE FROM `creature_loot_template` WHERE `entry` IN (53772, 53774, 53775);
