-- Kelpthar Forest SAI: cave rescues, eel auto-quest, raider/gilblin texts, turtle event,
-- On Our Own Terms scout scene, quest-accept phase casts. From retail sniff crosscheck.

-- ==================== creature_text group fixes ====================
-- 39313 Zin'jatar Raider: two aggro lines share group 0 (random pick), death line -> group 1
UPDATE `creature_text` SET `GroupID`=0, `ID`=1 WHERE `CreatureID`=39313 AND `GroupID`=1;
UPDATE `creature_text` SET `GroupID`=1, `ID`=0 WHERE `CreatureID`=39313 AND `GroupID`=2;

-- ==================== AIName assignments ====================
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry` IN (39663,41672,39887,40690,40701,40105,39942,40370,40371,40746,40736,40737);

-- ==================== conditions: rescue item spells target the right soldiers ====================
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=17 AND `SourceEntry` IN (74151,77825);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(17, 0, 74151, 0, 0, 31, 0, 3, 39663, 0, 0, 0, 0, '', 'Blow Bubble (A) only on Drowning Soldier'),
(17, 0, 77825, 0, 0, 31, 0, 3, 41672, 0, 0, 0, 0, '', 'Blow Bubble (H) only on Drowning Warrior');

-- ==================== smart_scripts ====================
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (39663,41672,40701,40105,39942,40370,40371,40746,40736,39887,40690,40737) AND `source_type`=0;
DELETE FROM `smart_scripts` WHERE `entryorguid`=39313 AND `source_type`=0 AND `id` IN (1,2);
DELETE FROM `smart_scripts` WHERE `entryorguid`=40855 AND `source_type`=0 AND `id` IN (1,2);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (40810,40677) AND `source_type`=0 AND `id`=1;
DELETE FROM `smart_scripts` WHERE `entryorguid`=40811 AND `source_type`=0 AND `id` IN (1,2);
DELETE FROM `smart_scripts` WHERE `entryorguid`=40223 AND `source_type`=0 AND `id` IN (1,2,3);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (4022300,4022301,4037000,3994200,4069000,4069001) AND `source_type`=9;

INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
-- --- 39313 Zin'jatar Raider: aggro + death lines ---
(39313, 0, 1, 0, 4, 0, 75, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Zin''jatar Raider - On Aggro - Say Line 0'),
(39313, 0, 2, 0, 6, 0, 25, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Zin''jatar Raider - On Death - Say Line 1'),
-- --- 40855 Slitherfin Eel: kill-launched auto-quest Once More, With Eeling (27729) ---
(40855, 0, 1, 2, 6, 0, 100, 0, 0, 0, 0, 0, 7, 27729, 1, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Slitherfin Eel - On Death - Offer Quest 27729 (direct add)'),
(40855, 0, 2, 0, 61, 0, 100, 0, 0, 0, 0, 0, 33, 40855, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Slitherfin Eel - Linked - Kill Credit to killer'),
-- --- 39663 Drowning Soldier: rescued via Blow Bubble (74151) ---
(39663, 0, 0, 1, 8, 0, 100, 0, 74151, 0, 0, 0, 11, 74416, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Drowning Soldier - On Spellhit Blow Bubble - Cast Bubble Self'),
(39663, 0, 1, 2, 61, 0, 100, 0, 0, 0, 0, 0, 33, 39663, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Drowning Soldier - Linked - Kill Credit'),
(39663, 0, 2, 0, 61, 0, 100, 0, 0, 0, 0, 0, 41, 15000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Drowning Soldier - Linked - Despawn in 15s'),
-- --- 41672 Drowning Warrior (H): rescued via Blow Bubble (77825) ---
(41672, 0, 0, 1, 8, 0, 100, 0, 77825, 0, 0, 0, 11, 74416, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Drowning Warrior - On Spellhit Blow Bubble - Cast Bubble Self'),
(41672, 0, 1, 2, 61, 0, 100, 0, 0, 0, 0, 0, 33, 41672, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Drowning Warrior - Linked - Kill Credit'),
(41672, 0, 2, 0, 61, 0, 100, 0, 0, 0, 0, 0, 41, 15000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Drowning Warrior - Linked - Despawn in 15s'),
-- --- Gilblin ambient chatter ---
(40810, 0, 1, 0, 1, 0, 100, 0, 30000, 60000, 45000, 90000, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Gilblin Scavenger - OOC - Say Line 0'),
(40677, 0, 1, 0, 1, 0, 100, 0, 30000, 60000, 45000, 90000, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Gilblin Scavenger - OOC - Say Line 0'),
-- --- 40811 Gilblin Scavenger combat casts (79367 moved out of addon auras) ---
(40811, 0, 1, 0, 0, 0, 100, 0, 6000, 9000, 12000, 18000, 11, 79379, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Gilblin Scavenger - IC - Cast 79379'),
(40811, 0, 2, 0, 0, 0, 100, 0, 10000, 15000, 20000, 30000, 11, 79367, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Gilblin Scavenger - IC - Cast 79367'),
-- --- 25477 turtle ride: 40223 Speckled Sea Turtle ---
(40223, 0, 2, 0, 8, 0, 100, 0, 75189, 0, 0, 0, 80, 4022300, 0, 2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sea Turtle - On Spellhit Mount Sea Turtle - Run Ride Script'),
(40223, 0, 3, 0, 8, 0, 100, 0, 75279, 0, 0, 0, 80, 4022301, 0, 2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sea Turtle - On Spellhit Attack Turtle - Run Death Script'),
(4022300, 9, 0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 11, 75198, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sea Turtle Ride - Cast Encumbered'),
(4022300, 9, 1, 0, 0, 0, 100, 0, 100, 100, 0, 0, 11, 75276, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sea Turtle Ride - Cast Turtle Timer Aura'),
(4022300, 9, 2, 0, 0, 0, 100, 0, 100, 100, 0, 0, 11, 75210, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sea Turtle Ride - Cast Meatstick Visual'),
(4022300, 9, 3, 0, 0, 0, 100, 0, 14600, 14600, 0, 0, 11, 75277, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sea Turtle Ride - Timer Expired'),
(4022300, 9, 4, 0, 0, 0, 100, 0, 200, 200, 0, 0, 11, 75278, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sea Turtle Ride - Summon Shark'),
(4022301, 9, 0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 11, 75385, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sea Turtle Death - Kill Credit to rider'),
(4022301, 9, 1, 0, 0, 0, 100, 0, 100, 100, 0, 0, 11, 50630, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sea Turtle Death - Eject Passengers'),
(4022301, 9, 2, 0, 0, 0, 100, 0, 100, 100, 0, 0, 11, 75283, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sea Turtle Death - Turtle Blood'),
(4022301, 9, 3, 0, 0, 0, 100, 0, 100, 100, 0, 0, 11, 75281, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sea Turtle Death - Summon Death Bunny'),
(4022301, 9, 4, 0, 0, 0, 100, 0, 200, 200, 0, 0, 11, 75282, 2, 0, 0, 0, 0, 19, 40370, 30, 0, 0, 0, 0, 0, 'Sea Turtle Death - Ride Shark Mouth'),
(4022301, 9, 5, 0, 0, 0, 100, 0, 7000, 7000, 0, 0, 41, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sea Turtle Death - Despawn'),
-- --- 40370 Frenzied Reef Shark ---
(40370, 0, 0, 0, 54, 0, 100, 0, 0, 0, 0, 0, 80, 4037000, 0, 2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Reef Shark - Just Summoned - Run Attack Script'),
(4037000, 9, 0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 69, 1, 0, 0, 3, 0, 0, 23, 0, 0, 0, 0, 0, 0, 0, 'Reef Shark - Move to Turtle'),
(4037000, 9, 1, 0, 0, 0, 100, 0, 1500, 1500, 0, 0, 11, 75279, 2, 0, 0, 0, 0, 23, 0, 0, 0, 0, 0, 0, 0, 'Reef Shark - Attack Turtle'),
(4037000, 9, 2, 0, 0, 0, 100, 0, 9000, 9000, 0, 0, 41, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Reef Shark - Despawn'),
-- --- 40371 Turtle Death Bunny gore ---
(40371, 0, 0, 1, 54, 0, 100, 0, 0, 0, 0, 0, 11, 75376, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Turtle Death Bunny - Turtle Parts'),
(40371, 0, 1, 2, 61, 0, 100, 0, 0, 0, 0, 0, 11, 75353, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Turtle Death Bunny - Turtle Frag'),
(40371, 0, 2, 0, 61, 0, 100, 0, 0, 0, 0, 0, 41, 5000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Turtle Death Bunny - Despawn'),
-- --- 39942 Abyssal Lure (25371 objective 1) ---
(39942, 0, 0, 1, 54, 0, 100, 0, 0, 0, 0, 0, 11, 74536, 0, 0, 0, 0, 0, 19, 40126, 20, 0, 0, 0, 0, 0, 'Abyssal Lure - Just Summoned - Rope to anchor'),
(39942, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 80, 3994200, 0, 2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Abyssal Lure - Linked - Run Ray Trigger Script'),
(3994200, 9, 0, 0, 0, 0, 100, 0, 7500, 7500, 0, 0, 11, 74539, 0, 0, 0, 0, 0, 19, 39996, 150, 0, 0, 0, 0, 0, 'Abyssal Lure - Trigger Abyssal Ray'),
-- --- 40105 Erunak at the raft: seahorse success line ---
(40105, 0, 0, 0, 8, 0, 100, 0, 86372, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Erunak Stonespeaker - On Erunak Success Ping - Say Line 0'),
-- --- Erunak battle copies: Lava Bolt pressure ---
(40746, 0, 0, 0, 0, 0, 100, 0, 3000, 4000, 4000, 5000, 11, 76110, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Erunak (Briny Cutter battle) - IC - Lava Bolt'),
(40736, 0, 0, 0, 0, 0, 100, 0, 3000, 4000, 4000, 5000, 11, 76110, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Erunak (Briny Cutter battle) - IC - Lava Bolt'),
-- --- 39887 Captain Taylor: On Our Own Terms accept -> event phase ---
(39887, 0, 0, 0, 19, 0, 100, 0, 25547, 0, 0, 0, 11, 95849, 2, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Captain Taylor - On Quest Accept On Our Own Terms - Phase invoker to 407'),
-- --- 40690 Captain Taylor (event copy): scout scene + All or Nothing start ---
(40690, 0, 0, 0, 10, 0, 100, 0, 1, 15, 120000, 120000, 80, 4069000, 0, 2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Captain Taylor (event) - OOC LOS - Scout Scene'),
(40690, 0, 1, 0, 19, 0, 100, 0, 25558, 0, 0, 0, 80, 4069001, 0, 2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Captain Taylor (event) - On Quest Accept All or Nothing - Battle Start'),
(40737, 0, 0, 0, 19, 0, 100, 0, 25558, 0, 0, 0, 80, 4069001, 0, 2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Captain Taylor (battle) - On Quest Accept All or Nothing - Battle Start'),
(4069000, 9, 0, 0, 0, 0, 100, 0, 2000, 2000, 0, 0, 1, 0, 0, 0, 0, 0, 0, 19, 40691, 30, 0, 0, 0, 0, 0, 'Scout Scene - Soldier: Captain! The naga!'),
(4069000, 9, 1, 0, 0, 0, 100, 0, 200, 200, 0, 0, 12, 40701, 3, 27000, 0, 0, 0, 8, 0, 0, 0, -4456.34, 3813.36, -83.62, 4.747, 'Scout Scene - Summon Zin''jatar Scout'),
(4069000, 9, 2, 0, 0, 0, 100, 0, 2000, 2000, 0, 0, 1, 0, 0, 0, 0, 0, 0, 19, 40701, 30, 0, 0, 0, 0, 0, 'Scout Scene - Scout: Sssoftsskinss!'),
(4069000, 9, 3, 0, 0, 0, 100, 0, 2500, 2500, 0, 0, 69, 2, 0, 0, 1, 0, 0, 8, 0, 0, 0, -4459.60, 3808.50, -82.80, 0, 'Scout Scene - Taylor charges scout'),
(4069000, 9, 4, 0, 0, 0, 100, 0, 1000, 1000, 0, 0, 11, 75917, 0, 0, 0, 0, 0, 19, 40701, 30, 0, 0, 0, 0, 0, 'Scout Scene - Taylor strikes'),
(4069000, 9, 5, 0, 0, 0, 100, 0, 300, 300, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Scout Scene - Taylor: Die, beast!'),
(4069000, 9, 6, 0, 0, 0, 100, 0, 4700, 4700, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Scout Scene - Taylor: We''ve been discovered...'),
(4069000, 9, 7, 0, 0, 0, 100, 0, 2000, 2000, 0, 0, 69, 3, 0, 0, 1, 0, 0, 8, 0, 0, 0, -4440.32, 3817.34, -82.74, 2.007, 'Scout Scene - Taylor returns'),
(4069001, 9, 0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Battle Start - Taylor: To battle, men!'),
(4069001, 9, 1, 0, 0, 0, 100, 0, 300, 300, 0, 0, 1, 0, 0, 0, 0, 0, 0, 19, 40696, 30, 0, 0, 0, 0, 0, 'Battle Start - Soldier: For Stormwind!'),
(4069001, 9, 2, 0, 0, 0, 100, 0, 5700, 5700, 0, 0, 28, 95849, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Battle Start - Remove ship phase from invoker'),
(4069001, 9, 3, 0, 0, 0, 100, 0, 100, 100, 0, 0, 11, 75901, 2, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Battle Start - Phase invoker to 180'),
-- --- 40701 Zin'jatar Scout ---
(40701, 0, 0, 0, 54, 0, 100, 0, 0, 0, 0, 0, 69, 1, 0, 0, 1, 0, 0, 8, 0, 0, 0, -4459.34, 3810.26, -83.32, 0, 'Zin''jatar Scout - Just Summoned - Swim in'),
(40701, 0, 1, 2, 8, 0, 100, 0, 75917, 0, 0, 0, 11, 29266, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Zin''jatar Scout - On Spellhit - Feign Death'),
(40701, 0, 2, 0, 61, 0, 100, 0, 0, 0, 0, 0, 41, 20000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Zin''jatar Scout - Linked - Despawn 20s');
