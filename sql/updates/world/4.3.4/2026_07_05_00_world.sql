--
-- Gilneas (Worgen starter zone) — phase ladder + quest-data fixes
-- Covers quests from Grandma Wahl (14398) through Endgame (26706).
--
-- Player phasing for the post-14466 chain uses the native spell_area aura
-- ladder (Blizzard's "Phase - Quest Zone-Specific" spells, Aura 261 with
-- MiscValueB = PhaseId), extending the working 68481/68482/68483 rows:
--   69077 -> 184, 69484 -> 186, 69485 -> 187, 69486 -> 188,
--   70695 -> 189, 70696 -> 190, 74093 -> 191, 74096 -> 194
-- quest_start_status/quest_end_status masks: 1=NONE, 2=COMPLETE,
-- 8=INCOMPLETE, 32=FAILED, 64=REWARDED (74 = from accept onward,
-- 10 = while on the log, 43 = until rewarded, 1 = until accepted).
--

-- Phase ladder
DELETE FROM `spell_area` WHERE `spell` IN (69077, 69484, 69485, 69486, 70695, 70696, 74093, 74096);
INSERT INTO `spell_area` (`spell`, `area`, `quest_start`, `quest_end`, `aura_spell`, `racemask`, `gender`, `flags`, `quest_start_status`, `quest_end_status`) VALUES
-- 184: manor approach, from "To Greymane Manor" accept until "The King's Observatory" rewarded (stacks with 183)
(69077, 4714, 14465, 14466, 0, 0, 2, 3, 74, 43),
-- 186: Duskhaven destroyed, permanent from "The King's Observatory" rewarded
(69484, 4714, 14466, 0, 0, 0, 2, 3, 64, 43),
(69484, 4755, 14466, 0, 0, 0, 2, 3, 64, 43),
-- 194: stagecoach crash-site ogres, while "Exodus" is on the log
(74096, 4714, 24438, 0, 0, 0, 2, 3, 10, 43),
-- 187: city battle, only while "The Battle for Gilneas City" is on the log
(69485, 4714, 24904, 0, 0, 0, 2, 3, 10, 43),
(69485, 4755, 24904, 0, 0, 0, 2, 3, 10, 43),
-- 190: post-battle Genn camp, from 24904 rewarded until "Keel Harbor" accepted
(70696, 4714, 24904, 24680, 0, 0, 2, 3, 64, 1),
(70696, 4755, 24904, 24680, 0, 0, 2, 3, 64, 1),
-- 188: aftermath/funeral era, from "Vengeance or Survival" accept until "Keel Harbor" accepted (stacks with 190)
(69486, 4714, 24903, 24680, 0, 0, 2, 3, 74, 1),
(69486, 4755, 24903, 24680, 0, 0, 2, 3, 74, 1),
-- 189: Keel Harbor battle, from "Keel Harbor" accept until "They Have Allies, But So Do We" rewarded
(70695, 4714, 24680, 24681, 0, 0, 2, 3, 74, 43),
(70695, 4755, 24680, 24681, 0, 0, 2, 3, 74, 43),
-- 191: departure, from 24681 rewarded (cleared natively on the map 654 -> 1 teleport)
(74093, 4714, 24681, 0, 0, 0, 2, 3, 64, 43),
(74093, 4755, 24681, 0, 0, 0, 2, 3, 64, 43);

-- "Alas, Gilneas!" (14467) chain reinsert: retail flow is 14466 -> 14467 -> 24438,
-- with cinematic 167 delivered by 14467's reward spell 69257
UPDATE `quest_template_addon` SET `PrevQuestID` = 14466 WHERE `ID` = 14467;
UPDATE `quest_template_addon` SET `PrevQuestID` = 14467 WHERE `ID` = 24438;
UPDATE `quest_template` SET `RewardNextQuest` = 14467 WHERE `ID` = 14466;

-- Missing quest_details rows (retail ships no emotes for either)
DELETE FROM `quest_details` WHERE `ID` IN (14467, 14434);
INSERT INTO `quest_details` (`ID`, `Emote1`, `Emote2`, `Emote3`, `Emote4`, `EmoteDelay1`, `EmoteDelay2`, `EmoteDelay3`, `EmoteDelay4`, `VerifiedBuild`) VALUES
(14467, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(14434, 0, 0, 0, 0, 0, 0, 0, 0, 0);

-- Stormglen Village inn rest zone (AreaTrigger.dbc 6006 box over Willa Arnes' inn)
DELETE FROM `areatrigger_tavern` WHERE `id` = 6006;
INSERT INTO `areatrigger_tavern` (`id`, `name`) VALUES (6006, 'Stormglen Village Inn');

-- "I Can't Wear This" (14400): Grandma's Good Clothes chest spawned in the wrong
-- phase (181); the quest runs entirely in phase 183 like its sibling chest 196473
UPDATE `gameobject` SET `PhaseId` = 183 WHERE `guid` = 236357;
