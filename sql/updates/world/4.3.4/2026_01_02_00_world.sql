-- Crazed Leper Gnomes fighting S.A.F.E. Operatives in New Tinkertown
SET @CGUID := 9000000;

DELETE FROM `creature` WHERE `guid` BETWEEN @CGUID+0 AND @CGUID+3;
DELETE FROM `creature_addon` WHERE `guid` BETWEEN @CGUID+0 AND @CGUID+3;

INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild`) VALUES
(@CGUID+0, 46391, 0, 1, 5495, 1, 0, 1, 0, 0, -1, 0, 0, -4987.0967, 838.5954, 276.41562, 1.3961974, 120, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+1, 46391, 0, 1, 5495, 1, 0, 1, 0, 0, -1, 0, 0, -4980.3125, 837.8875, 276.48666, 1.2304425, 120, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+2, 46391, 0, 1, 5495, 1, 0, 1, 0, 0, -1, 0, 0, -4982.892, 838.112, 276.519, 1.4661742, 120, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+3, 46391, 0, 1, 5495, 1, 0, 1, 0, 0, -1, 0, 0, -4989.5664, 839.0433, 276.42877, 1.7279886, 120, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595);

UPDATE `creature_template` SET `faction` = 14, `AIName` = 'SmartAI', `ScriptName` = '' WHERE `entry` = 46391;
UPDATE `creature_template` SET `spell1` = 85756, `spell2` = 74413 WHERE `entry` = 45847;

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND `entryorguid` IN (-(@CGUID+0), -(@CGUID+1), -(@CGUID+2), -(@CGUID+3));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(-(@CGUID+0), 0, 0, 0, 63, 0, 100, 0, 0, 0, 0, 0, 49, 0, 0, 0, 0, 0, 0, 19, 45847, 15, 0, 0, 0, 0, 0, 'Crazed Leper Gnome - On just created - Attack closest S.A.F.E. Operative'),
(-(@CGUID+1), 0, 0, 0, 63, 0, 100, 0, 0, 0, 0, 0, 49, 0, 0, 0, 0, 0, 0, 19, 45847, 15, 0, 0, 0, 0, 0, 'Crazed Leper Gnome - On just created - Attack closest S.A.F.E. Operative'),
(-(@CGUID+2), 0, 0, 0, 63, 0, 100, 0, 0, 0, 0, 0, 49, 0, 0, 0, 0, 0, 0, 19, 45847, 15, 0, 0, 0, 0, 0, 'Crazed Leper Gnome - On just created - Attack closest S.A.F.E. Operative'),
(-(@CGUID+3), 0, 0, 0, 63, 0, 100, 0, 0, 0, 0, 0, 49, 0, 0, 0, 0, 0, 0, 19, 45847, 15, 0, 0, 0, 0, 0, 'Crazed Leper Gnome - On just created - Attack closest S.A.F.E. Operative');
