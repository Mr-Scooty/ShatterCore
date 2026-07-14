--
-- Don't Go Into the Light! (14239): implement the retail resuscitation
-- vignette from the P2 sniff.
--
-- Retail flow: on washing ashore the player self-casts 69010 "Near Death!"
-- (native stun + invisibility type 7 + lying-down visual) via spell_area,
-- which autocasts 69018 "Summon Doc Zapnozzle" - a per-player personal-spawn
-- summon (SummonProperties 3052, flag 0x10) that spawns on top of the
-- shipwreck, runs down to the shore, hops onto the barrel next to the player
-- and runs the jumper-cables vignette targeting his summoner. Turning in
-- 14239 casts 69013 (reward spell), which strips Near Death (unroot + stand
-- up), and Doc says his farewells and swims off toward the shore camp.
--

-- Spell scripts: 69018 dedupes the private Doc, 69013 removes Near Death.
DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_lost_isles_summon_doc_zapnozzle','spell_lost_isles_dont_go_into_the_light');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(69018, 'spell_lost_isles_summon_doc_zapnozzle'),
(69013, 'spell_lost_isles_dont_go_into_the_light');

-- Doc is a per-player summon on retail - drop the permanent spawn that stood
-- underwater at the mid-run spline position imported from the sniff.
DELETE FROM `creature` WHERE `guid`=9000400 AND `id`=36608;
DELETE FROM `creature_addon` WHERE `guid`=9000400;

-- Summon destination anchor: 69018 uses TARGET_DEST_NEARBY_ENTRY (46), so it
-- needs a nearby-entry unit to borrow its position from. Invisible trigger on
-- top of the shipwreck at the sniffed summon point.
DELETE FROM `creature` WHERE `guid`=9000884;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild`) VALUES
(9000884, 21252, 648, 4720, 4721, 1, 0, 1, 170, 0, -1, 0, 0, 552.5913, 3258.5024, 10.2924, 2.2063, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 15595);

-- Implicit-target conditions: summon dest = the wreck trigger; the forced
-- zap 69086 (from 69085) targets Geargrinder Gizmo.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry` IN (69018, 69086);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 69018, 0, 0, 31, 0, 3, 21252, 0, 0, 0, 0, '', 'Summon Doc Zapnozzle - dest at the shipwreck world trigger'),
(13, 1, 69086, 0, 0, 31, 0, 3, 36600, 0, 0, 0, 0, '', 'Don''t Go Into the Light - forced zap targets Geargrinder Gizmo');

-- Near Death + Doc summon while 14239 is not rewarded (status mask 11 =
-- NONE|COMPLETE|INCOMPLETE), goblins (racemask 256) in the crash site (4721).
-- 69018 is chained off the 69010 aura via aura_spell, which also re-summons
-- Doc after a relog while still Near Death.
DELETE FROM `spell_area` WHERE `spell` IN (69010, 69018) AND `area`=4721;
INSERT INTO `spell_area` (`spell`, `area`, `quest_start`, `quest_end`, `aura_spell`, `racemask`, `gender`, `flags`, `quest_start_status`, `quest_end_status`) VALUES
(69010, 4721, 14239, 0, 0, 256, 2, 3, 11, 11),
(69018, 4721, 14239, 0, 69010, 256, 2, 1, 11, 11);

-- 14239 requires the Kezan finale.
UPDATE `quest_template_addon` SET `PrevQuestID`=14126 WHERE `ID`=14239;

-- Doc Zapnozzle: entrance + vignette + farewell (timings from the sniff).
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (36608, 36600) AND `source_type`=0;
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (3660800, 3660801, 3660000) AND `source_type`=9;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
-- Entrance: store the summoner, run down the wreck, hop onto the barrel.
(36608, 0, 0, 1, 54, 0, 100, 0, 0, 0, 0, 0, 0, 64, 1, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Doc Zapnozzle - On Just Summoned - Store summoner'),
(36608, 0, 1, 2, 61, 0, 100, 0, 0, 0, 0, 0, 0, 59, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Doc Zapnozzle - Linked - Set run'),
(36608, 0, 2, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 69, 1, 0, 1, 0, 0, 0, 8, 0, 0, 0, 538.4896, 3271.2048, -0.6523, 0, 'Doc Zapnozzle - Linked - Run down to the shore'),
(36608, 0, 3, 4, 34, 0, 100, 0, 8, 1, 0, 0, 0, 97, 8, 6, 0, 0, 0, 0, 8, 0, 0, 0, 537.135, 3272.25, 0.18, 0, 'Doc Zapnozzle - On Point 1 Reached - Hop onto the barrel'),
(36608, 0, 4, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 80, 3660800, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Doc Zapnozzle - Linked - Run revive vignette'),
-- Farewell on quest turn-in.
(36608, 0, 5, 6, 20, 0, 100, 0, 14239, 0, 0, 0, 0, 64, 1, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Doc Zapnozzle - On Quest 14239 Rewarded - Store player'),
(36608, 0, 6, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 80, 3660801, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Doc Zapnozzle - Linked - Run farewell'),
-- Revive vignette (actionlist 3660800), all beats aimed at the summoner.
(3660800, 9, 0, 0, 0, 0, 100, 0, 1000, 1000, 0, 0, 0, 66, 0, 0, 0, 0, 0, 0, 12, 1, 0, 0, 0, 0, 0, 0, 'Doc vignette - Face the player'),
(3660800, 9, 1, 0, 0, 0, 100, 0, 300, 300, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 12, 1, 0, 0, 0, 0, 0, 0, 'Doc vignette - Say 0 (Gizmo, what are you doing)'),
(3660800, 9, 2, 0, 0, 0, 100, 0, 5600, 5600, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 12, 1, 0, 0, 0, 0, 0, 0, 'Doc vignette - Say 1 (That''s $n!)'),
(3660800, 9, 3, 0, 0, 0, 100, 0, 3100, 3100, 0, 0, 0, 11, 69085, 2, 0, 0, 0, 0, 12, 1, 0, 0, 0, 0, 0, 0, 'Doc vignette - Force player to zap Gizmo'),
(3660800, 9, 4, 0, 0, 0, 100, 0, 6500, 6500, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 12, 1, 0, 0, 0, 0, 0, 0, 'Doc vignette - Say 2 (Stay back)'),
(3660800, 9, 5, 0, 0, 0, 100, 0, 5500, 5500, 0, 0, 0, 11, 69022, 2, 0, 0, 0, 0, 12, 1, 0, 0, 0, 0, 0, 0, 'Doc vignette - Jumper Cables #1'),
(3660800, 9, 6, 0, 0, 0, 100, 0, 5700, 5700, 0, 0, 0, 1, 3, 0, 0, 0, 0, 0, 12, 1, 0, 0, 0, 0, 0, 0, 'Doc vignette - Say 3 (Come on! Clear!)'),
(3660800, 9, 7, 0, 0, 0, 100, 0, 3100, 3100, 0, 0, 0, 11, 69022, 2, 0, 0, 0, 0, 12, 1, 0, 0, 0, 0, 0, 0, 'Doc vignette - Jumper Cables #2'),
(3660800, 9, 8, 0, 0, 0, 100, 0, 5700, 5700, 0, 0, 0, 1, 4, 0, 0, 0, 0, 0, 12, 1, 0, 0, 0, 0, 0, 0, 'Doc vignette - Say 4 (That''s all I''ve got)'),
-- Farewell (actionlist 3660801): thanks, point at the shore, swim off, despawn.
(3660801, 9, 0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 1, 5, 0, 0, 0, 0, 0, 12, 1, 0, 0, 0, 0, 0, 0, 'Doc farewell - Say 5 (You made the right choice)'),
(3660801, 9, 1, 0, 0, 0, 100, 0, 5200, 5200, 0, 0, 0, 1, 6, 0, 0, 0, 0, 0, 12, 1, 0, 0, 0, 0, 0, 0, 'Doc farewell - Say 6 (See you on the shore)'),
(3660801, 9, 2, 0, 0, 0, 100, 0, 3100, 3100, 0, 0, 0, 69, 2, 0, 1, 0, 0, 0, 8, 0, 0, 0, 540.5487, 3262.3333, -0.6787, 0, 'Doc farewell - Wade off the barrel'),
(3660801, 9, 3, 0, 0, 0, 100, 0, 1400, 1400, 0, 0, 0, 69, 3, 0, 1, 0, 0, 0, 8, 0, 0, 0, 579.0174, 3162.5486, -0.6787, 0, 'Doc farewell - Swim toward the shore camp'),
(3660801, 9, 4, 0, 0, 0, 100, 0, 11000, 11000, 0, 0, 0, 41, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Doc farewell - Despawn'),
-- Geargrinder Gizmo: zap reply (hit by the player's forced 69086, not 69085)
-- and the escape-pods bark 3.5s after Goblin Escape Pods (14474) is accepted.
(36600, 0, 0, 0, 8, 0, 100, 0, 69086, 0, 5000, 5000, 0, 1, 1, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Geargrinder Gizmo - On Spellhit Zap - Say reply'),
(36600, 0, 1, 0, 19, 0, 100, 0, 14474, 0, 0, 0, 0, 80, 3660000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Geargrinder Gizmo - On Quest 14474 Accepted - Run pods bark'),
(3660000, 9, 0, 0, 0, 0, 100, 0, 3500, 3500, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Gizmo pods bark - Say 0 (tons of people still trapped)');
