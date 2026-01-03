-- Sanitron 500 decontamination event
SET @PATH := 4618500;

DELETE FROM `waypoint_data_addon` WHERE `PathID` = @PATH;
DELETE FROM `waypoint_data` WHERE `id` = @PATH;
INSERT INTO `waypoint_data` (`id`, `point`, `position_x`, `position_y`, `position_z`, `orientation`, `velocity`, `delay`, `smoothTransition`, `move_type`) VALUES
(@PATH, 0, -5173.6133, 726.069, 290.88895, NULL, 2.5, 3000, 0, 1),
(@PATH, 1, -5174.34, 718.14655, 289.9155, NULL, 2.5, 5000, 0, 1),
(@PATH, 2, -5174.6787, 707.457, 291.69577, NULL, 2.5, 4000, 0, 1),
(@PATH, 3, -5174.837, 702.4595, 291.69577, NULL, 2.5, 3000, 0, 1),
(@PATH, 4, -5174.837, 702.4595, 287.4155, NULL, 2.5, 0, 0, 1);

DELETE FROM `creature_text` WHERE `CreatureID` = 46185;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `comment`) VALUES
(46185, 0, 0, 'Commencing decontamination sequence...', 12, 0, 100, 0, 0, 0, 46323, 'Sanitron 500'),
(46185, 1, 0, 'Decontamination complete. Standby for delivery.', 12, 0, 100, 0, 0, 0, 46324, 'Sanitron 500'),
(46185, 2, 0, 'Warning, system overload. Malfunction imminent!', 12, 0, 100, 0, 0, 0, 46325, 'Sanitron 500');

DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 46185;
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(46185, 86106, 0, 0);

UPDATE `creature_template` SET `ScriptName` = 'npc_sanitron_500' WHERE `entry` = 46185;
UPDATE `creature_template` SET `HoverHeight` = 2.5 WHERE `entry` = 46185;
UPDATE `creature` SET `position_z` = `position_z` + 1.49023 WHERE `id` = 46185 AND `map` = 0 AND `areaId` = 5495;
-- Adjust Sanitron 500 spawn height to match sniffed Z values
UPDATE `creature_template` SET `HoverHeight` = 2.5 WHERE `entry` = 46185;
UPDATE `creature` SET `position_z` = `position_z` + 1.49023
WHERE `id` = 46185 AND `map` = 0 AND `position_z` < 288.0;
-- Fix Sanitron 500 spawn positions/heights to match sniff data
UPDATE `creature`
SET `position_x` = -5173.01, `position_y` = 734.88, `position_z` = 288.88733, `orientation` = 4.660028934
WHERE `id` = 46185 AND `map` = 0
  AND ABS(`position_x` - -5173.01) < 0.1 AND ABS(`position_y` - 734.88) < 0.1;

UPDATE `creature`
SET `position_x` = -5177.83, `position_y` = 735.214, `position_z` = 288.94135, `orientation` = 4.660028934
WHERE `id` = 46185 AND `map` = 0
  AND ABS(`position_x` - -5177.83) < 0.1 AND ABS(`position_y` - 735.214) < 0.1;

UPDATE `creature`
SET `position_x` = -5168.48, `position_y` = 734.568, `position_z` = 288.78833, `orientation` = 4.660028934
WHERE `id` = 46185 AND `map` = 0
  AND ABS(`position_x` - -5168.48) < 0.1 AND ABS(`position_y` - 734.568) < 0.1;

-- Keep Sanitron 500 stationary until a player boards it
UPDATE `creature`
SET `MovementType` = 0, `currentwaypoint` = 0
WHERE `id` = 46185 AND `map` = 0;

UPDATE `creature_addon` ca
JOIN `creature` c ON c.`guid` = ca.`guid`
SET ca.`waypointPathId` = 0
WHERE c.`id` = 46185 AND c.`map` = 0;
-- Ensure Sanitron 500 uses DisableGravity like Target Acquisition Device
DELETE FROM `creature_template_movement` WHERE `CreatureId` = 46185;
INSERT INTO `creature_template_movement`
(`CreatureId`, `Ground`, `Swim`, `Flight`, `Rooted`, `Random`, `InteractionPauseTimer`)
VALUES
(46185, 0, 0, 1, 0, 0, NULL);
-- Require Decontamination quest for Sanitron 500 spellclick
DELETE FROM `conditions`
WHERE `SourceTypeOrReferenceId` = 18 AND `SourceGroup` = 46185 AND `SourceEntry` = 86106;

INSERT INTO `conditions`
(`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `ConditionTypeOrReference`, `ConditionValue1`, `ScriptName`, `Comment`)
VALUES
(18, 46185, 86106, 9, 27635, '', 'Required quest active for Sanitron 500 spellclick');
