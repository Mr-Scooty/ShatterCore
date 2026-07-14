-- 447 (14125): reveal the Claims Adjuster and the fireworks bunnies with the
-- burning HQ (P2 sniff).
--
-- The Claims Adjuster (37602, quest ender) and the five 447 Fireworks Bunnies
-- (37682) were spawned in the evacuation era (384) but carried the permanent
-- quest-invisibility aura 70268 "447: Claims Adjuster Quest Invis 1" from the
-- modern spawn import - nothing server-side ever grants the matching detection,
-- so they could never be seen (the quest was impossible to turn in). In the
-- sniff all six appear in the same update burst as the 23 "447 Fire" objects,
-- so they use the same mechanism here: the post-arson phase 385, entered when
-- 14125 is complete/rewarded.
UPDATE `creature` SET `PhaseId` = 385 WHERE `guid` IN (253715, 253716, 253717, 253718, 253719, 253720);
UPDATE `creature_addon` SET `auras` = '' WHERE `guid` IN (253715, 253716, 253717, 253718, 253719, 253720);

-- Sniffed reveal yell (targeted at the arsonist, 0.4s after the explosion);
-- the old TDB placeholder line and its say-on-quest-reward SAI trigger are
-- retired - the reveal is driven from the credit scripts.
DELETE FROM `creature_text` WHERE `CreatureID` = 37602;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(37602, 0, 0, 'Another faulty electrical-gas-flammable bed-fireworks accident?!', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Claims Adjuster - 447 HQ ablaze reveal');

DELETE FROM `smart_scripts` WHERE `entryorguid` = 37602 AND `source_type` = 0 AND `id` = 1;

-- The bunnies celebrate the insurance fraud with a firework every ~5s each
-- (sniff: 78102-78105 at ~1/s across the five spawns).
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` = 37682;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 37682 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(37682, 0, 0, 0, 1, 0, 100, 0, 1000, 6000, 14000, 22000, 0, 11, 78102, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, '447 Fireworks Bunny - OOC - Cast Red Streaks Firework'),
(37682, 0, 1, 0, 1, 0, 100, 0, 2000, 8000, 16000, 24000, 0, 11, 78103, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, '447 Fireworks Bunny - OOC - Cast Yellow Rose Firework'),
(37682, 0, 2, 0, 1, 0, 100, 0, 3000, 9000, 15000, 23000, 0, 11, 78104, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, '447 Fireworks Bunny - OOC - Cast Blue Firework'),
(37682, 0, 3, 0, 1, 0, 100, 0, 4000, 10000, 17000, 25000, 0, 11, 78105, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, '447 Fireworks Bunny - OOC - Cast Green Firework');
