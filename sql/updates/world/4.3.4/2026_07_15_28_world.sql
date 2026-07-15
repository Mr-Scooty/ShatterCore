-- Final Confrontation (25251): P6-sniff fixes.
-- Revenue Stream (74005) pulses its 74006 money-beam visual every 250ms via AuraScript;
-- Thrall's Force of Nature whirlwinds (31688) pulse Whirlwind damage continuously;
-- Sassy barks her get-into-the-uniform line on accept.
DELETE FROM `spell_script_names` WHERE `spell_id`=74005;
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(74005,'spell_lost_isles_revenue_stream');

UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=31688 AND `ScriptName`='';
DELETE FROM `smart_scripts` WHERE `entryorguid`=31688 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(31688,0,0,0,54,0,100,0,0,0,0,0,11,59549,2,0,0,0,0,1,0,0,0,0,0,0,0,'Whirlwind - On Just Summoned - Self Visual'),
(31688,0,1,0,1,0,100,0,500,1500,1000,2000,11,59550,2,0,0,0,0,1,0,0,0,0,0,0,0,'Whirlwind - Out of Combat - Pulse Whirlwind');

INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(38387,0,5,0,19,0,100,0,25251,0,0,0,1,7,0,0,0,0,0,7,0,0,0,0,0,0,0,'Sassy Hardwrench - On Quest Accepted (Final Confrontation) - Say Line 7');
