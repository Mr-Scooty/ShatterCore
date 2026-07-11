-- Morchok: 4.3.4 health, heroic twin mechanics support, texts and deterministic loot repair

-- HealthModifier is a FLOAT and therefore only approximates some odd retail
-- values. boss_morchok sets the exact integer max health at runtime. Heroic
-- Morchok begins with the combined pool (42,946,000 / 180,404,194), then both
-- max and current health are halved when Kohcrom appears (21,473,000 / 90,202,097).
-- The 15,031,100 Classic 10H NPC value is exactly 70% of the unnerfed half.
UPDATE `creature_template` SET
    `HealthModifier` = CASE
        WHEN `entry` IN (55265, 57773) THEN 419.133295301
        WHEN `entry` IN (57409, 57774) THEN 1187.537838215
        WHEN `entry` = 57771 THEN 500.000000000
        WHEN `entry` = 57772 THEN 2100.360848507
        WHEN `entry` = 57995 THEN 250.000000000
        WHEN `entry` = 57996 THEN 1050.180424254
    END,
    `HealthModifierExtra` = 1,
    `unit_class` = 2,
    `StaticFlags` = `StaticFlags` & 4294967287
WHERE `entry` IN (55265, 57409, 57771, 57772, 57773, 57774, 57995, 57996);

-- Complete Kohcrom's difficulty chain; the difficulty entries were WDB stubs.
UPDATE `creature_template` SET
    `difficulty_entry_1` = 57774,
    `difficulty_entry_2` = 57995,
    `difficulty_entry_3` = 57996,
    `ScriptName` = 'npc_kohcrom'
WHERE `entry` = 57773;

UPDATE `creature_template` SET
    `minlevel` = 88,
    `maxlevel` = 88,
    `exp` = 3,
    `faction` = 14,
    `unit_class` = 2,
    `mechanic_immune_mask` = 650854271
WHERE `entry` IN (57774, 57995, 57996);

UPDATE `creature_template` SET `ScriptName` = 'boss_morchok' WHERE `entry` = 55265;
UPDATE `creature_template` SET `ScriptName` = 'npc_morchok_resonating_crystal' WHERE `entry` = 55346;
UPDATE `creature_template` SET `mechanic_immune_mask` = 650854271
WHERE `entry` IN (55265, 57409, 57771, 57772, 57773, 55346);

-- Power of the Aspects was a progressive post-release raid nerf. Do not apply
-- its -35% damage/-25% health aura to the unnerfed 4.3.4 encounter (or LFR).
UPDATE `creature_template_addon` SET `auras` = NULL
WHERE `entry` IN (55265, 57409, 57771, 57772, 57773, 57774, 57995, 57996);

-- Complete Morchok VO. Each omen/completion pair occupies adjacent groups so
-- the AI can select one pair and keep the two lines matched.
DELETE FROM `creature_text` WHERE `CreatureID` = 55265;
INSERT INTO `creature_text`
(`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(55265,  0, 0, 'No mortal shall turn me from my task.',                                      14, 0, 100, 0, 0, 26282, 0, 56455, 0, 'Morchok - Intro 1'),
(55265,  1, 0, 'Wyrmrest will fall. All will be dust.',                                       14, 0, 100, 0, 0, 26273, 0, 56478, 0, 'Morchok - Intro 2'),
(55265,  2, 0, 'You seek to halt an avalanche. I will bury you.',                             14, 0, 100, 0, 0, 26268, 0, 56456, 0, 'Morchok - Aggro'),
(55265,  3, 0, 'You thought to fight me alone? The earth splits to swallow and crush you.',   14, 0, 100, 0, 0, 26288, 0, 56457, 0, 'Morchok - Summon Kohcrom'),
(55265,  4, 0, '%s summons a Resonating Crystal!',                                            41, 0, 100, 0, 0,     0, 0,     0, 0, 'Morchok - Announce Resonating Crystal'),
(55265,  5, 0, 'Flee, and die.',                                                              14, 0, 100, 0, 0, 26283, 0, 56471, 0, 'Morchok - Resonating Crystal 1'),
(55265,  5, 1, 'Run, and perish.',                                                            14, 0, 100, 0, 0, 26284, 0, 56472, 0, 'Morchok - Resonating Crystal 2'),
(55265,  6, 0, 'The rocks tremble...',                                                        14, 0, 100, 0, 0, 26276, 0, 56465, 0, 'Morchok - Black Blood Omen A'),
(55265,  7, 0, '...and the rage of the true gods follows.',                                   14, 0, 100, 0, 0, 26280, 0, 56469, 0, 'Morchok - Black Blood A'),
(55265,  8, 0, 'The stone calls...',                                                          14, 0, 100, 0, 0, 26274, 0, 56462, 0, 'Morchok - Black Blood Omen B'),
(55265,  9, 0, '...and the black blood of the earth consumes you.',                           14, 0, 100, 0, 0, 26278, 0, 56467, 0, 'Morchok - Black Blood B'),
(55265, 10, 0, 'The ground shakes...',                                                        14, 0, 100, 0, 0, 26275, 0, 56463, 0, 'Morchok - Black Blood Omen C'),
(55265, 11, 0, '...and there is no escape from the Old Gods.',                                14, 0, 100, 0, 0, 26279, 0, 56468, 0, 'Morchok - Black Blood C'),
(55265, 12, 0, 'The surface quakes...',                                                       14, 0, 100, 0, 0, 26277, 0, 56466, 0, 'Morchok - Black Blood Omen D'),
(55265, 13, 0, '...and you drown in the hate of The Master.',                                 14, 0, 100, 0, 0, 26281, 0, 56470, 0, 'Morchok - Black Blood D'),
(55265, 14, 0, '$n! Get out of the black ooze on the ground!',                                42, 0, 100, 0, 0,     0, 0,     0, 0, 'Morchok - Black Blood Whisper'),
(55265, 15, 0, 'Impossible. This cannot be. The tower... must... fall...',                     14, 0, 100, 0, 0, 26269, 0, 56458, 0, 'Morchok - Death'),
(55265, 16, 0, 'I am unstoppable.',                                                           14, 0, 100, 0, 0, 26285, 0, 56459, 0, 'Morchok - Slay 1'),
(55265, 16, 1, 'It was inevitable.',                                                          14, 0, 100, 0, 0, 26286, 0, 56460, 0, 'Morchok - Slay 2'),
(55265, 16, 2, 'Ground to dust.',                                                              14, 0, 100, 0, 0, 26287, 0, 56461, 0, 'Morchok - Slay 3');

-- Base spells and all SpellDifficulty.dbc forks.
DELETE FROM `spell_script_names` WHERE `ScriptName` IN
('spell_morchok_stomp', 'spell_morchok_resonating_crystal_detonate',
 'spell_morchok_earthen_vortex', 'aura_morchok_earthen_vortex',
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
(103821, 'aura_morchok_earthen_vortex'),
(110047, 'aura_morchok_earthen_vortex'),
(110046, 'aura_morchok_earthen_vortex'),
(110045, 'aura_morchok_earthen_vortex'),
(103785, 'spell_morchok_black_blood_damage'),
(108570, 'spell_morchok_black_blood_damage'),
(110288, 'spell_morchok_black_blood_damage'),
(110287, 'spell_morchok_black_blood_damage'),
(103176, 'aura_morchok_falling_fragments');

-- Rebuild every destination from literal WDB values. The previous migration
-- copied from 55265 and then deleted its LFR/heroic source rows, so rerunning it
-- or upgrading a partially migrated database produced empty loot tables.
DROP TEMPORARY TABLE IF EXISTS `tmp_morchok_loot`;
CREATE TEMPORARY TABLE `tmp_morchok_loot`
(
    `Tier` TINYINT UNSIGNED NOT NULL,
    `Item` INT UNSIGNED NOT NULL,
    `Chance` FLOAT NOT NULL,
    `MaxCount` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    PRIMARY KEY (`Tier`, `Item`)
);

-- Tier 1: normal (item level 397)
INSERT INTO `tmp_morchok_loot` (`Tier`, `Item`, `Chance`, `MaxCount`) VALUES
(1, 77262, 19.9639, 1), (1, 77214, 19.1530, 1), (1, 77270, 17.7745, 1),
(1, 77269, 17.4004, 1), (1, 77271, 17.3559, 1), (1, 77212, 16.9662, 1),
(1, 77267, 16.0663, 1), (1, 77268, 16.0559, 1), (1, 77213, 11.4782, 1),
(1, 77261, 11.0571, 1), (1, 77263, 10.5417, 1), (1, 77266, 10.4790, 1),
(1, 77265, 10.3534, 1), (1, 77232,  1.2556, 1), (1, 77210,  1.2399, 1),
(1, 77230,  1.1980, 1), (1, 77207,  1.1693, 1), (1, 77208,  1.1379, 1),
(1, 77231,  1.1274, 1), (1, 77209,  1.1013, 1), (1, 77229,  1.0646, 1),
(1, 77211,  1.0620, 1), (1, 77228,  1.0489, 1);

-- Tier 2: Raid Finder (item level 384)
INSERT INTO `tmp_morchok_loot` (`Tier`, `Item`, `Chance`, `MaxCount`) VALUES
(2, 78382, 41.3743, 1), (2, 78375, 35.8341, 1), (2, 78376, 35.6850, 1),
(2, 78377, 33.1136, 1), (2, 78381, 32.5198, 1), (2, 78374, 23.6051, 1),
(2, 78378, 22.1455, 1), (2, 78380, 20.6963, 1), (2, 78384, 20.0345, 1),
(2, 78385, 19.3230, 1), (2, 78386, 18.0727, 1),
(2, 78874,  6.2335, 1), (2, 78862,  6.0425, 1), (2, 78865,  5.5324, 1),
(2, 78868,  5.4671, 1), (2, 78871,  5.3232, 1), (2, 78875,  4.9857, 1),
(2, 78872,  4.9596, 1), (2, 78869,  4.8000, 1), (2, 78863,  4.5960, 1),
(2, 78876,  4.4286, 1), (2, 78873,  4.3972, 1), (2, 78867,  4.3946, 1),
(2, 78864,  4.3213, 1), (2, 78870,  3.6831, 1), (2, 78866,  3.6255, 1),
(2, 77980,  2.4353, 1), (2, 77982,  2.3045, 1), (2, 78498,  2.2679, 1),
(2, 78497,  2.2653, 1), (2, 78496,  2.1685, 1), (2, 77981,  1.9671, 1),
(2, 77983,  1.9462, 1), (2, 78494,  1.9357, 1), (2, 78495,  1.8912, 1),
(2, 77979,  1.8546, 1);

-- Tier 3: heroic (item level 410)
INSERT INTO `tmp_morchok_loot` (`Tier`, `Item`, `Chance`, `MaxCount`) VALUES
(3, 78363, 12.3362, 1), (3, 78364, 12.1008, 1), (3, 78362, 11.1015, 1),
(3, 78373, 10.9812, 1), (3, 78368, 10.6254, 1), (3, 78371, 10.5888, 1),
(3, 78366, 10.1703, 1), (3, 78367,  9.5843, 1), (3, 78369,  7.0078, 1),
(3, 78372,  6.9711, 1), (3, 78365,  6.9371, 1), (3, 78370,  6.5657, 1),
(3, 78361,  6.5108, 1), (3, 78492,  0.8240, 1), (3, 78490,  0.7952, 1),
(3, 77999,  0.7769, 1), (3, 78000,  0.7638, 1), (3, 78002,  0.7507, 1),
(3, 78491,  0.7507, 1), (3, 78489,  0.7455, 1), (3, 78001,  0.7377, 1),
(3, 78003,  0.6853, 1), (3, 78493,  0.6827, 1);

-- Tier 4: shared quest/currency-style drops
INSERT INTO `tmp_morchok_loot` (`Tier`, `Item`, `Chance`, `MaxCount`) VALUES
(4, 71716, 100.0000, 1), (4, 77952, 81.1060, 3), (4, 71998, 75.4846, 3);

DELETE FROM `creature_loot_template` WHERE `Entry` IN (55265, 57409, 57771, 57772);

-- 10 normal: normal + shared, all in default loot mode.
INSERT INTO `creature_loot_template`
(`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT 55265, `Item`, 0, `Chance`, 0, 0, 1, 0, 1, `MaxCount`, 'Morchok 10N'
FROM `tmp_morchok_loot` WHERE `Tier` IN (1, 4);

-- The 25-normal creature is also the LFR creature. Shared rows use mode 3 so
-- they are enabled by both normal mode 1 and the LFR mode-2 mask.
INSERT INTO `creature_loot_template`
(`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT 57409, `Item`, 0, `Chance`, 0, 0, 1, 0, 1, `MaxCount`, 'Morchok 25N'
FROM `tmp_morchok_loot` WHERE `Tier` = 1;

INSERT INTO `creature_loot_template`
(`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT 57409, `Item`, 0, `Chance`, 0, 0, 2, 0, 1, `MaxCount`, 'Morchok LFR'
FROM `tmp_morchok_loot` WHERE `Tier` = 2;

INSERT INTO `creature_loot_template`
(`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT 57409, `Item`, 0, `Chance`, 0, 0, 3, 0, 1, `MaxCount`, 'Morchok shared'
FROM `tmp_morchok_loot` WHERE `Tier` = 4;

-- Both heroic raid sizes use the heroic and shared rows.
INSERT INTO `creature_loot_template`
(`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT 57771, `Item`, 0, `Chance`, 0, 0, 1, 0, 1, `MaxCount`, 'Morchok 10H'
FROM `tmp_morchok_loot` WHERE `Tier` IN (3, 4);

INSERT INTO `creature_loot_template`
(`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT 57772, `Item`, 0, `Chance`, 0, 0, 1, 0, 1, `MaxCount`, 'Morchok 25H'
FROM `tmp_morchok_loot` WHERE `Tier` IN (3, 4);

UPDATE `creature_template` SET `lootid` = `entry` WHERE `entry` IN (55265, 57409, 57771, 57772);
DROP TEMPORARY TABLE `tmp_morchok_loot`;
