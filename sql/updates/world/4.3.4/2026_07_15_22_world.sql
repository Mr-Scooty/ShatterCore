-- Old Friends (25023): Sassy's "Get me up into the skies!" gossip option was a dead end
-- (it only closed the window; boarding worked solely by clicking the parked bomber).
-- Retail (P4 sniff 10:30:36): the select casts 73135 "Quest Accept & Bind" (binds the
-- hearthstone to Lost Peak) and 73105 "Summon Flying Bomber" - a native summon-and-ride
-- (BasePoints 73137, prop 827) anchored on Sassy.
DELETE FROM `smart_scripts` WHERE `entryorguid`=38928 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(38928,0,0,1,62,0,100,0,11146,0,0,0,85,73135,2,0,0,0,0,7,0,0,0,0,0,0,0,'Sassy Hardwrench (Lost Peak) - Gossip Select - Invoker casts Quest Accept & Bind'),
(38928,0,1,2,61,0,100,0,0,0,0,0,85,73105,2,0,0,0,0,7,0,0,0,0,0,0,0,'Sassy Hardwrench (Lost Peak) - Linked - Invoker summons Flying Bomber'),
(38928,0,2,0,61,0,100,0,0,0,0,0,72,0,0,0,0,0,0,7,0,0,0,0,0,0,0,'Sassy Hardwrench (Lost Peak) - Linked - Close gossip');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry`=73105;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(13,1,73105,0,0,31,0,3,38928,0,0,0,0,'','Old Friends: Summon Flying Bomber - dest anchored on Sassy Hardwrench');
