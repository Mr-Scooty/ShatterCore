-- Precious Cargo (14242) / Meet Me Up Top (14326) / Warchief's Revenge (14243): P3-sniff fixes.

-- 14242 credit comes from GOSSIPING the caged Thrall below deck (ADD_CREDIT ObjectID 36145
-- fires on CMSG_TALK_TO_GOSSIP in the sniff) - not from the gyrochoppa landing.
-- Accepting 14326 makes Thrall cast 68407 at the player, who then summons the freed
-- Thrall runner (69079, dest anchored on the cage).
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry` IN (36145,36161,36177);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (36145,36161,36177) AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(36145,0,0,0,64,0,100,0,0,0,0,0,33,36145,0,0,0,0,0,7,0,0,0,0,0,0,0,'Thrall (caged) - On Gossip Hello - Kill Credit Precious Cargo'),
(36145,0,1,2,19,0,100,0,14326,0,0,0,11,68407,2,0,0,0,0,7,0,0,0,0,0,0,0,'Thrall (caged) - On Quest Accepted (Meet Me Up Top) - Cast Quest Accept'),
(36145,0,2,0,61,0,100,0,0,0,0,0,85,69079,2,0,0,0,0,7,0,0,0,0,0,0,0,'Thrall (caged) - Linked - Invoker Summons Freed Thrall'),
(36161,0,0,1,1,0,100,0,8000,15000,12000,16000,11,68440,0,0,0,0,0,1,0,0,0,0,0,0,0,'Thrall (upper deck) - Out of Combat - Cast Chain Lightning at deckhands'),
(36161,0,1,0,61,0,40,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Thrall (upper deck) - Linked - Storm Yell'),
(36177,0,0,0,8,0,100,0,68440,0,4000,8000,11,42345,2,0,0,0,0,1,0,0,0,0,0,0,0,'Alliance Deckhand - On Spellhit Chain Lightning - Ragdoll Knockback');

-- Thrall's three storm yells into one group for random selection.
UPDATE `creature_text` SET `GroupID`=0, `ID`=1 WHERE `CreatureID`=36161 AND `GroupID`=1;
UPDATE `creature_text` SET `GroupID`=0, `ID`=2 WHERE `CreatureID`=36161 AND `GroupID`=2;

-- 68440 chain-targets the deckhands; 69079 summons the runner at the cage.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry` IN (68440,69079);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(13,1,68440,0,0,31,0,3,36177,0,0,0,0,'','Warchief''s Revenge: Chain Lightning targets Alliance deckhands'),
(13,1,69079,0,0,31,0,3,36145,0,0,0,0,'','Meet Me Up Top: Summon The Warchief - dest anchored on the caged Thrall');

UPDATE `creature_template` SET `ScriptName`='npc_lost_isles_freed_thrall' WHERE `entry`=36622;
