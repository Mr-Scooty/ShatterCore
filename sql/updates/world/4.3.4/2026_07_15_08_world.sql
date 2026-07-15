-- Vale chain quest-giver barks: Aggra (35875) and Kilag Gorefang (35917) both carry
-- imported creature_text rows that nothing triggered. Wire them to their quest accepts.
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry` IN (35875,35917);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (35875,35917) AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(35875,0,0,0,19,0,100,0,14235,0,0,0,1,0,0,0,0,0,0,7,0,0,0,0,0,0,0,'Aggra - On Quest Accepted (The Vicious Vale) - Say Line 0'),
(35875,0,1,0,19,0,100,0,14237,0,0,0,1,1,0,0,0,0,0,7,0,0,0,0,0,0,0,'Aggra - On Quest Accepted (Forward Movement) - Say Line 1'),
(35917,0,0,0,19,0,100,0,14238,0,0,0,1,0,0,0,0,0,0,7,0,0,0,0,0,0,0,'Kilag Gorefang - On Quest Accepted (Infrared = Infradead) - Say Line 0'),
(35917,0,1,0,19,0,100,0,14240,0,0,0,1,1,0,0,0,0,0,7,0,0,0,0,0,0,0,'Kilag Gorefang - On Quest Accepted (To the Cliffs) - Say Line 1');
