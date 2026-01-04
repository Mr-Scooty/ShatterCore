-- Add missing gossip text for Sister Goldskimmer non-priests.
DELETE FROM `gossip_menu` WHERE `MenuID` = 10685;
INSERT INTO `gossip_menu` (`MenuID`, `TextID`, `VerifiedBuild`) VALUES
(10685, 14601, 15595);

INSERT INTO `npc_text` (`ID`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`)
VALUES
(14601, 'You don\'t have to be a priest to tithe. Your coin is just as welcome, $g sir:ma\'am;.', '', 0, 0, 1),
(14602, 'You don\'t have to be a priest to tithe. Your coin is just as welcome, $g sir:ma\'am;.', '', 0, 0, 1)
ON DUPLICATE KEY UPDATE
`text0_0` = VALUES(`text0_0`),
`text0_1` = VALUES(`text0_1`),
`BroadcastTextID0` = VALUES(`BroadcastTextID0`),
`lang0` = VALUES(`lang0`),
`Probability0` = VALUES(`Probability0`);
