-- Miner Troubles (14021): P3-sniff corrections.
-- Retail grants the credit when the miner reaches the final chamber stop, NOT when the
-- Pygmy Witchdoctor dies (it can still be fighting the player at credit time). The cpp
-- escort now handles credit/farewell/run-off; drop the witchdoctor death-credit row and
-- give it its real kit (Shadow Bolt in combat, ambient pygmy yells).
UPDATE `creature_template` SET `ScriptName`='npc_lost_isles_ore_cart' WHERE `entry`=35814;

DELETE FROM `smart_scripts` WHERE `entryorguid`=35838 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(35838,0,0,0,4,0,100,1,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Pygmy Witchdoctor - On Aggro - Say Line 0 (once)'),
(35838,0,1,0,0,0,100,0,2500,4000,3500,5500,11,9613,0,0,0,0,0,2,0,0,0,0,0,0,0,'Pygmy Witchdoctor - In Combat - Cast Shadow Bolt'),
(35838,0,2,0,1,0,100,0,45000,75000,150000,210000,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,'Pygmy Witchdoctor - Out of Combat - Ambient Yell (Mkay, m''ne ta oor!)'),
(35838,0,3,0,1,0,100,0,100000,140000,150000,210000,1,3,0,0,0,0,0,1,0,0,0,0,0,0,0,'Pygmy Witchdoctor - Out of Combat - Ambient Yell (Ooga booga!)');

-- Smart Mining Monkeys pelt the miner/player with Throw (38560, 16 casts in the sniff).
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=35812;
DELETE FROM `smart_scripts` WHERE `entryorguid`=35812 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(35812,0,0,0,0,0,100,0,3000,6000,5000,9000,11,38560,0,0,0,0,0,2,0,0,0,0,0,0,0,'Smart Mining Monkey - In Combat - Cast Throw');

-- Dampwick barks: accept say ("The kaja'mite has gone and made these monkeys smart!")
-- and turn-in say ("On behalf of the Bilgewater Cartel..."), both sniffed.
DELETE FROM `smart_scripts` WHERE `entryorguid`=35769 AND `source_type`=0 AND `id` IN (2,3);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(35769,0,2,0,19,0,100,0,14021,0,0,0,1,0,0,0,0,0,0,7,0,0,0,0,0,0,0,'Foreman Dampwick - On Quest Accepted (Miner Troubles) - Say Line 0'),
(35769,0,3,0,20,0,100,0,14021,0,0,0,1,1,0,0,0,0,0,7,0,0,0,0,0,0,0,'Foreman Dampwick - On Quest Rewarded (Miner Troubles) - Say Line 1');
