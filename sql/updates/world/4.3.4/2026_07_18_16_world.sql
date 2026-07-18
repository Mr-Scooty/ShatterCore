-- ============================================================================
-- ABYSSAL DEPTHS + CROSS-ZONE ORPHANS — BATCH D (unsniffed reconstruction)
-- Specs: ARC AB3 (early Abyssal sort-5145, both factions) + ARC AB4 (orphan sweep)
-- TrinityCore 4.3.4 fork (ShatterCore). Idempotent: safe to re-apply.
-- Creature guids used: 9001600-9001624. GO guids used: none.
-- Custom PhaseIds used: none (all spawns ride zone-base PhaseId 169).
-- All spell IDs verified in 4.3.4 Spell.dbc/SpellEffect.dbc (77935, 77951,
-- 78191, 78277, 78514, 79127, 78547). Fork enums verified in SmartScriptMgr.h /
-- ConditionMgr.h (KILLEDMONSTER=33, AREAEXPLORED=15, ADD_ITEM=56, CLOSE_GOSSIP=72,
-- CALL_TIMED_ACTIONLIST=80, GOSSIP_HELLO=64).
-- Rows marked "AUTHORED" are faithful-flavor text written for this batch
-- (no sniff/WPP source exists). Everything else is DB/DBC/WPP-derived.
-- ============================================================================

-- ############################################################################
-- SECTION 1 — DUP-SPAWN CLEANUP (AB3 §6)
-- 41642 Wil'hai: 346307/348588 identical -> keep 346307.
-- 41659 Hallazeal: 347378/348444 identical -> keep 347378 (347763 is 33yd off,
--   a distinct platform spot — kept, verify in walkthrough).
-- 42197 L'ghorek: 348221/348222 4.5yd apart -> keep 348221.
-- ############################################################################
DELETE FROM `creature` WHERE `guid` IN (348588, 348444, 348222);
DELETE FROM `creature_addon` WHERE `guid` IN (348588, 348444, 348222);

-- ############################################################################
-- SECTION 2 — MISSING QUESTGIVER / ACTOR SPAWNS (AB3 §1, §6)
-- Promontory Point coords are 5.5.3 retail-sniffed (incl. orientations for
-- 41598/41600/41639). 41636 Nazgrim + 44540 + Valoren + Erunak hub copies are
-- POI/neighbor-derived estimates (walkthrough: verify Valoren Z!).
-- ############################################################################
DELETE FROM `creature` WHERE `guid` BETWEEN 9001600 AND 9001624;
INSERT INTO `creature` (`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseUseFlags`,`phaseMask`,`PhaseId`,`PhaseGroup`,`terrainSwapMap`,`modelid`,`equipment_id`,`position_x`,`position_y`,`position_z`,`orientation`,`spawntimesecs`,`wander_distance`,`currentwaypoint`,`curhealth`,`curmana`,`MovementType`,`npcflag`,`unit_flags`,`dynamicflags`,`ScriptName`,`VerifiedBuild`) VALUES
-- Promontory Point neutral hub (area 5100)
(9001600,41598,0,5145,5100,1,0,1,169,0,-1,0,0,-5937.39,6490.73,-835.722,2.37365,300,0,0,1,0,0,0,0,0,'',0),  -- Captain Taylor (sniffed)
(9001601,41636,0,5145,5100,1,0,1,169,0,-1,0,0,-5934.00,6495.00,-835.50,2.37365,300,0,0,1,0,0,0,0,0,'',0),   -- Legionnaire Nazgrim (est.)
(9001602,41600,0,5145,5100,1,0,1,169,0,-1,0,0,-5936.12,6493.86,-835.492,2.39110,300,0,0,1,0,0,0,0,0,'',0),  -- Erunak Stonespeaker (sniffed)
(9001603,41639,0,5145,5100,1,0,1,169,0,-1,0,0,-5936.59,6492.24,-835.575,2.37365,300,0,0,1,0,0,0,0,0,'',0),  -- Wavespeaker Tulra (sniffed)
-- Nightmare Depths (area 5102) — 26056/26057/26065
(9001604,41640,0,5145,5102,1,0,1,169,0,-1,0,0,-5662.00,6090.00,-960.00,0.90,300,0,0,1,0,0,0,0,0,'',0),      -- Wavespeaker Valoren (POI est., VERIFY Z)
-- Erunak hub copies — 26181/26182/26193/26194 turn-in/pickup
(9001605,41600,0,5145,4976,1,0,1,169,0,-1,0,0,-6889.01,5963.42,-765.38,1.55,300,0,0,1,0,0,0,0,0,'',0),      -- Erunak @ Darkbreak Cove (sniffed pos)
(9001606,41600,0,5145,4975,1,0,1,169,0,-1,0,0,-6553.00,6134.00,-671.00,3.90,300,0,0,1,0,0,0,0,0,'',0),      -- Erunak @ Tenebrous Cavern (est.)
-- Rift finale enders — 26193/26194
(9001607,44490,0,5145,0,1,0,1,169,0,-1,0,0,-5841.18,5390.71,-1213.89,1.60,300,0,0,1,0,0,0,0,0,'',0),        -- Captain Taylor @ rift (sniffed pos)
(9001608,44540,0,5145,0,1,0,1,169,0,-1,0,0,-5845.00,5394.00,-1213.90,1.60,300,0,0,1,0,0,0,0,0,'',0),        -- Legionnaire Nazgrim @ rift (est. adjacent)
-- 42234 Horde Prisoner x16 (26149) — cloned from 42225 set, offset +3/-3 (area 4971)
(9001609,42234,0,5145,4971,1,0,1,169,0,-1,0,0,-6685.92,6946.64,-798.068,4.64258,300,0,0,1,0,0,0,0,0,'',0),
(9001610,42234,0,5145,4971,1,0,1,169,0,-1,0,0,-6653.89,6924.67,-803.154,4.39823,300,0,0,1,0,0,0,0,0,'',0),
(9001611,42234,0,5145,4971,1,0,1,169,0,-1,0,0,-6652.02,6861.16,-805.340,6.05629,300,0,0,1,0,0,0,0,0,'',0),
(9001612,42234,0,5145,4971,1,0,1,169,0,-1,0,0,-6600.99,7110.15,-792.013,5.49779,300,0,0,1,0,0,0,0,0,'',0),
(9001613,42234,0,5145,4971,1,0,1,169,0,-1,0,0,-6621.10,7066.93,-789.382,1.43117,300,0,0,1,0,0,0,0,0,'',0),
(9001614,42234,0,5145,4971,1,0,1,169,0,-1,0,0,-6616.34,6796.76,-806.903,2.32129,300,0,0,1,0,0,0,0,0,'',0),
(9001615,42234,0,5145,4971,1,0,1,169,0,-1,0,0,-6530.58,6832.88,-813.413,2.14675,300,0,0,1,0,0,0,0,0,'',0),
(9001616,42234,0,5145,4971,1,0,1,169,0,-1,0,0,-6560.49,6956.38,-805.035,5.51524,300,0,0,1,0,0,0,0,0,'',0),
(9001617,42234,0,5145,4971,1,0,1,169,0,-1,0,0,-6515.98,6958.99,-804.007,5.74213,300,0,0,1,0,0,0,0,0,'',0),
(9001618,42234,0,5145,4971,1,0,1,169,0,-1,0,0,-6535.67,7003.66,-804.356,0.36652,300,0,0,1,0,0,0,0,0,'',0),
(9001619,42234,0,5145,4971,1,0,1,169,0,-1,0,0,-6558.30,7121.50,-797.689,3.99680,300,0,0,1,0,0,0,0,0,'',0),
(9001620,42234,0,5145,4971,1,0,1,169,0,-1,0,0,-6423.58,7159.50,-798.290,4.39823,300,0,0,1,0,0,0,0,0,'',0),
(9001621,42234,0,5145,4971,1,0,1,169,0,-1,0,0,-6425.98,7078.31,-810.711,0.95993,300,0,0,1,0,0,0,0,0,'',0),
(9001622,42234,0,5145,4971,1,0,1,169,0,-1,0,0,-6471.14,7039.60,-812.910,3.54302,300,0,0,1,0,0,0,0,0,'',0),
(9001623,42234,0,5145,4971,1,0,1,169,0,-1,0,0,-6397.56,7003.15,-813.367,2.75762,300,0,0,1,0,0,0,0,0,'',0),
(9001624,42234,0,5145,4971,1,0,1,169,0,-1,0,0,-6388.14,7126.90,-803.692,3.52556,300,0,0,1,0,0,0,0,0,'',0);

-- ############################################################################
-- SECTION 3 — TEMPLATE / RELATION / CHAIN FIXES
-- ############################################################################
-- Taylor 41598: retail gossip menu 11623 (AB3 §6); npc_text 16236 is the
-- Alliance variant of Nazgrim's 16241 PP speech — verified content match.
UPDATE `creature_template` SET `gossip_menu_id`=11623 WHERE `entry`=41598;
DELETE FROM `gossip_menu` WHERE `MenuID`=11623;
INSERT INTO `gossip_menu` (`MenuID`,`TextID`,`VerifiedBuild`) VALUES (11623,16236,0);

-- SmartAI enablement for entries scripted below (all exclusively batch-D owned
-- except 41666, which gets high SAI ids 20+ to coexist with the A-sniff batch).
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry` IN (41666,41669,41916,42007,42099,42394,41642,42234);
UPDATE `gameobject_template` SET `AIName`='SmartGameObjectAI' WHERE `entry`=203461;

-- 41927 Devious Great-Eel: enable loot (Enormous Eel Egg source, 26019/26090)
UPDATE `creature_template` SET `lootid`=41927 WHERE `entry`=41927;

-- 26014: Humphrey mount shell 41910 carries visible rider 41907 (WPP row)
DELETE FROM `vehicle_template_accessory` WHERE `entry`=41910 AND `seat_id`=0;
INSERT INTO `vehicle_template_accessory` (`entry`,`accessory_entry`,`seat_id`,`minion`,`description`,`summontype`,`summontimer`) VALUES
(41910,41907,0,1,'Humphrey Digsong - upside-down brother rider',6,30000);

-- 26154 Twilight Extermination: ender = 42281 Twilight Devotee (sniffed fact,
-- missing in TDB and WPP; 42281 has 138 spawns)
DELETE FROM `creature_questender` WHERE `id`=42281 AND `quest`=26154;
INSERT INTO `creature_questender` (`id`,`quest`) VALUES (42281,26154);

-- 26019/26090 egg orphans: item 56570 has StartQuest=0 in the 4.3.4 client
-- (Item-sparse verified) -> cannot item-start. Workaround: quest offered by
-- Digsong/Orako, visible only while carrying the egg (conditions below).
DELETE FROM `creature_queststarter` WHERE (`id`=41910 AND `quest`=26019) OR (`id`=41908 AND `quest`=26090);
INSERT INTO `creature_queststarter` (`id`,`quest`) VALUES (41910,26019),(41908,26090);

-- Completion-event gating: 25988 Put It On (H) + 26106/26126 fuel quests have
-- no objectives; SpecialFlags=2 (EXPLORATION_OR_EVENT) makes them wait for the
-- scripted event instead of auto-completing on accept.
UPDATE `quest_template_addon` SET `SpecialFlags`=2 WHERE `ID` IN (25988,26106,26126);

-- ############################################################################
-- SECTION 4 — QUEST-AVAILABILITY CONDITIONS (source 19)
-- ############################################################################
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=19 AND `SourceEntry` IN (26019,26090,26193,26194);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(19,0,26019,0,0,2,0,56570,1,0,0,0,0,'','Enormous Eel Egg (A): only offered while carrying the egg'),
(19,0,26090,0,0,2,0,56570,1,0,0,0,0,'','I Brought You This Egg (H): only offered while carrying the egg'),
(19,0,26193,0,0,6,0,469,0,0,0,0,0,'','Defending the Rift (A): Alliance only'),
(19,0,26193,0,0,8,0,26181,0,0,0,0,0,'','Defending the Rift (A): requires Back to Darkbreak Cove rewarded'),
(19,0,26194,0,0,6,0,67,0,0,0,0,0,'','Defending the Rift (H): Horde only'),
(19,0,26194,0,0,8,0,26182,0,0,0,0,0,'','Defending the Rift (H): requires Back to the Tenebrous Cavern rewarded');

-- ############################################################################
-- SECTION 5 — LOOT FIXES (AB3 §4/§6, AB4 §2a)
-- ############################################################################
-- Quest-objective drops must not drop off-quest
UPDATE `creature_loot_template` SET `QuestRequired`=1 WHERE (`Entry`=41593 AND `Item`=56235)   -- Deepfin Scrounger: Reclaimed Treasures (25975/25976)
   OR (`Entry`=41601 AND `Item`=56254)                                                        -- Merciless One: heads (25981/25982)
   OR (`Entry` IN (41922,41923) AND `Item` IN (56568,56573))                                  -- Scuttler/Grouper: Phosphora (26015/26087)
   OR (`Entry`=41648 AND `Item`=56822)                                                        -- Ick'thys: quest drop (26111 chain)
   OR (`Entry`=41657 AND `Item`=57096)                                                        -- Twilight Candidate: 26141 runestone item
   OR (`Entry`=41017 AND `Item`=55188);                                                       -- AB4: Gilblin Collector -> Medallion Fragment (25419)

-- Faction split on the Prisoners starter keys (57102=A 26144, 57118=H 26149)
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=1 AND `SourceGroup` IN (41652,41657) AND `SourceEntry` IN (57102,57118);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(1,41652,57102,0,0,6,0,469,0,0,0,0,0,'','Cell Door Key (A) from Twilight Champion: Alliance only'),
(1,41652,57118,0,0,6,0,67,0,0,0,0,0,'','Cell Door Key (H) from Twilight Champion: Horde only'),
(1,41657,57102,0,0,6,0,469,0,0,0,0,0,'','Cell Door Key (A) from Twilight Candidate: Alliance only'),
(1,41657,57118,0,0,6,0,67,0,0,0,0,0,'','Cell Door Key (H) from Twilight Candidate: Horde only');

-- 56570 Enormous Eel Egg from Devious Great-Eel (26019/26090 hook):
-- drops once the Digsong/Orako arc is open, until the egg quest is done.
DELETE FROM `creature_loot_template` WHERE `Entry`=41927 AND `Item`=56570;
INSERT INTO `creature_loot_template` (`Entry`,`Item`,`Reference`,`Chance`,`QuestRequired`,`IsCurrency`,`LootMode`,`GroupId`,`MinCount`,`MaxCount`,`Comment`) VALUES
(41927,56570,0,100,0,0,1,0,1,1,'Enormous Eel Egg - starts 26019/26090 via queststarter+condition');
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=1 AND `SourceGroup`=41927 AND `SourceEntry`=56570;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(1,41927,56570,0,0,8,0,26014,0,0,0,0,0,'','Eel Egg: Brothers Digsong (A) rewarded'),
(1,41927,56570,0,0,8,0,26019,0,0,1,0,0,'','Eel Egg: 26019 not yet rewarded'),
(1,41927,56570,0,0,8,0,26090,0,0,1,0,0,'','Eel Egg: 26090 not yet rewarded'),
(1,41927,56570,0,0,2,0,56570,1,1,1,0,0,'','Eel Egg: not already carrying one'),
(1,41927,56570,0,1,8,0,26086,0,0,0,0,0,'','Eel Egg: Orako breadcrumb (H) rewarded'),
(1,41927,56570,0,1,8,0,26019,0,0,1,0,0,'','Eel Egg: 26019 not yet rewarded'),
(1,41927,56570,0,1,8,0,26090,0,0,1,0,0,'','Eel Egg: 26090 not yet rewarded'),
(1,41927,56570,0,1,2,0,56570,1,1,1,0,0,'','Eel Egg: not already carrying one');

-- AB4 §2b cleanup: 55805 Pewter Pounder loot on King Gurboggle can never drop
-- (its quest 25742 is deprecated/disabled) — remove the dead row.
DELETE FROM `creature_loot_template` WHERE `Entry`=41018 AND `Item`=55805;

-- ############################################################################
-- SECTION 6 — SPELL-CAST CONDITIONS (source 17/13)
-- ############################################################################
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=17 AND `SourceEntry` IN (77935,77951,78277,78514,79127);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry`=78547;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
-- 25977/25980: plant the standard only at Azrajar's perch (elite 41590)
(17,0,77935,0,0,29,0,41590,40,0,0,0,0,'','A Standard Day (A): must be near Fathom-Caller Azrajar'),
(17,0,77951,0,0,29,0,41590,40,0,0,0,0,'','A Standard Day (H): must be near Fathom-Caller Azrajar'),
-- 26021/26091: Eel-Splosive Device only near a Devious Great-Eel
(17,0,78277,0,0,29,0,41927,15,0,0,0,0,'','Eel-Egg-Trick (A): Devious Great-Eel within 15yd'),
(17,0,78514,0,0,29,0,41927,15,0,0,0,0,'','Here Fishie 2 (H): Devious Great-Eel within 15yd'),
-- 26065: Shrinkage Totem only near Wil'hai (30yd — he is huge; walkthrough knob)
(17,0,79127,0,0,29,0,41642,30,0,0,0,0,'','Free Wil''hai: Wil''hai within 30yd'),
-- 26103/26122: Sampling Pump (78547) only hits Terrapin/Remora
(13,1,78547,0,0,31,0,3,42108,0,0,0,0,'','Extract Oil: Seabrush Terrapin only'),
(13,1,78547,0,1,31,0,3,42112,0,0,0,0,'','Extract Oil: Scourgut Remora only');

-- ############################################################################
-- SECTION 7 — SMART SCRIPTS
-- ############################################################################

-- ------------- 26017 A Lure / 26088 Here Fishie Fishie -------------
-- 41916 Underlight Nibbler (x318, no prior SAI): swims to a hat-wearer,
-- gets captured (78191 Captured Nibbler: CREATE_ITEM 56569 on target), despawns.
DELETE FROM `smart_scripts` WHERE `entryorguid`=41916 AND `source_type`=0 AND `id` IN (0,1);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(41916,0,0,1,10,0,100,0,1,6,3000,3000,0,11,78191,0,0,0,0,0,7,0,0,0,0,0,0,0,'Underlight Nibbler - OOC LOS (hat-wearer) - cast Captured Nibbler on player'),
(41916,0,1,0,61,0,100,0,0,0,0,0,0,41,1500,0,0,0,0,0,1,0,0,0,0,0,0,0,'Underlight Nibbler - linked - despawn (captured)');
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=22 AND `SourceGroup`=1 AND `SourceEntry`=41916 AND `SourceId`=0;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(22,1,41916,0,0,9,0,26017,0,0,0,0,0,'','Nibbler capture: on A Lure'),
(22,1,41916,0,0,3,0,56572,0,0,0,0,0,'','Nibbler capture: Handsome Hat equipped'),
(22,1,41916,0,1,9,0,26088,0,0,0,0,0,'','Nibbler capture: on Here Fishie Fishie'),
(22,1,41916,0,1,3,0,56813,0,0,0,0,0,'','Nibbler capture: Fashionable Hat equipped');

-- ------------- 26021 / 26091 Eel-Egg-Trick Boogaloo -------------
-- Egg-splosive Bunny (42007 A / 42099 H, summoned by 78277/78514): credits
-- 42007 (shared objective) to summoner, swaps nearest Devious Great-Eel for
-- 42006 Weakened Great-Eel (KillCredit1=41927 -> objective 2).
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (42007,42099) AND `source_type`=0 AND `id`=0;
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (4200700,4209900) AND `source_type`=9;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(42007,0,0,0,54,0,100,0,0,0,0,0,0,80,4200700,0,2,0,0,0,1,0,0,0,0,0,0,0,'Egg-splosive Bunny (A) - just summoned - run detonation list'),
(42099,0,0,0,54,0,100,0,0,0,0,0,0,80,4209900,0,2,0,0,0,1,0,0,0,0,0,0,0,'Egg-splosive Bunny (H) - just summoned - run detonation list'),
(4200700,9,0,0,0,0,100,0,500,500,0,0,0,33,42007,0,0,0,0,0,23,0,0,0,0,0,0,0,'Bunny (A) - credit 42007 to summoner'),
(4200700,9,1,0,0,0,100,0,500,500,0,0,0,12,42006,1,120000,0,0,0,19,41927,15,0,0,0,0,0,'Bunny (A) - summon Weakened Great-Eel at nearest eel'),
(4200700,9,2,0,0,0,100,0,200,200,0,0,0,41,0,0,0,0,0,0,19,41927,15,0,0,0,0,0,'Bunny (A) - despawn the original eel'),
(4200700,9,3,0,0,0,100,0,300,300,0,0,0,41,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Bunny (A) - self cleanup'),
(4209900,9,0,0,0,0,100,0,500,500,0,0,0,33,42007,0,0,0,0,0,23,0,0,0,0,0,0,0,'Bunny (H) - credit 42007 to summoner'),
(4209900,9,1,0,0,0,100,0,500,500,0,0,0,12,42006,1,120000,0,0,0,19,41927,15,0,0,0,0,0,'Bunny (H) - summon Weakened Great-Eel at nearest eel'),
(4209900,9,2,0,0,0,100,0,200,200,0,0,0,41,0,0,0,0,0,0,19,41927,15,0,0,0,0,0,'Bunny (H) - despawn the original eel'),
(4209900,9,3,0,0,0,100,0,300,300,0,0,0,41,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Bunny (H) - self cleanup');

-- ------------- 26065 Free Wil'hai -------------
-- 42394 Shrinkage Totem (summoned by 79127): credit + trigger Wil'hai RP.
-- 41642 Wil'hai: sheds bound-aura 78326, swims off, despawns (respawn 300s).
DELETE FROM `smart_scripts` WHERE `entryorguid`=42394 AND `source_type`=0 AND `id`=0;
DELETE FROM `smart_scripts` WHERE `entryorguid`=4239400 AND `source_type`=9;
DELETE FROM `smart_scripts` WHERE `entryorguid`=41642 AND `source_type`=0 AND `id` IN (0,1,2);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(42394,0,0,0,1,0,100,0,1500,1500,0,0,0,80,4239400,0,2,0,0,0,1,0,0,0,0,0,0,0,'Shrinkage Totem - on spawn (OOC update once) - run list'),
(4239400,9,0,0,0,0,100,0,0,0,0,0,0,33,41642,0,0,0,0,0,23,0,0,0,0,0,0,0,'Shrinkage Totem - credit Wil''hai to summoner'),
(4239400,9,1,0,0,0,100,0,1000,1000,0,0,0,45,1,1,0,0,0,0,19,41642,60,0,0,0,0,0,'Shrinkage Totem - set data on Wil''hai (start RP)'),
(41642,0,0,1,38,0,100,0,1,1,60000,60000,0,28,78326,0,0,0,0,0,1,0,0,0,0,0,0,0,'Wil''hai - data set - remove bound aura'),
(41642,0,1,2,61,0,100,0,0,0,0,0,0,69,1,0,1,0,0,0,8,0,0,0,-5680,5875,-925,0,'Wil''hai - linked - swim off into the depths'),
(41642,0,2,0,61,0,100,0,0,0,0,0,0,41,9000,0,0,0,0,0,1,0,0,0,0,0,0,0,'Wil''hai - linked - despawn after swim (respawns 300s)');

-- ------------- 26103 Bio-Fuel / 26122 Environmental Awareness -------------
-- Pump spell 78547 hit -> grant oil to the caster, animal expires.
-- (42108 keeps existing SAI id 0; 42112 keeps ids 0-1.)
DELETE FROM `smart_scripts` WHERE `entryorguid`=42108 AND `source_type`=0 AND `id` IN (1,2);
DELETE FROM `smart_scripts` WHERE `entryorguid`=42112 AND `source_type`=0 AND `id` IN (2,3);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(42108,0,1,2,8,0,100,0,78547,0,1000,1000,0,56,56818,1,0,0,0,0,7,0,0,0,0,0,0,0,'Seabrush Terrapin - hit by Extract Oil - give Terrapin Oil'),
(42108,0,2,0,61,0,100,0,0,0,0,0,0,41,2500,0,0,0,0,0,1,0,0,0,0,0,0,0,'Seabrush Terrapin - linked - expire'),
(42112,0,2,3,8,0,100,0,78547,0,1000,1000,0,56,56819,1,0,0,0,0,7,0,0,0,0,0,0,0,'Scourgut Remora - hit by Extract Oil - give Remora Oil'),
(42112,0,3,0,61,0,100,0,0,0,0,0,0,41,2500,0,0,0,0,0,1,0,0,0,0,0,0,0,'Scourgut Remora - linked - expire');

-- ------------- 26106 Fuel-ology 101 (A) / 26126 The Perfect Fuel (H) -------------
-- Completion via retail gossip line (menu 11535/11536 option 7, rows already in
-- DB) AND via the spawned Fuel Sampling Station GO 203461 (quest log says to
-- use the station; both paths call the quest event).
DELETE FROM `smart_scripts` WHERE `entryorguid`=41666 AND `source_type`=0 AND `id` IN (20,21);
DELETE FROM `smart_scripts` WHERE `entryorguid`=41669 AND `source_type`=0 AND `id` IN (0,1,2,3,4);
DELETE FROM `smart_scripts` WHERE `entryorguid`=203461 AND `source_type`=1 AND `id` IN (0,1,2);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(41666,0,20,21,62,0,100,0,11535,7,0,0,0,15,26106,0,0,0,0,0,7,0,0,0,0,0,0,0,'Hexascrub - gossip opt 7 (fuel sample) - complete Fuel-ology 101'),
(41666,0,21,0,61,0,100,0,0,0,0,0,0,72,0,0,0,0,0,0,7,0,0,0,0,0,0,0,'Hexascrub - linked - close gossip'),
(41669,0,3,4,62,0,100,0,11536,7,0,0,0,15,26126,0,0,0,0,0,7,0,0,0,0,0,0,0,'Sizzlegrin - gossip opt 7 (fuel sample) - complete The Perfect Fuel'),
(41669,0,4,0,61,0,100,0,0,0,0,0,0,72,0,0,0,0,0,0,7,0,0,0,0,0,0,0,'Sizzlegrin - linked - close gossip'),
(203461,1,0,1,64,0,100,0,0,0,0,0,0,15,26106,0,0,0,0,0,7,0,0,0,0,0,0,0,'Fuel Sampling Station - on use - complete Fuel-ology 101 (A)'),
(203461,1,1,2,61,0,100,0,0,0,0,0,0,15,26126,0,0,0,0,0,7,0,0,0,0,0,0,0,'Fuel Sampling Station - linked - complete The Perfect Fuel (H)'),
(203461,1,2,0,61,0,100,0,0,0,0,0,0,72,0,0,0,0,0,0,7,0,0,0,0,0,0,0,'Fuel Sampling Station - linked - close gossip');

-- ------------- 25988 Put It On (H twin of sniffed 25987) -------------
-- Per-turn-in flavor event at Tenebrous Cavern: giver 41669 summons the H
-- event copies (41852 Sizzlegrin / 41886 Toldrek / 41887 Gertrude + shared
-- 41840 Merciless One) around his stand, plays the freak-out, then flags the
-- quest event for the accepting player. Gossip 11536 opt 0 replays it.
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(41669,0,0,0,19,0,100,0,25988,0,0,0,0,80,4166900,0,2,0,0,0,1,0,0,0,0,0,0,0,'Sizzlegrin - accepted Put It On - run mask event'),
(41669,0,1,2,62,0,100,0,11536,0,0,0,0,72,0,0,0,0,0,0,7,0,0,0,0,0,0,0,'Sizzlegrin - gossip opt 0 (see it again) - close gossip'),
(41669,0,2,0,61,0,100,0,0,0,0,0,0,80,4166900,0,2,0,0,0,1,0,0,0,0,0,0,0,'Sizzlegrin - linked - replay mask event');
DELETE FROM `smart_scripts` WHERE `entryorguid`=4166900 AND `source_type`=9;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(4166900,9,0,0,0,0,100,0,0,0,0,0,0,12,41840,2,21000,0,0,0,8,0,0,0,-6559.5,6133.0,-671.0,0.80,'Put It On (H) - summon Merciless One'),
(4166900,9,1,0,0,0,100,0,0,0,0,0,0,12,41852,2,21000,0,0,0,8,0,0,0,-6554.3,6132.6,-671.0,2.40,'Put It On (H) - summon Sizzlegrin copy'),
(4166900,9,2,0,0,0,100,0,0,0,0,0,0,12,41886,2,21000,0,0,0,8,0,0,0,-6561.0,6139.5,-671.0,5.50,'Put It On (H) - summon Toldrek copy'),
(4166900,9,3,0,0,0,100,0,0,0,0,0,0,12,41887,2,21000,0,0,0,8,0,0,0,-6552.8,6139.0,-671.0,4.20,'Put It On (H) - summon Gertrude copy'),
(4166900,9,4,0,0,0,100,0,1500,1500,0,0,0,1,0,2500,0,0,0,0,19,41852,20,0,0,0,0,0,'Put It On (H) - Sizzlegrin copy line 0'),
(4166900,9,5,0,0,0,100,0,1500,1500,0,0,0,1,0,2500,0,0,0,0,19,41840,20,0,0,0,0,0,'Put It On (H) - Merciless One line 0'),
(4166900,9,6,0,0,0,100,0,1500,1500,0,0,0,1,0,2500,0,0,0,0,19,41886,20,0,0,0,0,0,'Put It On (H) - Toldrek copy line 0'),
(4166900,9,7,0,0,0,100,0,1500,1500,0,0,0,1,0,2500,0,0,0,0,19,41887,20,0,0,0,0,0,'Put It On (H) - Gertrude copy line 0'),
(4166900,9,8,0,0,0,100,0,1500,1500,0,0,0,1,1,2500,0,0,0,0,19,41840,20,0,0,0,0,0,'Put It On (H) - Merciless One line 1'),
(4166900,9,9,0,0,0,100,0,1500,1500,0,0,0,1,1,2500,0,0,0,0,19,41886,20,0,0,0,0,0,'Put It On (H) - Toldrek copy line 1'),
(4166900,9,10,0,0,0,100,0,1500,1500,0,0,0,1,1,2500,0,0,0,0,19,41852,20,0,0,0,0,0,'Put It On (H) - Sizzlegrin copy line 1'),
(4166900,9,11,0,0,0,100,0,1500,1500,0,0,0,1,1,2500,0,0,0,0,19,41887,20,0,0,0,0,0,'Put It On (H) - Gertrude copy line 1'),
(4166900,9,12,0,0,0,100,0,1500,1500,0,0,0,1,2,2500,0,0,0,0,19,41840,20,0,0,0,0,0,'Put It On (H) - Merciless One line 2'),
(4166900,9,13,0,0,0,100,0,1500,1500,0,0,0,1,2,2500,0,0,0,0,19,41886,20,0,0,0,0,0,'Put It On (H) - Toldrek copy line 2'),
(4166900,9,14,0,0,0,100,0,1500,1500,0,0,0,1,2,2500,0,0,0,0,19,41852,20,0,0,0,0,0,'Put It On (H) - Sizzlegrin copy line 2'),
(4166900,9,15,0,0,0,100,0,1000,1000,0,0,0,1,2,2500,0,0,0,0,19,41887,20,0,0,0,0,0,'Put It On (H) - Gertrude copy line 3'),
(4166900,9,16,0,0,0,100,0,1000,1000,0,0,0,15,25988,0,0,0,0,0,7,0,0,0,0,0,0,0,'Put It On (H) - completion event for player');

-- ------------- 26149 Prisoners (H twin of sniffed 26144) -------------
-- Horde player with the quest approaches a caged prisoner: thanks, credit,
-- prisoner slips away (respawns 300s; 16 spawns for 5 credits).
DELETE FROM `smart_scripts` WHERE `entryorguid`=42234 AND `source_type`=0 AND `id` IN (0,1,2);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(42234,0,0,1,10,0,100,0,1,8,15000,15000,0,1,0,3000,0,0,0,0,1,0,0,0,0,0,0,0,'Horde Prisoner - rescuer in LOS - thanks'),
(42234,0,1,2,61,0,100,0,0,0,0,0,0,33,42234,0,0,0,0,0,7,0,0,0,0,0,0,0,'Horde Prisoner - linked - credit rescuer'),
(42234,0,2,0,61,0,100,0,0,0,0,0,0,41,4000,0,0,0,0,0,1,0,0,0,0,0,0,0,'Horde Prisoner - linked - slip away');
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=22 AND `SourceGroup`=1 AND `SourceEntry`=42234 AND `SourceId`=0;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(22,1,42234,0,0,9,0,26149,0,0,0,0,0,'','Horde Prisoner: rescuer on Prisoners (26149)');

-- ############################################################################
-- SECTION 8 — GOSSIP OPTION CONDITIONS (source 15; option rows already in DB)
-- ############################################################################
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=15 AND ((`SourceGroup`=11535 AND `SourceEntry`=7) OR (`SourceGroup`=11536 AND `SourceEntry` IN (0,7)));
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(15,11535,7,0,0,9,0,26106,0,0,0,0,0,'','Hexascrub fuel-sample line: only while on Fuel-ology 101'),
(15,11536,7,0,0,9,0,26126,0,0,0,0,0,'','Sizzlegrin fuel-sample line: only while on The Perfect Fuel'),
(15,11536,0,0,0,8,0,25988,0,0,0,0,0,'','Sizzlegrin mask replay: Put It On rewarded'),
(15,11536,0,0,1,9,0,25988,0,0,0,0,0,'','Sizzlegrin mask replay: recovery path while quest active');

-- ############################################################################
-- SECTION 9 — CREATURE TEXT
-- 41840 / 41648 / 41659 / 42325 / 42197 verbatim from retail WPP recovery.
-- 41852 / 41886 / 41887 AUTHORED (H voices adapted from the recovered A event;
-- H dialogue unrecoverable). 42234 AUTHORED (adapted from A prisoner set,
-- $n-free because SAI talker has no talk target).
-- ############################################################################
DELETE FROM `creature_text` WHERE (`CreatureID`=41840 AND `GroupID` IN (0,1,2))
   OR (`CreatureID`=41852 AND `GroupID` IN (0,1,2))
   OR (`CreatureID`=41886 AND `GroupID` IN (0,1,2))
   OR (`CreatureID`=41887 AND `GroupID` IN (0,1,2))
   OR (`CreatureID`=42234 AND `GroupID`=0)
   OR (`CreatureID`=41648 AND `GroupID`=0)
   OR (`CreatureID`=41659 AND `GroupID`=0)
   OR (`CreatureID`=42325 AND `GroupID` IN (0,1))
   OR (`CreatureID`=42197 AND `GroupID`=0);
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
-- Merciless One event copy (shared A/H) — WPP verbatim
(41840,0,0,'|cFF68228BI SEE YOU.|r',14,0,100,0,0,14989,0,0,0,'Merciless One - Put It On'),
(41840,1,0,'|cFF68228BDIE.|r',14,0,100,0,0,14989,0,0,0,'Merciless One - Put It On'),
(41840,2,0,'|cFF68228BYOUR SIMPLE MIND CANNOT GRASP WHAT IS TRANSPIRING.|r',14,0,100,0,0,14987,0,0,0,'Merciless One - Put It On'),
-- Fiasco Sizzlegrin event copy — AUTHORED (mirrors Hexascrub 41837)
(41852,0,0,'It''s not dead! It''s NOT DEAD! Swim for it!',14,0,100,0,0,0,0,0,0,'Sizzlegrin copy - Put It On (authored)'),
(41852,1,0,'This is YOUR fault! It was supposed to be dead! Now it''s eating your brains!',14,0,100,0,0,0,0,0,0,'Sizzlegrin copy - Put It On (authored)'),
(41852,2,0,'MOMMY!',14,0,100,0,0,0,0,0,0,'Sizzlegrin copy - Put It On (authored)'),
-- Blood Guard Toldrek event copy — AUTHORED (mirrors Jorlan 41884)
(41886,0,0,'What in the Nether is that?!',14,0,100,5,0,0,0,0,0,'Toldrek copy - Put It On (authored)'),
(41886,1,0,'Stop flailing around and FACE it, you cowards!',14,0,100,25,0,0,0,0,0,'Toldrek copy - Put It On (authored)'),
(41886,2,0,'Somebody knock that thing off their head!',14,0,100,15,0,0,0,0,0,'Toldrek copy - Put It On (authored)'),
-- Sergeant Gertrude event copy — AUTHORED (mirrors Foxy 41889)
(41887,0,0,'Ha! Now THAT is a proper hat.',14,0,100,0,0,0,0,0,0,'Gertrude copy - Put It On (authored)'),
(41887,1,0,'Hold still. I have a knife and very steady hands.',14,0,100,0,0,0,0,0,0,'Gertrude copy - Put It On (authored)'),
(41887,2,0,'You''re on barnacle-scrubbing duty for a month for putting that on.',14,0,100,0,0,0,0,0,0,'Gertrude copy - Put It On (authored)'),
-- Horde Prisoner thanks — AUTHORED (adapted from recovered 42225 set)
(42234,0,0,'Lok''tar! I owe you my life, friend.',12,1,100,66,0,0,0,0,0,'Horde Prisoner - freed (authored)'),
(42234,0,1,'I thought they''d sacrifice me for sure. Thank you!',12,1,100,5,0,0,0,0,0,'Horde Prisoner - freed (authored)'),
(42234,0,2,'When I find my axe, every last one of these cultists dies!',12,1,100,25,0,0,0,0,0,'Horde Prisoner - freed (authored)'),
-- One-liners from WPP recovery (AB3 §6 punch list)
(41648,0,0,'I laugh at you, feeble $n, but I will oblige. Now, let us return to my master, below!',14,0,100,25,0,15136,0,0,0,'Ick''thys the Unfathomable - WPP'),
(41659,0,0,'%s begins channelling power for a massive attack!',41,0,100,0,0,0,0,0,0,'Hallazeal the Ascended - WPP'),
(42325,0,0,'%s''s bindings have been released! Full powers unlocked.',42,0,100,0,0,0,0,0,0,'Possessed Torrent - WPP'),
(42325,1,0,'I am freed! Let us slay Hallazeal in Neptulon''s name, $n. He lurks within the temple.',14,0,100,0,0,0,0,0,0,'Possessed Torrent - WPP'),
(42197,0,0,'L''ghorek Dies!',16,0,100,0,0,0,0,0,0,'L''ghorek - death emote - WPP');

-- ############################################################################
-- SECTION 10 — QUEST TEXT ROWS (all verified absent from DB; AUTHORED where
-- no sniff exists, per AB3/AB4 recommendations)
-- ############################################################################
DELETE FROM `quest_offer_reward` WHERE `ID` IN (25503,26121,27394);
INSERT INTO `quest_offer_reward` (`ID`,`Emote1`,`Emote2`,`Emote3`,`Emote4`,`EmoteDelay1`,`EmoteDelay2`,`EmoteDelay3`,`EmoteDelay4`,`RewardText`,`VerifiedBuild`) VALUES
-- AUTHORED: GO 202916 Sandy Mound turn-in (AB4 §2a)
(25503,0,0,0,0,0,0,0,0,'<You dig into the sandy mound and pry loose a barnacle-crusted chest. Whatever gilblin king or greedy cousin once laid claim to it, Blackfin''s booty is yours now.>',0),
-- AUTHORED: Jorlan Trueblade, adapted from H twin 26125 (AB3 §4 26121)
(26121,1,0,0,0,0,0,0,0,'Seven fewer Seadogs prowling our doorstep. Well fought, $N.$B$BKorthun''s End is ours, and with it a clear line to the trench. The Horde will think twice before sniffing around Darkbreak Cove again.',0),
-- AUTHORED: GO 203140 Broken Prong turn-in (AB4 §1 27394)
(27394,0,0,0,0,0,0,0,0,'<The broken prong hums against your palm. Whatever power called out to Wavespeaker Tulra from this battlefield has already moved on — deeper, toward the trench. Something down there is waking.>',0);

DELETE FROM `quest_details` WHERE `ID`=27394;
INSERT INTO `quest_details` (`ID`,`Emote1`,`Emote2`,`Emote3`,`Emote4`,`EmoteDelay1`,`EmoteDelay2`,`EmoteDelay3`,`EmoteDelay4`,`VerifiedBuild`) VALUES
(27394,1,0,0,0,0,0,0,0,0); -- AUTHORED (emote-only row; every sibling H twin has one)

DELETE FROM `quest_request_items` WHERE `ID` IN (25975,25981,26015,26017,26019,26103);
INSERT INTO `quest_request_items` (`ID`,`EmoteOnComplete`,`EmoteOnIncomplete`,`CompletionText`,`VerifiedBuild`) VALUES
-- AUTHORED A-side progress texts, adapted from the sniffed H twins
(25975,0,0,'Every coin those naga stole is a coin owed, $N. What did you recover?',0),
(25981,0,0,'What''s that smell?',0),
(26015,0,0,'Phosphora, phosphora! Did you bring enough glow-juice for the both of us?',0),
(26017,0,0,'How many did you catch?',0),
(26019,0,0,'<Humphrey squints at your bags, upside-down.>$B$BGot something for me, kid?',0),
(26103,0,0,'Do you have those oil samples? The fuel computations wait for no one.',0);

-- ############################################################################
-- SECTION 11 — DEPRECATED-QUEST DISPOSITION
-- 26104/26119/26123 already in `disables` with no starters (verified — no
-- clearing needed). AB4 sweep: the ONLY missing disables row zone-wide:
-- ############################################################################
DELETE FROM `disables` WHERE `sourceType`=1 AND `entry`=26191;
INSERT INTO `disables` (`sourceType`,`entry`,`flags`,`params_0`,`params_1`,`comment`) VALUES
(1,26191,0,'','','Deprecated quest: The Culmination of Our Efforts - unused legacy 77284 carrier, retail delivers the credit from the vision-2 exit trigger');

-- ============================================================================
-- END OF BATCH D
-- ============================================================================
