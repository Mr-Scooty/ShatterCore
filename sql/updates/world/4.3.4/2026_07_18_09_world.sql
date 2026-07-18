-- Kelpthar Horde mirror: Nazgrim chain semantics, H-cave state 166, event spawns,
-- authored texts (no retail sniff exists for these - flagged for walkthrough), vendors.

-- ==================== 1) Quest chain semantics (mirror of the A-side fixes) ====================
UPDATE `quest_template_addon` SET `ExclusiveGroup`=-25942 WHERE `ID` IN (25942,25943,25946);
UPDATE `quest_template_addon` SET `ExclusiveGroup`=-25944 WHERE `ID` IN (25944,25947);
UPDATE `quest_template_addon` SET `PrevQuestID`=25941 WHERE `ID` IN (25942,25943,27668);
UPDATE `quest_template_addon` SET `PrevQuestID`=27668 WHERE `ID`=25946;
UPDATE `quest_template` SET `Flags`=`Flags`|0x400000 WHERE `ID`=25944;

-- ==================== 2) H-cave state 166 (post-Girding Our Loins crew) ====================
DELETE FROM `phase_area` WHERE `AreaId`=5056 AND `PhaseId`=166;
INSERT INTO `phase_area` (`AreaId`, `PhaseId`, `Comment`) VALUES
(5056, 166, 'Vashjir H-cave: Nazgrim + Hellscream vanguard crew');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=26 AND `SourceGroup`=166 AND `SourceEntry`=5056;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=26 AND `SourceGroup`=165 AND `SourceEntry`=5056 AND `ConditionTypeOrReference`=8 AND `ConditionValue1`=25944;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(26, 165, 5056, 0, 0, 8, 0, 25944, 0, 0, 1, 0, 0, '', 'H-cave recovering state ends at Girding Our Loins reward'),
(26, 166, 5056, 0, 0, 8, 0, 25944, 0, 0, 0, 0, 0, '', 'H-cave crew state: Girding Our Loins rewarded'),
(26, 166, 5056, 0, 0, 47, 0, 25948, 10, 0, 1, 0, 0, '', 'H-cave crew state: Bring It On! not active'),
(26, 166, 5056, 0, 0, 47, 0, 25949, 10, 0, 1, 0, 0, '', 'H-cave crew state: Blood and Thunder! not active');

-- ==================== 3) Spawns (guids 9001160+) ====================
DELETE FROM `creature` WHERE `guid` BETWEEN 9001160 AND 9001189;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild`) VALUES
-- State 166: armored Nazgrim (display 32574) + Hellscream's Vanguard at the recovering-warrior spots
(9001160, 41711, 0, 4815, 5056, 1, 0, 1, 166, 0, -1, 32574, 0, -4596.330, 3999.520, -71.270, 3.91, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 0),
(9001161, 41749, 0, 4815, 5056, 1, 0, 1, 166, 0, -1, 0, 0, -4608.600, 3985.740, -70.330, 2.6005, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 0),
(9001162, 41752, 0, 4815, 5056, 1, 0, 1, 166, 0, -1, 0, 0, -4597.570, 3993.120, -70.650, 2.0420, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 0),
(9001163, 41753, 0, 4815, 5056, 1, 0, 1, 166, 0, -1, 0, 0, -4592.640, 3997.820, -71.010, 2.6704, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 0),
(9001164, 41754, 0, 4815, 5056, 1, 0, 1, 166, 0, -1, 0, 0, -4596.740, 3995.940, -71.030, 4.0317, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 0),
(9001165, 41755, 0, 4815, 5056, 1, 0, 1, 166, 0, -1, 0, 0, -4617.610, 3983.930, -70.740, 0.8552, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 0),
(9001166, 41756, 0, 4815, 5056, 1, 0, 1, 166, 0, -1, 0, 0, -4619.520, 3986.770, -71.340, 0.0, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 0),
-- Bring It On! scene set (phase 407, mirrors A 40690/40691)
(9001167, 41750, 0, 4815, 5056, 1, 0, 1, 407, 0, -1, 0, 0, -4596.330, 3999.520, -71.270, 3.91, 120, 0, 0, 1, 0, 0, 0, 0, 0, '', 0),
(9001168, 41752, 0, 4815, 5056, 1, 0, 1, 407, 0, -1, 0, 0, -4592.640, 3997.820, -71.010, 3.30, 120, 0, 0, 1, 0, 0, 0, 0, 0, '', 0),
-- Blood and Thunder! battle sets (phase 180/181, synthesized wreck spot - walkthrough-tune)
(9001169, 41769, 0, 4815, 5056, 1, 0, 1, 180, 0, -1, 0, 0, -4655.000, 3990.000, -95.000, 0.90, 120, 0, 0, 1, 0, 0, 0, 0, 0, '', 0),
(9001170, 41766, 0, 4815, 5056, 1, 0, 1, 180, 0, -1, 0, 0, -4650.000, 3985.000, -92.000, 3.14, 120, 0, 0, 1, 0, 0, 0, 0, 0, '', 0),
(9001171, 41793, 0, 4815, 5056, 1, 0, 1, 181, 0, -1, 0, 0, -4655.000, 3990.000, -95.000, 0.90, 120, 0, 0, 1, 0, 0, 0, 0, 0, '', 0),
(9001172, 41796, 0, 4815, 5056, 1, 0, 1, 181, 0, -1, 0, 0, -4662.000, 3979.000, -97.000, 1.20, 120, 0, 0, 1, 0, 0, 0, 0, 0, '', 0),
(9001173, 41797, 0, 4815, 5056, 1, 0, 1, 181, 0, -1, 0, 0, -4649.000, 3999.000, -94.500, 5.20, 120, 0, 0, 1, 0, 0, 0, 0, 0, '', 0),
(9001174, 41798, 0, 4815, 5056, 1, 0, 1, 181, 0, -1, 0, 0, -4643.000, 3982.000, -93.000, 2.10, 120, 0, 0, 1, 0, 0, 0, 0, 0, '', 0),
(9001175, 41799, 0, 4815, 5056, 1, 0, 1, 181, 0, -1, 0, 0, -4667.000, 3995.000, -98.000, 4.70, 120, 0, 0, 1, 0, 0, 0, 0, 0, '', 0),
(9001176, 41800, 0, 4815, 5056, 1, 0, 1, 181, 0, -1, 0, 0, -4658.000, 4002.000, -96.000, 0.30, 120, 0, 0, 1, 0, 0, 0, 0, 0, '', 0),
(9001177, 41751, 0, 4815, 5056, 1, 0, 1, 181, 0, -1, 0, 0, -4652.000, 3988.000, -94.000, 3.00, 120, 0, 0, 1, 0, 0, 0, 0, 0, '', 0),
-- Extra Drowning Warrior rescue targets on the 25936 POI ring
(9001178, 41672, 0, 4815, 5056, 1, 0, 1, 169, 0, -1, 0, 0, -4606.000, 3861.000, -89.000, 1.20, 60, 0, 0, 1, 0, 0, 0, 0, 0, '', 0),
(9001179, 41672, 0, 4815, 5056, 1, 0, 1, 169, 0, -1, 0, 0, -4551.000, 3890.000, -91.000, 2.40, 60, 0, 0, 1, 0, 0, 0, 0, 0, '', 0),
(9001180, 41672, 0, 4815, 5056, 1, 0, 1, 169, 0, -1, 0, 0, -4525.000, 3955.000, -91.500, 3.10, 60, 0, 0, 1, 0, 0, 0, 0, 0, '', 0),
(9001181, 41672, 0, 4815, 5056, 1, 0, 1, 169, 0, -1, 0, 0, -4548.000, 4005.000, -89.500, 4.00, 60, 0, 0, 1, 0, 0, 0, 0, 0, '', 0),
(9001182, 41672, 0, 4815, 5056, 1, 0, 1, 169, 0, -1, 0, 0, -4652.000, 3925.000, -90.000, 0.60, 60, 0, 0, 1, 0, 0, 0, 0, 0, '', 0),
(9001183, 41672, 0, 4815, 5056, 1, 0, 1, 169, 0, -1, 0, 0, -4679.000, 3885.000, -92.000, 1.00, 60, 0, 0, 1, 0, 0, 0, 0, 0, '', 0);
UPDATE `creature` SET `spawntimesecs`=60 WHERE `guid` BETWEEN 9001070 AND 9001075;

-- ==================== 4) Quest texts (authored - no sniff; walkthrough-verify) ====================
DELETE FROM `quest_request_items` WHERE `ID`=25929;
INSERT INTO `quest_request_items` (`ID`, `EmoteOnComplete`, `EmoteOnIncomplete`, `CompletionText`, `VerifiedBuild`) VALUES
(25929, 0, 0, 'Were you able to gather the items I requested, $r?', 0);

DELETE FROM `quest_details` WHERE `ID` IN (25944,25946,25947,27668);
INSERT INTO `quest_details` (`ID`, `Emote1`, `Emote2`, `Emote3`, `Emote4`, `EmoteDelay1`, `EmoteDelay2`, `EmoteDelay3`, `EmoteDelay4`, `VerifiedBuild`) VALUES
(25944, 5, 396, 0, 0, 0, 1000, 0, 0, 0),
(25946, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(25947, 396, 0, 0, 0, 0, 0, 0, 0, 0),
(27668, 1, 0, 0, 0, 0, 0, 0, 0, 0);

DELETE FROM `quest_offer_reward` WHERE `ID` IN (25942,25943,25944,25946,27668,28816);
INSERT INTO `quest_offer_reward` (`ID`, `Emote1`, `Emote2`, `Emote3`, `Emote4`, `EmoteDelay1`, `EmoteDelay2`, `EmoteDelay3`, `EmoteDelay4`, `RewardText`, `VerifiedBuild`) VALUES
(25942, 1, 0, 0, 0, 0, 0, 0, 0, 'Well fought, $c. Every naga you cut down buys us another breath down here.', 0),
(25943, 66, 0, 0, 0, 0, 0, 0, 0, 'Crab meat. It''s no wolf steak, but an army moves on its stomach - even at the bottom of the sea.', 0),
(25944, 5, 0, 0, 0, 0, 0, 0, 0, 'Armed and armored. Now we look like Hellscream''s soldiers again, not shipwreck survivors.', 0),
(25946, 396, 0, 0, 0, 0, 0, 0, 0, 'Alliance-issue gear, waterlogged but serviceable. Take what we can use - the Earthspeaker''s cause makes strange allies of us all.', 0),
(27668, 25, 0, 0, 0, 0, 0, 0, 0, 'So the Alliance lost a ship down here as well. Keep your eyes open, $c.', 0),
(28816, 1, 0, 0, 0, 0, 0, 0, 0, 'The mercenary ship is ready to sail. Get aboard, $c - Vashj''ir won''t wait.', 0);

-- H event creature texts (authored mirrors of the A scene lines)
DELETE FROM `creature_text` WHERE `CreatureID` IN (41750,41752,41755,41757,41769,41793);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(41750, 0, 0, 'Die, beast!', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Legionnaire Nazgrim (scene)'),
(41750, 1, 0, 'We''ve been discovered... Hurry $n, there''s little time!', 12, 0, 100, 0, 0, 0, 0, 0, 0, 'Legionnaire Nazgrim (scene)'),
(41750, 2, 0, 'To battle, warriors of the Horde!', 14, 0, 100, 53, 0, 0, 0, 0, 0, 'Legionnaire Nazgrim (scene)'),
(41752, 0, 0, 'Legionnaire! The naga! They''ve followed $n back to the ship!', 14, 0, 100, 5, 0, 0, 0, 0, 0, 'Hellscream''s Vanguard'),
(41755, 0, 0, 'For the Horde!', 14, 0, 100, 15, 0, 0, 0, 0, 0, 'Hellscream''s Vanguard'),
(41757, 0, 0, 'Sssoftsskinss! Sssoon you shall ssserve my Lady!', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Zin''jatar Scout (H)'),
(41769, 0, 0, 'Hold the line! Lok''tar ogar!', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Legionnaire Nazgrim (battle)'),
(41769, 1, 0, 'Rest while you can. That wasn''t the last of them....', 12, 0, 100, 0, 0, 0, 0, 0, 0, 'Legionnaire Nazgrim (battle)'),
(41793, 0, 0, 'Brace yourselves!', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Legionnaire Nazgrim (battle 2)'),
(41793, 1, 0, 'Make them pay!', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Legionnaire Nazgrim (battle 2)'),
(41793, 2, 0, 'Stay alert! Don''t let them drag you away!', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Legionnaire Nazgrim (battle 2)'),
(41793, 3, 0, 'Don''t give up! Fight! Fight for the Horde!', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Legionnaire Nazgrim (battle 2)'),
(41793, 4, 0, 'Their numbers are... endless...', 12, 0, 100, 0, 0, 0, 0, 0, 0, 'Legionnaire Nazgrim (battle 2)'),
(41793, 5, 0, 'My warriors - they''re all taken... Go! Save yourself!', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Legionnaire Nazgrim (battle 2)');

-- ==================== 5) SAI: Bring It On! scene + battle start ====================
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry` IN (41750,41769,41757);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (41750,41769,41757) AND `source_type`=0;
DELETE FROM `smart_scripts` WHERE `entryorguid`=41711 AND `source_type`=0;
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (4175000,4175001) AND `source_type`=9;
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=41711;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(41711, 0, 0, 0, 19, 0, 100, 0, 25948, 0, 0, 0, 11, 95849, 2, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Nazgrim - On Quest Accept Bring It On! - Phase invoker to 407'),
(41750, 0, 0, 0, 10, 0, 100, 0, 1, 15, 120000, 120000, 80, 4175000, 0, 2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Nazgrim (scene) - OOC LOS - Scout Scene'),
(41750, 0, 1, 0, 19, 0, 100, 0, 25949, 0, 0, 0, 80, 4175001, 0, 2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Nazgrim (scene) - On Quest Accept Blood and Thunder! - Battle Start'),
(41769, 0, 0, 0, 19, 0, 100, 0, 25949, 0, 0, 0, 80, 4175001, 0, 2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Nazgrim (battle) - On Quest Accept Blood and Thunder! - Battle Start'),
(4175000, 9, 0, 0, 0, 0, 100, 0, 2000, 2000, 0, 0, 1, 0, 0, 0, 0, 0, 0, 19, 41752, 30, 0, 0, 0, 0, 0, 'H Scout Scene - Vanguard: Legionnaire! The naga!'),
(4175000, 9, 1, 0, 0, 0, 100, 0, 200, 200, 0, 0, 12, 41757, 3, 27000, 0, 0, 0, 8, 0, 0, 0, -4612.30, 3993.50, -71.50, 1.20, 'H Scout Scene - Summon Zin''jatar Scout'),
(4175000, 9, 2, 0, 0, 0, 100, 0, 2000, 2000, 0, 0, 1, 0, 0, 0, 0, 0, 0, 19, 41757, 30, 0, 0, 0, 0, 0, 'H Scout Scene - Scout: Sssoftsskinss!'),
(4175000, 9, 3, 0, 0, 0, 100, 0, 2500, 2500, 0, 0, 69, 2, 0, 0, 1, 0, 0, 8, 0, 0, 0, -4609.50, 3995.00, -71.30, 0, 'H Scout Scene - Nazgrim charges'),
(4175000, 9, 4, 0, 0, 0, 100, 0, 1000, 1000, 0, 0, 11, 75917, 0, 0, 0, 0, 0, 19, 41757, 30, 0, 0, 0, 0, 0, 'H Scout Scene - Nazgrim strikes'),
(4175000, 9, 5, 0, 0, 0, 100, 0, 300, 300, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'H Scout Scene - Die, beast!'),
(4175000, 9, 6, 0, 0, 0, 100, 0, 4700, 4700, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'H Scout Scene - We''ve been discovered...'),
(4175000, 9, 7, 0, 0, 0, 100, 0, 2000, 2000, 0, 0, 69, 3, 0, 0, 1, 0, 0, 8, 0, 0, 0, -4596.33, 3999.52, -71.27, 3.91, 'H Scout Scene - Nazgrim returns'),
(4175001, 9, 0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'H Battle Start - To battle!'),
(4175001, 9, 1, 0, 0, 0, 100, 0, 300, 300, 0, 0, 1, 0, 0, 0, 0, 0, 0, 19, 41755, 40, 0, 0, 0, 0, 0, 'H Battle Start - For the Horde!'),
(4175001, 9, 2, 0, 0, 0, 100, 0, 5700, 5700, 0, 0, 28, 95849, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'H Battle Start - Remove ship phase'),
(4175001, 9, 3, 0, 0, 0, 100, 0, 100, 100, 0, 0, 11, 75901, 2, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'H Battle Start - Phase invoker to 180'),
-- Scout feign-death mirror
(41757, 0, 0, 1, 8, 0, 100, 0, 75917, 0, 0, 0, 11, 29266, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Zin''jatar Scout (H) - On Spellhit - Feign Death'),
(41757, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 41, 20000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Zin''jatar Scout (H) - Linked - Despawn 20s');

-- ==================== 6) Loot + vendors ====================
UPDATE `creature_loot_template` SET `QuestRequired`=1 WHERE `Entry`=41746 AND `Item` IN (56243,56244,56245);

DELETE FROM `npc_vendor` WHERE `entry` IN (41508,41618);
INSERT INTO `npc_vendor` (`entry`, `slot`, `item`, `maxcount`, `incrtime`, `ExtendedCost`, `VerifiedBuild`) VALUES
(41508, 1, 4565, 0, 0, 0, 0),
(41508, 2, 63388, 0, 0, 0, 0),
(41508, 3, 64670, 0, 0, 0, 0),
(41618, 1, 58275, 0, 0, 0, 0),
(41618, 2, 58274, 0, 0, 0, 0),
(41618, 3, 4600, 0, 0, 0, 0);
