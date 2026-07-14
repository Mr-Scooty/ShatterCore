-- Life Savings (14126): the dock ride could not be started.
--
-- Sassy tells the player "Hand over the keys, boss. I'm driving. Just let me
-- know when you're ready to go." but her gossip menu (10547) shipped with no
-- options at all, and the only other trigger - the Keys to the Hot Rod item
-- (46856, on-use 66393 "Rolling with my Homies: Summon Hot Rod") - is the
-- source item of 14071 and is destroyed when that quest is rewarded, so by the
-- finale most players have no way to summon the Hot Rod and reach the yacht.
--
-- Retail (P2 sniff) starts the ride with the same 66393 chain (summon 34840 +
-- auto-board via SummonProperties 827 ride-spell BP 66392), so the restored
-- "ready to go" option simply has the player cast it. Everything downstream
-- already works: run-over AI on the Hot Rod, the port mortar (207355 -> 92629
-- -> 92633 jump onto the yacht), Gallywix's turn-in and the escape movie.
--
-- 14126 is a delivery quest (StartItem 49866 = the required item), so it is
-- COMPLETE in the log immediately - the option condition must accept both
-- in-progress and completed states (QUESTSTATE mask 2|8).
DELETE FROM `gossip_menu_option` WHERE `MenuID` = 10547;
INSERT INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcflag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES
(10547, 0, 0, 'I''m ready to go, Sassy.', 0, 1, 1, 0, 0, 0, 0, '', 0, 0);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 15 AND `SourceGroup` = 10547;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(15, 10547, 0, 0, 47, 0, 14126, 10, 0, 0, 0, 0, '', 'Sassy Hardwrench: show ready-to-go option while Life Savings is in the quest log');

DELETE FROM `smart_scripts` WHERE `entryorguid` = 34668 AND `source_type` = 0 AND `id` IN (20, 21);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(34668, 0, 20, 21, 62, 0, 100, 0, 10547, 0, 0, 0, 0, 72, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sassy Hardwrench - On Gossip Option 0 Selected - Close Gossip'),
(34668, 0, 21, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 85, 66393, 1, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Sassy Hardwrench - On Gossip Option 0 Selected - Invoker Cast ''Rolling with my Homies: Summon Hot Rod''');
