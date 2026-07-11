-- Madness of Deathwing: retail-era health retune, Raid Finder support,
-- heroic mechanics (Corrupting Parasite / Congealing Blood), Chromatic
-- Champion worldstates and script bindings.

-- ---------------------------------------------------------------------------
-- Deathwing (body 56173 + head 57962 share health and must stay identical).
-- User retail values: 10H 106,901,181 / 25H 320,763,669 (= TDB x 0.70).
-- GenerateHealth works in float32, so the closest representable values land
-- at 106,901,184 (+3) and 320,763,680 (+11). 10N/25N TDB values already match
-- retail (127,120,160 / 381,360,480) and stay untouched.
-- ---------------------------------------------------------------------------
UPDATE `creature_template` SET `HealthModifier` = 1244.6 WHERE `entry` IN (58000, 58125); -- 10H body/head
UPDATE `creature_template` SET `HealthModifier` = 3734.5 WHERE `entry` IN (58001, 58126); -- 25H body/head

-- Heroic adds x 0.70 (same era ratio as the boss, user-approved)
UPDATE `creature_template` SET `HealthModifier` = 327.6 WHERE `entry` IN (58131, 58129, 58133); -- Arm/Wing Tentacles 10H (~28.14M)
UPDATE `creature_template` SET `HealthModifier` = 982.8 WHERE `entry` IN (58132, 58130, 58134); -- Arm/Wing Tentacles 25H (~84.41M)
UPDATE `creature_template` SET `HealthModifier` = 147   WHERE `entry` = 58137; -- Mutated Corruption 10H (~12.63M)
UPDATE `creature_template` SET `HealthModifier` = 420   WHERE `entry` = 58138; -- Mutated Corruption 25H (~36.07M)
UPDATE `creature_template` SET `HealthModifier` = 28.7  WHERE `entry` = 58127; -- Elementium Terror 10H (~2.38M)
UPDATE `creature_template` SET `HealthModifier` = 87.5  WHERE `entry` = 58128; -- Elementium Terror 25H (~7.26M)
UPDATE `creature_template` SET `HealthModifier` = 14.7  WHERE `entry` IN (58140, 58141); -- Elementium Fragment 10H/25H (~1.22M)
UPDATE `creature_template` SET `HealthModifier` = 2.1   WHERE `entry` = 58142; -- Blistering Tentacle 10H (~174k)
UPDATE `creature_template` SET `HealthModifier` = 6.3   WHERE `entry` = 58143; -- Blistering Tentacle 25H (~523k)
UPDATE `creature_template` SET `HealthModifier` = 11.2  WHERE `entry` = 58135; -- Regenerative Blood 10H (~930k)
UPDATE `creature_template` SET `HealthModifier` = 36.4  WHERE `entry` = 58136; -- Regenerative Blood 25H (~3.02M)

-- Heroic-only adds: base row serves 10H (raid fallback mode 2 -> base), the
-- 25N difficulty slot serves 25H (mode 3 -> 1)
UPDATE `creature_template` SET `HealthModifier` = 26.6 WHERE `entry` = 57479; -- Corrupting Parasite 10H (~2.21M)
UPDATE `creature_template` SET `HealthModifier` = 79.8 WHERE `entry` = 58034; -- Corrupting Parasite 25H (~6.62M)
UPDATE `creature_template` SET `HealthModifier` = 8.4  WHERE `entry` = 57798; -- Congealing Blood 10H (~697k)
UPDATE `creature_template` SET `HealthModifier` = 28   WHERE `entry` = 57980; -- Congealing Blood 25H (~2.32M)

-- ---------------------------------------------------------------------------
-- Elementium Bolt: TDB ships unlinked difficulty variants with broken level
-- data (level 1 / exp 0 / faction 35). Repair and link the chain; heroic
-- rows get the x 0.70 era retune (base 10N: 10 -> 830k, 25N: 30 -> 2.49M).
-- ---------------------------------------------------------------------------
UPDATE `creature_template` SET `minlevel` = 87, `maxlevel` = 87, `exp` = 3, `faction` = 14, `speed_run` = 7.1428, `RegenHealth` = 0 WHERE `entry` IN (57979, 58144, 58145);
UPDATE `creature_template` SET `HealthModifier` = 8.4  WHERE `entry` = 58144; -- 10H (~697k)
UPDATE `creature_template` SET `HealthModifier` = 25.2 WHERE `entry` = 58145; -- 25H (~2.09M)
UPDATE `creature_template` SET `difficulty_entry_1` = 57979, `difficulty_entry_2` = 58144, `difficulty_entry_3` = 58145 WHERE `entry` = 56262;

-- ---------------------------------------------------------------------------
-- Raid Finder stats templates (70% of 25N, applied by script)
-- ---------------------------------------------------------------------------
DELETE FROM `creature_template` WHERE `entry` BETWEEN 58257 AND 58264;
INSERT INTO `creature_template` (`entry`, `name`, `femaleName`, `subname`, `minlevel`, `maxlevel`, `exp`, `faction`, `unit_class`, `type`, `HealthModifier`, `AIName`, `ScriptName`, `VerifiedBuild`) VALUES
(58257, 'Deathwing', '', 'LFR Stats', 88, 88, 3, 14, 1, 4, 3108, '', '', 0),           -- 266,952,336 (body + head)
(58258, 'Arm Tentacle', '', 'LFR Stats', 88, 88, 3, 14, 1, 4, 682.5, '', '', 0),       -- ~58,621,288
(58259, 'Mutated Corruption', '', 'LFR Stats', 88, 88, 3, 14, 1, 4, 273, '', '', 0),   -- ~23,448,516
(58260, 'Regenerative Blood', '', 'LFR Stats', 87, 87, 3, 14, 4, 4, 16.8, '', '', 0),  -- ~1,394,300
(58261, 'Blistering Tentacle', '', 'LFR Stats', 87, 87, 3, 14, 1, 4, 4.2, '', '', 0),  -- ~348,575
(58262, 'Elementium Bolt', '', 'LFR Stats', 87, 87, 3, 14, 1, 4, 21, '', '', 0),       -- ~1,742,874
(58263, 'Elementium Fragment', '', 'LFR Stats', 87, 87, 3, 14, 1, 4, 9.8, '', '', 0),  -- ~813,342
(58264, 'Elementium Terror', '', 'LFR Stats', 87, 87, 3, 14, 1, 4, 73.5, '', '', 0);   -- ~6,100,059

DELETE FROM `creature_template_model` WHERE `CreatureID` BETWEEN 58257 AND 58264;
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `Probability`, `VerifiedBuild`) VALUES
(58257, 0, 39355, 1, 0),
(58258, 0, 39354, 1, 0),
(58259, 0, 39405, 1, 0),
(58260, 0, 40190, 1, 0),
(58261, 0, 39347, 1, 0),
(58262, 0, 39381, 1, 0),
(58263, 0, 39474, 1, 0),
(58264, 0, 39463, 1, 0);

-- ---------------------------------------------------------------------------
-- Script bindings for the head and the heroic adds
-- ---------------------------------------------------------------------------
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_madness_of_deathwing_deathwing_head' WHERE `entry` = 57962;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_madness_of_deathwing_corrupting_parasite' WHERE `entry` = 57479;
-- energy is script-driven; deny native power regeneration (UNIT_FLAG2_REGENERATE_POWER)
UPDATE `creature_template` SET `unit_flags2` = `unit_flags2` & ~2048 WHERE `entry` IN (57479, 58034);
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_madness_of_deathwing_congealing_blood' WHERE `entry` = 57798;
UPDATE `creature_template` SET `AIName` = 'PassiveAI', `ScriptName` = '' WHERE `entry` = 57480;

DELETE FROM `spell_script_names` WHERE `spell_id` IN (108597, 108649, 108813, 106860, 107029);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(108597, 'spell_madness_of_deathwing_corrupting_parasite'),
(108649, 'spell_madness_of_deathwing_corrupting_parasite_aura'),
(108813, 'spell_madness_of_deathwing_unstable_corruption'),
(106860, 'spell_madness_of_deathwing_cauterize_phase_two'),
(107029, 'spell_madness_of_deathwing_impale_aspect');

-- ---------------------------------------------------------------------------
-- Conditions: Time Zone slows the heroic parasites; Congealing Blood summon
-- destinations resolve to the Congealing Blood Target ring; the heal hits
-- both health-linked Deathwing units (share health mirrors damage only)
-- ---------------------------------------------------------------------------
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 105830 AND `ConditionValue2` IN (57479, 57480);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` IN (109089, 109102);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 7, 105830, 0, 2, 31, 0, 3, 57479, 0, 0, 0, 0, '', 'Time Zone - Target Corrupting Parasite'),
(13, 7, 105830, 0, 3, 31, 0, 3, 57480, 0, 0, 0, 0, '', 'Time Zone - Target Corrupting Parasite Tentacle'),
(13, 1, 109102, 0, 0, 31, 0, 3, 57962, 0, 0, 0, 0, '', 'Congealing Blood - Heal Deathwing (head)'),
(13, 2, 109102, 0, 0, 31, 0, 3, 56173, 0, 0, 0, 0, '', 'Congealing Blood - Heal Deathwing (body)');

-- ---------------------------------------------------------------------------
-- Chromatic Champion (6180): the client criteria 18658-18661 are gated on
-- per-map worldstates ("<Aspect> Assaulted First"). Without template rows,
-- SetValue would fall back to realm-global state shared between instances.
-- ---------------------------------------------------------------------------
DELETE FROM `world_state` WHERE `ID` IN (6258, 6259, 6260, 6261);
INSERT INTO `world_state` (`ID`, `DefaultValue`, `MapIDs`, `AreaIDs`, `ScriptName`, `Comment`) VALUES
(6258, 0, '967', NULL, '', 'Dragon Soul - Madness of Deathwing - Alexstrasza Assaulted First (Chromatic Champion)'),
(6259, 0, '967', NULL, '', 'Dragon Soul - Madness of Deathwing - Nozdormu Assaulted First (Chromatic Champion)'),
(6260, 0, '967', NULL, '', 'Dragon Soul - Madness of Deathwing - Ysera Assaulted First (Chromatic Champion)'),
(6261, 0, '967', NULL, '', 'Dragon Soul - Madness of Deathwing - Kalecgos Assaulted First (Chromatic Champion)');

-- ---------------------------------------------------------------------------
-- Mutated Corruption: raid emote while it tries to impale an unattended
-- Dragon Aspect (broadcast_text 56722, sniffed)
-- ---------------------------------------------------------------------------
DELETE FROM `creature_text` WHERE `CreatureID` = 56471;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `Comment`) VALUES
(56471, 0, 0, '%s begins to impale the Aspect! Stop it!', 41, 0, 100, 0, 0, 0, 56722, 3, 'Mutated Corruption - Announce Impale Aspect');
