-- End Time (map 938): audit follow-up
-- Correct the Azure Dragonshrine trash kits, scope Jaina's counter world states,
-- and add Tyrande's missing slay line.

-- The client world-state rows must be map scoped. Without these templates the core
-- treats DoUpdateWorldState as a realm-wide update.
DELETE FROM `world_state` WHERE `ID` IN (6025,6046);
INSERT INTO `world_state` (`ID`, `DefaultValue`, `MapIDs`, `AreaIDs`, `ScriptName`, `Comment`) VALUES
(6025, 0, '938', '5793', '', 'End Time - Azure Dragonshrine - Collected Staff Fragments'),
(6046, 0, '938', '5793', '', 'End Time - Azure Dragonshrine - Show Staff Fragment Counter');

-- Exact 4.3.4 spell kits from Spell.dbc/SpellEffect.dbc. The previous Rifleman,
-- Priest and Sorceress rows used provisional generic spells, and Footmen/Fountains
-- had no AI at all.
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry` IN (54687,54690,54691,54693,54795);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (54687,54690,54691,54693,54795) AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
-- Time-Twisted Footman
(54687, 0, 0, 0, 0, 0, 100, 0, 3000, 5000, 8000, 12000, 0, 11, 101817, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Time-Twisted Footman - IC - Cast Shield Bash'),
(54687, 0, 1, 0, 0, 0, 100, 0, 6000, 9000, 12000, 16000, 0, 11, 101820, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Time-Twisted Footman - IC - Cast Thunderclap'),
(54687, 0, 2, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 101811, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Time-Twisted Footman - At 30% HP - Cast Shield Wall'),
-- Time-Twisted Priest
(54690, 0, 0, 0, 0, 0, 100, 0, 3000, 6000, 20000, 30000, 0, 11, 102405, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Time-Twisted Priest - IC - Cast Fountain of Light'),
(54690, 0, 1, 0, 2, 0, 100, 1, 0, 70, 0, 0, 0, 11, 102409, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Time-Twisted Priest - At 70% HP - Cast Power Word: Shield'),
-- Time-Twisted Sorceress
(54691, 0, 0, 0, 0, 0, 100, 0, 1000, 2000, 3000, 4000, 0, 11, 101816, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Time-Twisted Sorceress - IC - Cast Arcane Blast'),
(54691, 0, 1, 0, 0, 0, 100, 0, 8000, 12000, 15000, 20000, 0, 11, 101812, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Time-Twisted Sorceress - IC - Cast Blink'),
-- Time-Twisted Rifleman
(54693, 0, 0, 0, 0, 0, 100, 0, 1000, 2000, 3000, 4000, 0, 11, 102410, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Time-Twisted Rifleman - IC - Cast Shoot'),
(54693, 0, 1, 0, 0, 0, 100, 0, 5000, 8000, 10000, 14000, 0, 11, 102411, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Time-Twisted Rifleman - IC - Cast Multi-Shot'),
-- Fountain of Light (summoned by 102405; its verified static flags make it sessile/passive)
(54795, 0, 0, 0, 54, 0, 100, 0, 0, 0, 0, 0, 0, 11, 102406, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Fountain of Light - Just Summoned - Cast Light Rain');

DELETE FROM `creature_text` WHERE `CreatureID`=54544 AND `GroupID`=15;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54544, 15, 0, IF(`Text`='', `Text1`, `Text`), 14, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Echo of Tyrande - SAY_SLAY' FROM `broadcast_text` WHERE `ID`=53156;
