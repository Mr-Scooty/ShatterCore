-- Fix Kezan Goblin quest availability (starter chain + bank/new look gating)
INSERT INTO `quest_template_addon` (
  `ID`, `MaxLevel`, `AllowableClasses`, `SourceSpellID`, `PrevQuestID`, `NextQuestID`, `ExclusiveGroup`,
  `BreadcrumbForQuestId`, `RewardMailTemplateID`, `RewardMailDelay`, `RequiredSkillID`, `RequiredSkillPoints`,
  `RequiredMinRepFaction`, `RequiredMaxRepFaction`, `RequiredMinRepValue`, `RequiredMaxRepValue`,
  `ProvidedItemCount`, `SpecialFlags`, `AllowableRaces`, `TimeAllowed`
) VALUES
(14138, 0, 0, 0, 0, 14069, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 256, 0),
(14069, 0, 0, 0, 14138, 25473, -14069, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0),
(14075, 0, 0, 0, 14138, 25473, -14069, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0),
(25473, 0, 0, 0, 0, 28349, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 256, 0),
(28349, 0, 0, 0, 25473, 14071, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0),
(14071, 0, 0, 0, 28349, 24567, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 256, 0),
(14070, 0, 0, 0, 14071, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0),
(26711, 0, 0, 0, 14071, 14110, 26711, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0),
(26712, 0, 0, 0, 14071, 14109, 26711, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0),
(14109, 0, 0, 0, 26712, 14113, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0),
(14110, 0, 0, 0, 26711, 14153, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0),
(14113, 0, 0, 0, 14109, 14115, 14113, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0),
(14153, 0, 0, 0, 14110, 14115, 14113, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0)
ON DUPLICATE KEY UPDATE
  `PrevQuestID` = VALUES(`PrevQuestID`),
  `NextQuestID` = VALUES(`NextQuestID`),
  `ExclusiveGroup` = VALUES(`ExclusiveGroup`);

-- Remove "The Keys to the Hot Rod" quest from Megs Dreadshredder
DELETE cq
FROM `creature_queststarter` AS cq
INNER JOIN `quest_template` AS qt ON qt.`ID` = cq.`quest`
WHERE cq.`id` = 34874
  AND qt.`LogTitle` = 'The Keys to the Hot Rod';

DELETE cq
FROM `creature_questender` AS cq
INNER JOIN `quest_template` AS qt ON qt.`ID` = cq.`quest`
WHERE cq.`id` = 34874
  AND qt.`LogTitle` = 'The Keys to the Hot Rod';
