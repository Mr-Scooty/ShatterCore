-- Good-bye, Sweet Oil (25207): the oil rig now visibly blows up - the explosion
-- controller bunny (39383, summoned by 73888) runs the sniffed 3-stage detonation with
-- camera shakes 2.4s apart.
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=39383 AND `ScriptName`='';
DELETE FROM `smart_scripts` WHERE `entryorguid`=39383 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(39383,0,0,1,54,0,100,0,0,0,0,0,11,73904,2,0,0,0,0,1,0,0,0,0,0,0,0,'Oil Rig Explosion Bunny - On Summoned - Explosions 01'),
(39383,0,1,0,61,0,100,0,0,0,0,0,11,73905,2,0,0,0,0,1,0,0,0,0,0,0,0,'Oil Rig Explosion Bunny - Linked - Camera Shake'),
(39383,0,2,3,1,0,100,1,2400,2400,0,0,11,73890,2,0,0,0,0,1,0,0,0,0,0,0,0,'Oil Rig Explosion Bunny - Timed - Explosions 02'),
(39383,0,3,0,61,0,100,0,0,0,0,0,11,73905,2,0,0,0,0,1,0,0,0,0,0,0,0,'Oil Rig Explosion Bunny - Linked - Camera Shake'),
(39383,0,4,5,1,0,100,1,4800,4800,0,0,11,73891,2,0,0,0,0,1,0,0,0,0,0,0,0,'Oil Rig Explosion Bunny - Timed - Explosions 03'),
(39383,0,5,0,61,0,100,0,0,0,0,0,11,73905,2,0,0,0,0,1,0,0,0,0,0,0,0,'Oil Rig Explosion Bunny - Linked - Camera Shake');
