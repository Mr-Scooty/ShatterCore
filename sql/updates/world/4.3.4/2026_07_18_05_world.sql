-- Kelpthar: Gnaws' Boneyard / Smuggler's Scar hub (25587/25598/25388/25390/25389/25602/25377/25459/25358).
-- Retail sniff crosscheck. Phases: 167 shivering Mack, 168 warmed Mack + bonfire, 141 jewelry GOs.

-- ==================== 1) Quest chain fixes ====================
-- 25384 "Raw Materials" was cut on retail (no starter, never sniffed) but still gates 25602.
UPDATE `quest_template_addon` SET `PrevQuestID`=25598 WHERE `ID` IN (25602,25389,25390);
UPDATE `quest_template_addon` SET `PrevQuestID`=25602 WHERE `ID` IN (25459,25358);
UPDATE `quest_template` SET `RewardNextQuest`=0 WHERE `ID`=25384;
UPDATE `quest_template` SET `Flags`=`Flags`|0x400000 WHERE `ID` IN (25598,25602,25390);

-- ==================== 2) Phasing (Smuggler's Scar area 5057) ====================
DELETE FROM `phase_area` WHERE `AreaId`=5057 AND `PhaseId` IN (167,168,141);
INSERT INTO `phase_area` (`AreaId`, `PhaseId`, `Comment`) VALUES
(5057, 167, 'Smugglers Scar: shivering Mack (post-rescue, pre-bonfire)'),
(5057, 168, 'Smugglers Scar: warmed Mack + lit bonfire'),
(5057, 141, 'Smugglers Scar: Adarrah jewelry gifts');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=26 AND `SourceGroup` IN (167,168,141) AND `SourceEntry`=5057;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(26, 167, 5057, 0, 0, 47, 0, 25598, 66, 0, 0, 0, 0, '', 'Shivering Mack: Aint Too Proud to Beg complete'),
(26, 167, 5057, 0, 0, 8, 0, 25602, 0, 0, 1, 0, 0, '', 'Shivering Mack: bonfire quest not rewarded'),
(26, 168, 5057, 0, 0, 8, 0, 25602, 0, 0, 0, 0, 0, '', 'Warmed Mack: Cant Start a Fire rewarded'),
(26, 141, 5057, 0, 0, 8, 0, 25390, 0, 0, 0, 0, 0, '', 'Jewelry gifts: A Girls Best Friend rewarded');

-- Existing spawns move into their states
UPDATE `creature` SET `PhaseId`=168 WHERE `guid`=349424;  -- 40983 warmed Mack
UPDATE `gameobject` SET `PhaseId`=168 WHERE `guid`=221962; -- 203114 Lit Bonfire

-- ==================== 3) Spawns ====================
DELETE FROM `creature` WHERE `guid` BETWEEN 9001076 AND 9001077;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild`) VALUES
(9001076, 39884, 0, 4815, 5057, 1, 0, 1, 169, 0, -1, 0, 0, -4558.870, 3464.310, -101.460, 0.60, 300, 5, 0, 1, 0, 1, 0, 0, 0, '', 0),
(9001077, 39885, 0, 4815, 5057, 1, 0, 1, 167, 0, -1, 0, 0, -4566.040, 3464.830, -101.570, 5.864, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 0);

DELETE FROM `gameobject` WHERE `guid` BETWEEN 9001029 AND 9001031;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `VerifiedBuild`) VALUES
(9001029, 202770, 0, 4815, 5057, 1, 0, 1, 141, 0, -1, -4555.560, 3471.490, -101.260, 0, 0, 0, 0, 1, 300, 255, 1, '', 0),
(9001030, 202771, 0, 4815, 5057, 1, 0, 1, 141, 0, -1, -4555.550, 3471.170, -101.190, 0, 0, 0, 0, 1, 300, 255, 1, '', 0),
(9001031, 187235, 0, 4815, 5057, 1, 0, 1, 169, 0, -1, -4561.230, 3456.320, -101.760, 1.902, 0, 0, 0.81344, 0.58165, 300, 255, 1, '', 0);

-- Shivering Mack cowers; personal escort Adarrah is invisible (generic quest invis 7, revealed by 76318)
DELETE FROM `creature_template_addon` WHERE `entry` IN (39885,39868);
INSERT INTO `creature_template_addon` (`entry`, `waypointPathId`, `cyclicSplinePathId`, `mount`, `StandState`, `AnimTier`, `VisFlags`, `SheathState`, `PvPFlags`, `emote`, `aiAnimKit`, `movementAnimKit`, `meleeAnimKit`, `visibilityDistanceType`, `auras`) VALUES
(39885, 0, 0, 0, 0, 0, 0, 1, 0, 431, 0, 0, 0, 0, ''),
(39868, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, '49414');

-- ==================== 4) SAI ====================
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry` IN (46316,39868,39669,39884);
UPDATE `creature` SET `ScriptName`='' WHERE `guid`=342346;
-- guid-scoped SAI for the cave-exit signal bunny (entry 32520 is a generic bunny used everywhere)
DELETE FROM `smart_scripts` WHERE `entryorguid`=-342346 AND `source_type`=0;
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (46316,39868,39669,39884) AND `source_type`=0;
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (3986800,3988400) AND `source_type`=9;

INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
-- 46316 cave-entrance credit: KC no-ops for players without the quest; whisper gated by condition
(46316, 0, 0, 1, 10, 0, 100, 0, 1, 6, 30000, 30000, 11, 86325, 2, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Gimme Shelter KC - OOC LOS - credit qualifying player'),
(46316, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Gimme Shelter KC - Linked - whisper Leave Smugglers Scar'),
-- cave-exit signal bunny (guid 342346): stun + credit + summon personal Adarrah + detect
(-342346, 0, 0, 1, 10, 0, 100, 0, 1, 7, 60000, 60000, 11, 86327, 2, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Signal bunny - OOC LOS - Signal Adarrah stun'),
(-342346, 0, 1, 2, 61, 0, 100, 0, 0, 0, 0, 0, 85, 77365, 2, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Signal bunny - Linked - invoker casts KC 46312'),
(-342346, 0, 2, 3, 61, 0, 100, 0, 0, 0, 0, 0, 85, 74449, 2, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Signal bunny - Linked - invoker summons Adarrah escort'),
(-342346, 0, 3, 0, 61, 0, 100, 0, 0, 0, 0, 0, 85, 76318, 2, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Signal bunny - Linked - invoker casts See Adarrah'),
-- 39868 personal escort Adarrah
(39868, 0, 0, 0, 54, 0, 100, 0, 0, 0, 0, 0, 80, 3986800, 0, 2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Adarrah escort - Just Summoned - Run escort script'),
(3986800, 9, 0, 0, 0, 0, 100, 0, 1500, 1500, 0, 0, 5, 396, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Adarrah escort - emote talk'),
(3986800, 9, 1, 0, 0, 0, 100, 0, 700, 700, 0, 0, 1, 0, 0, 1, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Adarrah escort - Come inside, $r'),
(3986800, 9, 2, 0, 0, 0, 100, 0, 2000, 2000, 0, 0, 69, 1, 0, 0, 0, 0, 0, 8, 0, 0, 0, -4664.39, 3566.14, -118.04, 0, 'Adarrah escort - swim to cave 1'),
(3986800, 9, 3, 0, 0, 0, 100, 0, 2500, 2500, 0, 0, 69, 2, 0, 0, 0, 0, 0, 8, 0, 0, 0, -4638.29, 3535.42, -119.15, 0, 'Adarrah escort - swim to cave 2'),
(3986800, 9, 4, 0, 0, 0, 100, 0, 4000, 4000, 0, 0, 69, 3, 0, 0, 0, 0, 0, 8, 0, 0, 0, -4613.83, 3515.18, -106.40, 0, 'Adarrah escort - swim to cave 3'),
(3986800, 9, 5, 0, 0, 0, 100, 0, 3500, 3500, 0, 0, 41, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Adarrah escort - despawn'),
-- 39669 wreck Samir: gossip select -> rescue credit
(39669, 0, 0, 0, 62, 0, 100, 0, 11444, 0, 0, 0, 33, 39669, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Captain Samir - Gossip select - Kill Credit'),
(39669, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 72, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Captain Samir - Linked - Close Gossip'),
-- 39884 Scar Samir: bonfire celebration on 25602 reward
(39884, 0, 0, 0, 20, 0, 100, 0, 25602, 0, 0, 0, 80, 3988400, 0, 2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Captain Samir - On 25602 rewarded - bonfire script'),
(3988400, 9, 0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Samir bonfire - Woohooo!'),
(3988400, 9, 1, 0, 0, 0, 100, 0, 300, 300, 0, 0, 11, 76505, 0, 0, 0, 0, 0, 19, 28960, 30, 0, 0, 0, 0, 0, 'Samir bonfire - Throw Keg at bonfire bunny'),
(3988400, 9, 2, 0, 0, 0, 100, 0, 1700, 1700, 0, 0, 85, 76503, 2, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Samir bonfire - invoker sees lit bonfire');

-- Fix the linked gossip-select row id gap (39669 row 1 link)
UPDATE `smart_scripts` SET `link`=1 WHERE `entryorguid`=39669 AND `source_type`=0 AND `id`=0;

-- Whisper/summon gating conditions (smart_event source 22: SourceGroup = id+1, SourceEntry = entryorguid)
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=22 AND `SourceEntry` IN (46316,-342346);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(22, 1, 46316, 0, 0, 9, 0, 25587, 0, 0, 0, 0, 0, '', 'Gimme Shelter KC OOC LOS: only players on 25587'),
(22, 1, -342346, 0, 0, 9, 0, 25587, 0, 0, 0, 0, 0, '', 'Signal bunny OOC LOS: only players on 25587'),
-- Gossip option on the wreck Samir only during 25598
(15, 11444, 0, 0, 0, 9, 0, 25598, 0, 0, 0, 0, 0, '', 'Samir rescue gossip option: only on Aint Too Proud to Beg');
