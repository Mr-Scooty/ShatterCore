-- Three Little Pygmies (24945): P4-sniff fixes.
-- Malmo and Teloch were each spawned twice on the same spot; drop the duplicates.
DELETE FROM `creature_addon` WHERE `guid` IN (393885,393952);
DELETE FROM `creature` WHERE `guid` IN (393885,393952);

-- Boss kits (sniffed): all three cast Soul Missile (72941) and Zombie Transformation
-- (72935); Gaahl adds Frost Shock, Malmo Lightning Bolt, Teloch Fire and the Flames.
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry` IN (38808,38809,38810);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (38808,38809,38810) AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(38808,0,0,0,0,0,100,0,3000,6000,7000,11000,11,12548,0,0,0,0,0,2,0,0,0,0,0,0,0,'Gaahl - In Combat - Cast Frost Shock'),
(38808,0,1,0,0,0,100,0,6000,10000,10000,16000,11,72941,0,0,0,0,0,2,0,0,0,0,0,0,0,'Gaahl - In Combat - Cast Soul Missile'),
(38808,0,2,0,0,0,100,0,12000,18000,16000,24000,11,72935,0,0,0,0,0,2,0,0,0,0,0,0,0,'Gaahl - In Combat - Cast Zombie Transformation'),
(38809,0,0,0,0,0,100,0,2500,4500,3400,5000,11,57780,0,0,0,0,0,2,0,0,0,0,0,0,0,'Malmo - In Combat - Cast Lightning Bolt'),
(38809,0,1,0,0,0,100,0,6000,10000,10000,16000,11,72941,0,0,0,0,0,2,0,0,0,0,0,0,0,'Malmo - In Combat - Cast Soul Missile'),
(38809,0,2,0,0,0,100,0,12000,18000,16000,24000,11,72935,0,0,0,0,0,2,0,0,0,0,0,0,0,'Malmo - In Combat - Cast Zombie Transformation'),
(38810,0,0,0,0,0,100,0,4000,8000,10000,18000,11,75946,0,0,0,0,0,2,0,0,0,0,0,0,0,'Teloch - In Combat - Cast Fire and the Flames'),
(38810,0,1,0,0,0,100,0,6000,10000,10000,16000,11,72941,0,0,0,0,0,2,0,0,0,0,0,0,0,'Teloch - In Combat - Cast Soul Missile'),
(38810,0,2,0,0,0,100,0,12000,18000,16000,24000,11,72935,0,0,0,0,0,2,0,0,0,0,0,0,0,'Teloch - In Combat - Cast Zombie Transformation');
