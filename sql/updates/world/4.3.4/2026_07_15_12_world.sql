-- Surrender or Else! (24868): the entire retail event was missing - the Faceless of the
-- Deep sat in the pool as a permanent world spawn and nothing summoned Ace or the leashed
-- hatchlings. P3 sniff: accept summons Ace (72058) + 12 Naga Hatchlings (72073) who parade
-- to the spawning pool; crossing areatrigger 5721 makes Ace demand the leader's surrender
-- and the Faceless emerges (event scripted in lost_isles_act12.cpp).

-- Summon dest anchors (both accept-summons resolve TARGET_DEST_NEARBY_ENTRY on Megs 38432;
-- the Faceless water-spray 72076 anchors on the Faceless itself).
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry` IN (72058,72073,72076);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(13,1,72058,0,0,31,0,3,38432,0,0,0,0,'','Surrender Or Else!: Summon Ace - dest anchored on Megs Dreadshredder'),
(13,1,72073,0,0,31,0,3,38432,0,0,0,0,'','Surrender Or Else!: Summon Naga Hatchlings - dest anchored on Megs Dreadshredder'),
(13,1,72076,0,0,31,0,3,38448,0,0,0,0,'','Surrender Or Else!: Faceless Beam Effect - dest anchored on the Faceless');

UPDATE `creature_template` SET `ScriptName`='npc_lost_isles_ace_surrender' WHERE `entry`=38455;
UPDATE `creature_template` SET `ScriptName`='npc_lost_isles_faceless_of_the_deep' WHERE `entry`=38448;

-- The leashed hatchlings are friendly escort clones (sniff faction 2204), not hostiles.
UPDATE `creature_template` SET `faction`=2204 WHERE `entry`=38457;
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=38457;
DELETE FROM `smart_scripts` WHERE `entryorguid`=38457 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(38457,0,0,0,54,0,100,0,0,0,0,0,29,3,2,0,0,0,0,7,0,0,0,0,0,0,0,'Naga Hatchling (escort) - On Just Summoned - Follow Summoner');

DELETE FROM `areatrigger_scripts` WHERE `entry`=5721;
INSERT INTO `areatrigger_scripts` (`entry`,`ScriptName`) VALUES
(5721,'at_lost_isles_surrender_or_else');

-- The Faceless is event-summoned; remove the permanent world spawn.
DELETE FROM `creature_addon` WHERE `guid`=392856;
DELETE FROM `creature` WHERE `guid`=392856;
