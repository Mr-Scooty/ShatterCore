--
-- Gilneas (Worgen starter zone) — script bindings for gilneas_chapter_3/4.cpp
-- Ships together with the C++ (AddSC_gilneas_chapter_3/4).
--

-- SpellScripts
DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_gilneas_king_observatory', 'spell_gilneas_alas_gilneas', 'spell_gilneas_belysras_talisman', 'spell_gilneas_horn_of_taldoren', 'spell_gilneas_half_burnt_torch', 'spell_gilneas_blessed_offerings', 'spell_gilneas_summon_tobias', 'spell_gilneas_funeral_camera_summon');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(68953, 'spell_gilneas_king_observatory'),
(69257, 'spell_gilneas_alas_gilneas'),
(70797, 'spell_gilneas_belysras_talisman'),
(71061, 'spell_gilneas_horn_of_taldoren'),
(70631, 'spell_gilneas_half_burnt_torch'),
(72853, 'spell_gilneas_blessed_offerings'),
(72470, 'spell_gilneas_summon_tobias'),
(94244, 'spell_gilneas_funeral_camera_summon');

-- Creature AIs
UPDATE `creature_template` SET `ScriptName` = 'npc_swift_mountain_horse' WHERE `entry` = 36741;
UPDATE `creature_template` SET `ScriptName` = 'npc_gilneas_dark_scout' WHERE `entry` = 37953;
UPDATE `creature_template` SET `ScriptName` = 'npc_battle_for_gilneas_controller' WHERE `entry` = 38218;
UPDATE `creature_template` SET `ScriptName` = 'npc_gilneas_tobias_hunt' WHERE `entry` = 38507;
UPDATE `creature_template` SET `ScriptName` = 'npc_gilneas_captured_riding_bat' WHERE `entry` = 38540;
UPDATE `creature_template` SET `ScriptName` = 'npc_gilneas_funeral_controller' WHERE `entry` = 50893;
UPDATE `creature_template` SET `ScriptName` = 'npc_gilneas_funeral_camera' WHERE `entry` = 51083;
UPDATE `creature_template` SET `ScriptName` = 'npc_gilneas_glaive_thrower' WHERE `entry` = 37927;
UPDATE `creature_template` SET `ScriptName` = 'npc_gilneas_endgame_controller' WHERE `entry` = 43566;
UPDATE `creature_template` SET `ScriptName` = 'npc_gilneas_endgame_hippogryph' WHERE `entry` = 43751;
UPDATE `creature_template` SET `ScriptName` = 'npc_gilneas_endgame_wyvern' WHERE `entry` = 43713;

-- "The Hunt For Sylvanas" (24902) completes via the scripted cathedral scene
-- (AreaExploredOrEventHappens in npc_gilneas_tobias_hunt)
UPDATE `quest_template_addon` SET `SpecialFlags` = 2 WHERE `ID` = 24902;

-- No implicit-target conditions for 70797/94244: their dests/targets are pinned by
-- SpellScripts (a src13 row on 70797 would hard-fail the no-scout fallback branch).
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` IN (94244, 70797);

-- Dark Scout shipped friendly (faction 35) — make it a hostile Forsaken like its
-- Dark Ranger kin or the 24616 kill objective is unreachable
UPDATE `creature_template` SET `faction` = 2213 WHERE `entry` = 37953;

-- Dark Scout ambush (24616): sniff-proven trail trigger (35374 guid 256204) casts
-- 70794 on approaching quest holders (stun + forcecast 95845 + summon 37953).
-- The trigger must live in phase 186 or the phased player can never be seen by it
-- (PhaseGroup 372 is stale legacy data and conflicts with an explicit PhaseId).
UPDATE `creature` SET `PhaseId` = 186, `PhaseGroup` = 0 WHERE `guid` = 256204;
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` = 35374;
DELETE FROM `smart_scripts` WHERE `entryorguid` = -256204 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(-256204, 0, 0, 0, 10, 0, 100, 0, 1, 10, 30000, 30000, 0, 11, 70794, 2, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Generic Trigger LAB - OOC LOS - Cast Grab Player (Dark Scout ambush)');
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 22 AND `SourceEntry` = -256204;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(22, 1, -256204, 0, 0, 9, 0, 24616, 0, 0, 0, 0, 0, '', 'Ambush trigger: Losing Your Tail taken'),
(22, 1, -256204, 0, 0, 1, 0, 70794, 0, 0, 1, 0, 0, '', 'Ambush trigger: invoker not already grabbed'),
(22, 1, -256204, 0, 0, 29, 0, 37953, 50, 0, 1, 0, 0, '', 'Ambush trigger: no Dark Scout nearby');

-- Gwen Armstead coach re-ride (14465): gossip option + invoker-cast of the coach summon
DELETE FROM `gossip_menu_option` WHERE `MenuID` = 10833;
INSERT INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcflag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES
(10833, 0, 0, 'Ride to Greymane Manor.', 0, 1, 1, 0, 0, 0, 0, '', 0, 0);
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` = 36452;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 36452 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(36452, 0, 0, 1, 62, 0, 100, 0, 10833, 0, 0, 0, 0, 72, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Gwen Armstead - Gossip - Close'),
(36452, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 85, 69255, 2, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Gwen Armstead - Gossip - Invoker casts Summon Swift Mountain Horse (triggered)');
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 15 AND `SourceGroup` = 10833;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(15, 10833, 0, 0, 0, 9, 0, 14465, 0, 0, 0, 0, 0, '', 'Coach re-ride: To Greymane Manor taken'),
(15, 10833, 0, 0, 1, 8, 0, 14465, 0, 0, 0, 0, 0, '', 'Coach re-ride: To Greymane Manor rewarded...'),
(15, 10833, 0, 0, 1, 8, 0, 14466, 0, 0, 1, 0, 0, '', '...and The King''s Observatory not yet rewarded');

-- Battle for Gilneas City retry (24904): existing gossip option 12693/0 restarts the
-- controller via SetData(1, 1) on Prince Liam; gate the option on the quest
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` = 38611;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 38611 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(38611, 0, 0, 1, 62, 0, 100, 0, 12693, 0, 0, 0, 0, 72, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Lorna Crowley - Gossip - Close'),
(38611, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 45, 1, 1, 0, 0, 0, 0, 19, 38218, 500, 0, 0, 0, 0, 0, 'Lorna Crowley - Gossip - Restart battle controller (Set Data 1 1 on Prince Liam)');
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 15 AND `SourceGroup` = 12693;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(15, 12693, 0, 0, 0, 9, 0, 24904, 0, 0, 0, 0, 0, '', 'Battle retry: The Battle for Gilneas City taken');

-- Admiral Nightwind "I am ready." (12609): only after Endgame is rewarded
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 15 AND `SourceGroup` = 12609;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(15, 12609, 0, 0, 0, 8, 0, 26706, 0, 0, 0, 0, 0, '', 'Nightwind gossip: Endgame rewarded');

-- "Take Back What's Ours" (24646) pickup vignette: Tobias warns about the Scythe,
-- Darius answers (sniff-proven beat at the quest handoff)
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` = 37195;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 37195 AND `source_type` = 0;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 3719500 AND `source_type` = 9;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(37195, 0, 0, 0, 19, 0, 100, 0, 24646, 0, 0, 0, 0, 80, 3719500, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Lord Darius Crowley - On Quest 24646 Taken - Run Scythe vignette'),
(3719500, 9, 0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 19, 38051, 15, 0, 0, 0, 0, 0, 'Scythe vignette - Tobias Say 0'),
(3719500, 9, 1, 0, 0, 0, 100, 0, 2500, 2500, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Scythe vignette - Darius Say 0');

-- Endgame turn-in bark (26706): Lorna sends everyone to the ships
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` = 43727;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 43727 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(43727, 0, 0, 0, 20, 0, 100, 0, 26706, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Lorna Crowley - On Quest 26706 Rewarded - Say Line 0');

-- Missing creature_text lines (verbatim from the retail sniff; existing groups untouched)
DELETE FROM `creature_text` WHERE (`CreatureID` = 37953 AND `GroupID` IN (1, 2)) OR (`CreatureID` = 38218 AND `GroupID` IN (12, 13)) OR (`CreatureID` = 38469 AND `GroupID` = 2) OR (`CreatureID` = 38474 AND `GroupID` IN (1, 2)) OR (`CreatureID` = 43566 AND `GroupID` = 7);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(37953, 1, 0, 'Such easy prey.  Sylvanas will be most pleased!', 12, 0, 100, 0, 0, 0, 0, 0, 'Dark Scout - ambush taunt'),
(37953, 2, 0, 'Use Belysra''s Talisman to escape before the Dark Scout shoots you!$B$B|TInterface\\Icons\\inv_jewelry_talisman_10.blp:64|t', 42, 0, 100, 1, 0, 0, 0, 0, 'Dark Scout - talisman hint (raid boss whisper)'),
(38218, 12, 0, 'Gilneas will prevail!', 12, 0, 100, 0, 0, 0, 0, 0, 'Prince Liam Greymane - march bark'),
(38218, 13, 0, 'Your time is up, Forsaken scum!', 12, 0, 100, 0, 0, 0, 0, 0, 'Prince Liam Greymane - final push'),
(38469, 2, 0, 'Such a waste.  That arrow''s poison was not meant to be wasted on your whelp.  We''ll meet again, Genn!', 12, 0, 100, 0, 0, 0, 0, 0, 'Lady Sylvanas Windrunner - Liam death'),
(38474, 1, 0, 'We did it, father...', 12, 0, 100, 0, 0, 0, 0, 0, 'Prince Liam Greymane - dying 1'),
(38474, 2, 0, 'We took back our city... we took back...', 12, 0, 100, 0, 0, 0, 0, 0, 'Prince Liam Greymane - dying 2'),
(43566, 7, 0, 'Attack!', 14, 0, 100, 0, 0, 0, 0, 0, 'Lorna Crowley - gunship boarding');
