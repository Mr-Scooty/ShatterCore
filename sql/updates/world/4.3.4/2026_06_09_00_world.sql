SET @CGUID := 9000200;

-- Sinestra
UPDATE `creature_template` SET
    `ScriptName` = 'boss_sinestra',
    `AIName` = '',
    `minlevel` = 88,
    `maxlevel` = 88,
    `faction` = 16,
    `unit_flags` = `unit_flags`|32832,
    `type_flags` = `type_flags`|270532716,
    `flags_extra` = `flags_extra`|1
WHERE `entry` = 45213;

-- Twilight Whelps (all difficulties)
UPDATE `creature_template` SET
    `ScriptName` = 'npc_sinestra_twilight_whelp',
    `AIName` = ''
WHERE `entry` IN (47265, 48047, 48048, 48049);

-- NOTE: Shadow Orb (NPC 49863) creature_template is created in 2026_01_16_00_world.sql
-- The INSERT there creates the full entry with correct flags and ScriptName

DELETE FROM `creature_text` WHERE `CreatureID` = 45213 AND `GroupID` IN (0, 1, 2, 3, 4, 5, 6);
DELETE FROM `creature_text` WHERE `CreatureID` = 46277 AND `GroupID` IN (0, 1, 2, 3, 4, 5);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
-- Sinestra
(45213, 0, 0, 'We were fools to entrust an imbecile like Cho\'gall with such a sacred duty! I will deal with you intruders myself!', 14, 0, 100, 0, 0, 20199, 0, 0, 'Sinestra - Aggro'),
(45213, 1, 0, 'Feed, children! Take your fill from their meaty husks!', 14, 0, 100, 0, 0, 20207, 0, 0, 'Sinestra - Spawn Whelps'),
(45213, 2, 0, 'I tire of this. Do you see this clutch amidst which you stand? I have nurtured the spark within them, but that life-force is and always will be mine. Behold, power beyond your comprehension!', 14, 0, 100, 0, 0, 20204, 0, 0, 'Sinestra - Phase 2 Transition'),
(45213, 3, 0, 'You mistake this for weakness? Fool!', 14, 0, 100, 0, 0, 20203, 0, 0, 'Sinestra - Weakness Comment'),
(45213, 4, 0, 'The barrier protecting the Pulsing Twilight Eggs dissipates as Sinestra harnesses their power!', 41, 0, 100, 0, 0, 0, 0, 0, 'Sinestra - Barrier Announcement'),
(45213, 5, 0, 'Enough! Drawing upon this source will set us back months. You should feel honored to be worthy of its expenditure. Now... die!', 14, 0, 100, 0, 0, 20206, 0, 0, 'Sinestra - Phase 3'),
(45213, 6, 0, 'Deathwing! I have fallen.... The brood... is lost.', 14, 0, 100, 0, 0, 20200, 0, 0, 'Sinestra - Death'),

-- Calen (Red Drake helper)
(46277, 0, 0, 'Heroes, you are not alone in this dark place!', 14, 0, 100, 0, 0, 21588, 0, 0, 'Calen - Intro'),
(46277, 1, 0, 'Sintharia, your master owes me a great debt -- one that I intend to extract from his consort\'s hide!', 14, 0, 100, 0, 0, 21590, 0, 0, 'Calen - Aggro'),
(46277, 2, 0, 'Heroes! My power wanes....', 14, 0, 100, 0, 0, 21589, 0, 0, 'Calen - Power Waning'),
(46277, 3, 0, 'All is lost.... Forgive me, my Queen....', 14, 0, 100, 0, 0, 21598, 0, 0, 'Calen - Death'),
(46277, 4, 0, 'You are weakening, Sintharia! Accept the inevitable!', 14, 0, 100, 0, 0, 21593, 0, 0, 'Calen - Sinestra Weakening'),
(46277, 5, 0, 'The fires dim, champions.... Take this, the last of my power. Succeed where I have failed. Avenge me. Avenge the world....', 14, 0, 100, 0, 0, 21591, 0, 0, 'Calen - Last Power');

-- Twilight Whelp Text
DELETE FROM `creature_text` WHERE `CreatureID` = 47265 AND `GroupID` IN (0, 1, 2);
DELETE FROM `creature_text` WHERE `CreatureID` = 48047 AND `GroupID` IN (0, 1, 2);
DELETE FROM `creature_text` WHERE `CreatureID` = 48048 AND `GroupID` IN (0, 1, 2);
DELETE FROM `creature_text` WHERE `CreatureID` = 48049 AND `GroupID` IN (0, 1, 2);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(47265, 0, 0, 'Twilight essence begins to bubble forth from the corpse of the whelp....', 16, 0, 100, 0, 0, 0, 0, 0, 'Twilight Whelp - Death Pool Spawn'),
(47265, 1, 0, '%s\'s corpse disintegrates into frothing twilight essence.', 16, 0, 100, 0, 0, 0, 0, 0, 'Twilight Whelp - Corpse Disintegration'),
(47265, 2, 0, '%s is revived by the commingled essences!', 16, 0, 100, 0, 0, 0, 0, 0, 'Twilight Whelp - Revival'),
(48047, 0, 0, 'Twilight essence begins to bubble forth from the corpse of the whelp....', 16, 0, 100, 0, 0, 0, 0, 0, 'Twilight Whelp - Death Pool Spawn'),
(48047, 1, 0, '%s is revived by the commingled essences!', 16, 0, 100, 0, 0, 0, 0, 0, 'Twilight Whelp - Revival'),
(48047, 2, 0, '%s\'s corpse disintegrates into frothing twilight essence.', 16, 0, 100, 0, 0, 0, 0, 0, 'Twilight Whelp - Corpse Disintegration'),
(48048, 0, 0, 'Twilight essence begins to bubble forth from the corpse of the whelp....', 16, 0, 100, 0, 0, 0, 0, 0, 'Twilight Whelp - Death Pool Spawn'),
(48048, 1, 0, '%s is revived by the commingled essences!', 16, 0, 100, 0, 0, 0, 0, 0, 'Twilight Whelp - Revival'),
(48048, 2, 0, '%s\'s corpse disintegrates into frothing twilight essence.', 16, 0, 100, 0, 0, 0, 0, 0, 'Twilight Whelp - Corpse Disintegration'),
(48049, 0, 0, 'Twilight essence begins to bubble forth from the corpse of the whelp....', 16, 0, 100, 0, 0, 0, 0, 0, 'Twilight Whelp - Death Pool Spawn'),
(48049, 1, 0, '%s is revived by the commingled essences!', 16, 0, 100, 0, 0, 0, 0, 0, 'Twilight Whelp - Revival'),
(48049, 2, 0, '%s\'s corpse disintegrates into frothing twilight essence.', 16, 0, 100, 0, 0, 0, 0, 0, 'Twilight Whelp - Corpse Disintegration');

-- Sinestra spawn location (Sinestra's lair below Cho'gall's platform)
DELETE FROM `creature` WHERE `id` = 45213 AND `map` = 671;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `VerifiedBuild`) VALUES
(@CGUID+0, 45213, 671, 0, 0, 6, 0, 0, 0, 0, -946.5, -730.995, 437.09033, 4.136430, 7200, 0, 0, 0, 0, 0, 0, 0, 0, 0);

-- Remove permanent Cache of the Broodmother spawn (should only spawn when Sinestra dies)
DELETE FROM `gameobject` WHERE `id` = 208045 AND `map` = 671;

-- Sinestra
UPDATE `creature_template` SET
    `ScriptName` = 'boss_sinestra',
    `AIName` = '',
    `minlevel` = 88,
    `maxlevel` = 88,
    `faction` = 16,
    `unit_flags` = 32832,      -- Standard boss flags (includes 0x8000 SWIMMING + 0x40 UNK_6)
    `unit_flags2` = 2048,       -- Standard flags2
    `RangeAttackTime` = 0,      -- Keep at 0 (default melee), spell casting handled by AI
    `BaseAttackTime` = 2000,    -- Standard attack time
    `type_flags` = 270532716,   -- Boss type flags
    `flags_extra` = 1           -- CREATURE_FLAG_EXTRA_INSTANCE_BIND
WHERE `entry` = 45213;

-- Ensure creature spawn exists and is not corrupted
DELETE FROM `creature` WHERE `id` = 45213 AND `map` = 671;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `VerifiedBuild`) VALUES
(9000200, 45213, 671, 0, 0, 6, 0, 0, 0, 0, -946.5, -730.995, 437.09033, 4.136430, 7200, 0, 0, 0, 0, 0, 0, 0, 0, 0);

-- Fix Twilight Whelps being invincible (stuck at 1 HP)
-- Issue: Whelps have CREATURE_STATIC_FLAG_UNKILLABLE (0x08) which prevents death
-- Root cause: Unit.cpp line 824-827 reduces damage to health-1 if creature has UNKILLABLE flag
-- Solution: Remove the UNKILLABLE flag from StaticFlags
--
-- Current StaticFlags = 2155872268 (0x8080000C) which includes 0x08 (UNKILLABLE)
-- Corrected StaticFlags = 2155872260 (0x80800004) without UNKILLABLE flag

UPDATE `creature_template` SET
    `StaticFlags` = 2155872260     -- Remove CREATURE_STATIC_FLAG_UNKILLABLE (0x08)
WHERE `entry` IN (47265, 48047, 48048, 48049);

-- Configure Twilight Essence Pool creature (NPC 48018)
-- This is the pool that spawns when Twilight Whelps die (via spell 88146 from client DBC)
-- The spell should handle the summoning automatically if the creature template is configured
UPDATE `creature_template` SET
    `minlevel` = 85,
    `maxlevel` = 85,
    `faction` = 35,                    -- Friendly to all
    `unit_flags` = 33554432,           -- UNIT_FLAG_NOT_SELECTABLE
    `flags_extra` = 128,               -- CREATURE_FLAG_EXTRA_TRIGGER (no threat, untargetable)
    `AIName` = 'NullCreatureAI'
WHERE `entry` = 48018;
-- Sinestra encounter: Twilight Essence pool (NPC 48018)
-- This creature spawns when Twilight Whelps die and creates a visual pool effect

-- First, ensure the creature_template entry exists with correct values
-- unit_flags: UNIT_FLAG_NON_ATTACKABLE (0x2) | UNIT_FLAG_NOT_SELECTABLE (0x2000000) = 33554434
-- flags_extra: 2 (CIVILIAN - not aggressive) - NOTE: Do NOT use 128 (TRIGGER) as it makes the NPC invisible
DELETE FROM `creature_template` WHERE `entry` = 48018;
INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `modelid1`, `modelid2`, `modelid3`, `modelid4`, `name`, `femaleName`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `exp_unk`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `scale`, `rank`, `dmgschool`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `trainer_type`, `trainer_class`, `trainer_race`, `type`, `type_flags`, `type_flags2`, `lootid`, `pickpocketloot`, `skinloot`, `resistance1`, `resistance2`, `resistance3`, `resistance4`, `resistance5`, `resistance6`, `spell1`, `spell2`, `spell3`, `spell4`, `spell5`, `spell6`, `spell7`, `spell8`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `HealthModifierExtra`, `ManaModifier`, `ManaModifierExtra`, `ArmorModifier`, `DamageModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES
(48018, 0, 0, 0, 0, 0, 11686, 0, 0, 0, 'Twilight Essence', '', '', '', 0, 87, 87, 3, 0, 16, 0, 1, 1.14286, 1, 0, 0, 2000, 2000, 1, 1, 1, 33554434, 2048, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 2, 'npc_sinestra_twilight_essence', 0);

-- Shadow Orb (NPC 49863) for Twilight Slicer mechanic
-- Two orbs spawn, fixate on random players, and create a damaging beam between them
-- unit_flags: UNIT_FLAG_NON_ATTACKABLE (0x2) | UNIT_FLAG_NOT_SELECTABLE (0x2000000) = 33554434
-- flags_extra: 2 (CIVILIAN) - Do NOT use 128 (TRIGGER) as it makes NPC invisible
-- Using displayid 31096 (orb model, same as Frozen Orb) for visibility
DELETE FROM `creature_template` WHERE `entry` = 49863;
INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `modelid1`, `modelid2`, `modelid3`, `modelid4`, `name`, `femaleName`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `exp_unk`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `scale`, `rank`, `dmgschool`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `trainer_type`, `trainer_class`, `trainer_race`, `type`, `type_flags`, `type_flags2`, `lootid`, `pickpocketloot`, `skinloot`, `resistance1`, `resistance2`, `resistance3`, `resistance4`, `resistance5`, `resistance6`, `spell1`, `spell2`, `spell3`, `spell4`, `spell5`, `spell6`, `spell7`, `spell8`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `HealthModifierExtra`, `ManaModifier`, `ManaModifierExtra`, `ArmorModifier`, `DamageModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES
(49863, 0, 0, 0, 0, 0, 31096, 0, 0, 0, 'Shadow Orb', '', '', '', 0, 87, 87, 3, 0, 16, 0, 1, 1.14286, 1, 0, 0, 2000, 2000, 1, 1, 1, 33554434, 2048, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 2, 'npc_sinestra_shadow_orb', 0);

-- Register spell scripts for Sinestra encounter
DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_sinestra_wrack');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(92955, 'spell_sinestra_wrack'),            -- Wrack (25-man)
(89421, 'spell_sinestra_wrack');            -- Wrack (10-man)
-- Sinestra: ensure Twilight Whelps use script and are killable
UPDATE `creature_template` SET
    `ScriptName` = 'npc_sinestra_twilight_whelp',
    `AIName` = ''
WHERE `entry` IN (47265, 48047, 48048, 48049);

UPDATE `creature_template` SET
    `StaticFlags` = `StaticFlags` & ~8
WHERE `entry` IN (47265, 48047, 48048, 48049);

-- Sinestra: ensure Twilight Essence pool is visible (do not use CREATURE_FLAG_EXTRA_TRIGGER)
REPLACE INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `modelid1`, `modelid2`, `modelid3`, `modelid4`, `name`, `femaleName`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `exp_unk`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `scale`, `rank`, `dmgschool`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `trainer_type`, `trainer_class`, `trainer_race`, `type`, `type_flags`, `type_flags2`, `lootid`, `pickpocketloot`, `skinloot`, `resistance1`, `resistance2`, `resistance3`, `resistance4`, `resistance5`, `resistance6`, `spell1`, `spell2`, `spell3`, `spell4`, `spell5`, `spell6`, `spell7`, `spell8`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `HealthModifierExtra`, `ManaModifier`, `ManaModifierExtra`, `ArmorModifier`, `DamageModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES
(48018, 0, 0, 0, 0, 0, 11686, 0, 0, 0, 'Twilight Essence', '', '', '', 0, 87, 87, 3, 0, 16, 0, 1, 1.14286, 1, 0, 0, 2000, 2000, 1, 1, 1, 33554434, 2048, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 2, '', 0);
