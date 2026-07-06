-- Morchok (Dragon Soul) - encounter implementation
-- Boss 55265 (10N) with difficulty entries 57409 (25N) / 57771 (10H) / 57772 (25H)
-- Kohcrom 57773 with difficulty entries 57774 (25N, unused) / 57995 (10H) / 57996 (25H)

-- Script bindings (base entries only - the core ignores ScriptName on difficulty entries)
UPDATE `creature_template` SET `ScriptName` = 'boss_morchok' WHERE `entry` = 55265;
UPDATE `creature_template` SET `ScriptName` = 'npc_kohcrom' WHERE `entry` = 57773;
UPDATE `creature_template` SET `ScriptName` = 'npc_morchok_resonating_crystal' WHERE `entry` = 55346;

-- 10 Heroic health: 85,892 base * 175.0 = 15,031,100 (user-supplied retail value)
UPDATE `creature_template` SET `HealthModifier` = 175 WHERE `entry` IN (57771, 57995);

-- Kohcrom difficulty chain + stub template fixes (57774/57995/57996 were level-1 faction-35 WDB stubs)
UPDATE `creature_template` SET `difficulty_entry_1` = 57774, `difficulty_entry_2` = 57995, `difficulty_entry_3` = 57996 WHERE `entry` = 57773;
UPDATE `creature_template` SET `minlevel` = 88, `maxlevel` = 88, `exp` = 3, `faction` = 14, `unit_class` = 1 WHERE `entry` IN (57774, 57995, 57996);

-- Full boss CC immunity mask (fork convention, see Ragnaros)
UPDATE `creature_template` SET `mechanic_immune_mask` = 650854271 WHERE `entry` IN (55265, 57409, 57771, 57772, 57773, 57774, 57995, 57996, 55346);

-- Resonating Crystal: drop the invisible trigger model from the random pool,
-- immune to players/creatures (never attackable), no melee
UPDATE `creature_template` SET `modelid1` = 11686, `modelid2` = 0, `unit_flags` = 768 WHERE `entry` = 55346;

-- Texts (from retail sniffs; sounds from SoundEntries.dbc VO_DS_MORCHOK_*)
DELETE FROM `creature_text` WHERE `CreatureID` = 55265;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(55265, 0, 0, 'No mortal shall turn me from my task.', 14, 0, 100, 0, 0, 26282, 0, 0, 'Morchok - Intro 1'),
(55265, 1, 0, 'Wyrmrest will fall. All will be dust.', 14, 0, 100, 0, 0, 26273, 0, 0, 'Morchok - Intro 2'),
(55265, 2, 0, 'You seek to halt an avalanche. I will bury you.', 14, 0, 100, 0, 0, 26268, 0, 0, 'Morchok - Aggro'),
(55265, 3, 0, 'You thought to fight me alone? The earth splits to swallow and crush you.', 14, 0, 100, 0, 0, 26288, 0, 0, 'Morchok - Summon Kohcrom'),
(55265, 4, 0, '%s summons a Resonating Crystal!', 41, 0, 100, 0, 0, 0, 0, 0, 'Morchok - Announce Resonating Crystal'),
(55265, 5, 0, 'Flee, and die.', 14, 0, 100, 0, 0, 26283, 0, 0, 'Morchok - Resonating Crystal 1'),
(55265, 5, 1, 'Run, and perish.', 14, 0, 100, 0, 0, 26284, 0, 0, 'Morchok - Resonating Crystal 2'),
(55265, 6, 0, 'The rocks tremble...', 14, 0, 100, 0, 0, 26276, 0, 0, 'Morchok - Black Blood Omen A'),
(55265, 7, 0, '...and the rage of the true gods follows.', 14, 0, 100, 0, 0, 26280, 0, 0, 'Morchok - Black Blood A'),
(55265, 8, 0, 'The stone calls...', 14, 0, 100, 0, 0, 26274, 0, 0, 'Morchok - Black Blood Omen B'),
(55265, 9, 0, '...and the black blood of the earth consumes you.', 14, 0, 100, 0, 0, 26278, 0, 0, 'Morchok - Black Blood B'),
(55265, 10, 0, '$n! Get out of the black ooze on the ground!', 42, 0, 100, 0, 0, 0, 0, 0, 'Morchok - Black Blood Whisper'),
(55265, 11, 0, 'Impossible. This cannot be. The tower... must... fall...', 14, 0, 100, 0, 0, 26269, 0, 0, 'Morchok - Death');

-- Don't Stand So Close to Me (achievement 6174, criteria 18607) -> instance script check
DELETE FROM `achievement_criteria_data` WHERE `criteria_id` = 18607 AND `type` = 18;
INSERT INTO `achievement_criteria_data` (`criteria_id`, `type`, `value1`, `value2`, `ScriptName`) VALUES
(18607, 18, 0, 0, '');

-- Spell script bindings (base spell + SpellDifficulty.dbc forks)
DELETE FROM `spell_script_names` WHERE `ScriptName` IN
('spell_morchok_stomp', 'spell_morchok_resonating_crystal_detonate', 'spell_morchok_earthen_vortex',
 'spell_morchok_black_blood_damage', 'aura_morchok_falling_fragments');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(103414, 'spell_morchok_stomp'),
(108571, 'spell_morchok_stomp'),
(109033, 'spell_morchok_stomp'),
(109034, 'spell_morchok_stomp'),
(103545, 'spell_morchok_resonating_crystal_detonate'),
(108572, 'spell_morchok_resonating_crystal_detonate'),
(110041, 'spell_morchok_resonating_crystal_detonate'),
(110040, 'spell_morchok_resonating_crystal_detonate'),
(103821, 'spell_morchok_earthen_vortex'),
(110047, 'spell_morchok_earthen_vortex'),
(110046, 'spell_morchok_earthen_vortex'),
(110045, 'spell_morchok_earthen_vortex'),
(103785, 'spell_morchok_black_blood_damage'),
(108570, 'spell_morchok_black_blood_damage'),
(110288, 'spell_morchok_black_blood_damage'),
(110287, 'spell_morchok_black_blood_damage'),
(103176, 'aura_morchok_falling_fragments');

-- ---------------------------------------------------------------------------
-- Loot: split the mixed WDB table on 55265 into per-difficulty tables.
--   55265 (10N):  ilvl 397 + shared
--   57409 (25N):  ilvl 397 (LootMode 1), ilvl 384 LFR (LootMode 2), shared (LootMode 3)
--   57771 (10H):  ilvl 410 + shared
--   57772 (25H):  ilvl 410 + shared
-- Item classes resolved against Item-sparse.db2 (4.3.4 client).
-- ---------------------------------------------------------------------------
SET @NORMAL_ITEMS := '77207,77208,77209,77210,77211,77212,77213,77214,77228,77229,77230,77231,77232,77261,77262,77263,77265,77266,77267,77268,77269,77270,77271';
SET @LFR_ITEMS    := '77979,77980,77981,77982,77983,78375,78376,78377,78378,78380,78381,78382,78384,78385,78386,78494,78495,78496,78497,78498,78862,78863,78864,78865,78866,78867,78868,78869,78870,78871,78872,78873,78874,78875,78876';
SET @HEROIC_ITEMS := '77999,78000,78001,78002,78003,78361,78362,78363,78364,78365,78366,78367,78368,78369,78370,78371,78372,78373,78374,78489,78490,78491,78492,78493';
SET @SHARED_ITEMS := '71716,71998,77952';

DELETE FROM `creature_loot_template` WHERE `Entry` IN (57409, 57771, 57772);

-- 25 Normal: 397 loot
INSERT INTO `creature_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT 57409, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, 1, `GroupId`, `MinCount`, `MaxCount`, 'Morchok 25N'
FROM `creature_loot_template` WHERE `Entry` = 55265 AND FIND_IN_SET(`Item`, @NORMAL_ITEMS);

-- 25 Normal table: LFR loot as LootMode 2
INSERT INTO `creature_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT 57409, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, 2, `GroupId`, `MinCount`, `MaxCount`, 'Morchok LFR'
FROM `creature_loot_template` WHERE `Entry` = 55265 AND FIND_IN_SET(`Item`, @LFR_ITEMS);

-- 25 Normal table: shared drops available in both loot modes
INSERT INTO `creature_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT 57409, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, 3, `GroupId`, `MinCount`, `MaxCount`, 'Morchok shared'
FROM `creature_loot_template` WHERE `Entry` = 55265 AND FIND_IN_SET(`Item`, @SHARED_ITEMS);

-- Heroic tables
INSERT INTO `creature_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT 57771, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, 1, `GroupId`, `MinCount`, `MaxCount`, 'Morchok 10H'
FROM `creature_loot_template` WHERE `Entry` = 55265 AND (FIND_IN_SET(`Item`, @HEROIC_ITEMS) OR FIND_IN_SET(`Item`, @SHARED_ITEMS));

INSERT INTO `creature_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT 57772, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, 1, `GroupId`, `MinCount`, `MaxCount`, 'Morchok 25H'
FROM `creature_loot_template` WHERE `Entry` = 55265 AND (FIND_IN_SET(`Item`, @HEROIC_ITEMS) OR FIND_IN_SET(`Item`, @SHARED_ITEMS));

-- 10 Normal keeps only 397 + shared
DELETE FROM `creature_loot_template` WHERE `Entry` = 55265 AND (FIND_IN_SET(`Item`, @LFR_ITEMS) OR FIND_IN_SET(`Item`, @HEROIC_ITEMS));

UPDATE `creature_template` SET `lootid` = `entry` WHERE `entry` IN (57409, 57771, 57772);
