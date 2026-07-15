-- Release the Valves (25204): the valve's 73838 dummy tells the nearby bunny to vent -
-- ELM bunny 33765 now bursts steam (73839) when hit by it (2-3 bursts on retail).
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=33765 AND `ScriptName`='';
DELETE FROM `smart_scripts` WHERE `entryorguid`=33765 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(33765,0,0,1,8,0,100,0,73838,0,3000,3000,11,73839,2,0,0,0,0,1,0,0,0,0,0,0,0,'ELM Bunny - On Spellhit Release The Valves Dummy - Steam Burst'),
(33765,0,1,0,61,0,100,0,0,0,0,0,67,1,3300,3300,0,0,0,1,0,0,0,0,0,0,0,'ELM Bunny - Linked - Second burst timer'),
(33765,0,2,0,59,0,100,0,1,0,0,0,11,73839,2,0,0,0,0,1,0,0,0,0,0,0,0,'ELM Bunny - Timed - Steam Burst 2');
