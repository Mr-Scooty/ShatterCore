-- Kezan quest chain: foundations for "Report for Tryouts" through "Life Savings"
-- Part 0: chain fixes, gender gating, player-side phasing (phase_area + conditions)

-- ----------------------------------------------------------------------------
-- 1) Quest template flags: QUEST_FLAGS_UPDATE_PHASESHIFT (0x400000) on every
--    quest whose reward advances the Kezan phase ladder (378->384 era states).
--    Without this flag Player::RewardQuest never calls PhasingHandler::OnConditionChange.
-- ----------------------------------------------------------------------------
UPDATE `quest_template` SET `Flags` = `Flags` | 0x400000
WHERE `ID` IN (24520, 14109, 14110, 14121, 14122, 14123, 14124);

-- ----------------------------------------------------------------------------
-- 2) Life of the Party requires BOTH The New You (PrevQuestID; a second
--    dependent-previous entry would be OR-ed, not AND-ed) and Give Sassy the
--    News (enforced via a quest-available condition below). Also redirect the
--    auto-offered follow-up of Necessary Roughness to the auto-accept variant
--    of Fourth and Goal (28414) that Coach Crosscheck already starts.
-- ----------------------------------------------------------------------------
UPDATE `quest_template_addon` SET `PrevQuestID` = 14109 WHERE `ID` = 14113;
UPDATE `quest_template_addon` SET `PrevQuestID` = 14110 WHERE `ID` = 14153;
UPDATE `quest_template` SET `RewardNextQuest` = 28414 WHERE `ID` = 24502;

-- ----------------------------------------------------------------------------
-- 3) Gender gating for the girlfriend/boyfriend quest variants.
--    Male path via Candy Cane: 26712 -> 14109 -> 14113
--    Female path via Chip Endale: 26711 -> 14110 -> 14153
--    CONDITION_SOURCE_TYPE_QUEST_AVAILABLE (19), CONDITION_GENDER (20): value1 0=male, 1=female.
-- ----------------------------------------------------------------------------
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 19 AND `SourceEntry` IN (26711, 26712, 14109, 14110, 14113, 14153);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(19, 0, 26712, 0, 0, 20, 0, 0, 0, 0, 0, 0, 0, '', 'Off to the Bank (26712) only available to males'),
(19, 0, 14109, 0, 0, 20, 0, 0, 0, 0, 0, 0, 0, '', 'The New You (14109) only available to males'),
(19, 0, 14113, 0, 0, 20, 0, 0, 0, 0, 0, 0, 0, '', 'Life of the Party (14113) only available to males'),
(19, 0, 26711, 0, 0, 20, 0, 1, 0, 0, 0, 0, 0, '', 'Off to the Bank (26711) only available to females'),
(19, 0, 14110, 0, 0, 20, 0, 1, 0, 0, 0, 0, 0, '', 'The New You (14110) only available to females'),
(19, 0, 14153, 0, 0, 20, 0, 1, 0, 0, 0, 0, 0, '', 'Life of the Party (14153) only available to females'),
(19, 0, 14113, 0, 0, 8, 0, 24520, 0, 0, 0, 0, 0, '', 'Life of the Party (14113) requires Give Sassy the News rewarded'),
(19, 0, 14153, 0, 0, 8, 0, 24520, 0, 0, 0, 0, 0, '', 'Life of the Party (14153) requires Give Sassy the News rewarded');

-- ----------------------------------------------------------------------------
-- 4) Remove TDB junk: Steamwheedle Shark as queststarter of Necessary Roughness.
-- ----------------------------------------------------------------------------
DELETE FROM `creature_queststarter` WHERE `quest` = 24502 AND `id` = 37114;

-- ----------------------------------------------------------------------------
-- 5) Player-side phasing for Kezan (zone 4737). Exactly one progression phase
--    at a time; base state (pre-24520) has no phase rows so players keep the
--    Unphased flag and see the default (PhaseId 169) spawns.
--    Ladder: 379 party era -> 380 pirate crashers -> 381 cleanup -> 382 bazillion
--    macaroons -> 383 heist -> 384 evacuation.
--    Conditions: source 26 (PHASE), SourceGroup = PhaseId, SourceEntry = AreaId.
--    Type 8 = QUESTREWARDED; NegativeCondition=1 inverts. Same ElseGroup = AND.
-- ----------------------------------------------------------------------------
DELETE FROM `phase_area` WHERE `AreaId` = 4737;
INSERT INTO `phase_area` (`AreaId`, `PhaseId`, `Comment`) VALUES
(4737, 379, 'Kezan - KTC party era'),
(4737, 380, 'Kezan - pirate party crashers era'),
(4737, 381, 'Kezan - post-crasher cleanup era'),
(4737, 382, 'Kezan - a bazillion macaroons era'),
(4737, 383, 'Kezan - bank heist era'),
(4737, 384, 'Kezan - evacuation era');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 26 AND `SourceEntry` = 4737;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
-- Phase 379: 24520 rewarded AND (New You m/f rewarded) AND NOT Life of the Party rewarded
(26, 379, 4737, 0, 1, 8, 0, 24520, 0, 0, 0, 0, 0, '', 'Kezan phase 379: Give Sassy the News rewarded'),
(26, 379, 4737, 0, 1, 8, 0, 14109, 0, 0, 0, 0, 0, '', 'Kezan phase 379: The New You (male) rewarded'),
(26, 379, 4737, 0, 1, 8, 0, 14113, 0, 0, 1, 0, 0, '', 'Kezan phase 379: NOT Life of the Party (male) rewarded'),
(26, 379, 4737, 0, 1, 8, 0, 14153, 0, 0, 1, 0, 0, '', 'Kezan phase 379: NOT Life of the Party (female) rewarded'),
(26, 379, 4737, 0, 2, 8, 0, 24520, 0, 0, 0, 0, 0, '', 'Kezan phase 379: Give Sassy the News rewarded'),
(26, 379, 4737, 0, 2, 8, 0, 14110, 0, 0, 0, 0, 0, '', 'Kezan phase 379: The New You (female) rewarded'),
(26, 379, 4737, 0, 2, 8, 0, 14113, 0, 0, 1, 0, 0, '', 'Kezan phase 379: NOT Life of the Party (male) rewarded'),
(26, 379, 4737, 0, 2, 8, 0, 14153, 0, 0, 1, 0, 0, '', 'Kezan phase 379: NOT Life of the Party (female) rewarded'),
-- Phase 380: Life of the Party rewarded AND NOT Pirate Party Crashers rewarded
(26, 380, 4737, 0, 1, 8, 0, 14113, 0, 0, 0, 0, 0, '', 'Kezan phase 380: Life of the Party (male) rewarded'),
(26, 380, 4737, 0, 1, 8, 0, 14115, 0, 0, 1, 0, 0, '', 'Kezan phase 380: NOT Pirate Party Crashers rewarded'),
(26, 380, 4737, 0, 2, 8, 0, 14153, 0, 0, 0, 0, 0, '', 'Kezan phase 380: Life of the Party (female) rewarded'),
(26, 380, 4737, 0, 2, 8, 0, 14115, 0, 0, 1, 0, 0, '', 'Kezan phase 380: NOT Pirate Party Crashers rewarded'),
-- Phase 381: 14115 rewarded AND NOT 14116 rewarded
(26, 381, 4737, 0, 0, 8, 0, 14115, 0, 0, 0, 0, 0, '', 'Kezan phase 381: Pirate Party Crashers rewarded'),
(26, 381, 4737, 0, 0, 8, 0, 14116, 0, 0, 1, 0, 0, '', 'Kezan phase 381: NOT The Uninvited Guest rewarded'),
-- Phase 382: 14116 rewarded AND NOT 14120 rewarded
(26, 382, 4737, 0, 0, 8, 0, 14116, 0, 0, 0, 0, 0, '', 'Kezan phase 382: The Uninvited Guest rewarded'),
(26, 382, 4737, 0, 0, 8, 0, 14120, 0, 0, 1, 0, 0, '', 'Kezan phase 382: NOT A Bazillion Macaroons?! rewarded'),
-- Phase 383: 14120 rewarded AND NOT all four heist quests rewarded (four OR-groups, one per missing heist quest)
(26, 383, 4737, 0, 1, 8, 0, 14120, 0, 0, 0, 0, 0, '', 'Kezan phase 383: A Bazillion Macaroons?! rewarded'),
(26, 383, 4737, 0, 1, 8, 0, 14121, 0, 0, 1, 0, 0, '', 'Kezan phase 383: NOT Robbing Hoods rewarded'),
(26, 383, 4737, 0, 2, 8, 0, 14120, 0, 0, 0, 0, 0, '', 'Kezan phase 383: A Bazillion Macaroons?! rewarded'),
(26, 383, 4737, 0, 2, 8, 0, 14122, 0, 0, 1, 0, 0, '', 'Kezan phase 383: NOT The Great Bank Heist rewarded'),
(26, 383, 4737, 0, 3, 8, 0, 14120, 0, 0, 0, 0, 0, '', 'Kezan phase 383: A Bazillion Macaroons?! rewarded'),
(26, 383, 4737, 0, 3, 8, 0, 14123, 0, 0, 1, 0, 0, '', 'Kezan phase 383: NOT Waltz Right In rewarded'),
(26, 383, 4737, 0, 4, 8, 0, 14120, 0, 0, 0, 0, 0, '', 'Kezan phase 383: A Bazillion Macaroons?! rewarded'),
(26, 383, 4737, 0, 4, 8, 0, 14124, 0, 0, 1, 0, 0, '', 'Kezan phase 383: NOT Liberate the Kaja''mite rewarded'),
-- Phase 384: all four heist quests rewarded (persists through 447/Life Savings)
(26, 384, 4737, 0, 0, 8, 0, 14121, 0, 0, 0, 0, 0, '', 'Kezan phase 384: Robbing Hoods rewarded'),
(26, 384, 4737, 0, 0, 8, 0, 14122, 0, 0, 0, 0, 0, '', 'Kezan phase 384: The Great Bank Heist rewarded'),
(26, 384, 4737, 0, 0, 8, 0, 14123, 0, 0, 0, 0, 0, '', 'Kezan phase 384: Waltz Right In rewarded'),
(26, 384, 4737, 0, 0, 8, 0, 14124, 0, 0, 0, 0, 0, '', 'Kezan phase 384: Liberate the Kaja''mite rewarded');
