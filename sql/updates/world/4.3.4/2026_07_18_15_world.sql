-- Shimmering reconciliation: the C++ module owns the Nespirah tunnel entry (areatrigger
-- script summons the escort) and the Quel'Dormir bridge finale (npc_vashjir_bridge_controller
-- full 183/184/185 ladder) - remove the overlapping data-side paths from batch B.

-- Escort double-summon guard: AT script at_nespirah_tunnel replaces the spell_area summon
DELETE FROM `spell_area` WHERE `spell`=77963 AND `area`=4962;

-- East-bridge 60s-hold backstop (guid-scoped SAI casting 78329) superseded by the controller
DELETE FROM `smart_scripts` WHERE `entryorguid`=-9001437 AND `source_type`=0;
DELETE FROM `smart_scripts` WHERE `entryorguid`=4078920 AND `source_type`=9;

-- Defense controller bunny is event-phase only (its AI adds the second phase)
UPDATE `creature` SET `PhaseId`=171 WHERE `guid`=9001232;

-- Bridge controller spawn (phase 183; AI adds 184) at the south bridge post
DELETE FROM `creature` WHERE `guid`=9001450;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild`) VALUES
(9001450, 42135, 0, 5144, 4968, 1, 0, 1, 183, 0, -1, 0, 0, -7300.68, 4823.61, -284.88, 0, 120, 0, 0, 1, 0, 0, 0, 0, 0, '', 0);
