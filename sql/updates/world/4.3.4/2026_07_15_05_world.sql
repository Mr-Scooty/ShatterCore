-- Weed Whacker (14236): the Vicious Vale plants had no combat AI at all (P3 sniff).
-- Poison Spitter 35896: Shoot Thorns (68208) + Poison Spit DoT (68207), bursts on death (68226).
-- Freezya 35897: Frostbolt (68209, damage + snare), bursts on death.
-- Strangle Vine 35995 (VehicleId 500): grabs its victim - 68264 force-casts 68267 (ride vine),
-- then Strangle (68265) ticks on passenger 0; the vine lets go after ~10s (remove 68267)
-- and native vehicle-death eject frees the player if the vine dies first.
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry` IN (35896,35897,35995);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (35896,35897,35995) AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(35896,0,0,0,0,0,100,0,2000,3500,3000,4500,11,68208,0,0,0,0,0,2,0,0,0,0,0,0,0,'Poison Spitter - In Combat - Cast Shoot Thorns'),
(35896,0,1,0,0,0,100,0,5000,9000,10000,14000,11,68207,0,0,0,0,0,2,0,0,0,0,0,0,0,'Poison Spitter - In Combat - Cast Poison Spit'),
(35896,0,2,0,6,0,100,0,0,0,0,0,11,68226,2,0,0,0,0,1,0,0,0,0,0,0,0,'Poison Spitter - On Death - Cast Plant Burst'),
(35897,0,0,0,0,0,100,0,1500,3000,2800,4000,11,68209,0,0,0,0,0,2,0,0,0,0,0,0,0,'Freezya - In Combat - Cast Frostbolt'),
(35897,0,1,0,6,0,100,0,0,0,0,0,11,68226,2,0,0,0,0,1,0,0,0,0,0,0,0,'Freezya - On Death - Cast Plant Burst'),
(35995,0,0,0,4,0,100,0,0,0,0,0,11,68264,2,0,0,0,0,2,0,0,0,0,0,0,0,'Strangle Vine - On Aggro - Cast Strangle Cover (grab)'),
(35995,0,1,0,0,0,100,0,16000,22000,16000,22000,11,68264,2,0,0,0,0,2,0,0,0,0,0,0,0,'Strangle Vine - In Combat - Re-grab victim'),
(35995,0,2,3,27,0,100,0,0,0,0,0,11,68265,2,0,0,0,0,1,0,0,0,0,0,0,0,'Strangle Vine - On Passenger Boarded - Cast Strangle'),
(35995,0,3,0,61,0,100,0,0,0,0,0,67,1,9000,12000,0,0,0,1,0,0,0,0,0,0,0,'Strangle Vine - On Passenger Boarded - Start release timer'),
(35995,0,4,0,59,0,100,0,1,0,0,0,28,68267,0,0,0,0,0,1,0,0,0,0,0,0,0,'Strangle Vine - Release Timer - Remove Strangle Ride aura'),
(35995,0,5,0,7,0,100,0,0,0,0,0,28,68267,0,0,0,0,0,1,0,0,0,0,0,0,0,'Strangle Vine - On Evade - Remove Strangle Ride aura');
