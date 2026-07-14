-- Monkey Business (14019): retail fixes (P3 sniff, 08:57:35-09:01).
-- The Bomb-Throwing Monkeys ambiently lob cosmetic bombs at random nearby points (66142,
-- 170 casts in the sniff window) - previously missing entirely.
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=34699;
DELETE FROM `smart_scripts` WHERE `entryorguid`=34699 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(34699,0,0,0,1,0,100,0,5000,25000,15000,30000,11,66142,0,0,0,0,0,1,0,0,0,0,0,0,0,'Bomb-Throwing Monkey - Out of Combat - Cast Cosmetic Throw Bomb');

-- Monkey Business is available in parallel with Get Our Stuff Back! and It's Our Problem Now
-- (all three accepted back-to-back in the sniff); it only requires the escape-pods quest.
UPDATE `quest_template_addon` SET `PrevQuestID`=14474 WHERE `ID`=14019;
