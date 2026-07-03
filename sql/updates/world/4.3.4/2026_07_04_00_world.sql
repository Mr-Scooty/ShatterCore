-- The Lost Isles (zone 4720): foundations
-- Part 0: quest flags, chain repairs, gender gating, player-side phasing ladder.

-- ----------------------------------------------------------------------------
-- 1) QUEST_FLAGS_UPDATE_PHASESHIFT (0x400000) on ladder-advancing quests
--    (reward-triggered flips + belt-and-braces on the mid-quest flip quests).
-- ----------------------------------------------------------------------------
UPDATE `quest_template` SET `Flags` = `Flags` | 0x400000
WHERE `ID` IN (14303, 14240, 14242, 24868, 24929, 25251, 14245, 24958, 25184);

-- AUTO_ACCEPT (0x80000) on handoffs that cross a phase flip, so RewardQuest's
-- server-side auto-add beats the giver phasing out.
UPDATE `quest_template` SET `Flags` = `Flags` | 0x80000
WHERE `ID` IN (14237, 14241, 14326, 24897, 24937, 25265);

-- ----------------------------------------------------------------------------
-- 2) Player-side phasing ladder for zone 4720 (parent-area walk makes one row
--    per phase cover every sub-area). Exactly one phase at a time, as sniffed.
--    Type 8 = QUESTREWARDED, type 28 = QUESTCOMPLETE (mid-quest flips:
--    14245 town plunger, 24958 Volcanoth, 25184 mine cart ride end).
-- ----------------------------------------------------------------------------
DELETE FROM `phase_area` WHERE `AreaId` = 4720;
INSERT INTO `phase_area` (`AreaId`, `PhaseId`, `Comment`) VALUES
(4720, 170, 'Lost Isles - crash beach era'),
(4720, 171, 'Lost Isles - SI:7 vale era'),
(4720, 172, 'Lost Isles - cliffs/Gyrochoppa era'),
(4720, 179, 'Lost Isles - Thrall met / pre-town era'),
(4720, 180, 'Lost Isles - Town-in-a-Box era'),
(4720, 181, 'Lost Isles - naga coast era'),
(4720, 182, 'Lost Isles - Oomlot / volcano trek era'),
(4720, 183, 'Lost Isles - post-Volcanoth / labor mine era'),
(4720, 184, 'Lost Isles - stadium/finale era'),
(4720, 185, 'Lost Isles - victory era');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 26 AND `SourceEntry` = 4720;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(26, 170, 4720, 0, 0, 8,  0, 14126, 0, 0, 0, 0, 0, '', 'LI 170: Life Savings (Kezan) rewarded'),
(26, 170, 4720, 0, 0, 8,  0, 14303, 0, 0, 1, 0, 0, '', 'LI 170: NOT Back to Aggra rewarded'),
(26, 171, 4720, 0, 0, 8,  0, 14303, 0, 0, 0, 0, 0, '', 'LI 171: Back to Aggra rewarded'),
(26, 171, 4720, 0, 0, 8,  0, 14240, 0, 0, 1, 0, 0, '', 'LI 171: NOT To the Cliffs rewarded'),
(26, 172, 4720, 0, 0, 8,  0, 14240, 0, 0, 0, 0, 0, '', 'LI 172: To the Cliffs rewarded'),
(26, 172, 4720, 0, 0, 8,  0, 14242, 0, 0, 1, 0, 0, '', 'LI 172: NOT Precious Cargo rewarded'),
(26, 179, 4720, 0, 0, 8,  0, 14242, 0, 0, 0, 0, 0, '', 'LI 179: Precious Cargo rewarded'),
(26, 179, 4720, 0, 0, 28, 0, 14245, 0, 0, 1, 0, 0, '', 'LI 179: NOT Town-In-A-Box complete'),
(26, 179, 4720, 0, 0, 8,  0, 14245, 0, 0, 1, 0, 0, '', 'LI 179: NOT Town-In-A-Box rewarded'),
(26, 180, 4720, 0, 1, 28, 0, 14245, 0, 0, 0, 0, 0, '', 'LI 180: Town-In-A-Box complete'),
(26, 180, 4720, 0, 1, 8,  0, 24868, 0, 0, 1, 0, 0, '', 'LI 180: NOT Surrender or Else rewarded'),
(26, 180, 4720, 0, 2, 8,  0, 14245, 0, 0, 0, 0, 0, '', 'LI 180: Town-In-A-Box rewarded'),
(26, 180, 4720, 0, 2, 8,  0, 24868, 0, 0, 1, 0, 0, '', 'LI 180: NOT Surrender or Else rewarded'),
-- 181 persists until BOTH Oomlot quests are rewarded (the shaman captors are
-- phase-181 spawns and Free the Captives must stay completable).
(26, 181, 4720, 0, 1, 8,  0, 24868, 0, 0, 0, 0, 0, '', 'LI 181: Surrender or Else rewarded'),
(26, 181, 4720, 0, 1, 8,  0, 24929, 0, 0, 1, 0, 0, '', 'LI 181: NOT Send a Message rewarded'),
(26, 181, 4720, 0, 2, 8,  0, 24868, 0, 0, 0, 0, 0, '', 'LI 181: Surrender or Else rewarded'),
(26, 181, 4720, 0, 2, 8,  0, 24925, 0, 0, 1, 0, 0, '', 'LI 181: NOT Free the Captives rewarded'),
(26, 182, 4720, 0, 0, 8,  0, 24929, 0, 0, 0, 0, 0, '', 'LI 182: Send a Message rewarded'),
(26, 182, 4720, 0, 0, 8,  0, 24925, 0, 0, 0, 0, 0, '', 'LI 182: Free the Captives rewarded'),
(26, 182, 4720, 0, 0, 28, 0, 24958, 0, 0, 1, 0, 0, '', 'LI 182: NOT Volcanoth! complete'),
(26, 182, 4720, 0, 0, 8,  0, 24958, 0, 0, 1, 0, 0, '', 'LI 182: NOT Volcanoth! rewarded'),
(26, 183, 4720, 0, 1, 28, 0, 24958, 0, 0, 0, 0, 0, '', 'LI 183: Volcanoth! complete'),
(26, 183, 4720, 0, 1, 28, 0, 25184, 0, 0, 1, 0, 0, '', 'LI 183: NOT Mine Cart Ride complete'),
(26, 183, 4720, 0, 1, 8,  0, 25184, 0, 0, 1, 0, 0, '', 'LI 183: NOT Mine Cart Ride rewarded'),
(26, 183, 4720, 0, 2, 8,  0, 24958, 0, 0, 0, 0, 0, '', 'LI 183: Volcanoth! rewarded'),
(26, 183, 4720, 0, 2, 28, 0, 25184, 0, 0, 1, 0, 0, '', 'LI 183: NOT Mine Cart Ride complete'),
(26, 183, 4720, 0, 2, 8,  0, 25184, 0, 0, 1, 0, 0, '', 'LI 183: NOT Mine Cart Ride rewarded'),
(26, 184, 4720, 0, 1, 28, 0, 25184, 0, 0, 0, 0, 0, '', 'LI 184: Mine Cart Ride complete'),
(26, 184, 4720, 0, 1, 8,  0, 25251, 0, 0, 1, 0, 0, '', 'LI 184: NOT Final Confrontation rewarded'),
(26, 184, 4720, 0, 2, 8,  0, 25184, 0, 0, 0, 0, 0, '', 'LI 184: Mine Cart Ride rewarded'),
(26, 184, 4720, 0, 2, 8,  0, 25251, 0, 0, 1, 0, 0, '', 'LI 184: NOT Final Confrontation rewarded'),
(26, 185, 4720, 0, 0, 8,  0, 25251, 0, 0, 0, 0, 0, '', 'LI 185: Final Confrontation rewarded');

-- ----------------------------------------------------------------------------
-- 3) Chain repairs (sniff-verified).
-- ----------------------------------------------------------------------------
-- 14001 is an exact duplicate of 14474 (Goblin Escape Pods); the sniff uses
-- 14474. Its Prev references also lock out 14474-takers today.
DELETE FROM `creature_queststarter` WHERE `quest` = 14001;
DELETE FROM `creature_questender` WHERE `quest` = 14001;
DELETE FROM `disables` WHERE `sourceType` = 1 AND `entry` = 14001;
INSERT INTO `disables` (`sourceType`, `entry`, `flags`, `params_0`, `params_1`, `comment`) VALUES
(1, 14001, 0, '', '', 'Duplicate of 14474 (Goblin Escape Pods) - sniff uses 14474');
UPDATE `quest_template_addon` SET `PrevQuestID` = 14014 WHERE `ID` IN (14019, 14473);

-- Exclusive-group repairs (pairs held simultaneously in the sniff).
UPDATE `quest_template_addon` SET `ExclusiveGroup` = -24925 WHERE `ID` IN (24925, 24929);
UPDATE `quest_template_addon` SET `ExclusiveGroup` = -25122 WHERE `ID` IN (25122, 25123);
UPDATE `quest_template_addon` SET `ExclusiveGroup` = 0 WHERE `ID` IN (14031, 14233);
UPDATE `quest_template_addon` SET `ExclusiveGroup` = -25200, `NextQuestID` = 25204 WHERE `ID` = 25200;
UPDATE `quest_template_addon` SET `ExclusiveGroup` = 0, `NextQuestID` = 0 WHERE `ID` = 24859;
UPDATE `quest_template_addon` SET `ExclusiveGroup` = -25024, `NextQuestID` = 25058 WHERE `ID` IN (25024, 25093);

-- Era gates for Prev=0 quests offered by multi-phase givers.
UPDATE `quest_template_addon` SET `PrevQuestID` = 24924 WHERE `ID` IN (24925, 24929);
UPDATE `quest_template_addon` SET `PrevQuestID` = 25213 WHERE `ID` = 25214;
UPDATE `quest_template_addon` SET `PrevQuestID` = 25184 WHERE `ID` IN (25202, 25203, 25243, 25244);

-- Native accept-cast boardings (Player::AddQuest casts SourceSpellID).
UPDATE `quest_template_addon` SET `SourceSpellID` = 73532 WHERE `ID` = 25100; -- Let's Ride: summon+mount Bastia 39152
UPDATE `quest_template_addon` SET `SourceSpellID` = 89164 WHERE `ID` = 25213; -- The Slave Pits: Footbomb Uniform disguise
UPDATE `quest_template_addon` SET `SourceSpellID` = 73759 WHERE `ID` = 25184; -- Wild Mine Cart Ride: giver-cast board chain

-- Morale Boost hands out the Kaja'Cola Zero-One cans.
UPDATE `quest_template` SET `StartItem` = 52484 WHERE `ID` = 25122;
UPDATE `quest_template_addon` SET `ProvidedItemCount` = 20 WHERE `ID` = 25122;

-- The Warchief Wants You belongs to the zone (cosmetic sort fix).
UPDATE `quest_template` SET `QuestSortID` = 4720 WHERE `ID` = 25098;

-- Null broken reward spells: 73604 force-summons a personal Greely (replaced
-- by the static 39199 spawn); 73915/73962 do not exist in 4.3.4.
UPDATE `quest_template` SET `RewardSpell` = 0 WHERE `ID` = 25110 AND `RewardSpell` = 73604;
UPDATE `quest_template` SET `RewardSpell` = 0 WHERE `ID` = 25202 AND `RewardSpell` = 73915;
UPDATE `quest_template` SET `RewardSpell` = 0 WHERE `ID` = 25243 AND `RewardSpell` = 73962;
-- Null RewardSpells missing from the 4.3.4 client.
UPDATE `quest_template` SET `RewardSpell` = 0 WHERE `ID` = 14474 AND `RewardSpell` = 68685;
UPDATE `quest_template` SET `RewardSpell` = 0 WHERE `ID` = 14242 AND `RewardSpell` = 69082;
UPDATE `quest_template` SET `RewardSpell` = 0 WHERE `ID` = 24864 AND `RewardSpell` = 83137;

-- ----------------------------------------------------------------------------
-- 4) Gender gating for the romance branches (sniff-corrected pairing:
--    male = 25203 + 25243, female = 25202 + 25244).
--    Source 19 = QUEST_AVAILABLE, type 20 = GENDER (0 male / 1 female).
-- ----------------------------------------------------------------------------
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 19 AND `SourceEntry` IN (25202, 25203, 25243, 25244);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(19, 0, 25203, 0, 0, 20, 0, 0, 0, 0, 0, 0, 0, '', 'What Kind of Name is Chip, Anyway? (25203) males only'),
(19, 0, 25243, 0, 0, 20, 0, 0, 0, 0, 0, 0, 0, '', 'She Loves Me, She Loves Me NOT! (25243) males only'),
(19, 0, 25202, 0, 0, 20, 0, 1, 0, 0, 0, 0, 0, '', 'The Fastest Way to His Heart (25202) females only'),
(19, 0, 25244, 0, 0, 20, 0, 1, 0, 0, 0, 0, 0, '', 'What Kind of Name is Candy, Anyway? (25244) females only');
