-- Goblin Escape Pods (14474, dormant twin 14001) could be accepted before finishing
-- Don't Go Into the Light! (14239). Gate both on 14239 being rewarded, matching retail.
UPDATE `quest_template_addon` SET `PrevQuestID`=14239 WHERE `ID` IN (14001,14474);
