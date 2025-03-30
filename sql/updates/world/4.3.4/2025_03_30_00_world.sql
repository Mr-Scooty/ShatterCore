-- Add the Lorna Horse Trigger creature template
DELETE FROM `creature_template` WHERE `entry` = 800100;
INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `modelid1`, `modelid2`, `name`, `femaleName`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `scale`, `rank`, `dmgschool`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `DamageModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`, `ScriptName`, `VerifiedBuild`) 
VALUES (800100, 0, 0, 0, 0, 0, 11686, 0, 'Lorna Crowley Trigger', '', '', '', 0, 1, 1, 0, 35, 0, 1, 1.14286, 1, 0, 0, 2000, 2000, 1, 1, 1, 33554432, 2048, 0, 0, 10, 1024, 0, 0, 0, 0, 0, 0, '', 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 130, 'npc_lorna_horse_trigger', 0);

-- Spawn the trigger at Lorna's coordinates
-- Lorna's known coordinates are: -2059.70, 2254.17, 22.57
DELETE FROM `creature` WHERE `id` = 800100;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild`) 
VALUES (8001000, 800100, 654, 4714, 4714, 1, 0, 2, 0, 0, -1, 11686, 0, -2059.70, 2254.17, 22.57, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 0);

-- Make sure the invisible bunny model is available
UPDATE `creature_template` SET `modelid1` = 11686, `modelid2` = 11686 WHERE `entry` = 800100;

-- Make the trigger have a larger collision box to ensure it detects horses
UPDATE `creature_model_info` SET `BoundingRadius` = 10, `CombatReach` = 10 WHERE `DisplayID` = 11686;

-- Update Lorna Crowley's coords if needed (just to be extra safe)
UPDATE `creature` SET `position_x` = -2059.70, `position_y` = 2254.17, `position_z` = 22.57 WHERE `id` = 36457 AND ABS(`position_x` - (-2059.70)) < 5 AND ABS(`position_y` - 2254.17) < 5;

-- Add condition for Mountain Horse NPC so it can only be mounted by players with "The Hungry Ettin" quest
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 18 AND `SourceGroup` = 36540 AND `SourceEntry` = 94654;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) 
VALUES (18, 36540, 94654, 0, 0, 9, 0, 14416, 0, 0, 0, 0, 0, '', 'Mountain Horse - Only allow spell click if player has The Hungry Ettin quest');

-- Make "The Hungry Ettin" (14416) a prerequisite for "Horses for Duskhaven" (14463)
UPDATE `quest_template_addon` SET `PrevQuestID` = 14416 WHERE `ID` = 14463;