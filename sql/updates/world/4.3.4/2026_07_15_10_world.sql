-- Naga Hide (24859) / Ruins of Vashj'elan: the naga had no combat or ambient AI (P3 sniff).
-- Vashj'elan Marauder 38359: Heroic Strike (57846), Enrage (8599) at low health.
-- Vashj'elan Siren 38360: Frostbolt (9672), Frost Nova (11831), ambient water cosmetic
-- channel (71802) out of combat.
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry` IN (38359,38360);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (38359,38360) AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(38359,0,0,0,0,0,100,0,3500,6000,7000,11000,11,57846,0,0,0,0,0,2,0,0,0,0,0,0,0,'Vashj''elan Marauder - In Combat - Cast Heroic Strike'),
(38359,0,1,0,2,0,100,1,0,30,0,0,11,8599,0,0,0,0,0,1,0,0,0,0,0,0,0,'Vashj''elan Marauder - At 30% Health - Cast Enrage (once)'),
(38360,0,0,0,0,0,100,0,2000,4000,3400,4800,11,9672,0,0,0,0,0,2,0,0,0,0,0,0,0,'Vashj''elan Siren - In Combat - Cast Frostbolt'),
(38360,0,1,0,9,0,100,0,0,8,10000,16000,11,11831,0,0,0,0,0,1,0,0,0,0,0,0,0,'Vashj''elan Siren - Enemy Within 8yd - Cast Frost Nova'),
(38360,0,2,0,1,0,100,0,5000,15000,30000,60000,11,71802,0,0,0,0,0,1,0,0,0,0,0,0,0,'Vashj''elan Siren - Out of Combat - Water Cosmetic Channel');
