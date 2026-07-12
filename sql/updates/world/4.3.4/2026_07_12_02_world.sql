-- Kezan: "Robbing Hoods" (14121) - the Hot Rod could not run anything over.
-- Retail (Goblin_P2 sniff): while the Hot Rod moves, the driver spams 66301 (frontal cone
-- knockback, ~100ms cadence). A hit Hired Looter (35234) casts 67041 on the driver (creates
-- Stolen Loot 47530), is launched by the native knockback, and dies; Kezan Citizens
-- (35063/35075) and Villa Mooks (49218) are also launched, and the citizens yell at the driver.
-- Nothing cast 66301 here, so driving into looters just aggroed them and they burned down the
-- 102hp Hot Rod, dumping the driver into melee. Scripted in spell_kezan_hot_rod_run_over +
-- npc_hot_rod_vehicle.
DELETE FROM `spell_script_names` WHERE `spell_id`=66301;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(66301, 'spell_kezan_hot_rod_run_over');

-- 66301 uses TARGET_UNIT_CONE_ENTRY (60): without implicit-target conditions it selects nothing.
-- Sniffed victims: both Kezan Citizen entries, the Hired Looter and the Villa Mook.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry`=66301;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 66301, 0, 31, 0, 3, 35063, 0, 0, 0, 0, '', 'Hot Rod Knock Back targets Kezan Citizen'),
(13, 1, 66301, 1, 31, 0, 3, 35075, 0, 0, 0, 0, '', 'Hot Rod Knock Back targets Kezan Citizen'),
(13, 1, 66301, 2, 31, 0, 3, 35234, 0, 0, 0, 0, '', 'Hot Rod Knock Back targets Hired Looter'),
(13, 1, 66301, 3, 31, 0, 3, 49218, 0, 0, 0, 0, '', 'Hot Rod Knock Back targets Villa Mook');

-- Run-over yells, all sniffed (MonsterYell targeted at the driver, sounds/emotes as captured).
DELETE FROM `creature_text` WHERE `CreatureID` IN (35063,35075) AND `GroupID`=0;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(35063, 0, 0, 'Learn how to drive you maniac!', 14, 0, 100, 5, 0, 1411, 0, 0, 'Kezan Citizen - run over by Hot Rod'),
(35063, 0, 1, 'The Trade Prince will hear about this, $n!', 14, 0, 100, 5, 0, 1411, 0, 0, 'Kezan Citizen - run over by Hot Rod'),
(35063, 0, 2, 'You''re a public nuisance, $n!', 14, 0, 100, 5, 0, 1411, 0, 0, 'Kezan Citizen - run over by Hot Rod'),
(35063, 0, 3, 'You''re gonna hear from my lawyer!', 14, 0, 100, 25, 0, 1411, 0, 0, 'Kezan Citizen - run over by Hot Rod'),
(35063, 0, 4, 'My neck! I''m gonna sue!', 14, 0, 100, 6, 0, 18500, 0, 0, 'Kezan Citizen - run over by Hot Rod'),
(35063, 0, 5, 'Watch where you''re going!', 14, 0, 100, 5, 0, 1411, 0, 0, 'Kezan Citizen - run over by Hot Rod'),
(35063, 0, 6, 'I''ll get you, $n!', 14, 0, 100, 5, 0, 1411, 0, 0, 'Kezan Citizen - run over by Hot Rod'),
(35063, 0, 7, 'Ouch!', 14, 0, 100, 5, 0, 1411, 0, 0, 'Kezan Citizen - run over by Hot Rod'),
(35075, 0, 0, 'Watch where you''re going!', 14, 0, 100, 11, 0, 18500, 0, 0, 'Kezan Citizen - run over by Hot Rod'),
(35075, 0, 1, 'The Trade Prince will hear about this, $n!', 14, 0, 100, 92, 0, 18500, 0, 0, 'Kezan Citizen - run over by Hot Rod'),
(35075, 0, 2, 'Learn how to drive you maniac!', 14, 0, 100, 5, 0, 1411, 0, 0, 'Kezan Citizen - run over by Hot Rod'),
(35075, 0, 3, 'You''re gonna hear from my lawyer!', 14, 0, 100, 1, 0, 18500, 0, 0, 'Kezan Citizen - run over by Hot Rod'),
(35075, 0, 4, 'Ouch!', 14, 0, 100, 5, 0, 12924, 0, 0, 'Kezan Citizen - run over by Hot Rod'),
(35075, 0, 5, 'I''ll get you, $n!', 14, 0, 100, 5, 0, 18500, 0, 0, 'Kezan Citizen - run over by Hot Rod'),
(35075, 0, 6, 'You''re a public nuisance, $n!', 14, 0, 100, 5, 0, 1411, 0, 0, 'Kezan Citizen - run over by Hot Rod'),
(35075, 0, 7, 'My neck! I''m gonna sue!', 14, 0, 100, 5, 0, 1411, 0, 0, 'Kezan Citizen - run over by Hot Rod');

-- Hot Rod unit_flags: sniffed create shows 0x4008 = CANNOT_SWIM + PLAYER_CONTROLLED (dynamic).
-- The imported 0x80000 (IN_COMBAT) template flag is nonsense; store CANNOT_SWIM (16384).
UPDATE `creature_template` SET `unit_flags`=16384 WHERE `entry` IN (34840,37676);
