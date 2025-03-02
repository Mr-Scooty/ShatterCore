-- Set the Round Up Horse spell for the Mountain Horse
UPDATE `creature_template` SET `spell1` = 68903 WHERE `entry` = 36540;

-- Make sure the Mountain Horse is scriptable
UPDATE `creature_template` SET `ScriptName` = 'npc_mountain_horse' WHERE `entry` = 36540;

-- Add the spell to spell_script_names if it doesn't exist
DELETE FROM `spell_script_names` WHERE `spell_id` = 68903;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (68903, 'spell_round_up_horse');

-- Create the follower version of the Mountain Horse
DELETE FROM `creature_template` WHERE `entry` = 36555;
INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `modelid1`, `modelid2`, `modelid3`, `modelid4`, `name`, `femaleName`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `exp_unk`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `scale`, `rank`, `dmgschool`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `trainer_type`, `trainer_class`, `trainer_race`, `type`, `type_flags`, `type_flags2`, `lootid`, `pickpocketloot`, `skinloot`, `resistance1`, `resistance2`, `resistance3`, `resistance4`, `resistance5`, `resistance6`, `spell1`, `spell2`, `spell3`, `spell4`, `spell5`, `spell6`, `spell7`, `spell8`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `HealthModifierExtra`, `ManaModifier`, `ManaModifierExtra`, `ArmorModifier`, `DamageModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`, `StaticFlags`, `StaticFlags2`, `StaticFlags3`, `StaticFlags4`, `StaticFlags5`, `ScriptName`, `VerifiedBuild`) 
SELECT `entry`+15, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `modelid1`, `modelid2`, `modelid3`, `modelid4`, CONCAT(`name`, ' (Follower)'), `femaleName`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `exp_unk`, `faction`, 0, `speed_walk`, `speed_run` * 1.1, `scale`, `rank`, `dmgschool`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `trainer_type`, `trainer_class`, `trainer_race`, `type`, `type_flags` & ~2048, `type_flags2`, `lootid`, `pickpocketloot`, `skinloot`, `resistance1`, `resistance2`, `resistance3`, `resistance4`, `resistance5`, `resistance6`, 0, `spell2`, `spell3`, `spell4`, `spell5`, `spell6`, `spell7`, `spell8`, `PetSpellDataId`, 0, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `HealthModifierExtra`, `ManaModifier`, `ManaModifierExtra`, `ArmorModifier`, `DamageModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`, `StaticFlags`, `StaticFlags2`, `StaticFlags3`, `StaticFlags4`, `StaticFlags5`, 'npc_mountain_horse_follower', `VerifiedBuild` 
FROM `creature_template` WHERE `entry` = 36540;

-- Make sure the follower version is not a vehicle
UPDATE `creature_template` SET `type_flags` = `type_flags` & ~2048, `VehicleId` = 0, `npcflag` = 0 WHERE `entry` = 36555; 