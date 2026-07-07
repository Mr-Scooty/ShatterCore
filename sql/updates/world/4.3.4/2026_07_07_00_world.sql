-- Yor'sahj the Unsleeping (Dragon Soul) - encounter implementation
-- Boss 55312 (10N) with difficulty entries 55313 (25N) / 55314 (10H) / 55315 (25H)
-- Globules (base 10N -> 25N / 10H / 25H):
--   Acidic   (green)  55862 -> 57359 / 57360 / 57361
--   Shadowed (purple) 55863 -> 57374 / 57375 / 57376
--   Glowing  (yellow) 55864 -> 57371 / 57372 / 57373
--   Crimson  (red)    55865 -> 57368 / 57369 / 57370
--   Cobalt   (blue)   55866 -> 57365 / 57366 / 57367
--   Dark     (black)  55867 -> 57362 / 57363 / 57364
-- Forgotten One 56265 -> 57434 / 57435 / 57436
-- Mana Void     56231 -> 57437 / 57438 / 57439
--
-- SpellDifficulty.dbc forks (client-side, no spelldifficulty_dbc rows needed):
--   104849 Void Bolt            -> 108383 / 108384 / 108385
--   105416 Void Bolt AoE        -> 109549 / 109550 / 109551
--   105033 Searing Blood        -> 108356 / 108357 / 108358 (LFR base 108218 -> 108363)
--   105573 Digestive Acid dmg   -> 108350 / 108351 / 108352 (LFR base 108419 -> 109543)
--   105173 Deep Corruption dmg  -> 108347 / 108348 / 108349
--   105671 Psychic Slice        -> 108353 / 108354 / 108355
--   105636 Corrupted Minions    -> 109558 (10H slot)

-- Script bindings (base entries only - the core ignores ScriptName on difficulty entries)
UPDATE `creature_template` SET `ScriptName` = 'boss_yorsahj' WHERE `entry` = 55312;
UPDATE `creature_template` SET `ScriptName` = 'npc_yorsahj_globule' WHERE `entry` IN (55862, 55863, 55864, 55865, 55866, 55867);
UPDATE `creature_template` SET `ScriptName` = 'npc_yorsahj_forgotten_one' WHERE `entry` = 56265;
UPDATE `creature_template` SET `ScriptName` = 'npc_yorsahj_mana_void' WHERE `entry` = 56231;

-- Boss difficulty chain + stub template fixes (55313/55314/55315 were level-1 faction-35 WDB stubs)
UPDATE `creature_template` SET `difficulty_entry_1` = 55313, `difficulty_entry_2` = 55314, `difficulty_entry_3` = 55315 WHERE `entry` = 55312;
UPDATE `creature_template` SET `minlevel` = 88, `maxlevel` = 88, `exp` = 3, `faction` = 14, `unit_class` = 1, `unit_flags2` = 134219776 WHERE `entry` IN (55313, 55314, 55315);

-- Health (user-supplied retail values, base health 85,892 at level 88 expansion 3):
--   10N 55312:  550.0 -> 47,240,600  (already correct)
--   25N 55313: 1650.0 -> 141,721,800 (already correct)
--   10H 55314:  729.4 -> 62,649,624  (was 1042)
--   25H 55315: 3010.0 -> 258,534,920 (already correct)
UPDATE `creature_template` SET `HealthModifier` = 729.4 WHERE `entry` = 55314;

-- Full boss CC immunity mask (fork convention); globules must resist stun/root/
-- snare/fear/taunt/etc., Mana Void keeps the grip bit free for Death Grip
UPDATE `creature_template` SET `mechanic_immune_mask` = 650854271 WHERE `entry` IN
(55312, 55313, 55314, 55315,
 55862, 55863, 55864, 55865, 55866, 55867,
 57359, 57360, 57361, 57362, 57363, 57364, 57365, 57366, 57367, 57368, 57369, 57370, 57371, 57372, 57373, 57374, 57375, 57376,
 56265, 57434, 57435, 57436,
 56231, 57437, 57438, 57439);

-- Mana Void shipped with CREATURE_FLAG_EXTRA_NO_COMBAT (8192) - it must be attackable
UPDATE `creature_template` SET `flags_extra` = `flags_extra` & ~8192 WHERE `entry` IN (56231, 57437, 57438, 57439);

-- Raid Finder stats templates (health only - community retail values, user-approved)
--   58227 boss:          85,892 (L88) * 1234 = 105,990,728 (~106.0M)
--   58228 globule:       82,994 (L87) *  49.4 =  4,099,903 (~4.1M)
--   58229 Mana Void:     77,490 (L85) *  49   =  3,797,010 (~3.8M)
--   58230 Forgotten One: 82,994 (L87) *  13.3 =  1,103,820 (~1.1M)
DELETE FROM `creature_template` WHERE `entry` IN (58227, 58228, 58229, 58230);
INSERT INTO `creature_template` (`entry`, `name`, `femaleName`, `subname`, `minlevel`, `maxlevel`, `exp`, `faction`, `unit_class`, `type`, `HealthModifier`, `AIName`, `ScriptName`, `VerifiedBuild`) VALUES
(58227, 'Yor''sahj the Unsleeping', '', 'LFR Stats', 88, 88, 3, 14, 1, 10, 1234, '', '', 0),
(58228, 'Yor''sahj Globule', '', 'LFR Stats', 87, 87, 3, 16, 1, 10, 49.4, '', '', 0),
(58229, 'Mana Void', '', 'LFR Stats', 85, 85, 3, 14, 1, 10, 49, '', '', 0),
(58230, 'Forgotten One', '', 'LFR Stats', 87, 87, 3, 16, 1, 10, 13.3, '', '', 0);

DELETE FROM `creature_template_model` WHERE `CreatureID` IN (58227, 58228, 58229, 58230);
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `Probability`, `VerifiedBuild`) VALUES
(58227, 0, 39101, 1, 0),
(58228, 0, 36110, 1, 0),
(58229, 0, 39369, 1, 0),
(58230, 0, 27849, 1, 0);

-- Spawns (map 967, all four raid spawn modes; positions from retail sniffs).
-- 9000634: the boss, at the room center where the globules converge.
-- 9000635: Maw of Shu'ma, the cosmetic mouth in the ceiling above the room.
DELETE FROM `creature` WHERE `guid` IN (9000634, 9000635);
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `VerifiedBuild`) VALUES
(9000634, 55312, 967, 0, 0, 15, 0, 0, 0, 0, -1765.65, -3034.35, -182.376, 3.5256, 604800, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(9000635, 55544, 967, 0, 0, 15, 0, 0, 0, 0, -1762.56, -3036.65, -116.44, 4.7124, 604800, 0, 0, 0, 0, 0, 0, 0, 0, 0);

-- Texts (broadcast_text 56802-56821; each Shath'Yar yell is paired with a
-- translated whisper sent to the raid). Sounds: VO_DS_YORSAHJ_* 26326-26334.
DELETE FROM `creature_text` WHERE `CreatureID` = 55312;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(55312, 0, 0, 'Ak''agthshi ma uhnish, ak''uq shg''cul vwahuhn! H''iwn iggksh Phquathi gag OOU KAAXTH SHUUL!', 14, 0, 100, 0, 0, 26328, 56802, 0, 'Yorsahj - Intro'),
(55312, 1, 0, 'Our numbers are endless, our power beyond reckoning! All who oppose the Destroyer will DIE A THOUSAND DEATHS!', 42, 0, 100, 0, 0, 0, 56803, 0, 'Yorsahj - Intro Whisper'),
(55312, 2, 0, 'Iilth qi''uothk shn''ma yeh''glu Shath''Yar! H''IWN IILTH!', 14, 0, 100, 0, 0, 26326, 56804, 0, 'Yorsahj - Aggro'),
(55312, 3, 0, 'You will drown in the blood of the Old Gods! ALL OF YOU!', 42, 0, 100, 0, 0, 0, 56805, 0, 'Yorsahj - Aggro Whisper'),
(55312, 4, 0, 'KYTH ag''xig yyg''far IIQAATH ONGG!', 14, 0, 100, 0, 0, 26332, 56816, 0, 'Yorsahj - Summon 1'),
(55312, 5, 0, 'SEE how we pour from the CURSED EARTH!', 42, 0, 100, 0, 0, 0, 56817, 0, 'Yorsahj - Summon 1 Whisper'),
(55312, 6, 0, 'UULL lwhuk H''IWN!', 14, 0, 100, 0, 0, 26333, 56818, 0, 'Yorsahj - Summon 2'),
(55312, 7, 0, 'The DARKNESS devours ALL!', 42, 0, 100, 0, 0, 0, 56819, 0, 'Yorsahj - Summon 2 Whisper'),
(55312, 8, 0, 'Awtgsshu LWHUK iilth!', 14, 0, 100, 0, 0, 26334, 56820, 0, 'Yorsahj - Summon 3'),
(55312, 9, 0, 'Corruption CONSUMES you!', 42, 0, 100, 0, 0, 0, 56821, 0, 'Yorsahj - Summon 3 Whisper'),
(55312, 10, 0, 'Sk''yahf qi''plahf PH''MAGG!', 14, 0, 100, 0, 0, 26329, 56809, 0, 'Yorsahj - Slay 1'),
(55312, 11, 0, 'Your soul will know ENDLESS TORMENT!', 42, 0, 100, 0, 0, 0, 56810, 0, 'Yorsahj - Slay 1 Whisper'),
(55312, 12, 0, 'H''iwn zaix Shuul''wah, PHQUATHI!', 14, 0, 100, 0, 0, 26330, 56811, 0, 'Yorsahj - Slay 2'),
(55312, 13, 0, 'All praise Deathwing, THE DESTROYER!', 42, 0, 100, 0, 0, 0, 56812, 0, 'Yorsahj - Slay 2 Whisper'),
(55312, 14, 0, 'Shkul an''zig qvsakf KSSH''GA, ag''THYZAK agthu!', 14, 0, 100, 0, 0, 26331, 56814, 0, 'Yorsahj - Slay 3'),
(55312, 15, 0, 'From its BLEAKEST DEPTHS, we RECLAIM this world!', 42, 0, 100, 0, 0, 0, 56815, 0, 'Yorsahj - Slay 3 Whisper'),
(55312, 16, 0, 'Ez, Shuul''wah! Sk''woth''gl yu''gaz yoh''ghyl iilth!', 14, 0, 100, 0, 0, 26327, 56806, 0, 'Yorsahj - Death'),
(55312, 17, 0, 'O, Deathwing! Your faithful servant has failed you!', 42, 0, 100, 0, 0, 0, 56808, 0, 'Yorsahj - Death Whisper');

-- Taste the Rainbow! (achievement 6129, criteria 18495-18498) -> instance script check
DELETE FROM `achievement_criteria_data` WHERE `criteria_id` IN (18495, 18496, 18497, 18498) AND `type` = 18;
INSERT INTO `achievement_criteria_data` (`criteria_id`, `type`, `value1`, `value2`, `ScriptName`) VALUES
(18495, 18, 0, 0, ''),  -- Black and Yellow
(18496, 18, 0, 0, ''),  -- Red and Green
(18497, 18, 0, 0, ''),  -- Black and Blue
(18498, 18, 0, 0, '');  -- Purple and Yellow

-- Deep Corruption (105171): route taken heals (direct + periodic) into the
-- proc-triggered stack counter 103628 (same proc shape as Baleroc's Torment)
DELETE FROM `spell_proc` WHERE `SpellId` = 105171;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(105171, 0, 0, 0, 0, 0, 0x88800, 0x2, 0x0, 0, 0, 0, 0, 100, 0, 0);

-- Spell script bindings (base spell + SpellDifficulty.dbc forks)
DELETE FROM `spell_script_names` WHERE `ScriptName` IN
('spell_yorsahj_color_combination', 'spell_yorsahj_deep_corruption', 'spell_yorsahj_deep_corruption_explosion',
 'spell_yorsahj_searing_blood', 'spell_yorsahj_digestive_acid', 'spell_yorsahj_fusing_vapors',
 'spell_yorsahj_mana_diffusion', 'spell_yorsahj_void_bolt_lfr');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(105420, 'spell_yorsahj_color_combination'),
(105435, 'spell_yorsahj_color_combination'),
(105436, 'spell_yorsahj_color_combination'),
(105437, 'spell_yorsahj_color_combination'),
(105439, 'spell_yorsahj_color_combination'),
(105440, 'spell_yorsahj_color_combination'),
(105171, 'spell_yorsahj_deep_corruption'),
(105173, 'spell_yorsahj_deep_corruption_explosion'),
(108347, 'spell_yorsahj_deep_corruption_explosion'),
(108348, 'spell_yorsahj_deep_corruption_explosion'),
(108349, 'spell_yorsahj_deep_corruption_explosion'),
(105033, 'spell_yorsahj_searing_blood'),
(108356, 'spell_yorsahj_searing_blood'),
(108357, 'spell_yorsahj_searing_blood'),
(108358, 'spell_yorsahj_searing_blood'),
(108218, 'spell_yorsahj_searing_blood'),
(108363, 'spell_yorsahj_searing_blood'),
(105573, 'spell_yorsahj_digestive_acid'),
(108350, 'spell_yorsahj_digestive_acid'),
(108351, 'spell_yorsahj_digestive_acid'),
(108352, 'spell_yorsahj_digestive_acid'),
(103635, 'spell_yorsahj_fusing_vapors'),
(108233, 'spell_yorsahj_fusing_vapors'),
(105539, 'spell_yorsahj_mana_diffusion'),
(104849, 'spell_yorsahj_void_bolt_lfr'),
(108383, 'spell_yorsahj_void_bolt_lfr'),
(108384, 'spell_yorsahj_void_bolt_lfr'),
(108385, 'spell_yorsahj_void_bolt_lfr'),
(105416, 'spell_yorsahj_void_bolt_lfr'),
(109549, 'spell_yorsahj_void_bolt_lfr'),
(109550, 'spell_yorsahj_void_bolt_lfr'),
(109551, 'spell_yorsahj_void_bolt_lfr');

-- ---------------------------------------------------------------------------
-- Loot: split the mixed WDB table on 55312 into per-difficulty tables.
--   55312 (10N):  ilvl 397 + shared
--   55313 (25N):  ilvl 397 (LootMode 1), ilvl 384 LFR (LootMode 2), shared (LootMode 3)
--   55314 (10H):  ilvl 410 + shared
--   55315 (25H):  ilvl 410 + shared
-- Item classes resolved against Item-sparse.db2 (4.3.4 client).
-- The base table is rebuilt from the original TDB 434.22011 rows first so
-- this update stays idempotent.
-- ---------------------------------------------------------------------------
DELETE FROM `creature_loot_template` WHERE `Entry` IN (55312, 55313, 55314, 55315);
INSERT INTO `creature_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`) VALUES
(55312,71998,0,72.7962,0,0,1,0,1,3,NULL),
(55312,77203,0,19.4367,0,0,1,0,1,1,NULL),
(55312,77206,0,14.4124,0,0,1,0,1,1,NULL),
(55312,77207,0,0.8414,0,0,1,0,1,1,NULL),
(55312,77208,0,0.9245,0,0,1,0,1,1,NULL),
(55312,77209,0,0.9079,0,0,1,0,1,1,NULL),
(55312,77210,0,0.9935,0,0,1,0,1,1,NULL),
(55312,77211,0,1.098,0,0,1,0,1,1,NULL),
(55312,77217,0,15.6506,0,0,1,0,1,1,NULL),
(55312,77218,0,13.1099,0,0,1,0,1,1,NULL),
(55312,77219,0,18.0677,0,0,1,0,1,1,NULL),
(55312,77228,0,1.0648,0,0,1,0,1,1,NULL),
(55312,77229,0,0.839,0,0,1,0,1,1,NULL),
(55312,77230,0,0.9673,0,0,1,0,1,1,NULL),
(55312,77231,0,0.839,0,0,1,0,1,1,NULL),
(55312,77232,0,1.022,0,0,1,0,1,1,NULL),
(55312,77252,0,10.0012,0,0,1,0,1,1,NULL),
(55312,77253,0,9.5853,0,0,1,0,1,1,NULL),
(55312,77254,0,13.407,0,0,1,0,1,1,NULL),
(55312,77952,0,82.7285,0,0,1,0,1,3,NULL),
(55312,77970,0,19.0446,0,0,1,0,1,1,NULL),
(55312,77971,0,30.4504,0,0,1,0,1,1,NULL),
(55312,77979,0,1.4308,0,0,1,0,1,1,NULL),
(55312,77980,0,1.4759,0,0,1,0,1,1,NULL),
(55312,77981,0,1.369,0,0,1,0,1,1,NULL),
(55312,77982,0,1.2977,0,0,1,0,1,1,NULL),
(55312,77983,0,1.514,0,0,1,0,1,1,NULL),
(55312,77990,0,5.2882,0,0,1,0,1,1,NULL),
(55312,77991,0,7.3036,0,0,1,0,1,1,NULL),
(55312,77999,0,0.2543,0,0,1,0,1,1,NULL),
(55312,78000,0,0.2757,0,0,1,0,1,1,NULL),
(55312,78001,0,0.2187,0,0,1,0,1,1,NULL),
(55312,78002,0,0.2044,0,0,1,0,1,1,NULL),
(55312,78003,0,0.2733,0,0,1,0,1,1,NULL),
(55312,78171,0,42.9851,0,0,1,0,1,1,NULL),
(55312,78176,0,34.7427,0,0,1,0,1,1,NULL),
(55312,78181,0,35.4153,0,0,1,0,1,1,NULL),
(55312,78401,0,4.9816,0,0,1,0,1,1,NULL),
(55312,78402,0,4.7629,0,0,1,0,1,1,NULL),
(55312,78403,0,5.8063,0,0,1,0,1,1,NULL),
(55312,78404,0,6.738,0,0,1,0,1,1,NULL),
(55312,78405,0,3.7623,0,0,1,0,1,1,NULL),
(55312,78406,0,3.7956,0,0,1,0,1,1,NULL),
(55312,78408,0,20.9887,0,0,1,0,1,1,NULL),
(55312,78411,0,15.3084,0,0,1,0,1,1,NULL),
(55312,78412,0,14.5787,0,0,1,0,1,1,NULL),
(55312,78489,0,0.202,0,0,1,0,1,1,NULL),
(55312,78490,0,0.3018,0,0,1,0,1,1,NULL),
(55312,78491,0,0.2044,0,0,1,0,1,1,NULL),
(55312,78492,0,0.221,0,0,1,0,1,1,NULL),
(55312,78493,0,0.2044,0,0,1,0,1,1,NULL),
(55312,78494,0,1.1717,0,0,1,0,1,1,NULL),
(55312,78495,0,1.2454,0,0,1,0,1,1,NULL),
(55312,78496,0,1.3333,0,0,1,0,1,1,NULL),
(55312,78497,0,1.6067,0,0,1,0,1,1,NULL),
(55312,78498,0,1.388,0,0,1,0,1,1,NULL),
(55312,78856,0,11.1777,0,0,1,0,1,1,NULL),
(55312,78857,0,11.1444,0,0,1,0,1,1,NULL),
(55312,78858,0,13.5258,0,0,1,0,1,1,NULL),
(55312,78862,0,6.0891,0,0,1,0,1,1,NULL),
(55312,78863,0,5.3428,0,0,1,0,1,1,NULL),
(55312,78864,0,4.3779,0,0,1,0,1,1,NULL),
(55312,78865,0,5.6684,0,0,1,0,1,1,NULL),
(55312,78866,0,4.0166,0,0,1,0,1,1,NULL),
(55312,78867,0,4.6322,0,0,1,0,1,1,NULL),
(55312,78868,0,5.9323,0,0,1,0,1,1,NULL),
(55312,78869,0,4.5799,0,0,1,0,1,1,NULL),
(55312,78870,0,4.2852,0,0,1,0,1,1,NULL),
(55312,78871,0,63.0541,0,0,1,0,1,1,NULL),
(55312,78872,0,55.5817,0,0,1,0,1,1,NULL),
(55312,78873,0,48.7677,0,0,1,0,1,1,NULL),
(55312,78874,0,6.7998,0,0,1,0,1,1,NULL),
(55312,78875,0,5.1622,0,0,1,0,1,1,NULL),
(55312,78876,0,4.6227,0,0,1,0,1,1,NULL);

SET @NORMAL_ITEMS := '77203,77206,77207,77208,77209,77210,77211,77217,77218,77219,77228,77229,77230,77231,77232,77252,77253,77254,78171,78176,78181';
SET @LFR_ITEMS    := '77970,77971,77979,77980,77981,77982,77983,78408,78411,78412,78494,78495,78496,78497,78498,78862,78863,78864,78865,78866,78867,78868,78869,78870,78871,78872,78873,78874,78875,78876';
SET @HEROIC_ITEMS := '77990,77991,77999,78000,78001,78002,78003,78401,78402,78403,78404,78405,78406,78489,78490,78491,78492,78493,78856,78857,78858';
SET @SHARED_ITEMS := '71998,77952';

-- 25 Normal: 397 loot
INSERT INTO `creature_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT 55313, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, 1, `GroupId`, `MinCount`, `MaxCount`, 'Yorsahj 25N'
FROM `creature_loot_template` WHERE `Entry` = 55312 AND FIND_IN_SET(`Item`, @NORMAL_ITEMS);

-- 25 Normal table: LFR loot as LootMode 2
INSERT INTO `creature_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT 55313, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, 2, `GroupId`, `MinCount`, `MaxCount`, 'Yorsahj LFR'
FROM `creature_loot_template` WHERE `Entry` = 55312 AND FIND_IN_SET(`Item`, @LFR_ITEMS);

-- 25 Normal table: shared drops available in both loot modes
INSERT INTO `creature_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT 55313, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, 3, `GroupId`, `MinCount`, `MaxCount`, 'Yorsahj shared'
FROM `creature_loot_template` WHERE `Entry` = 55312 AND FIND_IN_SET(`Item`, @SHARED_ITEMS);

-- Heroic tables
INSERT INTO `creature_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT 55314, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, 1, `GroupId`, `MinCount`, `MaxCount`, 'Yorsahj 10H'
FROM `creature_loot_template` WHERE `Entry` = 55312 AND (FIND_IN_SET(`Item`, @HEROIC_ITEMS) OR FIND_IN_SET(`Item`, @SHARED_ITEMS));

INSERT INTO `creature_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT 55315, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, 1, `GroupId`, `MinCount`, `MaxCount`, 'Yorsahj 25H'
FROM `creature_loot_template` WHERE `Entry` = 55312 AND (FIND_IN_SET(`Item`, @HEROIC_ITEMS) OR FIND_IN_SET(`Item`, @SHARED_ITEMS));

-- 10 Normal keeps only 397 + shared
DELETE FROM `creature_loot_template` WHERE `Entry` = 55312 AND (FIND_IN_SET(`Item`, @LFR_ITEMS) OR FIND_IN_SET(`Item`, @HEROIC_ITEMS));

UPDATE `creature_template` SET `lootid` = `entry` WHERE `entry` IN (55313, 55314, 55315);
