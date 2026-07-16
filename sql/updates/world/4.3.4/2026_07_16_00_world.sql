-- Don't Go Into the Light! (14239): spawn the personal Doc Zapnozzle directly
-- on the barrel.  The previous implementation recreated a modern-retail
-- airborne entrance and hop, causing Doc to fall into place instead.
--
-- The barrel is client-side scenery and has no server collision, so disable
-- gravity as soon as Doc is summoned.  Do not disable swimming on his creature
-- template: he must swim toward the shore after the quest is rewarded.
DELETE FROM `creature_template_movement` WHERE `CreatureId`=36608;

UPDATE `creature`
SET `position_x`=537.135, `position_y`=3272.25, `position_z`=0.18, `orientation`=2.858162
WHERE `guid`=9000884 AND `id`=21252;

DELETE FROM `smart_scripts` WHERE `entryorguid`=36608 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(36608,0,0,1,54,0,100,0,0,0,0,0,0,64,1,0,0,0,0,0,7,0,0,0,0,0,0,0,'Doc Zapnozzle - On Just Summoned - Store summoner'),
(36608,0,1,2,61,0,100,0,0,0,0,0,0,60,1,0,0,0,0,0,1,0,0,0,0,0,0,0,'Doc Zapnozzle - Linked - Disable gravity on the barrel'),
(36608,0,2,0,61,0,100,0,0,0,0,0,0,80,3660800,0,0,0,0,0,1,0,0,0,0,0,0,0,'Doc Zapnozzle - Linked - Run revive vignette'),
(36608,0,3,4,20,0,100,0,14239,0,0,0,0,64,1,0,0,0,0,0,7,0,0,0,0,0,0,0,'Doc Zapnozzle - On Quest 14239 Rewarded - Store player'),
(36608,0,4,0,61,0,100,0,0,0,0,0,0,80,3660801,0,0,0,0,0,1,0,0,0,0,0,0,0,'Doc Zapnozzle - Linked - Run farewell');

-- Goblin Escape Pods (14474) becomes available only after 14239 is rewarded.
-- Keep the normal quest-chain prerequisite and also gate QUEST_AVAILABLE so
-- the quest marker and direct acceptance path enforce the same requirement.
UPDATE `quest_template_addon` SET `PrevQuestID`=14239 WHERE `ID` IN (14001,14474);

DELETE FROM `conditions`
WHERE `SourceTypeOrReferenceId`=19 AND `SourceEntry` IN (14001,14474)
  AND `ConditionTypeOrReference`=8 AND `ConditionValue1`=14239;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(19,0,14001,0,0,8,0,14239,0,0,0,0,0,'','Goblin Escape Pods (14001) requires Don''t Go Into the Light! rewarded'),
(19,0,14474,0,0,8,0,14239,0,0,0,0,0,'','Goblin Escape Pods (14474) requires Don''t Go Into the Light! rewarded');
