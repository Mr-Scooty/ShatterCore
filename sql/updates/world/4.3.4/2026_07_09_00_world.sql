-- Ultraxion (Dragon Soul) - full encounter: LFR / 10N / 25N / 10H / 25H
-- Includes the Twilight Assaulter gauntlet pre-event, the Wyrmrest Summit
-- teleporter, creature texts, spell script bindings, retail health values and
-- the per-difficulty loot split.

-- ---------------------------------------------------------------------------
-- Script bindings (base entries only - the core ignores ScriptName on
-- difficulty child entries)
-- ---------------------------------------------------------------------------
UPDATE `creature_template` SET `ScriptName` = 'boss_ultraxion' WHERE `entry` = 55294;
UPDATE `creature_template` SET `ScriptName` = 'npc_ultraxion_gauntlet' WHERE `entry` = 56305;
UPDATE `creature_template` SET `ScriptName` = 'npc_ultraxion_twilight_assaulter' WHERE `entry` IN (56249, 56250, 56251, 56252, 57795);

-- ---------------------------------------------------------------------------
-- Health (user-supplied retail values; base HP 85,892 at level 88 exp 3)
--   10N  55294: 660    = 56,688,720 (already correct)
--   25N  56576: 2145   = 184,238,340 (already correct)
--   10H  56577: 693    = 59,523,156
--   25H  56578: 2251.9 = 193,420,194
-- ---------------------------------------------------------------------------
UPDATE `creature_template` SET `HealthModifier` = 693 WHERE `entry` = 56577;
UPDATE `creature_template` SET `HealthModifier` = 2251.9 WHERE `entry` = 56578;

-- Raid Finder stats template (70% of 25N, user-approved):
--   58245 Ultraxion: 85,892 (L88) * 1501.5 = 128,966,838
DELETE FROM `creature_template` WHERE `entry` = 58245;
INSERT INTO `creature_template` (`entry`, `name`, `femaleName`, `subname`, `minlevel`, `maxlevel`, `exp`, `faction`, `unit_class`, `type`, `HealthModifier`, `AIName`, `ScriptName`, `VerifiedBuild`) VALUES
(58245, 'Ultraxion', '', 'LFR Stats', 88, 88, 3, 14, 1, 2, 1501.5, '', '', 0);

DELETE FROM `creature_template_model` WHERE `CreatureID` = 58245;
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `Probability`, `VerifiedBuild`) VALUES
(58245, 0, 39099, 1, 0);

-- ---------------------------------------------------------------------------
-- Spawns:
--   57379 Travel to Wyrmrest Summit at the temple base (visibility gated on
--         Hagara's death by the instance script; spellclick 109835 exists)
--   56305 Ultraxion Gauntlet controller, rooted high above the platform
--         center (position from the retail sniff)
-- ---------------------------------------------------------------------------
DELETE FROM `creature` WHERE `guid` IN (9000644, 9000645);
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `VerifiedBuild`) VALUES
(9000644, 57379, 967, 0, 0, 15, 0, 0, 0, 0, -1778.40, -2392.00, 45.6201, 3.05, 300, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(9000645, 56305, 967, 0, 0, 15, 0, 0, 0, 0, -1786.9132, -2393.5486, 380.35043, 0, 604800, 0, 0, 0, 0, 0, 0, 0, 0, 0);

-- The aspects, the Dragon Soul, Deathwing's perch and the summit return
-- teleporter stay visible from inside the Twilight Realm (phase 16)
UPDATE `creature` SET `phaseUseFlags` = 1 WHERE `guid` IN (370741, 370744, 370746, 370747, 370748, 370749, 370606, 370699);

-- ---------------------------------------------------------------------------
-- Creature texts (broadcast_text IDs and VO sounds are retail rows)
-- ---------------------------------------------------------------------------
DELETE FROM `creature_text` WHERE `CreatureID` IN (55294, 55971, 56630, 56664, 56665, 56666, 56667);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
-- Ultraxion
(55294, 0, 0, 'I am the beginning of the end...the shadow which blots out the sun...the bell which tolls your doom...', 14, 0, 100, 0, 0, 26317, 55320, 0, 'Ultraxion - Intro'),
(55294, 1, 0, 'For this moment ALONE was I made. Look upon your death, mortals, and despair!', 14, 0, 100, 0, 0, 26318, 55323, 0, 'Ultraxion - Aggro'),
(55294, 2, 0, 'A monstrous force pulls you into the twilight realm!', 41, 0, 100, 0, 0, 0, 55410, 0, 'Ultraxion - Twilight Shift Emote'),
(55294, 3, 0, 'Now is the hour of twilight!', 14, 0, 100, 0, 0, 26314, 55329, 0, 'Ultraxion - Hour of Twilight'),
(55294, 4, 0, 'The final shred of light fades, and with it, your pitiful mortal existence!', 14, 0, 100, 0, 0, 26323, 55334, 0, 'Ultraxion - Fading Light'),
(55294, 5, 0, 'Lord Deathwing, your gift...it is too much! I am...rrrraaaaagghhh!!', 14, 0, 100, 0, 0, 26324, 55335, 0, 'Ultraxion - Unstable Monstrosity 1'),
(55294, 5, 1, 'Through the pain and fire my hatred burns!', 14, 0, 100, 0, 0, 26325, 55336, 0, 'Ultraxion - Unstable Monstrosity 2'),
(55294, 6, 0, 'Fall before Ultraxion!', 14, 0, 100, 0, 0, 26319, 55331, 0, 'Ultraxion - Slay 1'),
(55294, 6, 1, 'You have no hope!', 14, 0, 100, 0, 0, 26320, 55332, 0, 'Ultraxion - Slay 2'),
(55294, 6, 2, 'Hahahahaha!', 14, 0, 100, 0, 0, 26321, 55333, 0, 'Ultraxion - Slay 3'),
(55294, 7, 0, 'But...but...I am...Ul...trax...ionnnnnn...', 14, 0, 100, 0, 0, 26316, 55330, 0, 'Ultraxion - Death'),
(55294, 8, 0, 'I WILL DRAG YOU WITH ME INTO FLAME AND DARKNESS!', 14, 0, 100, 0, 0, 26315, 55337, 0, 'Ultraxion - Twilight Eruption'),
(55294, 9, 0, 'The shield surrounding the Aspects is destroyed by Ultraxion''s attack!', 41, 0, 100, 0, 0, 0, 55406, 0, 'Ultraxion - Unsoaked Hour of Twilight'),
(55294, 10, 0, 'The shields surrounding the Aspects shatter! The Aspects are vulnerable!', 41, 0, 100, 0, 0, 0, 55395, 0, 'Ultraxion - Aspects Vulnerable'),
(55294, 11, 0, '|TInterface\\Icons\\spell_fire_twilightimmolation.blp:20|t Ultraxion becomes |cFF9900CC|Hspell:106373|h[More Unstable]|h|r!', 41, 0, 100, 0, 0, 0, 55343, 0, 'Ultraxion - More Unstable Emote'),
(55294, 12, 0, '|TInterface\\Icons\\spell_fire_twilightcano.blp:20|t Ultraxion''s instability is causing a massive |cFF9900CC|Hspell:106388|h[Twilight Eruption]|h|r!', 41, 0, 100, 0, 0, 0, 55344, 0, 'Ultraxion - Twilight Eruption Emote'),
(55294, 13, 0, '|TInterface\\Icons\\spell_fire_twilightimmolation.blp:20|t Ultraxion reaches |cFF9900CC|Hspell:106395|h[Maximum Instability]|h|r!', 41, 0, 100, 0, 0, 0, 55345, 0, 'Ultraxion - Maximum Instability Emote'),
-- Deathwing (gauntlet RP)
(55971, 0, 0, 'It is good to see you again, Alexstrasza. I have been busy in my absence.', 14, 0, 100, 0, 0, 26360, 56080, 0, 'Deathwing - Ultraxion Gauntlet 1'),
(55971, 1, 0, 'Twisting your pitiful whelps into mindless abominations, bent only to my will. It was a very... painful process.', 14, 0, 100, 0, 0, 26361, 56646, 0, 'Deathwing - Ultraxion Gauntlet 2'),
(55971, 2, 0, 'Mere whelps, experiments, a means to a greater end. You will see what the research of my clutch has yielded.', 14, 0, 100, 0, 0, 26362, 55322, 0, 'Deathwing - Ultraxion Gauntlet End'),
(55971, 3, 0, 'Nefarian, Onyxia, Sinestra... they were nothing. Now you face my ultimate creation.', 14, 0, 100, 0, 0, 26363, 56647, 0, 'Deathwing - Ultimate Creation'),
(55971, 4, 0, 'The Hour of Twilight is nigh; the sun sets on your pitiful mortal existence.', 14, 0, 100, 0, 0, 26364, 56648, 0, 'Deathwing - Hour of Twilight'),
-- Alexstrasza the Life-Binder
(56630, 0, 0, 'Take heart, heroes, life will always blossom from the darkest soil!', 14, 0, 100, 0, 0, 26506, 55325, 0, 'Alexstrasza - Gift of Life'),
(56630, 1, 0, '|TInterface\\Icons\\inv_misc_head_dragon_01.blp:20|t Alexstrasza summons forth the |cFFFF0000|Hspell:105896|h[Gift of Life]|h|r!', 41, 0, 100, 0, 0, 0, 55338, 0, 'Alexstrasza - Gift of Life Emote'),
(56630, 2, 0, 'They... are my clutch no longer.  Bring them down.', 14, 0, 100, 0, 0, 26505, 56081, 0, 'Alexstrasza - Gauntlet Bring Them Down'),
(56630, 3, 0, 'They have failed us sister.', 14, 0, 100, 0, 0, 0, 56099, 0, 'Alexstrasza - Wipe'),
-- Ysera the Awakened
(56665, 0, 0, 'In dreams, we may overcome any obstacle!', 14, 0, 100, 0, 0, 26149, 55326, 0, 'Ysera - Essence of Dreams'),
(56665, 1, 0, '|TInterface\\Icons\\inv_misc_head_dragon_green.blp:20|t Ysera summons forth the |cFF00CC00|Hspell:105900|h[Essence of Dreams]|h|r!', 41, 0, 100, 0, 0, 0, 55339, 0, 'Ysera - Essence of Dreams Emote'),
(56665, 2, 0, 'Heroes, we must place this burden on your shoulders once again.  You must protect us from Deathwing''s forces while we imbue the Soul with the powers of the great Aspects of Azeroth.', 14, 0, 100, 0, 0, 0, 56077, 0, 'Ysera - Gauntlet Start'),
(56665, 3, 0, 'I sense a great disturbance in the balance approaching. The chaos of it burns my mind!', 14, 0, 100, 0, 0, 26148, 55321, 0, 'Ysera - Ultraxion Approaches'),
(56665, 4, 0, 'I have awakened only to sleep once again.', 14, 0, 100, 0, 0, 0, 56100, 0, 'Ysera - Wipe'),
-- Kalecgos
(56664, 0, 0, 'Winds of the arcane be at their backs, and refresh them in this hour of darkness!', 14, 0, 100, 0, 0, 26267, 55327, 0, 'Kalecgos - Source of Magic'),
(56664, 1, 0, '|TInterface\\Icons\\inv_misc_head_dragon_blue.blp:20|t Kalecgos summons forth the |cFF0000FF|Hspell:105903|h[Source of Magic]|h|r!', 41, 0, 100, 0, 0, 0, 55340, 0, 'Kalecgos - Source of Magic Emote'),
-- Nozdormu the Timeless One
(56666, 0, 0, 'The cycle of time brings an end to all things.', 14, 0, 100, 0, 0, 25954, 55328, 0, 'Nozdormu - Timeloop'),
(56666, 1, 0, '|TInterface\\Icons\\inv_misc_head_dragon_bronze.blp:20|t Nozdormu imbues you with a |cFFFFFF00|Hspell:105984|h[Timeloop]|h|r!', 41, 0, 100, 0, 0, 0, 55341, 0, 'Nozdormu - Timeloop Emote'),
-- Thrall
(56667, 0, 0, 'Strength of the Earth, hear my call! Shield them in this dark hour, the last defenders of Azeroth!', 14, 0, 100, 0, 0, 25907, 55324, 0, 'Thrall - Last Defender of Azeroth'),
(56667, 1, 0, '|TInterface\\Icons\\inv_misc_head_dragon_black.blp:20|t Thrall imbues the tanks with the strength of the earth! You are the |cFF9900CC|Hspell:106218|h[Last Defender of Azeroth]|h|r!', 41, 0, 100, 0, 0, 0, 55342, 0, 'Thrall - Last Defender Emote');

-- ---------------------------------------------------------------------------
-- Spell script bindings (base spell + every SpellDifficulty fork)
-- ---------------------------------------------------------------------------
DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
'spell_ultraxion_twilight_shift_aoe', 'spell_ultraxion_heroic_will',
'spell_ultraxion_hour_of_twilight', 'spell_ultraxion_hour_of_twilight_damage',
'spell_ultraxion_looming_darkness_missile', 'spell_ultraxion_fading_light',
'spell_ultraxion_fading_light_tank', 'spell_ultraxion_fading_light_raid',
'spell_ultraxion_unstable_monstrosity', 'spell_ultraxion_twilight_instability',
'spell_ultraxion_twilight_burst', 'spell_ultraxion_last_defender',
'spell_ultraxion_timeloop_targets', 'spell_ultraxion_timeloop', 'spell_ultraxion_gift_of_life',
'spell_ultraxion_essence_of_dreams', 'spell_ultraxion_source_of_magic',
'spell_ultraxion_essence_of_dreams_mirror');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(106369, 'spell_ultraxion_twilight_shift_aoe'),
(105554, 'spell_ultraxion_twilight_shift_aoe'),
(106108, 'spell_ultraxion_heroic_will'),
(106371, 'spell_ultraxion_hour_of_twilight'),
(109415, 'spell_ultraxion_hour_of_twilight'),
(109416, 'spell_ultraxion_hour_of_twilight'),
(109417, 'spell_ultraxion_hour_of_twilight'),
(103327, 'spell_ultraxion_hour_of_twilight_damage'),
(109231, 'spell_ultraxion_looming_darkness_missile'),
(105925, 'spell_ultraxion_fading_light'),
(110070, 'spell_ultraxion_fading_light'),
(110069, 'spell_ultraxion_fading_light'),
(110068, 'spell_ultraxion_fading_light'),
(109075, 'spell_ultraxion_fading_light'),
(110080, 'spell_ultraxion_fading_light'),
(110079, 'spell_ultraxion_fading_light'),
(110078, 'spell_ultraxion_fading_light'),
(105925, 'spell_ultraxion_fading_light_tank'),
(110070, 'spell_ultraxion_fading_light_tank'),
(110069, 'spell_ultraxion_fading_light_tank'),
(110068, 'spell_ultraxion_fading_light_tank'),
(109075, 'spell_ultraxion_fading_light_raid'),
(110080, 'spell_ultraxion_fading_light_raid'),
(110079, 'spell_ultraxion_fading_light_raid'),
(110078, 'spell_ultraxion_fading_light_raid'),
(106372, 'spell_ultraxion_unstable_monstrosity'),
(106376, 'spell_ultraxion_unstable_monstrosity'),
(106377, 'spell_ultraxion_unstable_monstrosity'),
(106378, 'spell_ultraxion_unstable_monstrosity'),
(106379, 'spell_ultraxion_unstable_monstrosity'),
(106380, 'spell_ultraxion_unstable_monstrosity'),
(106375, 'spell_ultraxion_twilight_instability'),
(109182, 'spell_ultraxion_twilight_instability'),
(109183, 'spell_ultraxion_twilight_instability'),
(109184, 'spell_ultraxion_twilight_instability'),
(106415, 'spell_ultraxion_twilight_burst'),
(106218, 'spell_ultraxion_last_defender'),
(105984, 'spell_ultraxion_timeloop_targets'),
(105896, 'spell_ultraxion_gift_of_life'),
(105900, 'spell_ultraxion_essence_of_dreams'),
(105903, 'spell_ultraxion_source_of_magic'),
(105900, 'spell_ultraxion_essence_of_dreams_mirror');

-- ---------------------------------------------------------------------------
-- Minutes to Midnight (achievement 6084, criteria 18391): routed through the
-- instance script (no raid member hit by Hour of Twilight more than once)
-- ---------------------------------------------------------------------------
DELETE FROM `achievement_criteria_data` WHERE `criteria_id` = 18391 AND `type` = 18;
INSERT INTO `achievement_criteria_data` (`criteria_id`, `type`, `value1`, `value2`, `ScriptName`) VALUES
(18391, 18, 0, 0, '');

-- ---------------------------------------------------------------------------
-- Loot: split the TDB rows for 55294 by item level (Item-sparse.db2)
--   397 Normal (21 + tokens), 410 Heroic, 384 LFR, shared mats/gem
--   10N keeps the base table; 25N carries Normal (LootMode 1) + LFR
--   (LootMode 2) + shared (LootMode 3); 10H/25H get Heroic + shared
-- ---------------------------------------------------------------------------
SET @NORMAL_ITEMS := '77205,77207,77208,77209,77210,77211,77223,77228,77229,77230,77231,77232,77242,77243,77244,77245,77246,77247,78013,78174,78179,78184';
SET @LFR_ITEMS    := '77972,77979,77980,77981,77982,77983,78438,78439,78440,78441,78442,78443,78444,78494,78495,78496,78497,78498,78862,78863,78864,78865,78866,78867,78868,78869,78870,78871,78872,78873,78874,78875,78876';
SET @HEROIC_ITEMS := '77992,77999,78000,78001,78002,78003,78429,78430,78431,78432,78433,78434,78435,78436,78489,78490,78491,78492,78493,78847,78848,78849';
SET @SHARED_ITEMS := '71998,77952';

-- Original TDB rows inlined so the split below is idempotent
DELETE FROM `creature_loot_template` WHERE `Entry` = 55294;
INSERT INTO `creature_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`) VALUES
(55294,71998,0,22.1757,0,0,1,0,1,2,NULL),
(55294,77205,0,5.9972,0,0,1,0,1,1,NULL),
(55294,77207,0,1.2552,0,0,1,0,1,1,NULL),
(55294,77208,0,0.4184,0,0,1,0,1,1,NULL),
(55294,77209,0,0.6974,0,0,1,0,1,1,NULL),
(55294,77210,0,0.5579,0,0,1,0,1,1,NULL),
(55294,77211,0,0.5579,0,0,1,0,1,1,NULL),
(55294,77223,0,6.1367,0,0,1,0,1,1,NULL),
(55294,77228,0,0.6974,0,0,1,0,1,1,NULL),
(55294,77229,0,0.1395,0,0,1,0,1,1,NULL),
(55294,77230,0,0.9763,0,0,1,0,1,1,NULL),
(55294,77231,0,0.1395,0,0,1,0,1,1,NULL),
(55294,77232,0,0.6974,0,0,1,0,1,1,NULL),
(55294,77242,0,5.7183,0,0,1,0,1,1,NULL),
(55294,77243,0,9.484,0,0,1,0,1,1,NULL),
(55294,77244,0,5.5788,0,0,1,0,1,1,NULL),
(55294,77245,0,9.0656,0,0,1,0,1,1,NULL),
(55294,77246,0,4.463,0,0,1,0,1,1,NULL),
(55294,77247,0,5.0209,0,0,1,0,1,1,NULL),
(55294,77952,0,24.9651,0,0,1,0,1,2,NULL),
(55294,77972,0,12.9707,0,0,1,0,1,1,NULL),
(55294,77979,0,0.8368,0,0,1,0,1,1,NULL),
(55294,77980,0,1.1158,0,0,1,0,1,1,NULL),
(55294,77981,0,0.6974,0,0,1,0,1,1,NULL),
(55294,77982,0,0.2789,0,0,1,0,1,1,NULL),
(55294,77983,0,1.1158,0,0,1,0,1,1,NULL),
(55294,77992,0,0.1395,0,0,1,0,1,1,NULL),
(55294,77999,0,0.1395,0,0,1,0,1,1,NULL),
(55294,78000,0,0.1395,0,0,1,0,1,1,NULL),
(55294,78001,0,0.1395,0,0,1,0,1,1,NULL),
(55294,78002,0,0.1395,0,0,1,0,1,1,NULL),
(55294,78003,0,0.1395,0,0,1,0,1,1,NULL),
(55294,78013,0,6.5551,0,0,1,0,1,1,NULL),
(55294,78174,0,20.9205,0,0,1,0,1,1,NULL),
(55294,78179,0,17.8522,0,0,1,0,1,1,NULL),
(55294,78184,0,20.9205,0,0,1,0,1,1,NULL),
(55294,78429,0,0.1395,0,0,1,0,1,1,NULL),
(55294,78430,0,0.1395,0,0,1,0,1,1,NULL),
(55294,78431,0,0.1395,0,0,1,0,1,1,NULL),
(55294,78432,0,0.1395,0,0,1,0,1,1,NULL),
(55294,78433,0,0.1395,0,0,1,0,1,1,NULL),
(55294,78434,0,0.1395,0,0,1,0,1,1,NULL),
(55294,78435,0,0.1395,0,0,1,0,1,1,NULL),
(55294,78436,0,0.1395,0,0,1,0,1,1,NULL),
(55294,78438,0,4.6025,0,0,1,0,1,1,NULL),
(55294,78439,0,5.9972,0,0,1,0,1,1,NULL),
(55294,78440,0,11.2971,0,0,1,0,1,1,NULL),
(55294,78441,0,8.6471,0,0,1,0,1,1,NULL),
(55294,78442,0,10.0418,0,0,1,0,1,1,NULL),
(55294,78443,0,9.9024,0,0,1,0,1,1,NULL),
(55294,78444,0,9.7629,0,0,1,0,1,1,NULL),
(55294,78489,0,0.1395,0,0,1,0,1,1,NULL),
(55294,78490,0,0.1395,0,0,1,0,1,1,NULL),
(55294,78491,0,0.1395,0,0,1,0,1,1,NULL),
(55294,78492,0,0.1395,0,0,1,0,1,1,NULL),
(55294,78493,0,0.1395,0,0,1,0,1,1,NULL),
(55294,78494,0,0.9763,0,0,1,0,1,1,NULL),
(55294,78495,0,0.4184,0,0,1,0,1,1,NULL),
(55294,78496,0,1.1158,0,0,1,0,1,1,NULL),
(55294,78497,0,0.1395,0,0,1,0,1,1,NULL),
(55294,78498,0,0.6974,0,0,1,0,1,1,NULL),
(55294,78847,0,0.1395,0,0,1,0,1,1,NULL),
(55294,78848,0,0.1395,0,0,1,0,1,1,NULL),
(55294,78849,0,0.1395,0,0,1,0,1,1,NULL),
(55294,78862,0,26.0809,0,0,1,0,1,1,NULL),
(55294,78863,0,25.6625,0,0,1,0,1,1,NULL),
(55294,78864,0,21.8968,0,0,1,0,1,1,NULL),
(55294,78865,0,1.9526,0,0,1,0,1,1,NULL),
(55294,78866,0,0.1395,0,0,1,0,1,1,NULL),
(55294,78867,0,0.1395,0,0,1,0,1,1,NULL),
(55294,78868,0,0.6974,0,0,1,0,1,1,NULL),
(55294,78869,0,0.1395,0,0,1,0,1,1,NULL),
(55294,78870,0,0.1395,0,0,1,0,1,1,NULL),
(55294,78871,0,0.1395,0,0,1,0,1,1,NULL),
(55294,78872,0,0.6974,0,0,1,0,1,1,NULL),
(55294,78873,0,0.1395,0,0,1,0,1,1,NULL),
(55294,78874,0,1.2552,0,0,1,0,1,1,NULL),
(55294,78875,0,0.4184,0,0,1,0,1,1,NULL),
(55294,78876,0,0.5579,0,0,1,0,1,1,NULL);

DELETE FROM `creature_loot_template` WHERE `Entry` IN (56576, 56577, 56578);

-- 25 Normal: 397 loot
INSERT INTO `creature_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT 56576, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, 1, `GroupId`, `MinCount`, `MaxCount`, 'Ultraxion 25N'
FROM `creature_loot_template` WHERE `Entry` = 55294 AND FIND_IN_SET(`Item`, @NORMAL_ITEMS);

-- 25 Normal table: LFR loot as LootMode 2
INSERT INTO `creature_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT 56576, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, 2, `GroupId`, `MinCount`, `MaxCount`, 'Ultraxion LFR'
FROM `creature_loot_template` WHERE `Entry` = 55294 AND FIND_IN_SET(`Item`, @LFR_ITEMS);

-- 25 Normal table: shared drops available in both loot modes
INSERT INTO `creature_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT 56576, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, 3, `GroupId`, `MinCount`, `MaxCount`, 'Ultraxion shared'
FROM `creature_loot_template` WHERE `Entry` = 55294 AND FIND_IN_SET(`Item`, @SHARED_ITEMS);

-- Heroic tables
INSERT INTO `creature_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT 56577, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, 1, `GroupId`, `MinCount`, `MaxCount`, 'Ultraxion 10H'
FROM `creature_loot_template` WHERE `Entry` = 55294 AND (FIND_IN_SET(`Item`, @HEROIC_ITEMS) OR FIND_IN_SET(`Item`, @SHARED_ITEMS));

INSERT INTO `creature_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT 56578, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, 1, `GroupId`, `MinCount`, `MaxCount`, 'Ultraxion 25H'
FROM `creature_loot_template` WHERE `Entry` = 55294 AND (FIND_IN_SET(`Item`, @HEROIC_ITEMS) OR FIND_IN_SET(`Item`, @SHARED_ITEMS));

-- 10 Normal keeps only 397 + shared
DELETE FROM `creature_loot_template` WHERE `Entry` = 55294 AND (FIND_IN_SET(`Item`, @LFR_ITEMS) OR FIND_IN_SET(`Item`, @HEROIC_ITEMS));

UPDATE `creature_template` SET `lootid` = `entry` WHERE `entry` IN (56576, 56577, 56578);
