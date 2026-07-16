-- Gallywix mine arc: wire the orphaned quest-giver barks (all texts already existed;
-- nothing triggered them - P4/P5 sniff timings).
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry` IN (38124,39066,38517,38441,39199) AND `ScriptName`='';
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (38124,39066,38517,38441,39199) AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(38124,0,0,0,19,0,100,0,25110,0,0,0,1,5,0,0,0,0,0,7,0,0,0,0,0,0,0,'Assistant Greely - On Quest Accepted (Kaja''Cola Gives You IDEAS!) - Drool Emote'),
(38124,0,1,0,19,0,100,0,25200,0,0,0,1,6,0,0,0,0,0,7,0,0,0,0,0,0,0,'Assistant Greely - On Quest Accepted (Shredder Shutdown) - Say Line 6'),
(38124,0,2,0,19,0,100,0,25204,0,0,0,1,7,0,0,0,0,0,7,0,0,0,0,0,0,0,'Assistant Greely - On Quest Accepted (Release the Valves) - Say Line 7'),
(38124,0,3,0,19,0,100,0,25213,0,0,0,1,8,0,0,0,0,0,7,0,0,0,0,0,0,0,'Assistant Greely - On Quest Accepted (The Slave Pits) - Say Line 8'),
(39066,0,0,0,19,0,100,0,25100,0,0,0,1,0,0,0,0,0,0,7,0,0,0,0,0,0,0,'Kilag Gorefang - On Quest Accepted (Let''s Ride) - Say Line 0'),
(38517,0,0,0,19,0,100,0,25109,0,0,0,1,0,0,0,0,0,0,7,0,0,0,0,0,0,0,'Slinky Sharpshiv - On Quest Accepted (The Gallywix Labor Mine) - Say Line 0'),
(38441,0,0,0,19,0,100,0,25203,0,0,0,1,0,0,0,0,0,0,7,0,0,0,0,0,0,0,'Ace - On Quest Accepted (What Kind of Name is Chip, Anyway?) - Say Line 0'),
(39199,0,0,0,19,0,100,0,25122,0,0,0,1,1,0,0,0,0,0,7,0,0,0,0,0,0,0,'Assistant Greely (mine) - On Quest Accepted (Morale Boost) - Say Line 1'),
(39199,0,1,0,19,0,100,0,25125,0,0,0,1,3,0,0,0,0,0,7,0,0,0,0,0,0,0,'Assistant Greely (mine) - On Quest Accepted (Light at the End of the Tunnel) - Say Line 3');

DELETE FROM `smart_scripts` WHERE (`entryorguid`=38387 AND `source_type`=0 AND `id`=6)
   OR (`entryorguid`=38738 AND `source_type`=0);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(38387,0,6,0,19,0,100,0,25098,0,0,0,1,5,0,0,0,0,0,7,0,0,0,0,0,0,0,'Sassy Hardwrench - On Quest Accepted (The Warchief Wants You) - Say Line 5'),
(38738,0,0,0,19,0,100,0,25201,0,0,0,1,2,0,0,0,0,0,7,0,0,0,0,0,0,0,'Coach Crosscheck - On Quest Accepted (The Ultimate Footbomb Uniform) - Say Line 2');
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=38738 AND `ScriptName`='';
