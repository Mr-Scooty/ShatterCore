-- Kelpthar: Undersea Sanctuary finale arc (25638/25794/25812/25824/25887/25885/25888/25884/25883/27708).
-- Retail sniff crosscheck. Phases: 124 wounded Pollard, 125 refuge hub, 170 Holding Pens combat (retail-native).

-- ==================== 1) Quest chain + flags ====================
UPDATE `quest_template_addon` SET `PrevQuestID`=25794 WHERE `ID`=25812;
UPDATE `quest_template_addon` SET `PrevQuestID`=25812 WHERE `ID`=25824;
UPDATE `quest_template_addon` SET `PrevQuestID`=25824 WHERE `ID`=25887;
UPDATE `quest_template_addon` SET `PrevQuestID`=25887 WHERE `ID` IN (25884,25885,25888);
UPDATE `quest_template` SET `Flags`=`Flags`|0x400000 WHERE `ID` IN (25812,25824);
-- 25885 has a stale second starter on the wounded Pollard copy
DELETE FROM `creature_queststarter` WHERE `id`=41324 AND `quest`=25885;

-- ==================== 2) creature_template ====================
UPDATE `creature_template` SET `VehicleId`=818 WHERE `entry`=41294; -- Watery Vision
UPDATE `creature_template` SET `VehicleId`=744 WHERE `entry`=42013; -- Dominated Great Shark

-- Caged soldiers: full cage-visual loop
UPDATE `creature_template_addon` SET `auras`='49414 77678 76143' WHERE `entry`=41548;

-- ==================== 3) Phasing (Deepmist Grotto 5058, The Clutch 5059, Lightless Reaches 5711) ====================
DELETE FROM `phase_area` WHERE (`AreaId`=5058 AND `PhaseId` IN (124,125)) OR (`AreaId` IN (5059,5711) AND `PhaseId`=170);
INSERT INTO `phase_area` (`AreaId`, `PhaseId`, `Comment`) VALUES
(5058, 124, 'Deepmist Grotto: wounded Private Pollard'),
(5058, 125, 'Deepmist Grotto: refuge hub quartet'),
(5059, 170, 'The Clutch: Holding Pens combat state'),
(5711, 170, 'Lightless Reaches: Holding Pens combat state');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=26 AND ((`SourceGroup` IN (124,125) AND `SourceEntry`=5058) OR (`SourceGroup`=170 AND `SourceEntry` IN (5059,5711)));
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
-- 124: Spelunking taken/complete, or rewarded but Debriefing not yet taken (Pollard lingers through the arrival RP)
(26, 124, 5058, 0, 0, 47, 0, 25812, 10, 0, 0, 0, 0, '', 'Wounded Pollard: Spelunking active'),
(26, 124, 5058, 0, 1, 8, 0, 25812, 0, 0, 0, 0, 0, '', 'Wounded Pollard: Spelunking rewarded'),
(26, 124, 5058, 0, 1, 47, 0, 25824, 74, 0, 1, 0, 0, '', 'Wounded Pollard: Debriefing not started'),
-- 125: hub quartet from Spelunking reward onward
(26, 125, 5058, 0, 0, 8, 0, 25812, 0, 0, 0, 0, 0, '', 'Refuge hub: Spelunking rewarded'),
-- 170: pens combat only while Wake of Destruction is active
(26, 170, 5059, 0, 0, 47, 0, 25887, 10, 0, 0, 0, 0, '', 'Holding Pens: Wake of Destruction active'),
(26, 170, 5711, 0, 0, 47, 0, 25887, 10, 0, 0, 0, 0, '', 'Holding Pens: Wake of Destruction active');

-- Famished sharks belong to the pens combat state
UPDATE `creature` SET `PhaseId`=170 WHERE `id`=41998;
-- Duplicate Warden Azjakir (classic dup-spawn pattern; sniff shows one at -5535)
DELETE FROM `creature` WHERE `guid`=349847;

-- ==================== 4) Spawns ====================
DELETE FROM `creature` WHERE `guid` BETWEEN 9001081 AND 9001085 OR `guid` BETWEEN 9001130 AND 9001153;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild`) VALUES
-- Refuge (Deepmist Grotto)
(9001081, 41324, 0, 4815, 5058, 1, 0, 1, 124, 0, -1, 0, 0, -5136.3525, 3278.0981, -118.331, 0.85, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 0),
(9001082, 41340, 0, 4815, 5058, 1, 0, 1, 125, 0, -1, 0, 0, -5136.3525, 3278.0981, -118.331, 0.85, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 0),
(9001083, 41341, 0, 4815, 5058, 1, 0, 1, 125, 0, -1, 0, 0, -5138.9272, 3281.5781, -118.778, 4.622, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 0),
(9001084, 41344, 0, 4815, 5058, 1, 0, 1, 125, 0, -1, 0, 0, -5138.3560, 3271.6267, -118.4396, 1.10, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 0),
(9001085, 41347, 0, 4815, 5058, 1, 0, 1, 125, 0, -1, 0, 0, -5128.4320, 3280.4949, -118.2017, 3.60, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 0),
-- Holding Pens guardians (phase 170, sniffed positions)
(9001130, 41996, 0, 4815, 5059, 1, 0, 1, 170, 0, -1, 0, 0, -5330.66, 3428.11, -90.42, 1.5, 300, 3, 0, 1, 0, 1, 0, 0, 0, '', 0),
(9001131, 41996, 0, 4815, 5059, 1, 0, 1, 170, 0, -1, 0, 0, -5337.38, 3385.66, -127.91, 2.8, 300, 3, 0, 1, 0, 1, 0, 0, 0, '', 0),
(9001132, 41996, 0, 4815, 5059, 1, 0, 1, 170, 0, -1, 0, 0, -5345.46, 3406.95, -127.58, 0.4, 300, 3, 0, 1, 0, 1, 0, 0, 0, '', 0),
(9001133, 41996, 0, 4815, 5059, 1, 0, 1, 170, 0, -1, 0, 0, -5355.73, 3370.00, -107.87, 4.1, 300, 3, 0, 1, 0, 1, 0, 0, 0, '', 0),
(9001134, 41996, 0, 4815, 5059, 1, 0, 1, 170, 0, -1, 0, 0, -5337.61, 3438.17, -125.67, 5.5, 300, 3, 0, 1, 0, 1, 0, 0, 0, '', 0),
(9001135, 41996, 0, 4815, 5059, 1, 0, 1, 170, 0, -1, 0, 0, -5352.65, 3444.32, -103.07, 1.0, 300, 3, 0, 1, 0, 1, 0, 0, 0, '', 0),
(9001136, 41996, 0, 4815, 5059, 1, 0, 1, 170, 0, -1, 0, 0, -5357.82, 3365.60, -85.00, 2.2, 300, 3, 0, 1, 0, 1, 0, 0, 0, '', 0),
(9001137, 41996, 0, 4815, 5059, 1, 0, 1, 170, 0, -1, 0, 0, -5329.03, 3382.85, -111.73, 3.9, 300, 3, 0, 1, 0, 1, 0, 0, 0, '', 0),
(9001138, 41996, 0, 4815, 5059, 1, 0, 1, 170, 0, -1, 0, 0, -5343.22, 3368.03, -94.92, 0.7, 300, 3, 0, 1, 0, 1, 0, 0, 0, '', 0),
(9001139, 41996, 0, 4815, 5059, 1, 0, 1, 170, 0, -1, 0, 0, -5329.17, 3458.33, -126.92, 5.0, 300, 3, 0, 1, 0, 1, 0, 0, 0, '', 0),
(9001140, 41996, 0, 4815, 5059, 1, 0, 1, 170, 0, -1, 0, 0, -5343.64, 3423.93, -114.16, 1.9, 300, 3, 0, 1, 0, 1, 0, 0, 0, '', 0),
(9001141, 41996, 0, 4815, 5059, 1, 0, 1, 170, 0, -1, 0, 0, -5371.60, 3402.90, -119.45, 3.1, 300, 3, 0, 1, 0, 1, 0, 0, 0, '', 0),
(9001142, 41996, 0, 4815, 5059, 1, 0, 1, 170, 0, -1, 0, 0, -5361.36, 3443.55, -84.18, 4.6, 300, 3, 0, 1, 0, 1, 0, 0, 0, '', 0),
(9001143, 41996, 0, 4815, 5059, 1, 0, 1, 170, 0, -1, 0, 0, -5351.93, 3468.67, -103.62, 0.2, 300, 3, 0, 1, 0, 1, 0, 0, 0, '', 0),
(9001144, 41996, 0, 4815, 5059, 1, 0, 1, 170, 0, -1, 0, 0, -5380.39, 3396.10, -83.43, 2.5, 300, 3, 0, 1, 0, 1, 0, 0, 0, '', 0),
(9001145, 41996, 0, 4815, 5059, 1, 0, 1, 170, 0, -1, 0, 0, -5387.60, 3382.39, -101.05, 3.7, 300, 3, 0, 1, 0, 1, 0, 0, 0, '', 0),
(9001146, 41996, 0, 4815, 5059, 1, 0, 1, 170, 0, -1, 0, 0, -5366.23, 3470.95, -125.15, 5.8, 300, 3, 0, 1, 0, 1, 0, 0, 0, '', 0),
(9001147, 41996, 0, 4815, 5059, 1, 0, 1, 170, 0, -1, 0, 0, -5394.80, 3381.83, -89.42, 1.3, 300, 3, 0, 1, 0, 1, 0, 0, 0, '', 0),
(9001148, 41996, 0, 4815, 5059, 1, 0, 1, 170, 0, -1, 0, 0, -5401.68, 3423.35, -94.32, 2.9, 300, 3, 0, 1, 0, 1, 0, 0, 0, '', 0),
(9001149, 41996, 0, 4815, 5059, 1, 0, 1, 170, 0, -1, 0, 0, -5408.57, 3421.08, -122.33, 4.4, 300, 3, 0, 1, 0, 1, 0, 0, 0, '', 0),
(9001150, 41996, 0, 4815, 5059, 1, 0, 1, 170, 0, -1, 0, 0, -5413.00, 3367.07, -90.65, 0.9, 300, 3, 0, 1, 0, 1, 0, 0, 0, '', 0),
(9001151, 41996, 0, 4815, 5059, 1, 0, 1, 170, 0, -1, 0, 0, -5405.80, 3451.81, -108.48, 2.0, 300, 3, 0, 1, 0, 1, 0, 0, 0, '', 0),
(9001152, 41996, 0, 4815, 5059, 1, 0, 1, 170, 0, -1, 0, 0, -5433.48, 3442.74, -110.01, 3.3, 300, 3, 0, 1, 0, 1, 0, 0, 0, '', 0),
(9001153, 41996, 0, 4815, 5059, 1, 0, 1, 170, 0, -1, 0, 0, -5402.79, 3490.61, -133.26, 5.2, 300, 3, 0, 1, 0, 1, 0, 0, 0, '', 0);

DELETE FROM `gameobject` WHERE `guid` BETWEEN 9001006 AND 9001013;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `VerifiedBuild`) VALUES
(9001006, 203128, 0, 4815, 5057, 1, 0, 1, 169, 0, -1, -4580.806, 3489.976, -103.568, 0, 0, 0, 0, 1, 300, 255, 1, '', 0),
(9001007, 203300, 0, 4815, 5059, 1, 0, 1, 169, 0, -1, -5322.09, 3433.80, -132.72, 0.9076, 0, 0, 0.43837, 0.89879, 300, 255, 1, '', 0),
(9001008, 203300, 0, 4815, 5059, 1, 0, 1, 169, 0, -1, -5343.75, 3443.90, -133.33, 0.9076, 0, 0, 0.43837, 0.89879, 300, 255, 1, '', 0),
(9001009, 203300, 0, 4815, 5059, 1, 0, 1, 169, 0, -1, -5344.80, 3367.72, -131.05, 0.9076, 0, 0, 0.43837, 0.89879, 300, 255, 1, '', 0),
(9001010, 203300, 0, 4815, 5059, 1, 0, 1, 169, 0, -1, -5358.01, 3394.20, -133.43, 0.9076, 0, 0, 0.43837, 0.89879, 300, 255, 1, '', 0),
(9001011, 203300, 0, 4815, 5059, 1, 0, 1, 169, 0, -1, -5385.39, 3371.18, -106.17, 0.9076, 0, 0, 0.43837, 0.89879, 300, 255, 1, '', 0),
(9001012, 203300, 0, 4815, 5059, 1, 0, 1, 169, 0, -1, -5425.22, 3347.83, -104.44, 0.9076, 0, 0, 0.43837, 0.89879, 300, 255, 1, '', 0),
(9001013, 203300, 0, 4815, 5059, 1, 0, 1, 169, 0, -1, -5472.94, 3502.07, -125.15, 0.9076, 0, 0, 0.43837, 0.89879, 300, 255, 1, '', 0);

-- ==================== 5) Page texts, quest texts ====================
DELETE FROM `page_text` WHERE `ID` IN (15095,15183);
INSERT INTO `page_text` (`ID`, `Text`, `NextPageID`, `VerifiedBuild`) VALUES
(15095, 'The parchment inside the bottle is water-stained, but the hurried script is still legible:$B$B"To any who find this - the naga have taken the crew. They drag their prisoners west, beyond the kelp. I go to find the shaman of the Earthen Ring at his raft. Seek him there. Hurry."', 0, 0),
(15183, 'Racks of cruelly barbed tridents stand ready for the naga war effort.$B$BDestroying them would surely blunt the Zin''jatar assault.', 0, 0);

DELETE FROM `quest_details` WHERE `ID` IN (25812,25824,25885,25888);
INSERT INTO `quest_details` (`ID`, `Emote1`, `Emote2`, `Emote3`, `Emote4`, `EmoteDelay1`, `EmoteDelay2`, `EmoteDelay3`, `EmoteDelay4`, `VerifiedBuild`) VALUES
(25812, 5, 0, 0, 0, 0, 0, 0, 0, 0),
(25824, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(25885, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(25888, 5, 396, 0, 0, 0, 1000, 0, 0, 0);

DELETE FROM `quest_request_items` WHERE `ID` IN (25883,25885);
INSERT INTO `quest_request_items` (`ID`, `EmoteOnComplete`, `EmoteOnIncomplete`, `CompletionText`, `VerifiedBuild`) VALUES
(25883, 0, 0, 'Have you destroyed the naga arsenal?', 0),
(25885, 0, 0, 'Were you able to find me any of their seaweed, $c?', 0);

-- ==================== 6) Loot + spell_area ====================
UPDATE `creature_loot_template` SET `QuestRequired`=1 WHERE `Item`=56167 AND `Entry` IN (41477,41481,41549,46474);

DELETE FROM `spell_area` WHERE `spell` IN (78271,78118) AND `area` IN (5058,5059,5711);
INSERT INTO `spell_area` (`spell`, `area`, `quest_start`, `quest_end`, `aura_spell`, `racemask`, `gender`, `flags`, `quest_start_status`, `quest_end_status`) VALUES
-- Pens combat phase aura while Wake of Destruction is active (autoremove on leave)
(78271, 5059, 25887, 25887, 0, 0, 2, 3, 10, 64),
(78271, 5711, 25887, 25887, 0, 0, 2, 3, 10, 64),
-- See Imprisoned Soldiers from Undersea Sanctuary through Decompression
(78118, 5058, 25794, 25888, 0, 0, 2, 1, 74, 64),
(78118, 5059, 25794, 25888, 0, 0, 2, 1, 74, 64),
(78118, 5711, 25794, 25888, 0, 0, 2, 1, 74, 64);

-- ==================== 7) SAI ====================
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry` IN (41340,41548,41582,41996,46392,41328,41330,41331,41339);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (41340,41548,41582,41996,46392,41328,41330,41331,41339) AND `source_type`=0;
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (4132800,4133000,4133100,4133900) AND `source_type`=9;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
-- Debriefing gossip ladder final step
(41340, 0, 0, 0, 62, 0, 100, 0, 11514, 0, 0, 0, 11, 77572, 2, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Private Pollard - final gossip select - Kill Credit'),
-- Decompression cage rescue
(41548, 0, 0, 1, 8, 0, 100, 0, 77671, 0, 0, 0, 12, 41582, 3, 20000, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Imprisoned Soldier - Breathstone hit - summon freed soldier'),
(41548, 0, 1, 2, 61, 0, 100, 0, 0, 0, 0, 0, 11, 77683, 2, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Imprisoned Soldier - Linked - Kill Credit to rescuer'),
(41548, 0, 2, 0, 61, 0, 100, 0, 0, 0, 0, 0, 41, 8000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Imprisoned Soldier - Linked - despawn cage copy'),
(41582, 0, 0, 1, 54, 0, 100, 0, 0, 0, 0, 0, 59, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Freed Soldier - Just Summoned - run'),
(41582, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 69, 1, 0, 0, 0, 0, 0, 8, 0, 0, 0, -5250.0, 3560.0, -80.0, 0, 'Freed Soldier - Linked - swim for open water'),
(41582, 0, 2, 0, 52, 0, 100, 0, 1, 0, 0, 0, 41, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Freed Soldier - movement inform - despawn'),
-- Pen guardian eaten by the dominated shark
(41996, 0, 0, 1, 8, 0, 100, 0, 78303, 0, 0, 0, 11, 35309, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Zin''jatar Guardian - Eat Naga hit - bloody explosion'),
(41996, 0, 1, 2, 61, 0, 100, 0, 0, 0, 0, 0, 11, 58951, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Zin''jatar Guardian - Linked - feign death'),
(41996, 0, 2, 0, 61, 0, 100, 0, 0, 0, 0, 0, 41, 4000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Zin''jatar Guardian - Linked - despawn'),
-- Shimmering Bubble minion drifts and pops
(46392, 0, 0, 1, 54, 0, 100, 0, 0, 0, 0, 0, 29, 2, 25, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Shimmering Bubble - Just Summoned - drift after player'),
(46392, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 41, 8000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shimmering Bubble - Linked - pop'),
-- Spelunking arrival RP doubles
(41328, 0, 0, 0, 54, 0, 100, 0, 0, 0, 0, 0, 80, 4132800, 0, 2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Erunak (Deepmist RP) - Just Summoned'),
(4132800, 9, 0, 0, 0, 0, 100, 0, 500, 500, 0, 0, 69, 1, 0, 0, 0, 0, 0, 8, 0, 0, 0, -5141.0, 3287.0, -119.5, 0, 'Erunak RP - swim to alcove'),
(4132800, 9, 1, 0, 0, 0, 100, 0, 3600, 3600, 0, 0, 1, 0, 0, 1, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Erunak RP - We have arrived, $n'),
(4132800, 9, 2, 0, 0, 0, 100, 0, 18000, 18000, 0, 0, 41, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Erunak RP - despawn'),
(41330, 0, 0, 0, 54, 0, 100, 0, 0, 0, 0, 0, 80, 4133000, 0, 2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Moanah (Deepmist RP) - Just Summoned'),
(4133000, 9, 0, 0, 0, 0, 100, 0, 800, 800, 0, 0, 69, 1, 0, 0, 0, 0, 0, 8, 0, 0, 0, -5134.0, 3280.5, -118.8, 0, 'Moanah RP - swim in'),
(4133000, 9, 1, 0, 0, 0, 100, 0, 3900, 3900, 0, 0, 11, 77434, 0, 0, 0, 0, 0, 19, 41324, 15, 0, 0, 0, 0, 0, 'Moanah RP - Healing Wave on Pollard'),
(4133000, 9, 2, 0, 0, 0, 100, 0, 2400, 2400, 0, 0, 11, 77434, 0, 0, 0, 0, 0, 19, 41324, 15, 0, 0, 0, 0, 0, 'Moanah RP - Healing Wave on Pollard'),
(4133000, 9, 3, 0, 0, 0, 100, 0, 14000, 14000, 0, 0, 41, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Moanah RP - despawn'),
(41331, 0, 0, 0, 54, 0, 100, 0, 0, 0, 0, 0, 80, 4133100, 0, 2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Rendel (Deepmist RP) - Just Summoned'),
(4133100, 9, 0, 0, 0, 0, 100, 0, 1000, 1000, 0, 0, 69, 1, 0, 0, 0, 0, 0, 8, 0, 0, 0, -5140.5, 3275.0, -118.8, 0, 'Rendel RP - swim in'),
(4133100, 9, 1, 0, 0, 0, 100, 0, 6300, 6300, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Rendel RP - This should warm the lad up!'),
(4133100, 9, 2, 0, 0, 0, 100, 0, 700, 700, 0, 0, 85, 77447, 2, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Rendel RP - invoker summons standing Pollard'),
(4133100, 9, 3, 0, 0, 0, 100, 0, 16000, 16000, 0, 0, 41, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Rendel RP - despawn'),
(41339, 0, 0, 0, 54, 0, 100, 0, 0, 0, 0, 0, 80, 4133900, 0, 2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Pollard (RP double) - Just Summoned'),
(4133900, 9, 0, 0, 0, 0, 100, 0, 3300, 3300, 0, 0, 1, 0, 0, 1, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Pollard RP - Who are you? What is this place?'),
(4133900, 9, 1, 0, 0, 0, 100, 0, 12000, 12000, 0, 0, 41, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Pollard RP - despawn');

-- Warden Azjakir: add the sniffed Bubble Explosion to his existing SAI
DELETE FROM `smart_scripts` WHERE `entryorguid`=41530 AND `source_type`=0 AND `id`=10;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(41530, 0, 10, 0, 0, 0, 100, 0, 15000, 20000, 18000, 25000, 11, 86411, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Warden Azjakir - IC - Bubble Explosion');
