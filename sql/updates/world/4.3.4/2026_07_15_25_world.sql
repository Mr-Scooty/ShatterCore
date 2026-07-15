-- Escape Velocity (25214): P5-sniff fixes.
-- The eight launch yells existed as separate groups - merge into one random group so the
-- launched captive picks one; Southsea Mercenaries use Swashbuckling Slice; Hobart barks
-- his booster-rockets line on accept.
UPDATE `creature_text` SET `ID`=`GroupID`, `GroupID`=0 WHERE `CreatureID`=39456;
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=39449;
DELETE FROM `smart_scripts` WHERE `entryorguid`=39449 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(39449,0,0,0,0,0,100,0,5000,9000,20000,30000,11,75361,0,0,0,0,0,2,0,0,0,0,0,0,0,'Southsea Mercenary - In Combat - Cast Swashbuckling Slice');

UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=38120;
DELETE FROM `smart_scripts` WHERE `entryorguid`=38120 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(38120,0,0,0,19,0,100,0,25214,0,0,0,1,8,0,0,0,0,0,7,0,0,0,0,0,0,0,'Hobart Grapplehammer - On Quest Accepted (Escape Velocity) - Say Line 8');
