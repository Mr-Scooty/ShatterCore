-- Don't Go Into the Light! (14239): Doc Zapnozzle is spawned over water, so
-- the core selects his swimming animation even though he is standing on the
-- barrel.  Prevent this only for the summoned instance while it is pinned in
-- place; his normal swimming capability is needed for the farewell movement.
DELETE FROM `smart_scripts` WHERE `entryorguid`=36608 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(36608,0,0,1,54,0,100,0,0,0,0,0,0,64,1,0,0,0,0,0,7,0,0,0,0,0,0,0,'Doc Zapnozzle - On Just Summoned - Store summoner'),
(36608,0,1,2,61,0,100,0,0,0,0,0,0,60,1,0,0,0,0,0,1,0,0,0,0,0,0,0,'Doc Zapnozzle - Linked - Disable gravity on the barrel'),
(36608,0,2,3,61,0,100,0,0,0,0,0,0,18,16384,0,0,0,0,0,1,0,0,0,0,0,0,0,'Doc Zapnozzle - Linked - Use standing animation on the barrel'),
(36608,0,3,0,61,0,100,0,0,0,0,0,0,80,3660800,0,0,0,0,0,1,0,0,0,0,0,0,0,'Doc Zapnozzle - Linked - Run revive vignette'),
(36608,0,4,5,20,0,100,0,14239,0,0,0,0,64,1,0,0,0,0,0,7,0,0,0,0,0,0,0,'Doc Zapnozzle - On Quest 14239 Rewarded - Store player'),
(36608,0,5,0,61,0,100,0,0,0,0,0,0,80,3660801,0,0,0,0,0,1,0,0,0,0,0,0,0,'Doc Zapnozzle - Linked - Run farewell');

-- Retail records spell 69022 as a normal 3000 ms cast.  The triggered flag
-- made both casts instant and suppressed their expected casting presentation.
UPDATE `smart_scripts`
SET `action_param2`=0
WHERE `entryorguid`=3660800 AND `source_type`=9 AND `id` IN (5,7)
  AND `action_type`=11 AND `action_param1`=69022;

-- Restore swimming before Doc leaves the barrel, then retain the previously
-- established farewell timings and path.
DELETE FROM `smart_scripts` WHERE `entryorguid`=3660801 AND `source_type`=9;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(3660801,9,0,0,0,0,100,0,0,0,0,0,0,1,5,0,0,0,0,0,12,1,0,0,0,0,0,0,'Doc farewell - Say 5 (You made the right choice)'),
(3660801,9,1,0,0,0,100,0,5200,5200,0,0,0,1,6,0,0,0,0,0,12,1,0,0,0,0,0,0,'Doc farewell - Say 6 (See you on the shore)'),
(3660801,9,2,0,0,0,100,0,3000,3000,0,0,0,19,16384,0,0,0,0,0,1,0,0,0,0,0,0,0,'Doc farewell - Restore swimming before leaving the barrel'),
(3660801,9,3,0,0,0,100,0,0,0,0,0,0,60,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Doc farewell - Re-enable gravity'),
(3660801,9,4,0,0,0,100,0,100,100,0,0,0,69,2,0,1,0,0,0,8,0,0,0,540.549,3262.33,-0.6787,0,'Doc farewell - Wade off the barrel'),
(3660801,9,5,0,0,0,100,0,1400,1400,0,0,0,69,3,0,1,0,0,0,8,0,0,0,579.017,3162.55,-0.6787,0,'Doc farewell - Swim toward the shore camp'),
(3660801,9,6,0,0,0,100,0,11000,11000,0,0,0,41,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Doc farewell - Despawn');
