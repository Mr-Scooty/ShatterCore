-- The Enemy of My Enemy (14234) requires ALL of Miner Troubles (14021), Capturing the
-- Unknown (14031) and Orcs Can Write? (14233). Only 14021 sat in a (single-member)
-- negative exclusive group, so rewarding any one of the three unlocked 14234.
-- Put all three in group -14021 (each already has NextQuestID=14234), and gate 14031/14233
-- on Help Wanted (14248) like 14021.
UPDATE `quest_template_addon` SET `PrevQuestID`=14248 WHERE `ID` IN (14031,14233);
UPDATE `quest_template_addon` SET `ExclusiveGroup`=-14021 WHERE `ID` IN (14021,14031,14233);
