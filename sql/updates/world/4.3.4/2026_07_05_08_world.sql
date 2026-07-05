-- Correct the starter and ender for quest 12697, Gothik the Harvester.
DELETE FROM `creature_queststarter` WHERE `quest` = 12697;
INSERT INTO `creature_queststarter` (`id`, `quest`) VALUES
(28377, 12697);

DELETE FROM `creature_questender` WHERE `quest` = 12697;
INSERT INTO `creature_questender` (`id`, `quest`) VALUES
(28658, 12697);
