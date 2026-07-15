-- Free the Captives (24925) / Send a Message (24929): P4-sniff fixes.

-- The 12 Oomlot Shamans channel their captive-drain, but no Goblin Captives existed near
-- them (the single 38812 spawn belongs to the Gallywix mine). Spawn one kneeling captive
-- in front of each shaman; killing the shaman frees the captive, who bolts and despawns.
DELETE FROM `creature` WHERE `guid` BETWEEN 9000885 AND 9000896;
INSERT INTO `creature` (`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseUseFlags`,`phaseMask`,`PhaseId`,`PhaseGroup`,`position_x`,`position_y`,`position_z`,`orientation`,`spawntimesecs`,`wander_distance`,`MovementType`) VALUES
(9000885,38812,648,0,0,1,0,0,181,0,744.359,1754.137,115.167,5.8643,300,0,0),
(9000886,38812,648,0,0,1,0,0,181,0,742.543,1702.643,116.025,2.0245,300,0,0),
(9000887,38812,648,0,0,1,0,0,181,0,741.023,1732.938,114.071,4.1015,300,0,0),
(9000888,38812,648,0,0,1,0,0,181,0,714.016,1719.202,115.081,1.8851,300,0,0),
(9000889,38812,648,0,0,1,0,0,181,0,788.297,1730.727,120.560,3.3337,300,0,0),
(9000890,38812,648,0,0,1,0,0,181,0,762.575,1733.757,118.773,5.5850,300,0,0),
(9000891,38812,648,0,0,1,0,0,181,0,768.678,1697.101,125.126,6.2307,300,0,0),
(9000892,38812,648,0,0,1,0,0,181,0,717.550,1662.144,121.202,1.8151,300,0,0),
(9000893,38812,648,0,0,1,0,0,181,0,766.592,1677.375,126.096,5.9342,300,0,0),
(9000894,38812,648,0,0,1,0,0,181,0,801.837,1696.063,125.786,1.6230,300,0,0),
(9000895,38812,648,0,0,1,0,0,181,0,694.527,1640.680,116.478,0.7157,300,0,0),
(9000896,38812,648,0,0,1,0,0,181,0,776.647,1657.361,126.815,3.4383,300,0,0);

-- Captives kneel while drained.
DELETE FROM `creature_addon` WHERE `guid` BETWEEN 9000885 AND 9000896;
INSERT INTO `creature_addon` (`guid`,`emote`) VALUES
(9000885,68),(9000886,68),(9000887,68),(9000888,68),(9000889,68),(9000890,68),
(9000891,68),(9000892,68),(9000893,68),(9000894,68),(9000895,68),(9000896,68);

-- Shaman: keep the drain channel, add Earth Shock (sniffed) and free the captive on death.
DELETE FROM `smart_scripts` WHERE `entryorguid`=38644 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(38644,0,0,0,1,0,100,0,2000,5000,15000,25000,11,72518,0,0,0,0,0,19,38812,10,0,0,0,0,0,'Oomlot Shaman - OOC - Channel drain on captive'),
(38644,0,1,0,0,0,100,0,3000,5000,8000,12000,11,13281,0,0,0,0,0,2,0,0,0,0,0,0,0,'Oomlot Shaman - In Combat - Cast Earth Shock'),
(38644,0,2,0,6,0,100,0,0,0,0,0,45,0,1,0,0,0,0,19,38812,15,0,0,0,0,0,'Oomlot Shaman - On Death - Free the nearby captive');

-- Freed captive bolts toward the village entrance and despawns.
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=38812;
DELETE FROM `smart_scripts` WHERE `entryorguid`=38812 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(38812,0,0,1,38,0,100,0,0,1,0,0,90,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Goblin Captive - On Data Set - Stand Up'),
(38812,0,1,2,61,0,100,0,0,0,0,0,59,1,0,0,0,0,0,1,0,0,0,0,0,0,0,'Goblin Captive - Linked - Set Run'),
(38812,0,2,3,61,0,100,0,0,0,0,0,69,1,0,0,0,0,0,8,0,0,0,688.0,1614.0,114.0,0,'Goblin Captive - Linked - Run to the village entrance'),
(38812,0,3,0,61,0,100,0,0,0,0,0,41,7000,0,0,0,0,0,1,0,0,0,0,0,0,0,'Goblin Captive - Linked - Despawn');

-- Yngwie (24929): voodoo illusion trick (75942 summons 40722 + hallucination) and his
-- Oomlot yells, previously orphaned; all five lines merged into one random group.
UPDATE `creature_text` SET `GroupID`=0, `ID`=1 WHERE `CreatureID`=38696 AND `GroupID`=1;
UPDATE `creature_text` SET `GroupID`=0, `ID`=2 WHERE `CreatureID`=38696 AND `GroupID`=2;
UPDATE `creature_text` SET `GroupID`=0, `ID`=3 WHERE `CreatureID`=38696 AND `GroupID`=3;
UPDATE `creature_text` SET `GroupID`=0, `ID`=4 WHERE `CreatureID`=38696 AND `GroupID`=4;
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=38696;
DELETE FROM `smart_scripts` WHERE `entryorguid`=38696 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(38696,0,0,0,4,0,100,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Yngwie - On Aggro - Oomlot Yell'),
(38696,0,1,0,0,0,100,0,10000,16000,14000,20000,11,75942,0,0,0,0,0,1,0,0,0,0,0,0,0,'Yngwie - In Combat - Cast Voodoo Illusion'),
(38696,0,2,0,0,0,100,60,18000,26000,20000,30000,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Yngwie - In Combat - Oomlot Yell');
