-- Spine of Deathwing (Dragon Soul) - full encounter: LFR / 10N / 25N / 10H / 25H
-- Deathwing controller spawn, breach-hole Spawners, Corruptions, Corrupted
-- Blood residue loop, Hideous Amalgamations, Burning Tendons, armor plate
-- gameobjects, barrel rolls, Skyfire launch gossip, creature texts, retail
-- health values, cache loot, Maelstrom teleporter and the Dizzy achievement.

-- ---------------------------------------------------------------------------
-- Script bindings (base entries only - the core ignores ScriptName on
-- difficulty child entries)
-- ---------------------------------------------------------------------------
UPDATE `creature_template` SET `ScriptName` = 'boss_spine_of_deathwing' WHERE `entry` = 53879;
UPDATE `creature_template` SET `ScriptName` = 'npc_ds_spine_spawner' WHERE `entry` = 53888;
UPDATE `creature_template` SET `ScriptName` = 'npc_ds_spine_corruption' WHERE `entry` IN (53891, 56161, 56162);
UPDATE `creature_template` SET `ScriptName` = 'npc_ds_corrupted_blood' WHERE `entry` = 53889;
UPDATE `creature_template` SET `ScriptName` = 'npc_ds_hideous_amalgamation' WHERE `entry` = 53890;
UPDATE `creature_template` SET `ScriptName` = 'npc_ds_burning_tendons' WHERE `entry` IN (56341, 56575);

-- Deathwing is a pure controller: unattackable, unselectable, PC-immune
UPDATE `creature_template` SET `unit_flags` = `unit_flags` | 0x02000102 WHERE `entry` IN (53879, 58862, 58863, 58864);

-- Anchored/objective units shrug off all crowd control (standard DS boss mask)
UPDATE `creature_template` SET `mechanic_immune_mask` = 650854271 WHERE `entry` IN
(53879, 58862, 58863, 58864,                                      -- Deathwing
 53888,                                                           -- Spawner
 53891, 57879, 57880, 57881,                                      -- Corruption
 56161, 57901, 57902, 57903,                                      -- Corruption (plug)
 56162, 57904, 57905, 57906,                                      -- Corruption (plate)
 56341, 57884, 57885, 57886, 56575, 57887, 57888, 57889);         -- Burning Tendons

-- ---------------------------------------------------------------------------
-- Health (user-supplied retail values)
--   Base health: 85,892 @ L88 exp3; Corrupted Blood is L87: 82,994
--   Corruption:           457,804 / 1,418,936 / 550,960* / 1,738,798  (5.33 / 16.52 / 6.531 / 20.244)
--   Hideous Amalgamation: 7,300,820 / 22,632,542 / 6,899,275 / 21,388,053 (85 / 263.5 / 80.325 / 249.011)
--   Corrupted Blood:      165,988 / 514,563 / 203,335 / 630,339      (2 / 6.2 / 2.45 / 7.595)
--   Burning Tendons:      3,006,220 / 9,319,282 / 7,543,207 / 23,380,876 (35 / 108.5 / 87.822 / 272.2125)
--   (10N and 25N TDB modifiers were already exact and stay untouched)
-- ---------------------------------------------------------------------------
UPDATE `creature_template` SET `HealthModifier` = 6.531 WHERE `entry` IN (57880, 57902, 57905);
UPDATE `creature_template` SET `HealthModifier` = 20.244 WHERE `entry` IN (57881, 57903, 57906);
UPDATE `creature_template` SET `HealthModifier` = 80.325 WHERE `entry` = 56517;
UPDATE `creature_template` SET `HealthModifier` = 249.011 WHERE `entry` = 56518;
UPDATE `creature_template` SET `HealthModifier` = 2.45 WHERE `entry` = 57896;
UPDATE `creature_template` SET `HealthModifier` = 7.595 WHERE `entry` = 57897;
UPDATE `creature_template` SET `HealthModifier` = 87.822 WHERE `entry` IN (57885, 57888);
UPDATE `creature_template` SET `HealthModifier` = 272.2125 WHERE `entry` IN (57886, 57889);

-- ---------------------------------------------------------------------------
-- Raid Finder stats templates (70% of 25N, applied by script)
-- ---------------------------------------------------------------------------
DELETE FROM `creature_template` WHERE `entry` IN (58253, 58254, 58255, 58256);
INSERT INTO `creature_template` (`entry`, `name`, `femaleName`, `subname`, `minlevel`, `maxlevel`, `exp`, `faction`, `unit_class`, `type`, `HealthModifier`, `AIName`, `ScriptName`, `VerifiedBuild`) VALUES
(58253, 'Corruption', '', 'LFR Stats', 88, 88, 3, 14, 1, 4, 11.564, '', '', 0),            -- ~993,255
(58254, 'Hideous Amalgamation', '', 'LFR Stats', 88, 88, 3, 14, 1, 4, 184.45, '', '', 0),  -- ~15,842,779
(58255, 'Corrupted Blood', '', 'LFR Stats', 87, 87, 3, 14, 1, 4, 4.34, '', '', 0),         -- ~360,194
(58256, 'Burning Tendons', '', 'LFR Stats', 88, 88, 3, 14, 1, 10, 75.95, '', '', 0);       -- ~6,523,497

DELETE FROM `creature_template_model` WHERE `CreatureID` IN (58253, 58254, 58255, 58256);
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `Probability`, `VerifiedBuild`) VALUES
(58253, 0, 38550, 1, 0),
(58254, 0, 38549, 1, 0),
(58255, 0, 38548, 1, 0),
(58256, 0, 39429, 1, 0);

-- ---------------------------------------------------------------------------
-- Spawns: Deathwing controller on the spine (sniffed position), the Maelstrom
-- teleporter on the flight deck (hidden until Spine is DONE), and the three
-- armor plate gameobjects on the spine centerline (rotation approximated)
-- ---------------------------------------------------------------------------
DELETE FROM `creature` WHERE `guid` IN (9000650, 9000651);
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `VerifiedBuild`) VALUES
(9000650, 53879, 967, 0, 0, 15, 0, 0, 0, 0, -13855.014, -13669.591, 265.178, 1.5708, 604800, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(9000651, 57443, 967, 0, 0, 15, 0, 0, 0, 0, 13455.5, -12140.5, 151.21, 1.60, 300, 0, 0, 0, 0, 0, 0, 0, 0, 0);

DELETE FROM `gameobject` WHERE `guid` IN (9000509, 9000510, 9000511);
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `VerifiedBuild`) VALUES
(9000509, 209623, 967, 0, 0, 15, 0, 1, 0, 0, -1, -13855.035, -13639.739, 267.866, 1.5708, 0, 0, 0.70711, 0.70711, 604800, 0, 0, '', 0),
(9000510, 209631, 967, 0, 0, 15, 0, 1, 0, 0, -1, -13854.962, -13619.114, 269.869, 1.5708, 0, 0, 0.70711, 0.70711, 604800, 0, 0, '', 0),
(9000511, 209632, 967, 0, 0, 15, 0, 1, 0, 0, -1, -13855.393, -13597.764, 272.356, 1.5708, 0, 0, 0.70711, 0.70711, 604800, 0, 0, '', 0);

-- The Maelstrom teleporter ports to the Madness platform (was a placeholder spell)
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 57443;
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(57443, 108449, 1, 0);

DELETE FROM `spell_target_position` WHERE `ID` = 108449;
INSERT INTO `spell_target_position` (`ID`, `EffectIndex`, `MapID`, `PositionX`, `PositionY`, `PositionZ`, `Orientation`, `VerifiedBuild`) VALUES
(108449, 0, 967, -12082.0, 12153.5, -2.66, 0.75, 0);

-- ---------------------------------------------------------------------------
-- Skyfire captains: second gossip option launches the raid onto the spine.
-- Shown once Blackhorn is defeated, hidden again after Spine is defeated.
-- ---------------------------------------------------------------------------
DELETE FROM `gossip_menu_option` WHERE `MenuID` = 13252 AND `OptionID` = 1;
INSERT INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcflag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES
(13252, 1, 0, 'Take me to the Skyfire to fight Deathwing.', 52984, 1, 1, 0, 0, 0, 0, NULL, 0, 0);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 15 AND `SourceGroup` = 13252 AND `SourceEntry` = 1;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(15, 13252, 1, 0, 0, 32, 0, 5, 3, 2, 0, 0, 0, '', 'Spine launch option - requires Warmaster Blackhorn DONE'),
(15, 13252, 1, 0, 0, 32, 0, 6, 3, 2, 1, 0, 0, '', 'Spine launch option - hidden once Spine of Deathwing is DONE');

-- ---------------------------------------------------------------------------
-- Serverside spell: Raid Finder Seal Armor Breach pacing. +50% cast time on
-- the LFR Tendons stretches the native 23s cast to retail LFR's 34.5s.
-- ---------------------------------------------------------------------------
DELETE FROM `spell_dbc` WHERE `Id` = 123458;
INSERT INTO `spell_dbc` (`Id`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `AttributesEx4`, `AttributesEx5`, `AttributesEx6`, `AttributesEx7`, `AttributesEx8`, `AttributesEx9`, `AttributesEx10`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `SchoolMask`, `SpellAuraOptionsId`, `SpellCastingRequirementsId`, `SpellCategoriesId`, `SpellClassOptionsId`, `SpellEquippedItemsId`, `SpellInterruptsId`, `SpellLevelsId`, `SpellTargetRestrictionsId`, `SpellName`) VALUES
(123458, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 21, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, '(Serverside/Non-DB2) Spine of Deathwing - Seal Armor Breach LFR Pacing <Do Not Translate>');

DELETE FROM `spelleffect_dbc` WHERE `Id` = 160118;
INSERT INTO `spelleffect_dbc` (`Id`, `Effect`, `EffectAmplitude`, `EffectAura`, `EffectAuraPeriod`, `EffectBasePoints`, `EffectBonusCoefficient`, `EffectChainAmplitude`, `EffectChainTargets`, `EffectDieSides`, `EffectItemType`, `EffectMechanic`, `EffectMiscValue`, `EffectMiscValueB`, `EffectPointsPerResource`, `EffectRadiusIndex`, `EffectRadiusMaxIndex`, `EffectRealPointsPerLevel`, `EffectSpellClassMaskA`, `EffectSpellClassMaskB`, `EffectSpellClassMaskC`, `EffectTriggerSpell`, `EffectImplicitTargetA`, `EffectImplicitTargetB`, `SpellID`, `EffectIndex`, `Comment`) VALUES
(160118, 6, 0, 65, 0, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 123458, 0, 'Spine of Deathwing - Seal Armor Breach LFR Pacing (+50% cast time)');

-- ---------------------------------------------------------------------------
-- Spell target conditions: entry-restricted area targets
-- ---------------------------------------------------------------------------
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` IN (105846, 105366, 105384, 105241);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 105846, 0, 0, 31, 0, 3, 56341, 0, 0, 0, 0, '', 'Nuclear Blast seam check targets Burning Tendons (right)'),
(13, 1, 105846, 0, 1, 31, 0, 3, 56575, 0, 0, 0, 0, '', 'Nuclear Blast seam check targets Burning Tendons (left)'),
(13, 2, 105366, 0, 0, 31, 0, 3, 56341, 0, 0, 0, 0, '', 'Plate Fly Off Left kills the right twin'),
(13, 2, 105384, 0, 0, 31, 0, 3, 56575, 0, 0, 0, 0, '', 'Plate Fly Off Right kills the left twin'),
(13, 1, 105241, 0, 0, 31, 0, 3, 53889, 0, 0, 0, 0, '', 'Absorb Blood consumes Corrupted Blood residue');

-- ---------------------------------------------------------------------------
-- Creature texts (broadcast_text verified 4.3.4 IDs)
-- ---------------------------------------------------------------------------
DELETE FROM `creature_text` WHERE `CreatureID` IN (53879, 53890);
DELETE FROM `creature_text` WHERE `CreatureID` IN (55870, 55891) AND `GroupID` = 9;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
-- Deathwing
(53879, 0, 0, '%s feels players on his left side.\nHe''s about to roll left!', 41, 0, 100, 0, 0, 0, 54875, 3, 'Spine of Deathwing - Roll Warning Left'),
(53879, 1, 0, '%s feels players on his right side.\nHe''s about to roll right!', 41, 0, 100, 0, 0, 0, 54878, 3, 'Spine of Deathwing - Roll Warning Right'),
(53879, 2, 0, '%s rolls left!', 41, 0, 100, 0, 0, 0, 54879, 3, 'Spine of Deathwing - Roll Left'),
(53879, 3, 0, '%s rolls right!', 41, 0, 100, 0, 0, 0, 54880, 3, 'Spine of Deathwing - Roll Right'),
(53879, 4, 0, '%s levels out.', 41, 0, 100, 0, 0, 0, 54894, 3, 'Spine of Deathwing - Level Out'),
(53879, 5, 0, 'Your efforts are insignificant. I carry you to your deaths.', 14, 0, 100, 0, 0, 0, 56624, 3, 'Spine of Deathwing - Taunt'),
(53879, 6, 0, 'Your tenacity is admirable, but pointless. You ride into the jaws of the apocalypse.', 14, 0, 100, 0, 0, 0, 56626, 3, 'Spine of Deathwing - Taunt'),
(53879, 7, 0, 'You are less than dust, fit only to be brushed from my back.', 14, 0, 100, 0, 0, 0, 55279, 3, 'Spine of Deathwing - Taunt'),
(53879, 8, 0, 'Cling while you can, "heroes." You and your world are doomed.', 14, 0, 100, 0, 0, 0, 56625, 3, 'Spine of Deathwing - Taunt'),
(53879, 9, 0, 'Ha! I had not realized you fools were still there.', 14, 0, 100, 0, 0, 0, 56623, 3, 'Spine of Deathwing - Taunt'),
-- Hideous Amalgamation
(53890, 0, 0, '|TInterface\\Icons\\inv_gizmo_supersappercharge.blp:20|t%s is casting |c0087CEFA|Hspell:105845|h[Nuclear Blast]|h|r!', 41, 0, 100, 0, 0, 0, 54955, 3, 'Hideous Amalgamation - Nuclear Blast'),
(53890, 1, 0, '|c0087CEFA|Hspell:105845|h[Nuclear Blast]|h|r wasn''t close enough to pry up the plate!', 41, 0, 100, 0, 0, 0, 57109, 3, 'Hideous Amalgamation - Blast Missed'),
(53890, 2, 0, '%s didn''t absorb enough Corrupted Blood residue to trigger a Nuclear Blast.', 41, 0, 100, 0, 0, 0, 57111, 3, 'Hideous Amalgamation - Not Enough Residue'),
-- Skyfire captains: Spine launch
(55870, 9, 0, 'The plates! He''s coming apart! Tear up the plates and we''ve got a shot at bringing him down!', 14, 0, 100, 0, 0, 26290, 56618, 0, 'Sky Captain Swayze - Spine Launch'),
(55891, 9, 0, 'The plates! He''s coming apart! Tear up the plates and we''ve got a shot at bringing him down!', 14, 0, 100, 0, 0, 26290, 56618, 0, 'Ka''anu Reevs - Spine Launch');

-- ---------------------------------------------------------------------------
-- Spell script bindings
-- ---------------------------------------------------------------------------
DELETE FROM `spell_script_names` WHERE `ScriptName` IN
('spell_ds_spine_searing_plasma', 'spell_ds_spine_fiery_grip', 'spell_ds_spine_absorb_blood',
 'spell_ds_spine_absorbed_blood', 'spell_ds_spine_nuclear_blast', 'spell_ds_spine_nuclear_blast_check',
 'spell_ds_spine_seal_armor_breach', 'spell_ds_spine_blood_corruption', 'spell_ds_spine_kill_credit');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(109379, 'spell_ds_spine_searing_plasma'),
(105490, 'spell_ds_spine_fiery_grip'),
(109457, 'spell_ds_spine_fiery_grip'),
(109458, 'spell_ds_spine_fiery_grip'),
(109459, 'spell_ds_spine_fiery_grip'),
(105241, 'spell_ds_spine_absorb_blood'),
(105248, 'spell_ds_spine_absorbed_blood'),
(105845, 'spell_ds_spine_nuclear_blast'),
(105846, 'spell_ds_spine_nuclear_blast_check'),
(105847, 'spell_ds_spine_seal_armor_breach'),
(105848, 'spell_ds_spine_seal_armor_breach'),
(106199, 'spell_ds_spine_blood_corruption'),
(106200, 'spell_ds_spine_blood_corruption'),
(104574, 'spell_ds_spine_kill_credit');

-- ---------------------------------------------------------------------------
-- Achievement: Maybe He'll Get Dizzy... (6133, criteria 18502) - handled by
-- the instance script (4+ rolls during the kill, non-LFR)
-- ---------------------------------------------------------------------------
DELETE FROM `achievement_criteria_data` WHERE `criteria_id` = 18502;
INSERT INTO `achievement_criteria_data` (`criteria_id`, `type`, `value1`, `value2`, `ScriptName`) VALUES
(18502, 18, 0, 0, '');

-- ---------------------------------------------------------------------------
-- Loot: Spine awards from the Cache of the Aspects (Lesser = 10-player,
-- Greater = 25-player). LootMode 1 = Normal (ilvl 403), 2 = Raid Finder
-- (ilvl 390, Greater only), 4 = Heroic (ilvl 416). Chances follow the TDB
-- distribution on Deathwing's (unused) creature loot table; the normal set
-- is scaled x5 to the sibling-boss average, heroic mirrors normal.
-- creature_loot_template 53879 stays untouched as the reference source.
-- ---------------------------------------------------------------------------
DELETE FROM `gameobject_loot_template` WHERE `Entry` IN (41171, 41176);
INSERT INTO `gameobject_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`) VALUES
-- Lesser Cache of the Aspects (10-player): Normal
(41171, 77197, 0, 23.179, 0, 0, 1, 0, 1, 1, 'Spine 10N'),
(41171, 77198, 0, 25.552, 0, 0, 1, 0, 1, 1, 'Spine 10N'),
(41171, 77199, 0, 19.647, 0, 0, 1, 0, 1, 1, 'Spine 10N'),
(41171, 77200, 0, 21.6885, 0, 0, 1, 0, 1, 1, 'Spine 10N'),
(41171, 77201, 0, 19.647, 0, 0, 1, 0, 1, 1, 'Spine 10N'),
(41171, 77235, 0, 14.0175, 0, 0, 1, 0, 1, 1, 'Spine 10N'),
(41171, 77236, 0, 29.8015, 0, 0, 1, 0, 1, 1, 'Spine 10N'),
(41171, 77237, 0, 16.17, 0, 0, 1, 0, 1, 1, 'Spine 10N'),
(41171, 77238, 0, 25.6625, 0, 0, 1, 0, 1, 1, 'Spine 10N'),
(41171, 78357, 0, 25.552, 0, 0, 1, 0, 1, 1, 'Spine 10N'),
-- Lesser Cache: Heroic
(41171, 77994, 0, 23.179, 0, 0, 4, 0, 1, 1, 'Spine 10H'),
(41171, 77995, 0, 25.552, 0, 0, 4, 0, 1, 1, 'Spine 10H'),
(41171, 77996, 0, 19.647, 0, 0, 4, 0, 1, 1, 'Spine 10H'),
(41171, 77997, 0, 21.6885, 0, 0, 4, 0, 1, 1, 'Spine 10H'),
(41171, 77998, 0, 19.647, 0, 0, 4, 0, 1, 1, 'Spine 10H'),
(41171, 78461, 0, 14.0175, 0, 0, 4, 0, 1, 1, 'Spine 10H'),
(41171, 78462, 0, 29.8015, 0, 0, 4, 0, 1, 1, 'Spine 10H'),
(41171, 78463, 0, 16.17, 0, 0, 4, 0, 1, 1, 'Spine 10H'),
(41171, 78464, 0, 25.6625, 0, 0, 4, 0, 1, 1, 'Spine 10H'),
(41171, 78465, 0, 25.552, 0, 0, 4, 0, 1, 1, 'Spine 10H'),
-- Lesser Cache: shared
(41171, 77952, 0, 13.0353, 0, 0, 5, 0, 1, 3, 'Spine - Fangs of the Father'),
(41171, 71998, 0, 100, 0, 0, 4, 0, 1, 3, 'Spine - Essence of Destruction (heroic)'),
-- Greater Cache of the Aspects (25-player): Normal
(41176, 77197, 0, 23.179, 0, 0, 1, 0, 1, 1, 'Spine 25N'),
(41176, 77198, 0, 25.552, 0, 0, 1, 0, 1, 1, 'Spine 25N'),
(41176, 77199, 0, 19.647, 0, 0, 1, 0, 1, 1, 'Spine 25N'),
(41176, 77200, 0, 21.6885, 0, 0, 1, 0, 1, 1, 'Spine 25N'),
(41176, 77201, 0, 19.647, 0, 0, 1, 0, 1, 1, 'Spine 25N'),
(41176, 77235, 0, 14.0175, 0, 0, 1, 0, 1, 1, 'Spine 25N'),
(41176, 77236, 0, 29.8015, 0, 0, 1, 0, 1, 1, 'Spine 25N'),
(41176, 77237, 0, 16.17, 0, 0, 1, 0, 1, 1, 'Spine 25N'),
(41176, 77238, 0, 25.6625, 0, 0, 1, 0, 1, 1, 'Spine 25N'),
(41176, 78357, 0, 25.552, 0, 0, 1, 0, 1, 1, 'Spine 25N'),
-- Greater Cache: Raid Finder (TDB chances)
(41176, 77974, 0, 26.4238, 0, 0, 2, 0, 1, 1, 'Spine LFR'),
(41176, 77975, 0, 24.8896, 0, 0, 2, 0, 1, 1, 'Spine LFR'),
(41176, 77976, 0, 18.2892, 0, 0, 2, 0, 1, 1, 'Spine LFR'),
(41176, 77977, 0, 26.2141, 0, 0, 2, 0, 1, 1, 'Spine LFR'),
(41176, 77978, 0, 18.8742, 0, 0, 2, 0, 1, 1, 'Spine LFR'),
(41176, 78466, 0, 27.5607, 0, 0, 2, 0, 1, 1, 'Spine LFR'),
(41176, 78467, 0, 27.4172, 0, 0, 2, 0, 1, 1, 'Spine LFR'),
(41176, 78468, 0, 18.0132, 0, 0, 2, 0, 1, 1, 'Spine LFR'),
(41176, 78469, 0, 18.8079, 0, 0, 2, 0, 1, 1, 'Spine LFR'),
(41176, 78470, 0, 23.5872, 0, 0, 2, 0, 1, 1, 'Spine LFR'),
-- Greater Cache: Heroic
(41176, 77994, 0, 23.179, 0, 0, 4, 0, 1, 1, 'Spine 25H'),
(41176, 77995, 0, 25.552, 0, 0, 4, 0, 1, 1, 'Spine 25H'),
(41176, 77996, 0, 19.647, 0, 0, 4, 0, 1, 1, 'Spine 25H'),
(41176, 77997, 0, 21.6885, 0, 0, 4, 0, 1, 1, 'Spine 25H'),
(41176, 77998, 0, 19.647, 0, 0, 4, 0, 1, 1, 'Spine 25H'),
(41176, 78461, 0, 14.0175, 0, 0, 4, 0, 1, 1, 'Spine 25H'),
(41176, 78462, 0, 29.8015, 0, 0, 4, 0, 1, 1, 'Spine 25H'),
(41176, 78463, 0, 16.17, 0, 0, 4, 0, 1, 1, 'Spine 25H'),
(41176, 78464, 0, 25.6625, 0, 0, 4, 0, 1, 1, 'Spine 25H'),
(41176, 78465, 0, 25.552, 0, 0, 4, 0, 1, 1, 'Spine 25H'),
-- Greater Cache: shared
(41176, 77952, 0, 13.0353, 0, 0, 7, 0, 1, 3, 'Spine - Fangs of the Father'),
(41176, 71998, 0, 100, 0, 0, 4, 0, 1, 3, 'Spine - Essence of Destruction (heroic)');
