-- The Heads of the SI:7 (25093): the three named rogues had no AI (P4 sniff kits).
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry` IN (39141,39142,39143);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (39141,39142,39143) AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(39141,0,0,0,0,0,100,0,3000,5000,5000,8000,11,60195,0,0,0,0,0,2,0,0,0,0,0,0,0,'Commander Arrington - In Combat - Cast Sinister Strike'),
(39141,0,1,0,0,0,100,0,8000,12000,10000,14000,11,79851,0,0,0,0,0,1,0,0,0,0,0,0,0,'Commander Arrington - In Combat - Cast Fan of Knives'),
(39141,0,2,0,0,0,100,0,10000,15000,12000,16000,11,79852,0,0,0,0,0,2,0,0,0,0,0,0,0,'Commander Arrington - In Combat - Cast Eviscerate'),
(39141,0,3,0,2,0,100,1,0,50,0,0,11,79853,0,0,0,0,0,1,0,0,0,0,0,0,0,'Commander Arrington - At 50% Health - Cast Evasion (once)'),
(39142,0,0,0,4,0,100,0,0,0,0,0,11,66060,2,0,0,0,0,1,0,0,0,0,0,0,0,'Darkblade Cyn - On Aggro - Cast Sprint'),
(39142,0,1,0,4,0,100,0,0,0,0,0,11,3583,2,0,0,0,0,1,0,0,0,0,0,0,0,'Darkblade Cyn - On Aggro - Cast Deadly Poison'),
(39142,0,2,0,0,0,100,0,4000,7000,6000,9000,11,60850,0,0,0,0,0,2,0,0,0,0,0,0,0,'Darkblade Cyn - In Combat - Cast Mutilate'),
(39142,0,3,0,0,0,100,0,6000,9000,5000,8000,11,60195,0,0,0,0,0,2,0,0,0,0,0,0,0,'Darkblade Cyn - In Combat - Cast Sinister Strike'),
(39143,0,0,0,4,0,100,0,0,0,0,0,11,79864,2,0,0,0,0,2,0,0,0,0,0,0,0,'Alexi Silenthowl - On Aggro - Cast Shadowstep'),
(39143,0,1,0,0,0,100,0,4000,6000,4000,6000,11,79863,0,0,0,0,0,2,0,0,0,0,0,0,0,'Alexi Silenthowl - In Combat - Cast Hemorrhage'),
(39143,0,2,0,0,0,100,0,8000,12000,10000,15000,11,79866,0,0,0,0,0,2,0,0,0,0,0,0,0,'Alexi Silenthowl - In Combat - Cast Deadly Throw');
