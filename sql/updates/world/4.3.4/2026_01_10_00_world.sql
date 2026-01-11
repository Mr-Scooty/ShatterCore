-- Shannox trash counter and spawn
SET @CGUID := 9000100;
SET @GROUP_ID := 460;
SET @COUNTER_ID := 1;
SET @KILL_COUNT_FIRST := 25;
SET @KILL_COUNT_SECOND := 45;
SET @KILL_COUNT_SPAWN := @KILL_COUNT_SECOND + 10;

SET @PATH_SHANNOX := (@CGUID+0) * 10;
SET @PATH_RAGEFACE := (@CGUID+1) * 10;
SET @PATH_RIPLIMB := (@CGUID+2) * 10;
SET @CTRL_GUID := @CGUID+3;

DELETE FROM `creature` WHERE `guid` IN (339184, 339185) AND `map` = 720;
DELETE FROM `creature_addon` WHERE `guid` IN (339184, 339185) AND `waypointPathId` >= 0;

DELETE FROM `creature` WHERE `guid` BETWEEN @CGUID AND @CGUID+3 AND `map` = 720;
DELETE FROM `creature_addon` WHERE `guid` BETWEEN @CGUID AND @CGUID+3 AND `waypointPathId` >= 0;

DELETE FROM `spawn_group` WHERE `groupId` = @GROUP_ID AND `spawnType` >= 0;
DELETE FROM `spawn_group` WHERE `spawnId` IN (@CGUID+0, @CGUID+1, @CGUID+2) AND `groupId` = @GROUP_ID;
DELETE FROM `spawn_group_template` WHERE `groupId` = @GROUP_ID AND `groupFlags` >= 0;
INSERT INTO `spawn_group_template` (`groupId`, `groupName`, `groupFlags`) VALUES
(@GROUP_ID, 'Firelands - Shannox Pack', 4);

INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild`) VALUES
(@CGUID+0, 53691, 720, 5723, 5767, 15, 0, 1, 169, 0, -1, 0, 0, -42.21528, -61.317707, 57.758217, 0, 7200, 0, 0, 1, 0, 2, 0, 0, 0, '', 0),
(@CGUID+1, 53695, 720, 5723, 5767, 15, 0, 1, 169, 0, -1, 0, 0, -42.21528, -64.317707, 57.758217, 0, 7200, 0, 0, 1, 0, 0, 0, 0, 0, '', 0),
(@CGUID+2, 53694, 720, 5723, 5767, 15, 0, 1, 169, 0, -1, 0, 0, -42.21528, -58.317707, 57.758217, 0, 7200, 0, 0, 1, 0, 0, 0, 0, 0, '', 0),
(@CTRL_GUID, 53910, 720, 5723, 5767, 15, 0, 1, 169, 0, -1, 0, 0, 47.87847, -66.979164, 53.731026, 0, 7200, 0, 0, 1, 0, 0, 0, 0, 0, '', 0);

INSERT INTO `spawn_group` (`groupId`, `spawnType`, `spawnId`) VALUES
(@GROUP_ID, 0, @CGUID+0),
(@GROUP_ID, 0, @CGUID+1),
(@GROUP_ID, 0, @CGUID+2);

INSERT INTO `creature_addon` (`guid`, `waypointPathId`) VALUES
(@CGUID+0, @PATH_SHANNOX);

DELETE FROM `creature_formations` WHERE `LeaderGUID` = @CGUID+0 AND `MemberGUID` IN (@CGUID+0, @CGUID+1, @CGUID+2);
INSERT INTO `creature_formations` (`LeaderGUID`, `MemberGUID`, `FollowDistance`, `FollowAngle`, `GroupAI`) VALUES
(@CGUID+0, @CGUID+0, 0, 0, 515),
(@CGUID+0, @CGUID+1, 5, 90, 515),
(@CGUID+0, @CGUID+2, 5, 270, 515);

DELETE FROM `waypoint_data_addon` WHERE `PathID` IN (@PATH_SHANNOX, @PATH_RAGEFACE, @PATH_RIPLIMB) AND `PointID` >= 0;
DELETE FROM `waypoint_data` WHERE `id` IN (@PATH_SHANNOX, @PATH_RAGEFACE, @PATH_RIPLIMB) AND `point` >= 0;
INSERT INTO `waypoint_data` (`id`, `point`, `position_x`, `position_y`, `position_z`, `orientation`, `velocity`, `delay`, `smoothTransition`, `move_type`) VALUES
(@PATH_RAGEFACE, 0, -6.2267265, -66.65927, 55.63297, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 1, -15.7109375, -65.25586, 56.493828, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 2, -25.570755, -64.35777, 56.966774, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 3, -35.16742, -62.604965, 57.576523, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 4, -35.68659, -62.510143, 57.620167, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 5, -55.60608, -65.511475, 57.612022, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 6, -55.48314, -62.19862, 57.6971, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 7, -55.13504, -52.818283, 57.572906, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 8, -58.692997, -41.97979, 57.493073, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 9, -55.94545, -32.603535, 57.19252, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 10, -51.36729, -23.852285, 57.03393, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 11, -49.634174, -14.34301, 57.0273, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 12, -43.803345, -5.0032253, 55.364857, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 13, -44.203796, 4.7607765, 56.546238, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 14, -45.704174, 14.401142, 56.94192, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 15, -45.43734, 24.15974, 56.874664, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 16, -55.4976, 37.369427, 55.52649, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 17, -49.75995, 45.260323, 55.625286, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 18, -43.81483, 52.97959, 55.786755, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 19, -38.26172, 60.86328, 56.375244, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 20, -32.55754, 68.946754, 56.625534, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 21, -26.988533, 76.870056, 56.659904, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 22, -21.472391, 85.55632, 56.83496, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 23, -17.137085, 94.04087, 56.75415, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 24, -10.087327, 96.797295, 56.502895, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 25, -5.6051617, 105.4932, 55.47238, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 26, -1.15205, 114.13276, 53.187634, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 27, 3.2321577, 122.63865, 52.792282, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 28, 14.160032, 129.56506, 50.594166, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 29, 16.182919, 139.1904, 47.43356, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 30, 14.762338, 148.92331, 46.057106, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 31, 13.341371, 158.65886, 45.776176, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 32, 20.258184, 173.536, 46.15793, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 33, 12.077705, 178.85617, 46.281178, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 34, 3.8699732, 184.19406, 46.77543, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 35, -3.0052433, 203.15076, 47.542336, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 36, -11.991558, 199.2047, 47.702343, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 37, -20.967857, 195.26201, 47.595425, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 38, -29.851562, 191.3711, 47.254864, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 39, -38.737106, 187.39386, 46.84548, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 40, -47.11693, 181.90428, 46.322235, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 41, -56.509037, 179.32411, 45.74009, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 42, -66.65462, 167.56651, 45.377903, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 43, -75.53326, 171.65335, 44.810425, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 44, -84.404106, 175.73584, 44.483986, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 45, -93.22319, 179.86345, 44.79511, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 46, -100.5793, 188.58392, 45.349663, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 47, -110.429016, 189.42636, 46.071487, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 48, -120.20618, 190.2626, 46.157642, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 49, -131.42673, 199.77855, 46.157642, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 50, -140.11136, 195.05724, 46.157642, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 51, -148.8031, 190.33206, 46.15764, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 52, -156.51534, 184.00015, 46.157665, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 53, -165.57417, 180.35538, 46.195583, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 54, -174.63605, 176.71153, 46.349533, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 55, -186.20251, 177.45059, 46.59192, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 56, -193.87404, 171.37059, 46.745876, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 57, -201.53484, 165.29544, 46.918903, NULL, 0, 0, 0, 0),
(@PATH_RAGEFACE, 58, -216.64597, 152.0003, 47.91211, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 0, -8.268386, -72.949486, 56.585144, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 1, -38.553593, -82.08624, 56.761528, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 2, -48.83876, -80.43881, 56.135742, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 3, -58.435425, -78.68601, 56.107388, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 4, -58.954594, -78.591194, 56.33289, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 5, -74.85065, -44.78356, 55.261, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 6, -74.72771, -41.470695, 54.988354, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 7, -74.37961, -32.09036, 54.330917, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 8, -72.25864, -17.160973, 54.53801, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 9, -69.508606, -7.7833595, 54.92053, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 10, -67.453094, -0.5875797, 55.64887, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 11, -65.71762, 8.923332, 56.348083, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 12, -64.60612, 14.160405, 56.23725, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 13, -65.00656, 23.924408, 55.46978, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 14, -65.150024, 34.94034, 54.979607, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 15, -64.883194, 44.698936, 54.055588, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 16, -59.912346, 65.30704, 55.67306, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 17, -54.17525, 73.197845, 58.22441, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 18, -48.65409, 80.8468, 58.61508, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 19, -43.032875, 88.83058, 57.234573, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 20, -37.41944, 96.81002, 57.07687, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 21, -31.25, 104.166016, 57.24156, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 22, -30.181993, 112.466225, 55.87258, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 23, -27.815554, 109.33973, 56.96692, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 24, -20.351461, 110.00546, 54.51027, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 25, -14.867489, 115.52276, 53.343006, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 26, -6.5223184, 118.464165, 53.165226, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 27, -5.3822203, 149.5792, 47.098297, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 28, 1.3471045, 134.41342, 50.24428, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 29, -6.4959183, 156.0922, 46.020737, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 30, -7.9165, 165.8251, 45.070534, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 31, -9.3125, 175.4375, 45.254906, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 32, -7.4118977, 167.67369, 44.959927, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 33, -15.592377, 172.99387, 45.24685, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 34, -23.80011, 178.33176, 45.431816, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 35, -13.27487, 176.79672, 45.372185, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 36, -22.26004, 172.85022, 45.22741, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 37, -31.236341, 168.90753, 44.825676, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 38, -40.159283, 164.98828, 44.35076, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 39, -49.083824, 161.07, 44.22637, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 40, -61.229774, 157.3925, 44.571274, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 41, -70.68713, 154.85, 44.383884, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 42, -93.18461, 157.7604, 44.41792, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 43, -84.25997, 162.4061, 43.933544, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 44, -110.93377, 165.92886, 45.328384, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 45, -119.72113, 169.97105, 45.248672, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 46, -122.21091, 170.36104, 45.026672, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 47, -132.06062, 171.20348, 44.783115, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 48, -141.83777, 172.03972, 44.75879, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 49, -139.44557, 172.65479, 44.758804, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 50, -148.1302, 167.93347, 44.758797, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 51, -149.01083, 170.94029, 44.898872, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 52, -167.60764, 157.98167, 45.2609, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 53, -176.66872, 154.33786, 45.938503, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 54, -185.7306, 150.69403, 45.814003, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 55, -189.36145, 149.39635, 45.886337, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 56, -190.58018, 152.81851, 45.612347, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 57, -204.77838, 137.19777, 46.255146, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 58, -212.92413, 152.33453, 47.687557, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 59, -213.90918, 150.48445, 47.832108, NULL, 0, 0, 0, 0),
(@PATH_RIPLIMB, 60, -215.60146, 157.27818, 47.57711, NULL, 0, 0, 0, 0),
(@PATH_SHANNOX, 0, -42.21528, -61.317707, 57.758217, NULL, 0, 0, 0, 0),
(@PATH_SHANNOX, 1, -55.36111, -58.916668, 57.692207, NULL, 0, 0, 0, 0),
(@PATH_SHANNOX, 2, -54.144096, -26.463541, 56.941048, NULL, 0, 0, 0, 0),
(@PATH_SHANNOX, 3, -48.08507, -5.861111, 55.799377, NULL, 0, 0, 0, 0),
(@PATH_SHANNOX, 4, -44.5625, 13.506945, 56.932823, NULL, 0, 0, 0, 0),
(@PATH_SHANNOX, 5, -45.265625, 30.439236, 56.243366, NULL, 0, 0, 0, 0),
(@PATH_SHANNOX, 6, -44.657986, 52.27778, 55.64666, NULL, 0, 0, 0, 0),
(@PATH_SHANNOX, 7, -33.59375, 67.49653, 56.624657, NULL, 0, 0, 0, 0),
(@PATH_SHANNOX, 8, -15.487847, 93.23264, 56.77095, NULL, 0, 0, 0, 0),
(@PATH_SHANNOX, 9, -1.0173612, 114.3941, 53.19801, NULL, 0, 0, 0, 0),
(@PATH_SHANNOX, 10, 14.203125, 143.92361, 46.590935, NULL, 0, 0, 0, 0),
(@PATH_SHANNOX, 11, 10.216797, 180.0664, 46.36843, NULL, 0, 0, 0, 0),
(@PATH_SHANNOX, 12, -16.32639, 197.30035, 47.697155, NULL, 0, 0, 0, 0),
(@PATH_SHANNOX, 13, -58.157986, 178.9323, 45.582504, NULL, 0, 0, 0, 0),
(@PATH_SHANNOX, 14, -79.11632, 173.30208, 44.625603, NULL, 0, 0, 0, 0),
(@PATH_SHANNOX, 15, -115.04514, 189.82118, 46.155876, NULL, 0, 0, 0, 0),
(@PATH_SHANNOX, 16, -145.02605, 192.38542, 46.157642, NULL, 0, 0, 0, 0),
(@PATH_SHANNOX, 17, -171.64062, 177.91667, 46.285553, NULL, 0, 0, 0, 0),
(@PATH_SHANNOX, 18, -199.98611, 166.52257, 46.877422, NULL, 0, 0, 0, 0),
(@PATH_SHANNOX, 19, -225.61632, 146.17708, 48.627422, NULL, 0, 0, 0, 0);

UPDATE `creature_template` SET `AIName` = 'SmartAI', `ScriptName` = '' WHERE `entry` = 53910;
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` IN (52672, 53094, 53095, 53096, 53102, 53115, 53116, 53119, 53120, 53121, 53127, 53128, 53129, 53130, 53134, 53141, 53167, 53185, 53187, 53188, 53206, 53209, 53222, 53223, 53224, 53244, 53545, 53575, 53616, 53617, 53619, 53631, 53635, 53639, 53640, 53642, 53648, 53732, 53901, 53914, 54073, 54143, 54161, 54274, 54275, 54276, 54277);

DELETE FROM `creature_text` WHERE `CreatureID` = 53910 AND `GroupID` IN (0, 1) AND `ID` = 0;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(53910, 0, 0, '|TInterface\\Icons\\inv_misc_horn_03.blp:20|tAs the creatures of the Firelands fall, a huntsman\'s horn sounds in the distance.', 41, 0, 100, 0, 0, 6423, 0, 3, 'Shannox Controller - Horn 1'),
(53910, 1, 0, '|TInterface\\Icons\\inv_misc_horn_03.blp:20|tThe hunting horn sounds again, nearer and more urgently.', 41, 0, 100, 0, 0, 6423, 0, 3, 'Shannox Controller - Horn 2');

DELETE FROM `creature_text` WHERE `CreatureID` = 53691 AND `GroupID` = 0 AND `ID` = 0;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(53691, 0, 0, 'Yes, I smell them too, Riplimb. Outsiders encroach on the Firelord\'s private grounds. Find their trail. Find them for me, that I may dispense punishment!', 14, 0, 100, 0, 0, 24584, 0, 3, 'Shannox - Spawn');

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND `entryorguid` = 53910 AND `id` IN (0, 1, 2, 3, 4);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND `entryorguid` IN (52672, 53094, 53095, 53096, 53102, 53115, 53116, 53119, 53120, 53121, 53127, 53128, 53129, 53130, 53134, 53141, 53167, 53185, 53187, 53188, 53206, 53209, 53222, 53223, 53224, 53244, 53545, 53575, 53616, 53617, 53619, 53631, 53635, 53639, 53640, 53642, 53648, 53732, 53901, 53914, 54073, 54143, 54161, 54274, 54275, 54276, 54277) AND `id` = 90;

INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(52672, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(53094, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(53095, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(53096, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(53102, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(53115, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(53116, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(53119, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(53120, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(53121, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(53127, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(53128, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(53129, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(53130, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(53134, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(53141, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(53167, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(53185, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(53187, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(53188, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(53206, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(53209, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(53222, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(53223, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(53224, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(53244, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(53545, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(53575, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(53616, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(53617, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(53619, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(53631, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(53635, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(53639, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(53640, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(53642, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(53648, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(53732, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(53901, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(53914, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(54073, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(54143, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(54161, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(54274, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(54275, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(54276, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(54277, 0, 90, 0, 6, 0, 100, 0, 0, 0, 0, 0, 63, @COUNTER_ID, 1, 0, 0, 0, 0, 10, @CTRL_GUID, 53910, 0, 0, 0, 0, 0, 'Firelands Trash - On death - Increment Shannox counter'),
(53910, 0, 0, 0, 77, 0, 100, 0, @COUNTER_ID, @KILL_COUNT_FIRST, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shannox Controller - Counter set - Horn 1'),
(53910, 0, 1, 0, 77, 0, 100, 0, @COUNTER_ID, @KILL_COUNT_SECOND, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shannox Controller - Counter set - Horn 2'),
(53910, 0, 2, 3, 77, 0, 100, 0, @COUNTER_ID, @KILL_COUNT_SPAWN, 0, 0, 131, @GROUP_ID, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Shannox Controller - Counter set - Spawn Shannox pack'),
(53910, 0, 3, 0, 61, 0, 100, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 10, @CGUID+0, 53691, 0, 0, 0, 0, 0, 'Shannox Controller - Link - Spawn yell'),
(53910, 0, 4, 0, 25, 0, 100, 0, 0, 0, 0, 0, 48, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shannox Controller - On reset - Set active');
