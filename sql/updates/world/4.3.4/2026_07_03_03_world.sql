-- Kezan quest chain: Batch C - the KTC party arc
-- 14113/14153 Life of the Party / 14115 Pirate Party Crashers /
-- 14116 The Uninvited Guest / 14120 A Bazillion Macaroons?!

-- ----------------------------------------------------------------------------
-- 1) Kezan Partygoers: replace the TDB bark-only SmartAI with the C++ script
--    that owns the wanted-action state (npc_kezan_partygoer).
-- ----------------------------------------------------------------------------
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_kezan_partygoer' WHERE `entry` IN (35175, 35185, 35186, 35201);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (35175, 35185, 35186, 35201) AND `source_type` = 0;

-- Replicate 35186's bark groups (0-4 wants, 5-9 served responses) to the other
-- partygoer entries so Talk() works on all of them.
DELETE FROM `creature_text` WHERE `CreatureID` IN (35175, 35185, 35201);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 35175, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`
FROM `creature_text` WHERE `CreatureID` = 35186;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 35185, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`
FROM `creature_text` WHERE `CreatureID` = 35186;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 35201, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`
FROM `creature_text` WHERE `CreatureID` = 35186;

-- Party action spells (validated against the partygoer's wanted state in C++).
DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_kezan_party_action';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(66909, 'spell_kezan_party_action'), -- Bubbly
(66910, 'spell_kezan_party_action'), -- Bucket
(66911, 'spell_kezan_party_action'), -- Dance
(66912, 'spell_kezan_party_action'), -- Fireworks
(66913, 'spell_kezan_party_action'); -- Hors D'oeuvres

-- ----------------------------------------------------------------------------
-- 2) SmartAI fixes:
--    - Sassy removes the real party aura (66908) on the female turn-in, not a
--      legacy phase aura (59073).
--    - Chip Endale mirrors Candy Cane: cast 66908 + party instructions whisper
--      on Life of the Party (female, 14153) accept.
-- ----------------------------------------------------------------------------
UPDATE `smart_scripts` SET `action_param1` = 66908, `comment` = 'Sassy Hardwrench - On Quest 14153 Finished - Remove Aura ''Awesome Party Ensemble''' WHERE `entryorguid` = 34668 AND `source_type` = 0 AND `id` = 10;

UPDATE `smart_scripts` SET `link` = 4 WHERE `entryorguid` = 35054 AND `source_type` = 0 AND `id` = 3;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 35054 AND `source_type` = 0 AND `id` IN (4, 5);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(35054, 0, 4, 5, 61, 0, 100, 0, 14153, 0, 0, 0, 0, 11, 66908, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Chip Endale - On Quest 14153 Taken - Cast ''Awesome Party Ensemble'''),
(35054, 0, 5, 0, 61, 0, 100, 0, 14153, 0, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Chip Endale - On Quest 14153 Taken - Whisper party instructions');

DELETE FROM `creature_text` WHERE `CreatureID` = 35054 AND `GroupID` = 2;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(35054, 2, 0, 'You are dressed to impress! Use your new powers below to make your party guests happy!', 42, 0, 100, 0, 0, 0, 0, 0, 'Chip Endale - party instructions whisper');

-- ----------------------------------------------------------------------------
-- 3) Missing questgiver spawns for the party/post-party eras (clones of the
--    base-phase rows with only guid/PhaseId changed).
-- ----------------------------------------------------------------------------
SET @CGUID := 9000300;
DELETE FROM `creature` WHERE `guid` BETWEEN @CGUID+0 AND @CGUID+56;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild`) VALUES
(@CGUID+0, 34668, 648, 4737, 4765, 1, 0, 1, 379, 0, -1, 0, 0, -8423.83, 1366.01, 104.815, 4.69494, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 15595), -- Sassy Hardwrench (party era)
(@CGUID+1, 35053, 648, 4737, 4765, 1, 0, 1, 379, 0, -1, 0, 0, -8425.2, 1367.76, 104.76, 5.07891, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 15595), -- Candy Cane (party era)
(@CGUID+2, 35054, 648, 4737, 4765, 1, 0, 1, 379, 0, -1, 0, 0, -8422.14, 1367.71, 104.758, 4.57276, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 15595), -- Chip Endale (party era)
(@CGUID+3, 35222, 648, 4737, 4765, 1, 0, 1, 380, 0, -1, 0, 0, -8423.747, 1362.0642, 116.945045, 4.660029, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 15595), -- Trade Prince Gallywix (KTC HQ, crashers era)
(@CGUID+4, 35222, 648, 4737, 4765, 1, 0, 1, 381, 0, -1, 0, 0, -8423.747, 1362.0642, 116.945045, 4.660029, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 15595), -- Trade Prince Gallywix (KTC HQ, The Uninvited Guest turn-in)
(@CGUID+5, 35222, 648, 4737, 4765, 1, 0, 1, 382, 0, -1, 0, 0, -8423.747, 1362.0642, 116.945045, 4.660029, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 15595), -- Trade Prince Gallywix (KTC HQ, A Bazillion Macaroons?! giver)
-- KTC Waiters/Waitresses with drink trays (vehicle accessories already in DB)
(@CGUID+6, 48719, 648, 4737, 4765, 1, 0, 1, 379, 0, -1, 0, 0, -8456.583, 1334.349, 101.69675, 1.068996, 300, 5, 0, 1, 0, 1, 0, 0, 0, '', 15595), -- KTC Waiter
(@CGUID+7, 48719, 648, 4737, 4765, 1, 0, 1, 379, 0, -1, 0, 0, -8511.726, 1367.9312, 101.696686, 1.60123, 300, 5, 0, 1, 0, 1, 0, 0, 0, '', 15595), -- KTC Waiter
(@CGUID+8, 48719, 648, 4737, 4765, 1, 0, 1, 379, 0, -1, 0, 0, -8515.0, 1304.9913, 101.843025, 0.698132, 300, 5, 0, 1, 0, 1, 0, 0, 0, '', 15595), -- KTC Waiter
(@CGUID+9, 48721, 648, 4737, 4765, 1, 0, 1, 379, 0, -1, 0, 0, -8444.272, 1334.1007, 102.24418, 3.193953, 300, 5, 0, 1, 0, 1, 0, 0, 0, '', 15595), -- KTC Waitress
(@CGUID+10, 48721, 648, 4737, 4765, 1, 0, 1, 379, 0, -1, 0, 0, -8455.221, 1374.3195, 102.10864, 3.857178, 300, 5, 0, 1, 0, 1, 0, 0, 0, '', 15595), -- KTC Waitress
(@CGUID+11, 48721, 648, 4737, 4765, 1, 0, 1, 379, 0, -1, 0, 0, -8521.179, 1367.9548, 101.79638, 6.248279, 300, 5, 0, 1, 0, 1, 0, 0, 0, '', 15595), -- KTC Waitress
(@CGUID+12, 48805, 648, 4737, 4765, 1, 0, 1, 379, 0, -1, 0, 0, -8446.911, 1375.1088, 102.687485, 2.281945, 300, 5, 0, 1, 0, 1, 0, 0, 0, '', 15595), -- KTC Waiter
(@CGUID+13, 48805, 648, 4737, 4765, 1, 0, 1, 379, 0, -1, 0, 0, -8444.127, 1314.6005, 102.36128, 1.868924, 300, 5, 0, 1, 0, 1, 0, 0, 0, '', 15595), -- KTC Waiter
(@CGUID+14, 48805, 648, 4737, 4765, 1, 0, 1, 379, 0, -1, 0, 0, -8484.101, 1379.1917, 101.696686, 3.796175, 300, 5, 0, 1, 0, 1, 0, 0, 0, '', 15595), -- KTC Waiter
(@CGUID+15, 48805, 648, 4737, 4765, 1, 0, 1, 379, 0, -1, 0, 0, -8517.445, 1371.79, 101.696686, 0.847443, 300, 5, 0, 1, 0, 1, 0, 0, 0, '', 15595), -- KTC Waiter
(@CGUID+16, 48806, 648, 4737, 4765, 1, 0, 1, 379, 0, -1, 0, 0, -8476.5625, 1304.167, 101.69001, 4.247992, 300, 5, 0, 1, 0, 1, 0, 0, 0, '', 15595), -- KTC Waitress
-- Mount Kajaro fiery boulder casters (volcanic bombardment ambience, eras 382 and 384)
(@CGUID+17, 37748, 648, 4737, 0, 1, 0, 1, 382, 0, -1, 0, 0, -8545.906, 1464.1111, 272.69073, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+18, 37748, 648, 4737, 0, 1, 0, 1, 382, 0, -1, 0, 0, -8552.268, 1361.2101, 304.12756, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+19, 37748, 648, 4737, 0, 1, 0, 1, 382, 0, -1, 0, 0, -8560.712, 1412.368, 316.24655, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+20, 37748, 648, 4737, 0, 1, 0, 1, 382, 0, -1, 0, 0, -8590.9375, 1255.1094, 271.27612, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+21, 37748, 648, 4737, 0, 1, 0, 1, 382, 0, -1, 0, 0, -8539.879, 1500.1945, 310.27283, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+22, 37748, 648, 4737, 0, 1, 0, 1, 382, 0, -1, 0, 0, -8552.9375, 1322.0035, 363.85782, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+23, 37748, 648, 4737, 0, 1, 0, 1, 382, 0, -1, 0, 0, -8542.863, 1538.4028, 307.0921, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+24, 37748, 648, 4737, 0, 1, 0, 1, 382, 0, -1, 0, 0, -8590.689, 1226.5798, 304.8886, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+25, 37748, 648, 4737, 0, 1, 0, 1, 382, 0, -1, 0, 0, -8570.441, 1293.0017, 356.79736, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+26, 37748, 648, 4737, 0, 1, 0, 1, 382, 0, -1, 0, 0, -8592.328, 1246.3594, 324.6682, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+27, 37748, 648, 4737, 0, 1, 0, 1, 382, 0, -1, 0, 0, -8555.018, 1583.4844, 270.1007, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+28, 37748, 648, 4737, 0, 1, 0, 1, 382, 0, -1, 0, 0, -8604.377, 1438.9202, 348.81445, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+29, 37748, 648, 4737, 0, 1, 0, 1, 382, 0, -1, 0, 0, -8649.674, 1373.1545, 329.03702, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+30, 37748, 648, 4737, 0, 1, 0, 1, 382, 0, -1, 0, 0, -8600.366, 1595.5817, 340.69833, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+31, 37748, 648, 4737, 0, 1, 0, 1, 382, 0, -1, 0, 0, -8634.647, 1492.3473, 327.53055, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+32, 37748, 648, 4737, 0, 1, 0, 1, 382, 0, -1, 0, 0, -8627.372, 1525.6805, 372.4441, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+33, 37748, 648, 4737, 0, 1, 0, 1, 382, 0, -1, 0, 0, -8613.292, 1549.1216, 311.92368, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+34, 37748, 648, 4737, 0, 1, 0, 1, 382, 0, -1, 0, 0, -8637.863, 1307.5173, 365.68848, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+35, 37748, 648, 4737, 0, 1, 0, 1, 382, 0, -1, 0, 0, -8636.703, 1349.1892, 367.26965, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+36, 37748, 648, 4737, 0, 1, 0, 1, 382, 0, -1, 0, 0, -8662.484, 1626.6354, 365.35272, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+37, 37748, 648, 4737, 0, 1, 0, 1, 384, 0, -1, 0, 0, -8545.906, 1464.1111, 272.69073, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+38, 37748, 648, 4737, 0, 1, 0, 1, 384, 0, -1, 0, 0, -8552.268, 1361.2101, 304.12756, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+39, 37748, 648, 4737, 0, 1, 0, 1, 384, 0, -1, 0, 0, -8560.712, 1412.368, 316.24655, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+40, 37748, 648, 4737, 0, 1, 0, 1, 384, 0, -1, 0, 0, -8590.9375, 1255.1094, 271.27612, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+41, 37748, 648, 4737, 0, 1, 0, 1, 384, 0, -1, 0, 0, -8539.879, 1500.1945, 310.27283, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+42, 37748, 648, 4737, 0, 1, 0, 1, 384, 0, -1, 0, 0, -8552.9375, 1322.0035, 363.85782, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+43, 37748, 648, 4737, 0, 1, 0, 1, 384, 0, -1, 0, 0, -8542.863, 1538.4028, 307.0921, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+44, 37748, 648, 4737, 0, 1, 0, 1, 384, 0, -1, 0, 0, -8590.689, 1226.5798, 304.8886, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+45, 37748, 648, 4737, 0, 1, 0, 1, 384, 0, -1, 0, 0, -8570.441, 1293.0017, 356.79736, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+46, 37748, 648, 4737, 0, 1, 0, 1, 384, 0, -1, 0, 0, -8592.328, 1246.3594, 324.6682, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+47, 37748, 648, 4737, 0, 1, 0, 1, 384, 0, -1, 0, 0, -8555.018, 1583.4844, 270.1007, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+48, 37748, 648, 4737, 0, 1, 0, 1, 384, 0, -1, 0, 0, -8604.377, 1438.9202, 348.81445, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+49, 37748, 648, 4737, 0, 1, 0, 1, 384, 0, -1, 0, 0, -8649.674, 1373.1545, 329.03702, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+50, 37748, 648, 4737, 0, 1, 0, 1, 384, 0, -1, 0, 0, -8600.366, 1595.5817, 340.69833, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+51, 37748, 648, 4737, 0, 1, 0, 1, 384, 0, -1, 0, 0, -8634.647, 1492.3473, 327.53055, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+52, 37748, 648, 4737, 0, 1, 0, 1, 384, 0, -1, 0, 0, -8627.372, 1525.6805, 372.4441, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+53, 37748, 648, 4737, 0, 1, 0, 1, 384, 0, -1, 0, 0, -8613.292, 1549.1216, 311.92368, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+54, 37748, 648, 4737, 0, 1, 0, 1, 384, 0, -1, 0, 0, -8637.863, 1307.5173, 365.68848, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+55, 37748, 648, 4737, 0, 1, 0, 1, 384, 0, -1, 0, 0, -8636.703, 1349.1892, 367.26965, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+56, 37748, 648, 4737, 0, 1, 0, 1, 384, 0, -1, 0, 0, -8662.484, 1626.6354, 365.35272, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595);

-- Boulder caster behavior: repeated cosmetic volcano boulders (spell picks its own
-- random destination). Entry 37748 is the dedicated 4.3.4 bombardment bunny.
UPDATE `creature_template` SET `AIName` = 'SmartAI', `unit_flags` = 33555200 WHERE `entry` = 37748;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 37748 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(37748, 0, 0, 0, 1, 0, 100, 0, 2000, 12000, 6000, 14000, 0, 11, 70097, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Fiery Boulder Caster - Out of Combat - Cast Fiery Boulder');
