-- Hagara the Stormbinder (Dragon Soul) - encounter implementation
-- Boss 55689 (10N) with difficulty entries 57462 (25N) / 57955 (10H) / 57956 (25H)
-- Phase objects (shared across modes unless chained):
--   Ice Tomb 55695, Ice Wave 56104, Ice Lance 56108,
--   Frozen Binding Crystal 56136 -> 57813 / 57832 / 57833,
--   Crystal Conductor 56165, Bound Lightning Elemental 56700 -> 57463 / 58250 / 58251,
--   Collapsing Icicle 57867
-- Intro assault event adds (10-man / 25-man template pairs):
--   Twilight Frost Evoker 57807/57808, Stormborn Myrmidon 57817/57818,
--   Stormbinder Adept 57823/57824, Corrupted Fragment 57819/57820,
--   Twilight Portal 57809
-- Teleporters: Travel to the Eye of Eternity 57377 (spellclick 106094),
--              Travel to Wyrmrest Base 57882 (spellclick fixed to 108202)
--
-- SpellDifficulty.dbc forks (client-side, no spelldifficulty_dbc rows needed):
--   107851 Focused Assault    -> 110900 / 110899 / 110898
--   105289 Shattered Ice      -> 108567 / 110888 / 110887
--   105465 Lightning Storm    -> 108568 / 110893 / 110892
--   105369 Lightning Conduit  -> 108569 / 109201 / 109202
--   105256 Frozen Tempest     -> 109552 / 109553 / 109554
--   105409 Water Shield       -> 109560 / 109561 / 109562
--   105316 Ice Lance (impact) -> 107061 / 107062 / 107063
--   109563 Storm Pillar (dmg) -> 109564 / 109565 / 109566

-- Script bindings (base entries only - the core ignores ScriptName on difficulty entries)
UPDATE `creature_template` SET `ScriptName` = 'boss_hagara' WHERE `entry` = 55689;
UPDATE `creature_template` SET `ScriptName` = 'npc_hagara_ice_tomb' WHERE `entry` = 55695;
UPDATE `creature_template` SET `ScriptName` = 'npc_hagara_ice_wave' WHERE `entry` = 56104;
UPDATE `creature_template` SET `ScriptName` = 'npc_hagara_ice_lance' WHERE `entry` = 56108;
UPDATE `creature_template` SET `ScriptName` = 'npc_hagara_binding_crystal' WHERE `entry` = 56136;
UPDATE `creature_template` SET `ScriptName` = 'npc_hagara_crystal_conductor' WHERE `entry` = 56165;
UPDATE `creature_template` SET `ScriptName` = 'npc_hagara_bound_lightning_elemental' WHERE `entry` = 56700;
UPDATE `creature_template` SET `ScriptName` = 'npc_hagara_collapsing_icicle' WHERE `entry` = 57867;
UPDATE `creature_template` SET `ScriptName` = 'npc_hagara_intro_add' WHERE `entry` IN (57807, 57808, 57817, 57818, 57823, 57824, 57819, 57820);

-- Health (user-supplied retail values, base health 85,892 at level 88 / 77,490 at level 85).
-- Everything else already matches retail; only the 10H slots were wrong.
--   Hagara 10H 57955:  420 -> 36,074,640 (was 600)
--   Frozen Binding Crystal 10H 57832: 21 -> 1,627,290 (was 30)
--   Bound Lightning Elemental 10H 58250: 14 -> 1,084,860 (was 20)
UPDATE `creature_template` SET `HealthModifier` = 420 WHERE `entry` = 57955;
UPDATE `creature_template` SET `HealthModifier` = 21 WHERE `entry` = 57832;
UPDATE `creature_template` SET `HealthModifier` = 14 WHERE `entry` = 58250;

-- The boss chain ships with WDB flags (not selectable / immune); the script
-- manages her intro flags itself.
UPDATE `creature_template` SET `unit_flags` = 0 WHERE `entry` IN (55689, 57462, 57955, 57956);

-- Full boss CC immunity mask (fork convention) for the boss chain and all
-- phase objects
UPDATE `creature_template` SET `mechanic_immune_mask` = 650854271 WHERE `entry` IN
(55689, 57462, 57955, 57956,
 55695,
 56104, 56108,
 56136, 57813, 57832, 57833,
 56165,
 56700, 57463, 58250, 58251,
 57867);

-- Ice Tomb must be attackable/killable (shipped with the NO_COMBAT extra flag,
-- same WDB bug as Yor'sahj's Mana Void)
UPDATE `creature_template` SET `flags_extra` = `flags_extra` & ~8192 WHERE `entry` = 55695;

-- 25-man intro add templates are level-1 faction-35 WDB stubs; mirror the
-- 10-man base entries (models are already correct)
UPDATE `creature_template` SET `minlevel` = 85, `maxlevel` = 85, `exp` = 3, `faction` = 14, `unit_class` = 2, `unit_flags2` = 33556480 WHERE `entry` IN (57808, 57818, 57824, 57820);

-- Crystal Conductor shipped with two models at equal probability, one of them
-- the invisible trigger model - the pillar would randomly be unseeable
DELETE FROM `creature_template_model` WHERE `CreatureID` = 56165;
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `Probability`, `VerifiedBuild`) VALUES
(56165, 0, 39575, 1, 0);

-- Raid Finder stats templates (health only): 70% of the 25N value, user-approved
--   58242 Hagara:                   85,892 (L88) * 840  = 72,149,280
--   58243 Frozen Binding Crystal:   77,490 (L85) * 10.5 =    813,645
--   58244 Bound Lightning Elemental: 77,490 (L85) * 31.5 =  2,440,935
DELETE FROM `creature_template` WHERE `entry` IN (58242, 58243, 58244);
INSERT INTO `creature_template` (`entry`, `name`, `femaleName`, `subname`, `minlevel`, `maxlevel`, `exp`, `faction`, `unit_class`, `type`, `HealthModifier`, `AIName`, `ScriptName`, `VerifiedBuild`) VALUES
(58242, 'Hagara the Stormbinder', '', 'LFR Stats', 88, 88, 3, 14, 2, 7, 840, '', '', 0),
(58243, 'Frozen Binding Crystal', '', 'LFR Stats', 85, 85, 3, 14, 1, 10, 10.5, '', '', 0),
(58244, 'Bound Lightning Elemental', '', 'LFR Stats', 85, 85, 3, 14, 2, 4, 31.5, '', '', 0);

DELETE FROM `creature_template_model` WHERE `CreatureID` IN (58242, 58243, 58244);
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `Probability`, `VerifiedBuild`) VALUES
(58242, 0, 39318, 1, 0),
(58243, 0, 40058, 1, 0),
(58244, 0, 34548, 1, 0);

-- Spawns (map 967, all four raid spawn modes; positions from retail sniffs).
-- Hagara waits on her perch above the platform until the intro event completes.
-- The four Crystal Conductors are permanent fixtures on the platform cardinals.
-- Teleporter NPC positions are first-pass (exact retail spots unknown) - tune
-- during the walkthrough.
DELETE FROM `creature` WHERE `guid` BETWEEN 9000637 AND 9000643;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `VerifiedBuild`) VALUES
(9000637, 55689, 967, 0, 0, 15, 0, 0, 0, 0, 13549.96, 13613.247, 134.327, 0, 604800, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(9000638, 56165, 967, 0, 0, 15, 0, 0, 0, 0, 13587.28, 13658.63, 123.567, 4.66003, 604800, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(9000639, 56165, 967, 0, 0, 15, 0, 0, 0, 0, 13633.02, 13612.09, 123.567, 3.14159, 604800, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(9000640, 56165, 967, 0, 0, 15, 0, 0, 0, 0, 13587.39, 13566.77, 123.567, 1.48353, 604800, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(9000641, 56165, 967, 0, 0, 15, 0, 0, 0, 0, 13541.83, 13611.32, 123.567, 0, 604800, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(9000642, 57377, 967, 0, 0, 15, 0, 0, 0, 0, -1794.6, -2392.03, 45.6201, 3.665, 300, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(9000643, 57882, 967, 0, 0, 15, 0, 0, 0, 0, 13627.4, 13606.0, 123.52, 2.63545, 300, 0, 0, 0, 0, 0, 0, 0, 0, 0);

-- The Focusing Iris (platform center visual)
DELETE FROM `gameobject` WHERE `guid` = 9000508;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `VerifiedBuild`) VALUES
(9000508, 210132, 967, 0, 0, 15, 0, 1, 0, 0, -1, 13587.37, 13611.96, 122.42, 0, 0, 0, 0, 1, 604800, 0, 1, '', 0);

-- The return teleporter shipped with a bogus spellclick (90102 is the Squirrel
-- Scrubber vehicle aura); 108202 "Teleport Single - To Wyrmrest Base" is the
-- real teleport and already has its spell_target_position row.
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 57882;
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(57882, 108202, 1, 0);

-- Hagara is the final boss of LFR wing 1 - "The Siege of Wyrmrest Temple"
-- (LFGDungeons 416); Madness (447) already uses the same hook for wing 2.
UPDATE `instance_encounters` SET `lastEncounterDungeon` = 416 WHERE `entry` = 1296;

-- Texts. All broadcast_text rows (55646-55674, 55715, 56614-56617, 57084) ship
-- with the client VO sounds (VO_DS_HAGARA_*); only the creature_text bindings
-- are missing. The VO-to-event mapping follows the sound file names
-- (INTRO/ADDS/CIRCUIT/CRYSTALDEAD/FROSTRAY/GLACIER/TRAP) - verify in the
-- walkthrough.
DELETE FROM `creature_text` WHERE `CreatureID` = 55689;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(55689, 0, 0, 'Even with the Aspect of Time on your side, you stumble foolishly into a trap?', 14, 0, 100, 0, 0, 26223, 56614, 0, 'Hagara - Intro Event Start'),
(55689, 1, 0, 'Don''t preen just yet, little pups. We''ll cleanse this world of your kind.', 14, 0, 100, 0, 0, 26224, 56615, 0, 'Hagara - Intro Wave 1'),
(55689, 1, 1, 'You''ll not leave this place alive!', 14, 0, 100, 0, 0, 26225, 56616, 0, 'Hagara - Intro Wave 2'),
(55689, 2, 0, 'Not one of you will live to see the final cataclysm! Finish them!', 14, 0, 100, 0, 0, 26226, 56617, 0, 'Hagara - Intro Final Wave'),
(55689, 3, 0, 'Swagger all you like; you pups don''t stand a chance. Flee now, while you can.', 14, 0, 100, 0, 0, 26251, 55646, 0, 'Hagara - Intro Complete'),
(55689, 4, 0, 'You cross the Stormbinder! I''ll slaughter you all.', 14, 0, 100, 0, 0, 26227, 55647, 0, 'Hagara - Aggro'),
(55689, 5, 0, 'Stay, pup.', 14, 0, 100, 0, 0, 26249, 55652, 0, 'Hagara - Ice Tomb 1'),
(55689, 5, 1, 'Hold still.', 14, 0, 100, 0, 0, 26250, 55653, 0, 'Hagara - Ice Tomb 2'),
(55689, 6, 0, 'You can''t outrun the storm.', 14, 0, 100, 0, 0, 26247, 55654, 0, 'Hagara - Ice Phase 1'),
(55689, 6, 1, 'Die beneath the ice.', 14, 0, 100, 0, 0, 26248, 55655, 0, 'Hagara - Ice Phase 2'),
(55689, 7, 0, 'Suffer the storm''s wrath!', 14, 0, 100, 0, 0, 26252, 55656, 0, 'Hagara - Lightning Phase 1'),
(55689, 7, 1, 'Thunder and lightning dance at my call!', 14, 0, 100, 0, 0, 26253, 55657, 0, 'Hagara - Lightning Phase 2'),
(55689, 8, 0, 'You face more than my axes, this close.', 14, 0, 100, 0, 0, 26244, 55658, 0, 'Hagara - Focused Assault 1'),
(55689, 8, 1, 'See what becomes of those who stand before me!', 14, 0, 100, 0, 0, 26245, 55659, 0, 'Hagara - Focused Assault 2'),
(55689, 8, 2, 'Feel a chill up your spine...?', 14, 0, 100, 0, 0, 26246, 55660, 0, 'Hagara - Focused Assault 3'),
(55689, 9, 0, 'The time I spent binding that, WASTED!', 14, 0, 100, 0, 0, 26235, 55661, 0, 'Hagara - Crystal Died 1'),
(55689, 9, 1, 'Enough!', 14, 0, 100, 0, 0, 26237, 55662, 0, 'Hagara - Crystal Died 2'),
(55689, 9, 2, 'You''ll PAY for that.', 14, 0, 100, 0, 0, 26236, 55663, 0, 'Hagara - Crystal Died 3'),
(55689, 9, 3, 'Aughhhh!', 14, 0, 100, 0, 0, 26238, 55664, 0, 'Hagara - Crystal Died 4'),
(55689, 9, 4, 'Again?!', 14, 0, 100, 0, 0, 26239, 55665, 0, 'Hagara - Crystal Died 5'),
(55689, 9, 5, 'Impudent pup!', 14, 0, 100, 0, 0, 26240, 55666, 0, 'Hagara - Crystal Died 6'),
(55689, 9, 6, 'Get away from that, mongrel!', 14, 0, 100, 0, 0, 26235, 57084, 0, 'Hagara - Crystal Died 7'),
(55689, 10, 0, 'The one remaining is still enough to finish you.', 14, 0, 100, 0, 0, 26241, 55667, 0, 'Hagara - One Crystal Left'),
(55689, 11, 0, 'What are you doing?', 14, 0, 100, 0, 0, 26228, 55668, 0, 'Hagara - Conductor Charged 1'),
(55689, 11, 1, 'You''re toying with death.', 14, 0, 100, 0, 0, 26229, 55669, 0, 'Hagara - Conductor Charged 2'),
(55689, 11, 2, 'You think you can play with my lightning?', 14, 0, 100, 0, 0, 26230, 55670, 0, 'Hagara - Conductor Charged 3'),
(55689, 11, 3, 'Impossible!', 14, 0, 100, 0, 0, 26231, 55671, 0, 'Hagara - Conductor Charged 4'),
(55689, 12, 0, 'No! More... lightning...', 14, 0, 100, 0, 0, 26233, 55672, 0, 'Hagara - Lightning Overload 1'),
(55689, 12, 1, 'Enough of your games! You won''t live to do it again.', 14, 0, 100, 0, 0, 26232, 55673, 0, 'Hagara - Lightning Overload 2'),
(55689, 13, 0, 'I''ll finish you now, pups!', 14, 0, 100, 0, 0, 26234, 55674, 0, 'Hagara - Feedback End'),
(55689, 14, 0, 'You should have run, dog.', 14, 0, 100, 0, 0, 26255, 55648, 0, 'Hagara - Slay 1'),
(55689, 14, 1, 'Feh!', 14, 0, 100, 0, 0, 26254, 55649, 0, 'Hagara - Slay 2'),
(55689, 14, 2, 'Down, pup.', 14, 0, 100, 0, 0, 26256, 55650, 0, 'Hagara - Slay 3'),
(55689, 14, 3, 'A waste of my time.', 14, 0, 100, 0, 0, 26257, 55651, 0, 'Hagara - Slay 4'),
(55689, 15, 0, 'Cowards! You pack of weakling... dogs...', 14, 0, 100, 0, 0, 26243, 55715, 0, 'Hagara - Death');

-- Holding Hands (achievement 6175, criteria 18608, asset = serverside spell
-- 110520). The boss casts 110520 on the raid when a lightning phase completes
-- with an unbroken conduit chain; the instance script gates the criteria.
-- Spell shape copied from 111533, the Madness of Deathwing achievement spell.
DELETE FROM `spell_dbc` WHERE `Id` = 110520;
INSERT INTO `spell_dbc` (`Id`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `AttributesEx4`, `AttributesEx5`, `AttributesEx6`, `AttributesEx7`, `AttributesEx8`, `AttributesEx9`, `AttributesEx10`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `SchoolMask`, `SpellAuraOptionsId`, `SpellCastingRequirementsId`, `SpellCategoriesId`, `SpellClassOptionsId`, `SpellEquippedItemsId`, `SpellInterruptsId`, `SpellLevelsId`, `SpellTargetRestrictionsId`, `SpellName`) VALUES
(110520, 2843738368, 268436512, 540677, 269943552, 128, 393225, 5120, 33554432, 32, 0, 0, 0, 36, 13, 0, 38, 0, 0, 0, 0, 0, 0, 0, '(Serverside/Non-DB2) Holding Hands Achievement Credit <Do Not Translate>');

DELETE FROM `spelleffect_dbc` WHERE `SpellID` = 110520;
INSERT INTO `spelleffect_dbc` (`Id`, `Effect`, `EffectAmplitude`, `EffectAura`, `EffectAuraPeriod`, `EffectBasePoints`, `EffectBonusCoefficient`, `EffectChainAmplitude`, `EffectChainTargets`, `EffectDieSides`, `EffectItemType`, `EffectMechanic`, `EffectMiscValue`, `EffectMiscValueB`, `EffectPointsPerResource`, `EffectRadiusIndex`, `EffectRadiusMaxIndex`, `EffectRealPointsPerLevel`, `EffectSpellClassMaskA`, `EffectSpellClassMaskB`, `EffectSpellClassMaskC`, `EffectTriggerSpell`, `EffectImplicitTargetA`, `EffectImplicitTargetB`, `SpellID`, `EffectIndex`, `Comment`) VALUES
(160116, 6, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 28, 0, 0, 0, 0, 0, 25, 0, 110520, 0, 'Hagara - Holding Hands Achievement Spell');

-- Frostflake Snare stand-in: 109337 is an effect-179 serverside areatrigger the
-- 4.3.4 core cannot execute. The patch is scripted geometrically; this custom
-- serverside aura carries the -50% snare it applies (short duration, refreshed
-- while standing in the patch).
DELETE FROM `spell_dbc` WHERE `Id` = 123457;
INSERT INTO `spell_dbc` (`Id`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `AttributesEx4`, `AttributesEx5`, `AttributesEx6`, `AttributesEx7`, `AttributesEx8`, `AttributesEx9`, `AttributesEx10`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `SchoolMask`, `SpellAuraOptionsId`, `SpellCastingRequirementsId`, `SpellCategoriesId`, `SpellClassOptionsId`, `SpellEquippedItemsId`, `SpellInterruptsId`, `SpellLevelsId`, `SpellTargetRestrictionsId`, `SpellName`) VALUES
(123457, 2843738368, 268436512, 540677, 269943552, 128, 393225, 5120, 33554432, 32, 0, 0, 0, 36, 13, 16, 38, 0, 0, 0, 0, 0, 0, 0, '(Serverside/Non-DB2) Frostflake Snare Slow <Do Not Translate>');

DELETE FROM `spelleffect_dbc` WHERE `SpellID` = 123457;
INSERT INTO `spelleffect_dbc` (`Id`, `Effect`, `EffectAmplitude`, `EffectAura`, `EffectAuraPeriod`, `EffectBasePoints`, `EffectBonusCoefficient`, `EffectChainAmplitude`, `EffectChainTargets`, `EffectDieSides`, `EffectItemType`, `EffectMechanic`, `EffectMiscValue`, `EffectMiscValueB`, `EffectPointsPerResource`, `EffectRadiusIndex`, `EffectRadiusMaxIndex`, `EffectRealPointsPerLevel`, `EffectSpellClassMaskA`, `EffectSpellClassMaskB`, `EffectSpellClassMaskC`, `EffectTriggerSpell`, `EffectImplicitTargetA`, `EffectImplicitTargetB`, `SpellID`, `EffectIndex`, `Comment`) VALUES
(160117, 6, 0, 33, 0, -50, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 25, 0, 123457, 0, 'Hagara - Frostflake Snare Slow');

DELETE FROM `achievement_criteria_data` WHERE `criteria_id` = 18608 AND `type` = 18;
INSERT INTO `achievement_criteria_data` (`criteria_id`, `type`, `value1`, `value2`, `ScriptName`) VALUES
(18608, 18, 0, 0, '');

-- Spell script bindings (base spell + SpellDifficulty.dbc forks)
DELETE FROM `spell_script_names` WHERE `ScriptName` IN
('spell_hagara_focused_assault', 'spell_hagara_focused_assault_strike', 'spell_hagara_shattered_ice',
 'spell_hagara_ice_tomb', 'spell_hagara_ice_lance', 'spell_hagara_frostflake',
 'spell_hagara_lightning_conduit', 'spell_hagara_lightning_storm', 'spell_hagara_ice_wave',
 'spell_hagara_icicle');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(107850, 'spell_hagara_focused_assault_strike'),
(107851, 'spell_hagara_focused_assault'),
(110900, 'spell_hagara_focused_assault'),
(110899, 'spell_hagara_focused_assault'),
(110898, 'spell_hagara_focused_assault'),
(105289, 'spell_hagara_shattered_ice'),
(108567, 'spell_hagara_shattered_ice'),
(110888, 'spell_hagara_shattered_ice'),
(110887, 'spell_hagara_shattered_ice'),
(104448, 'spell_hagara_ice_tomb'),
(105316, 'spell_hagara_ice_lance'),
(107061, 'spell_hagara_ice_lance'),
(107062, 'spell_hagara_ice_lance'),
(107063, 'spell_hagara_ice_lance'),
(109325, 'spell_hagara_frostflake'),
(105369, 'spell_hagara_lightning_conduit'),
(108569, 'spell_hagara_lightning_conduit'),
(109201, 'spell_hagara_lightning_conduit'),
(109202, 'spell_hagara_lightning_conduit'),
(105465, 'spell_hagara_lightning_storm'),
(108568, 'spell_hagara_lightning_storm'),
(110893, 'spell_hagara_lightning_storm'),
(110892, 'spell_hagara_lightning_storm'),
(105314, 'spell_hagara_ice_wave'),
(69425, 'spell_hagara_icicle');

-- ---------------------------------------------------------------------------
-- Loot: split the mixed WDB table on 55689 into per-difficulty tables.
--   55689 (10N):  ilvl 397 + shared
--   57462 (25N):  ilvl 397 (LootMode 1), ilvl 384 LFR (LootMode 2), shared (LootMode 3)
--   57955 (10H):  ilvl 410 + shared
--   57956 (25H):  ilvl 410 + shared
-- Item classes resolved against Item-sparse.db2 (4.3.4 client).
-- The base table is rebuilt from the original TDB rows first so this update
-- stays idempotent.
-- ---------------------------------------------------------------------------
DELETE FROM `creature_loot_template` WHERE `Entry` IN (55689, 57462, 57955, 57956);
INSERT INTO `creature_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`) VALUES
(55689,52078,0,38.8417,0,0,1,0,1,1,NULL),
(55689,71998,0,93.8786,0,0,1,0,1,3,NULL),
(55689,77207,0,1.1833,0,0,1,0,1,1,NULL),
(55689,77208,0,1.1868,0,0,1,0,1,1,NULL),
(55689,77209,0,1.4505,0,0,1,0,1,1,NULL),
(55689,77210,0,1.166,0,0,1,0,1,1,NULL),
(55689,77211,0,1.3325,0,0,1,0,1,1,NULL),
(55689,77220,0,15.6991,0,0,1,0,1,1,NULL),
(55689,77221,0,22.7817,0,0,1,0,1,1,NULL),
(55689,77228,0,1.1,0,0,1,0,1,1,NULL),
(55689,77229,0,1.3742,0,0,1,0,1,1,NULL),
(55689,77230,0,1.2909,0,0,1,0,1,1,NULL),
(55689,77231,0,1.3083,0,0,1,0,1,1,NULL),
(55689,77232,0,1.3985,0,0,1,0,1,1,NULL),
(55689,77248,0,12.4406,0,0,1,0,1,1,NULL),
(55689,77249,0,19.3566,0,0,1,0,1,1,NULL),
(55689,77250,0,19.5301,0,0,1,0,1,1,NULL),
(55689,77251,0,19.2664,0,0,1,0,1,1,NULL),
(55689,77952,0,100,0,0,1,0,1,3,NULL),
(55689,77979,0,1.1521,0,0,1,0,1,1,NULL),
(55689,77980,0,0.93,0,0,1,0,1,1,NULL),
(55689,77981,0,0.6767,0,0,1,0,1,1,NULL),
(55689,77982,0,0.8988,0,0,1,0,1,1,NULL),
(55689,77983,0,0.8675,0,0,1,0,1,1,NULL),
(55689,77999,0,0.4303,0,0,1,0,1,1,NULL),
(55689,78000,0,0.3401,0,0,1,0,1,1,NULL),
(55689,78001,0,0.3782,0,0,1,0,1,1,NULL),
(55689,78002,0,0.3713,0,0,1,0,1,1,NULL),
(55689,78003,0,0.354,0,0,1,0,1,1,NULL),
(55689,78011,0,25.6966,0,0,1,0,1,1,NULL),
(55689,78012,0,25.3947,0,0,1,0,1,1,NULL),
(55689,78170,0,61.2763,0,0,1,0,1,1,NULL),
(55689,78175,0,50.0642,0,0,1,0,1,1,NULL),
(55689,78180,0,50.7027,0,0,1,0,1,1,NULL),
(55689,78413,0,6.9785,0,0,1,0,1,1,NULL),
(55689,78414,0,4.9346,0,0,1,0,1,1,NULL),
(55689,78415,0,5.4968,0,0,1,0,1,1,NULL),
(55689,78416,0,5.5627,0,0,1,0,1,1,NULL),
(55689,78417,0,5.8195,0,0,1,0,1,1,NULL),
(55689,78418,0,6.399,0,0,1,0,1,1,NULL),
(55689,78419,0,7.5407,0,0,1,0,1,1,NULL),
(55689,78420,0,3.6506,0,0,1,0,1,1,NULL),
(55689,78421,0,18.6487,0,0,1,0,1,1,NULL),
(55689,78422,0,12.7737,0,0,1,0,1,1,NULL),
(55689,78423,0,15.1022,0,0,1,0,1,1,NULL),
(55689,78424,0,13.0895,0,0,1,0,1,1,NULL),
(55689,78425,0,15.8934,0,0,1,0,1,1,NULL),
(55689,78427,0,19.7731,0,0,1,0,1,1,NULL),
(55689,78428,0,9.3278,0,0,1,0,1,1,NULL),
(55689,78489,0,0.2984,0,0,1,0,1,1,NULL),
(55689,78490,0,0.3054,0,0,1,0,1,1,NULL),
(55689,78491,0,0.3193,0,0,1,0,1,1,NULL),
(55689,78492,0,0.413,0,0,1,0,1,1,NULL),
(55689,78493,0,0.406,0,0,1,0,1,1,NULL),
(55689,78494,0,0.7114,0,0,1,0,1,1,NULL),
(55689,78495,0,0.7877,0,0,1,0,1,1,NULL),
(55689,78496,0,0.9127,0,0,1,0,1,1,NULL),
(55689,78497,0,1.159,0,0,1,0,1,1,NULL),
(55689,78498,0,0.9647,0,0,1,0,1,1,NULL),
(55689,78859,0,11.1254,0,0,1,0,1,1,NULL),
(55689,78860,0,10.3342,0,0,1,0,1,1,NULL),
(55689,78861,0,13.1311,0,0,1,0,1,1,NULL),
(55689,78862,0,1.3846,0,0,1,0,1,1,NULL),
(55689,78863,0,1.0306,0,0,1,0,1,1,NULL),
(55689,78864,0,0.9508,0,0,1,0,1,1,NULL),
(55689,78865,0,1.1139,0,0,1,0,1,1,NULL),
(55689,78866,0,0.9751,0,0,1,0,1,1,NULL),
(55689,78867,0,1.107,0,0,1,0,1,1,NULL),
(55689,78868,0,1.2389,0,0,1,0,1,1,NULL),
(55689,78869,0,1.1347,0,0,1,0,1,1,NULL),
(55689,78870,0,0.7391,0,0,1,0,1,1,NULL),
(55689,78871,0,1.2284,0,0,1,0,1,1,NULL),
(55689,78872,0,1.0064,0,0,1,0,1,1,NULL),
(55689,78873,0,0.8675,0,0,1,0,1,1,NULL),
(55689,78874,0,42.055,0,0,1,0,1,1,NULL),
(55689,78875,0,36.8116,0,0,1,0,1,1,NULL),
(55689,78876,0,32.2969,0,0,1,0,1,1,NULL);

-- ilvl buckets from Item-sparse.db2 (52078 Maelstrom Crystal, 71998 Essence of
-- Destruction and the 77952 gem drop stay shared across all modes)
SET @NORMAL_ITEMS := '77207,77208,77209,77210,77211,77220,77221,77228,77229,77230,77231,77232,77248,77249,77250,77251,78011,78012,78170,78175,78180';
SET @LFR_ITEMS    := '77979,77980,77981,77982,77983,78421,78422,78423,78424,78425,78427,78428,78494,78495,78496,78497,78498,78862,78863,78864,78865,78866,78867,78868,78869,78870,78871,78872,78873,78874,78875,78876';
SET @HEROIC_ITEMS := '77999,78000,78001,78002,78003,78413,78414,78415,78416,78417,78418,78419,78420,78489,78490,78491,78492,78493,78859,78860,78861';
SET @SHARED_ITEMS := '52078,71998,77952';

-- 25 Normal: 397 loot
INSERT INTO `creature_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT 57462, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, 1, `GroupId`, `MinCount`, `MaxCount`, 'Hagara 25N'
FROM `creature_loot_template` WHERE `Entry` = 55689 AND FIND_IN_SET(`Item`, @NORMAL_ITEMS);

-- 25 Normal table: LFR loot as LootMode 2
INSERT INTO `creature_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT 57462, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, 2, `GroupId`, `MinCount`, `MaxCount`, 'Hagara LFR'
FROM `creature_loot_template` WHERE `Entry` = 55689 AND FIND_IN_SET(`Item`, @LFR_ITEMS);

-- 25 Normal table: shared drops available in both loot modes
INSERT INTO `creature_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT 57462, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, 3, `GroupId`, `MinCount`, `MaxCount`, 'Hagara shared'
FROM `creature_loot_template` WHERE `Entry` = 55689 AND FIND_IN_SET(`Item`, @SHARED_ITEMS);

-- Heroic tables
INSERT INTO `creature_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT 57955, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, 1, `GroupId`, `MinCount`, `MaxCount`, 'Hagara 10H'
FROM `creature_loot_template` WHERE `Entry` = 55689 AND (FIND_IN_SET(`Item`, @HEROIC_ITEMS) OR FIND_IN_SET(`Item`, @SHARED_ITEMS));

INSERT INTO `creature_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT 57956, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, 1, `GroupId`, `MinCount`, `MaxCount`, 'Hagara 25H'
FROM `creature_loot_template` WHERE `Entry` = 55689 AND (FIND_IN_SET(`Item`, @HEROIC_ITEMS) OR FIND_IN_SET(`Item`, @SHARED_ITEMS));

-- 10 Normal keeps only 397 + shared
DELETE FROM `creature_loot_template` WHERE `Entry` = 55689 AND (FIND_IN_SET(`Item`, @LFR_ITEMS) OR FIND_IN_SET(`Item`, @HEROIC_ITEMS));

UPDATE `creature_template` SET `lootid` = `entry` WHERE `entry` IN (57462, 57955, 57956);
