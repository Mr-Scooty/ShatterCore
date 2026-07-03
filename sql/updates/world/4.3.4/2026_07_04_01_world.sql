-- The Lost Isles (zone 4720): Part 1 - spawns, re-phases, gossip repairs,
-- vehicle/template deltas.

-- ----------------------------------------------------------------------------
-- 1) Missing questgivers and ladder-conflict clones (positions sniff-extracted).
-- ----------------------------------------------------------------------------
SET @CGUID := 9000400;
DELETE FROM `creature` WHERE `guid` BETWEEN @CGUID+0 AND @CGUID+9;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild`) VALUES
(@CGUID+0, 36608, 648, 4720, 4721, 1, 0, 1, 170, 0, -1, 0, 0, 551.2049, 3260.382, 0.5690133, 2.20629, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 15595), -- Doc Zapnozzle (14239 ender)
(@CGUID+1, 38928, 648, 4720, 0, 1, 0, 1, 183, 0, -1, 0, 0, 1163.5476, 1093.8887, 123.25151, 2.11185, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 15595), -- Sassy Hardwrench post-Volcanoth (24958 ender / 25023 giver)
(@CGUID+2, 39199, 648, 4720, 4923, 1, 0, 1, 183, 0, -1, 0, 0, 1775.6788, 1984.5188, 172.79369, 0.5, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 15595), -- Assistant Greely deep-mine (25122/25123/25125 giver)
(@CGUID+3, 35875, 648, 4720, 0, 1, 0, 1, 171, 0, -1, 0, 0, 532.09, 2684.59, 107.264, 4.24115, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 15595), -- Aggra @171 (14237 giver clone)
(@CGUID+4, 36112, 648, 4720, 4782, 1, 0, 1, 172, 0, -1, 0, 0, 1079.87, 3241.81, 80.8397, 2.60054, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 15595), -- Scout Brax @172 (14241 giver clone)
(@CGUID+5, 36145, 648, 4720, 4784, 1, 0, 1, 179, 0, -1, 0, 0, 993.276, 3852.15, 3.31015, 4.13643, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 15595), -- Thrall cliff base @179 (14326 giver clone)
(@CGUID+6, 38432, 648, 4720, 4873, 1, 0, 1, 181, 0, -1, 0, 0, 679.226, 2026.58, 50.2261, 3.80482, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 15595), -- Megs Dreadshredder @181 beach (24897 giver clone)
(@CGUID+7, 38381, 648, 4720, 4873, 1, 0, 1, 181, 0, -1, 0, 0, 676.993, 2026.60, 49.9124, 3.83972, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 15595), -- Brett "Coins" McQuid @181 (24859 straggler turn-ins)
(@CGUID+8, 38647, 648, 4720, 0, 1, 0, 1, 182, 0, -1, 0, 0, 715.191, 1829.92, 104.575, 0.872665, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 15595), -- Izzy @182 Oomlot (24937 giver clone)
(@CGUID+9, 38387, 648, 4720, 4870, 1, 0, 1, 183, 0, -1, 0, 0, 927.372, 2343.74, 5.80698, 4.03171, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 15595); -- Sassy town @183 (25058/25066/25098 giver clone)

-- Thrall/Aggra/Kilag orc-camp trio were mis-phased at 180 (they belong to the
-- post-Volcanoth era at the camp).
UPDATE `creature` SET `PhaseId` = 183 WHERE `guid` IN (392489, 392490, 392493);

-- Remove duplicate TDB spawns stacked at identical positions:
-- Geargrinder Gizmo (36600) at the arrival beach (keep 253763),
-- Trade Prince Gallywix/Thrall finale pair @184 (keep 394714/394712),
-- Chip Endale (39363) @184 (keep 394604, carries addon aura),
-- Pygmy Witchdoctor (35838) @170 (keep 390037).
DELETE FROM `creature_addon` WHERE `guid` IN (389369, 394995, 394994, 394905, 390301);
DELETE FROM `creature` WHERE `guid` IN (389369, 394995, 394994, 394905, 390301);

-- ----------------------------------------------------------------------------
-- 2) Gossip repairs: five sniffed quest-flow options are dead (OptionNpcFlag=0),
--    two NPCs point at the wrong root menu, and Sassy needs the set-sail option.
-- ----------------------------------------------------------------------------
UPDATE `gossip_menu_option` SET `OptionType` = 1, `OptionNpcflag` = 1
WHERE (`MenuID` = 10677 AND `OptionID` IN (0, 1))
   OR (`MenuID` = 10808 AND `OptionID` = 0)
   OR (`MenuID` = 11082 AND `OptionID` = 0)
   OR (`MenuID` = 11146 AND `OptionID` = 0)
   OR (`MenuID` = 11244 AND `OptionID` = 0);

UPDATE `creature_template` SET `gossip_menu_id` = 10677 WHERE `entry` = 35769; -- Dampwick: restore "I need another miner" root
UPDATE `creature_template` SET `gossip_menu_id` = 10808, `npcflag` = `npcflag` | 1 WHERE `entry` = 36425; -- Sassy @179: rocket re-offer
UPDATE `creature_template` SET `gossip_menu_id` = 11146, `npcflag` = `npcflag` | 1 WHERE `entry` = 38928; -- Sassy 38928: bomber hop

DELETE FROM `gossip_menu_option` WHERE `MenuID` = 12582 AND `OptionID` = 0;
INSERT INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcflag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES
(12582, 0, 0, 'Sassy, let''s set sail for Orgrimmar before the island explodes!', 0, 1, 1, 0, 0, 0, 0, NULL, 0, 15595);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 15 AND `SourceGroup` IN (10677, 10808, 11146, 12582);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(15, 10677, 0, 0, 0, 9, 0, 14021, 0, 0, 0, 0, 0, '', 'Dampwick: miner re-summon while Miner Troubles taken'),
(15, 10808, 0, 0, 0, 9, 0, 14244, 0, 0, 0, 0, 0, '', 'Sassy: rocket launch while Up, Up & Away! taken'),
(15, 11146, 0, 0, 0, 9, 0, 25023, 0, 0, 0, 0, 0, '', 'Sassy 38928: bomber hop while Old Friends taken'),
(15, 12582, 0, 0, 0, 9, 0, 25266, 0, 0, 0, 0, 0, '', 'Sassy: set-sail option while Warchief''s Emissary is in the log');

-- ----------------------------------------------------------------------------
-- 3) Vehicle template deltas (RecIDs sniff-extracted, all exist in 4.3.4
--    Vehicle.dbc) and spellclick flag/row fixes.
-- ----------------------------------------------------------------------------
UPDATE `creature_template` SET `VehicleId` = 530 WHERE `entry` = 36585; -- Bastia (To the Cliffs)
UPDATE `creature_template` SET `VehicleId` = 530 WHERE `entry` = 39152; -- Bastia (Let's Ride) - aligned with 36585
UPDATE `creature_template` SET `VehicleId` = 505 WHERE `entry` = 36143; -- Gyrochoppa
UPDATE `creature_template` SET `VehicleId` = 508 WHERE `entry` = 36178; -- Cyclone of the Elements
UPDATE `creature_template` SET `VehicleId` = 524 WHERE `entry` IN (36505, 36514); -- Sling Rockets
UPDATE `creature_template` SET `VehicleId` = 628 WHERE `entry` = 38318; -- Mechashark X-Steam
UPDATE `creature_template` SET `VehicleId` = 653 WHERE `entry` = 38802; -- Super Booster Rocket Boots
UPDATE `creature_template` SET `VehicleId` = 669 WHERE `entry` = 39074; -- Pride of Kezan
UPDATE `creature_template` SET `VehicleId` = 688 WHERE `entry` = 39329; -- Mine Cart (ride)
UPDATE `creature_template` SET `VehicleId` = 696 WHERE `entry` = 39598; -- Ultimate Footbomb Uniform
UPDATE `creature_template` SET `VehicleId` = 1293 WHERE `entry` = 47956; -- Footbomb Uniform (disguise)
UPDATE `creature_template` SET `VehicleId` = 802 WHERE `entry` = 39611; -- Battleworg
UPDATE `creature_template` SET `VehicleId` = 658 WHERE `entry` = 38918; -- Flying Bomber (borrowed from twin 38929)

-- Spellclick interaction bits (rows exist; the flag was missing).
UPDATE `creature_template` SET `npcflag` = `npcflag` | 16777216 WHERE `entry` IN (38111, 38412, 44580, 38526, 38918, 39456, 39592);

-- Naga Hatchling click spell 71916 does not exist in 4.3.4; 71919 does the
-- summon natively (credit handled by spell_lost_isles_pool_pony_click).
UPDATE `npc_spellclick_spells` SET `spell_id` = 71919 WHERE `npc_entry` IN (38412, 44580) AND `spell_id` = 71916;

-- Mechashark idle visual.
DELETE FROM `creature_template_addon` WHERE `entry` = 38318;
INSERT INTO `creature_template_addon` (`entry`, `waypointPathId`, `cyclicSplinePathId`, `mount`, `StandState`, `AnimTier`, `VisFlags`, `SheathState`, `PvPFlags`, `emote`, `aiAnimKit`, `movementAnimKit`, `meleeAnimKit`, `visibilityDistanceType`, `auras`) VALUES
(38318, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, '71663');
