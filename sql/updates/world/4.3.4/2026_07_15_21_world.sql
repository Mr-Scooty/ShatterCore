-- Repel the Paratroopers (25024): P4-sniff fixes.
-- The static Alliance Paratroopers (39069) and the camp's Orc Scout defenders (39068) had
-- no AI; retail shows Shoot (6660) as the paratrooper ranged auto (1551 casts) and the
-- scouts returning fire with 73388/73389 in a constant crossfire. The Gnomeregan Stealth
-- Fighters (39039) now drop paratroopers themselves via 73327 (retail: 102 casts in the
-- quest window; the dest is the caster so the trooper spawns at the fighter and
-- parachutes down via the existing npc_alliance_paratrooper AI) and catch fire on death.
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry` IN (39069,39068,39039);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (39069,39068,39039) AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(39069,0,0,0,0,0,100,0,1500,2500,1800,2600,11,6660,0,0,0,0,0,2,0,0,0,0,0,0,0,'Alliance Paratrooper - In Combat - Cast Shoot'),
(39068,0,0,0,0,0,100,0,1000,3000,3500,5500,11,73389,0,0,0,0,0,2,0,0,0,0,0,0,0,'Orc Scout (lookout) - In Combat - Cast Shoot'),
(39068,0,1,0,0,0,100,0,2500,4500,3500,5500,11,73388,0,0,0,0,0,2,0,0,0,0,0,0,0,'Orc Scout (lookout) - In Combat - Cast Shoot (cover)'),
(39039,0,0,0,1,0,100,0,5000,20000,15000,25000,11,73327,2,0,0,0,0,1,0,0,0,0,0,0,0,'Gnomeregan Stealth Fighter - OOC - Drop Alliance Paratrooper'),
(39039,0,1,0,6,0,100,0,0,0,0,0,11,73481,2,0,0,0,0,1,0,0,0,0,0,0,0,'Gnomeregan Stealth Fighter - On Death - Catch Fire');
