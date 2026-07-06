-- Beth'tilac (Firelands) encounter completion: 10/25 Normal + 10/25 Heroic
--
-- Boss HP (retail): 20,871,756 / 62,615,268 / 32,810,744 / 98,518,124
-- basehp3: level 88 class 2 = 85892 -> HealthModifier 243 / 729 / 382 / 1147 (exact)
UPDATE `creature_template` SET `HealthModifier` = 243  WHERE `entry` = 52498; -- 10N (was 206)
UPDATE `creature_template` SET `HealthModifier` = 729  WHERE `entry` = 53576; -- 25N (was 619)
UPDATE `creature_template` SET `HealthModifier` = 382  WHERE `entry` = 53577; -- 10H (was 324)
UPDATE `creature_template` SET `HealthModifier` = 1147 WHERE `entry` = 53578; -- 25H (was 974)

-- Add HP (retail, basehp3 level 85 = 77490):
-- Cinderweb Spinner    232,470 / 232,470 / 309,960 / 309,960 -> 3.0 / 3.0 / 4.0 / 4.0
-- Cinderweb Spiderling  77,490 / 100,737 / 116,235 / 139,482 -> 1.0 / 1.3 / 1.5 / 1.8
-- Engorged Broodling   (heroic only) 247,968 / 743,904       -> 3.2 (10H) / 9.6 (25H)
UPDATE `creature_template` SET `HealthModifier` = 3.0 WHERE `entry` = 52524;
UPDATE `creature_template` SET `HealthModifier` = 1.0 WHERE `entry` = 52447;
UPDATE `creature_template` SET `HealthModifier` = 3.2 WHERE `entry` = 53745;

-- Difficulty clone rows for the adds that lack them (Cinderweb Drone 52581 already
-- has 53582/53583/53584). Entries 990001-990008 verified free.
DROP TEMPORARY TABLE IF EXISTS `bethtilac_clone`;
CREATE TEMPORARY TABLE `bethtilac_clone` AS SELECT * FROM `creature_template` WHERE `entry` = 52524;
UPDATE `bethtilac_clone` SET `difficulty_entry_1` = 0, `difficulty_entry_2` = 0, `difficulty_entry_3` = 0, `ScriptName` = '';
UPDATE `bethtilac_clone` SET `entry` = 990001, `HealthModifier` = 3.0; INSERT INTO `creature_template` SELECT * FROM `bethtilac_clone`; -- Spinner 25N
UPDATE `bethtilac_clone` SET `entry` = 990002, `HealthModifier` = 4.0; INSERT INTO `creature_template` SELECT * FROM `bethtilac_clone`; -- Spinner 10H
UPDATE `bethtilac_clone` SET `entry` = 990003, `HealthModifier` = 4.0; INSERT INTO `creature_template` SELECT * FROM `bethtilac_clone`; -- Spinner 25H
DROP TEMPORARY TABLE `bethtilac_clone`;

CREATE TEMPORARY TABLE `bethtilac_clone` AS SELECT * FROM `creature_template` WHERE `entry` = 52447;
UPDATE `bethtilac_clone` SET `difficulty_entry_1` = 0, `difficulty_entry_2` = 0, `difficulty_entry_3` = 0, `ScriptName` = '';
UPDATE `bethtilac_clone` SET `entry` = 990004, `HealthModifier` = 1.3; INSERT INTO `creature_template` SELECT * FROM `bethtilac_clone`; -- Spiderling 25N
UPDATE `bethtilac_clone` SET `entry` = 990005, `HealthModifier` = 1.5; INSERT INTO `creature_template` SELECT * FROM `bethtilac_clone`; -- Spiderling 10H
UPDATE `bethtilac_clone` SET `entry` = 990006, `HealthModifier` = 1.8; INSERT INTO `creature_template` SELECT * FROM `bethtilac_clone`; -- Spiderling 25H
DROP TEMPORARY TABLE `bethtilac_clone`;

CREATE TEMPORARY TABLE `bethtilac_clone` AS SELECT * FROM `creature_template` WHERE `entry` = 53745;
UPDATE `bethtilac_clone` SET `difficulty_entry_1` = 0, `difficulty_entry_2` = 0, `difficulty_entry_3` = 0, `ScriptName` = '';
UPDATE `bethtilac_clone` SET `entry` = 990007, `HealthModifier` = 3.2; INSERT INTO `creature_template` SELECT * FROM `bethtilac_clone`; -- Broodling 10H
UPDATE `bethtilac_clone` SET `entry` = 990008, `HealthModifier` = 9.6; INSERT INTO `creature_template` SELECT * FROM `bethtilac_clone`; -- Broodling 25H
DROP TEMPORARY TABLE `bethtilac_clone`;

UPDATE `creature_template` SET `difficulty_entry_1` = 990001, `difficulty_entry_2` = 990002, `difficulty_entry_3` = 990003 WHERE `entry` = 52524;
UPDATE `creature_template` SET `difficulty_entry_1` = 990004, `difficulty_entry_2` = 990005, `difficulty_entry_3` = 990006 WHERE `entry` = 52447;
UPDATE `creature_template` SET `difficulty_entry_2` = 990007, `difficulty_entry_3` = 990008 WHERE `entry` = 53745;

-- Script bindings (base entries only; difficulty clones route through the base script)
UPDATE `creature_template` SET `ScriptName` = 'boss_bethtilac'           WHERE `entry` = 52498;
UPDATE `creature_template` SET `ScriptName` = 'npc_cinderweb_spinner'    WHERE `entry` = 52524;
UPDATE `creature_template` SET `ScriptName` = 'npc_cinderweb_drone'      WHERE `entry` = 52581;
UPDATE `creature_template` SET `ScriptName` = 'npc_cinderweb_spiderling' WHERE `entry` = 52447;
UPDATE `creature_template` SET `ScriptName` = 'npc_engorged_broodling'   WHERE `entry` = 53745;
UPDATE `creature_template` SET `ScriptName` = 'npc_spiderweb_filament'   WHERE `entry` = 53082;
UPDATE `creature_template` SET `ScriptName` = 'npc_web_rip'              WHERE `entry` = 53474;

-- Spiderweb Filament: rideable vehicle (sniffed: spellclick 98297 Ride Vehicle,
-- SMSG_SET_VEHICLE_REC_ID 1711 while riding)
UPDATE `creature_template` SET `VehicleId` = 1711, `npcflag` = `npcflag` | 0x01000000 WHERE `entry` = 53082;
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 53082;
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(53082, 98297, 1, 0);

-- SpellDifficulty chains: all present in the client 4.3.4 SpellDifficulty.dbc
-- (3872/3822/3823/3837/3772/3840). The world spelldifficulty_dbc table may
-- only ADD entries missing from the client DBC - duplicating a client id
-- trips the DBCDatabaseLoader unique-index assertion and aborts server boot.
DELETE FROM `spelldifficulty_dbc` WHERE `id` IN (3872, 3822, 3823, 3837, 3772, 3840);

-- Spell scripts: bound to every ID of each difficulty chain (spell scripts do not
-- follow spelldifficulty links)
DELETE FROM `spell_script_names` WHERE `ScriptName` IN
('spell_bethtilac_ember_flare', 'spell_bethtilac_venom_rain', 'spell_bethtilac_smoldering_devastation',
 'spell_bethtilac_fixate', 'spell_bethtilac_widows_kiss');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(98934,  'spell_bethtilac_ember_flare'),
(100648, 'spell_bethtilac_ember_flare'),
(100834, 'spell_bethtilac_ember_flare'),
(100835, 'spell_bethtilac_ember_flare'),
(99859,  'spell_bethtilac_ember_flare'),
(100649, 'spell_bethtilac_ember_flare'),
(100935, 'spell_bethtilac_ember_flare'),
(100936, 'spell_bethtilac_ember_flare'),
(99333,  'spell_bethtilac_venom_rain'),
(101128, 'spell_bethtilac_venom_rain'),
(101129, 'spell_bethtilac_venom_rain'),
(101130, 'spell_bethtilac_venom_rain'),
(99052,  'spell_bethtilac_smoldering_devastation'),
(99526,  'spell_bethtilac_fixate'),
(100014, 'spell_bethtilac_fixate'),
(99476,  'spell_bethtilac_widows_kiss');

-- Raid emotes (sniffed broadcast texts; Beth'tilac has no voice lines)
DELETE FROM `creature_text` WHERE `CreatureID` = 52498;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(52498, 0, 0, '%s''s smoldering body begins to flicker and combust!', 41, 0, 100, 0, 0, 0, 8715,  3, 'Bethtilac - EMOTE_DEVASTATION'),
(52498, 1, 0, 'Spiderlings have been roused from their nest!',        41, 0, 100, 0, 0, 0, 11778, 3, 'Bethtilac - EMOTE_SPIDERLINGS');
