-- Volcanoth! (24958): eruption ambience (P4 sniff).
-- After Volcanoth dies the volcano rim throws a continuous boulder barrage from generic
-- bunnies (43359) - directional spells 74070/74072/74076/74085 firing constantly through
-- the rest of the zone story. Guid-scoped SAI on the 11 rim spawns only (the entry is a
-- zone-wide generic). The four large bunnies (38908, phase 183) belch smoke (73016) over
-- the dead turtle's cave.
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry` IN (43359,38908);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (-394131,-394132,-394136,-394138,-394140,-394141,-394144,-394145,-394154,-394166,-394171) AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(-394131,0,0,0,1,0,100,0,1000,8000,8000,16000,11,74070,2,0,0,0,0,1,0,0,0,0,0,0,0,'Volcano Rim Bunny - OOC - Boulder South'),
(-394132,0,0,0,1,0,100,0,1000,8000,8000,16000,11,74072,2,0,0,0,0,1,0,0,0,0,0,0,0,'Volcano Rim Bunny - OOC - Boulder West'),
(-394136,0,0,0,1,0,100,0,1000,8000,8000,16000,11,74076,2,0,0,0,0,1,0,0,0,0,0,0,0,'Volcano Rim Bunny - OOC - Boulder North'),
(-394138,0,0,0,1,0,100,0,1000,8000,8000,16000,11,74085,2,0,0,0,0,1,0,0,0,0,0,0,0,'Volcano Rim Bunny - OOC - Boulder East'),
(-394140,0,0,0,1,0,100,0,1000,8000,8000,16000,11,74070,2,0,0,0,0,1,0,0,0,0,0,0,0,'Volcano Rim Bunny - OOC - Boulder South'),
(-394141,0,0,0,1,0,100,0,1000,8000,8000,16000,11,74076,2,0,0,0,0,1,0,0,0,0,0,0,0,'Volcano Rim Bunny - OOC - Boulder North'),
(-394144,0,0,0,1,0,100,0,1000,8000,8000,16000,11,74072,2,0,0,0,0,1,0,0,0,0,0,0,0,'Volcano Rim Bunny - OOC - Boulder West'),
(-394145,0,0,0,1,0,100,0,1000,8000,8000,16000,11,74085,2,0,0,0,0,1,0,0,0,0,0,0,0,'Volcano Rim Bunny - OOC - Boulder East'),
(-394154,0,0,0,1,0,100,0,1000,8000,8000,16000,11,74076,2,0,0,0,0,1,0,0,0,0,0,0,0,'Volcano Rim Bunny - OOC - Boulder North'),
(-394166,0,0,0,1,0,100,0,1000,8000,8000,16000,11,74070,2,0,0,0,0,1,0,0,0,0,0,0,0,'Volcano Rim Bunny - OOC - Boulder South'),
(-394171,0,0,0,1,0,100,0,1000,8000,8000,16000,11,74076,2,0,0,0,0,1,0,0,0,0,0,0,0,'Volcano Rim Bunny - OOC - Boulder North');

DELETE FROM `smart_scripts` WHERE `entryorguid`=38908 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(38908,0,0,0,1,0,100,0,1000,3000,1700,2500,11,73016,2,0,0,0,0,1,0,0,0,0,0,0,0,'Volcano Smoke Bunny - OOC - Cosmetic Flame Breath');
