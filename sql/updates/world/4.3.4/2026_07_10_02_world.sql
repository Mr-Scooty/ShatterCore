-- Kezan stadium: three citizen waypoint paths carry a literal duplicate point
-- (sniff import artifact encoding a pause as a zero-length hop). Every lap the
-- zero-length segment failed MoveSplineInitArgs::_checkPathLengths and logged
-- a Validate error. Fold the second row's delay into the first and drop it.

-- Kezan Citizen 35075 (guid 251985), path 2519850: points 4/5 identical
UPDATE `waypoint_data` SET `delay` = 13616 WHERE `id` = 2519850 AND `point` = 4;
DELETE FROM `waypoint_data` WHERE `id` = 2519850 AND `point` = 5;

-- Kezan Citizen 35063 (guid 251997), path 2519970: points 3/4 identical
UPDATE `waypoint_data` SET `delay` = 10681 WHERE `id` = 2519970 AND `point` = 3;
DELETE FROM `waypoint_data` WHERE `id` = 2519970 AND `point` = 4;

-- Kezan Citizen 35075 (guid 252143), path 2521430: points 4/5 identical
UPDATE `waypoint_data` SET `delay` = 9221 WHERE `id` = 2521430 AND `point` = 4;
DELETE FROM `waypoint_data` WHERE `id` = 2521430 AND `point` = 5;
UPDATE `waypoint_data` SET `point` = `point` - 1 WHERE `id` = 2521430 AND `point` > 5 ORDER BY `point` ASC;

-- Fourth and Goal: Deathwing's Attack (66858) is a speed-70 missile that
-- retail aims at an ELM trigger on top of Mount Kajaro (P2 sniff: entry 42196
-- at -8821.41, 1482.6, 469.67) so the flame streaks from Deathwing to the
-- volcano. No such spawn existed; add it in the default Kezan phase.
DELETE FROM `creature` WHERE `guid` = 9000652;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild`) VALUES
(9000652, 42196, 648, 4737, 4737, 1, 0, 1, 169, 0, -1, 0, 0, -8821.41, 1482.6, 469.67, 0, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 15595); -- Mount Kajaro breath target (Fourth and Goal)
