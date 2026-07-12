-- End Time (map 938): complete dungeon implementation
-- Echo of Baine, Echo of Sylvanas, Echo of Tyrande (+ Shadow Gauntlet), teleporter,
-- random echo selection support, encounter credits, achievements and wing trash AI.

-- 1. Creature script bindings
UPDATE `creature_template` SET `ScriptName`='boss_echo_of_baine' WHERE `entry`=54431;
UPDATE `creature_template` SET `ScriptName`='boss_echo_of_sylvanas' WHERE `entry`=54123;
UPDATE `creature_template` SET `ScriptName`='boss_echo_of_tyrande' WHERE `entry`=54544;
UPDATE `creature_template` SET `ScriptName`='npc_baines_totem' WHERE `entry`=54434;
UPDATE `creature_template` SET `ScriptName`='npc_echo_of_sylvanas_ghoul_anchor' WHERE `entry`=54197;
UPDATE `creature_template` SET `ScriptName`='npc_echo_of_sylvanas_risen_ghoul' WHERE `entry`=54191;
UPDATE `creature_template` SET `ScriptName`='npc_echo_of_sylvanas_blighted_arrows' WHERE `entry`=54403;
UPDATE `creature_template` SET `ScriptName`='npc_echo_of_sylvanas_brittle_ghoul' WHERE `entry`=54952;
UPDATE `creature_template` SET `ScriptName`='npc_echo_of_tyrande_pool_of_moonlight' WHERE `entry`=54508;
UPDATE `creature_template` SET `ScriptName`='npc_echo_of_tyrande_gauntlet_add', `AIName`='' WHERE `entry` IN (54512,54688,54699,54700,54701);
UPDATE `creature_template` SET `ScriptName`='npc_echo_of_tyrande_moonlance' WHERE `entry` IN (54574,54580,54581,54582);
UPDATE `creature_template` SET `ScriptName`='npc_echo_of_tyrande_eye_of_elune' WHERE `entry` IN (54941,54942);

-- 2. Gameobject script bindings (one Time Transit Device entry per shrine)
UPDATE `gameobject_template` SET `ScriptName`='go_end_time_time_transit_device' WHERE `entry` IN (209437,209438,209439,209440,209441,209442,209443);

-- 3. Spell script bindings
DELETE FROM `spell_script_names` WHERE `ScriptName` IN
('spell_echo_of_baine_pulverize','spell_echo_of_baine_pulverize_platform','spell_echo_of_sylvanas_summon_ghoul_ring','spell_echo_of_sylvanas_death_grip',
 'spell_echo_of_tyrande_achievement_tracker','spell_echo_of_tyrande_tears_of_elune');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(101625, 'spell_echo_of_baine_pulverize'),
(101815, 'spell_echo_of_baine_pulverize_platform'),
(101198, 'spell_echo_of_sylvanas_summon_ghoul_ring'),
(101397, 'spell_echo_of_sylvanas_death_grip'),
(102491, 'spell_echo_of_tyrande_achievement_tracker'),
(102242, 'spell_echo_of_tyrande_tears_of_elune');

-- 4. Serverside kill credit spells for the generic 'First Echo' / 'Second Echo' encounters
DELETE FROM `spell_dbc` WHERE `Id` IN (110163,110164);
INSERT INTO `spell_dbc` (`Id`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `AttributesEx4`, `AttributesEx5`, `AttributesEx6`, `AttributesEx7`, `AttributesEx8`, `AttributesEx9`, `AttributesEx10`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `SchoolMask`, `SpellAuraOptionsId`, `SpellCastingRequirementsId`, `SpellCategoriesId`, `SpellClassOptionsId`, `SpellEquippedItemsId`, `SpellInterruptsId`, `SpellLevelsId`, `SpellTargetRestrictionsId`, `SpellName`) VALUES
(110163, 2843738368, 268436512, 540677, 269943552, 128, 393225, 5120, 33554432, 32, 0, 0, 0, 36, 13, 0, 38, 0, 0, 0, 0, 0, 0, 0, '(Serverside/Non-DB2) First Echo Kill Credit'),
(110164, 2843738368, 268436512, 540677, 269943552, 128, 393225, 5120, 33554432, 32, 0, 0, 0, 36, 13, 0, 38, 0, 0, 0, 0, 0, 0, 0, '(Serverside/Non-DB2) Second Echo Kill Credit');

DELETE FROM `spelleffect_dbc` WHERE `Id` IN (160119,160120);
INSERT INTO `spelleffect_dbc` (`Id`, `Effect`, `EffectAura`, `EffectRadiusMaxIndex`, `EffectImplicitTargetA`, `EffectImplicitTargetB`, `SpellID`, `EffectIndex`, `Comment`) VALUES
(160119, 6, 4, 28, 22, 7, 110163, 0, 'First Echo Kill Credit'),
(160120, 6, 4, 28, 22, 7, 110164, 0, 'Second Echo Kill Credit');

-- 5. Dungeon encounter wiring (DungeonEncounter.dbc: 1269 First Echo, 1268 Second Echo, 1271 Murozond; LFGDungeons 435)
DELETE FROM `instance_encounters` WHERE `entry` IN (1268,1269,1271);
INSERT INTO `instance_encounters` (`entry`, `creditType`, `creditEntry`, `lastEncounterDungeon`, `comment`) VALUES
(1269, 1, 110163, 0, 'End Time - First Echo'),
(1268, 1, 110164, 0, 'End Time - Second Echo'),
(1271, 1, 110158, 435, 'End Time - Murozond');

-- 6. Echo of Sylvanas spawn (missing entirely; position from sniff)
DELETE FROM `creature` WHERE `guid`=9000700;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild`) VALUES
(9000700, 54123, 938, 5789, 5790, 2, 0, 1, 169, 0, -1, 0, 1, 3845.506, 909.3177, 56.146, 1.309, 7200, 0, 0, 1, 0, 0, 0, 0, 0, '', 0);

-- 7. Echo of Jaina joins her own manual spawn group (the instance script spawns her after 16 staff fragments).
--    Group 460 belongs to Firelands (Shannox) in this database - the End Time script now uses 463.
DELETE FROM `spawn_group_template` WHERE `groupId`=463;
INSERT INTO `spawn_group_template` (`groupId`, `groupName`, `groupFlags`) VALUES
(463, 'End Time - Echo of Jaina', 4);
DELETE FROM `spawn_group` WHERE `groupId`=463;
INSERT INTO `spawn_group` (`groupId`, `spawnType`, `spawnId`) VALUES
(463, 0, 341769);

-- 8. Pools of Moonlight are summoned sequentially by the gauntlet script - remove the static spawns
DELETE FROM `creature` WHERE `id`=54508 AND `map`=938;

-- 9. Template tweaks
--    Ring ghouls walk at 1.5 y/s (sniffed); the ring anchor is a cosmetic ghost and must not be attackable
UPDATE `creature_template` SET `speed_walk`=0.6 WHERE `entry` IN (54191,54197);
UPDATE `creature_template` SET `unit_flags`=`unit_flags`|33554432 WHERE `entry`=54197;

-- 10. Baine's Totem: spellclick (the visual passenger 54433 is summoned by the totem script)
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry`=54434;
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(54434, 107837, 3, 0);

-- 11. Proc data: Molten Axe / Molten Fists trigger their blast on melee swings,
--     the Moon Guard tracker procs on any damage taken
DELETE FROM `spell_proc` WHERE `SpellId` IN (101836,101866,102491);
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(101836, 0, 0, 0, 0, 0, 0x14, 0, 0, 0, 0, 0, 0, 100, 0, 0),      -- Molten Axe -> Molten Blast (101840)
(101866, 0, 0, 0, 0, 0, 0x14, 0, 0, 0, 0, 0, 0, 100, 0, 0),      -- Molten Fists -> 101867
(102491, 0, 0, 0, 0, 0, 0x100000, 0, 0, 0, 0, 0, 0, 100, 0, 0);  -- Tyrande Achievement Tracker -> Fail (102539, sends map event 29235)

-- 12. Implicit target whitelists
-- (Platform targeting for Pulverize 101815 is handled by spell_echo_of_baine_pulverize_platform -
--  the conditions validator rejects gameobject entries when TargetA is a unit target.)
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry` IN (101815,101401,101842,102002,102491,102542);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 101401, 0, 0, 31, 0, 3, 54403, 0, 0, 0, 0, '', 'Blighted Arrows targets Blighted Arrows stalkers'),
(13, 3, 101842, 0, 0, 31, 0, 3, 54512, 0, 0, 0, 0, '', 'Moonlit targets Time-Twisted Sentinel'),
(13, 3, 101842, 0, 1, 31, 0, 3, 54688, 0, 0, 0, 0, '', 'Moonlit targets Time-Twisted Nightsaber'),
(13, 3, 101842, 0, 2, 31, 0, 3, 54699, 0, 0, 0, 0, '', 'Moonlit targets Time-Twisted Nightsaber'),
(13, 3, 101842, 0, 3, 31, 0, 3, 54700, 0, 0, 0, 0, '', 'Moonlit targets Time-Twisted Nightsaber'),
(13, 3, 101842, 0, 4, 31, 0, 3, 54701, 0, 0, 0, 0, '', 'Moonlit targets Time-Twisted Huntress'),
(13, 1, 102002, 0, 0, 31, 0, 3, 54508, 0, 0, 0, 0, '', 'Shrink targets Pool of Moonlight'),
(13, 1, 102491, 0, 0, 31, 0, 4, 0, 0, 0, 0, 0, '', 'Tyrande Achievement Tracker targets players'),
(13, 3, 102542, 0, 0, 31, 0, 4, 0, 0, 0, 0, 0, '', 'Tyrande Achievement Spell targets players');

-- 13. Echo of Sylvanas jumps to the arena center (Death Grip self teleport)
DELETE FROM `spell_target_position` WHERE `ID`=101398;
INSERT INTO `spell_target_position` (`ID`, `EffectIndex`, `MapID`, `PositionX`, `PositionY`, `PositionZ`, `Orientation`, `VerifiedBuild`) VALUES
(101398, 0, 938, 3840.03, 914.043, 56.0167, 1.309, 0);

-- 14. Severed Ties (6130): criteria 18499 asks the instance script
DELETE FROM `achievement_criteria_data` WHERE `criteria_id`=18499;
INSERT INTO `achievement_criteria_data` (`criteria_id`, `type`, `value1`, `value2`, `ScriptName`) VALUES
(18499, 18, 0, 0, '');

-- 15. Creature texts (localized strings via broadcast_text)
DELETE FROM `creature_text` WHERE `CreatureID` IN (54431,54123,54544,54508);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54431, 0, 0, IF(`Text`='', `Text1`, `Text`), 14, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 3, 'Echo of Baine - SAY_INTRO' FROM `broadcast_text` WHERE `ID`=55812;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54431, 1, 0, IF(`Text`='', `Text1`, `Text`), 14, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Echo of Baine - SAY_AGGRO' FROM `broadcast_text` WHERE `ID`=55813;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54431, 2, 0, IF(`Text`='', `Text1`, `Text`), 14, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Echo of Baine - SAY_PULVERIZE' FROM `broadcast_text` WHERE `ID`=55818;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54431, 3, 0, IF(`Text`='', `Text1`, `Text`), 41, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Echo of Baine - EMOTE_PULVERIZE' FROM `broadcast_text` WHERE `ID`=53039;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54431, 4, `ID`-55815, IF(`Text`='', `Text1`, `Text`), 14, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Echo of Baine - SAY_SLAY' FROM `broadcast_text` WHERE `ID` IN (55815,55816,55817);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54431, 5, 0, IF(`Text`='', `Text1`, `Text`), 14, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Echo of Baine - SAY_DEATH' FROM `broadcast_text` WHERE `ID`=55814;

INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54123, 0, 0, IF(`Text`='', `Text1`, `Text`), 14, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Echo of Sylvanas - SAY_AGGRO' FROM `broadcast_text` WHERE `ID`=54769;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54123, 1, 0, IF(`Text`='', `Text1`, `Text`), 14, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Echo of Sylvanas - SAY_CALLING' FROM `broadcast_text` WHERE `ID`=54773;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54123, 2, 0, IF(`Text`='', `Text1`, `Text`), 14, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Echo of Sylvanas - SAY_SLAY' FROM `broadcast_text` WHERE `ID`=54772;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54123, 3, 0, IF(`Text`='', `Text1`, `Text`), 14, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Echo of Sylvanas - SAY_DEATH' FROM `broadcast_text` WHERE `ID`=54774;

INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54544, 0, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 3, 'Echo of Tyrande - SAY_GAUNTLET_START' FROM `broadcast_text` WHERE `ID`=53147;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54544, 1, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 3, 'Echo of Tyrande - SAY_POOL_1_FADES' FROM `broadcast_text` WHERE `ID`=53148;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54544, 2, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 3, 'Echo of Tyrande - SAY_POOL_2_FADES' FROM `broadcast_text` WHERE `ID`=53149;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54544, 3, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 3, 'Echo of Tyrande - SAY_POOL_3_FADES' FROM `broadcast_text` WHERE `ID`=53150;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54544, 4, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 3, 'Echo of Tyrande - SAY_POOL_4_FADES' FROM `broadcast_text` WHERE `ID`=53151;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54544, 5, 0, IF(`Text`='', `Text1`, `Text`), 41, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 3, 'Echo of Tyrande - EMOTE_DARK_MOONLIGHT' FROM `broadcast_text` WHERE `ID`=53110;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54544, 6, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Echo of Tyrande - SAY_AGGRO' FROM `broadcast_text` WHERE `ID`=53155;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54544, 7, 0, IF(`Text`='', `Text1`, `Text`), 14, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Echo of Tyrande - SAY_LUNAR_GUIDANCE_1' FROM `broadcast_text` WHERE `ID`=53152;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54544, 8, 0, IF(`Text`='', `Text1`, `Text`), 14, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Echo of Tyrande - SAY_LUNAR_GUIDANCE_2' FROM `broadcast_text` WHERE `ID`=53153;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54544, 9, 0, IF(`Text`='', `Text1`, `Text`), 14, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Echo of Tyrande - SAY_MOONLANCE' FROM `broadcast_text` WHERE `ID`=53158;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54544, 10, 0, IF(`Text`='', `Text1`, `Text`), 14, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Echo of Tyrande - SAY_EYES_OF_THE_GODDESS' FROM `broadcast_text` WHERE `ID`=53157;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54544, 11, 0, IF(`Text`='', `Text1`, `Text`), 14, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Echo of Tyrande - SAY_TEARS_OF_ELUNE' FROM `broadcast_text` WHERE `ID`=53154;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54544, 12, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Echo of Tyrande - SAY_DEATH' FROM `broadcast_text` WHERE `ID`=53159;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54544, 13, 0, IF(`Text`='', `Text1`, `Text`), 41, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Echo of Tyrande - EMOTE_LUNAR_GUIDANCE' FROM `broadcast_text` WHERE `ID`=53167;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54544, 14, 0, IF(`Text`='', `Text1`, `Text`), 41, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Echo of Tyrande - EMOTE_TEARS_OF_ELUNE' FROM `broadcast_text` WHERE `ID`=53160;

INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54508, 0, 0, IF(`Text`='', `Text1`, `Text`), 41, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 3, 'Pool of Moonlight - EMOTE_APPEARS' FROM `broadcast_text` WHERE `ID`=53097;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54508, 1, 0, IF(`Text`='', `Text1`, `Text`), 41, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 3, 'Pool of Moonlight - EMOTE_APPEARS_WEST' FROM `broadcast_text` WHERE `ID`=53098;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54508, 2, 0, IF(`Text`='', `Text1`, `Text`), 41, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 3, 'Pool of Moonlight - EMOTE_APPEARS_SOUTH' FROM `broadcast_text` WHERE `ID`=53099;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54508, 3, 0, IF(`Text`='', `Text1`, `Text`), 41, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 3, 'Pool of Moonlight - EMOTE_APPEARS_EAST' FROM `broadcast_text` WHERE `ID`=53101;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54508, 4, 0, IF(`Text`='', `Text1`, `Text`), 41, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 3, 'Pool of Moonlight - EMOTE_APPEARS_NORTH' FROM `broadcast_text` WHERE `ID`=53100;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54508, 5, 0, IF(`Text`='', `Text1`, `Text`), 41, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 3, 'Pool of Moonlight - EMOTE_FADES' FROM `broadcast_text` WHERE `ID`=53102;

-- 16. Wing trash abilities (SmartAI; spells verified against 4.3.4 Spell.dbc, timings sniff-derived)
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry` IN (54552,54553,54543,54511,54507,54920,54923,54693,54690,54691);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (54552,54553,54543,54511,54507,54920,54923,54693,54690,54691) AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
-- Time-Twisted Breaker (Obsidian)
(54552, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 9000, 12000, 0, 11, 102132, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Time-Twisted Breaker - IC - Cast Break Armor'),
(54552, 0, 1, 0, 0, 0, 100, 0, 8000, 12000, 13000, 18000, 0, 11, 102124, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Time-Twisted Breaker - IC - Cast Rupture Ground'),
-- Time-Twisted Seer (Obsidian)
(54553, 0, 0, 0, 0, 0, 100, 0, 4000, 8000, 15000, 18000, 0, 11, 102158, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Time-Twisted Seer - IC - Cast Sear Flesh'),
-- Time-Twisted Drake (Obsidian)
(54543, 0, 0, 0, 0, 0, 100, 0, 6000, 10000, 11000, 15000, 0, 11, 102135, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Time-Twisted Drake - IC - Cast Flame Breath'),
(54543, 0, 1, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 102134, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Time-Twisted Drake - At 30% HP - Cast Enrage'),
-- Time-Twisted Geist (Ruby)
(54511, 0, 0, 0, 2, 0, 100, 1, 0, 40, 0, 0, 0, 11, 101862, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Time-Twisted Geist - At 40% HP - Cast Cannibalize'),
-- Time-Twisted Scourge Beast (Ruby)
(54507, 0, 0, 0, 0, 0, 100, 0, 5000, 9000, 10000, 15000, 0, 11, 101888, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Time-Twisted Scourge Beast - IC - Cast Face Kick'),
-- Infinite Suppressor (Bronze causeway)
(54920, 0, 0, 0, 0, 0, 100, 0, 2000, 3000, 3000, 4000, 0, 11, 102601, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Infinite Suppressor - IC - Cast Arcane Wave'),
(54920, 0, 1, 0, 0, 0, 100, 0, 8000, 12000, 15000, 20000, 0, 11, 102600, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Infinite Suppressor - IC - Cast Temporal Vortex'),
-- Infinite Warden (Bronze causeway)
(54923, 0, 0, 0, 0, 0, 100, 0, 4000, 6000, 8000, 12000, 0, 11, 102598, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Infinite Warden - IC - Cast Void Strike'),
(54923, 0, 1, 0, 0, 0, 100, 0, 10000, 15000, 20000, 30000, 0, 11, 102599, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Infinite Warden - IC - Cast Void Shield'),
-- Time-Twisted Rifleman (Azure ambush) - provisional generic kit, no sniff coverage of this wing
(54693, 0, 0, 0, 0, 0, 100, 0, 1000, 2000, 2500, 3500, 0, 11, 6660, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Time-Twisted Rifleman - IC - Cast Shoot'),
-- Time-Twisted Priest (Azure ambush) - provisional
(54690, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 3500, 5000, 0, 11, 25054, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Time-Twisted Priest - IC - Cast Holy Smite'),
-- Time-Twisted Sorceress (Azure ambush) - provisional
(54691, 0, 0, 0, 0, 0, 100, 0, 1000, 2000, 3000, 4000, 0, 11, 15043, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Time-Twisted Sorceress - IC - Cast Frostbolt');
