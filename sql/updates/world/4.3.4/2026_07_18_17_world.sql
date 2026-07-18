-- ============================================================================
-- Abyssal Depths batch C — sniffed L'ghorek arcs (AB1: Darkbreak Cove/Promontory
-- Point 25987-26132; AB2: L'ghorek interior + finale 26140-26194)
-- Fork: ShatterCore 4.3.4. Idempotent. Guids: creature 9001500-9001562,
-- GO 9001120-9001135. Custom phases: 233 (breach battle), 234 (breach aftermath).
-- Serverside spells: 123460-123462; spelleffect_dbc Ids 160140-160147.
-- ============================================================================

-- ----------------------------------------------------------------------------
-- 1) QUEST CHAIN FIXES
-- ----------------------------------------------------------------------------
-- 26143 "All that Rises": RewardSpell 79052 is 5.5.3-only (serverside stub, no
-- effects) - never client-cast it. Torrent cleanup handled via SAI on 42197.
UPDATE `quest_template` SET `RewardSpell`=0 WHERE `ID`=26143 AND `RewardSpell`=79052;

-- 26193/26194 "Defending the Rift": sniff-faithful objective = 42819 x15
-- (ObjectiveText1 "Fight in the Battle for the Abyssal Breach" masks the name).
-- 42565 Camera Bunny never spawns in the modern sniff.
UPDATE `quest_template` SET `RequiredNpcOrGo1`=42819, `RequiredNpcOrGoCount1`=15 WHERE `ID` IN (26193,26194);
-- Phase flips depend on quest state (taken -> battle 233, complete -> +234,
-- rewarded -> both drop): need QUEST_FLAGS_UPDATE_PHASESHIFT.
UPDATE `quest_template` SET `Flags`=`Flags`|0x400000 WHERE `ID` IN (26193,26194);

-- 26140 "Communing with the Ancient": player casts 78729 Merciless Disguise on
-- accept (sniffed AB:1558522).
UPDATE `quest_template_addon` SET `SourceSpellID`=78729 WHERE `ID`=26140;

-- Gate the finale behind the zone epilogue travel quests (sniffed accept order;
-- without this every Erunak spawn offers Defending the Rift prematurely)
UPDATE `quest_template_addon` SET `PrevQuestID`=26181 WHERE `ID`=26193 AND `PrevQuestID`=0;
UPDATE `quest_template_addon` SET `PrevQuestID`=26182 WHERE `ID`=26194 AND `PrevQuestID`=0;

-- ----------------------------------------------------------------------------
-- 2) CREATURE TEMPLATE FIXES
-- ----------------------------------------------------------------------------
-- 26070 shared credit (sniff-proven; idempotent re-assert)
UPDATE `creature_template` SET `KillCredit1`=41646 WHERE `entry` IN (41645,41647);
-- 26193/26194 credit funnel: warriors/maulers/shadoweavers count as 42819
UPDATE `creature_template` SET `KillCredit1`=42819 WHERE `entry` IN (42818,42821,42370);
-- Put It On: sniffed 5.5.3 vehicle kits (both exist in 4.3.4 Vehicle.dbc)
UPDATE `creature_template` SET `VehicleId`=842 WHERE `entry`=41840;  -- Merciless One (player clone carrier)
UPDATE `creature_template` SET `VehicleId`=841 WHERE `entry`=41814;  -- Merciless One in Control of You
-- Twilight Extermination: Possessed Torrent = kit 1342 (SMSG_SET_VEHICLE_REC_ID
-- at dismount AB:3016543) + sniff-ordered pre-release ability bar
UPDATE `creature_template` SET `VehicleId`=1342,
  `spell1`=78972, `spell2`=78968, `spell3`=79213, `spell4`=79012, `spell5`=90677, `spell6`=0
  WHERE `entry`=42325;
-- All that Rises: Vengeful Torrent (post-26143 ride target for C++ swap script)
UPDATE `creature_template` SET `VehicleId`=1342,
  `spell1`=79222, `spell2`=79223, `spell3`=78968, `spell4`=78972, `spell5`=79224, `spell6`=90677
  WHERE `entry`=48620;
-- Bound Torrents are passive rooted channel targets, not combatants
UPDATE `creature_template` SET `faction`=35 WHERE `entry`=47969;
-- SmartAI enable for entries scripted below (only where currently unscripted)
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `AIName`='' AND `ScriptName`='' AND `entry` IN
  (41666,42197,42225,42234,42054,42128,42818,42822,42370,41814,41837,41840,41884,41889,47090,47094,42213,50259,42325,48620,44490,44540);

-- ----------------------------------------------------------------------------
-- 3) CREATURE TEMPLATE ADDON (persistent auras)
-- ----------------------------------------------------------------------------
-- Clearing the Defiled cosmetics: 78342 Defile Me (area aura) + 78344 Defiled Visual
UPDATE `creature_template_addon` SET `auras`='78342 78344' WHERE `entry` IN (41645,41646,41647);
-- Possessed/Vengeful Torrent flight aura (serverside 79001, dummy effect added below)
UPDATE `creature_template_addon` SET `auras`='79001' WHERE `entry` IN (42325,48620);
-- Horde Prisoner mirrors Alliance Prisoner kneel tier
UPDATE `creature_template_addon` SET `AnimTier`=1 WHERE `entry`=42234;

-- ----------------------------------------------------------------------------
-- 4) SERVERSIDE SPELLS (spell_dbc / spelleffect_dbc)
-- ----------------------------------------------------------------------------
DELETE FROM `spell_dbc` WHERE `Id` IN (123460,123461,123462);
INSERT INTO `spell_dbc` (`Id`,`Attributes`,`AttributesEx`,`AttributesEx2`,`AttributesEx3`,`AttributesEx4`,`AttributesEx5`,`AttributesEx6`,`AttributesEx7`,`AttributesEx8`,`AttributesEx9`,`AttributesEx10`,`CastingTimeIndex`,`DurationIndex`,`RangeIndex`,`SchoolMask`,`SpellAuraOptionsId`,`SpellCastingRequirementsId`,`SpellCategoriesId`,`SpellClassOptionsId`,`SpellEquippedItemsId`,`SpellInterruptsId`,`SpellLevelsId`,`SpellTargetRestrictionsId`,`SpellName`) VALUES
(123460,2843738368,268436512,0,0,0,0,0,0,0,0,0,1,36,13,0,0,0,0,0,0,0,0,0,'(Serverside/Non-DB2) Put It On: Quest Complete <Do Not Translate>'),
(123461,2843738368,268436512,0,0,0,0,0,0,0,0,0,1,36,13,0,0,0,0,0,0,0,0,0,'(Serverside/Non-DB2) Create Attuned Runestone of Binding <Do Not Translate>'),
(123462,2843738368,268436512,0,0,0,0,0,0,0,0,0,1,25,13,0,0,0,0,0,0,0,0,0,'(Serverside/Non-DB2) Defending the Rift: Summon Captain Taylor <Do Not Translate>');

-- Effects: 78727/78822 cage-open (kill credit + release-dummy on caged prisoner),
-- 79001 flight aura dummy, 123460 quest-complete 25987, 123461 create item 57172,
-- 123462 summon 50259 (SummonProperties 64, 180s cap, SAI despawns earlier).
DELETE FROM `spelleffect_dbc` WHERE `SpellID` IN (78727,78822,79001,123460,123461,123462) OR `Id` BETWEEN 160140 AND 160147;
INSERT INTO `spelleffect_dbc` (`Id`,`Effect`,`EffectAmplitude`,`EffectAura`,`EffectAuraPeriod`,`EffectBasePoints`,`EffectBonusCoefficient`,`EffectChainAmplitude`,`EffectChainTargets`,`EffectDieSides`,`EffectItemType`,`EffectMechanic`,`EffectMiscValue`,`EffectMiscValueB`,`EffectPointsPerResource`,`EffectRadiusIndex`,`EffectRadiusMaxIndex`,`EffectRealPointsPerLevel`,`EffectSpellClassMaskA`,`EffectSpellClassMaskB`,`EffectSpellClassMaskC`,`EffectTriggerSpell`,`EffectImplicitTargetA`,`EffectImplicitTargetB`,`SpellID`,`EffectIndex`,`Comment`) VALUES
(160140, 90,0,0,0,1,0,1,0,0,0,0,42225,0,0,0,0,0,0,0,0,0, 1,0,78727,0,'Prisoners: Cage Opened - Alliance - kill credit 42225'),
(160141,  3,0,0,0,0,0,1,0,0,0,0,    0,0,0,0,0,0,0,0,0,0,46,0,78727,1,'Prisoners: Cage Opened - Alliance - dummy hit on caged prisoner'),
(160142, 90,0,0,0,1,0,1,0,0,0,0,42234,0,0,0,0,0,0,0,0,0, 1,0,78822,0,'Prisoners: Cage Opened - Horde - kill credit 42234'),
(160143,  3,0,0,0,0,0,1,0,0,0,0,    0,0,0,0,0,0,0,0,0,0,46,0,78822,1,'Prisoners: Cage Opened - Horde - dummy hit on caged prisoner'),
(160144,  6,0,4,0,0,0,1,0,0,0,0,    0,0,0,0,0,0,0,0,0,0, 1,0,79001,0,'Twilight Extermination: Possessed Torrent - Flight Aura (dummy)'),
(160145, 16,0,0,0,0,0,1,0,0,0,0,25987,0,0,0,0,0,0,0,0,0, 1,0,123460,0,'Put It On - complete quest 25987'),
(160146, 24,0,0,0,1,0,1,0,0,57172,0,  0,0,0,0,0,0,0,0,0,0, 1,0,123461,0,'Create Attuned Runestone of Binding (gossip 11607/0 re-grant)'),
(160147, 28,0,0,0,1,0,1,0,0,0,0,50259,64,0,0,0,0,0,0,0,0, 1,0,123462,0,'Defending the Rift - summon post-battle Captain Taylor 50259');

-- ----------------------------------------------------------------------------
-- 5) SPELL_AREA — Merciless Disguise held only while 26140 active, zone 5145
-- ----------------------------------------------------------------------------
DELETE FROM `spell_area` WHERE `spell`=78729 AND `area`=5145;
INSERT INTO `spell_area` (`spell`,`area`,`quest_start`,`quest_end`,`aura_spell`,`racemask`,`gender`,`flags`,`quest_start_status`,`quest_end_status`) VALUES
(78729,5145,26140,0,0,0,2,3,10,11);

-- ----------------------------------------------------------------------------
-- 6) LOOT — the Brain of the Unfathomable is quest loot (26111)
-- ----------------------------------------------------------------------------
UPDATE `creature_loot_template` SET `QuestRequired`=1 WHERE `Entry`=41648 AND `Item`=56822;

-- ----------------------------------------------------------------------------
-- 7) SPAWN DELETES (rows owned by this batch)
-- ----------------------------------------------------------------------------
-- Static Ick'thys spawn converted to per-event summon by 42128 controller
DELETE FROM `creature` WHERE `guid`=348675 AND `id`=41648;
-- Hallazeal dup + stray (sniff shows exactly one, at 347378's position)
DELETE FROM `creature` WHERE `guid` IN (348444,347763) AND `id`=41659;
-- NOTE: both 42197 L'ghorek spawns (348221/348222) are retail-genuine (AB2 sniff:
-- both answer SMSG_QUEST_GIVER_STATUS) - intentionally NOT deleted.

-- Fast respawns where quests need them
UPDATE `creature` SET `spawntimesecs`=60 WHERE `id`=41644;   -- Faceless Defiler (26072: 5 credits from 7 spawns)
UPDATE `creature` SET `spawntimesecs`=90 WHERE `id`=47969;   -- Bound Torrent (despawns on possession)

-- ----------------------------------------------------------------------------
-- 8) NEW CREATURE SPAWNS (guids 9001500-9001562)
-- ----------------------------------------------------------------------------
DELETE FROM `creature` WHERE `guid` BETWEEN 9001500 AND 9001599;
INSERT INTO `creature` (`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseUseFlags`,`phaseMask`,`PhaseId`,`PhaseGroup`,`terrainSwapMap`,`modelid`,`equipment_id`,`position_x`,`position_y`,`position_z`,`orientation`,`spawntimesecs`,`wander_distance`,`currentwaypoint`,`curhealth`,`curmana`,`MovementType`,`npcflag`,`unit_flags`,`dynamicflags`,`ScriptName`,`VerifiedBuild`) VALUES
-- Promontory Point trio (sniffed create-block positions, phase 169)
(9001500,41598,0,5145,5100,1,0,1,169,0,-1,0,0,-5937.39,6490.73,-835.72,2.374,300,0,0,1,0,0,0,0,0,'',0),
(9001501,41600,0,5145,5100,1,0,1,169,0,-1,0,0,-5936.12,6493.86,-835.49,2.391,300,0,0,1,0,0,0,0,0,'',0),
(9001502,41639,0,5145,5100,1,0,1,169,0,-1,0,0,-5936.59,6492.24,-835.57,2.374,300,0,0,1,0,0,0,0,0,'',0),
-- Ick'thys event controller (ELM bunny scale x4, sniffed position)
(9001503,42128,0,5145,5103,1,0,1,169,0,-1,0,0,-5648.52,6311.88,-1079.50,2.67,300,0,0,1,0,0,0,0,0,'',0),
-- Erunak Stonespeaker at Darkbreak Cove (26181 turn-in / 26193 giver, sniffed)
(9001504,41600,0,5145,4976,1,0,1,169,0,-1,0,0,-6889.01,5963.42,-765.38,1.30,300,0,0,1,0,0,0,0,0,'',0),
-- Horde Prisoners x16 (mirror of 42225 cage spots, +1.8/+1.8 offset)
(9001505,42234,0,5145,4971,1,0,1,169,0,-1,0,0,-6687.12,6951.44,-798.068,4.64258,300,0,0,1,0,0,0,0,0,'',0),
(9001506,42234,0,5145,4971,1,0,1,169,0,-1,0,0,-6655.09,6929.47,-803.154,4.39823,300,0,0,1,0,0,0,0,0,'',0),
(9001507,42234,0,5145,4971,1,0,1,169,0,-1,0,0,-6653.22,6865.96,-805.34,6.05629,300,0,0,1,0,0,0,0,0,'',0),
(9001508,42234,0,5145,4971,1,0,1,169,0,-1,0,0,-6602.19,7114.95,-792.013,5.49779,300,0,0,1,0,0,0,0,0,'',0),
(9001509,42234,0,5145,4971,1,0,1,169,0,-1,0,0,-6622.30,7071.73,-789.382,1.43117,300,0,0,1,0,0,0,0,0,'',0),
(9001510,42234,0,5145,4971,1,0,1,169,0,-1,0,0,-6617.54,6801.56,-806.903,2.32129,300,0,0,1,0,0,0,0,0,'',0),
(9001511,42234,0,5145,4971,1,0,1,169,0,-1,0,0,-6531.78,6837.68,-813.413,2.14675,300,0,0,1,0,0,0,0,0,'',0),
(9001512,42234,0,5145,4971,1,0,1,169,0,-1,0,0,-6561.69,6961.18,-805.035,5.51524,300,0,0,1,0,0,0,0,0,'',0),
(9001513,42234,0,5145,4971,1,0,1,169,0,-1,0,0,-6517.18,6963.79,-804.007,5.74213,300,0,0,1,0,0,0,0,0,'',0),
(9001514,42234,0,5145,4971,1,0,1,169,0,-1,0,0,-6536.87,7008.46,-804.356,0.366519,300,0,0,1,0,0,0,0,0,'',0),
(9001515,42234,0,5145,4971,1,0,1,169,0,-1,0,0,-6559.50,7126.30,-797.689,3.9968,300,0,0,1,0,0,0,0,0,'',0),
(9001516,42234,0,5145,4971,1,0,1,169,0,-1,0,0,-6424.78,7164.30,-798.29,4.39823,300,0,0,1,0,0,0,0,0,'',0),
(9001517,42234,0,5145,4971,1,0,1,169,0,-1,0,0,-6427.18,7083.11,-810.711,0.959931,300,0,0,1,0,0,0,0,0,'',0),
(9001518,42234,0,5145,4971,1,0,1,169,0,-1,0,0,-6472.34,7044.40,-812.91,3.54302,300,0,0,1,0,0,0,0,0,'',0),
(9001519,42234,0,5145,4971,1,0,1,169,0,-1,0,0,-6398.76,7007.95,-813.367,2.75762,300,0,0,1,0,0,0,0,0,'',0),
(9001520,42234,0,5145,4971,1,0,1,169,0,-1,0,0,-6389.34,7131.70,-803.692,3.52556,300,0,0,1,0,0,0,0,0,'',0),
-- Abyssal Breach aftermath (phase 234): turn-in NPCs, Taylor at sniffed spot
(9001521,44490,0,5145,5047,1,0,1,234,0,-1,0,0,-5841.18,5390.71,-1213.89,0.35,300,0,0,1,0,0,0,0,0,'',0),
(9001522,44540,0,5145,5047,1,0,1,234,0,-1,0,0,-5838.20,5393.70,-1213.89,0.55,300,0,0,1,0,0,0,0,0,'',0),
-- Abyssal Breach battle (phase 233): Revenant of Neptulon defensive arc (12)
(9001523,42822,0,5145,5047,1,0,1,233,0,-1,0,0,-5834.71,5366.56,-1215.0,4.974,90,0,0,1,0,0,0,0,0,'',0),
(9001524,42822,0,5145,5047,1,0,1,233,0,-1,0,0,-5829.21,5368.76,-1215.2,5.212,90,0,0,1,0,0,0,0,0,'',0),
(9001525,42822,0,5145,5047,1,0,1,233,0,-1,0,0,-5824.36,5372.22,-1215.4,5.451,90,0,0,1,0,0,0,0,0,'',0),
(9001526,42822,0,5145,5047,1,0,1,233,0,-1,0,0,-5820.46,5376.73,-1215.6,5.690,90,0,0,1,0,0,0,0,0,'',0),
(9001527,42822,0,5145,5047,1,0,1,233,0,-1,0,0,-5817.76,5381.95,-1215.8,5.925,90,0,0,1,0,0,0,0,0,'',0),
(9001528,42822,0,5145,5047,1,0,1,233,0,-1,0,0,-5816.36,5387.75,-1216.0,6.165,90,0,0,1,0,0,0,0,0,'',0),
(9001529,42822,0,5145,5047,1,0,1,233,0,-1,0,0,-5816.36,5393.67,-1216.0,0.119,90,0,0,1,0,0,0,0,0,'',0),
(9001530,42822,0,5145,5047,1,0,1,233,0,-1,0,0,-5817.76,5399.47,-1215.8,0.358,90,0,0,1,0,0,0,0,0,'',0),
(9001531,42822,0,5145,5047,1,0,1,233,0,-1,0,0,-5820.46,5404.69,-1215.6,0.593,90,0,0,1,0,0,0,0,0,'',0),
(9001532,42822,0,5145,5047,1,0,1,233,0,-1,0,0,-5824.36,5409.20,-1215.4,0.833,90,0,0,1,0,0,0,0,0,'',0),
(9001533,42822,0,5145,5047,1,0,1,233,0,-1,0,0,-5829.21,5412.66,-1215.2,1.072,90,0,0,1,0,0,0,0,0,'',0),
(9001534,42822,0,5145,5047,1,0,1,233,0,-1,0,0,-5834.71,5414.86,-1215.0,1.309,90,0,0,1,0,0,0,0,0,'',0),
-- Azsh'ir Depthseeker x15 (three assault clusters east of the arc)
(9001535,42819,0,5145,5047,1,0,1,233,0,-1,0,0,-5801.0,5377.0,-1220.0,3.1,75,0,0,1,0,0,0,0,0,'',0),
(9001536,42819,0,5145,5047,1,0,1,233,0,-1,0,0,-5797.0,5380.0,-1220.0,3.1,75,0,0,1,0,0,0,0,0,'',0),
(9001537,42819,0,5145,5047,1,0,1,233,0,-1,0,0,-5804.0,5382.0,-1220.0,3.1,75,0,0,1,0,0,0,0,0,'',0),
(9001538,42819,0,5145,5047,1,0,1,233,0,-1,0,0,-5796.0,5373.0,-1220.0,3.1,75,0,0,1,0,0,0,0,0,'',0),
(9001539,42819,0,5145,5047,1,0,1,233,0,-1,0,0,-5806.0,5374.0,-1220.0,3.1,75,0,0,1,0,0,0,0,0,'',0),
(9001540,42819,0,5145,5047,1,0,1,233,0,-1,0,0,-5799.0,5407.0,-1220.0,3.1,75,0,0,1,0,0,0,0,0,'',0),
(9001541,42819,0,5145,5047,1,0,1,233,0,-1,0,0,-5795.0,5404.0,-1220.0,3.1,75,0,0,1,0,0,0,0,0,'',0),
(9001542,42819,0,5145,5047,1,0,1,233,0,-1,0,0,-5803.0,5411.0,-1220.0,3.1,75,0,0,1,0,0,0,0,0,'',0),
(9001543,42819,0,5145,5047,1,0,1,233,0,-1,0,0,-5793.0,5409.0,-1220.0,3.1,75,0,0,1,0,0,0,0,0,'',0),
(9001544,42819,0,5145,5047,1,0,1,233,0,-1,0,0,-5801.0,5401.0,-1220.0,3.1,75,0,0,1,0,0,0,0,0,'',0),
(9001545,42819,0,5145,5047,1,0,1,233,0,-1,0,0,-5807.0,5433.0,-1222.0,3.1,75,0,0,1,0,0,0,0,0,'',0),
(9001546,42819,0,5145,5047,1,0,1,233,0,-1,0,0,-5802.0,5435.0,-1222.0,3.1,75,0,0,1,0,0,0,0,0,'',0),
(9001547,42819,0,5145,5047,1,0,1,233,0,-1,0,0,-5811.0,5430.0,-1222.0,3.1,75,0,0,1,0,0,0,0,0,'',0),
(9001548,42819,0,5145,5047,1,0,1,233,0,-1,0,0,-5804.0,5439.0,-1222.0,3.1,75,0,0,1,0,0,0,0,0,'',0),
(9001549,42819,0,5145,5047,1,0,1,233,0,-1,0,0,-5813.0,5437.0,-1222.0,3.1,75,0,0,1,0,0,0,0,0,'',0),
-- Azsh'ir Warrior x8
(9001550,42818,0,5145,5047,1,0,1,233,0,-1,0,0,-5793.0,5377.0,-1220.0,3.1,75,0,0,1,0,0,0,0,0,'',0),
(9001551,42818,0,5145,5047,1,0,1,233,0,-1,0,0,-5799.0,5370.0,-1220.0,3.1,75,0,0,1,0,0,0,0,0,'',0),
(9001552,42818,0,5145,5047,1,0,1,233,0,-1,0,0,-5803.0,5385.0,-1220.0,3.1,75,0,0,1,0,0,0,0,0,'',0),
(9001553,42818,0,5145,5047,1,0,1,233,0,-1,0,0,-5791.0,5410.0,-1220.0,3.1,75,0,0,1,0,0,0,0,0,'',0),
(9001554,42818,0,5145,5047,1,0,1,233,0,-1,0,0,-5798.0,5414.0,-1220.0,3.1,75,0,0,1,0,0,0,0,0,'',0),
(9001555,42818,0,5145,5047,1,0,1,233,0,-1,0,0,-5805.0,5405.0,-1220.0,3.1,75,0,0,1,0,0,0,0,0,'',0),
(9001556,42818,0,5145,5047,1,0,1,233,0,-1,0,0,-5800.0,5431.0,-1222.0,3.1,75,0,0,1,0,0,0,0,0,'',0),
(9001557,42818,0,5145,5047,1,0,1,233,0,-1,0,0,-5808.0,5426.0,-1222.0,3.1,75,0,0,1,0,0,0,0,0,'',0),
-- Faceless Mauler x3 + Faceless Shadoweaver x2 (deeper field)
(9001558,42821,0,5145,5047,1,0,1,233,0,-1,0,0,-5803.0,5428.0,-1222.0,3.1,75,0,0,1,0,0,0,0,0,'',0),
(9001559,42821,0,5145,5047,1,0,1,233,0,-1,0,0,-5745.0,5350.0,-1228.0,3.5,75,0,0,1,0,0,0,0,0,'',0),
(9001560,42821,0,5145,5047,1,0,1,233,0,-1,0,0,-5762.0,5432.0,-1226.0,3.0,75,0,0,1,0,0,0,0,0,'',0),
(9001561,42370,0,5145,5047,1,0,1,233,0,-1,0,0,-5729.0,5391.0,-1230.0,3.14,75,0,0,1,0,0,0,0,0,'',0),
(9001562,42370,0,5145,5047,1,0,1,233,0,-1,0,0,-5771.0,5311.0,-1228.0,2.6,75,0,0,1,0,0,0,0,0,'',0);

-- ----------------------------------------------------------------------------
-- 9) NEW GO SPAWNS — Horde cages 203709 x16 (mirror 203705, +1.8/+1.8 offset)
-- ----------------------------------------------------------------------------
DELETE FROM `gameobject` WHERE `guid` BETWEEN 9001120 AND 9001149;
INSERT INTO `gameobject` (`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseUseFlags`,`phaseMask`,`PhaseId`,`PhaseGroup`,`terrainSwapMap`,`position_x`,`position_y`,`position_z`,`orientation`,`rotation0`,`rotation1`,`rotation2`,`rotation3`,`spawntimesecs`,`animprogress`,`state`,`ScriptName`,`VerifiedBuild`) VALUES
(9001120,203709,0,5145,4971,1,0,1,169,0,-1,-6531.75,6837.64,-813.611,2.16421,0,0,0.882948,0.469471,300,100,1,'',0),
(9001121,203709,0,5145,4971,1,0,1,169,0,-1,-6653.40,6866.02,-805.622,6.12611,0,0,0.0784569,-0.996917,300,100,1,'',0),
(9001122,203709,0,5145,4971,1,0,1,169,0,-1,-6617.48,6801.45,-807.127,2.32129,0,0,0.91706,0.398748,300,100,1,'',0),
(9001123,203709,0,5145,4971,1,0,1,169,0,-1,-6655.01,6929.63,-803.412,4.31097,0,0,0.833884,-0.55194,300,100,1,'',0),
(9001124,203709,0,5145,4971,1,0,1,169,0,-1,-6472.27,7044.38,-813.167,3.47321,0,0,0.986285,-0.16505,300,100,1,'',0),
(9001125,203709,0,5145,4971,1,0,1,169,0,-1,-6559.45,7126.33,-797.9,3.97936,0,0,0.913544,-0.406741,300,100,1,'',0),
(9001126,203709,0,5145,4971,1,0,1,169,0,-1,-6536.89,7008.44,-804.498,0.418879,0,0,0.207912,0.978148,300,100,1,'',0),
(9001127,203709,0,5145,4971,1,0,1,169,0,-1,-6687.11,6951.53,-798.315,4.60767,0,0,0.743145,-0.669131,300,100,1,'',0),
(9001128,203709,0,5145,4971,1,0,1,169,0,-1,-6427.23,7083.11,-810.884,0.925024,0,0,0.446198,0.894934,300,100,1,'',0),
(9001129,203709,0,5145,4971,1,0,1,169,0,-1,-6602.18,7115.11,-792.496,5.42798,0,0,0.414691,-0.909963,300,100,1,'',0),
(9001130,203709,0,5145,4971,1,0,1,169,0,-1,-6398.71,7007.92,-813.543,2.77507,0,0,0.983255,0.182237,300,100,1,'',0),
(9001131,203709,0,5145,4971,1,0,1,169,0,-1,-6561.74,6961.24,-805.169,5.49779,0,0,0.382682,-0.92388,300,100,1,'',0),
(9001132,203709,0,5145,4971,1,0,1,169,0,-1,-6517.17,6963.79,-804.196,5.74214,0,0,0.267235,-0.963631,300,100,1,'',0),
(9001133,203709,0,5145,4971,1,0,1,169,0,-1,-6622.29,7071.81,-789.603,1.43117,0,0,0.656059,0.75471,300,100,1,'',0),
(9001134,203709,0,5145,4971,1,0,1,169,0,-1,-6424.78,7164.29,-798.647,4.31097,0,0,0.833884,-0.55194,300,100,1,'',0),
(9001135,203709,0,5145,4971,1,0,1,169,0,-1,-6389.23,7131.71,-803.952,3.52557,0,0,0.981627,-0.190811,300,100,1,'',0);

-- ----------------------------------------------------------------------------
-- 10) PHASING — Abyssal Breach battle (233) / aftermath (234), area 5047
--     Mirrors sniffed 5.5.3 phases 3398 (battle) + 3401 (aftermath overlay).
-- ----------------------------------------------------------------------------
DELETE FROM `phase_area` WHERE `AreaId`=5047 AND `PhaseId` IN (233,234);
INSERT INTO `phase_area` (`AreaId`,`PhaseId`,`Comment`) VALUES
(5047,233,'Abyssal Breach - Defending the Rift battle while 26193/26194 taken or complete'),
(5047,234,'Abyssal Breach - Defending the Rift aftermath (Taylor/Nazgrim) while complete');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=26 AND `SourceGroup` IN (233,234) AND `SourceEntry`=5047;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(26,233,5047,0,0,47,0,26193,10,0,0,0,0,'','Breach battle: Defending the Rift (A) taken or complete'),
(26,233,5047,0,1,47,0,26194,10,0,0,0,0,'','Breach battle: Defending the Rift (H) taken or complete'),
(26,234,5047,0,0,47,0,26193,2,0,0,0,0,'','Breach aftermath: Defending the Rift (A) complete'),
(26,234,5047,0,1,47,0,26194,2,0,0,0,0,'','Breach aftermath: Defending the Rift (H) complete');

-- ----------------------------------------------------------------------------
-- 11) CONDITIONS — spell targets, gossip gates, SAI gates
-- ----------------------------------------------------------------------------
-- 11a) Implicit-target anchors (all scene/ambience spells use NEARBY_ENTRY dests)
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry` IN (78021,78083,78085,87752,87760,78008,77988,78523,78605,78850,78766,78977,78727,78822);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(13,1,78021,0,0,31,0,3,41666,0,0,0,0,'','Put It On: Hexascrub copy summoned at real Hexascrub'),
(13,1,78083,0,0,31,0,3,41665,0,0,0,0,'','Put It On: Jorlan copy summoned at real Jorlan Trueblade'),
(13,1,78085,0,0,31,0,3,41667,0,0,0,0,'','Put It On: Foxy Topper copy summoned at real Foxy Topper'),
(13,1,87752,0,0,31,0,3,42967,0,0,0,0,'','Put It On: Rallings copy summoned at real Quartermaster Rallings'),
(13,1,87760,0,0,31,0,3,42974,0,0,0,0,'','Put It On: Darkbreak Guard copy summoned at real Darkbreak Guard'),
(13,1,78008,0,0,31,0,3,41666,0,0,0,0,'','Put It On: player clone (Merciless One Controlled You) at Hexascrub'),
(13,1,77988,0,0,31,0,3,41840,0,0,0,0,'','Put It On: Merciless One in Control summoned at the player clone'),
(13,1,78523,0,0,31,0,3,41644,0,0,0,0,'','Into the Totem: totem channel targets Faceless Defiler'),
(13,1,78605,0,0,31,0,3,42128,0,0,0,0,'','It Will Come: Ick''thys beam visual anchors to ELM controller'),
(13,1,78850,0,0,31,0,3,42250,0,0,0,0,'','Ascend No More flavor: conduit bunny channels Twilight Candidate'),
(13,1,78766,0,0,31,0,3,42280,0,0,0,0,'','Runestones of Binding: candidate beam to Twilight Devotee'),
(13,1,78977,0,0,31,0,3,41658,0,0,0,0,'','Devotee to Ascendant missile targets Ascendant of the Deeps'),
(13,2,78727,0,0,31,0,3,42225,0,0,0,0,'','Prisoners (A): cage-open dummy hits Alliance Prisoner'),
(13,2,78822,0,0,31,0,3,42234,0,0,0,0,'','Prisoners (H): cage-open dummy hits Horde Prisoner');

-- 11b) Attuned Runestone 79045 only castable on Bound Torrents
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=17 AND `SourceEntry`=79045;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(17,0,79045,0,0,31,1,3,47969,0,0,0,0,'','Twilight Extermination: Attuned Runestone requires Bound Torrent target');

-- 11c) Gossip option gates
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=15 AND `SourceGroup` IN (11607,11535) AND `SourceEntry`=0;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(15,11607,0,0,0,47,0,26154,8,0,0,0,0,'','L''ghorek runestone re-grant: Twilight Extermination active'),
(15,11607,0,0,0,2,0,57172,1,0,1,0,0,'','L''ghorek runestone re-grant: player lacks Attuned Runestone'),
(15,11607,0,0,1,47,0,26143,8,0,0,0,0,'','L''ghorek runestone re-grant: All that Rises active'),
(15,11607,0,0,1,2,0,57172,1,0,1,0,0,'','L''ghorek runestone re-grant: player lacks Attuned Runestone'),
(15,11535,0,0,0,8,0,25987,0,0,0,0,0,'','Hexascrub scene replay: Put It On rewarded');

-- 11d) SAI gates
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=22 AND `SourceEntry` IN (41644,42128) AND `SourceId`=0;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(22,2,41644,0,0,47,0,26072,8,0,0,0,0,'','Defiler death credit: Into the Totem taken'),
(22,2,41644,0,0,29,0,42054,12,0,0,0,0,'','Defiler death credit: Confinement Totem within 12yd'),
(22,1,42128,0,0,47,0,26111,8,0,0,0,0,'','Ick''thys event: ...It Will Come taken and incomplete');

-- ----------------------------------------------------------------------------
-- 12) CREATURE TEXT (WPP import, BroadcastTextIds preserved; 42234/48620 crafted)
-- ----------------------------------------------------------------------------
DELETE FROM `creature_text` WHERE `CreatureID` IN (41648,41659,41837,41840,41884,41889,42197,42225,42234,42325,48620,50259);
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(41648,0,0,'I laugh at you, feeble $n, but I will oblige. Now, let us return to my master, below!',14,0,100,25,0,0,0,15136,0,'Ick''thys the Unfathomable - aggro'),
(41659,0,0,'%s begins channelling power for a massive attack!',41,0,100,0,0,0,0,0,0,'Hallazeal the Ascended - Overwhelming Power emote'),
(41837,0,0,'Oh gods! It''s not dead! RUN!',14,0,100,0,0,0,0,14993,0,'Engineer Hexascrub copy - scene 1'),
(41837,1,0,'We''re all going to die!',14,0,100,0,0,0,0,14989,0,'Engineer Hexascrub copy - scene 2'),
(41837,2,0,'It''s all your fault! It was supposed to be dead! Now it ate your brains!',14,0,100,0,0,0,0,14991,0,'Engineer Hexascrub copy - scene 3'),
(41837,3,0,'Mommy!',14,0,100,0,0,0,0,14991,0,'Engineer Hexascrub copy - scene 4'),
(41840,0,0,'|cFF68228BI SEE YOU.|r',14,0,100,0,0,0,0,14989,0,'Merciless One - scene 1'),
(41840,1,0,'|cFF68228BDIE.|r',14,0,100,0,0,0,0,14989,0,'Merciless One - scene 2'),
(41840,2,0,'|cFF68228BYOUR SIMPLE MIND CANNOT GRASP WHAT IS TRANSPIRING.|r',14,0,100,0,0,0,0,14987,0,'Merciless One - scene 3'),
(41884,0,0,'What the... $n?',14,0,100,5,0,0,0,14993,0,'Jorlan Trueblade copy - scene 1'),
(41884,1,0,'You fools stop running around! Face it!',14,0,100,25,0,0,0,14987,0,'Jorlan Trueblade copy - scene 2'),
(41884,2,0,'Somebody knock that thing off of $n''s head!',14,0,100,15,0,0,0,14990,0,'Jorlan Trueblade copy - scene 3'),
(41889,0,0,'This is marbles and conkers!',14,0,100,0,0,0,0,14993,0,'Foxy Topper copy - scene 1'),
(41889,1,0,'Nutmegs don''t fail me now!',14,0,100,0,0,0,0,14989,0,'Foxy Topper copy - scene 2'),
(41889,2,0,'You''re all fore and aft for putting that on!',14,0,100,0,0,0,0,14993,0,'Foxy Topper copy - scene 3'),
(42197,0,0,'L''ghorek Dies!',42,0,100,0,0,0,0,0,0,'L''ghorek - Back to Darkbreak Cove accept whisper'),
(42225,0,0,'When I get out of here, I''m going to kill every last one of you Twilight''s Hammer scum!',12,7,100,5,0,0,0,0,0,'Alliance Prisoner - caged threat'),
(42225,1,0,'See you on the other side, $g brother : sister;. Which way is out?',12,7,100,0,0,0,0,0,0,'Alliance Prisoner - released 1'),
(42225,2,0,'Thank you, $n. Thank you!',12,7,100,0,0,0,0,0,0,'Alliance Prisoner - released 2'),
(42225,3,0,'Oh gods, someone get me out of this cage!',12,7,100,20,0,0,0,0,0,'Alliance Prisoner - caged plea 1'),
(42225,4,0,'I''ve been in here for days. They were just about to sacrifice me!',12,7,100,0,0,0,0,0,0,'Alliance Prisoner - caged plea 2'),
(42225,5,0,'I thought I was a goner for sure. Thank you, $n!',12,7,100,0,0,0,0,0,0,'Alliance Prisoner - released 3'),
(42234,0,0,'When I get free, I will grind every last one of you Twilight''s Hammer curs into the sea floor!',12,1,100,5,0,0,0,0,0,'Horde Prisoner - caged threat (crafted mirror)'),
(42234,1,0,'Lok''tar, $g brother : sister;. Which way is out?',12,1,100,0,0,0,0,0,0,'Horde Prisoner - released 1 (crafted mirror)'),
(42234,2,0,'Thank you, $n. Thank you!',12,1,100,0,0,0,0,0,0,'Horde Prisoner - released 2 (crafted mirror)'),
(42234,3,0,'Someone get me out of this cursed cage!',12,1,100,20,0,0,0,0,0,'Horde Prisoner - caged plea 1 (crafted mirror)'),
(42234,4,0,'They were about to sacrifice me to their squid-faced masters!',12,1,100,0,0,0,0,0,0,'Horde Prisoner - caged plea 2 (crafted mirror)'),
(42234,5,0,'I thought I was done for. Thank you, $n!',12,1,100,0,0,0,0,0,0,'Horde Prisoner - released 3 (crafted mirror)'),
(42325,0,0,'%s''s bindings have been released! Full powers unlocked.',42,0,100,0,0,0,0,0,0,'Possessed Torrent - 26143 accept whisper'),
(42325,1,0,'I am freed! Let us slay Hallazeal in Neptulon''s name, $n. He lurks within the temple.',14,0,100,0,0,0,0,0,0,'Possessed Torrent - 26143 accept yell'),
(48620,0,0,'%s''s bindings have been released! Full powers unlocked.',42,0,100,0,0,0,0,0,0,'Vengeful Torrent - swap whisper (for C++ swap script)'),
(48620,1,0,'I am freed! Let us slay Hallazeal in Neptulon''s name, $n. He lurks within the temple.',14,0,100,0,0,0,0,0,0,'Vengeful Torrent - swap yell (for C++ swap script)'),
(50259,0,0,'I''m going in after Erunak. Follow me, $n!',12,0,100,1,0,0,0,0,0,'Captain Taylor - post-battle dive');

-- ----------------------------------------------------------------------------
-- 13) SMART SCRIPTS
-- ----------------------------------------------------------------------------
-- === 25987 "Put It On" scene =================================================
DELETE FROM `smart_scripts` WHERE `source_type`=0 AND `entryorguid` IN (41666,41814,41837,41840,41884,41889,47090,47094);
DELETE FROM `smart_scripts` WHERE `source_type`=9 AND `entryorguid` IN (4166600,4181400,4183700,4184000,4188400,4188900);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(41666,0,0,0,19,0,100,0,25987,0,0,0,0,80,4166600,0,0,0,0,0,1,0,0,0,0,0,0,0,'Hexascrub - On Quest Put It On Accepted - Run Scene Actionlist'),
(41666,0,1,2,62,0,100,0,11535,0,0,0,0,72,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Hexascrub - On Gossip Option 0 (replay) - Close Gossip'),
(41666,0,2,0,61,0,100,0,0,0,0,0,0,80,4166600,0,0,0,0,0,1,0,0,0,0,0,0,0,'Hexascrub - Link - Run Scene Actionlist'),
(4166600,9,0,0,0,0,100,0,0,0,0,0,0,85,78021,2,0,0,0,0,7,0,0,0,0,0,0,0,'Put It On - invoker casts Summon Hexascrub Copy'),
(4166600,9,1,0,0,0,100,0,0,0,0,0,0,85,78083,2,0,0,0,0,7,0,0,0,0,0,0,0,'Put It On - invoker casts Summon Jorlan Copy'),
(4166600,9,2,0,0,0,100,0,0,0,0,0,0,85,78085,2,0,0,0,0,7,0,0,0,0,0,0,0,'Put It On - invoker casts Summon Foxy Topper Copy'),
(4166600,9,3,0,0,0,100,0,0,0,0,0,0,85,87752,2,0,0,0,0,7,0,0,0,0,0,0,0,'Put It On - invoker casts Summon Rallings Copy'),
(4166600,9,4,0,0,0,100,0,0,0,0,0,0,85,87760,2,0,0,0,0,7,0,0,0,0,0,0,0,'Put It On - invoker casts Summon Darkbreak Guard Copy'),
(4166600,9,5,0,0,0,100,0,500,500,0,0,0,85,78008,2,0,0,0,0,7,0,0,0,0,0,0,0,'Put It On - invoker casts Summon Merciless One Controlled You (clone)'),
(4166600,9,6,0,0,0,100,0,700,700,0,0,0,85,77988,2,0,0,0,0,7,0,0,0,0,0,0,0,'Put It On - invoker casts Summon Merciless One in Control of You'),
(4166600,9,7,0,0,0,100,0,300,300,0,0,0,85,87746,2,0,0,0,0,7,0,0,0,0,0,0,0,'Put It On - invoker casts Camera Channel'),
(4166600,9,8,0,0,0,100,0,38500,38500,0,0,0,85,123460,2,0,0,0,0,7,0,0,0,0,0,0,0,'Put It On - invoker casts serverside quest complete 25987'),
(41814,0,0,0,54,0,100,0,0,0,0,0,0,80,4181400,0,0,0,0,0,1,0,0,0,0,0,0,0,'Merciless One in Control - On Just Summoned - Run Actionlist'),
(4181400,9,0,0,0,0,100,0,500,500,0,0,0,11,46598,0,0,0,0,0,19,41840,15,0,0,0,0,0,'Merciless One in Control - Ride Vehicle Hardcoded on player clone'),
(4181400,9,1,0,0,0,100,0,41500,41500,0,0,0,41,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Merciless One in Control - despawn at scene end'),
(41840,0,0,0,54,0,100,0,0,0,0,0,0,80,4184000,0,0,0,0,0,1,0,0,0,0,0,0,0,'Merciless One clone - On Just Summoned - Run Actionlist'),
(4184000,9,0,0,0,0,100,0,1500,1500,0,0,0,11,78037,0,0,0,0,0,1,0,0,0,0,0,0,0,'Merciless One clone - cast Strangulate State'),
(4184000,9,1,0,0,0,100,0,4500,4500,0,0,0,1,0,0,1,0,0,0,7,0,0,0,0,0,0,0,'Merciless One clone - I SEE YOU.'),
(4184000,9,2,0,0,0,100,0,8000,8000,0,0,0,1,1,0,1,0,0,0,7,0,0,0,0,0,0,0,'Merciless One clone - DIE.'),
(4184000,9,3,0,0,0,100,0,2000,2000,0,0,0,1,2,0,1,0,0,0,7,0,0,0,0,0,0,0,'Merciless One clone - YOUR SIMPLE MIND...'),
(4184000,9,4,0,0,0,100,0,16000,16000,0,0,0,1,2,0,1,0,0,0,7,0,0,0,0,0,0,0,'Merciless One clone - YOUR SIMPLE MIND... (repeat)'),
(4184000,9,5,0,0,0,100,0,10500,10500,0,0,0,41,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Merciless One clone - despawn'),
(41837,0,0,0,54,0,100,0,0,0,0,0,0,80,4183700,0,0,0,0,0,1,0,0,0,0,0,0,0,'Hexascrub copy - On Just Summoned - Run Actionlist'),
(4183700,9,0,0,0,0,100,0,0,0,0,0,0,11,78087,0,0,0,0,0,1,0,0,0,0,0,0,0,'Hexascrub copy - Cower AnimKit'),
(4183700,9,1,0,0,0,100,0,2000,2000,0,0,0,1,0,0,1,0,0,0,7,0,0,0,0,0,0,0,'Hexascrub copy - Oh gods! It''s not dead! RUN!'),
(4183700,9,2,0,0,0,100,0,3000,3000,0,0,0,1,1,0,1,0,0,0,7,0,0,0,0,0,0,0,'Hexascrub copy - We''re all going to die!'),
(4183700,9,3,0,0,0,100,0,25000,25000,0,0,0,1,2,0,1,0,0,0,7,0,0,0,0,0,0,0,'Hexascrub copy - It''s all your fault!'),
(4183700,9,4,0,0,0,100,0,12000,12000,0,0,0,41,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Hexascrub copy - despawn'),
(41884,0,0,0,54,0,100,0,0,0,0,0,0,80,4188400,0,0,0,0,0,1,0,0,0,0,0,0,0,'Jorlan copy - On Just Summoned - Run Actionlist'),
(4188400,9,0,0,0,0,100,0,2000,2000,0,0,0,1,0,0,1,0,0,0,7,0,0,0,0,0,0,0,'Jorlan copy - What the... $n?'),
(4188400,9,1,0,0,0,100,0,14000,14000,0,0,0,1,1,0,1,0,0,0,7,0,0,0,0,0,0,0,'Jorlan copy - You fools stop running around!'),
(4188400,9,2,0,0,0,100,0,11000,11000,0,0,0,1,2,0,1,0,0,0,7,0,0,0,0,0,0,0,'Jorlan copy - Somebody knock that thing off!'),
(4188400,9,3,0,0,0,100,0,15000,15000,0,0,0,41,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Jorlan copy - despawn'),
(41889,0,0,0,54,0,100,0,0,0,0,0,0,80,4188900,0,0,0,0,0,1,0,0,0,0,0,0,0,'Foxy copy - On Just Summoned - Run Actionlist'),
(4188900,9,0,0,0,0,100,0,0,0,0,0,0,11,78087,0,0,0,0,0,1,0,0,0,0,0,0,0,'Foxy copy - Cower AnimKit'),
(4188900,9,1,0,0,0,100,0,2000,2000,0,0,0,1,0,0,1,0,0,0,7,0,0,0,0,0,0,0,'Foxy copy - This is marbles and conkers!'),
(4188900,9,2,0,0,0,100,0,18000,18000,0,0,0,1,1,0,1,0,0,0,7,0,0,0,0,0,0,0,'Foxy copy - Nutmegs don''t fail me now!'),
(4188900,9,3,0,0,0,100,0,22000,22000,0,0,0,41,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Foxy copy - despawn'),
(47090,0,0,0,54,0,100,0,0,0,0,0,0,41,42000,0,0,0,0,0,1,0,0,0,0,0,0,0,'Rallings copy - despawn 42s after summon'),
(47094,0,0,0,54,0,100,0,0,0,0,0,0,41,42000,0,0,0,0,0,1,0,0,0,0,0,0,0,'Darkbreak Guard copy - despawn 42s after summon');

-- === 26072 "Into the Totem" ==================================================
DELETE FROM `smart_scripts` WHERE `source_type`=0 AND `entryorguid`=42054;
DELETE FROM `smart_scripts` WHERE `source_type`=0 AND `entryorguid`=41644 AND `id` IN (1,2);
DELETE FROM `smart_scripts` WHERE `source_type`=9 AND `entryorguid`=4205400;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(42054,0,0,0,54,0,100,0,0,0,0,0,0,80,4205400,0,0,0,0,0,1,0,0,0,0,0,0,0,'Confinement Totem - On Just Summoned - Run Actionlist'),
(42054,0,1,0,38,0,100,0,0,1,0,0,0,11,78523,0,0,0,0,0,19,41644,40,0,0,0,0,0,'Confinement Totem - On Data Set 0 1 - re-channel at closest Defiler'),
(4205400,9,0,0,0,0,100,0,500,500,0,0,0,11,78523,0,0,0,0,0,19,41644,40,0,0,0,0,0,'Confinement Totem - channel Area Aura Effect at closest Defiler'),
(4205400,9,1,0,0,0,100,0,60000,60000,0,0,0,41,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Confinement Totem - despawn after 60s'),
(41644,0,1,2,6,0,100,0,0,0,0,0,0,33,42027,0,0,0,0,0,7,0,0,0,0,0,0,0,'Faceless Defiler - On Death - Into the Totem credit to killer (conditions: quest + totem near)'),
(41644,0,2,0,61,0,100,0,0,0,0,0,0,45,0,1,0,0,0,0,19,42054,15,0,0,0,0,0,'Faceless Defiler - Link - Set Data on Confinement Totem (suck-in re-channel)');

-- === 26111 "... It Will Come" — Ick'thys event ===============================
DELETE FROM `smart_scripts` WHERE `source_type`=0 AND `entryorguid`=42128;
DELETE FROM `smart_scripts` WHERE `source_type`=0 AND `entryorguid`=41648 AND `id` IN (3,4);
DELETE FROM `smart_scripts` WHERE `source_type`=0 AND `entryorguid` IN (-348740);
DELETE FROM `smart_scripts` WHERE `source_type`=9 AND `entryorguid` IN (4212800,4212801,4164800);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(42128,0,0,0,10,0,100,0,1,25,90000,90000,0,80,4212800,0,0,0,0,0,1,0,0,0,0,0,0,0,'Ick''thys controller - On friendly in LOS (quest 26111 taken) - Run Event Actionlist'),
(4212800,9,0,0,0,0,100,0,0,0,0,0,0,45,0,1,0,0,0,0,19,23837,40,0,0,0,0,0,'Ick''thys event - Set Data on fountain bunny'),
(4212800,9,1,0,0,0,100,0,0,0,0,0,0,11,78607,0,0,0,0,0,1,0,0,0,0,0,0,0,'Ick''thys event - Ground Rumble Earthquake 1'),
(4212800,9,2,0,0,0,100,0,2000,2000,0,0,0,11,78607,0,0,0,0,0,1,0,0,0,0,0,0,0,'Ick''thys event - Ground Rumble Earthquake 2'),
(4212800,9,3,0,0,0,100,0,2000,2000,0,0,0,11,78607,0,0,0,0,0,1,0,0,0,0,0,0,0,'Ick''thys event - Ground Rumble Earthquake 3'),
(4212800,9,4,0,0,0,100,0,2000,2000,0,0,0,11,78607,0,0,0,0,0,1,0,0,0,0,0,0,0,'Ick''thys event - Ground Rumble Earthquake 4'),
(4212800,9,5,0,0,0,100,0,1000,1000,0,0,0,12,41648,1,300000,0,0,0,8,0,0,0,-5650.27,6312.47,-1065.4,2.55,'Ick''thys event - Summon Ick''thys the Unfathomable'),
(-348740,0,0,0,38,0,100,0,0,1,0,0,0,80,4212801,0,0,0,0,0,1,0,0,0,0,0,0,0,'ELM bunny 348740 - On Data Set - Energy Fountain pulses'),
(4212801,9,0,0,0,0,100,0,0,0,0,0,0,11,78532,0,0,0,0,0,1,0,0,0,0,0,0,0,'Energy Fountain Visual 1'),
(4212801,9,1,0,0,0,100,0,2000,2000,0,0,0,11,78532,0,0,0,0,0,1,0,0,0,0,0,0,0,'Energy Fountain Visual 2'),
(4212801,9,2,0,0,0,100,0,2000,2000,0,0,0,11,78532,0,0,0,0,0,1,0,0,0,0,0,0,0,'Energy Fountain Visual 3'),
(4212801,9,3,0,0,0,100,0,2000,2000,0,0,0,11,78532,0,0,0,0,0,1,0,0,0,0,0,0,0,'Energy Fountain Visual 4'),
(41648,0,3,0,4,0,100,0,0,0,0,0,0,1,0,0,1,0,0,0,7,0,0,0,0,0,0,0,'Ick''thys - On Aggro - yell (BCT 15136)'),
(41648,0,4,0,54,0,100,0,0,0,0,0,0,80,4164800,0,0,0,0,0,1,0,0,0,0,0,0,0,'Ick''thys - On Just Summoned - Run Emerge Actionlist'),
(4164800,9,0,0,0,0,100,0,0,0,0,0,0,11,78605,0,0,0,0,0,1,0,0,0,0,0,0,0,'Ick''thys emerge - Beam Visual 1'),
(4164800,9,1,0,0,0,100,0,600,600,0,0,0,11,78605,0,0,0,0,0,1,0,0,0,0,0,0,0,'Ick''thys emerge - Beam Visual 2'),
(4164800,9,2,0,0,0,100,0,600,600,0,0,0,11,78605,0,0,0,0,0,1,0,0,0,0,0,0,0,'Ick''thys emerge - Beam Visual 3'),
(4164800,9,3,0,0,0,100,0,400,400,0,0,0,11,72126,0,0,0,0,0,1,0,0,0,0,0,0,0,'Ick''thys emerge - Freeze Anim'),
(4164800,9,4,0,0,0,100,0,1500,1500,0,0,0,69,1,0,0,0,0,0,8,0,0,0,-5650.27,6312.47,-1113.0,0,'Ick''thys emerge - slow descent toward the chasm floor');

-- === 26154 "Twilight Extermination" — Bound Torrent passivation ==============
DELETE FROM `smart_scripts` WHERE `source_type`=0 AND `entryorguid`=47969;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(47969,0,0,0,8,0,100,0,79045,0,0,0,0,41,3000,0,0,0,0,0,1,0,0,0,0,0,0,0,'Bound Torrent - On Spellhit Attuned Runestone - despawn 3s (possession consumed)');

-- === 26143 "All that Rises" + 26181 + runestone regrant on 42197 =============
DELETE FROM `smart_scripts` WHERE `source_type`=0 AND `entryorguid` IN (42197,42325,48620);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(42197,0,0,0,19,0,100,0,26181,0,0,0,0,1,0,0,1,0,0,0,7,0,0,0,0,0,0,0,'L''ghorek - On Quest Back to Darkbreak Cove Accepted - whisper L''ghorek Dies!'),
(42197,0,1,2,62,0,100,0,11607,0,0,0,0,85,123461,2,0,0,0,0,7,0,0,0,0,0,0,0,'L''ghorek - On Gossip Option 0 - re-grant Attuned Runestone'),
(42197,0,2,0,61,0,100,0,0,0,0,0,0,72,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'L''ghorek - Link - Close Gossip'),
(42197,0,3,4,20,0,100,0,26143,0,0,0,0,45,0,1,0,0,0,0,19,42325,60,0,0,0,0,0,'L''ghorek - On Quest All that Rises Rewarded - despawn player''s Possessed Torrent'),
(42197,0,4,0,61,0,100,0,0,0,0,0,0,45,0,1,0,0,0,0,19,48620,60,0,0,0,0,0,'L''ghorek - Link - despawn player''s Vengeful Torrent'),
(42325,0,0,0,38,0,100,0,0,1,0,0,0,41,1000,0,0,0,0,0,1,0,0,0,0,0,0,0,'Possessed Torrent - On Data Set 0 1 - despawn (26143 rewarded cleanup)'),
(48620,0,0,0,38,0,100,0,0,1,0,0,0,41,1000,0,0,0,0,0,1,0,0,0,0,0,0,0,'Vengeful Torrent - On Data Set 0 1 - despawn (26143 rewarded cleanup)');

-- === 26144/26149 "Prisoners" =================================================
DELETE FROM `smart_scripts` WHERE `source_type`=0 AND `entryorguid` IN (42225,42234);
DELETE FROM `smart_scripts` WHERE `source_type`=9 AND `entryorguid` IN (4222500,4223400);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(42225,0,0,0,8,0,100,0,78727,0,0,0,0,80,4222500,0,0,0,0,0,1,0,0,0,0,0,0,0,'Alliance Prisoner - On Cage Opened hit - Run Release Actionlist'),
(42225,0,1,0,1,0,40,0,30000,60000,45000,90000,0,1,3,0,0,0,0,0,1,0,0,0,0,0,0,0,'Alliance Prisoner - OOC - caged plea 1'),
(42225,0,2,0,1,0,30,0,45000,90000,60000,120000,0,1,4,0,0,0,0,0,1,0,0,0,0,0,0,0,'Alliance Prisoner - OOC - caged plea 2'),
(42225,0,3,0,1,0,25,0,60000,120000,90000,150000,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Alliance Prisoner - OOC - caged threat'),
(4222500,9,0,0,0,0,100,0,0,0,0,0,0,69,1,0,0,0,0,0,7,0,0,0,0,0,0,0,'Alliance Prisoner - move out of cage toward rescuer'),
(4222500,9,1,0,0,0,100,0,2500,2500,0,0,0,1,2,0,1,0,0,0,7,0,0,0,0,0,0,0,'Alliance Prisoner - Thank you, $n!'),
(4222500,9,2,0,0,0,100,0,6000,6000,0,0,0,41,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Alliance Prisoner - despawn'),
(42234,0,0,0,8,0,100,0,78822,0,0,0,0,80,4223400,0,0,0,0,0,1,0,0,0,0,0,0,0,'Horde Prisoner - On Cage Opened hit - Run Release Actionlist'),
(42234,0,1,0,1,0,40,0,30000,60000,45000,90000,0,1,3,0,0,0,0,0,1,0,0,0,0,0,0,0,'Horde Prisoner - OOC - caged plea 1'),
(42234,0,2,0,1,0,30,0,45000,90000,60000,120000,0,1,4,0,0,0,0,0,1,0,0,0,0,0,0,0,'Horde Prisoner - OOC - caged plea 2'),
(42234,0,3,0,1,0,25,0,60000,120000,90000,150000,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Horde Prisoner - OOC - caged threat'),
(4223400,9,0,0,0,0,100,0,0,0,0,0,0,69,1,0,0,0,0,0,7,0,0,0,0,0,0,0,'Horde Prisoner - move out of cage toward rescuer'),
(4223400,9,1,0,0,0,100,0,2500,2500,0,0,0,1,2,0,1,0,0,0,7,0,0,0,0,0,0,0,'Horde Prisoner - Thank you, $n!'),
(4223400,9,2,0,0,0,100,0,6000,6000,0,0,0,41,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Horde Prisoner - despawn');

-- === Hallazeal rewrite (adds sniffed RaidBossEmote to Overwhelming Power) ====
DELETE FROM `smart_scripts` WHERE `source_type`=0 AND `entryorguid`=41659;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(41659,0,0,0,0,0,100,0,6000,8000,14000,15000,0,11,90550,0,0,0,0,0,2,0,0,0,0,0,0,0,'Hallazeal the Ascended - IC - Cast Arcane Barrage'),
(41659,0,1,2,0,0,100,0,12000,12000,21000,24000,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Hallazeal the Ascended - IC - Overwhelming Power emote'),
(41659,0,2,0,61,0,100,0,0,0,0,0,0,11,90551,0,0,0,0,0,2,0,0,0,0,0,0,0,'Hallazeal the Ascended - Link - Cast Overwhelming Power');

-- === 26193/26194 finale: battle combat + reward RP ===========================
DELETE FROM `smart_scripts` WHERE `source_type`=0 AND `entryorguid` IN (42818,42822,42370,44490,44540,50259);
DELETE FROM `smart_scripts` WHERE `source_type`=9 AND `entryorguid`=5025900;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(42818,0,0,0,0,0,100,0,4000,7000,9000,13000,0,11,51876,0,0,0,0,0,2,0,0,0,0,0,0,0,'Azsh''ir Warrior - IC - Cast Stormstrike'),
(42370,0,0,0,0,0,100,0,5000,9000,12000,16000,0,11,79117,0,0,0,0,0,2,0,0,0,0,0,0,0,'Faceless Shadoweaver - IC - Cast Shadow Crash'),
(42822,0,0,0,0,0,100,0,4000,6000,9000,12000,0,11,79093,0,0,0,0,0,2,0,0,0,0,0,0,0,'Revenant of Neptulon - IC - Cast Frost Cleave'),
(42822,0,1,0,0,0,100,0,9000,13000,16000,21000,0,11,79091,0,0,0,0,0,2,0,0,0,0,0,0,0,'Revenant of Neptulon - IC - Cast Wrath of Neptulon'),
(44490,0,0,0,20,0,100,0,26193,0,0,0,0,85,123462,2,0,0,0,0,7,0,0,0,0,0,0,0,'Captain Taylor - On Defending the Rift (A) Rewarded - invoker summons dive Taylor'),
(44540,0,0,0,20,0,100,0,26194,0,0,0,0,85,123462,2,0,0,0,0,7,0,0,0,0,0,0,0,'Legionnaire Nazgrim - On Defending the Rift (H) Rewarded - invoker summons dive Taylor'),
(50259,0,0,0,54,0,100,0,0,0,0,0,0,80,5025900,0,0,0,0,0,1,0,0,0,0,0,0,0,'Captain Taylor (dive) - On Just Summoned - Run Dive Actionlist'),
(5025900,9,0,0,0,0,100,0,500,500,0,0,0,11,76040,0,0,0,0,0,1,0,0,0,0,0,0,0,'Dive Taylor - cast Re-Breather'),
(5025900,9,1,0,0,0,100,0,1500,1500,0,0,0,1,0,0,1,0,0,0,7,0,0,0,0,0,0,0,'Dive Taylor - I''m going in after Erunak. Follow me, $n!'),
(5025900,9,2,0,0,0,100,0,2000,2000,0,0,0,59,1,0,0,0,0,0,1,0,0,0,0,0,0,0,'Dive Taylor - set run'),
(5025900,9,3,0,0,0,100,0,500,500,0,0,0,69,1,0,0,0,0,0,8,0,0,0,-5836.06,5388.30,-1209.31,0,'Dive Taylor - spline point 1'),
(5025900,9,4,0,0,0,100,0,2500,2500,0,0,0,69,2,0,0,0,0,0,8,0,0,0,-5827.14,5378.77,-1207.90,0,'Dive Taylor - spline point 2'),
(5025900,9,5,0,0,0,100,0,2500,2500,0,0,0,69,3,0,0,0,0,0,8,0,0,0,-5811.93,5367.41,-1219.74,0,'Dive Taylor - spline point 3'),
(5025900,9,6,0,0,0,100,0,5000,5000,0,0,0,69,4,0,0,0,0,0,8,0,0,0,-5768.05,5354.44,-1260.82,0,'Dive Taylor - spline point 4'),
(5025900,9,7,0,0,0,100,0,7000,7000,0,0,0,69,5,0,0,0,0,0,8,0,0,0,-5706.03,5340.79,-1322.68,0,'Dive Taylor - spline point 5 (Throne of the Tides rift)'),
(5025900,9,8,0,0,0,100,0,4000,4000,0,0,0,41,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Dive Taylor - despawn in the rift');

-- === L'ghorek interior ambience (sniffed cosmetic casts) =====================
DELETE FROM `smart_scripts` WHERE `source_type`=0 AND `entryorguid`=42213;
DELETE FROM `smart_scripts` WHERE `source_type`=0 AND `entryorguid`=41657 AND `id`=2;
DELETE FROM `smart_scripts` WHERE `source_type`=0 AND `entryorguid` IN (42280,42281) AND `id` IN (2,3);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(42213,0,0,0,1,0,100,0,5000,15000,15000,25000,0,11,78850,0,0,0,0,0,1,0,0,0,0,0,0,0,'Ancient Conduit Bunny - OOC - Channel to Twilight Candidate Flavor'),
(41657,0,2,0,1,0,75,0,5000,15000,12000,20000,0,11,78766,0,0,0,0,0,1,0,0,0,0,0,0,0,'Twilight Candidate - OOC - Candidate Beam'),
(42280,0,2,0,1,0,60,0,8000,20000,15000,30000,0,11,78977,0,0,0,0,0,1,0,0,0,0,0,0,0,'Twilight Devotee - OOC - Devotee to Ascendant Missile'),
(42280,0,3,0,1,0,40,0,10000,30000,45000,75000,0,11,78900,0,0,0,0,0,1,0,0,0,0,0,0,0,'Twilight Devotee - OOC - Worshipping Anim Kit'),
(42281,0,2,0,1,0,60,0,8000,20000,15000,30000,0,11,78977,0,0,0,0,0,1,0,0,0,0,0,0,0,'Twilight Devotee - OOC - Devotee to Ascendant Missile'),
(42281,0,3,0,1,0,40,0,10000,30000,45000,75000,0,11,78900,0,0,0,0,0,1,0,0,0,0,0,0,0,'Twilight Devotee - OOC - Worshipping Anim Kit');

-- L'ghorek's Brain Activity pulses: guid-scoped SAI for the 27 mouth bunnies
-- (23837 is a global entry with entry-level SAI; guid scripts take precedence)
DELETE FROM `smart_scripts` WHERE `source_type`=0 AND `entryorguid` IN (-348223,-348224,-348225,-348226,-348227,-348228,-348229,-348230,-348231,-348232,-348233,-348234,-348235,-348236,-348237,-348238,-348239,-348240,-348241,-348242,-348243,-348244,-348246,-348247,-348248,-348249,-348250);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(-348223,0,0,0,1,0,100,0,1000,20000,15000,30000,0,11,78716,0,0,0,0,0,1,0,0,0,0,0,0,0,'ELM bunny - OOC - L''ghorek Brain Activity'),
(-348224,0,0,0,1,0,100,0,1000,20000,15000,30000,0,11,78716,0,0,0,0,0,1,0,0,0,0,0,0,0,'ELM bunny - OOC - L''ghorek Brain Activity'),
(-348225,0,0,0,1,0,100,0,1000,20000,15000,30000,0,11,78716,0,0,0,0,0,1,0,0,0,0,0,0,0,'ELM bunny - OOC - L''ghorek Brain Activity'),
(-348226,0,0,0,1,0,100,0,1000,20000,15000,30000,0,11,78716,0,0,0,0,0,1,0,0,0,0,0,0,0,'ELM bunny - OOC - L''ghorek Brain Activity'),
(-348227,0,0,0,1,0,100,0,1000,20000,15000,30000,0,11,78716,0,0,0,0,0,1,0,0,0,0,0,0,0,'ELM bunny - OOC - L''ghorek Brain Activity'),
(-348228,0,0,0,1,0,100,0,1000,20000,15000,30000,0,11,78716,0,0,0,0,0,1,0,0,0,0,0,0,0,'ELM bunny - OOC - L''ghorek Brain Activity'),
(-348229,0,0,0,1,0,100,0,1000,20000,15000,30000,0,11,78716,0,0,0,0,0,1,0,0,0,0,0,0,0,'ELM bunny - OOC - L''ghorek Brain Activity'),
(-348230,0,0,0,1,0,100,0,1000,20000,15000,30000,0,11,78716,0,0,0,0,0,1,0,0,0,0,0,0,0,'ELM bunny - OOC - L''ghorek Brain Activity'),
(-348231,0,0,0,1,0,100,0,1000,20000,15000,30000,0,11,78716,0,0,0,0,0,1,0,0,0,0,0,0,0,'ELM bunny - OOC - L''ghorek Brain Activity'),
(-348232,0,0,0,1,0,100,0,1000,20000,15000,30000,0,11,78716,0,0,0,0,0,1,0,0,0,0,0,0,0,'ELM bunny - OOC - L''ghorek Brain Activity'),
(-348233,0,0,0,1,0,100,0,1000,20000,15000,30000,0,11,78716,0,0,0,0,0,1,0,0,0,0,0,0,0,'ELM bunny - OOC - L''ghorek Brain Activity'),
(-348234,0,0,0,1,0,100,0,1000,20000,15000,30000,0,11,78716,0,0,0,0,0,1,0,0,0,0,0,0,0,'ELM bunny - OOC - L''ghorek Brain Activity'),
(-348235,0,0,0,1,0,100,0,1000,20000,15000,30000,0,11,78716,0,0,0,0,0,1,0,0,0,0,0,0,0,'ELM bunny - OOC - L''ghorek Brain Activity'),
(-348236,0,0,0,1,0,100,0,1000,20000,15000,30000,0,11,78716,0,0,0,0,0,1,0,0,0,0,0,0,0,'ELM bunny - OOC - L''ghorek Brain Activity'),
(-348237,0,0,0,1,0,100,0,1000,20000,15000,30000,0,11,78716,0,0,0,0,0,1,0,0,0,0,0,0,0,'ELM bunny - OOC - L''ghorek Brain Activity'),
(-348238,0,0,0,1,0,100,0,1000,20000,15000,30000,0,11,78716,0,0,0,0,0,1,0,0,0,0,0,0,0,'ELM bunny - OOC - L''ghorek Brain Activity'),
(-348239,0,0,0,1,0,100,0,1000,20000,15000,30000,0,11,78716,0,0,0,0,0,1,0,0,0,0,0,0,0,'ELM bunny - OOC - L''ghorek Brain Activity'),
(-348240,0,0,0,1,0,100,0,1000,20000,15000,30000,0,11,78716,0,0,0,0,0,1,0,0,0,0,0,0,0,'ELM bunny - OOC - L''ghorek Brain Activity'),
(-348241,0,0,0,1,0,100,0,1000,20000,15000,30000,0,11,78716,0,0,0,0,0,1,0,0,0,0,0,0,0,'ELM bunny - OOC - L''ghorek Brain Activity'),
(-348242,0,0,0,1,0,100,0,1000,20000,15000,30000,0,11,78716,0,0,0,0,0,1,0,0,0,0,0,0,0,'ELM bunny - OOC - L''ghorek Brain Activity'),
(-348243,0,0,0,1,0,100,0,1000,20000,15000,30000,0,11,78716,0,0,0,0,0,1,0,0,0,0,0,0,0,'ELM bunny - OOC - L''ghorek Brain Activity'),
(-348244,0,0,0,1,0,100,0,1000,20000,15000,30000,0,11,78716,0,0,0,0,0,1,0,0,0,0,0,0,0,'ELM bunny - OOC - L''ghorek Brain Activity'),
(-348246,0,0,0,1,0,100,0,1000,20000,15000,30000,0,11,78716,0,0,0,0,0,1,0,0,0,0,0,0,0,'ELM bunny - OOC - L''ghorek Brain Activity'),
(-348247,0,0,0,1,0,100,0,1000,20000,15000,30000,0,11,78716,0,0,0,0,0,1,0,0,0,0,0,0,0,'ELM bunny - OOC - L''ghorek Brain Activity'),
(-348248,0,0,0,1,0,100,0,1000,20000,15000,30000,0,11,78716,0,0,0,0,0,1,0,0,0,0,0,0,0,'ELM bunny - OOC - L''ghorek Brain Activity'),
(-348249,0,0,0,1,0,100,0,1000,20000,15000,30000,0,11,78716,0,0,0,0,0,1,0,0,0,0,0,0,0,'ELM bunny - OOC - L''ghorek Brain Activity'),
(-348250,0,0,0,1,0,100,0,1000,20000,15000,30000,0,11,78716,0,0,0,0,0,1,0,0,0,0,0,0,0,'ELM bunny - OOC - L''ghorek Brain Activity');

-- ============================================================================
-- END OF FILE
-- ============================================================================
