-- The Pride of Kezan (25066): P4-sniff fixes.
-- Sassy's accept bark ("I forget... did you take flying lessons back on Kezan?") existed
-- untriggered; the stealth fighters now shoot back - their seat-1 machine-gun accessory
-- (40785) fires 73485 at the nearest flying goblin (35 casts sniffed).
DELETE FROM `smart_scripts` WHERE `entryorguid`=38387 AND `source_type`=0 AND `id`=4;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(38387,0,4,0,19,0,100,0,25066,0,0,0,1,4,0,0,0,0,0,7,0,0,0,0,0,0,0,'Sassy Hardwrench - On Quest Accepted (The Pride of Kezan) - Say Line 4');

UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=40785;
DELETE FROM `smart_scripts` WHERE `entryorguid`=40785 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(40785,0,0,0,1,0,100,0,4000,8000,5000,9000,11,73485,2,0,0,0,0,21,120,0,0,0,0,0,0,'Stealth Fighter Machine Gun - OOC - Return fire at nearest flyer');
