-- It's Our Problem Now (14473): P3-sniff fixes.
-- Teraptor Hatchlings open with Rushing Charge (6268) when they engage.
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=36740;
DELETE FROM `smart_scripts` WHERE `entryorguid`=36740 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(36740,0,0,0,4,0,100,0,0,0,0,0,11,6268,0,0,0,0,0,2,0,0,0,0,0,0,0,'Teraptor Hatchling - On Aggro - Cast Rushing Charge'),
(36740,0,1,0,0,0,100,0,9000,14000,12000,18000,11,6268,0,0,0,0,0,2,0,0,0,0,0,0,0,'Teraptor Hatchling - In Combat - Cast Rushing Charge');

-- Available in parallel with 14014/14019; gated only on the escape-pods quest (sniff-proven).
UPDATE `quest_template_addon` SET `PrevQuestID`=14474 WHERE `ID`=14473;
