-- Lost Isles Acts 2-3: import creature_text from WowPacketParser sniff dumps (Goblin starting zone)

-- Ace (38455)
DELETE FROM `creature_text` WHERE `CreatureID`=38455 AND `GroupID` BETWEEN 0 AND 5;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(38455, 0, 0, 'I got these little hellions on a short leash, $n. The naga won\'t attack us while we have their hatchlings.', 12, 0, 100, 1, 0, 0, 0, 0, 0, 'Ace to Player'),
(38455, 1, 0, 'You ready to make their leader surrender, $g buddy : lady;? Okay, here we go.', 12, 0, 100, 1, 0, 0, 0, 0, 0, 'Ace to Player'),
(38455, 2, 0, 'Out of our way, or your hatchlings get it!', 14, 0, 100, 5, 0, 0, 0, 0, 0, 'Ace'),
(38455, 3, 0, 'You naga keep your distance. We\'re not kidding around here!', 14, 0, 100, 5, 0, 0, 0, 0, 0, 'Ace'),
(38455, 4, 0, 'Alright, naga leader, come on out from hiding and surrender in the name of $n and the Bilgewater Cartel!', 14, 0, 100, 5, 0, 0, 0, 0, 0, 'Ace to Player'),
(38455, 5, 0, 'Um, dude, this does not look good. I\'m out of here!', 12, 0, 100, 5, 0, 0, 0, 0, 0, 'Ace to Faceless of the Deep');

-- Goblin Captive (38643)
DELETE FROM `creature_text` WHERE `CreatureID`=38643 AND `GroupID` BETWEEN 4 AND 6;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(38643, 4, 0, 'I felt my life slipping out of me.', 12, 0, 100, 1, 0, 0, 0, 0, 0, 'Goblin Captive to Oomlot Shaman'),
(38643, 5, 0, 'I gotta get out of here before they catch me again!', 12, 0, 100, 5, 0, 0, 0, 0, 0, 'Goblin Captive to Oomlot Shaman'),
(38643, 6, 0, 'I\'m free! I owe you my life!', 12, 0, 100, 4, 0, 0, 0, 0, 0, 'Goblin Captive to Oomlot Shaman');

-- Yngwie (38696)
DELETE FROM `creature_text` WHERE `CreatureID`=38696 AND `GroupID` BETWEEN 2 AND 4;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(38696, 2, 0, 'Meesa na watun ta longa!', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Yngwie'),
(38696, 3, 0, 'Killum gobins! Na cayno da BOOM!', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Yngwie'),
(38696, 4, 0, 'F\'dun ta gobins ta da Beeg Badda!', 14, 0, 100, 15, 0, 0, 0, 0, 0, 'Yngwie');

-- Kezan Citizen (38745)
DELETE FROM `creature_text` WHERE `CreatureID`=38745 AND `GroupID` BETWEEN 4 AND 11;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(38745, 4, 0, 'That\'s right. Look at the pretty flags.', 14, 0, 100, 274, 0, 0, 0, 0, 0, 'Kezan Citizen to B.C. Eliminator'),
(38745, 5, 0, 'Medic!', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Kezan Citizen'),
(38745, 6, 0, 'Murder permits!', 14, 0, 100, 0, 0, 2304, 0, 0, 0, 'Kezan Citizen'),
(38745, 7, 0, 'Tether-footbomb!', 14, 0, 100, 0, 0, 2304, 0, 0, 0, 'Kezan Citizen'),
(38745, 8, 0, 'Stairways... for horizontal surfaces!', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Kezan Citizen'),
(38745, 9, 0, 'Feed pigs rubber, \'til they bounce. There\'s got to be an application for that.', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Kezan Citizen'),
(38745, 10, 0, 'Thirteen-sided dice!', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Kezan Citizen'),
(38745, 11, 0, 'Clowns. Except instead of making you laugh, they\'re there for beating.', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Kezan Citizen');

-- Goblin Survivor (38409)
DELETE FROM `creature_text` WHERE `CreatureID`=38409 AND `GroupID` BETWEEN 13 AND 18;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(38409, 13, 0, 'Have you heard of Azshara? There are no zombies there.', 14, 0, 100, 25, 0, 0, 0, 0, 0, 'Goblin Survivor to B.C. Eliminator'),
(38409, 14, 0, 'Attach two vehicles to a bigger vehicle, and then have the passengers jump from one vehicle to the other!', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Goblin Survivor'),
(38409, 15, 0, 'Water-proof soap!  For underwater use!', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Goblin Survivor'),
(38409, 16, 0, 'A spring-loaded plunger with blades attached, for processing food. Or people you disagree with.', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Goblin Survivor'),
(38409, 17, 0, 'It\'s like bungie-jumping, right? But without a cord, see? And you start at the bottom. I guess it\'s just sorta like regular jumping. But with guns.', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Goblin Survivor'),
(38409, 18, 0, 'What if we were to ORGANIZE crime?', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Goblin Survivor');

-- Kaja'Cola Balloon (37804)
DELETE FROM `creature_text` WHERE `CreatureID`=37804 AND `GroupID` BETWEEN 1 AND 3;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(37804, 1, 0, 'Enjoy the refreshingly intelligent taste of all new Kaja\'Cola!', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Kaja\'Cola Balloon'),
(37804, 2, 0, 'Kaja\'Cola! It gives you IDEAS!', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Kaja\'Cola Balloon'),
(37804, 3, 0, 'Kaja\'Cola! Jinxy the Weasel says, Drink it... or else!', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Kaja\'Cola Balloon');

-- Kilag Gorefang (39066)
DELETE FROM `creature_text` WHERE `CreatureID`=39066 AND `GroupID` BETWEEN 1 AND 1;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(39066, 1, 0, 'Lok\'tar ogar!', 14, 1, 100, 15, 0, 0, 0, 0, 0, 'Kilag Gorefang');

-- Voodoo Illusion (40722)
DELETE FROM `creature_text` WHERE `CreatureID`=40722 AND `GroupID` BETWEEN 0 AND 0;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(40722, 0, 0, 'GOBIN!!!', 12, 0, 100, 0, 0, 0, 0, 0, 0, 'Voodoo Illusion to Player');
