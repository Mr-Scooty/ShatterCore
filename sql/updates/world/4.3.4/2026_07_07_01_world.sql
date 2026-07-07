-- Warlord Zon'ozz (Dragon Soul) - encounter implementation
-- Boss 55308 (10N) with difficulty entries 55309 (25N) / 55310 (10H) / 55311 (25H)
-- Void of the Unmaking 55334 (shared across modes)
-- Tentacles of Go'rath (heroic-only summons, base 10N -> 25N / 10H / 25H):
--   Eye   55416 -> 55751 / 55752 / 55753
--   Flail 55417 -> 55754 / 55755 / 55756
--   Claw  55418 -> 55757 / 55758 / 55759
--
-- SpellDifficulty.dbc forks (client-side, no spelldifficulty_dbc rows needed):
--   104543 Focused Anger          -> 109409 / 109410 / 109411
--   104322 Psychic Drain          -> 104606 / 104607 / 104608
--   103434 Disrupting Shadows     -> 104599 / 104600 / 104601
--   103948 Disrupting Shadows kb  -> 108342 / 108343 / 108344 (heroic forks are ally-AoE)
--   103527 Void Diffusion         -> 104605 / 108345 / 108346 (split damage, stacks 106836 on the ball)
--   104378 Black Blood of Go'rath -> 110322 (30s raid pulse)
--   104377 Black Blood eruption   -> 110306 (heroic, stacks per living Eye)
--   103627 Void visual            -> 110305 / 110304 / 110303
--   104347 Shadow Gaze            -> 104602 / 104603 / 104604
--   109199 Wild Flail             -> 110308
-- 104031 (boss +5% damage taken per stack) and 103571 (summon) have no forks.

-- Script bindings (base entries only - the core ignores ScriptName on difficulty entries)
UPDATE `creature_template` SET `ScriptName` = 'boss_warlord_zonozz' WHERE `entry` = 55308;
UPDATE `creature_template` SET `ScriptName` = 'npc_void_of_the_unmaking' WHERE `entry` = 55334;
UPDATE `creature_template` SET `ScriptName` = 'npc_zonozz_eye_of_gorath' WHERE `entry` = 55416;
UPDATE `creature_template` SET `ScriptName` = 'npc_zonozz_flail_of_gorath' WHERE `entry` = 55417;
UPDATE `creature_template` SET `ScriptName` = 'npc_zonozz_claw_of_gorath' WHERE `entry` = 55418;

-- Boss difficulty chain + stub template fixes (55309/55310/55311 were level-1 faction-35 WDB stubs)
UPDATE `creature_template` SET `difficulty_entry_1` = 55309, `difficulty_entry_2` = 55310, `difficulty_entry_3` = 55311 WHERE `entry` = 55308;
UPDATE `creature_template` SET `minlevel` = 88, `maxlevel` = 88, `exp` = 3, `faction` = 14, `unit_class` = 1, `unit_flags2` = 2048 WHERE `entry` IN (55309, 55310, 55311);

-- Health (user-supplied retail values, base health 85,892 at level 88 expansion 3):
--   10N 55308:  794.0 ->  68,198,248 (already correct)
--   25N 55309: 2378.0 -> 204,251,176 (already correct)
--   10H 55310:  704.9 ->  60,545,271 (was 1007)
--   25H 55311: 2114.7 -> 181,635,812 (was 3021)
UPDATE `creature_template` SET `HealthModifier` = 704.9 WHERE `entry` = 55310;
UPDATE `creature_template` SET `HealthModifier` = 2114.7 WHERE `entry` = 55311;

-- Tentacle difficulty chains + stub fixes. Only the heroic slots ever spawn;
-- health targets (base 77,490 at level 85): Eye ~0.6M/1.6M, Flail ~3.1M/9.2M,
-- Claw ~4.3M/13M for 10H/25H.
UPDATE `creature_template` SET `difficulty_entry_1` = 55751, `difficulty_entry_2` = 55752, `difficulty_entry_3` = 55753 WHERE `entry` = 55416;
UPDATE `creature_template` SET `difficulty_entry_1` = 55754, `difficulty_entry_2` = 55755, `difficulty_entry_3` = 55756 WHERE `entry` = 55417;
UPDATE `creature_template` SET `difficulty_entry_1` = 55757, `difficulty_entry_2` = 55758, `difficulty_entry_3` = 55759 WHERE `entry` = 55418;
-- unit_flags2 mirrors each base entry (2048 Eye/Flail, 0 Claw) - the core
-- warns when a difficulty entry diverges from its base
UPDATE `creature_template` SET `minlevel` = 85, `maxlevel` = 85, `exp` = 3, `faction` = 14, `unit_class` = 1, `unit_flags2` = 2048 WHERE `entry` IN
(55751, 55752, 55753, 55754, 55755, 55756);
UPDATE `creature_template` SET `minlevel` = 85, `maxlevel` = 85, `exp` = 3, `faction` = 14, `unit_class` = 1, `unit_flags2` = 0 WHERE `entry` IN
(55757, 55758, 55759);
UPDATE `creature_template` SET `HealthModifier` = 7.7   WHERE `entry` = 55752; -- Eye 10H
UPDATE `creature_template` SET `HealthModifier` = 20.6  WHERE `entry` = 55753; -- Eye 25H
UPDATE `creature_template` SET `HealthModifier` = 40.0  WHERE `entry` = 55755; -- Flail 10H
UPDATE `creature_template` SET `HealthModifier` = 118.7 WHERE `entry` = 55756; -- Flail 25H
UPDATE `creature_template` SET `HealthModifier` = 55.5  WHERE `entry` = 55758; -- Claw 10H
UPDATE `creature_template` SET `HealthModifier` = 167.8 WHERE `entry` = 55759; -- Claw 25H

-- Full boss CC immunity mask (fork convention) for the boss chain, the void
-- sphere and the rooted tentacles
UPDATE `creature_template` SET `mechanic_immune_mask` = 650854271 WHERE `entry` IN
(55308, 55309, 55310, 55311,
 55334,
 55416, 55751, 55752, 55753,
 55417, 55754, 55755, 55756,
 55418, 55757, 55758, 55759);

-- The Void of the Unmaking shipped with two models - Idx 0 is an invisible
-- trigger model and would make the ball randomly unseeable. Keep the orb only.
DELETE FROM `creature_template_model` WHERE `CreatureID` = 55334;
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `Probability`, `VerifiedBuild`) VALUES
(55334, 0, 39108, 1, 0);

-- Raid Finder stats template (health only): 70% of the 25N value, user-approved
--   58231 boss: 85,892 (L88) * 1664.5 = 142,967,234 (~143.0M)
DELETE FROM `creature_template` WHERE `entry` = 58231;
INSERT INTO `creature_template` (`entry`, `name`, `femaleName`, `subname`, `minlevel`, `maxlevel`, `exp`, `faction`, `unit_class`, `type`, `HealthModifier`, `AIName`, `ScriptName`, `VerifiedBuild`) VALUES
(58231, 'Warlord Zon''ozz', '', 'LFR Stats', 88, 88, 3, 14, 1, 10, 1664.5, '', '', 0);

DELETE FROM `creature_template_model` WHERE `CreatureID` = 58231;
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `Probability`, `VerifiedBuild`) VALUES
(58231, 0, 39138, 1, 0);

-- Spawn (map 967, all four raid spawn modes; home position from retail sniffs)
DELETE FROM `creature` WHERE `guid` = 9000636;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `VerifiedBuild`) VALUES
(9000636, 55308, 967, 0, 0, 15, 0, 0, 0, 0, -1769.3281, -1916.8698, -226.271, 1.29154, 604800, 0, 0, 0, 0, 0, 0, 0, 0, 0);

-- Texts. Each Shath'Yar yell is paired with a translated whisper to the raid
-- (Yor'sahj pattern). Sounds: VO_DS_ZONOZZ_* 26335-26345; the spell-VO to
-- event mapping (Void/Shadows/Phase) is a best fit - verify in the walkthrough.
DELETE FROM `broadcast_text` WHERE `ID` BETWEEN 73849 AND 73870;
INSERT INTO `broadcast_text` (`ID`, `LanguageID`, `Text`, `Text1`, `EmoteID1`, `EmoteID2`, `EmoteID3`, `EmoteDelay1`, `EmoteDelay2`, `EmoteDelay3`, `SoundEntriesID`, `EmotesID`, `Flags`, `VerifiedBuild`) VALUES
(73849, 0, 'Vwyq agth sshoq''meg N''Zoth vra zz shfk qwor ga''halahs agthu. Uulg''ma, ag qam.', '', 0, 0, 0, 0, 0, 0, 26337, 0, 0, 0),
(73850, 0, 'Once more shall the twisted flesh-banners of N''Zoth chitter and howl above the fly-blown corpse of this world. After millennia, we have returned.', '', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(73851, 0, 'Zzof Shuul''wah. Thoq fssh N''Zoth!', '', 0, 0, 0, 0, 0, 0, 26335, 0, 0, 0),
(73852, 0, 'Victory for Deathwing. For the glory of N''Zoth!', '', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(73853, 0, 'Gul''kafh an''qov N''Zoth.', '', 0, 0, 0, 0, 0, 0, 26340, 0, 0, 0),
(73854, 0, 'Gaze into the heart of N''Zoth.', '', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(73855, 0, 'N''Zoth ga zyqtahg iilth.', '', 0, 0, 0, 0, 0, 0, 26345, 0, 0, 0),
(73856, 0, 'The will of N''Zoth corrupts you.', '', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(73857, 0, 'Sk''shgn eqnizz hoq.', '', 0, 0, 0, 0, 0, 0, 26342, 0, 0, 0),
(73858, 0, 'Your fear drives me.', '', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(73859, 0, 'Sk''magg yawifk hoq.', '', 0, 0, 0, 0, 0, 0, 26343, 0, 0, 0),
(73860, 0, 'Your suffering strengthens me.', '', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(73861, 0, 'Sk''uuyat guulphg hoq.', '', 0, 0, 0, 0, 0, 0, 26344, 0, 0, 0),
(73862, 0, 'Your agony sustains me.', '', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(73863, 0, 'Sk''tek agth nuq N''Zoth yyqzz.', '', 0, 0, 0, 0, 0, 0, 26338, 0, 0, 0),
(73864, 0, 'Your skulls shall adorn N''Zoth''s writhing throne.', '', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(73865, 0, 'Sk''shuul agth vorzz N''Zoth naggwa''fssh.', '', 0, 0, 0, 0, 0, 0, 26339, 0, 0, 0),
(73866, 0, 'Your deaths shall sing of N''Zoth''s unending glory.', '', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(73867, 0, 'Sk''yahf agth huqth N''Zoth qornaus.', '', 0, 0, 0, 0, 0, 0, 26341, 0, 0, 0),
(73868, 0, 'Your souls shall sate N''Zoth''s endless hunger.', '', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(73869, 0, 'Uovssh thyzz... qwaz...', '', 0, 0, 0, 0, 0, 0, 26336, 0, 0, 0),
(73870, 0, 'To have waited so long... for this...', '', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

DELETE FROM `creature_text` WHERE `CreatureID` = 55308;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(55308, 0, 0, 'Vwyq agth sshoq''meg N''Zoth vra zz shfk qwor ga''halahs agthu. Uulg''ma, ag qam.', 14, 0, 100, 0, 0, 26337, 73849, 0, 'Zonozz - Intro'),
(55308, 1, 0, 'Once more shall the twisted flesh-banners of N''Zoth chitter and howl above the fly-blown corpse of this world. After millennia, we have returned.', 42, 0, 100, 0, 0, 0, 73850, 0, 'Zonozz - Intro Whisper'),
(55308, 2, 0, 'Zzof Shuul''wah. Thoq fssh N''Zoth!', 14, 0, 100, 0, 0, 26335, 73851, 0, 'Zonozz - Aggro'),
(55308, 3, 0, 'Victory for Deathwing. For the glory of N''Zoth!', 42, 0, 100, 0, 0, 0, 73852, 0, 'Zonozz - Aggro Whisper'),
(55308, 4, 0, 'Gul''kafh an''qov N''Zoth.', 14, 0, 100, 0, 0, 26340, 73853, 0, 'Zonozz - Void of the Unmaking'),
(55308, 5, 0, 'Gaze into the heart of N''Zoth.', 42, 0, 100, 0, 0, 0, 73854, 0, 'Zonozz - Void Whisper'),
(55308, 6, 0, 'N''Zoth ga zyqtahg iilth.', 14, 0, 100, 0, 0, 26345, 73855, 0, 'Zonozz - Black Blood'),
(55308, 7, 0, 'The will of N''Zoth corrupts you.', 42, 0, 100, 0, 0, 0, 73856, 0, 'Zonozz - Black Blood Whisper'),
(55308, 8, 0, 'Sk''shgn eqnizz hoq.', 14, 0, 100, 0, 0, 26342, 73857, 0, 'Zonozz - Shadows 1'),
(55308, 9, 0, 'Your fear drives me.', 42, 0, 100, 0, 0, 0, 73858, 0, 'Zonozz - Shadows 1 Whisper'),
(55308, 10, 0, 'Sk''magg yawifk hoq.', 14, 0, 100, 0, 0, 26343, 73859, 0, 'Zonozz - Shadows 2'),
(55308, 11, 0, 'Your suffering strengthens me.', 42, 0, 100, 0, 0, 0, 73860, 0, 'Zonozz - Shadows 2 Whisper'),
(55308, 12, 0, 'Sk''uuyat guulphg hoq.', 14, 0, 100, 0, 0, 26344, 73861, 0, 'Zonozz - Shadows 3'),
(55308, 13, 0, 'Your agony sustains me.', 42, 0, 100, 0, 0, 0, 73862, 0, 'Zonozz - Shadows 3 Whisper'),
(55308, 14, 0, 'Sk''tek agth nuq N''Zoth yyqzz.', 14, 0, 100, 0, 0, 26338, 73863, 0, 'Zonozz - Slay 1'),
(55308, 15, 0, 'Your skulls shall adorn N''Zoth''s writhing throne.', 42, 0, 100, 0, 0, 0, 73864, 0, 'Zonozz - Slay 1 Whisper'),
(55308, 16, 0, 'Sk''shuul agth vorzz N''Zoth naggwa''fssh.', 14, 0, 100, 0, 0, 26339, 73865, 0, 'Zonozz - Slay 2'),
(55308, 17, 0, 'Your deaths shall sing of N''Zoth''s unending glory.', 42, 0, 100, 0, 0, 0, 73866, 0, 'Zonozz - Slay 2 Whisper'),
(55308, 18, 0, 'Sk''yahf agth huqth N''Zoth qornaus.', 14, 0, 100, 0, 0, 26341, 73867, 0, 'Zonozz - Slay 3'),
(55308, 19, 0, 'Your souls shall sate N''Zoth''s endless hunger.', 42, 0, 100, 0, 0, 0, 73868, 0, 'Zonozz - Slay 3 Whisper'),
(55308, 20, 0, 'Uovssh thyzz... qwaz...', 14, 0, 100, 0, 0, 26336, 73869, 0, 'Zonozz - Death'),
(55308, 21, 0, 'To have waited so long... for this...', 42, 0, 100, 0, 0, 0, 73870, 0, 'Zonozz - Death Whisper');

-- Ping Pong Champion (achievement 6128, criteria 18494) -> instance script check
DELETE FROM `achievement_criteria_data` WHERE `criteria_id` = 18494 AND `type` = 18;
INSERT INTO `achievement_criteria_data` (`criteria_id`, `type`, `value1`, `value2`, `ScriptName`) VALUES
(18494, 18, 0, 0, '');

-- Void Diffusion (103527 + forks): the impact damage is split among everyone
-- packed around the collision (SPELL_ATTR0_CU_SHARE_DAMAGE = 0x8)
DELETE FROM `spell_custom_attr` WHERE `entry` IN (103527, 104605, 108345, 108346);
INSERT INTO `spell_custom_attr` (`entry`, `attributes`) VALUES
(103527, 8),
(104605, 8),
(108345, 8),
(108346, 8);

-- Spell script bindings (base spell + SpellDifficulty.dbc forks)
DELETE FROM `spell_script_names` WHERE `ScriptName` IN
('spell_zonozz_disrupting_shadows', 'spell_zonozz_disrupting_shadows_aura', 'spell_zonozz_disrupting_shadows_knockback');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(103434, 'spell_zonozz_disrupting_shadows'),
(104599, 'spell_zonozz_disrupting_shadows'),
(104600, 'spell_zonozz_disrupting_shadows'),
(104601, 'spell_zonozz_disrupting_shadows'),
(103434, 'spell_zonozz_disrupting_shadows_aura'),
(104599, 'spell_zonozz_disrupting_shadows_aura'),
(104600, 'spell_zonozz_disrupting_shadows_aura'),
(104601, 'spell_zonozz_disrupting_shadows_aura'),
(103948, 'spell_zonozz_disrupting_shadows_knockback'),
(108342, 'spell_zonozz_disrupting_shadows_knockback'),
(108343, 'spell_zonozz_disrupting_shadows_knockback'),
(108344, 'spell_zonozz_disrupting_shadows_knockback');

-- ---------------------------------------------------------------------------
-- Loot: split the mixed WDB table on 55308 into per-difficulty tables.
--   55308 (10N):  ilvl 397 + shared
--   55309 (25N):  ilvl 397 (LootMode 1), ilvl 384 LFR (LootMode 2), shared (LootMode 3)
--   55310 (10H):  ilvl 410 + shared
--   55311 (25H):  ilvl 410 + shared
-- Item classes resolved against Item-sparse.db2 (4.3.4 client).
-- The base table is rebuilt from the original TDB 434.22011 rows first so
-- this update stays idempotent.
-- ---------------------------------------------------------------------------
DELETE FROM `creature_loot_template` WHERE `Entry` IN (55308, 55309, 55310, 55311);
INSERT INTO `creature_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`) VALUES
(55308,71998,0,78.051,0,0,1,0,1,3,NULL),
(55308,77204,0,22.8777,0,0,1,0,1,1,NULL),
(55308,77207,0,1.228,0,0,1,0,1,1,NULL),
(55308,77208,0,1.1272,0,0,1,0,1,1,NULL),
(55308,77209,0,1.1456,0,0,1,0,1,1,NULL),
(55308,77210,0,1.1395,0,0,1,0,1,1,NULL),
(55308,77211,0,1.0295,0,0,1,0,1,1,NULL),
(55308,77215,0,17.2018,0,0,1,0,1,1,NULL),
(55308,77216,0,15.0848,0,0,1,0,1,1,NULL),
(55308,77228,0,1.1639,0,0,1,0,1,1,NULL),
(55308,77229,0,1.1059,0,0,1,0,1,1,NULL),
(55308,77230,0,1.1242,0,0,1,0,1,1,NULL),
(55308,77231,0,1.1914,0,0,1,0,1,1,NULL),
(55308,77232,0,1.1211,0,0,1,0,1,1,NULL),
(55308,77255,0,17.5164,0,0,1,0,1,1,NULL),
(55308,77257,0,11.6389,0,0,1,0,1,1,NULL),
(55308,77258,0,15.1275,0,0,1,0,1,1,NULL),
(55308,77259,0,16.7313,0,0,1,0,1,1,NULL),
(55308,77260,0,17.049,0,0,1,0,1,1,NULL),
(55308,77952,0,89.4669,0,0,1,0,1,3,NULL),
(55308,77969,0,31.9016,0,0,1,0,1,1,NULL),
(55308,77979,0,1.4358,0,0,1,0,1,1,NULL),
(55308,77980,0,1.8329,0,0,1,0,1,1,NULL),
(55308,77981,0,1.4633,0,0,1,0,1,1,NULL),
(55308,77982,0,1.4999,0,0,1,0,1,1,NULL),
(55308,77983,0,1.8024,0,0,1,0,1,1,NULL),
(55308,77989,0,6.2166,0,0,1,0,1,1,NULL),
(55308,77999,0,0.333,0,0,1,0,1,1,NULL),
(55308,78000,0,0.3177,0,0,1,0,1,1,NULL),
(55308,78001,0,0.3452,0,0,1,0,1,1,NULL),
(55308,78002,0,0.3146,0,0,1,0,1,1,NULL),
(55308,78003,0,0.2566,0,0,1,0,1,1,NULL),
(55308,78173,0,49.8641,0,0,1,0,1,1,NULL),
(55308,78178,0,41.8604,0,0,1,0,1,1,NULL),
(55308,78183,0,41.7321,0,0,1,0,1,1,NULL),
(55308,78387,0,4.4478,0,0,1,0,1,1,NULL),
(55308,78388,0,4.402,0,0,1,0,1,1,NULL),
(55308,78389,0,4.4142,0,0,1,0,1,1,NULL),
(55308,78390,0,3.8155,0,0,1,0,1,1,NULL),
(55308,78391,0,4.4203,0,0,1,0,1,1,NULL),
(55308,78392,0,3.8827,0,0,1,0,1,1,NULL),
(55308,78393,0,2.8654,0,0,1,0,1,1,NULL),
(55308,78395,0,28.6177,0,0,1,0,1,1,NULL),
(55308,78396,0,27.3133,0,0,1,0,1,1,NULL),
(55308,78397,0,17.8647,0,0,1,0,1,1,NULL),
(55308,78398,0,21.5274,0,0,1,0,1,1,NULL),
(55308,78399,0,17.9319,0,0,1,0,1,1,NULL),
(55308,78400,0,13.4901,0,0,1,0,1,1,NULL),
(55308,78489,0,0.2872,0,0,1,0,1,1,NULL),
(55308,78490,0,0.2933,0,0,1,0,1,1,NULL),
(55308,78491,0,0.2688,0,0,1,0,1,1,NULL),
(55308,78492,0,0.3513,0,0,1,0,1,1,NULL),
(55308,78493,0,0.3208,0,0,1,0,1,1,NULL),
(55308,78494,0,1.393,0,0,1,0,1,1,NULL),
(55308,78495,0,1.6557,0,0,1,0,1,1,NULL),
(55308,78496,0,1.6527,0,0,1,0,1,1,NULL),
(55308,78497,0,1.5824,0,0,1,0,1,1,NULL),
(55308,78498,0,1.5641,0,0,1,0,1,1,NULL),
(55308,78853,0,8.7613,0,0,1,0,1,1,NULL),
(55308,78854,0,9.2409,0,0,1,0,1,1,NULL),
(55308,78855,0,10.6889,0,0,1,0,1,1,NULL),
(55308,78862,0,0.5804,0,0,1,0,1,1,NULL),
(55308,78863,0,0.5377,0,0,1,0,1,1,NULL),
(55308,78864,0,0.5254,0,0,1,0,1,1,NULL),
(55308,78865,0,57.5347,0,0,1,0,1,1,NULL),
(55308,78866,0,45.2665,0,0,1,0,1,1,NULL),
(55308,78867,0,50.1176,0,0,1,0,1,1,NULL),
(55308,78868,0,0.5713,0,0,1,0,1,1,NULL),
(55308,78869,0,0.669,0,0,1,0,1,1,NULL),
(55308,78870,0,0.4582,0,0,1,0,1,1,NULL),
(55308,78871,0,0.7148,0,0,1,0,1,1,NULL),
(55308,78872,0,0.5499,0,0,1,0,1,1,NULL),
(55308,78873,0,0.5926,0,0,1,0,1,1,NULL),
(55308,78874,0,0.7881,0,0,1,0,1,1,NULL),
(55308,78875,0,0.7484,0,0,1,0,1,1,NULL),
(55308,78876,0,0.504,0,0,1,0,1,1,NULL);

-- ilvl buckets from Item-sparse.db2 (71998 Essence of Destruction and
-- 77952 gem drop stay shared across all modes)
SET @NORMAL_ITEMS := '77204,77207,77208,77209,77210,77211,77215,77216,77228,77229,77230,77231,77232,77255,77257,77258,77259,77260,78173,78178,78183';
SET @LFR_ITEMS    := '77969,77979,77980,77981,77982,77983,78395,78396,78397,78398,78399,78400,78494,78495,78496,78497,78498,78862,78863,78864,78865,78866,78867,78868,78869,78870,78871,78872,78873,78874,78875,78876';
SET @HEROIC_ITEMS := '77989,77999,78000,78001,78002,78003,78387,78388,78389,78390,78391,78392,78393,78489,78490,78491,78492,78493,78853,78854,78855';
SET @SHARED_ITEMS := '71998,77952';

-- 25 Normal: 397 loot
INSERT INTO `creature_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT 55309, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, 1, `GroupId`, `MinCount`, `MaxCount`, 'Zonozz 25N'
FROM `creature_loot_template` WHERE `Entry` = 55308 AND FIND_IN_SET(`Item`, @NORMAL_ITEMS);

-- 25 Normal table: LFR loot as LootMode 2
INSERT INTO `creature_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT 55309, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, 2, `GroupId`, `MinCount`, `MaxCount`, 'Zonozz LFR'
FROM `creature_loot_template` WHERE `Entry` = 55308 AND FIND_IN_SET(`Item`, @LFR_ITEMS);

-- 25 Normal table: shared drops available in both loot modes
INSERT INTO `creature_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT 55309, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, 3, `GroupId`, `MinCount`, `MaxCount`, 'Zonozz shared'
FROM `creature_loot_template` WHERE `Entry` = 55308 AND FIND_IN_SET(`Item`, @SHARED_ITEMS);

-- Heroic tables
INSERT INTO `creature_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT 55310, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, 1, `GroupId`, `MinCount`, `MaxCount`, 'Zonozz 10H'
FROM `creature_loot_template` WHERE `Entry` = 55308 AND (FIND_IN_SET(`Item`, @HEROIC_ITEMS) OR FIND_IN_SET(`Item`, @SHARED_ITEMS));

INSERT INTO `creature_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT 55311, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, 1, `GroupId`, `MinCount`, `MaxCount`, 'Zonozz 25H'
FROM `creature_loot_template` WHERE `Entry` = 55308 AND (FIND_IN_SET(`Item`, @HEROIC_ITEMS) OR FIND_IN_SET(`Item`, @SHARED_ITEMS));

-- 10 Normal keeps only 397 + shared
DELETE FROM `creature_loot_template` WHERE `Entry` = 55308 AND (FIND_IN_SET(`Item`, @LFR_ITEMS) OR FIND_IN_SET(`Item`, @HEROIC_ITEMS));

UPDATE `creature_template` SET `lootid` = `entry` WHERE `entry` IN (55309, 55310, 55311);
