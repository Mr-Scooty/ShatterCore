-- Alysrazor (Firelands) encounter completion: 10/25 Normal + 10/25 Heroic
--
-- Boss HP (retail, user-provided): 51,535,200 / 154,605,600 / 77,302,800 / 231,908,400
-- basehp3: level 88 class 2 = 85892 -> HealthModifier 600 / 1800 / 900 / 2700 (exact)
UPDATE `creature_template` SET `HealthModifier` = 600  WHERE `entry` = 52530; -- 10N (was 450)
UPDATE `creature_template` SET `HealthModifier` = 1800 WHERE `entry` = 54044; -- 25N (was 1350)
UPDATE `creature_template` SET `HealthModifier` = 900  WHERE `entry` = 54045; -- 10H (was 765)
UPDATE `creature_template` SET `HealthModifier` = 2700 WHERE `entry` = 54046; -- 25H (was 2295)

-- Add HP (community-approximate retail targets; tune during walkthrough).
-- basehp3: level 87 class 1 = 82994, level 86 class 1 = 80195.
-- Voracious Hatchling ~1.4M / 4.2M / 1.8M / 5.5M -> 17 / 50 / 22 / 66
UPDATE `creature_template` SET `HealthModifier` = 17 WHERE `entry` IN (53509, 53898);
UPDATE `creature_template` SET `HealthModifier` = 50 WHERE `entry` IN (54052, 54060);
UPDATE `creature_template` SET `HealthModifier` = 22 WHERE `entry` IN (54053, 54061);
UPDATE `creature_template` SET `HealthModifier` = 66 WHERE `entry` IN (54054, 54062);
-- Blazing Talon Initiate ~775K / 2.3M / 1.0M / 3.0M -> 9.6 / 29 / 12.5 / 37
UPDATE `creature_template` SET `HealthModifier` = 9.6  WHERE `entry` IN (53896, 53369);
UPDATE `creature_template` SET `HealthModifier` = 29   WHERE `entry` IN (54063, 54047);
UPDATE `creature_template` SET `HealthModifier` = 12.5 WHERE `entry` IN (54064, 54048);
UPDATE `creature_template` SET `HealthModifier` = 37   WHERE `entry` IN (54065, 54049);
-- Blazing Talon Clawshaper ~700K / 2.1M / 900K / 2.7M -> 8.7 / 26 / 11.2 / 34
UPDATE `creature_template` SET `HealthModifier` = 8.7 WHERE `entry` = 53734;
UPDATE `creature_template` SET `HealthModifier` = 26  WHERE `entry` = 54055;
-- Plump Lava Worm (hatchling food, generous pool) -> 8.4 / 25 / 11 / 31
UPDATE `creature_template` SET `HealthModifier` = 8.4 WHERE `entry` = 53520;
-- Herald of the Burning End (heroic only) ~1.5M (10H) / 4.5M (25H) -> 18 / 54
UPDATE `creature_template` SET `HealthModifier` = 18 WHERE `entry` = 53375;

-- Difficulty clone rows for adds that lack them (pattern from Beth'tilac file;
-- entries continue the 9900xx block, 990009+ verified free).
DROP TEMPORARY TABLE IF EXISTS `alysrazor_clone`;
CREATE TEMPORARY TABLE `alysrazor_clone` AS SELECT * FROM `creature_template` WHERE `entry` = 53734;
UPDATE `alysrazor_clone` SET `difficulty_entry_1` = 0, `difficulty_entry_2` = 0, `difficulty_entry_3` = 0, `ScriptName` = '';
UPDATE `alysrazor_clone` SET `entry` = 990009, `HealthModifier` = 11.2; INSERT INTO `creature_template` SELECT * FROM `alysrazor_clone`; -- Clawshaper 10H
UPDATE `alysrazor_clone` SET `entry` = 990010, `HealthModifier` = 34;   INSERT INTO `creature_template` SELECT * FROM `alysrazor_clone`; -- Clawshaper 25H
DROP TEMPORARY TABLE `alysrazor_clone`;

CREATE TEMPORARY TABLE `alysrazor_clone` AS SELECT * FROM `creature_template` WHERE `entry` = 53375;
UPDATE `alysrazor_clone` SET `difficulty_entry_1` = 0, `difficulty_entry_2` = 0, `difficulty_entry_3` = 0, `ScriptName` = '';
UPDATE `alysrazor_clone` SET `entry` = 990011, `HealthModifier` = 18; INSERT INTO `creature_template` SELECT * FROM `alysrazor_clone`; -- Herald 10H
UPDATE `alysrazor_clone` SET `entry` = 990012, `HealthModifier` = 54; INSERT INTO `creature_template` SELECT * FROM `alysrazor_clone`; -- Herald 25H
DROP TEMPORARY TABLE `alysrazor_clone`;

CREATE TEMPORARY TABLE `alysrazor_clone` AS SELECT * FROM `creature_template` WHERE `entry` = 53520;
UPDATE `alysrazor_clone` SET `difficulty_entry_1` = 0, `difficulty_entry_2` = 0, `difficulty_entry_3` = 0, `ScriptName` = '';
UPDATE `alysrazor_clone` SET `entry` = 990013, `HealthModifier` = 25; INSERT INTO `creature_template` SELECT * FROM `alysrazor_clone`; -- Worm 25N
UPDATE `alysrazor_clone` SET `entry` = 990014, `HealthModifier` = 11; INSERT INTO `creature_template` SELECT * FROM `alysrazor_clone`; -- Worm 10H
UPDATE `alysrazor_clone` SET `entry` = 990015, `HealthModifier` = 31; INSERT INTO `creature_template` SELECT * FROM `alysrazor_clone`; -- Worm 25H
DROP TEMPORARY TABLE `alysrazor_clone`;

UPDATE `creature_template` SET `difficulty_entry_2` = 990009, `difficulty_entry_3` = 990010 WHERE `entry` = 53734;
UPDATE `creature_template` SET `difficulty_entry_2` = 990011, `difficulty_entry_3` = 990012 WHERE `entry` = 53375;
UPDATE `creature_template` SET `difficulty_entry_1` = 990013, `difficulty_entry_2` = 990014, `difficulty_entry_3` = 990015 WHERE `entry` = 53520;

-- Script bindings (base entries only; difficulty clones route through base)
UPDATE `creature_template` SET `ScriptName` = 'boss_alysrazor'                          WHERE `entry` = 52530;
UPDATE `creature_template` SET `ScriptName` = 'npc_alysrazor_majordomo_staghelm_intro'  WHERE `entry` = 54015;
UPDATE `creature_template` SET `ScriptName` = 'npc_alysrazor_voracious_hatchling'       WHERE `entry` IN (53509, 53898);
UPDATE `creature_template` SET `ScriptName` = 'npc_alysrazor_blazing_talon_initiate'    WHERE `entry` IN (53896, 53369);
UPDATE `creature_template` SET `ScriptName` = 'npc_alysrazor_blazing_broodmother'       WHERE `entry` IN (53680, 53900);
UPDATE `creature_template` SET `ScriptName` = 'npc_alysrazor_molten_egg'                WHERE `entry` IN (53681, 53899);
UPDATE `creature_template` SET `ScriptName` = 'npc_alysrazor_plump_lava_worm'           WHERE `entry` = 53520;
UPDATE `creature_template` SET `ScriptName` = 'npc_alysrazor_blazing_talon_clawshaper'  WHERE `entry` = 53734;
UPDATE `creature_template` SET `ScriptName` = 'npc_alysrazor_incendiary_cloud'          WHERE `entry` = 53541;
UPDATE `creature_template` SET `ScriptName` = 'npc_alysrazor_blazing_power'             WHERE `entry` = 53554;
UPDATE `creature_template` SET `ScriptName` = 'npc_alysrazor_fiery_vortex'              WHERE `entry` = 53693;
UPDATE `creature_template` SET `ScriptName` = 'npc_alysrazor_fiery_tornado'             WHERE `entry` = 53698;
UPDATE `creature_template` SET `ScriptName` = 'npc_alysrazor_brushfire'                 WHERE `entry` = 53372;
UPDATE `creature_template` SET `ScriptName` = 'npc_alysrazor_molten_meteor'             WHERE `entry` = 53784;
UPDATE `creature_template` SET `ScriptName` = 'npc_alysrazor_meteor_caller'             WHERE `entry` = 53487;
UPDATE `creature_template` SET `ScriptName` = 'npc_alysrazor_herald_of_the_burning_end' WHERE `entry` = 53375;

-- Trigger flags (CREATURE_FLAG_EXTRA_TRIGGER = 0x80) for pure hazard triggers
UPDATE `creature_template` SET `flags_extra` = `flags_extra` | 0x80 WHERE `entry` IN (53541, 53554, 53693, 53521);
-- Hatchlings cannot be taunted off their Imprinted target
UPDATE `creature_template` SET `flags_extra` = `flags_extra` | 0x100 WHERE `entry` IN (53509, 53898, 54052, 54053, 54054, 54060, 54061, 54062);
-- Clawshaper channel is interruptible by death only (stun/silence/interrupt/CC immune)
UPDATE `creature_template` SET `mechanic_immune_mask` = 617299803 WHERE `entry` IN (53734, 54055, 990009, 990010);

-- Spell script bindings
DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
'spell_alysrazor_molten_feather_pickup', 'spell_alysrazor_wings_of_flame', 'spell_alysrazor_blazing_power',
'spell_alysrazor_fieroblast_encounter', 'spell_alysrazor_brushfire_damage', 'spell_alysrazor_gushing_wound',
'spell_alysrazor_ignition', 'spell_alysrazor_firestorm_damage', 'spell_alysrazor_blazing_claw',
'spell_alysrazor_harsh_winds');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(97128,  'spell_alysrazor_molten_feather_pickup'),
(98624,  'spell_alysrazor_wings_of_flame'),
(99461,  'spell_alysrazor_blazing_power'),
(101223, 'spell_alysrazor_fieroblast_encounter'),
(98885,  'spell_alysrazor_brushfire_damage'),
(99308,  'spell_alysrazor_gushing_wound'),
(99919,  'spell_alysrazor_ignition'),
(100745, 'spell_alysrazor_firestorm_damage'), -- Firestorm 100744 triggered raid tick [DBC]
(100761, 'spell_alysrazor_cataclysm'),
(100640, 'spell_alysrazor_harsh_winds');

-- Feather pickup: the spellclick must cast 97128 (grant chain -> 98734
-- energize +1 feather power); the shipped 99933 row is the REMOVAL spell
-- ("Molten Feather Aura Multi-Cancel") and is wrong for clicks [DBC/sniff].
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 53089;
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(53089, 97128, 1, 0);

-- Do a Barrel Roll! (achievement 5813, criteria are generic boss-kill records;
-- gate via SCRIPT criteria data reading the boss AI flag)
DELETE FROM `achievement_criteria_data` WHERE `criteria_id` IN (17533, 17535, 17536, 17538) AND `type` = 11;
INSERT INTO `achievement_criteria_data` (`criteria_id`, `type`, `value1`, `value2`, `ScriptName`) VALUES
(17533, 11, 0, 0, 'achievement_alysrazor_barrel_roll'),
(17535, 11, 0, 0, 'achievement_alysrazor_barrel_roll'),
(17536, 11, 0, 0, 'achievement_alysrazor_barrel_roll'),
(17538, 11, 0, 0, 'achievement_alysrazor_barrel_roll');

-- Encounter texts (BroadcastTextId from local broadcast_text; type 14 = yell, 41 = boss emote)
DELETE FROM `creature_text` WHERE `CreatureID` IN (52530, 53896, 53369, 53375, 53520, 53680, 53900, 53681, 53899, 54015);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(52530, 0,  0, 'I serve a new master now, mortals!', 14, 0, 100, 0, 0, 24426, 52319, 3, 'Alysrazor - Aggro'),
(52530, 1,  0, 'These skies are MINE!', 14, 0, 100, 0, 0, 24434, 52322, 3, 'Alysrazor - Stage 2'),
(52530, 2,  0, 'Alysrazor begins to fly in a rapid circle!  The harsh winds will remove Wings of Flame!', 41, 0, 100, 0, 0, 0, 52314, 3, 'Alysrazor - Emote Fly Circle'),
(52530, 3,  0, '|TInterface\\Icons\\ability_mage_firestarter.blp:20|t The harsh winds form a |cFFFF0000|Hspell:99794|h[Fiery Vortex]|h|r!', 41, 0, 100, 0, 0, 0, 52315, 3, 'Alysrazor - Emote Fiery Vortex'),
(52530, 4,  0, '|TInterface\\Icons\\spell_holiday_tow_spicecloud.blp:20|t Alysrazor''s fire |cFFFF0000|Hspell:99432|h[Burns Out]|h|r!', 41, 0, 100, 0, 0, 0, 52316, 3, 'Alysrazor - Emote Burnout'),
(52530, 5,  0, 'The light...mustn''t burn out...', 14, 0, 100, 0, 0, 24429, 52326, 3, 'Alysrazor - Burnout'),
(52530, 6,  0, '|TInterface\\Icons\\inv_elemental_primal_fire.blp:20|t Alysrazor''s firey core |cFFFF0000|Hspell:99922|h[Re-Ignites]|h|r!', 41, 0, 100, 0, 0, 0, 52317, 3, 'Alysrazor - Emote Ignited'),
(52530, 7,  0, '|TInterface\\Icons\\spell_shaman_improvedfirenova.blp:20|t Alysrazor is at |cFFFF0000|Hspell:99925|h[Full Power]|h|r!', 41, 0, 100, 0, 0, 0, 52318, 3, 'Alysrazor - Emote Full Power'),
(52530, 8,  0, 'Reborn in flame!', 14, 0, 100, 0, 0, 24437, 52325, 3, 'Alysrazor - Full Power'),
(52530, 9,  0, 'BURN!', 14, 0, 100, 0, 0, 24430, 52320, 3, 'Alysrazor - Firestorm'),
(52530, 10, 0, 'Fire...fire...', 14, 0, 100, 0, 0, 24436, 52324, 3, 'Alysrazor - Death'),
(52530, 11, 0, 'For his glory!', 14, 0, 100, 0, 0, 24431, 52321, 3, 'Alysrazor - Kill 1'),
(52530, 11, 1, 'I will burn you from the sky!', 14, 0, 100, 0, 0, 24435, 52323, 3, 'Alysrazor - Kill 2'),
(53896, 0,  0, 'We call upon you, Firelord!', 14, 0, 100, 0, 0, 24797, 52332, 3, 'Blazing Talon Initiate - Spawn'),
(53369, 0,  0, 'We call upon you, Firelord!', 14, 0, 100, 0, 0, 24808, 52328, 3, 'Blazing Talon Initiate - Spawn'),
(53375, 0,  0, 'None escape the rage of the Firelands!', 14, 0, 100, 0, 0, 24813, 52331, 3, 'Herald of the Burning End - Spawn'),
(53520, 0,  0, 'Fiery Lava Worms erupt from the ground!', 41, 0, 100, 0, 0, 0, 52586, 3, 'Plump Lava Worm - Emerge'),
(53680, 0,  0, 'The Molten Eggs begin to hatch!', 41, 0, 100, 0, 0, 0, 52313, 3, 'Blazing Broodmother - Eggs'),
(53900, 0,  0, 'The Molten Eggs begin to hatch!', 41, 0, 100, 0, 0, 0, 52313, 3, 'Blazing Broodmother - Eggs'),
(53681, 0,  0, 'The Molten Eggs begin to crack and splinter!', 41, 0, 100, 0, 0, 0, 52591, 3, 'Molten Egg - Crack'),
(53899, 0,  0, 'The Molten Eggs begin to crack and splinter!', 41, 0, 100, 0, 0, 0, 52591, 3, 'Molten Egg - Crack'),
(54015, 0,  0, 'What have we here - visitors to our kingdom in the Firelands?', 14, 0, 100, 1, 0, 24466, 52658, 3, 'Majordomo Staghelm - Alysrazor Intro 1'),
(54015, 1,  0, 'You mortals may remember Alysra, who spirited me to freedom in Mount Hyjal. She, too has been reborn. Born of flame!', 14, 0, 100, 1, 0, 24467, 52659, 3, 'Majordomo Staghelm - Alysrazor Intro 2'),
(54015, 2,  0, 'I wish I could watch her reduce your pitiful band to cinders, but I am needed elsewhere. Farewell!', 14, 0, 100, 1, 0, 24468, 52660, 3, 'Majordomo Staghelm - Alysrazor Intro 3');
