-- Goblin Escape Pods (14474): retail pod-opening event (P2 sniff, 16:51:38-16:52:50).
-- Retail flow on goober use: player is credited, self-casts 66137 "Goblin Escape Pods: Summon Live
-- Goblin Survivor" (summon 34748 at TARGET_DEST_NEARBY_ENTRY = the pod GO) + 66136 "Summons
-- Controller" (self dummy), the pod GO despawns, the survivor self-casts 37744 "Emote State: Swim",
-- yells a random line at the rescuer (~+1.8s), then swims/runs to the shore point (+4.7s), on to the
-- beach camp, and despawns. The goober's own Data10 spell 67474 does not exist in 4.3.4 Spell.dbc.

-- 66137 EFFECT_0 summon dest anchor: nearest Goblin Escape Pod GO.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry`=66137;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(13,1,66137,0,0,31,0,5,195188,0,0,0,0,'','Goblin Escape Pods: Summon Live Goblin Survivor - implicit dest = Goblin Escape Pod');

-- Pod goober: credit, retail player-casts, pod despawn (respawn via spawntimesecs 300).
DELETE FROM `smart_scripts` WHERE `entryorguid`=195188 AND `source_type`=1;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(195188,1,0,1,64,0,100,0,0,0,0,0,33,34748,0,0,0,0,0,7,0,0,0,0,0,0,0,'Goblin Escape Pod - On Used - Kill Credit Goblin Survivor'),
(195188,1,1,2,64,0,100,0,0,0,0,0,85,66137,2,0,0,0,0,7,0,0,0,0,0,0,0,'Goblin Escape Pod - On Used - Invoker Casts Summon Live Goblin Survivor'),
(195188,1,2,3,64,0,100,0,0,0,0,0,85,66136,2,0,0,0,0,7,0,0,0,0,0,0,0,'Goblin Escape Pod - On Used - Invoker Casts Summons Controller'),
(195188,1,3,0,64,0,100,0,0,0,0,0,41,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Goblin Escape Pod - On Used - Despawn');

-- Goblin Survivor: swim state, random thank-you yell at the rescuer, two-leg escape run, despawn.
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=34748;
DELETE FROM `smart_scripts` WHERE `entryorguid`=34748 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(34748,0,0,1,54,0,100,0,0,0,0,0,59,1,0,0,0,0,0,1,0,0,0,0,0,0,0,'Goblin Survivor - On Just Summoned - Set Run'),
(34748,0,1,0,54,0,100,0,0,0,0,0,11,37744,2,0,0,0,0,1,0,0,0,0,0,0,0,'Goblin Survivor - On Just Summoned - Cast Emote State: Swim'),
(34748,0,2,0,1,0,100,1,1500,2000,0,0,1,0,0,0,0,0,0,7,0,0,0,0,0,0,0,'Goblin Survivor - Out of Combat - Yell Random Thanks At Rescuer'),
(34748,0,3,0,1,0,100,1,4600,5000,0,0,69,1,0,1,0,0,0,8,0,0,0,571.281,3181.45,-2.889165,0,'Goblin Survivor - Out of Combat - Move To Shore'),
(34748,0,4,0,34,0,100,0,8,1,0,0,69,2,0,1,0,0,0,8,0,0,0,594.541,3135.0342,-0.904856,0,'Goblin Survivor - On Reached Shore - Move To Camp'),
(34748,0,5,0,34,0,100,0,8,2,0,0,41,2000,0,0,0,0,0,1,0,0,0,0,0,0,0,'Goblin Survivor - On Reached Camp - Despawn'),
(34748,0,6,0,1,0,100,1,60000,60000,0,0,41,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Goblin Survivor - Out of Combat - Despawn Failsafe');

-- One text group so the yell is picked at random (retail rotates all seven lines).
DELETE FROM `creature_text` WHERE `CreatureID`=34748;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(34748,0,0,'What were those Alliance ships doing firing at us?! Did I see a Horde ship, too?',12,0,100,5,0,0,0,35009,0,'Goblin Survivor to Player'),
(34748,0,1,'Blowing open my escape pod. I like your style, $n!',12,0,100,5,0,0,0,35004,0,'Goblin Survivor to Player'),
(34748,0,2,'$n! Please tell me that monster, the Trade Prince, didn''t survive?!',12,0,100,5,0,0,0,35007,0,'Goblin Survivor to Player'),
(34748,0,3,'$n, it''s you! Thanks, $g man : sweetie;!',12,0,100,5,0,0,0,35003,0,'Goblin Survivor to Player'),
(34748,0,4,'Thanks, $n. You still owe me that money, $g man : lady;!',12,0,100,5,0,0,0,35008,0,'Goblin Survivor to Player'),
(34748,0,5,'What, are you crazy?! Trying to blow me to smithereens like that!',12,0,100,5,0,0,0,35002,0,'Goblin Survivor to Player'),
(34748,0,6,'Couldn''t you have just used a crowbar?',12,0,100,5,0,0,0,35006,0,'Goblin Survivor to Player');
