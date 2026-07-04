--
-- Gilneas (Worgen starter zone) — spawn fixes and additions
-- Custom guid block 9000500+ (9000300-360 Kezan, 9000400-422 Lost Isles are taken).
--

-- "Stranded at the Marsh" (24468): Swamp Crocolisk 37078 had zero spawns anywhere.
-- Retail positions from sniff object creates (lines 15424581-15470858).
DELETE FROM `creature` WHERE `guid` BETWEEN 9000500 AND 9000509;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild`) VALUES
(9000500, 37078, 654, 4714, 4787, 1, 0, 1, 186, 0, -1, 0, 0, -2178.44, 1788.62, 12.49, 6.10, 120, 5, 0, 1, 0, 1, 0, 0, 0, '', 0),
(9000501, 37078, 654, 4714, 4787, 1, 0, 1, 186, 0, -1, 0, 0, -2172.27, 1787.07, 9.84, 5.98, 120, 5, 0, 1, 0, 1, 0, 0, 0, '', 0),
(9000502, 37078, 654, 4714, 4787, 1, 0, 1, 186, 0, -1, 0, 0, -2162.23, 1783.98, 7.87, 5.98, 120, 5, 0, 1, 0, 1, 0, 0, 0, '', 0),
(9000503, 37078, 654, 4714, 4787, 1, 0, 1, 186, 0, -1, 0, 0, -2158.89, 1782.95, 7.46, 5.98, 120, 5, 0, 1, 0, 1, 0, 0, 0, '', 0),
(9000504, 37078, 654, 4714, 4787, 1, 0, 1, 186, 0, -1, 0, 0, -2129.27, 1775.32, 6.64, 5.98, 120, 5, 0, 1, 0, 1, 0, 0, 0, '', 0),
(9000505, 37078, 654, 4714, 4787, 1, 0, 1, 186, 0, -1, 0, 0, -2087.44, 1766.25, 6.79, 3.11, 120, 5, 0, 1, 0, 1, 0, 0, 0, '', 0),
(9000506, 37078, 654, 4714, 4787, 1, 0, 1, 186, 0, -1, 0, 0, -2085.65, 1788.31, 5.03, 1.24, 120, 5, 0, 1, 0, 1, 0, 0, 0, '', 0),
(9000507, 37078, 654, 4714, 4787, 1, 0, 1, 186, 0, -1, 0, 0, -2083.20, 1789.28, 4.71, 4.64, 120, 5, 0, 1, 0, 1, 0, 0, 0, '', 0),
(9000508, 37078, 654, 4714, 4787, 1, 0, 1, 186, 0, -1, 0, 0, -2069.42, 1783.83, 5.13, 6.26, 120, 5, 0, 1, 0, 1, 0, 0, 0, '', 0),
(9000509, 37078, 654, 4714, 4787, 1, 0, 1, 186, 0, -1, 0, 0, -2069.10, 1783.01, 5.16, 4.72, 120, 5, 0, 1, 0, 1, 0, 0, 0, '', 0);

-- "Introductions Are in Order" (24472): Koroth's Banner GO spawn (template 201594
-- already exists; loot 27732 -> 49742 already wired). WPP-verified retail placement.
DELETE FROM `gameobject` WHERE `guid` = 9000500;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `VerifiedBuild`) VALUES
(9000500, 201594, 654, 4714, 4794, 1, 0, 1, 186, 0, -1, -2278.399, 1969.408, 98.088, 2.809975, 0, 0, -0.986285, 0.165050, 120, 100, 1, '', 0);

-- Multi-phase NPC duplicates (retail reuses these NPCs across phases; this fork's
-- spawns are single-PhaseId, so give each a copy in the additional phase).
DELETE FROM `creature` WHERE `guid` BETWEEN 9000510 AND 9000518;
-- King Genn Greymane 36743 -> phase 186 (serves "Alas, Gilneas!" 14467 and "Exodus" 24438 after the 183->186 flip)
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild`)
SELECT 9000510, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, 186, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild` FROM `creature` WHERE `guid` = 256017;
-- Lorna Crowley 38611 -> phase 190 (serves "The Hunt For Sylvanas" 24902 S/E and "Vengeance or Survival" 24903 S after 187 drops)
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild`)
SELECT 9000514, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, 190, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild` FROM `creature` WHERE `guid` = 257106;
-- Phase-191 Keel Harbor trio (sniff-proven positions; Endgame turn-in + Rut'theran handoff happen in 191)
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild`)
SELECT 9000511, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, 191, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild` FROM `creature` WHERE `guid` = 258347;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild`)
SELECT 9000512, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, 191, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild` FROM `creature` WHERE `guid` = 258342;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild`)
SELECT 9000513, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, 191, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild` FROM `creature` WHERE `guid` = 258283;

-- "They Have Allies, But So Do We" (24681): 2 static Glaive Throwers (sniff creates)
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild`) VALUES
(9000515, 37927, 654, 4714, 4726, 1, 0, 1, 189, 0, -1, 0, 0, -1325.08, 2108.16, 5.71, 5.19, 120, 0, 0, 1, 0, 0, 0, 0, 0, '', 0),
(9000516, 37927, 654, 4714, 4726, 1, 0, 1, 189, 0, -1, 0, 0, -1240.62, 2454.97, 63.40, 4.40, 120, 0, 0, 1, 0, 0, 0, 0, 0, '', 0);

-- Teldrassil bridge: Genn Greymane 48736 had zero spawns on any map.
-- (a) Rut'theran Village beside Krennan 42968 -> starter of "Breaking Waves of Change" (26385)
-- (b) The Howling Oak, Darnassus -> ender of "The Howling Oak" (28517)
--     NOTE: Z at the Howling Oak is a placeholder - snap in-game before commit.
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild`) VALUES
(9000517, 48736, 1, 141, 702, 1, 0, 1, 0, 0, -1, 0, 0, 8352.90, 993.40, 21.85, 5.30, 120, 0, 0, 1, 0, 0, 0, 0, 0, '', 0),
(9000518, 48736, 1, 1657, 1657, 1, 0, 1, 0, 0, -1, 0, 0, 10316.00, 2447.00, 1330.00, 2.00, 120, 0, 0, 1, 0, 0, 0, 0, 0, '', 0);

-- (Funeral cast statics, Endgame deck controller/GO spawns are appended by the
--  follow-up hunk once sniff extraction confirms their retail positions.)

-- "Patriarch's Blessing" (24679): static funeral cast at Aderic's Repose, phase 187
-- (retail uses per-player clones; static trio + text loop reproduces the scene).
-- Also the missing Liam's Coffin GO (lid 207999 is already spawned ph188).
DELETE FROM `creature` WHERE `guid` BETWEEN 9000519 AND 9000521;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild`) VALUES
(9000519, 50893, 654, 4714, 4727, 1, 0, 1, 187, 0, -1, 0, 0, -1644.2379, 1904.0660, 30.0416, 3.56047, 120, 0, 0, 1, 0, 0, 0, 0, 0, '', 0),
(9000520, 50881, 654, 4714, 4727, 1, 0, 1, 187, 0, -1, 0, 0, -1640.8837, 1903.5955, 30.0350, 3.35103, 120, 0, 0, 1, 0, 0, 0, 0, 0, '', 0),
(9000521, 50902, 654, 4714, 4727, 1, 0, 1, 187, 0, -1, 0, 0, -1642.3507, 1907.4670, 30.0350, 3.57793, 120, 0, 0, 1, 0, 0, 0, 0, 0, '', 0);
DELETE FROM `gameobject` WHERE `guid` = 9000501;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `VerifiedBuild`) VALUES
(9000501, 207626, 654, 4714, 4727, 1, 0, 1, 187, 0, -1, -1645.566, 1902.8091, 29.9963, 5.09636, 0, 0, -0.559193, 0.829038, 120, 100, 1, '', 0);

-- "Endgame" (26706): the retail Orc Gunship is a moving transport (GO 203428, taxi
-- path 2338) whose deck crew lives on transport map 749 and is never instantiated by
-- this fork. Static implementation: a non-moving gunship backdrop (custom template
-- 800100, type 5 generic, same display) anchored at the sniff-solved ship frame
-- (T = -1465.76, 3251.48, 93.84, yaw 2.1433485), with every map-749 deck spawn
-- re-based into map-654 world coordinates (phase 191). Korm Bonegrind and the
-- deck squad are animated by the chapter-4 endgame controller.
DELETE FROM `gameobject_template` WHERE `entry` = 800100;
INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `IconName`, `castBarCaption`, `unk1`, `size`, `Data0`, `RequiredLevel`, `AIName`, `ScriptName`, `VerifiedBuild`) VALUES
(800100, 5, 8253, 'Orc Gunship', '', '', '', 1, 0, 0, '', '', 0);
DELETE FROM `gameobject` WHERE `guid` = 9000502;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `VerifiedBuild`) VALUES
(9000502, 800100, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, -1465.76, 3251.48, 93.84, 2.14335, 0, 0, 0.878003, 0.478655, 7200, 255, 1, '', 0);

-- Deck crew re-based from transport map 749 (Lorna Crowley 43566, Korm Bonegrind
-- 43567, Worgen Warriors, Gilnean Sharpshooters, 22 Gunship Grunts, rope vehicles,
-- Admiral Stormblood, Navigators, static Wyverns, scene triggers).
DELETE FROM `creature` WHERE `guid` BETWEEN 9000530 AND 9000633;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild`) VALUES
(9000530, 40350, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1466.0557, 3253.9979, 128.7046, 2.14335, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000531, 42141, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1449.1645, 3210.7258, 106.0685, 2.74077, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000532, 42141, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1432.0620, 3237.4233, 127.9301, 5.71719, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000533, 42141, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1439.7516, 3259.1799, 183.7706, 2.64730, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000534, 42141, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1470.6025, 3235.9972, 127.7021, 6.02801, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000535, 42141, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1447.4807, 3249.2124, 127.7011, 2.32383, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000536, 42141, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1482.2771, 3236.3319, 128.0948, 5.60288, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000537, 42141, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1475.6637, 3250.9155, 178.5926, 2.07637, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000538, 42141, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1459.2870, 3257.8886, 178.7009, 4.23554, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000539, 42141, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1489.6652, 3239.4331, 183.7384, 2.61431, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000540, 42141, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1451.6098, 3276.8627, 183.6875, 1.95922, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000541, 42141, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1453.5821, 3236.8829, 101.0848, 6.08261, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000542, 42141, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1491.0280, 3248.5638, 183.7068, 0.44515, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000543, 42141, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1486.9597, 3237.9195, 183.7455, 2.77921, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000544, 42141, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1473.0694, 3232.4669, 103.3885, 3.66452, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000545, 42141, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1444.7864, 3249.5766, 103.4684, 4.11281, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000546, 42141, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1496.8531, 3258.4758, 129.4972, 1.36561, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000547, 42141, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1480.5514, 3241.9435, 102.9348, 5.75516, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000548, 42141, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1461.3966, 3281.0623, 129.3387, 3.71119, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000549, 42141, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1456.5454, 3293.8797, 183.6855, 0.43292, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000550, 42141, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1456.1348, 3275.2743, 103.4696, 0.54244, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000551, 42141, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1490.7687, 3280.6674, 104.1650, 1.86397, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000552, 42141, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1482.0649, 3286.0308, 104.1697, 2.11203, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000553, 43566, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1469.5297, 3257.1838, 178.6370, 2.14335, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000554, 43567, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1436.3203, 3229.6905, 105.4304, 3.18799, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000555, 43651, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1464.5105, 3257.0695, 178.6370, 2.14335, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000556, 43651, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1436.3437, 3255.1347, 183.8693, 2.19571, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000557, 43651, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1486.5565, 3234.9729, 183.8380, 2.19571, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000558, 43651, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1471.4442, 3252.8706, 178.6370, 2.19571, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000559, 43651, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1454.3689, 3294.1607, 183.6896, 4.74389, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000560, 43703, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1469.1530, 3249.1349, 178.6370, 2.14335, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000561, 43703, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1461.6543, 3253.3062, 178.6370, 2.14335, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000562, 43703, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1448.7525, 3274.1654, 183.7911, 3.67924, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000563, 43703, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1492.1290, 3247.8054, 183.7875, 0.65981, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000566, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1515.0466, 3192.9139, 192.5124, 5.30240, 7200, 10, 0, 0, 0, 1, 0, 0, 0, '', 0),
(9000567, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1525.3691, 3205.0103, 191.3730, 5.30240, 7200, 10, 0, 0, 0, 1, 0, 0, 0, '', 0),
(9000568, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1480.7969, 3207.4021, 126.6867, 5.30240, 7200, 10, 0, 0, 0, 1, 0, 0, 0, '', 0),
(9000569, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1539.2209, 3228.1033, 188.7473, 5.30240, 7200, 10, 0, 0, 0, 1, 0, 0, 0, '', 0),
(9000570, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1465.3678, 3213.5755, 128.0498, 5.30240, 7200, 10, 0, 0, 0, 1, 0, 0, 0, '', 0),
(9000571, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1547.2424, 3250.8087, 186.0023, 5.30240, 7200, 10, 0, 0, 0, 1, 0, 0, 0, '', 0),
(9000572, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1488.7609, 3245.1834, 128.8155, 5.30240, 7200, 10, 0, 0, 0, 1, 0, 0, 0, '', 0),
(9000573, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1483.4151, 3197.3038, 191.8181, 5.30240, 7200, 10, 0, 0, 0, 1, 0, 0, 0, '', 0),
(9000574, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1518.7164, 3234.4788, 185.2311, 5.30240, 7200, 10, 0, 0, 0, 1, 0, 0, 0, '', 0),
(9000575, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1471.3191, 3245.0661, 99.6108, 5.30240, 7200, 10, 0, 0, 0, 1, 0, 0, 0, '', 0),
(9000576, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1438.3966, 3187.2226, 136.9323, 5.30240, 7200, 10, 0, 0, 0, 1, 0, 0, 0, '', 0),
(9000577, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1478.9869, 3255.0853, 100.0077, 5.30240, 7200, 10, 0, 0, 0, 1, 0, 0, 0, '', 0),
(9000578, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1485.7610, 3267.1126, 101.1925, 5.30240, 7200, 10, 0, 0, 0, 1, 0, 0, 0, '', 0),
(9000579, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1463.1206, 3229.9053, 131.3801, 5.30240, 7200, 10, 0, 0, 0, 1, 0, 0, 0, '', 0),
(9000580, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1445.9706, 3207.1387, 129.3064, 5.30240, 7200, 10, 0, 0, 0, 1, 0, 0, 0, '', 0),
(9000581, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1484.4770, 3215.9453, 188.7704, 5.30240, 7200, 10, 0, 0, 0, 1, 0, 0, 0, '', 0),
(9000582, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1489.4463, 3216.4974, 199.3990, 5.30240, 7200, 10, 0, 0, 0, 1, 0, 0, 0, '', 0),
(9000583, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1525.3820, 3253.0447, 199.5130, 5.30240, 7200, 10, 0, 0, 0, 1, 0, 0, 0, '', 0),
(9000584, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1447.2532, 3219.0344, 136.7256, 5.30240, 7200, 10, 0, 0, 0, 1, 0, 0, 0, '', 0),
(9000585, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1477.3230, 3269.8681, 103.5007, 5.30240, 7200, 10, 0, 0, 0, 1, 0, 0, 0, '', 0),
(9000586, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1489.0358, 3265.6551, 135.2217, 5.30240, 7200, 10, 0, 0, 0, 1, 0, 0, 0, '', 0),
(9000587, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1503.9798, 3243.6202, 193.5830, 5.30240, 7200, 10, 0, 0, 0, 1, 0, 0, 0, '', 0),
(9000588, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1521.2009, 3262.8701, 188.2779, 5.30240, 7200, 10, 0, 0, 0, 1, 0, 0, 0, '', 0),
(9000589, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1467.3093, 3259.4839, 128.3582, 5.30240, 7200, 10, 0, 0, 0, 1, 0, 0, 0, '', 0),
(9000590, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1433.9493, 3210.2198, 134.5244, 5.30240, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000591, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1425.3706, 3231.1528, 127.8819, 5.30240, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000592, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1390.6895, 3257.2173, 190.6969, 5.30240, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000593, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1431.6403, 3265.2418, 195.1640, 5.30240, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000594, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1412.2722, 3249.6524, 196.1490, 5.30240, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000595, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1465.9938, 3254.0910, 178.6370, 5.30240, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000596, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1423.1487, 3253.7573, 193.6107, 5.30240, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000597, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1414.6926, 3256.2532, 147.6549, 5.30240, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000598, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1435.2902, 3263.6077, 119.4530, 5.30240, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000599, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1440.5228, 3233.0702, 129.4297, 5.30240, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000600, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1442.1264, 3262.9418, 128.2229, 5.30240, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000601, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1415.9067, 3309.6432, 190.4181, 5.30240, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000602, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1400.5457, 3287.2535, 194.4610, 5.30240, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000603, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1423.2264, 3284.3398, 210.4390, 5.30240, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000604, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1414.6339, 3291.1730, 203.6500, 5.30240, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000605, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1444.8090, 3284.9701, 190.7752, 5.30240, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000606, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1474.1939, 3297.1919, 133.7289, 5.30240, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000607, 43718, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1427.0523, 3321.1131, 188.4044, 5.30240, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000608, 43764, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1495.5464, 3296.4190, 124.0188, 5.49438, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000610, 43767, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1492.4806, 3297.9350, 124.0188, 4.90097, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000612, 43791, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1485.4375, 3225.9014, 103.7806, 0.60745, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000613, 43791, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1442.6330, 3269.3294, 103.2590, 3.67924, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000614, 43791, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1499.8438, 3248.2223, 103.9418, 0.45038, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000615, 43793, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1483.4160, 3250.6597, 128.5803, 2.14335, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000616, 43793, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1474.5842, 3256.7332, 178.6370, 2.14335, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000617, 43793, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1467.4359, 3260.9848, 178.6371, 2.14335, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000618, 43793, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1456.7066, 3269.3595, 128.6720, 2.14335, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000619, 43793, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1456.8333, 3271.6709, 128.8134, 2.14335, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000620, 43793, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1459.3826, 3265.4223, 128.4932, 2.14335, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000621, 43793, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1459.0556, 3270.6870, 128.7259, 2.14335, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000622, 43793, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1454.5951, 3268.3414, 128.6976, 2.14335, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000623, 43793, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1482.0791, 3252.5015, 128.5037, 2.14335, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000624, 43793, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1487.0177, 3255.4915, 128.8507, 2.14335, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000625, 43793, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1485.6775, 3251.2620, 128.6739, 2.14335, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000626, 43793, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1456.3020, 3266.3807, 128.5959, 2.14335, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000627, 43793, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1460.3295, 3268.7092, 128.5918, 2.14335, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000628, 43793, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1484.1297, 3253.4371, 128.6137, 2.14335, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000629, 43793, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1479.8220, 3275.0094, 126.0776, 2.14335, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(9000633, 43793, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, 0, 0, -1483.6124, 3255.8195, 128.6190, 2.14335, 7200, 0, 0, 0, 0, 0, 0, 0, 0, '', 0);

-- Deck gameobjects re-based from map 749 (rappel rope + 3x Incendiary Explosives)
DELETE FROM `gameobject` WHERE `guid` BETWEEN 9000503 AND 9000507;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `VerifiedBuild`) VALUES
(9000503, 204428, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, -1474.6670, 3255.4113, 178.5537, 2.14335, 0, 0, 0.878003, 0.478655, 7200, 255, 1, '', 0),
(9000504, 204458, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, -1494.8164, 3296.5706, 104.2867, 2.14335, 0, 0, 0.878003, 0.478655, 7200, 255, 1, '', 0),
(9000505, 204458, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, -1493.0065, 3297.7837, 104.2948, 2.14335, 0, 0, 0.878003, 0.478655, 7200, 255, 1, '', 0),
(9000506, 204458, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, -1496.3229, 3294.9650, 104.2769, 2.14335, 0, 0, 0.878003, 0.478655, 7200, 255, 1, '', 0);
-- Endgame deck combat spawns need fast respawn for the repeatable controller loop
UPDATE `creature` SET `spawntimesecs` = 120 WHERE `guid` BETWEEN 9000530 AND 9000633 AND `id` IN (42141, 43567);

-- Second deck rappel-rope GO (sniff lists two at deck level)
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `VerifiedBuild`) VALUES
(9000507, 204428, 654, 4714, 4714, 1, 0, 1, 191, 0, -1, -1467.12, 3260.64, 178.55, 2.14335, 0, 0, 0.878003, 0.478655, 7200, 255, 1, '', 0);

-- Tobias Mistmantle 38051 for the "Take Back What's Ours" Scythe vignette had zero
-- spawns; place him beside Lord Darius (guid 256293). Z/orientation: snap in-game.
DELETE FROM `creature` WHERE `guid` = 9000522;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild`) VALUES
(9000522, 38051, 654, 4714, 4841, 1, 0, 1, 186, 0, -1, 0, 0, -2072.50, 1279.50, -85.31, 5.76, 120, 0, 0, 1, 0, 0, 0, 0, 0, '', 0);

-- The retail moving Orc Gunship transport would instantiate the whole map-749 crew
-- on its own hull, duplicating the static Endgame set above - remove it.
DELETE FROM `transports` WHERE `entry` = 203428;
