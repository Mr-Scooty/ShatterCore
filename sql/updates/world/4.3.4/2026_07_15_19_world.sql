-- Children of a Turtle God (24954): the Volcanoth brood had no AI (P4 sniff).
-- Child of Volcanoth 38845: ambient Cosmetic Fire Breath (66049, ~10-15s OOC) and a real
-- Flame Breath (8873) in combat. Volcanoth Champion 38850: Heated Weapon (36102) self-buff.
-- Volcanoth Priest 38851: Fireball (72991).
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry` IN (38845,38850,38851);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (38845,38850,38851) AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(38845,0,0,0,1,0,100,0,3000,15000,10000,15000,11,66049,2,0,0,0,0,1,0,0,0,0,0,0,0,'Child of Volcanoth - Out of Combat - Cosmetic Fire Breath'),
(38845,0,1,0,0,0,100,0,5000,9000,8000,12000,11,8873,0,0,0,0,0,2,0,0,0,0,0,0,0,'Child of Volcanoth - In Combat - Cast Flame Breath'),
(38850,0,0,0,0,0,100,0,2000,6000,10000,15000,11,36102,0,0,0,0,0,1,0,0,0,0,0,0,0,'Volcanoth Champion - In Combat - Cast Heated Weapon'),
(38851,0,0,0,0,0,100,0,2000,4000,3400,4800,11,72991,0,0,0,0,0,2,0,0,0,0,0,0,0,'Volcanoth Priest - In Combat - Cast Fireball');
