-- Chip Endale (25203) and Candy Cane (25244): the jilted lovers had no AI (P5 sniff).
-- Chip: Heartbroken cosmetic + aggro line, Full Monte (2s knockback pulses) ~8s in,
-- one Beatdown. Candy: Heartbroken + "But, sweetie...?".
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry` IN (39363,39426) AND `ScriptName`='';
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (39363,39426) AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(39363,0,0,1,4,0,100,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Chip Endale - On Aggro - Say Line 0'),
(39363,0,1,0,61,0,100,0,0,0,0,0,11,62013,2,0,0,0,0,1,0,0,0,0,0,0,0,'Chip Endale - On Aggro - Heartbroken'),
(39363,0,2,0,0,0,100,1,8000,8000,0,0,11,75968,0,0,0,0,0,1,0,0,0,0,0,0,0,'Chip Endale - In Combat - Full Monte (once)'),
(39363,0,3,0,0,0,100,1,12000,20000,0,0,11,75964,0,0,0,0,0,2,0,0,0,0,0,0,0,'Chip Endale - In Combat - Beatdown (once)'),
(39426,0,0,1,4,0,100,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Candy Cane - On Aggro - Say Line 0'),
(39426,0,1,0,61,0,100,0,0,0,0,0,11,62013,2,0,0,0,0,1,0,0,0,0,0,0,0,'Candy Cane - On Aggro - Heartbroken');
