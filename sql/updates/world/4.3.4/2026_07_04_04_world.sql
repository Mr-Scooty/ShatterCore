-- Lost Isles Act 4: import creature_text from WowPacketParser sniff dumps (Goblin starting zone)

-- Trade Prince Gallywix (39582)
DELETE FROM `creature_text` WHERE `CreatureID`=39582;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(39582, 0, 0, 'I like you. Here\'s a raise!', 14, 0, 100, 0, 0, 19578, 0, 0, 0, 'Trade Prince Gallywix - finale line 0'),
(39582, 1, 0, 'I need to move these toxic assets onto a sucker... like you!', 14, 0, 100, 0, 0, 19570, 0, 0, 0, 'Trade Prince Gallywix - finale line 1'),
(39582, 2, 0, 'I SEE THE TRAITOR IS HERE TO RESCUE YOU, WARCHIEF. HOW CONVENIENT. YOU WILL BOTH BOW TO ME OR FALL TOGETHER!', 14, 0, 100, 0, 0, 19567, 0, 0, 0, 'Trade Prince Gallywix - finale line 2'),
(39582, 3, 0, 'You burned down my headquarters. Now I\'m gonna burn down you!', 14, 0, 100, 0, 0, 19576, 0, 0, 0, 'Trade Prince Gallywix - finale line 3'),
(39582, 4, 0, 'I\'m so money!', 14, 0, 100, 0, 0, 19579, 0, 0, 0, 'Trade Prince Gallywix - finale line 4'),
(39582, 5, 0, 'Excuse me while I dispose of these toxic assets all over you!', 14, 0, 100, 0, 0, 19569, 0, 0, 0, 'Trade Prince Gallywix - finale line 5'),
(39582, 6, 0, 'I\'m all fired up over finally gettin\' rid of you!', 14, 0, 100, 0, 0, 19575, 0, 0, 0, 'Trade Prince Gallywix - finale line 6'),
(39582, 7, 0, 'Eat it!', 14, 0, 100, 0, 0, 19566, 0, 0, 0, 'Trade Prince Gallywix - finale line 7'),
(39582, 8, 0, 'Here, I need to unload some toxic assets!', 14, 0, 100, 0, 0, 19568, 0, 0, 0, 'Trade Prince Gallywix - finale line 8'),
(39582, 9, 0, 'Uncle! Uncle! I give! You guys are too much for me!', 14, 0, 100, 20, 0, 19580, 0, 0, 0, 'Trade Prince Gallywix - finale line 9'),
(39582, 10, 0, 'I\'m beaten. You\'ve shown me the error of my ways. From here on out, I promise to reform the way the cartel is run!', 14, 0, 100, 274, 0, 19581, 0, 0, 0, 'Trade Prince Gallywix - finale line 10'),
(39582, 11, 0, 'I\'m your goblin, Thrall. What would you have of me?', 14, 0, 100, 6, 0, 19582, 0, 0, 0, 'Trade Prince Gallywix - finale line 11'),
(39582, 12, 0, 'It will be as you say! Long live the Bilgewater Cartel! For the Horde!', 14, 0, 100, 15, 0, 19583, 0, 0, 0, 'Trade Prince Gallywix - finale line 12');

-- Ace (39198)
DELETE FROM `creature_text` WHERE `CreatureID`=39198 AND `GroupID` BETWEEN 1 AND 2;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(39198, 1, 0, 'Mom detectors.', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Ace to Player'),
(39198, 2, 0, 'Distilling the juice out of kaja\'mite to make a delicious, carbonated beverage that will give people IDEAS! Hey, wait a minute...', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Ace to Player');

-- Izzy (39200)
DELETE FROM `creature_text` WHERE `CreatureID`=39200 AND `GroupID` BETWEEN 2 AND 3;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(39200, 2, 0, 'Electrical wires used to send messages over great distances... no, impractical. Giant rockets, with speakers attached....', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Izzy to Player'),
(39200, 3, 0, 'Tauren Paladins!', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Izzy to Player');

-- Gobber (39201)
DELETE FROM `creature_text` WHERE `CreatureID`=39201 AND `GroupID` BETWEEN 0 AND 1;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(39201, 0, 0, 'A spring-loaded plunger with blades attached, for processing food. Or people you disagree with.', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Gobber to Player'),
(39201, 1, 0, 'Upside-down boats!  They\'ll never capsize!', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Gobber to Player');

-- Captured Goblin (39456)
DELETE FROM `creature_text` WHERE `CreatureID`=39456 AND `GroupID` BETWEEN 6 AND 7;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(39456, 6, 0, 'What are those rockets for?!', 12, 0, 100, 0, 0, 0, 0, 0, 0, 'Captured Goblin to Player'),
(39456, 7, 0, 'The pirates have keys!', 12, 0, 100, 0, 0, 0, 0, 0, 0, 'Captured Goblin to Player');
