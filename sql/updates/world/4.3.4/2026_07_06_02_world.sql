-- Quest 12678: the Mine Car floated above the terrain while FOLLOWing the Scarlet Miner down
-- the hill (follow splines chord over the steep descent). Drive the cart along its own copy of
-- the miner's sniffed ground path instead, started 1s behind so it trails on the chain.

-- Cart path = exact copy of the miner's path 2884100 (sniffed ground positions).
DELETE FROM `waypoint_data` WHERE `id` = 2881700;
INSERT INTO `waypoint_data` (`id`,`point`,`position_x`,`position_y`,`position_z`,`orientation`,`velocity`,`delay`,`smoothTransition`,`move_type`,`action`,`action_chance`,`wpguid`)
SELECT 2881700, `point`, `position_x`, `position_y`, `position_z`, `orientation`, `velocity`, `delay`, `smoothTransition`, `move_type`, `action`, `action_chance`, `wpguid`
FROM `waypoint_data` WHERE `id` = 2884100;

-- Drop the cart's Spellhit -> Follow rule; 52465 remains as the chain visual only.
DELETE FROM `smart_scripts` WHERE `entryorguid` = 28817 AND `source_type` = 0 AND `id` = 2;

-- Start the cart on its path 1s after the miner sets off (drag sequence actionlist).
DELETE FROM `smart_scripts` WHERE `entryorguid` = 2884103 AND `source_type` = 9 AND `id` = 5;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(2884103,9,5,0,0,0,100,0,1000,1000,0,0,0,142,2881700,0,0,0,0,0,23,0,0,0,0,0,0,0,'Scarlet Miner - Drag Sequence - Start Mine Car waypoint_data Path');
