--
-- Death Knight quest 12848 "The Endless Hunger" — The Lich King whispers.
--
-- Bind the whisper scripts and wire the trigger: when a player pulls a Citizen of Havenshire
-- (28576 / 28577), the Citizen makes The Lich King (guid 128581 / entry 28765) cross-cast the
-- "Lich King VO Blocker" (58207) onto that player (50% on aggro). 58207 then random-fires one
-- of the whisper spells 58208-58223, each of which whispers a broadcast line + plays a sound.
--
DELETE FROM `spell_script_names` WHERE `spell_id` = 58207 AND `ScriptName` = 'spell_lich_king_vo_blocker';
DELETE FROM `spell_script_names` WHERE `spell_id` BETWEEN 58208 AND 58223 AND `ScriptName` = 'spell_lich_king_whisper';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(58207, 'spell_lich_king_vo_blocker'),
(58208, 'spell_lich_king_whisper'),
(58209, 'spell_lich_king_whisper'),
(58210, 'spell_lich_king_whisper'),
(58211, 'spell_lich_king_whisper'),
(58212, 'spell_lich_king_whisper'),
(58213, 'spell_lich_king_whisper'),
(58214, 'spell_lich_king_whisper'),
(58215, 'spell_lich_king_whisper'),
(58216, 'spell_lich_king_whisper'),
(58217, 'spell_lich_king_whisper'),
(58218, 'spell_lich_king_whisper'),
(58219, 'spell_lich_king_whisper'),
(58220, 'spell_lich_king_whisper'),
(58221, 'spell_lich_king_whisper'),
(58222, 'spell_lich_king_whisper'),
(58223, 'spell_lich_king_whisper');

-- Give the Citizens of Havenshire the on-aggro cross-cast (they had no SmartAI before).
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` IN (28576, 28577);

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND `entryorguid` IN (28576, 28577);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(28576, 0, 0, 0, 4, 0, 50, 0, 0, 0, 0, 0, 0, 86, 58207, 0, 10, 128581, 28765, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Citizen of Havenshire - On Aggro (50%) - Lich King (128581) cross-casts VO Blocker on player'),
(28577, 0, 0, 0, 4, 0, 50, 0, 0, 0, 0, 0, 0, 86, 58207, 0, 10, 128581, 28765, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Citizen of Havenshire - On Aggro (50%) - Lich King (128581) cross-casts VO Blocker on player');
