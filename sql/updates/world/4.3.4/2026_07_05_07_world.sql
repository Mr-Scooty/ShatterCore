-- Ensure Deliver Stolen Horse targets Salanar the Horseman.
UPDATE `conditions`
   SET `ConditionTypeOrReference` = 31, `ConditionValue1` = 3, `ConditionValue2` = 28653, `ConditionValue3` = 0,
       `Comment` = 'Deliver Stolen Horse (52264) - implicit target must be Salanar the Horseman'
 WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 52264 AND `SourceGroup` = 1;

-- Require the horse to be within 5 yards of Salanar.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 17 AND `SourceEntry` = 52264;
INSERT INTO `conditions`
 (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
 (17,0,52264,0,0,29,0,28653,5,0,0,0,0,'','Deliver Stolen Horse (52264) - only castable near Salanar the Horseman');

-- Handle passenger removal consistently for every stolen horse.
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` IN (28605,28606,28607);

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND `entryorguid` IN (28605,28606,28607);
INSERT INTO `smart_scripts`
 (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
 (28605,0,0,1,28,0,100,0,500,500,0,0,0, 5,377,0,0,0,0,0, 1,0,0,0,0,0,0,0,'Havenshire Stallion - On Passenger Removed - Play Emote 377'),
 (28605,0,1,0,61,0,100,0,  0,  0,0,0,0,41,5000,0,0,0,0,0, 1,0,0,0,0,0,0,0,'Havenshire Stallion - Linked - Force Despawn In 5000 ms'),
 (28606,0,0,1,28,0,100,0,500,500,0,0,0, 5,377,0,0,0,0,0, 1,0,0,0,0,0,0,0,'Havenshire Mare - On Passenger Removed - Play Emote 377'),
 (28606,0,1,0,61,0,100,0,  0,  0,0,0,0,41,5000,0,0,0,0,0, 1,0,0,0,0,0,0,0,'Havenshire Mare - Linked - Force Despawn In 5000 ms'),
 (28607,0,0,1,28,0,100,0,500,500,0,0,0, 5,377,0,0,0,0,0, 1,0,0,0,0,0,0,0,'Havenshire Colt - On Passenger Removed - Play Emote 377'),
 (28607,0,1,0,61,0,100,0,  0,  0,0,0,0,41,5000,0,0,0,0,0, 1,0,0,0,0,0,0,0,'Havenshire Colt - Linked - Force Despawn In 5000 ms');
