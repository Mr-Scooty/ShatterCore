-- Hour of Twilight (map 940): complete dungeon implementation
-- Thrall escort framework (5 legs), Arcurion, Asira Dawnslayer, Archbishop
-- Benedictus, Life Warden flight, trash kits, texts, Eclipse wiring.

-- 1. Creature script bindings
UPDATE `creature_template` SET `ScriptName`='boss_arcurion' WHERE `entry`=54590;
UPDATE `creature_template` SET `ScriptName`='boss_asira_dawnslayer' WHERE `entry`=54968;
UPDATE `creature_template` SET `ScriptName`='boss_archbishop_benedictus' WHERE `entry`=54938;
UPDATE `creature_template` SET `ScriptName`='npc_hot_thrall_entrance' WHERE `entry`=54548;
UPDATE `creature_template` SET `ScriptName`='npc_hot_thrall_frozen' WHERE `entry`=55779;
UPDATE `creature_template` SET `ScriptName`='npc_hot_thrall_galakrond' WHERE `entry`=54972;
UPDATE `creature_template` SET `ScriptName`='npc_hot_thrall_titans' WHERE `entry`=54634;
UPDATE `creature_template` SET `ScriptName`='npc_hot_thrall_epilogue' WHERE `entry`=54971;
UPDATE `creature_template` SET `ScriptName`='npc_hot_frozen_servitor_summon' WHERE `entry`=54600;
UPDATE `creature_template` SET `ScriptName`='npc_hot_icy_tomb' WHERE `entry`=54995;
UPDATE `creature_template` SET `ScriptName`='npc_hot_canyon_ambusher' WHERE `entry` IN (54555,55559,55563);
UPDATE `creature_template` SET `ScriptName`='npc_hot_twilight_assassin' WHERE `entry`=55106;
UPDATE `creature_template` SET `ScriptName`='npc_hot_life_warden_thrall' WHERE `entry`=55415;
UPDATE `creature_template` SET `ScriptName`='npc_hot_life_warden_taxi' WHERE `entry`=55549;
UPDATE `creature_template` SET `ScriptName`='npc_hot_rising_fire_totem' WHERE `entry`=55474;
UPDATE `creature_template` SET `ScriptName`='npc_hot_benedictus_orb' WHERE `entry` IN (55377,55467);
UPDATE `creature_template` SET `ScriptName`='npc_hot_benedictus_pool' WHERE `entry` IN (55427,55468);
UPDATE `creature_template` SET `ScriptName`='npc_hot_benedictus_wave' WHERE `entry` IN (55441,55469);
UPDATE `creature_template` SET `ScriptName`='npc_hot_benedictus_controller' WHERE `entry`=55445;

-- 2. Trash ability kits (SmartAI) - spells verified in the 4.3.4 client DBC and retail sniffs
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry` IN (55107,55109,55111,55112,54632,54633,54686,54646);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (55107,55109,55111,55112,54632,54633,54686,54646) AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
-- Twilight Ranger 55107: Shoot filler, Ice Trap on melee contact, Disengage
(55107,0,0,0,0,0,100,0,2400,3800,2400,3800,11,102978,64,0,0,0,0,2,0,0,0,0,0,0,0,'Twilight Ranger - IC - Cast Shoot'),
(55107,0,1,0,9,0,100,0,0,5,20000,25000,11,102977,0,0,0,0,0,1,0,0,0,0,0,0,0,'Twilight Ranger - victim in melee - Cast Ice Trap'),
(55107,0,2,0,9,0,100,0,0,5,25000,30000,11,102975,0,0,0,0,0,1,0,0,0,0,0,0,0,'Twilight Ranger - victim in melee - Cast Disengage'),
(55107,0,3,0,4,0,50,1,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Twilight Ranger - on aggro - Say "Death to all who oppose us!"'),
-- Twilight Shadow-Walker 55109: Mind Flay channel, Hungering Shadows
(55109,0,0,0,0,0,100,0,5000,8000,9000,12000,11,103024,0,0,0,0,0,2,0,0,0,0,0,0,0,'Twilight Shadow-Walker - IC - Cast Mind Flay'),
(55109,0,1,0,0,0,100,0,10000,14000,16000,20000,11,103021,0,0,0,0,0,5,0,0,0,0,0,0,0,'Twilight Shadow-Walker - IC - Cast Hungering Shadows'),
-- Twilight Thug 55111: Beatdown, Bash
(55111,0,0,0,0,0,100,0,7000,10000,9000,13000,11,102989,0,0,0,0,0,2,0,0,0,0,0,0,0,'Twilight Thug - IC - Cast Beatdown'),
(55111,0,1,0,0,0,100,0,12000,16000,15000,20000,11,102990,0,0,0,0,0,2,0,0,0,0,0,0,0,'Twilight Thug - IC - Cast Bash'),
-- Twilight Bruiser 55112: Cleave, Staggering Blow, Mortal Strike
(55112,0,0,0,0,0,100,0,5000,8000,6000,9000,11,103001,0,0,0,0,0,2,0,0,0,0,0,0,0,'Twilight Bruiser - IC - Cast Cleave'),
(55112,0,1,0,0,0,100,0,9000,12000,11000,15000,11,103000,0,0,0,0,0,2,0,0,0,0,0,0,0,'Twilight Bruiser - IC - Cast Staggering Blow'),
(55112,0,2,0,0,0,100,0,13000,17000,14000,18000,11,103002,0,0,0,0,0,2,0,0,0,0,0,0,0,'Twilight Bruiser - IC - Cast Mortal Strike'),
-- Faceless Brute 54632: Tentacle Smash, Squeeze Lifeless on a random player
(54632,0,0,0,0,0,100,0,8000,12000,10000,15000,11,102848,0,0,0,0,0,2,0,0,0,0,0,0,0,'Faceless Brute - IC - Cast Tentacle Smash'),
(54632,0,1,0,0,0,100,0,18000,24000,25000,32000,11,102861,0,0,0,0,0,5,0,0,0,0,0,0,0,'Faceless Brute - IC - Cast Squeeze Lifeless'),
-- Faceless Shadow Weaver 54633: Shadow Volley, Seeking Shadows
(54633,0,0,0,0,0,100,0,4000,7000,7000,10000,11,102992,0,0,0,0,0,2,0,0,0,0,0,0,0,'Faceless Shadow Weaver - IC - Cast Shadow Volley'),
(54633,0,1,0,0,0,100,0,10000,15000,14000,19000,11,102983,0,0,0,0,0,5,0,0,0,0,0,0,0,'Faceless Shadow Weaver - IC - Cast Seeking Shadows'),
-- Shadow Borer 54686: Shadow Bore
(54686,0,0,0,0,0,100,0,4000,6000,5000,8000,11,102997,0,0,0,0,0,2,0,0,0,0,0,0,0,'Shadow Borer - IC - Cast Shadow Bore'),
-- Corrupted Slime 54646: Corrupted Bite
(54646,0,0,0,0,0,100,0,4000,7000,6000,9000,11,102224,0,0,0,0,0,2,0,0,0,0,0,0,0,'Corrupted Slime - IC - Cast Corrupted Bite');

-- 3. Template addon corrections
-- 55779: TDB ships him permanently frozen (103251) - he walks in free and only gets tombed mid-fight.
UPDATE `creature_template_addon` SET `auras`='' WHERE `entry`=55779;
-- 55445: TDB ships the P1 Seaping Light driver as a permanent aura - the controller script drives states.
UPDATE `creature_template_addon` SET `auras`='' WHERE `entry`=55445;
-- 54646: needs the faceless aggro pulse so rained slimes engage on landing (sniffed).
DELETE FROM `creature_template_addon` WHERE `entry`=54646;
INSERT INTO `creature_template_addon` (`entry`,`auras`) VALUES (54646,'102231');

-- 4. Gossip: the gauntlet Thrall's menu ships without its ready-check option
DELETE FROM `gossip_menu_option` WHERE `MenuID`=13022;
INSERT INTO `gossip_menu_option` (`MenuID`,`OptionID`,`OptionIcon`,`OptionText`,`OptionBroadcastTextID`,`OptionType`,`OptionNpcFlag`,`VerifiedBuild`) VALUES
(13022,0,0,'Yes Thrall, lets do this!',53067,1,1,15595);

-- 5. Spell script bindings
DELETE FROM `spell_script_names` WHERE `ScriptName` IN
('spell_arcurion_icy_tomb_summon','spell_asira_mark_of_silence','spell_asira_throw_knife','spell_asira_choking_smoke_bomb',
 'spell_asira_blade_barrier','spell_benedictus_engulfing_twilight','spell_benedictus_wave_pulse','spell_benedictus_chain_lightning',
 'spell_hot_teleporter_trigger');

-- 103762 Engulfing Twilight selects its victim via TARGET_UNIT_SRC_AREA_ENTRY:
-- retail imprisons Thrall (54971).
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry`=103762;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(13,1,103762,0,0,31,0,3,54971,0,0,0,0,'','Engulfing Twilight targets Thrall');
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(103249,'spell_arcurion_icy_tomb_summon'),
(102726,'spell_asira_mark_of_silence'),
(103597,'spell_asira_throw_knife'),
(103558,'spell_asira_choking_smoke_bomb'),
(103419,'spell_asira_blade_barrier'),
(103562,'spell_asira_blade_barrier'),
(103684,'spell_benedictus_wave_pulse'),
(103781,'spell_benedictus_wave_pulse'),
(103637,'spell_benedictus_chain_lightning'),
(108925,'spell_hot_teleporter_trigger');

-- 6. Eclipse (6132): criteria 18669 (kill 10x Twilight Spark 55466) resets when the player
--    is hit by serverside spell 110260 (FailEvent 9 = BeSpellTarget, straight from the DBC).
--    The instance casts it on everyone when Benedictus dies.
DELETE FROM `spell_dbc` WHERE `Id`=110260;
INSERT INTO `spell_dbc` (`Id`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `AttributesEx4`, `AttributesEx5`, `AttributesEx6`, `AttributesEx7`, `AttributesEx8`, `AttributesEx9`, `AttributesEx10`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `SchoolMask`, `SpellAuraOptionsId`, `SpellCastingRequirementsId`, `SpellCategoriesId`, `SpellClassOptionsId`, `SpellEquippedItemsId`, `SpellInterruptsId`, `SpellLevelsId`, `SpellTargetRestrictionsId`, `SpellName`) VALUES
(110260, 2843738368, 268436512, 540677, 269943552, 128, 393225, 5120, 33554432, 32, 0, 0, 0, 36, 13, 0, 38, 0, 0, 0, 0, 0, 0, 0, '(Serverside/Non-DB2) Cancel Eclipse');
DELETE FROM `spelleffect_dbc` WHERE `Id`=160121;
INSERT INTO `spelleffect_dbc` (`Id`, `Effect`, `EffectAura`, `EffectRadiusMaxIndex`, `EffectImplicitTargetA`, `EffectImplicitTargetB`, `SpellID`, `EffectIndex`, `Comment`) VALUES
(160121, 6, 4, 28, 22, 7, 110260, 0, 'Cancel Eclipse - spark counter fail event');

-- 7. creature_text (texts + VO pulled from broadcast_text; all IDs sniff-verified)
DELETE FROM `creature_text` WHERE `CreatureID` IN (54548,55779,54972,54634,54971,54590,54968,54938,55549,54628,55107);

-- Thrall 54548 (canyon leg)
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54548, 0, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Thrall - SAY_CANYON_INTRO' FROM `broadcast_text` WHERE `ID`=53046;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54548, 1, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Thrall - SAY_CANYON_FOUND_US' FROM `broadcast_text` WHERE `ID`=53826;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54548, 2, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Thrall - SAY_CANYON_WHAT_MAGIC' FROM `broadcast_text` WHERE `ID`=53050;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54548, 3, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Thrall - SAY_CANYON_LOOK_OUT' FROM `broadcast_text` WHERE `ID`=54489;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54548, 4, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Thrall - SAY_CANYON_KEEP_MOVING' FROM `broadcast_text` WHERE `ID`=53048;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54548, 5, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Thrall - SAY_CANYON_ANOTHER' FROM `broadcast_text` WHERE `ID`=54490;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54548, 6, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Thrall - SAY_CANYON_BREATHER' FROM `broadcast_text` WHERE `ID`=54491;

-- Arcurion 54590 (canyon voice heard map-wide, then the encounter)
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54590, 0, 0, IF(`Text`='', `Text1`, `Text`), 14, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 3, 'Arcurion - SAY_CANYON_INTRO' FROM `broadcast_text` WHERE `ID`=53797;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54590, 1, 0, IF(`Text`='', `Text1`, `Text`), 14, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 3, 'Arcurion - SAY_CANYON_AMBUSH' FROM `broadcast_text` WHERE `ID`=53798;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54590, 2, 0, IF(`Text`='', `Text1`, `Text`), 14, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 3, 'Arcurion - SAY_CANYON_ARRIVAL' FROM `broadcast_text` WHERE `ID`=53803;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54590, 3, 0, IF(`Text`='', `Text1`, `Text`), 14, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Arcurion - SAY_MATERIALIZE' FROM `broadcast_text` WHERE `ID`=53818;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54590, 4, 0, IF(`Text`='', `Text1`, `Text`), 14, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Arcurion - SAY_AGGRO' FROM `broadcast_text` WHERE `ID`=54495;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54590, 5, 0, IF(`Text`='', `Text1`, `Text`), 41, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Arcurion - EMOTE_RIM_FORCES' FROM `broadcast_text` WHERE `ID`=54176;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54590, 6, 0, IF(`Text`='', `Text1`, `Text`), 41, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Arcurion - EMOTE_FREEZE_THRALL' FROM `broadcast_text` WHERE `ID`=54199;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54590, 7, 0, IF(`Text`='', `Text1`, `Text`), 14, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Arcurion - SAY_FREEZE_THRALL' FROM `broadcast_text` WHERE `ID`=54497;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54590, 8, 0, IF(`Text`='', `Text1`, `Text`), 14, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Arcurion - SAY_TORRENT' FROM `broadcast_text` WHERE `ID`=54097;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54590, 9, 0, IF(`Text`='', `Text1`, `Text`), 14, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Arcurion - SAY_DEATH' FROM `broadcast_text` WHERE `ID`=54502;

-- Thrall 55779 (Arcurion leg)
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 55779, 0, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Thrall - SAY_SHOW_YOURSELF' FROM `broadcast_text` WHERE `ID`=53049;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 55779, 1, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Thrall - SAY_SURROUNDED' FROM `broadcast_text` WHERE `ID`=54177;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 55779, 2, 0, IF(`Text`='', `Text1`, `Text`), 14, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Thrall - SAY_ALMOST_GOT_HIM' FROM `broadcast_text` WHERE `ID`=53963;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 55779, 3, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Thrall - SAY_DISCOVERED' FROM `broadcast_text` WHERE `ID`=54294;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 55779, 4, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Thrall - SAY_FOLLOW_ME' FROM `broadcast_text` WHERE `ID`=54446;

-- Thrall 54972 (Galakrond leg / Asira)
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54972, 0, 0, IF(`Text`='', `Text1`, `Text`), 14, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Thrall - SAY_ROAD_BEWARE' FROM `broadcast_text` WHERE `ID`=53553;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54972, 1, 0, IF(`Text`='', `Text1`, `Text`), 14, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Thrall - SAY_ROAD_LET_NONE' FROM `broadcast_text` WHERE `ID`=53821;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54972, 2, 0, IF(`Text`='', `Text1`, `Text`), 14, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Thrall - SAY_ROAD_LET_THEM_COME' FROM `broadcast_text` WHERE `ID`=53823;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54972, 3, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Thrall - SAY_ROAD_DRAKES_MEET' FROM `broadcast_text` WHERE `ID`=53857;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54972, 4, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Thrall - SAY_ROAD_ABOVE_US' FROM `broadcast_text` WHERE `ID`=53875;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54972, 5, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Thrall - SAY_ROAD_ASSASSIN' FROM `broadcast_text` WHERE `ID`=53913;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54972, 6, 0, IF(`Text`='', `Text1`, `Text`), 14, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Thrall - SAY_ROAD_NOT_STOPPED' FROM `broadcast_text` WHERE `ID`=53968;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54972, 7, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Thrall - SAY_ROAD_WELL_DONE' FROM `broadcast_text` WHERE `ID`=53986;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54972, 8, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Thrall - SAY_ROAD_FLY_AHEAD' FROM `broadcast_text` WHERE `ID`=53987;

-- Asira Dawnslayer 54968 (texts live in the female Text1 field)
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54968, 0, 0, IF(`Text`='', `Text1`, `Text`), 14, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Asira Dawnslayer - SAY_INTRO' FROM `broadcast_text` WHERE `ID`=53825;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54968, 1, 0, IF(`Text`='', `Text1`, `Text`), 14, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Asira Dawnslayer - SAY_BANTER' FROM `broadcast_text` WHERE `ID`=53751;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54968, 2, 0, IF(`Text`='', `Text1`, `Text`), 14, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Asira Dawnslayer - SAY_ENGAGE' FROM `broadcast_text` WHERE `ID`=53268;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54968, 3, `ID`-54826, IF(`Text`='', `Text1`, `Text`), 14, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Asira Dawnslayer - SAY_BARK' FROM `broadcast_text` WHERE `ID` IN (54826,54827,54828);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54968, 4, 0, IF(`Text`='', `Text1`, `Text`), 14, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Asira Dawnslayer - SAY_DEATH' FROM `broadcast_text` WHERE `ID`=54821;

-- Life Warden taxi 55549
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 55549, 0, 0, IF(`Text`='', `Text1`, `Text`), 15, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Life Warden - WHISPER_LOOK_THERE' FROM `broadcast_text` WHERE `ID`=54036;

-- Dark Haze 54628
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54628, 0, 0, IF(`Text`='', `Text1`, `Text`), 41, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Dark Haze - EMOTE_OLD_GODS' FROM `broadcast_text` WHERE `ID`=53085;

-- Twilight Ranger 55107 (SmartAI group 0)
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 55107, 0, 0, IF(`Text`='', `Text1`, `Text`), 14, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Twilight Ranger - SAY_AGGRO' FROM `broadcast_text` WHERE `ID`=53757;

-- Thrall 54634 has no lines (gossip only)

-- Archbishop Benedictus 54938
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54938, 0, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Benedictus - SAY_GET_INSIDE' FROM `broadcast_text` WHERE `ID`=53254;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54938, 1, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Benedictus - SAY_DEMAND_SOUL' FROM `broadcast_text` WHERE `ID`=53255;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54938, 2, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 396, 0, `SoundEntriesID`, `ID`, 0, 'Benedictus - SAY_THIS_WAY' FROM `broadcast_text` WHERE `ID`=53283;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54938, 3, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Benedictus - SAY_ONLY_POWER' FROM `broadcast_text` WHERE `ID`=53278;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54938, 4, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Benedictus - SAY_TRUE_MASTERS' FROM `broadcast_text` WHERE `ID`=54755;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54938, 5, 0, IF(`Text`='', `Text1`, `Text`), 14, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Benedictus - SAY_AGGRO' FROM `broadcast_text` WHERE `ID`=56544;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54938, 6, 0, IF(`Text`='', `Text1`, `Text`), 14, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Benedictus - SAY_WAVE_OF_VIRTUE' FROM `broadcast_text` WHERE `ID`=56542;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54938, 7, 0, IF(`Text`='', `Text1`, `Text`), 14, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Benedictus - SAY_EPIPHANY' FROM `broadcast_text` WHERE `ID`=56541;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54938, 8, 0, IF(`Text`='', `Text1`, `Text`), 14, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Benedictus - SAY_WAVE_OF_TWILIGHT' FROM `broadcast_text` WHERE `ID`=56543;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54938, 9, 0, IF(`Text`='', `Text1`, `Text`), 14, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Benedictus - SAY_DEATH' FROM `broadcast_text` WHERE `ID`=56538;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54938, 10, 0, IF(`Text`='', `Text1`, `Text`), 41, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Benedictus - EMOTE_WAVE_OF_VIRTUE' FROM `broadcast_text` WHERE `ID`=53877;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54938, 11, 0, IF(`Text`='', `Text1`, `Text`), 41, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Benedictus - EMOTE_WAVE_OF_TWILIGHT' FROM `broadcast_text` WHERE `ID`=53896;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54938, 12, 0, IF(`Text`='', `Text1`, `Text`), 41, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Benedictus - EMOTE_EPIPHANY' FROM `broadcast_text` WHERE `ID`=53904;

-- Thrall 54971 (Benedictus fight / epilogue)
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54971, 0, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 396, 0, `SoundEntriesID`, `ID`, 0, 'Thrall - SAY_THRALL_REFUSE' FROM `broadcast_text` WHERE `ID`=53275;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 54971, 1, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Thrall - SAY_THRALL_FIGUREHEAD' FROM `broadcast_text` WHERE `ID`=53279;
