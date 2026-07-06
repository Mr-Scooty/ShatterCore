-- Sinestra P2/P3 completion (Bastion of Twilight, heroic-only)
-- Companion to the boss_sinestra.cpp Phase 2/3 implementation.

-- Sinestra spawn: heroic-only. Previous mask 6 (25N|10H) was wrong — she must
-- exist on 10H (4) and 25H (8) only.
UPDATE `creature` SET `spawnMask` = 12 WHERE `guid` = 9000200 AND `id` = 45213;

-- Twilight Essence pool: rebind the script (clobbered by the trailing REPLACE
-- in 2026_06_09_00_world.sql, which left ScriptName empty).
UPDATE `creature_template` SET
    `ScriptName` = 'npc_sinestra_twilight_essence',
    `AIName` = ''
WHERE `entry` = 48018;

-- Sinestra 25H difficulty wiring: 49744 is the retail 25-heroic clone
-- (HealthModifier 1500 = ~128.8M HP; instance_encounters 1083 already credits it).
UPDATE `creature_template` SET
    `difficulty_entry_3` = 49744,
    `mechanic_immune_mask` = 617299803
WHERE `entry` = 45213;

-- Fix the half-baked 49744 template (was level 1, faction 35). Difficulty
-- entries inherit the base entry's script, so ScriptName stays empty.
UPDATE `creature_template` SET
    `minlevel` = 88,
    `maxlevel` = 88,
    `exp` = 3,
    `faction` = 16,
    `unit_class` = 2,
    `unit_flags` = 32832,
    `unit_flags2` = 2048,
    `dynamicflags` = 0,
    `BaseAttackTime` = 2000,
    `RangeAttackTime` = 0,
    `speed_walk` = (SELECT s.`speed_walk` FROM (SELECT `speed_walk` FROM `creature_template` WHERE `entry` = 45213) s),
    `speed_run` = (SELECT s.`speed_run` FROM (SELECT `speed_run` FROM `creature_template` WHERE `entry` = 45213) s),
    `type_flags` = 270532716,
    `flags_extra` = 1,
    `mechanic_immune_mask` = 617299803,
    `ScriptName` = '',
    `AIName` = ''
WHERE `entry` = 49744;

-- Script bindings for the Phase 2/3 support NPCs.
UPDATE `creature_template` SET `ScriptName` = 'npc_sinestra_calen', `AIName` = '' WHERE `entry` = 46277;
UPDATE `creature_template` SET `ScriptName` = 'npc_sinestra_pulsing_twilight_egg', `AIName` = '' WHERE `entry` = 46842;
UPDATE `creature_template` SET `ScriptName` = 'npc_sinestra_twilight_spitecaller', `AIName` = '' WHERE `entry` = 48415;
UPDATE `creature_template` SET `ScriptName` = 'npc_sinestra_twilight_drake', `AIName` = '' WHERE `entry` = 48436;

-- Retail hotfix parity: whelps are immune to roots (1 << (MECHANIC_ROOT-1) = 64)
-- and snares (1 << (MECHANIC_SNARE-1) = 1024).
UPDATE `creature_template` SET
    `mechanic_immune_mask` = `mechanic_immune_mask` | 1088
WHERE `entry` IN (47265, 48047, 48048, 48049);

-- Pulsing Twilight Eggs: CC-proof like a boss, but stay selectable
-- (players tab them and see "immune" outside carapace windows, as on retail).
UPDATE `creature_template` SET
    `mechanic_immune_mask` = 617299803
WHERE `entry` = 46842;

-- Spell script bindings (Mana Barrier economy, Fiery Barrier zone, Pyrrhic Focus burn).
DELETE FROM `spell_script_names` WHERE `ScriptName` IN
    ('spell_sinestra_mana_barrier', 'spell_calen_fiery_barrier', 'spell_calen_pyrrhic_focus');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(87299, 'spell_sinestra_mana_barrier'),     -- Mana Barrier
(87229, 'spell_calen_fiery_barrier'),       -- Fiery Barrier (periodic zone application)
(87323, 'spell_calen_pyrrhic_focus');       -- Pyrrhic Focus (Calen self-burn)

-- Fiery Barrier cosmetic stalker (51608): template exists but had no spawn on 671.
-- One spawn at Calen's position carrying the dome visual (95791).
DELETE FROM `creature` WHERE `id` = 51608 AND `map` = 671;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `VerifiedBuild`) VALUES
(9000201, 51608, 671, 0, 0, 12, 0, 0, 0, 0, -1010.7465, -813.04865, 438.586, 0.628318, 7200, 0, 0, 0, 0, 0, 0, 0, 0, 0);

DELETE FROM `creature_addon` WHERE `guid` = 9000201;
INSERT INTO `creature_addon` (`guid`, `waypointPathId`, `cyclicSplinePathId`, `mount`, `StandState`, `AnimTier`, `VisFlags`, `SheathState`, `PvPFlags`, `emote`, `aiAnimKit`, `movementAnimKit`, `meleeAnimKit`, `visibilityDistanceType`, `auras`) VALUES
(9000201, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 4, '95791');
