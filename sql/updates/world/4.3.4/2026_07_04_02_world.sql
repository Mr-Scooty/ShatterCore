-- Lost Isles Act 1: import creature_text from WowPacketParser sniff dumps (Goblin starting zone)

-- Doc Zapnozzle (36608)
DELETE FROM `creature_text` WHERE `CreatureID`=36608 AND `GroupID` BETWEEN 0 AND 6;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(36608, 0, 0, 'Gizmo, what are you doing just sitting there? Don\'t you recognize who that is laying next to you?!', 12, 0, 100, 396, 0, 0, 0, 0, 0, 'Doc Zapnozzle to Player'),
(36608, 1, 0, 'That\'s $n! $G He\'s : She\'s; the whole reason we\'re still breathing and not crispy fried critters back on Kezan.', 12, 0, 100, 396, 0, 0, 0, 0, 0, 'Doc Zapnozzle to Player'),
(36608, 2, 0, 'Stay back, I\'m going to resuscitate $g him : her;! I hope these wet jumper cables don\'t kill us all!', 12, 0, 100, 396, 0, 0, 0, 0, 0, 'Doc Zapnozzle to Player'),
(36608, 3, 0, 'Come on! Clear!', 12, 0, 100, 396, 0, 0, 0, 0, 0, 'Doc Zapnozzle to Player'),
(36608, 4, 0, 'That\'s all I\'ve got. It\'s up to $g him : her; now. You hear me, $n? Come on, snap out of it! Don\'t go into the Light!', 12, 0, 100, 396, 0, 0, 0, 0, 0, 'Doc Zapnozzle to Player'),
(36608, 5, 0, 'You made the right choice. We all owe you a great deal, $n. Try not to get yourself killed out here.', 12, 0, 100, 396, 0, 0, 0, 0, 0, 'Doc Zapnozzle to Player'),
(36608, 6, 0, 'There are more survivors to tend to. I\'ll see you on the shore.', 12, 0, 100, 397, 0, 0, 0, 0, 0, 'Doc Zapnozzle to Player');

-- Geargrinder Gizmo (36600)
DELETE FROM `creature_text` WHERE `CreatureID`=36600 AND `GroupID` BETWEEN 1 AND 1;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(36600, 1, 0, 'That\'s $n?! Sorry, Doc, I thought $g he : she; was dead!', 12, 0, 100, 0, 0, 0, 0, 0, 0, 'Geargrinder Gizmo to Player');

-- Doc Zapnozzle (36615)
DELETE FROM `creature_text` WHERE `CreatureID`=36615 AND `GroupID` BETWEEN 1 AND 1;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(36615, 1, 0, 'I only have two hands and two feet here!', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Doc Zapnozzle to Goblin Survivor');

-- Frightened Miner (35813)
DELETE FROM `creature_text` WHERE `CreatureID`=35813 AND `GroupID` BETWEEN 5 AND 6;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(35813, 5, 0, 'We\'ve hit the jackpot in this place!', 12, 0, 100, 396, 0, 0, 0, 0, 0, 'Frightened Miner to Player'),
(35813, 6, 0, 'Let\'s move on.', 12, 0, 100, 396, 0, 0, 0, 0, 0, 'Frightened Miner to Player');

-- Pygmy Witchdoctor (35838)
DELETE FROM `creature_text` WHERE `CreatureID`=35838 AND `GroupID` BETWEEN 3 AND 3;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(35838, 3, 0, 'Ooga booga!', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Pygmy Witchdoctor');

-- Gyrochoppa Pilot (36129)
DELETE FROM `creature_text` WHERE `CreatureID`=36129 AND `GroupID` BETWEEN 0 AND 0;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(36129, 0, 0, 'Hey! Get away from my flying machine!', 12, 0, 100, 0, 0, 0, 0, 0, 0, 'Gyrochoppa Pilot to Player');

-- Thrall (36161)
DELETE FROM `creature_text` WHERE `CreatureID`=36161 AND `GroupID` BETWEEN 2 AND 2;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(36161, 2, 0, 'Speed of the storm, heed my call!', 14, 0, 100, 0, 0, 20144, 0, 0, 0, 'Thrall');

-- Evol Fingers (36519)
DELETE FROM `creature_text` WHERE `CreatureID`=36519 AND `GroupID` BETWEEN 0 AND 0;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(36519, 0, 0, 'That rocket looks unsafe even by our standards!', 12, 0, 100, 5, 0, 0, 0, 0, 0, 'Evol Fingers');
