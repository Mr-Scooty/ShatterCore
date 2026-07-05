-- Quest 12678 "If Chaos Drives, Let Suffering Hold the Reins": retail pacing for the Scarlet Miner.
-- The miner previously fired face/say/cast/walk in a single tick via linked events; move the
-- sequence into a timed actionlist (arrive -> 3s -> say -> 3s -> chain + run), matching sniffs.

DELETE FROM `smart_scripts` WHERE `entryorguid` = 28841 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(28841,0,0,1,54,0,100,0,0,0,0,0,0,59,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Scarlet Miner - On Just Summoned - Set Run Off'),
(28841,0,1,0,61,0,100,0,0,0,0,0,0,69,0,0,0,0,0,0,8,0,0,0,2386.67,-5900.59,108.576,0,'Scarlet Miner - Linked - Move To Mine Car'),
(28841,0,2,0,34,0,100,0,8,0,0,0,0,80,2884103,0,2,0,0,0,1,0,0,0,0,0,0,0,'Scarlet Miner - On Movement Inform - Run Drag Sequence'),
(28841,0,3,0,58,0,100,0,0,0,0,0,0,80,2884102,0,2,0,0,0,1,0,0,0,0,0,0,0,'Scarlet Miner - On Waypoint Ended - Run Cleanup');

DELETE FROM `smart_scripts` WHERE `entryorguid` = 2884103 AND `source_type` = 9;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(2884103,9,0,0,0,0,100,0,0,0,0,0,0,66,0,0,0,0,0,0,23,0,0,0,0,0,0,0,'Scarlet Miner - Drag Sequence - Face Mine Car'),
(2884103,9,1,0,0,0,100,0,3000,3000,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Scarlet Miner - Drag Sequence - Say Line 0'),
(2884103,9,2,0,0,0,100,0,3000,3000,0,0,0,11,52465,1,0,0,0,0,23,0,0,0,0,0,0,0,'Scarlet Miner - Drag Sequence - Cast Drag Mine Cart'),
(2884103,9,3,0,0,0,100,0,0,0,0,0,0,59,1,0,0,0,0,0,1,0,0,0,0,0,0,0,'Scarlet Miner - Drag Sequence - Set Run On'),
(2884103,9,4,0,0,0,100,0,0,0,0,0,0,142,2884100,0,0,0,0,0,1,0,0,0,0,0,0,0,'Scarlet Miner - Drag Sequence - Start waypoint_data Path');

-- Remove orphaned rows from the abandoned SmartAI-escort attempt (`waypoints` table).
DELETE FROM `waypoints` WHERE `entry` IN (2884100, 2884101);
