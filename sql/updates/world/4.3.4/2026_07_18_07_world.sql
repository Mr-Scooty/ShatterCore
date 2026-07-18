-- Kelpthar: Budd shark arc (25651/25657/27699/25670/25732/25743). Retail sniff crosscheck.
-- Phases: 142 = Gnaws' Tooth GOs, 122 = hat-Budd + corpse scene + Pewter Prophet, 123 = farewell Budd.

-- ==================== 1) Vehicle kit corrections (sniff create-block RecIDs) ====================
-- The kits belong on the event vehicles; the passenger bunnies ride via scripted 46598 casts.
-- Corrects 2026_07_18_00 rows that followed the earlier (wrong) mapping.
UPDATE `creature_template` SET `VehicleId`=803 WHERE `entry`=41051; -- Bait Bunny
UPDATE `creature_template` SET `VehicleId`=804 WHERE `entry`=41057; -- Gnaws (event I)
UPDATE `creature_template` SET `VehicleId`=807 WHERE `entry`=41093; -- Player-Bait
UPDATE `creature_template` SET `VehicleId`=808 WHERE `entry`=41098; -- Gnaws (event II)
UPDATE `creature_template` SET `VehicleId`=0 WHERE `entry` IN (41048,41150,41154,46403);

-- ==================== 2) Quest flags ====================
UPDATE `quest_template` SET `Flags`=`Flags`|0x400000 WHERE `ID` IN (25657,27699,25670,25743);

-- ==================== 3) Phasing (Skeletal Reef 5054 + Gurboggle's Ledge 5053) ====================
DELETE FROM `phase_area` WHERE `AreaId` IN (5054,5053) AND `PhaseId` IN (142,122,123);
INSERT INTO `phase_area` (`AreaId`, `PhaseId`, `Comment`) VALUES
(5054, 142, 'Skeletal Reef: Gnaws tooth GOs'),
(5053, 142, 'Gurboggles Ledge: Gnaws tooth GOs'),
(5054, 122, 'Skeletal Reef: hat-Budd, Gnaws corpse, Pewter Prophet'),
(5053, 122, 'Gurboggles Ledge: Gnaws corpse scene'),
(5054, 123, 'Skeletal Reef: farewell Budd');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=26 AND `SourceGroup` IN (142,122,123) AND `SourceEntry` IN (5054,5053);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(26, 142, 5054, 0, 0, 8, 0, 25657, 0, 0, 0, 0, 0, '', 'Tooth GOs: Dah Nunt rewarded'),
(26, 142, 5054, 0, 0, 8, 0, 27699, 0, 0, 1, 0, 0, '', 'Tooth GOs: Shark Weak not yet rewarded'),
(26, 142, 5053, 0, 0, 8, 0, 25657, 0, 0, 0, 0, 0, '', 'Tooth GOs: Dah Nunt rewarded'),
(26, 142, 5053, 0, 0, 8, 0, 27699, 0, 0, 1, 0, 0, '', 'Tooth GOs: Shark Weak not yet rewarded'),
(26, 122, 5054, 0, 0, 8, 0, 25670, 0, 0, 0, 0, 0, '', 'Hat-Budd scene: DUN-dun rewarded'),
(26, 122, 5054, 0, 0, 47, 0, 25743, 66, 0, 1, 0, 0, '', 'Hat-Budd scene: Decisions not done'),
(26, 122, 5053, 0, 0, 8, 0, 25670, 0, 0, 0, 0, 0, '', 'Corpse scene: DUN-dun rewarded'),
(26, 122, 5053, 0, 0, 47, 0, 25743, 66, 0, 1, 0, 0, '', 'Corpse scene: Decisions not done'),
(26, 123, 5054, 0, 0, 47, 0, 25743, 66, 0, 0, 0, 0, '', 'Farewell Budd: Decisions complete');

-- Existing always-on spawns move into the scene phase
UPDATE `creature` SET `PhaseId`=122 WHERE `guid` IN (349701,349598); -- 46458 hat-Budd, 41192 Pewter Prophet

-- ==================== 4) Spawns ====================
DELETE FROM `creature` WHERE `guid` BETWEEN 9001078 AND 9001080;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild`) VALUES
(9001078, 46338, 0, 4815, 5070, 1, 0, 1, 169, 0, -1, 0, 0, -4745.842, 3521.846, -108.300, 1.745, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 0),
(9001079, 46463, 0, 4815, 5054, 1, 0, 1, 123, 0, -1, 0, 0, -4928.320, 3434.210, -116.670, 1.850, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 0),
(9001080, 41157, 0, 4815, 5053, 1, 0, 1, 122, 0, -1, 0, 0, -5126.347, 3383.204, -110.986, 3.176, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 0);

DELETE FROM `gameobject` WHERE `guid` BETWEEN 9001014 AND 9001028;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `VerifiedBuild`) VALUES
(9001014, 203170, 0, 4815, 5054, 1, 0, 1, 142, 0, -1, -4981.700, 3407.990, -105.070, 0, 0, 0, 0, 1, 300, 255, 1, '', 0),
(9001015, 203170, 0, 4815, 5054, 1, 0, 1, 142, 0, -1, -4975.210, 3422.140, -108.060, 0, 0, 0, 0, 1, 300, 255, 1, '', 0),
(9001016, 203170, 0, 4815, 5054, 1, 0, 1, 142, 0, -1, -4963.870, 3422.040, -108.360, 0, 0, 0, 0, 1, 300, 255, 1, '', 0),
(9001017, 203170, 0, 4815, 5054, 1, 0, 1, 142, 0, -1, -4959.670, 3434.100, -110.680, 0, 0, 0, 0, 1, 300, 255, 1, '', 0),
(9001018, 203170, 0, 4815, 5054, 1, 0, 1, 142, 0, -1, -4949.140, 3456.860, -120.510, 0, 0, 0, 0, 1, 300, 255, 1, '', 0),
(9001019, 203170, 0, 4815, 5054, 1, 0, 1, 142, 0, -1, -4934.080, 3456.260, -118.760, 0, 0, 0, 0, 1, 300, 255, 1, '', 0),
(9001020, 203170, 0, 4815, 5054, 1, 0, 1, 142, 0, -1, -4924.220, 3437.800, -117.300, 0, 0, 0, 0, 1, 300, 255, 1, '', 0),
(9001021, 203170, 0, 4815, 5054, 1, 0, 1, 142, 0, -1, -4923.660, 3410.050, -116.170, 0, 0, 0, 0, 1, 300, 255, 1, '', 0),
(9001022, 203170, 0, 4815, 5054, 1, 0, 1, 142, 0, -1, -4921.840, 3383.880, -111.180, 0, 0, 0, 0, 1, 300, 255, 1, '', 0),
(9001023, 203170, 0, 4815, 5054, 1, 0, 1, 142, 0, -1, -4918.080, 3393.850, -113.050, 0, 0, 0, 0, 1, 300, 255, 1, '', 0),
(9001024, 203170, 0, 4815, 5054, 1, 0, 1, 142, 0, -1, -4910.720, 3425.200, -116.670, 0, 0, 0, 0, 1, 300, 255, 1, '', 0),
(9001025, 203170, 0, 4815, 5054, 1, 0, 1, 142, 0, -1, -4900.040, 3422.140, -115.460, 0, 0, 0, 0, 1, 300, 255, 1, '', 0),
(9001026, 203170, 0, 4815, 5054, 1, 0, 1, 142, 0, -1, -4896.680, 3409.810, -111.330, 0, 0, 0, 0, 1, 300, 255, 1, '', 0),
(9001027, 203170, 0, 4815, 5054, 1, 0, 1, 142, 0, -1, -4890.010, 3459.170, -114.290, 0, 0, 0, 0, 1, 300, 255, 1, '', 0),
(9001028, 203170, 0, 4815, 5054, 1, 0, 1, 142, 0, -1, -4876.460, 3453.780, -110.690, 0, 0, 0, 0, 1, 300, 255, 1, '', 0);

-- ==================== 5) Loot ====================
DELETE FROM `gameobject_loot_template` WHERE `Entry`=29543 AND `Item`=62228;
INSERT INTO `gameobject_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`) VALUES
(29543, 62228, 0, 25, 1, 1, 0, 1, 1, 'Pilfered Cannonballs - Barrel of Gunpowder');
UPDATE `creature_loot_template` SET `QuestRequired`=1 WHERE `Entry`=41018 AND `Item`=55805;

-- ==================== 6) Event summon destinations (sniff coords) ====================
DELETE FROM `spell_target_position` WHERE `ID` IN (76707,76819,76747);
INSERT INTO `spell_target_position` (`ID`, `EffectIndex`, `MapID`, `PositionX`, `PositionY`, `PositionZ`, `Orientation`, `VerifiedBuild`) VALUES
(76707, 0, 0, -4995.885, 3393.267, -86.080, 5.524, 0), -- Summon Gnaws (event I)
(76819, 0, 0, -4996.150, 3393.520, -86.410, 5.074, 0), -- Summon Gnaws II
(76747, 0, 0, -4929.500, 3435.600, -115.400, 0.0, 0);  -- Gnaws KC teleport back to reef floor
