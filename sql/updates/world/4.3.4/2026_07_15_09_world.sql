-- Trading Up (24741): P3-sniff fixes.
-- Spiny Raptors (145 spawns guarding the nests) fight with Head Butt (42320) and burst
-- into a bloody explosion when put down (35309 + the retail feign-death trick; the visual
-- burst is the player-facing part).
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=38187;
DELETE FROM `smart_scripts` WHERE `entryorguid`=38187 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(38187,0,0,0,0,0,100,0,4000,7000,8000,12000,11,42320,0,0,0,0,0,2,0,0,0,0,0,0,0,'Spiny Raptor - In Combat - Cast Head Butt'),
(38187,0,1,0,6,0,100,0,0,0,0,0,11,35309,2,0,0,0,0,1,0,0,0,0,0,0,0,'Spiny Raptor - On Death - Cast Bloody Explosion');

-- Only five Spiny Raptor Egg nests are spawned; keep them turning over so the
-- 5-egg objective doesn't starve (they are 100% quest loot).
UPDATE `gameobject` SET `spawntimesecs`=60 WHERE `id`=201974;
