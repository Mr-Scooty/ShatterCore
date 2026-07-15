-- Throw It On the Ground! (25123): P4-sniff fixes.
-- Blastshadow the Brutemaster had no AI: Shadow Bolt + Corruption in combat, his aggro
-- line, Delicia's reactions, and the soulstone chest is summoned by HIS death (73703)
-- instead of only sitting pre-spawned.
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry` IN (39194,39195);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (39194,39195) AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(39194,0,0,1,4,0,100,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Blastshadow the Brutemaster - On Aggro - Say Line 0'),
(39194,0,1,0,61,0,100,0,0,0,0,0,45,0,1,0,0,0,0,19,39195,30,0,0,0,0,0,'Blastshadow the Brutemaster - On Aggro - Tell Delicia'),
(39194,0,2,0,0,0,100,0,2000,4000,3400,4800,11,9613,0,0,0,0,0,2,0,0,0,0,0,0,0,'Blastshadow the Brutemaster - In Combat - Cast Shadow Bolt'),
(39194,0,3,0,0,0,100,0,6000,10000,14000,20000,11,32063,0,0,0,0,0,2,0,0,0,0,0,0,0,'Blastshadow the Brutemaster - In Combat - Cast Corruption'),
(39194,0,4,5,6,0,100,0,0,0,0,0,11,73703,2,0,0,0,0,1,0,0,0,0,0,0,0,'Blastshadow the Brutemaster - On Death - Summon Soulstone Chest'),
(39194,0,5,0,61,0,100,0,0,0,0,0,45,0,2,0,0,0,0,19,39195,30,0,0,0,0,0,'Blastshadow the Brutemaster - On Death - Tell Delicia'),
(39195,0,0,0,38,0,100,0,0,1,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Delicia Whipsnaps - On Data Set 0 1 - Say Kill that goblin'),
(39195,0,1,0,38,0,100,0,0,2,0,0,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,'Delicia Whipsnaps - On Data Set 0 2 - Yell NOOOOOOO!');
