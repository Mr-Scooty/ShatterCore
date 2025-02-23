-- Fix early availability of "Grandma Wahl" and "The Hayward Brothers"
UPDATE quest_template_addon 
SET PrevQuestID = 14397 
WHERE ID IN (14398, 14403, 14406);