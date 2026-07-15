-- Gallywix mine arc ambience/combat kits (P4 sniff; all creatures had no AI).
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry` IN (38515,38513,38514,39193,39354,39376) AND `ScriptName`='';
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (38515,38513,38514,39193,39354,39376) AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(38515,0,0,0,1,0,100,0,5000,15000,25000,35000,11,12550,2,0,0,0,0,1,0,0,0,0,0,0,0,'Maxx Avalanche - OOC - Lightning Shield'),
(38515,0,1,0,1,0,100,0,15000,25000,30000,40000,11,78273,2,0,0,0,0,1,0,0,0,0,0,0,0,'Maxx Avalanche - OOC - Flametongue Weapon'),
(38513,0,0,0,0,0,100,0,2000,4000,4000,6000,11,73538,0,0,0,0,0,2,0,0,0,0,0,0,0,'Evol Fingers - In Combat - Shadow Bolt'),
(38514,0,0,0,0,0,100,0,2000,4000,4000,6000,11,73543,0,0,0,0,0,2,0,0,0,0,0,0,0,'Fizz Lighter - In Combat - Fireball'),
(39193,0,0,0,1,0,100,0,20000,60000,60000,90000,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Brute Overseer - OOC - Ambient Say'),
(39193,0,1,0,0,0,100,0,4000,8000,8000,12000,11,3551,0,0,0,0,0,2,0,0,0,0,0,0,0,'Brute Overseer - In Combat - Skull Crack'),
(39193,0,2,0,0,0,100,0,10000,15000,12000,18000,11,76137,0,0,0,0,0,1,0,0,0,0,0,0,0,'Brute Overseer - In Combat - Acidic Sweat'),
(39354,0,0,0,0,0,100,0,3000,6000,4500,6500,11,75962,0,0,0,0,0,2,0,0,0,0,0,0,0,'Steamwheedle Shark - In Combat - Shred Armor'),
(39354,0,1,0,0,0,100,0,7000,11000,9000,14000,11,32735,0,0,0,0,0,2,0,0,0,0,0,0,0,'Steamwheedle Shark - In Combat - Saw Blade'),
(39354,0,2,0,6,0,100,0,0,0,0,0,11,73852,2,0,0,0,0,1,0,0,0,0,0,0,0,'Steamwheedle Shark - On Death - Spin-out FX'),
(39376,0,0,0,0,0,100,0,25000,35000,30000,45000,11,73867,0,0,0,0,0,2,0,0,0,0,0,0,0,'KTC Oil Bot - In Combat - Cutting Laser'),
(39376,0,1,0,1,0,100,0,5000,20000,8000,20000,11,73873,2,0,0,0,0,19,23837,15,0,0,0,0,0,'KTC Oil Bot - OOC - Ambient Cutting Laser');
-- Brute Overseer say groups into one random group.
UPDATE `creature_text` SET `ID`=`GroupID`, `GroupID`=0 WHERE `CreatureID`=39193;
