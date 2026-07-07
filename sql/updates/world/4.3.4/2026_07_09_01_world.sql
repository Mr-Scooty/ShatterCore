-- Warmaster Blackhorn (Dragon Soul) - full encounter: LFR / 10N / 25N / 10H / 25H
-- Gunship defense on the flight-arena Skyfire: staging captains, pursuit
-- controller, ship health proxy, drake waves, harpoon guns, sappers, Goriona,
-- creature texts, spell script bindings, retail health values, Deck Defender
-- and the per-difficulty loot split.

-- ---------------------------------------------------------------------------
-- Script bindings (base entries only - the core ignores ScriptName on
-- difficulty child entries)
-- ---------------------------------------------------------------------------
UPDATE `creature_template` SET `ScriptName` = 'boss_warmaster_blackhorn' WHERE `entry` = 56427;
UPDATE `creature_template` SET `ScriptName` = 'npc_ds_goriona' WHERE `entry` = 56781;
UPDATE `creature_template` SET `ScriptName` = 'npc_gunship_pursuit_controller' WHERE `entry` = 56599;
UPDATE `creature_template` SET `ScriptName` = 'npc_ds_skyfire' WHERE `entry` = 56598;
UPDATE `creature_template` SET `ScriptName` = 'npc_ds_skyfire_captain' WHERE `entry` IN (55870, 55891);
UPDATE `creature_template` SET `ScriptName` = 'npc_ds_twilight_assault_drake' WHERE `entry` IN (56855, 56587);
UPDATE `creature_template` SET `ScriptName` = 'npc_ds_twilight_elite' WHERE `entry` IN (56854, 56848);
UPDATE `creature_template` SET `ScriptName` = 'npc_ds_skyfire_harpoon_gun' WHERE `entry` = 56681;
UPDATE `creature_template` SET `ScriptName` = 'npc_ds_skyfire_cannon' WHERE `entry` = 57260;
UPDATE `creature_template` SET `ScriptName` = 'npc_ds_twilight_infiltrator' WHERE `entry` = 56922;
UPDATE `creature_template` SET `ScriptName` = 'npc_ds_twilight_sapper' WHERE `entry` = 56923;
UPDATE `creature_template` SET `ScriptName` = 'npc_ds_twilight_flames' WHERE `entry` = 57268;
UPDATE `creature_template` SET `ScriptName` = 'npc_ds_deck_fire' WHERE `entry` = 57920;

-- Sappers cannot be taunted (they can be slowed, rooted and stunned)
UPDATE `creature_template` SET `flags_extra` = `flags_extra` | 0x100 WHERE `entry` IN (56923, 57704, 57717, 57718);

-- Ka'anu Reevs offers the same launch gossip as Swayze
UPDATE `creature_template` SET `npcflag` = `npcflag` | 1, `gossip_menu_id` = 13252 WHERE `entry` = 55891;

-- ---------------------------------------------------------------------------
-- Health (user-supplied retail values)
--   Blackhorn (base 85,892 @ L88 exp3):
--     10N 56427: 240      = 20,614,080 (already correct)
--     25N 57699: 600      = 51,535,200 (already correct)
--     10H 57847: 304.5    = 26,154,114
--     25H 57848: 730.8    = 62,769,873
--   Goriona (L88): 1,025,030 / 1,826,950 / 1,917,799 / 6,217,968
--   Assault Drake (base 80,195 @ L86 exp3): 79,008 / 237,022 / 114,920 / 344,760
--   Elite Dreadblade: 229,840 / 723,996 / 310,284 / 977,395
--   Elite Slayer:     229,840 / 723,996 / 310,284 / 535,245
--   Sapper: 24,420 / 86,190 / 34,476 / 120,666
--   The Skyfire (base 6,639 @ L88 exp0): 4.0M (10) / 15.0M (25), same on heroic
-- ---------------------------------------------------------------------------
UPDATE `creature_template` SET `HealthModifier` = 304.5 WHERE `entry` = 57847;
UPDATE `creature_template` SET `HealthModifier` = 730.8 WHERE `entry` = 57848;

UPDATE `creature_template` SET `HealthModifier` = 11.9339 WHERE `entry` = 56781;
UPDATE `creature_template` SET `HealthModifier` = 21.2703 WHERE `entry` = 57937;
UPDATE `creature_template` SET `HealthModifier` = 22.328 WHERE `entry` = 57938;
UPDATE `creature_template` SET `HealthModifier` = 72.3929 WHERE `entry` = 57939;

UPDATE `creature_template` SET `HealthModifier` = 0.9852 WHERE `entry` IN (56587, 56855);
UPDATE `creature_template` SET `HealthModifier` = 2.9556 WHERE `entry` IN (57700, 57701);
UPDATE `creature_template` SET `HealthModifier` = 1.433 WHERE `entry` IN (57837, 57839);
UPDATE `creature_template` SET `HealthModifier` = 4.299 WHERE `entry` IN (57838, 57840);

UPDATE `creature_template` SET `HealthModifier` = 2.866 WHERE `entry` IN (56854, 56848);
UPDATE `creature_template` SET `HealthModifier` = 9.0279 WHERE `entry` IN (57702, 57703);
UPDATE `creature_template` SET `HealthModifier` = 3.8691 WHERE `entry` IN (57841, 57843);
UPDATE `creature_template` SET `HealthModifier` = 12.1877 WHERE `entry` = 57842;
UPDATE `creature_template` SET `HealthModifier` = 6.6743 WHERE `entry` = 57844;

UPDATE `creature_template` SET `HealthModifier` = 0.3045 WHERE `entry` = 56923;
UPDATE `creature_template` SET `HealthModifier` = 1.0748 WHERE `entry` = 57704;
UPDATE `creature_template` SET `HealthModifier` = 0.4299 WHERE `entry` = 57717;
UPDATE `creature_template` SET `HealthModifier` = 1.5047 WHERE `entry` = 57718;

UPDATE `creature_template` SET `HealthModifier` = 602.5 WHERE `entry` IN (56598, 57845);
UPDATE `creature_template` SET `HealthModifier` = 2259.375 WHERE `entry` IN (57698, 57846);

-- Raid Finder stats templates (70% of 25N, user-approved).
-- 58250/58251 are live rows (Bound Lightning Elemental) - skipped.
DELETE FROM `creature_template` WHERE `entry` IN (58246, 58247, 58248, 58249, 58252);
INSERT INTO `creature_template` (`entry`, `name`, `femaleName`, `subname`, `minlevel`, `maxlevel`, `exp`, `faction`, `unit_class`, `type`, `HealthModifier`, `AIName`, `ScriptName`, `VerifiedBuild`) VALUES
(58246, 'Warmaster Blackhorn', '', 'LFR Stats', 88, 88, 3, 14, 1, 7, 420, '', '', 0),       -- 36,074,640
(58247, 'Goriona', '', 'LFR Stats', 88, 88, 3, 14, 1, 2, 14.8892, '', '', 0),               -- ~1,278,866
(58248, 'Twilight Assault Drake', '', 'LFR Stats', 86, 86, 3, 14, 1, 2, 2.0689, '', '', 0), -- ~165,915
(58249, 'Twilight Elite', '', 'LFR Stats', 86, 86, 3, 16, 1, 7, 6.3196, '', '', 0),         -- ~506,800
(58252, 'The Skyfire', '', 'LFR Stats', 88, 88, 0, 35, 1, 4, 1581.5625, '', '', 0);         -- 10,500,000

DELETE FROM `creature_template_model` WHERE `CreatureID` IN (58246, 58247, 58248, 58249, 58252);
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `Probability`, `VerifiedBuild`) VALUES
(58246, 0, 39399, 1, 0),
(58247, 0, 39691, 1, 0),
(58248, 0, 40158, 1, 0),
(58249, 0, 29059, 1, 0),
(58252, 0, 31043, 1, 0);

-- ---------------------------------------------------------------------------
-- Vehicle accessories (seat map from the retail sniff). Riders persist after
-- being dropped on deck (minion 0); gun crews die with their mounts.
-- ---------------------------------------------------------------------------
-- The accessory loader requires a spellclick row per vehicle entry (TDB
-- convention: Ride Vehicle Hardcoded). None of these are player-clickable -
-- their npcflag lacks UNIT_NPC_FLAG_SPELLCLICK.
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` IN (56781, 56855, 56587, 56923, 56681, 57260);
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(56781, 46598, 1, 0),
(56855, 46598, 1, 0),
(56587, 46598, 1, 0),
(56923, 46598, 1, 0),
(56681, 46598, 1, 0),
(57260, 46598, 1, 0);

DELETE FROM `vehicle_template_accessory` WHERE `entry` IN (56781, 56855, 56587, 56923, 56681, 57260);
INSERT INTO `vehicle_template_accessory` (`entry`, `accessory_entry`, `seat_id`, `minion`, `description`, `summontype`, `summontimer`) VALUES
(56781, 56427, 0, 0, 'Goriona - Warmaster Blackhorn', 6, 30000),
(56855, 56854, 0, 0, 'Twilight Assault Drake - Twilight Elite Dreadblade', 6, 30000),
(56587, 56848, 0, 0, 'Twilight Assault Drake - Twilight Elite Slayer', 6, 30000),
(56923, 57470, 0, 1, 'Twilight Sapper - Dynamite Bundle', 6, 30000),
(56923, 57470, 1, 1, 'Twilight Sapper - Dynamite Bundle', 6, 30000),
(56681, 57264, 0, 1, 'Skyfire Harpoon Gun - Skyfire Commando', 6, 30000),
(57260, 57264, 0, 1, 'Skyfire Cannon - Skyfire Commando', 6, 30000);

-- ---------------------------------------------------------------------------
-- Spawns:
--   56599 Gunship Pursuit Controller beside the parked staging ship (its grid
--         loads with the raid; the whole flight arena is summoned by it)
--   55870/55891 the sky captains aboard the parked Skyfire (sniff positions)
--   57378 Travel to the Skyfire deck at the summit teleporter cluster
--         (visibility gated on Blackhorn's death by the instance script)
-- ---------------------------------------------------------------------------
DELETE FROM `creature` WHERE `guid` IN (9000646, 9000647, 9000648, 9000649);
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `VerifiedBuild`) VALUES
(9000646, 56599, 967, 0, 0, 15, 0, 0, 0, 0, -1704.00, -2355.00, 345.00, 0, 604800, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(9000647, 55870, 967, 0, 0, 15, 0, 0, 0, 0, -1695.60, -2353.70, 339.85, 1.5533, 300, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(9000648, 55891, 967, 0, 0, 15, 0, 0, 0, 0, -1692.10, -2353.50, 339.85, 1.5533, 300, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(9000649, 57378, 967, 0, 0, 15, 0, 0, 0, 0, -1801.00, -2392.00, 341.36, 0.10, 300, 0, 0, 0, 0, 0, 0, 0, 0, 0);

-- The deck teleporter ports to the flight arena (was a placeholder spell)
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 57378;
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(57378, 108263, 1, 0);

-- Onslaught Target carries its swirl marker state aura from spawn
DELETE FROM `creature_template_addon` WHERE `entry` = 57238;
INSERT INTO `creature_template_addon` (`entry`, `waypointPathId`, `cyclicSplinePathId`, `mount`, `StandState`, `AnimTier`, `VisFlags`, `SheathState`, `PvPFlags`, `emote`, `aiAnimKit`, `movementAnimKit`, `meleeAnimKit`, `visibilityDistanceType`, `auras`) VALUES
(57238, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, '107927');

-- The launch gossip option (menu 13252 is shared by both captains)
DELETE FROM `gossip_menu_option` WHERE `MenuID` = 13252;
INSERT INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcflag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES
(13252, 0, 0, 'Take me to the deck of The Skyfire to fight Blackhorn.', 56259, 1, 1, 0, 0, 0, 0, '', 0, 0);

-- ---------------------------------------------------------------------------
-- Creature texts (broadcast_text IDs and VO sounds are retail rows; Horde
-- captain VO is unknown and stays silent)
-- ---------------------------------------------------------------------------
DELETE FROM `creature_text` WHERE `CreatureID` IN (56427, 56781, 56598, 56922, 55870, 55891);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
-- Warmaster Blackhorn
(56427, 0, 0, 'Hah! I was hoping you''d make it this far. You''d best be ready for a real fight.', 14, 0, 100, 0, 0, 26214, 55442, 0, 'Blackhorn - Intro'),
(56427, 1, 0, 'You won''t get near the Master. Dragonriders, attack!', 14, 0, 100, 0, 0, 26210, 55443, 0, 'Blackhorn - Aggro'),
(56427, 2, 0, 'Goriona! Give them hell!', 14, 0, 100, 0, 0, 26219, 55453, 0, 'Blackhorn - Goriona'),
(56427, 3, 0, 'Looks like I''m doing this myself. Good!', 14, 0, 100, 0, 0, 26212, 55456, 0, 'Blackhorn - Phase Two'),
(56427, 4, 0, 'Mess with the bull....', 14, 0, 100, 0, 0, 26215, 55446, 0, 'Blackhorn - Shockwave'),
(56427, 5, 0, 'How''s THIS?', 14, 0, 100, 0, 0, 26221, 55454, 0, 'Blackhorn - Flavor 1'),
(56427, 5, 1, 'COME ON!', 14, 0, 100, 0, 0, 26222, 55455, 0, 'Blackhorn - Flavor 2'),
(56427, 6, 0, 'Down you go!', 14, 0, 100, 0, 0, 26217, 55447, 0, 'Blackhorn - Slay 1'),
(56427, 6, 1, 'Get up! Oh... weakling!', 14, 0, 100, 0, 0, 26218, 55449, 0, 'Blackhorn - Slay 2'),
(56427, 7, 0, 'We''re flying a little too close. It''s been a good fight, but I''m ending it, now.', 14, 0, 100, 0, 0, 26213, 55457, 0, 'Blackhorn - Berserk'),
(56427, 8, 0, 'Well... done, heh. But I wonder if you''re good enough... to best him.', 14, 0, 100, 0, 0, 26211, 55444, 0, 'Blackhorn - Death'),
(56427, 9, 0, '%s siphons vitality from Goriona and attacks with renewed vigor!', 41, 0, 100, 0, 0, 0, 57062, 0, 'Blackhorn - Siphon Vitality Emote'),
-- Goriona
(56781, 0, 0, '|TInterface\\Icons\\spell_fire_twilightflamebreath.blp:20|t%s prepares to unleash a |cFF9900CC|Hspell:106401|h[Twilight Onslaught]|h|r!', 41, 0, 100, 0, 0, 0, 0, 0, 'Goriona - Twilight Onslaught Emote'),
(56781, 1, 0, '|Tinterface\\icons\\spell_fire_twilightrainoffire.blp:0|t%s fires a |cFF9900CC|Hspell:110153|h[Broadside]|h|r at the Skyfire!', 41, 0, 100, 0, 0, 0, 57050, 0, 'Goriona - Broadside Emote'),
(56781, 2, 0, '%s screeches in pain and retreats into the swirling clouds.', 41, 0, 100, 0, 0, 0, 56156, 0, 'Goriona - Retreat Emote'),
-- The Skyfire
(56598, 0, 0, '|Tinterface\\icons\\spell_fire_ragnaros_lavabolt.blp:0|tStructural damage to the Skyfire triggers a sudden |cFFFF0000|Hspell:110095|h[Deck Fire]|h|r!', 41, 0, 100, 0, 0, 0, 57030, 0, 'The Skyfire - Deck Fire Emote'),
-- Twilight Infiltrator
(56922, 0, 0, 'A drake swoops down to drop a Twilight Sapper onto the deck!', 41, 0, 100, 0, 0, 0, 56264, 0, 'Twilight Infiltrator - Sapper Emote'),
-- Sky Captain Swayze
(55870, 0, 0, 'Welcome aboard the Skyfire.  You ready to chase down the end of the world?', 14, 0, 100, 0, 0, 26302, 56629, 0, 'Swayze - Welcome'),
(55870, 1, 0, 'All ahead full. Everything depends on our speed! We can''t let the Destroyer get away.', 14, 0, 100, 0, 0, 26292, 55430, 0, 'Swayze - Launch'),
(55870, 2, 0, 'Our engines are damaged!  We''re sitting ducks up here!', 14, 0, 100, 0, 0, 26303, 56079, 0, 'Swayze - Engines'),
(55870, 3, 0, 'All hands to battle stations; get those monsters away from the ship!', 14, 0, 100, 0, 0, 26294, 55431, 0, 'Swayze - Battle Stations'),
(55870, 4, 0, 'Concentrate everything on the armored drake!', 14, 0, 100, 0, 0, 26295, 55433, 0, 'Swayze - Harpoon'),
(55870, 5, 0, 'An enemy sapper''s breached the engine room!', 14, 0, 100, 0, 0, 26296, 55434, 0, 'Swayze - Sapper Breach'),
(55870, 6, 0, 'The Skyfire can''t take much more of this!', 14, 0, 100, 0, 0, 26297, 55435, 0, 'Swayze - Ship Low'),
(55870, 7, 0, 'We''re going down. Abandon the ship!', 14, 0, 100, 0, 0, 26298, 55436, 0, 'Swayze - Abandon Ship'),
(55870, 8, 0, 'The engines are back online! Full speed ahead! We can''t let Deathwing escape!', 14, 0, 100, 0, 0, 26304, 55441, 0, 'Swayze - Outro'),
-- Ka'anu Reevs (same lines; Horde VO ids unknown)
(55891, 0, 0, 'Welcome aboard the Skyfire.  You ready to chase down the end of the world?', 14, 0, 100, 0, 0, 0, 56629, 0, 'Reevs - Welcome'),
(55891, 1, 0, 'All ahead full. Everything depends on our speed! We can''t let the Destroyer get away.', 14, 0, 100, 0, 0, 0, 55430, 0, 'Reevs - Launch'),
(55891, 2, 0, 'Our engines are damaged!  We''re sitting ducks up here!', 14, 0, 100, 0, 0, 0, 56079, 0, 'Reevs - Engines'),
(55891, 3, 0, 'All hands to battle stations; get those monsters away from the ship!', 14, 0, 100, 0, 0, 0, 55431, 0, 'Reevs - Battle Stations'),
(55891, 4, 0, 'Concentrate everything on the armored drake!', 14, 0, 100, 0, 0, 0, 55433, 0, 'Reevs - Harpoon'),
(55891, 5, 0, 'An enemy sapper''s breached the engine room!', 14, 0, 100, 0, 0, 0, 55434, 0, 'Reevs - Sapper Breach'),
(55891, 6, 0, 'The Skyfire can''t take much more of this!', 14, 0, 100, 0, 0, 0, 55435, 0, 'Reevs - Ship Low'),
(55891, 7, 0, 'We''re going down. Abandon the ship!', 14, 0, 100, 0, 0, 0, 55436, 0, 'Reevs - Abandon Ship'),
(55891, 8, 0, 'The engines are back online! Full speed ahead! We can''t let Deathwing escape!', 14, 0, 100, 0, 0, 0, 55441, 0, 'Reevs - Outro');

-- ---------------------------------------------------------------------------
-- Spell script bindings (base spell + every SpellDifficulty fork)
-- ---------------------------------------------------------------------------
DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
'spell_blackhorn_twilight_barrage', 'spell_goriona_twilight_onslaught',
'spell_blackhorn_harpoon', 'spell_blackhorn_detonate',
'spell_blackhorn_disrupting_roar', 'spell_blackhorn_shockwave',
'spell_blackhorn_vengeance', 'spell_goriona_consuming_shroud',
'spell_goriona_broadside');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(107439, 'spell_blackhorn_twilight_barrage'),
(109203, 'spell_blackhorn_twilight_barrage'),
(109204, 'spell_blackhorn_twilight_barrage'),
(109205, 'spell_blackhorn_twilight_barrage'),
(106401, 'spell_goriona_twilight_onslaught'),
(108862, 'spell_goriona_twilight_onslaught'),
(109226, 'spell_goriona_twilight_onslaught'),
(109227, 'spell_goriona_twilight_onslaught'),
(108038, 'spell_blackhorn_harpoon'),
(107518, 'spell_blackhorn_detonate'),
(108044, 'spell_blackhorn_disrupting_roar'),
(109228, 'spell_blackhorn_disrupting_roar'),
(109229, 'spell_blackhorn_disrupting_roar'),
(109230, 'spell_blackhorn_disrupting_roar'),
(110137, 'spell_blackhorn_shockwave'),
(108045, 'spell_blackhorn_vengeance'),
(110214, 'spell_goriona_consuming_shroud'),
(110598, 'spell_goriona_consuming_shroud'),
(110153, 'spell_goriona_broadside');

-- ---------------------------------------------------------------------------
-- Deck Defender (achievement 6105, criteria 18444): routed through the
-- instance script (no Twilight Barrage damaged the Skyfire)
-- ---------------------------------------------------------------------------
DELETE FROM `achievement_criteria_data` WHERE `criteria_id` = 18444 AND `type` = 18;
INSERT INTO `achievement_criteria_data` (`criteria_id`, `type`, `value1`, `value2`, `ScriptName`) VALUES
(18444, 18, 0, 0, '');

-- ---------------------------------------------------------------------------
-- Loot: split the TDB rows for 56427 by item level (Item-sparse.db2)
--   397 Normal (22), 410 Heroic (22), 384 LFR (32), shared 71998 + 77952
--   10N keeps the base table; 25N carries Normal (LootMode 1) + LFR
--   (LootMode 2) + shared (LootMode 3); 10H/25H get Heroic + shared
-- ---------------------------------------------------------------------------
SET @NORMAL_ITEMS := '77202,77207,77208,77209,77210,77211,77224,77225,77226,77227,77228,77229,77230,77231,77232,77234,77239,77240,77241,78172,78177,78182';
SET @LFR_ITEMS    := '77973,77979,77980,77981,77982,77983,78454,78455,78456,78457,78458,78460,78494,78495,78496,78497,78498,78862,78863,78864,78865,78866,78867,78868,78869,78870,78871,78872,78873,78874,78875,78876';
SET @HEROIC_ITEMS := '77993,77999,78000,78001,78002,78003,78445,78446,78447,78448,78449,78450,78451,78452,78489,78490,78491,78492,78493,78850,78851,78852';
SET @SHARED_ITEMS := '71998,77952';

-- Original TDB rows inlined so the split below is idempotent
DELETE FROM `creature_loot_template` WHERE `Entry` = 56427;
INSERT INTO `creature_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`) VALUES
(56427,71998,0,100,0,0,1,0,1,3,NULL),
(56427,77202,0,24.7719,0,0,1,0,1,1,NULL),
(56427,77207,0,1.0671,0,0,1,0,1,1,NULL),
(56427,77208,0,1.1269,0,0,1,0,1,1,NULL),
(56427,77209,0,1.2565,0,0,1,0,1,1,NULL),
(56427,77210,0,1.092,0,0,1,0,1,1,NULL),
(56427,77211,0,1.3962,0,0,1,0,1,1,NULL),
(56427,77224,0,23.0167,0,0,1,0,1,1,NULL),
(56427,77225,0,21.3613,0,0,1,0,1,1,NULL),
(56427,77226,0,17.2077,0,0,1,0,1,1,NULL),
(56427,77227,0,17.0481,0,0,1,0,1,1,NULL),
(56427,77228,0,1.1419,0,0,1,0,1,1,NULL),
(56427,77229,0,1.2067,0,0,1,0,1,1,NULL),
(56427,77230,0,1.1768,0,0,1,0,1,1,NULL),
(56427,77231,0,1.2914,0,0,1,0,1,1,NULL),
(56427,77232,0,1.2865,0,0,1,0,1,1,NULL),
(56427,77234,0,17.7811,0,0,1,0,1,1,NULL),
(56427,77239,0,17.3373,0,0,1,0,1,1,NULL),
(56427,77240,0,19.0327,0,0,1,0,1,1,NULL),
(56427,77241,0,18.6537,0,0,1,0,1,1,NULL),
(56427,77952,0,100,0,0,1,0,1,3,NULL),
(56427,77973,0,25.6295,0,0,1,0,1,1,NULL),
(56427,77979,0,1.0272,0,0,1,0,1,1,NULL),
(56427,77980,0,0.9823,0,0,1,0,1,1,NULL),
(56427,77981,0,1.0471,0,0,1,0,1,1,NULL),
(56427,77982,0,1.0222,0,0,1,0,1,1,NULL),
(56427,77983,0,0.8626,0,0,1,0,1,1,NULL),
(56427,77993,0,4.7519,0,0,1,0,1,1,NULL),
(56427,77999,0,0.1945,0,0,1,0,1,1,NULL),
(56427,78000,0,0.2294,0,0,1,0,1,1,NULL),
(56427,78001,0,0.2344,0,0,1,0,1,1,NULL),
(56427,78002,0,0.1795,0,0,1,0,1,1,NULL),
(56427,78003,0,0.2892,0,0,1,0,1,1,NULL),
(56427,78172,0,64.2583,0,0,1,0,1,1,NULL),
(56427,78177,0,52.8945,0,0,1,0,1,1,NULL),
(56427,78182,0,55.3827,0,0,1,0,1,1,NULL),
(56427,78445,0,4.3131,0,0,1,0,1,1,NULL),
(56427,78446,0,3.7198,0,0,1,0,1,1,NULL),
(56427,78447,0,4.0788,0,0,1,0,1,1,NULL),
(56427,78448,0,3.261,0,0,1,0,1,1,NULL),
(56427,78449,0,3.6699,0,0,1,0,1,1,NULL),
(56427,78450,0,2.9918,0,0,1,0,1,1,NULL),
(56427,78451,0,3.9741,0,0,1,0,1,1,NULL),
(56427,78452,0,3.2909,0,0,1,0,1,1,NULL),
(56427,78454,0,18.4044,0,0,1,0,1,1,NULL),
(56427,78455,0,16.1207,0,0,1,0,1,1,NULL),
(56427,78456,0,15.2182,0,0,1,0,1,1,NULL),
(56427,78457,0,16.8836,0,0,1,0,1,1,NULL),
(56427,78458,0,15.2182,0,0,1,0,1,1,NULL),
(56427,78460,0,16.2603,0,0,1,0,1,1,NULL),
(56427,78489,0,0.1945,0,0,1,0,1,1,NULL),
(56427,78490,0,0.1895,0,0,1,0,1,1,NULL),
(56427,78491,0,0.2792,0,0,1,0,1,1,NULL),
(56427,78492,0,0.3391,0,0,1,0,1,1,NULL),
(56427,78493,0,0.2344,0,0,1,0,1,1,NULL),
(56427,78494,0,1.0421,0,0,1,0,1,1,NULL),
(56427,78495,0,0.8377,0,0,1,0,1,1,NULL),
(56427,78496,0,0.9624,0,0,1,0,1,1,NULL),
(56427,78497,0,0.9225,0,0,1,0,1,1,NULL),
(56427,78498,0,1.1419,0,0,1,0,1,1,NULL),
(56427,78850,0,8.3869,0,0,1,0,1,1,NULL),
(56427,78851,0,8.2124,0,0,1,0,1,1,NULL),
(56427,78852,0,9.9377,0,0,1,0,1,1,NULL),
(56427,78862,0,3.5851,0,0,1,0,1,1,NULL),
(56427,78863,0,3.2461,0,0,1,0,1,1,NULL),
(56427,78864,0,2.5679,0,0,1,0,1,1,NULL),
(56427,78865,0,3.281,0,0,1,0,1,1,NULL),
(56427,78866,0,2.2688,0,0,1,0,1,1,NULL),
(56427,78867,0,2.5031,0,0,1,0,1,1,NULL),
(56427,78868,0,57.6016,0,0,1,0,1,1,NULL),
(56427,78869,0,49.8479,0,0,1,0,1,1,NULL),
(56427,78870,0,45.1957,0,0,1,0,1,1,NULL),
(56427,78871,0,3.625,0,0,1,0,1,1,NULL),
(56427,78872,0,2.6328,0,0,1,0,1,1,NULL),
(56427,78873,0,2.4233,0,0,1,0,1,1,NULL),
(56427,78874,0,3.7198,0,0,1,0,1,1,NULL),
(56427,78875,0,3.1164,0,0,1,0,1,1,NULL),
(56427,78876,0,2.7674,0,0,1,0,1,1,NULL);

DELETE FROM `creature_loot_template` WHERE `Entry` IN (57699, 57847, 57848);

-- 25 Normal: 397 loot
INSERT INTO `creature_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT 57699, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, 1, `GroupId`, `MinCount`, `MaxCount`, 'Blackhorn 25N'
FROM `creature_loot_template` WHERE `Entry` = 56427 AND FIND_IN_SET(`Item`, @NORMAL_ITEMS);

-- 25 Normal table: LFR loot as LootMode 2
INSERT INTO `creature_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT 57699, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, 2, `GroupId`, `MinCount`, `MaxCount`, 'Blackhorn LFR'
FROM `creature_loot_template` WHERE `Entry` = 56427 AND FIND_IN_SET(`Item`, @LFR_ITEMS);

-- 25 Normal table: shared drops available in both loot modes
INSERT INTO `creature_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT 57699, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, 3, `GroupId`, `MinCount`, `MaxCount`, 'Blackhorn shared'
FROM `creature_loot_template` WHERE `Entry` = 56427 AND FIND_IN_SET(`Item`, @SHARED_ITEMS);

-- Heroic tables
INSERT INTO `creature_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT 57847, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, 1, `GroupId`, `MinCount`, `MaxCount`, 'Blackhorn 10H'
FROM `creature_loot_template` WHERE `Entry` = 56427 AND (FIND_IN_SET(`Item`, @HEROIC_ITEMS) OR FIND_IN_SET(`Item`, @SHARED_ITEMS));

INSERT INTO `creature_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT 57848, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, 1, `GroupId`, `MinCount`, `MaxCount`, 'Blackhorn 25H'
FROM `creature_loot_template` WHERE `Entry` = 56427 AND (FIND_IN_SET(`Item`, @HEROIC_ITEMS) OR FIND_IN_SET(`Item`, @SHARED_ITEMS));

-- 10 Normal keeps only 397 + shared
DELETE FROM `creature_loot_template` WHERE `Entry` = 56427 AND (FIND_IN_SET(`Item`, @LFR_ITEMS) OR FIND_IN_SET(`Item`, @HEROIC_ITEMS));

UPDATE `creature_template` SET `lootid` = `entry` WHERE `entry` IN (57699, 57847, 57848);
