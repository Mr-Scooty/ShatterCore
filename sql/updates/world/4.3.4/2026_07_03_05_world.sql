-- Kezan quest chain: Batch E - the finale
-- 14125 "447" / 14126 Life Savings / yacht escape to the Lost Isles

-- ----------------------------------------------------------------------------
-- 1) 447 arson targets: the three quest goobers (native GO credit via Data1)
--    belong to the evacuation era, not the base city.
-- ----------------------------------------------------------------------------
UPDATE `gameobject` SET `PhaseId` = 384 WHERE `id` IN (201733, 201734, 201735) AND `map` = 648;

-- On use each house goober spawns a 447 Fire and gasses the arsonist (cosmetic;
-- the quest credit itself is native goober behavior).
UPDATE `gameobject_template` SET `AIName` = 'SmartGameObjectAI' WHERE `entry` IN (201733, 201734, 201735);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (201733, 201734, 201735) AND `source_type` = 1;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(201733, 1, 0, 1, 64, 0, 100, 0, 0, 0, 0, 0, 0, 50, 201745, 60, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Leaky Stove - On Used - Summon 447 Fire'),
(201733, 1, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 11, 70223, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Leaky Stove - On Used - Cast Gas visual on invoker'),
(201734, 1, 0, 1, 64, 0, 100, 0, 0, 0, 0, 0, 0, 50, 201745, 60, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Flammable Bed - On Used - Summon 447 Fire'),
(201734, 1, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 11, 70223, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Flammable Bed - On Used - Cast Gas visual on invoker'),
(201735, 1, 0, 1, 64, 0, 100, 0, 0, 0, 0, 0, 0, 50, 201745, 60, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Defective Generator - On Used - Summon 447 Fire'),
(201735, 1, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 11, 70223, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Defective Generator - On Used - Cast Gas visual on invoker');

-- ----------------------------------------------------------------------------
-- 2) Gasbot: player-summoned via the control panel (GameObjectScript), follows
--    its summoner and detonates at the KTC building for the fourth credit.
-- ----------------------------------------------------------------------------
UPDATE `creature_template` SET `VehicleId` = 590, `npcflag` = 0, `ScriptName` = 'npc_gasbot' WHERE `entry` = 37598;
-- Keep the (37598, 46598) spellclick row: the accessory loader requires one for
-- vehicles with vehicle_template_accessory data (the four gas targets), and
-- npcflag=0 already prevents players from spell-clicking the bot.
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 37598;
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(37598, 46598, 1, 0);
-- The Gasbot is a summoned helper, not a questgiver (Sassy starts 14125).
DELETE FROM `creature_queststarter` WHERE `id` = 37598 AND `quest` = 14125;
UPDATE `gameobject_template` SET `ScriptName` = 'go_gasbot_control_panel' WHERE `entry` = 201736;

-- Static hazard markers at the three arson houses (gas visual bunnies).
UPDATE `creature_template` SET `AIName` = 'SmartAI', `unit_flags` = 33555200 WHERE `entry` IN (37561, 37590, 37594);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (37561, 37590, 37594) AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(37561, 0, 0, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 75, 70226, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Overloaded Generator - On Respawn - Add Smoldering aura'),
(37590, 0, 0, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 75, 70226, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Stove Leak - On Respawn - Add Smoldering aura'),
(37594, 0, 0, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 75, 70226, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Smoldering Bed - On Respawn - Add Smoldering aura');

SET @CGUID := 9000358;
DELETE FROM `creature` WHERE `guid` BETWEEN @CGUID+0 AND @CGUID+2;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild`) VALUES
(@CGUID+0, 37561, 648, 4737, 4765, 1, 0, 1, 384, 0, -1, 0, 0, -8420.861, 1372.6111, 105.75543, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595), -- Overloaded Generator
(@CGUID+1, 37590, 648, 4737, 4765, 1, 0, 1, 384, 0, -1, 0, 0, -8402.417, 1371.3733, 105.70275, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595), -- Stove Leak
(@CGUID+2, 37594, 648, 4737, 4765, 1, 0, 1, 384, 0, -1, 0, 0, -8402.31, 1363.6007, 118.293495, 0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 15595); -- Smoldering Bed

-- ----------------------------------------------------------------------------
-- 3) Claims Adjuster: drop the legacy phase-aura removal (68480) wired to the
--    447 turn-in; phasing is handled by phase_area conditions now.
-- ----------------------------------------------------------------------------
UPDATE `smart_scripts` SET `link` = 0 WHERE `entryorguid` = 37602 AND `source_type` = 0 AND `id` = 1;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 37602 AND `source_type` = 0 AND `id` = 2;

-- ----------------------------------------------------------------------------
-- 4) Megs Dreadshredder whispers the Robbing Hoods hint (existing text group 3)
--    when the quest is accepted.
-- ----------------------------------------------------------------------------
DELETE FROM `smart_scripts` WHERE `entryorguid` = 34874 AND `source_type` = 0 AND `id` = 10;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(34874, 0, 10, 0, 19, 0, 100, 0, 14121, 0, 0, 0, 0, 1, 3, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Megs Dreadshredder - On Quest 14121 Taken - Whisper Hot Rod hint');

-- ----------------------------------------------------------------------------
-- 5) Life Savings: the yacht boarding mortar (GO 207355, native goober spell
--    92629) launches the player onto the yacht deck via 92633 (JUMP_DEST with
--    an existing spell_target_position row). The SpellScript bridges the two.
-- ----------------------------------------------------------------------------
DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_kezan_yacht_mortar';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(92629, 'spell_kezan_yacht_mortar');
