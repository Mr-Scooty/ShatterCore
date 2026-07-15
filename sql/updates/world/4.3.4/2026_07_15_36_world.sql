-- Don't Go Into the Light! (14239): Doc Zapnozzle would not stand on his barrel.
-- The barrel is client-side scenery with no server collision, and the barrel floats in
-- shallow water - the per-tick swim check (SetSwim) flipped the Doc into the swimming
-- animation the moment his hop landed, leaving him bobbing beside the barrel instead of
-- standing on it. Deny him swimming and pin him (gravity off) once the hop lands; gravity
-- is restored when he wades off during the farewell.
DELETE FROM `creature_template_movement` WHERE `CreatureId`=36608;
INSERT INTO `creature_template_movement` (`CreatureId`,`Ground`,`Swim`,`Flight`,`Rooted`) VALUES
(36608,1,0,0,0);

DELETE FROM `smart_scripts` WHERE `entryorguid`=36608 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(36608,0,0,1,54,0,100,0,0,0,0,0,64,1,0,0,0,0,0,7,0,0,0,0,0,0,0,'Doc Zapnozzle - On Just Summoned - Store summoner'),
(36608,0,1,2,61,0,100,0,0,0,0,0,59,1,0,0,0,0,0,1,0,0,0,0,0,0,0,'Doc Zapnozzle - Linked - Set run'),
(36608,0,2,0,61,0,100,0,0,0,0,0,69,1,0,1,0,0,0,8,0,0,0,538.49,3271.2,-0.6523,0,'Doc Zapnozzle - Linked - Run down to the shore'),
(36608,0,3,4,34,0,100,0,8,1,0,0,97,8,6,0,0,0,0,8,0,0,0,537.135,3272.25,0.18,0,'Doc Zapnozzle - On Point 1 Reached - Hop onto the barrel'),
(36608,0,4,5,61,0,100,0,0,0,0,0,60,1,0,0,0,0,0,1,0,0,0,0,0,0,0,'Doc Zapnozzle - Linked - Pin onto the barrel (gravity off)'),
(36608,0,5,0,61,0,100,0,0,0,0,0,80,3660800,0,0,0,0,0,1,0,0,0,0,0,0,0,'Doc Zapnozzle - Linked - Run revive vignette'),
(36608,0,6,7,20,0,100,0,14239,0,0,0,64,1,0,0,0,0,0,7,0,0,0,0,0,0,0,'Doc Zapnozzle - On Quest 14239 Rewarded - Store player'),
(36608,0,7,0,61,0,100,0,0,0,0,0,80,3660801,0,0,0,0,0,1,0,0,0,0,0,0,0,'Doc Zapnozzle - Linked - Run farewell');

DELETE FROM `smart_scripts` WHERE `entryorguid`=3660801 AND `source_type`=9;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(3660801,9,0,0,0,0,100,0,0,0,0,0,1,5,0,0,0,0,0,12,1,0,0,0,0,0,0,'Doc farewell - Say 5 (You made the right choice)'),
(3660801,9,1,0,0,0,100,0,5200,5200,0,0,1,6,0,0,0,0,0,12,1,0,0,0,0,0,0,'Doc farewell - Say 6 (See you on the shore)'),
(3660801,9,2,0,0,0,100,0,3000,3000,0,0,60,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Doc farewell - Unpin from the barrel (gravity on)'),
(3660801,9,3,0,0,0,100,0,100,100,0,0,69,2,0,1,0,0,0,8,0,0,0,540.549,3262.33,-0.6787,0,'Doc farewell - Wade off the barrel'),
(3660801,9,4,0,0,0,100,0,1400,1400,0,0,69,3,0,1,0,0,0,8,0,0,0,579.017,3162.55,-0.6787,0,'Doc farewell - Swim toward the shore camp'),
(3660801,9,5,0,0,0,100,0,11000,11000,0,0,41,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Doc farewell - Despawn');
