-- ============================================================================
-- SHIMMERING EXPANSE - HORDE TWIN BATCH
-- Mirrors the A-side implementation in 2026_07_18_12/13/14/15 (+16 orphans).
-- TrinityCore 4.3.4 fork (ShatterCore). Idempotent: safe to re-apply.
-- MUST APPLY AFTER updates 12-16 (adds parallel rows to structures they own).
--
-- Creature guids used: 9001700-9001725 (block 9001700-9001799).
-- GO guids used:       9001180-9001181 (block 9001180-9001199).
-- Custom PhaseIds used: 230 (H surface-rescue set, mirror of A 224),
--                       231 (H sub staging, mirror of A 228). 232 unused.
-- Retail/existing phases reused: 169 base, 179 assault era, 180 aftermath era
--   (H OR-conditions added to the existing 179/180 phase_area ladders).
-- All new spell IDs verified in 4.3.4 Spell.dbc/SpellEffect.dbc:
--   78053 Forcecast Ruins Assault Horde (E0 forcecast 78051 / E1 KC 40918)
--   78051 Summon Tamed Bombing Ray Horde (E0 summon 41868 / E2 KC 40918)
--   79241 Move Horde Occupants to Land (E0 teleport TargetA=17 DEST_DB)
--   76350 Spiralung use (E0 dummy aura on target)  |  77313 Restock Ammo (shared)
-- Fork SAI enums verified in SmartScriptMgr.h: TALK=1, CAST=11, KC=33, DESPAWN=41,
--   SET_RUN=59, MOVE_RANDOM=89, CLOSE_GOSSIP=72, TIMED_ACTIONLIST=80, SELF_CAST=85,
--   ACCEPTED_QUEST=19, SPELLHIT=8, OOC_LOS=10, GOSSIP_SELECT=62, LINK=61.
-- "AUTHORED" marks H-flavor text written for this batch (no H sniff exists).
-- "SYNTH" marks synthesized coordinates (POI-derived X/Y + terrain-anchor Z).
-- ============================================================================

-- ############################################################################
-- SECTION 1 - QUEST CHAIN / TEMPLATE FIXES (mirror the A-side fixes)
-- ############################################################################
-- Flags |= 0x400000 (UPDATE_PHASE_SHIFT) on the H quests that drive the
-- phase_area ladders, mirroring batch 13's list 25752/25755/25898/25911/26005/26219.
UPDATE `quest_template` SET `Flags`=`Flags`|0x400000 WHERE `ID` IN (25963,25966,25972,25973,26006,26221);

-- 25990 "Breaking Through" (H): same serverside-complete conversion batch 13
-- applied to A twin 25916 - hidden kill-credit objective granted by the shared
-- Duarn/Voice RP (actionlist 4163300 ends with KC 41633 on invoker's party).
UPDATE `quest_template` SET `RequiredNpcOrGo1`=41633, `RequiredNpcOrGoCount1`=1,
  `ObjectiveText1`='Listen to the Voice of Nespirah' WHERE `ID`=25990;

-- 25989 "Capture the Crab" (H): A twin 25909 is gated on BOTH shared prequests
-- 25907+25908 via their NextQuestID=25909 + negative EG -25907 (batch 13).
-- NextQuestID can only point one way, so gate the H copy through PrevQuestID:
-- 25907 sits in negative EG -25907 => core requires the whole group rewarded.
UPDATE `quest_template_addon` SET `PrevQuestID`=25907 WHERE `ID`=25989 AND `PrevQuestID`=0;

-- ############################################################################
-- SECTION 2 - QUEST TEXT (only gap left on the H side: 25593 request text)
-- All 38 H quests already have quest_details + quest_offer_reward (TDB/batch 16);
-- quest_request_items exist for every item turn-in except 25593. AUTHORED:
-- ############################################################################
DELETE FROM `quest_request_items` WHERE `ID`=25593;
INSERT INTO `quest_request_items` (`ID`,`EmoteOnComplete`,`EmoteOnIncomplete`,`CompletionText`,`VerifiedBuild`) VALUES
(25593,1,0,'Every shell we strap to a survivor is one less of us lost to the depths. Do you have those Spiralungs, $c?',0);

-- ############################################################################
-- SECTION 3 - LOOT: 55141 Spiralung (25593 objective item, currently sourceless)
-- Source per AB1: 39745 "Spiralung" (93 spawns). Template lootid is 0.
-- ############################################################################
UPDATE `creature_template` SET `lootid`=39745 WHERE `entry`=39745 AND `lootid`=0;
DELETE FROM `creature_loot_template` WHERE `Entry`=39745 AND `Item`=55141;
INSERT INTO `creature_loot_template` (`Entry`,`Item`,`Reference`,`Chance`,`QuestRequired`,`IsCurrency`,`LootMode`,`GroupId`,`MinCount`,`MaxCount`,`Comment`) VALUES
(39745,55141,0,75,1,0,1,0,1,1,'Spiralung - Spiralung (25593 Shelled Salvation, quest drop)');

-- ############################################################################
-- SECTION 4 - SPELL SUPPORT
-- ############################################################################
-- 79241 Move Horde Occupants to Land: E0 teleport, TargetA=17 DEST_DB (verified)
-- -> dest at the Darkbreak Cove H landing beside ender 41663 Capt. "Jewels" Verne
--    (spawned at -6577.6 6138 -671.1; 26221 turn-in POI blob -6585/6126). SYNTH dest.
-- 78051 Summon Tamed Bombing Ray Horde: E0 summon TargetA=0 (caster dest) like the
-- A twin 77322; defensive DEST row mirrors batch 13's 77322 row. SYNTH dest at the
-- H staging camp where the assault gossip happens.
DELETE FROM `spell_target_position` WHERE `ID` IN (79241,78051);
INSERT INTO `spell_target_position` (`ID`,`EffectIndex`,`MapID`,`PositionX`,`PositionY`,`PositionZ`,`Orientation`,`VerifiedBuild`) VALUES
(79241,0,0,-6590.0,6125.0,-668.0,1.2,0),  -- H voyage landing (Darkbreak Cove, near Verne) SYNTH
(78051,0,0,-6531.0,4785.0,-600.0,3.6,0);  -- H bombing ray fallback dest (H staging camp) SYNTH

-- H Tamed Bombing Ray 41868: mirror the A ray 41247 (VehicleId 816, bomb spell
-- 77330 Improvised Explosives shared, run-finish despawn on 77342 hit).
UPDATE `creature_template` SET `VehicleId`=816, `spell1`=77330, `AIName`='SmartAI' WHERE `entry`=41868;

-- ############################################################################
-- SECTION 5 - CONDITIONS: parallel H OR-rows on A-owned gates (ADD ONLY -
-- every A row lives in ElseGroup 0; all rows below use ElseGroup 1)
-- ############################################################################
-- 5a. SAI event conditions (source 22) - H quest OR'd onto the shared actors
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=22 AND `ElseGroup`=1 AND `SourceEntry` IN (41160,41436,41484,40963,40964,40965,-9001358,-9001359,-9001360,-9001361,-9001407,-9001436);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(22,1,41160,0,1,9,0,25957,0,0,0,0,0,'','V1 bunny also reacts on H 25957'),
(22,1,41436,0,1,9,0,25966,0,0,0,0,0,'','V2 bunny also reacts on H 25966'),
(22,1,41484,0,1,9,0,26135,0,0,0,0,0,'','V3 bunny also reacts on H 26135'),
(22,1,40963,0,1,9,0,25955,0,0,0,0,0,'','Gardens scout credit also on H A Better Vantage 25955'),
(22,1,40964,0,1,9,0,25955,0,0,0,0,0,'','Tunnel scout credit also on H 25955'),
(22,1,40965,0,1,9,0,25955,0,0,0,0,0,'','Structures scout credit also on H 25955'),
(22,1,-9001358,0,1,9,0,25965,0,0,0,0,0,'','NW scout whisper also on H Gauging Success 25965'),
(22,1,-9001359,0,1,9,0,25965,0,0,0,0,0,'','Tunnel scout whisper also on H 25965'),
(22,1,-9001360,0,1,47,0,25963,2,0,0,0,0,'','Bomb controller S also finishes H Swift Action 25963'),
(22,1,-9001361,0,1,47,0,25963,2,0,0,0,0,'','Bomb controller N also finishes H 25963'),
(22,1,-9001407,0,1,9,0,25966,0,0,0,0,0,'','V2 exit trigger also on H 25966'),
(22,1,-9001436,0,1,9,0,26135,0,0,0,0,0,'','Crucible whisper/credit also on H 26135');

-- 5b. Gossip option conditions (source 15)
-- Duarn menu 11525 opt1 (Voice summon) also while H Breaking Through taken;
-- Fiasco 40918 menu 11534 opt0 ("I'm ready to begin the assault") on H 25963.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=15 AND `SourceGroup`=11525 AND `SourceEntry`=1 AND `ElseGroup`=1;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=15 AND `SourceGroup`=11534;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(15,11525,1,0,1,9,0,25990,0,0,0,0,0,'','Duarn opt1 - H Breaking Through (25990) taken'),
(15,11534,0,0,0,9,0,25963,0,0,0,0,0,'','Fiasco assault option only while H Swift Action (25963) taken');

-- 5c. Spellclick conditions (source 18)
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=18 AND `SourceGroup` IN (41520,41776) AND `ElseGroup`=1;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(18,41520,77684,0,1,9,0,25989,0,0,0,0,0,'','Crab click - H Capture the Crab (25989) taken'),
(18,41520,77684,0,1,1,1,77682,0,0,0,0,0,'','Crab click - crab netted (aura 77682, H path)'),
(18,41776,77927,0,1,9,0,25996,0,0,0,0,0,'','Escape seahorse click - H Waking the Beast (25996) taken');

-- ############################################################################
-- SECTION 6 - PHASING
-- 6a. H OR-groups on the existing assault/aftermath era ladders (179/180).
--     A rows are ElseGroup 0 (25752/25755); H mirror = ElseGroup 1 (25963/25966).
-- ############################################################################
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=26 AND `SourceGroup` IN (179,180) AND `ElseGroup`=1;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
-- phase 179 H: 25963 started AND 25966 not rewarded AND not in a vision
(26,179,4967,0,1,47,0,25963,74,0,0,0,0,'','P179 H: Swift Action (25963) started'),
(26,179,4967,0,1,8,0,25966,0,0,1,0,0,'','P179 H: The Slaughter (25966) not rewarded'),
(26,179,4967,0,1,1,0,73974,0,0,1,0,0,'','P179 H: not in vision 1'),
(26,179,4967,0,1,1,0,77565,0,0,1,0,0,'','P179 H: not in vision 2'),
(26,179,4967,0,1,1,0,78264,0,0,1,0,0,'','P179 H: not in vision 3'),
(26,179,4968,0,1,47,0,25963,74,0,0,0,0,'','P179 H: Swift Action started'),
(26,179,4968,0,1,8,0,25966,0,0,1,0,0,'','P179 H: 25966 not rewarded'),
(26,179,4968,0,1,1,0,73974,0,0,1,0,0,'','P179 H: not in vision 1'),
(26,179,4968,0,1,1,0,77565,0,0,1,0,0,'','P179 H: not in vision 2'),
(26,179,4968,0,1,1,0,78264,0,0,1,0,0,'','P179 H: not in vision 3'),
(26,179,5090,0,1,47,0,25963,74,0,0,0,0,'','P179 H: Swift Action started'),
(26,179,5090,0,1,8,0,25966,0,0,1,0,0,'','P179 H: 25966 not rewarded'),
(26,179,5090,0,1,1,0,73974,0,0,1,0,0,'','P179 H: not in vision 1'),
(26,179,5090,0,1,1,0,77565,0,0,1,0,0,'','P179 H: not in vision 2'),
(26,179,5090,0,1,1,0,78264,0,0,1,0,0,'','P179 H: not in vision 3'),
(26,179,5124,0,1,47,0,25963,74,0,0,0,0,'','P179 H: Swift Action started'),
(26,179,5124,0,1,8,0,25966,0,0,1,0,0,'','P179 H: 25966 not rewarded'),
(26,179,5124,0,1,1,0,73974,0,0,1,0,0,'','P179 H: not in vision 1'),
(26,179,5124,0,1,1,0,77565,0,0,1,0,0,'','P179 H: not in vision 2'),
(26,179,5124,0,1,1,0,78264,0,0,1,0,0,'','P179 H: not in vision 3'),
-- phase 180 H: 25966 rewarded AND not in a vision
(26,180,4967,0,1,8,0,25966,0,0,0,0,0,'','P180 H: The Slaughter (25966) rewarded'),
(26,180,4967,0,1,1,0,77565,0,0,1,0,0,'','P180 H: not in vision 2'),
(26,180,4967,0,1,1,0,78264,0,0,1,0,0,'','P180 H: not in vision 3'),
(26,180,4968,0,1,8,0,25966,0,0,0,0,0,'','P180 H: 25966 rewarded'),
(26,180,4968,0,1,1,0,77565,0,0,1,0,0,'','P180 H: not in vision 2'),
(26,180,4968,0,1,1,0,78264,0,0,1,0,0,'','P180 H: not in vision 3'),
(26,180,5090,0,1,8,0,25966,0,0,0,0,0,'','P180 H: 25966 rewarded'),
(26,180,5090,0,1,1,0,77565,0,0,1,0,0,'','P180 H: not in vision 2'),
(26,180,5090,0,1,1,0,78264,0,0,1,0,0,'','P180 H: not in vision 3'),
(26,180,5124,0,1,8,0,25966,0,0,0,0,0,'','P180 H: 25966 rewarded'),
(26,180,5124,0,1,1,0,77565,0,0,1,0,0,'','P180 H: not in vision 2'),
(26,180,5124,0,1,1,0,78264,0,0,1,0,0,'','P180 H: not in vision 3'),
(26,180,4966,0,1,8,0,25966,0,0,0,0,0,'','P180 H: 25966 rewarded (forward camps)'),
(26,180,4966,0,1,1,0,77565,0,0,1,0,0,'','P180 H: not in vision 2'),
(26,180,4966,0,1,1,0,78264,0,0,1,0,0,'','P180 H: not in vision 3'),
(26,180,4969,0,1,8,0,25966,0,0,0,0,0,'','P180 H: 25966 rewarded (forward camps)'),
(26,180,4969,0,1,1,0,77565,0,0,1,0,0,'','P180 H: not in vision 2'),
(26,180,4969,0,1,1,0,78264,0,0,1,0,0,'','P180 H: not in vision 3');

-- 6b. NEW phase 230 = H surface-rescue set (mirror of A 224: 25898/25911 -> 25972/25973)
--     NEW phase 231 = H sub staging (mirror of A 228: 26005/26219 -> 26006/26221).
--     231 rides area 5144 (parent-zone row - the H camp sits in the zone catch-all
--     area) + 4955 (Voldrin's/SW seafloor overlap). Overlay semantics: phase_area
--     phases are additive in this fork (base 169 stays visible).
DELETE FROM `phase_area` WHERE `PhaseId` IN (230,231);
INSERT INTO `phase_area` (`AreaId`,`PhaseId`,`Comment`) VALUES
(4966,230,'Shimmering H - 25972 surface rescue set (Toldrek/balloon/kit)'),
(4969,230,'Shimmering H - 25972 surface rescue set'),
(5144,231,'Shimmering H - Verne sub staging above the H camp'),
(4955,231,'Shimmering H - Verne sub staging (SW seafloor/Voldrin overlap)');
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=26 AND `SourceGroup` IN (230,231);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
-- phase 230: 25972 in log OR (25972 rewarded AND 25973 not rewarded)
(26,230,4966,0,0,47,0,25972,10,0,0,0,0,'','P230: Honor and Privilege (H) in log'),
(26,230,4966,0,1,8,0,25972,0,0,0,0,0,'','P230: 25972 rewarded'),
(26,230,4966,0,1,8,0,25973,0,0,1,0,0,'','P230: Welcome News (H) not yet rewarded'),
(26,230,4969,0,0,47,0,25972,10,0,0,0,0,'','P230: 25972 in log'),
(26,230,4969,0,1,8,0,25972,0,0,0,0,0,'','P230: 25972 rewarded'),
(26,230,4969,0,1,8,0,25973,0,0,1,0,0,'','P230: 25973 not yet rewarded'),
-- phase 231: 26006 started -> 26221 rewarded
(26,231,5144,0,0,47,0,26006,74,0,0,0,0,'','P231: A Breath of Fresh Air (H) started'),
(26,231,5144,0,0,8,0,26221,0,0,1,0,0,'','P231: Full Circle (H) not rewarded'),
(26,231,4955,0,0,47,0,26006,74,0,0,0,0,'','P231: 26006 started'),
(26,231,4955,0,0,8,0,26221,0,0,1,0,0,'','P231: 26221 not rewarded');

-- ############################################################################
-- SECTION 7 - CREATURE TEMPLATE: SmartAI hookups for H actors
-- (guarded: only entries with no AI and no C++ script; 41868 handled in S4)
-- ############################################################################
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry` IN (40918,39729,41779,42410,40919,40921,41770) AND `AIName`='' AND `ScriptName`='';

-- ############################################################################
-- SECTION 8 - SPAWNS
-- Camp geometry: POI-derived X/Y; Z from 39745/41566/41569 terrain anchors and
-- A-twin analogues. Everything marked SYNTH = verify in walkthrough.
-- ############################################################################
-- 8a. Reposition Elendri Goldenbrow (H, ender 27717 / giver-ender 25954).
-- Batch 12 parked her at the A tent "pending H-sniff verify"; AB1 pins her POI
-- at (-6775, 4195) between the H camp and the garden statues. Z anchor: 39745
-- spawn -6790/4199.6/-427.9. (H-actor refinement, not an A-row rewrite.)
UPDATE `creature` SET `zoneId`=5144, `areaId`=5144, `position_x`=-6775.0, `position_y`=4195.0, `position_z`=-428.5, `orientation`=2.30 WHERE `guid`=9001249 AND `id`=40920;

DELETE FROM `creature` WHERE `guid` BETWEEN 9001700 AND 9001799;
DELETE FROM `creature_addon` WHERE `guid` BETWEEN 9001700 AND 9001799;
INSERT INTO `creature` (`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseUseFlags`,`phaseMask`,`PhaseId`,`PhaseGroup`,`terrainSwapMap`,`modelid`,`equipment_id`,`position_x`,`position_y`,`position_z`,`orientation`,`spawntimesecs`,`wander_distance`,`currentwaypoint`,`curhealth`,`curmana`,`MovementType`,`npcflag`,`unit_flags`,`dynamicflags`,`ScriptName`,`VerifiedBuild`) VALUES
-- --- H MAIN CAMP (post-Nespirah Horde camp, base phase 169; POI cluster
--     -6893..-6940 / 4295..4332, Z from 39745 anchors ~-511..-520. SYNTH Z) ---
(9001700,40917,0,5144,5144,1,0,1,169,0,-1,0,0,-6908.00,4318.00,-514.00,5.60,300,0,0,1,0,0,0,0,0,'',0), -- Legionnaire Nazgrim (giver/ender 25592/25953/25960/25964/25967/25968/25973/25996)
(9001701,40916,0,5144,5144,1,0,1,169,0,-1,0,0,-6900.50,4309.50,-514.00,5.10,300,0,0,1,0,0,0,0,0,'',0), -- Captain Vilethorn (25593/25952/25955/25956/25959/25963/25965)
(9001702,40919,0,5144,5144,1,0,1,169,0,-1,0,0,-6895.50,4321.00,-514.50,4.90,300,0,0,1,0,0,0,0,0,'',0), -- Wavespeaker Tulra (25595/25957/25958/25966/25967/26006/26135/27394)
-- --- H STAGING CAMP (ruins reoccupation, base 169; POI cluster -6510..-6520 /
--     4758..4785, Z anchors -604..-608 area 4967) ---
(9001703,40916,0,5144,4967,1,0,1,169,0,-1,0,0,-6514.00,4762.00,-606.00,2.40,300,0,0,1,0,0,0,0,0,'',0), -- Vilethorn staging (ender 25958, giver/ender 25959, giver 25963)
(9001704,40917,0,5144,4967,1,0,1,169,0,-1,0,0,-6510.00,4780.00,-606.20,3.40,300,0,0,1,0,0,0,0,0,'',0), -- Nazgrim staging (giver/ender 25960 lookout restock)
(9001705,40918,0,5144,4967,1,0,1,169,0,-1,0,0,-6519.50,4778.00,-606.10,2.90,300,0,0,1,0,0,0,0,0,'',0), -- Fiasco staging (giver/ender 25962; 25963 assault gossip/credit)
-- --- VISION-1 RENDEZVOUS (shared spot with A, base 169; beside GO 203140
--     Broken Prong at -7196.5/4717.8/-594.4) ---
(9001706,40919,0,5144,4967,1,0,1,169,0,-1,0,0,-7190.50,4723.50,-594.60,2.40,300,0,0,1,0,0,0,0,0,'',0), -- Tulra rendezvous (27394 giver, 25957 turn-in, welcome-back whisper)
-- --- ASSAULT-ERA CAMP (phase 179, terrace south bridge; mirror of A 9001353-55) ---
(9001707,40916,0,5144,4968,1,0,1,179,0,-1,0,0,-7295.50,4783.00,-426.64,1.10,300,0,0,1,0,0,0,0,0,'',0), -- Vilethorn assault camp (ender 25963/25965)
(9001708,40917,0,5144,4968,1,0,1,179,0,-1,0,0,-7300.50,4776.00,-426.64,1.30,300,0,0,1,0,0,0,0,0,'',0), -- Nazgrim assault camp (giver/ender 25964)
(9001709,40919,0,5144,4968,1,0,1,179,0,-1,0,0,-7302.50,4782.50,-426.64,1.20,300,0,0,1,0,0,0,0,0,'',0), -- Tulra assault camp (giver 25966 vision 2)
-- --- V2 EXIT (phase 180, area 5090; mirror of A 9001356; shares A 48901 seahorse) ---
(9001710,40919,0,5144,5090,1,0,1,180,0,-1,0,0,-7314.00,5252.50,-426.64,5.60,300,0,0,1,0,0,0,0,0,'',0), -- Tulra V2 exit (25966 turn-in, 25967 giver)
-- --- AFTERMATH: H injured lookout at the H staging camp (phase 180) ---
(9001711,41779,0,5144,4967,1,0,1,180,0,-1,0,0,-6526.00,4754.00,-606.30,2.00,300,0,0,1,0,0,0,0,0,'',0), -- Injured Lookout H (25967 credit; POI obj blob -6526/4754)
-- --- H FORWARD CAMP (phase 180, Bielaran Ridge area 4966; POI -7310/4239,
--     Z from 41569/41566 anchors ~-254..-266. SYNTH Z) ---
(9001712,41770,0,5144,4966,1,0,1,180,0,-1,0,0,-7310.00,4239.00,-260.00,0.90,300,0,0,1,0,0,0,0,0,'',0), -- Fiasco forward camp (ender 25968-25971, giver 25970/25971/25972)
(9001713,40921,0,5144,4966,1,0,1,180,0,-1,0,0,-7315.00,4244.00,-260.50,0.60,300,0,0,1,0,0,0,0,0,'',0), -- Blood Guard Toldrek forward camp (giver 25969)
-- --- H SURFACE RESCUE SET (phase 230; above forward camp, POI -7317/4235.
--     Z mirror of A 224 set: 0.5 / 6.0. SYNTH) ---
(9001714,40921,0,5144,4966,1,0,1,230,0,-1,0,0,-7315.00,4237.00,0.50,1.60,300,0,0,1,0,0,0,0,0,'',0),   -- Toldrek surface (25972 credit+turn-in, 25973 giver)
(9001715,41572,0,5144,4966,1,0,1,230,0,-1,0,0,-7320.00,4243.00,6.00,4.20,300,0,0,1,0,0,0,0,0,'',0),   -- Rescue Balloon H copy (flare 77741 target; entry SAI shared)
-- --- H SUB STAGING (phase 231; surface above the H camp, POI 26006 blob
--     -6895/4274 + 26221 boarding blob -6900/4258. Z ~ sea surface. SYNTH) ---
(9001716,42410,0,5144,5144,1,0,1,231,0,-1,0,0,-6898.00,4268.00,0.70,5.80,300,0,0,1,0,0,0,0,0,'',0),   -- Legionnaire Nazgrim final (ender 26006, giver 26221; mirror of A 42411-in-228)
(9001717,42486,0,5144,5144,1,0,1,231,0,-1,0,0,-6906.00,4257.00,0.50,0.00,300,0,0,1,0,0,0,0,0,'',0),   -- Boarding Submarine trigger (guid-SAI voyage actionlist)
-- --- H LOOKOUTS x8 (25960; base 169; ring inside the 25960 POI polygon
--     -6551..-6828 / 4636..4904, gardens rim Z ~-605. SYNTH) ---
(9001718,41780,0,5144,4967,1,0,1,169,0,-1,0,0,-6560.00,4700.00,-607.00,2.90,300,0,0,1,0,0,0,0,0,'',0),
(9001719,41780,0,5144,4967,1,0,1,169,0,-1,0,0,-6551.00,4790.00,-607.50,3.10,300,0,0,1,0,0,0,0,0,'',0),
(9001720,41780,0,5144,4967,1,0,1,169,0,-1,0,0,-6570.00,4860.00,-605.50,3.60,300,0,0,1,0,0,0,0,0,'',0),
(9001721,41780,0,5144,5089,1,0,1,169,0,-1,0,0,-6650.00,4895.00,-604.50,4.20,300,0,0,1,0,0,0,0,0,'',0),
(9001722,41780,0,5144,5089,1,0,1,169,0,-1,0,0,-6730.00,4900.00,-604.60,4.60,300,0,0,1,0,0,0,0,0,'',0),
(9001723,41780,0,5144,5089,1,0,1,169,0,-1,0,0,-6790.00,4870.00,-604.70,5.10,300,0,0,1,0,0,0,0,0,'',0),
(9001724,41780,0,5144,5089,1,0,1,169,0,-1,0,0,-6820.00,4790.00,-604.70,5.80,300,0,0,1,0,0,0,0,0,'',0),
(9001725,41780,0,5144,5089,1,0,1,169,0,-1,0,0,-6790.00,4670.00,-604.80,0.60,300,0,0,1,0,0,0,0,0,'',0);

-- Wounded posture for the H injured lookout (mirror of A 41562 addon row)
INSERT INTO `creature_addon` (`guid`,`waypointPathId`,`cyclicSplinePathId`,`mount`,`StandState`,`AnimTier`,`VisFlags`,`SheathState`,`PvPFlags`,`emote`,`aiAnimKit`,`movementAnimKit`,`meleeAnimKit`,`visibilityDistanceType`,`auras`) VALUES
(9001711,0,0,0,8,0,0,0,0,0,0,0,0,0,'');

-- 8b. GO spawns
DELETE FROM `gameobject` WHERE `guid` BETWEEN 9001180 AND 9001199;
INSERT INTO `gameobject` (`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseUseFlags`,`phaseMask`,`PhaseId`,`PhaseGroup`,`terrainSwapMap`,`position_x`,`position_y`,`position_z`,`orientation`,`rotation0`,`rotation1`,`rotation2`,`rotation3`,`spawntimesecs`,`animprogress`,`state`,`ScriptName`,`VerifiedBuild`) VALUES
-- 203403 Survival Kit Remnants H copy (25972 flare chest -> item 56188; loot 29668 native)
(9001180,203403,0,5144,4966,1,0,1,230,0,-1,-7317.50,4233.50,0.30,0,0,0,0,1,120,255,1,'',0),
-- 205062 Boarding Plank (H sub staging scenery, mirror of A 9001081)
(9001181,205062,0,5144,5144,1,0,1,231,0,-1,-6903.50,4259.50,0.40,0,0,0,0,1,300,255,1,'',0);

-- ############################################################################
-- SECTION 9 - CREATURE_TEXT (all AUTHORED H flavor - no H-side sniff exists)
-- ############################################################################
DELETE FROM `creature_text` WHERE (`CreatureID`=40919 AND `GroupID`=0) OR (`CreatureID`=41779 AND `GroupID`=0) OR (`CreatureID`=41780 AND `GroupID`=0) OR (`CreatureID`=40918 AND `GroupID`=0) OR (`CreatureID`=41770 AND `GroupID`=0) OR (`CreatureID`=40921 AND `GroupID` IN (0,1)) OR (`CreatureID`=39729 AND `GroupID`=0) OR (`CreatureID`=42410 AND `GroupID` IN (0,1));
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(40919,0,0,'Welcome back, $n. The spirits have shown you much - I can see it in your eyes. Tell me everything.',12,0,100,1,0,0,0,0,0,'Wavespeaker Tulra - vision 1 exit welcome (AUTHORED)'),
(41779,0,0,'They hit us while you were away... Blood Guard Toldrek pulled the survivors back toward the ridge. Go - they need you more than I do.',12,0,100,0,0,0,0,0,0,'Injured Lookout H - 25967 (AUTHORED)'),
(41780,0,0,'Lok''tar! I was down to throwing rocks.',12,0,100,5,0,0,0,0,0,'Horde Lookout - restock 1 (AUTHORED)'),
(41780,0,1,'Good timing. The naga keep probing our line.',12,0,100,5,0,0,0,0,0,'Horde Lookout - restock 2 (AUTHORED)'),
(41780,0,2,'That will hold me for now. Watch yourself out there.',12,0,100,5,0,0,0,0,0,'Horde Lookout - restock 3 (AUTHORED)'),
(41780,0,3,'Supplies! Now we''re talking.',12,0,100,5,0,0,0,0,0,'Horde Lookout - restock 4 (AUTHORED)'),
(40918,0,0,'Light ''em up good! Nothing says "hello" like high explosives!',12,0,100,0,0,0,0,0,0,'Fiasco Sizzlegrin - 25963 bombing run (AUTHORED)'),
(41770,0,0,'Up ya go! Kick and keep kicking till you see sky!',12,0,100,0,0,0,0,0,0,'Fiasco Sizzlegrin fwd - 25972 accept (AUTHORED)'),
(40921,0,0,'Air! Never thought I''d taste it again. Our ships are just north, $n - put a flare in the sky for them.',12,0,100,4,0,0,0,0,0,'Blood Guard Toldrek - surface (AUTHORED)'),
(40921,1,0,'Ha! There''s no way they miss that. Help is coming.',12,0,100,4,0,0,0,0,0,'Blood Guard Toldrek - flare fired (AUTHORED)'),
(39729,0,0,'You... you saved me. I can breathe!',12,0,100,5,0,0,0,0,0,'Nespirah Survivor - Spiralung 1 (AUTHORED)'),
(39729,0,1,'The shell... it holds air! Thank you, stranger.',12,0,100,5,0,0,0,0,0,'Nespirah Survivor - Spiralung 2 (AUTHORED)'),
(39729,0,2,'I thought I was done for. My thanks.',12,0,100,5,0,0,0,0,0,'Nespirah Survivor - Spiralung 3 (AUTHORED)'),
(42410,0,0,'The naga thought Nespirah''s belly would break our spirit. They will learn otherwise. The Verne is inbound - be ready to board!',12,0,100,0,0,0,0,0,0,'Nazgrim final - 26221 accept (AUTHORED)'),
(42410,1,0,'The Verne has surfaced! All aboard, dogs of the Horde!',14,0,100,0,0,0,0,0,0,'Nazgrim final - all aboard (AUTHORED)');

-- ############################################################################
-- SECTION 10 - SMART SCRIPTS (targeted deletes: only ids this batch owns)
-- ############################################################################
DELETE FROM `smart_scripts` WHERE `source_type`=0 AND `entryorguid`=40918 AND `id` IN (0,1,2);
DELETE FROM `smart_scripts` WHERE `source_type`=0 AND `entryorguid`=41868 AND `id`=0;
DELETE FROM `smart_scripts` WHERE `source_type`=0 AND `entryorguid`=39729 AND `id` IN (0,1,2,3,4);
DELETE FROM `smart_scripts` WHERE `source_type`=0 AND `entryorguid`=41780 AND `id` IN (1,2);
DELETE FROM `smart_scripts` WHERE `source_type`=0 AND `entryorguid`=41779 AND `id` IN (0,1);
DELETE FROM `smart_scripts` WHERE `source_type`=0 AND `entryorguid`=42410 AND `id`=0;
DELETE FROM `smart_scripts` WHERE `source_type`=0 AND `entryorguid`=41770 AND `id`=0;
DELETE FROM `smart_scripts` WHERE `source_type`=0 AND `entryorguid`=41531 AND `id`=3;
DELETE FROM `smart_scripts` WHERE `source_type`=0 AND `entryorguid` IN (-9001706,-9001714,-9001717);
DELETE FROM `smart_scripts` WHERE `source_type`=9 AND `entryorguid`=4248601;
DELETE FROM `smart_scripts` WHERE `source_type`=9 AND `entryorguid`=4157200 AND `id`=2;

INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
-- ---- 40918 Fiasco Sizzlegrin: 25963 bombing run (mirror of 40639 ids 20-22;
--      78053 = native H chain: forcecast 78051 summon-ride 41868 + KC 40918) ----
(40918,0,0,1,62,0,100,0,11534,0,0,0,0,72,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Fiasco - assault gossip - close'),
(40918,0,1,2,61,0,100,0,0,0,0,0,0,11,78053,0,0,0,0,0,7,0,0,0,0,0,0,0,'Fiasco - forcecast Ruins Assault Horde (summon+ride H ray, KC 40918)'),
(40918,0,2,0,61,0,100,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Fiasco - Light em up good'),
-- ---- 41868 H Tamed Bombing Ray: run finished -> despawn (mirror 41247 id 0) ----
(41868,0,0,0,8,0,100,0,77342,0,0,0,0,41,1500,0,0,0,0,0,1,0,0,0,0,0,0,0,'H Bombing Ray - run finished (77342) - despawn/eject'),
-- ---- 39729 Nespirah Survivor: 25593 Spiralung rescue ----
(39729,0,0,1,8,0,100,0,76350,0,2000,2000,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Nespirah Survivor - Spiralung applied - thanks'),
(39729,0,1,2,61,0,100,0,0,0,0,0,0,33,39729,0,0,0,0,0,7,0,0,0,0,0,0,0,'Nespirah Survivor - 25593 credit to user'),
(39729,0,2,3,61,0,100,0,0,0,0,0,0,59,1,0,0,0,0,0,1,0,0,0,0,0,0,0,'Nespirah Survivor - set run'),
(39729,0,3,4,61,0,100,0,0,0,0,0,0,89,10,0,0,0,0,0,1,0,0,0,0,0,0,0,'Nespirah Survivor - swim off'),
(39729,0,4,0,61,0,100,0,0,0,0,0,0,41,8000,0,0,0,0,0,1,0,0,0,0,0,0,0,'Nespirah Survivor - despawn (respawns per spawn row)'),
-- ---- 41780 Horde Lookout: 25960 restock (append after existing id 0; mirror 41235) ----
(41780,0,1,2,8,0,100,0,77313,0,5000,5000,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Horde Lookout - restocked (77313) - talk'),
(41780,0,2,0,61,0,100,0,0,0,0,0,0,33,41780,0,0,0,0,0,7,0,0,0,0,0,0,0,'Horde Lookout - 25960 restock credit'),
-- ---- 41779 Injured Lookout H: 25967 credit (mirror 41562; only my spawn exists) ----
(41779,0,0,1,10,0,100,0,1,30,30000,30000,0,1,0,0,1,0,0,0,7,0,0,0,0,0,0,0,'Injured Lookout H - they hit us while you were away'),
(41779,0,1,0,61,0,100,0,0,0,0,0,0,33,41779,0,0,0,0,0,7,0,0,0,0,0,0,0,'Injured Lookout H - 25967 credit'),
-- ---- 42410 Nazgrim final: 26221 accept flavor ----
(42410,0,0,0,19,0,100,0,26221,1000,1000,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Nazgrim final - on accept Full Circle - The Verne is inbound'),
-- ---- 41770 Fiasco forward: 25972 accept flavor (mirror 41535 "Up you go") ----
(41770,0,0,0,19,0,100,0,25972,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Fiasco fwd - Up ya go'),
-- ---- 41531 Duarn: H Waking the Beast uses the same finale actionlist ----
(41531,0,3,0,19,0,100,0,25996,0,0,0,0,80,4153102,0,0,0,0,0,1,0,0,0,0,0,0,0,'Duarn - 25996 (H) accepted - Waking the Beast finale (shared actionlist)');

-- per-guid scripts (batch-H owned guids)
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(-9001706,0,0,0,10,0,100,0,1,30,120000,120000,0,1,0,0,1,0,0,0,7,0,0,0,0,0,0,0,'Tulra rendezvous - welcome back (25957 complete)'),
(-9001714,0,0,1,10,0,100,0,1,35,60000,60000,0,1,0,0,1,0,0,0,7,0,0,0,0,0,0,0,'Surface Toldrek - Air! (25972)'),
(-9001714,0,1,0,61,0,100,0,0,0,0,0,0,33,40921,0,0,0,0,0,7,0,0,0,0,0,0,0,'Surface Toldrek - swim-up credit'),
(-9001717,0,0,0,10,0,100,0,1,12,180000,180000,0,80,4248601,0,0,0,0,0,1,0,0,0,0,0,0,0,'H boarding trigger - 26221 voyage');
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(4248601,9,0,0,0,0,100,0,0,0,0,0,0,33,42486,0,0,0,0,0,7,0,0,0,0,0,0,0,'H voyage - boarding credit'),
(4248601,9,1,0,0,0,100,0,1500,1500,0,0,0,1,1,0,0,0,0,0,19,42410,60,0,0,0,0,0,'H voyage - Nazgrim: all aboard'),
(4248601,9,2,0,0,0,100,0,12000,12000,0,0,0,33,42487,0,0,0,0,0,7,0,0,0,0,0,0,0,'H voyage - cavern credit'),
(4248601,9,3,0,0,0,100,0,3000,3000,0,0,0,85,79241,2,0,0,0,0,7,0,0,0,0,0,0,0,'H voyage - teleport to Darkbreak Cove (Move Horde Occupants to Land)'),
-- Parallel row on the A-owned balloon actionlist: Toldrek cheer (no-op at the A
-- surface set - target 19 finds no 40921 there; Jorlan row id 1 no-ops for H).
(4157200,9,2,0,0,0,100,0,0,0,0,0,0,1,1,0,0,0,0,0,19,40921,60,0,0,0,0,0,'Rescue Balloon - Toldrek cheers (H parallel)');

-- SAI event conditions for batch-H rows (source 22; SourceGroup = id+1)
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=22 AND `SourceEntry` IN (41779,-9001706,-9001714,-9001717);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(22,1,41779,0,0,9,0,25967,0,0,0,0,0,'','Injured Lookout H - Losing Ground (25967) taken'),
(22,1,-9001706,0,0,47,0,25957,2,0,0,0,0,'','Tulra rendezvous - 25957 complete'),
(22,1,-9001714,0,0,9,0,25972,0,0,0,0,0,'','Surface Toldrek - 25972 taken'),
(22,1,-9001717,0,0,9,0,26221,0,0,0,0,0,'','H boarding trigger - 26221 taken');

-- ============================================================================
-- END OF H-TWIN BATCH
-- Native/no-op on the H side (verified against DB + A batches, deliberately
-- untouched): all 38 H quest relations/POI/details/offer_reward (TDB-shipped),
-- 62281/62282 faction loot conditions (batch 12 S4.6), item 55171 hotfix +
-- 56183/56194 quest loot + GO 29644/29669/29668 loot, statue goobers 25954,
-- crab net 56184/77682/77684, Boom Boots 57412, Nespirah interior H givers
-- 41810/41811/41813 (batch 13 spawns), escape seahorse H (C++ 77927 faction BP
-- -> 77915 -> 41778, ScriptName bound in batch 14), vision interiors + Nespirah
-- chain quests (all AllowableRaces = all), 25989 crab spawns (7 up).
--
-- C++ PARAMETERIZATION STILL WANTED (functional via SAI above, polish in module):
--   * npc_vashjir_escape_seahorse: H drop-off point (currently the A ledge-camp
--     path end; H players swim ~300yd west to Nazgrim) - per-faction path end.
--   * npc_vashjir_battlemaiden / vision exits already faction-agnostic.
--   * 25963/25972 escort framing: retail-style controller ownership optional -
--     the native 78053/78051 H spell chain + shared wave spawns cover gameplay.
-- ============================================================================
