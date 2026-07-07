-- Report for Tryouts: Coach Crosscheck is only a questgiver at Kajaro Field.
-- The sniff confirms him as the completion NPC for quest 24567, but does not
-- include a standalone gossip text before the quest hand-in. Remove the custom
-- placeholder gossip path so the client opens the quest turn-in directly.
-- Keep UNIT_NPC_FLAG_QUESTGIVER (2), clear UNIT_NPC_FLAG_GOSSIP (1).
UPDATE `creature_template`
SET `npcflag` = (`npcflag` | 2) & 4294967294,
    `gossip_menu_id` = 0
WHERE `entry` = 37106;

DELETE FROM `gossip_menu` WHERE `MenuID` = 10884;

DELETE FROM `npc_text`
WHERE `ID` IN (10884, 17551)
  AND `text0_0` = 'Missing npc_text';
