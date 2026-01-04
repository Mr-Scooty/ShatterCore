-- Fix trainer gossip binding for Bipsi Frostflinger (42331).
UPDATE `creature_template`
SET `npcflag` = `npcflag` | 17
WHERE `entry` = 42331;

DELETE FROM `creature_trainer` WHERE `CreatureID` = 42331;
INSERT INTO `creature_trainer` (`CreatureID`, `TrainerID`, `MenuID`, `OptionID`)
VALUES (42331, 44, 11620, 3);

DELETE FROM `gossip_menu_option` WHERE `MenuID` = 11620 AND `OptionID` = 3;
INSERT INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`,
    `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`)
VALUES (11620, 3, 3, 'Train me!', 3266, 5, 16, 0, 0, 0, 0, '', 0);
