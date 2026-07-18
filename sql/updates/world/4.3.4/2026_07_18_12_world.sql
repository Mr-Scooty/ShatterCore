-- ============================================================================
-- SHIMMERING EXPANSE — BATCH A (Arcs S1 Legion's Rest / S2 Tranquil Wash / S4 Silver Tide Hollow+ruins)
-- TrinityCore 4.3.4 fork (ShatterCore). Idempotent: safe to re-apply.
-- Creature guids used: 9001200-9001252. GO guids used: none.
-- Custom PhaseIds used: none (retail-native 170/171/172/194 via aura-261 spells only).
-- All spells verified present in 4.3.4 Spell.dbc; effect data verified in SpellEffect.dbc.
-- ============================================================================

-- ############################################################################
-- SECTION 0 — PHASE FOUNDATION (overlay semantics)
-- The fork's master-style phasing drops the Unphased flag once a player holds
-- any aura phase (74848=171 etc.), hiding all PhaseId-169 base spawns. These
-- unconditional phase_area rows keep base phase 169 in the player's shift in
-- the two event areas, so aura phases 170/171/172/194 OVERLAY the base scene
-- (retail semantics). Phase 169 = DEFAULT_PHASE (Normal flag in Phase.dbc).
-- ############################################################################
DELETE FROM `phase_area` WHERE `AreaId` IN (5006,5047,5145) AND `PhaseId`=169;
INSERT INTO `phase_area` (`AreaId`,`PhaseId`,`Comment`) VALUES
(5006,169,'Legion''s Rest - keep base phase visible during intro-cave event phases 170/171/172'),
(5047,169,'Abyssal Breach overlook - keep base phase visible during Spirit Vision phase 194'),
(5145,169,'Abyssal Breach (parent) - keep base phase visible during Spirit Vision phase 194');

-- Suppress the retail player-aura naga wave engine (74848 E1 aura23 10s -> 74845
-- aura23 2s -> 74843 summon 40162 at TARGET_DEST_NEARBY_ENTRY). The controller
-- bunny 40163 drives waves instead (deterministic inlet coords). Disabling
-- 74845 breaks the chain at its root; Spell::prepare checks disables even for
-- aura-triggered casts.
DELETE FROM `disables` WHERE `sourceType`=0 AND `entry`=74845;
INSERT INTO `disables` (`sourceType`,`entry`,`flags`,`params_0`,`params_1`,`comment`) VALUES
(0,74845,1,'','','Shimmering S1: suppress native 74848 wave-engine aura chain; controller 40163 summons waves');

-- ############################################################################
-- ARC S1 — LEGION'S REST (25471 -> 25334 -> 25164 -> 25221 -> 25222)
-- ############################################################################

-- ---------------------------------------------------------------------------
-- S1.1 Quest chain / flags / StartItem fixes
-- ---------------------------------------------------------------------------
-- Chain arrows per sniff: 25471 -> 25334 (already Prev) -> 25164 -> 25221 -> 25222 (already Prev).
UPDATE `quest_template_addon` SET `PrevQuestID`=25334 WHERE `ID`=25164;
UPDATE `quest_template_addon` SET `PrevQuestID`=25164, `ProvidedItemCount`=0 WHERE `ID`=25221;
-- 25221 StartItem 54466: no item involved in any sniffed credit; null it so accept validation cannot break.
UPDATE `quest_template` SET `StartItem`=0 WHERE `ID`=25221;
-- Phase flips depend on quest state (74850 spell_area gate on 25334; 74848/74849 event auras).
UPDATE `quest_template` SET `Flags`=`Flags`|0x400000 WHERE `ID` IN (25334,25164);

-- ---------------------------------------------------------------------------
-- S1.2 spell_area: phase auras bound to the cave (leak protection + pre-event autocast)
-- 74850 = phase 170 pre-vision cave: autocast in cave until 25334 rewarded, autoremove on leave
-- (also strips automatically when the vision teleports the player to area 5047).
-- 74848 (171) / 74849 (172): event-granted, autoremove-only so leaving the cave un-sticks the event.
-- ---------------------------------------------------------------------------
DELETE FROM `spell_area` WHERE `spell` IN (74850,74848,74849) AND `area`=5006;
INSERT INTO `spell_area` (`spell`,`area`,`quest_start`,`quest_end`,`aura_spell`,`racemask`,`gender`,`flags`,`quest_start_status`,`quest_end_status`) VALUES
(74850,5006,0,25334,0,0,2,3,64,11),  -- autocast+autoremove; drops once 25334 rewarded (status 64 not in mask 11)
(74848,5006,0,    0,0,0,2,2,64,11),  -- autoremove only (defense phase 171)
(74849,5006,0,    0,0,0,2,2,64,11);  -- autoremove only (showdown phase 172)

-- ---------------------------------------------------------------------------
-- S1.3 Spell glue (native DBC chains)
-- 74386 Spirit Trance (stun) expire -> player casts 74385 Spirit Vision (teleport+phase 194). Retail delay 5.1s = trance duration.
-- 75312 Tsunami Knockback hit -> hit player casts 75324 (removes 74849 -> leaves phase 172). E1 kill-credit 40161 is DBC-native.
-- ---------------------------------------------------------------------------
DELETE FROM `spell_linked_spell` WHERE (`spell_trigger`=-74386 AND `spell_effect`=74385) OR (`spell_trigger`=75312 AND `spell_effect`=75324);
INSERT INTO `spell_linked_spell` (`spell_trigger`,`spell_effect`,`type`,`comment`) VALUES
(-74386,74385,2,'Spirit Trance expire - cast Spirit Vision (teleport to Abyssal Breach overlook, phase 194)'),
(75312,75324,1,'Tsunami Knockback hit - Targeted Remove from Phase 3 (strips 74849)');

-- 75312 implicit targets (18/8 dest-area-entry) need spell_implicit_target conditions: hit players only.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry`=75312;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(13,7,75312,0,0,31,0,4,0,0,0,0,0,'','Tsunami Knockback (75312) hits players');

-- ---------------------------------------------------------------------------
-- S1.4 Gossip gate: menu 11608 option 0 (Gadra "join you in the vision") only while 25334 taken+incomplete
-- ---------------------------------------------------------------------------
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=15 AND `SourceGroup`=11608 AND `SourceEntry`=0;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(15,11608,0,0,0,9,0,25334,0,0,0,0,0,'','Gadra vision gossip only while The Looming Threat (25334) in progress');

-- ---------------------------------------------------------------------------
-- S1.5 creature_text imports (WPP 14-38-52 dump; UNKNOWN -> SoundType 0)
-- ---------------------------------------------------------------------------
DELETE FROM `creature_text` WHERE (`CreatureID`=39226 AND `GroupID`=0) OR (`CreatureID`=39877 AND `GroupID`=0) OR (`CreatureID`=40161 AND `GroupID` IN (0,1,2)) OR (`CreatureID`=40398 AND `GroupID` IN (0,1,2,3));
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(39226,0,0,'Calm yaself. Let yaself get lost in tha flames.',12,0,100,0,0,0,0,0,0,'Farseer Gadra - 25334 accept'),
(39877,0,0,'The naga escaped out of the tunnel ahead. Show them no mercy, $c.',12,0,100,0,0,0,0,0,0,'Toshe Chaosrender - 25221 accept'),
(40161,0,0,'Before the day is done, you will all be slaves and corpses!',14,0,100,0,0,21857,0,0,0,'Fathom-Lord Zin''jatar - aggro yell'),
(40161,1,0,'Fathom-Lord Zin''jatar has entered the battle!',41,0,100,0,0,21857,0,0,0,'Fathom-Lord Zin''jatar - entered battle boss emote'),
(40161,2,0,'Enough! Have your cave, little shaman.',14,0,100,0,0,21858,0,0,0,'Fathom-Lord Zin''jatar - concession at 10%'),
(40398,0,0,'Dis breach leads to tha plane of water. Tha realm of Neptulon, tha Tidehunter.',12,0,100,0,0,0,0,0,0,'Spirit of Farseer Gadra - vision RP 1'),
(40398,1,0,'Tha naga and de Twilight Cult be workin'' together, and dis be the focus of dey''re efforts.',12,0,100,0,0,0,0,0,0,'Spirit of Farseer Gadra - vision RP 2'),
(40398,2,0,'Disturbin'' prospects. And there be little we can be doin'' about it witout more men.',12,0,100,0,0,0,0,0,0,'Spirit of Farseer Gadra - vision RP 3'),
(40398,3,0,'Let me know when ya be ready ta leave.',12,0,100,0,0,0,0,0,0,'Spirit of Farseer Gadra - vision RP 4 (after credit)');

-- ---------------------------------------------------------------------------
-- S1.6 SmartAI enablement
-- ---------------------------------------------------------------------------
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry` IN (39226,39877,40161,40163,40174,40375,40398) AND `AIName`='';

-- ---------------------------------------------------------------------------
-- S1.7 SAI — 39226 Farseer Gadra
-- Reward-side phase flip for 25334 is NATIVE: quest RewardSpell 74858
-- (E0 forcecast 74848 / E1 remove 74385 / E2 remove 74849) — no SAI needed there.
-- ---------------------------------------------------------------------------
DELETE FROM `smart_scripts` WHERE `entryorguid`=39226 AND `source_type`=0 AND `id` IN (0,1,2,3);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(39226,0,0,0,19,0,100,0,25334,1000,1000,0,0,1,0,0,0,0,0,0,7,0,0,0,0,0,0,0,'Farseer Gadra - on accept The Looming Threat - say Calm yaself'),
(39226,0,1,2,62,0,100,0,11608,0,0,0,0,72,0,0,0,0,0,0,7,0,0,0,0,0,0,0,'Farseer Gadra - gossip vision option - close gossip'),
(39226,0,2,0,61,0,100,0,0,0,0,0,0,11,78720,2,0,0,0,0,7,0,0,0,0,0,0,0,'Farseer Gadra - linked - cast Forcecast Spirit Trance on player (74386 -> linked 74385 vision)'),
(39226,0,3,0,19,0,100,0,25164,1000,1000,0,0,45,1,1,0,0,0,0,19,40163,150,0,0,0,0,0,'Farseer Gadra - on accept Backed Into a Corner - set data 1 1 on Defense Controller');

-- ---------------------------------------------------------------------------
-- S1.8 SAI — 40398 Spirit of Farseer Gadra (summoned by player via 75482 in phase 194)
-- 75479 = native kill-credit 40307 (tgt summoner). 74391 = teleport back (spell_target_position present).
-- ---------------------------------------------------------------------------
DELETE FROM `smart_scripts` WHERE `entryorguid`=40398 AND `source_type`=0 AND `id` IN (0,1,2);
DELETE FROM `smart_scripts` WHERE `entryorguid`=4039800 AND `source_type`=9 AND `id` IN (0,1,2,3,4);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(40398,0,0,0,54,0,100,0,0,0,0,0,0,80,4039800,0,2,0,0,0,1,0,0,0,0,0,0,0,'Spirit of Gadra - just summoned - run vision RP actionlist'),
(40398,0,1,2,20,0,100,0,25334,1000,1000,0,0,11,74391,2,0,0,0,0,7,0,0,0,0,0,0,0,'Spirit of Gadra - 25334 rewarded - cast Spirit Vision return teleport on player (phases via quest RewardSpell 74858)'),
(40398,0,2,0,61,0,100,0,0,0,0,0,0,41,3000,0,0,0,0,0,1,0,0,0,0,0,0,0,'Spirit of Gadra - linked - despawn'),
(4039800,9,0,0,0,0,100,0,8900,8900,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Spirit RP - +8.9s - say Dis breach leads to tha plane of water'),
(4039800,9,1,0,0,0,100,0,10500,10500,0,0,0,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,'Spirit RP - +19.4s - say Tha naga and de Twilight Cult'),
(4039800,9,2,0,0,0,100,0,8000,8000,0,0,0,1,2,0,0,0,0,0,1,0,0,0,0,0,0,0,'Spirit RP - +27.4s - say Disturbin prospects'),
(4039800,9,3,0,0,0,100,0,8000,8000,0,0,0,11,75479,2,0,0,0,0,23,0,0,0,0,0,0,0,'Spirit RP - +35.4s - cast Spirit Vision Kill Credit (40307) on summoner'),
(4039800,9,4,0,0,0,100,0,200,200,0,0,0,1,3,0,0,0,0,0,1,0,0,0,0,0,0,0,'Spirit RP - say Let me know when ya be ready ta leave');

-- ---------------------------------------------------------------------------
-- S1.9 SAI — 40163 Intro Cave Defense Controller Bunny (waves + 3-minute timer)
-- Waves: 2-3 naga per ~10s cycling the 3 inlets above the cave mouth.
-- 74857 (cast on nearby players at 186s) = native: E0 kill-credit 40163,
-- E1 forcecast 75334, E2 forcecast 74849 (phase 172, removes 74848).
-- ---------------------------------------------------------------------------
DELETE FROM `smart_scripts` WHERE `entryorguid`=40163 AND `source_type`=0 AND `id` IN (0,1,2,3,4,5);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(40163,0,0,0,38,0,100,0,1,1,0,0,0,22,1,0,0,0,0,0,1,0,0,0,0,0,0,0,'Defense Controller - data 1 1 (25164 accepted) - enter event phase 1'),
(40163,0,1,0,1,1,100,0,4000,6000,9000,12000,0,12,40162,1,60000,0,0,0,8,0,0,0,-5151.2,4001.2,-27.0,4.71,'Defense Controller - phase 1 - summon Fathom-Stalker wave at inlet A'),
(40163,0,2,0,1,1,100,0,7000,9000,10000,13000,0,12,40372,1,60000,0,0,0,8,0,0,0,-5184.1,4000.2,-27.0,4.71,'Defense Controller - phase 1 - summon Fathom-Stalker wave at inlet B'),
(40163,0,3,0,1,1,100,0,10000,12000,11000,14000,0,12,39397,1,60000,0,0,0,8,0,0,0,-5208.0,4000.0,-27.0,4.71,'Defense Controller - phase 1 - summon Fathom-Stalker wave at inlet C'),
(40163,0,4,5,1,1,100,0,186000,186000,186000,186000,0,11,74857,2,0,0,0,0,18,100,0,0,0,0,0,0,'Defense Controller - phase 1 - 3m06s - cast Forcecast Phase Shift 3 on players (credit 40163 + phase 172)'),
(40163,0,5,0,61,0,100,0,0,0,0,0,0,22,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Defense Controller - linked - stop waves (event phase 0)');

-- ---------------------------------------------------------------------------
-- S1.10 SAI — wave naga 40162/40372/39397 (append after existing ids 0-1)
-- Dive from the inlet to the cave floor, then attack nearest enemy (tunnel Toshe/shamans/players).
-- No world spawns exist for these entries: evade-despawn is safe.
-- ---------------------------------------------------------------------------
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (40162,40372,39397) AND `source_type`=0 AND `id` IN (2,3,4);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(40162,0,2,0,54,0,100,0,0,0,0,0,0,69,7,0,0,0,0,0,8,0,0,0,-5185.0,3976.0,-14.5,0,'Zin''jatar Fathom-Stalker - just summoned - dive to cave floor'),
(40162,0,3,0,34,0,100,0,8,7,0,0,0,49,0,0,0,0,0,0,25,50,0,0,0,0,0,0,'Zin''jatar Fathom-Stalker - reached floor - attack closest enemy'),
(40162,0,4,0,7,0,100,0,0,0,0,0,0,41,3000,0,0,0,0,0,1,0,0,0,0,0,0,0,'Zin''jatar Fathom-Stalker - evade - despawn'),
(40372,0,2,0,54,0,100,0,0,0,0,0,0,69,7,0,0,0,0,0,8,0,0,0,-5185.0,3976.0,-14.5,0,'Zin''jatar Fathom-Stalker - just summoned - dive to cave floor'),
(40372,0,3,0,34,0,100,0,8,7,0,0,0,49,0,0,0,0,0,0,25,50,0,0,0,0,0,0,'Zin''jatar Fathom-Stalker - reached floor - attack closest enemy'),
(40372,0,4,0,7,0,100,0,0,0,0,0,0,41,3000,0,0,0,0,0,1,0,0,0,0,0,0,0,'Zin''jatar Fathom-Stalker - evade - despawn'),
(39397,0,2,0,54,0,100,0,0,0,0,0,0,69,7,0,0,0,0,0,8,0,0,0,-5185.0,3976.0,-14.5,0,'Zin''jatar Fathom-Stalker - just summoned - dive to cave floor'),
(39397,0,3,0,34,0,100,0,8,7,0,0,0,49,0,0,0,0,0,0,25,50,0,0,0,0,0,0,'Zin''jatar Fathom-Stalker - reached floor - attack closest enemy'),
(39397,0,4,0,7,0,100,0,0,0,0,0,0,41,3000,0,0,0,0,0,1,0,0,0,0,0,0,0,'Zin''jatar Fathom-Stalker - evade - despawn');

-- ---------------------------------------------------------------------------
-- S1.11 SAI — 40161 Fathom-Lord Zin'jatar (showdown, phase 172)
-- At 10%: 76187 (99% damage reduction), concession yell, immune, retreat to
-- tunnel, then Tsunami Bunnies cast 75312 (native credit 40161 + knockback +
-- linked 75324 phase-out), boss despawns (respawns in 120s for the next group).
-- ---------------------------------------------------------------------------
DELETE FROM `smart_scripts` WHERE `entryorguid`=40161 AND `source_type`=0 AND `id` IN (0,1,2,3);
DELETE FROM `smart_scripts` WHERE `entryorguid`=4016100 AND `source_type`=9 AND `id` IN (0,1,2,3,4,5,6,7);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(40161,0,0,1,4,0,100,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Fathom-Lord Zin''jatar - aggro - yell slaves and corpses'),
(40161,0,1,0,61,0,100,0,0,0,0,0,0,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,'Fathom-Lord Zin''jatar - linked - entered battle boss emote'),
(40161,0,2,0,0,0,100,0,6000,9000,9000,13000,0,11,70452,0,0,0,0,0,2,0,0,0,0,0,0,0,'Fathom-Lord Zin''jatar - IC - cast Frost Cast'),
(40161,0,3,0,2,0,100,1,0,10,0,0,0,80,4016100,0,2,0,0,0,1,0,0,0,0,0,0,0,'Fathom-Lord Zin''jatar - at 10% HP - run concession actionlist'),
(4016100,9,0,0,0,0,100,0,0,0,0,0,0,11,76187,2,0,0,0,0,1,0,0,0,0,0,0,0,'Zin''jatar concession - cast 99% Damage Reduction'),
(4016100,9,1,0,0,0,100,0,0,0,0,0,0,1,2,0,0,0,0,0,1,0,0,0,0,0,0,0,'Zin''jatar concession - yell Enough! Have your cave'),
(4016100,9,2,0,0,0,100,0,0,0,0,0,0,27,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Zin''jatar concession - combat stop'),
(4016100,9,3,0,0,0,100,0,0,0,0,0,0,18,770,0,0,0,0,0,1,0,0,0,0,0,0,0,'Zin''jatar concession - set non-attackable + immune'),
(4016100,9,4,0,0,0,100,0,200,200,0,0,0,59,1,0,0,0,0,0,1,0,0,0,0,0,0,0,'Zin''jatar concession - set run'),
(4016100,9,5,0,0,0,100,0,0,0,0,0,0,69,8,0,0,0,0,0,8,0,0,0,-5159.27,3990.04,-14.7,1.5,'Zin''jatar concession - retreat to tunnel'),
(4016100,9,6,0,0,0,100,0,7800,7800,0,0,0,45,1,1,0,0,0,0,9,40375,0,100,0,0,0,0,'Zin''jatar concession - +8s - trigger Tsunami Bunnies (data 1 1)'),
(4016100,9,7,0,0,0,100,0,700,700,0,0,0,41,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Zin''jatar concession - despawn');

-- 40375 Tsunami Bunny: cast the knockback/credit on trigger
DELETE FROM `smart_scripts` WHERE `entryorguid`=40375 AND `source_type`=0 AND `id`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(40375,0,0,0,38,0,100,0,1,1,0,0,0,11,75312,2,0,0,0,0,1,0,0,0,0,0,0,0,'Tsunami Bunny - data 1 1 - cast Tsunami Knockback (credit 40161 + knockback + phase-out link)');

-- ---------------------------------------------------------------------------
-- S1.12 SAI — 39877 Toshe Chaosrender (25221 accept flavor)
-- ---------------------------------------------------------------------------
DELETE FROM `smart_scripts` WHERE `entryorguid`=39877 AND `source_type`=0 AND `id`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(39877,0,0,0,19,0,100,0,25221,1000,1000,0,0,1,0,0,0,0,0,0,7,0,0,0,0,0,0,0,'Toshe Chaosrender - on accept Rundown - say Show them no mercy');

-- ---------------------------------------------------------------------------
-- S1.13 SAI — 40174 Fleeing Zin'jatar Fathom-Stalker (wounded, passive, running)
-- 53034 Set Health Random is a DUMMY effect (needs core script — flagged);
-- the wounded feel is delivered via HealthModifier 0.3 below.
-- ---------------------------------------------------------------------------
DELETE FROM `smart_scripts` WHERE `entryorguid`=40174 AND `source_type`=0 AND `id` IN (0,1,2);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(40174,0,0,1,25,0,100,0,0,0,0,0,0,11,53034,2,0,0,0,0,1,0,0,0,0,0,0,0,'Fleeing Fathom-Stalker - on reset - cast Set Health Random'),
(40174,0,1,2,61,0,100,0,0,0,0,0,0,8,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Fleeing Fathom-Stalker - linked - react passive'),
(40174,0,2,0,61,0,100,0,0,0,0,0,0,59,1,0,0,0,0,0,1,0,0,0,0,0,0,0,'Fleeing Fathom-Stalker - linked - set run');

-- Wounded stat + spawn tuning; prune 608 WPP dup spawns to ~100 spread along the corridor (keep guid%6=0, stable/idempotent)
UPDATE `creature_template` SET `HealthModifier`=0.3 WHERE `entry`=40174;
DELETE FROM `creature` WHERE `id`=40174 AND `guid` % 6 <> 0;
UPDATE `creature` SET `spawntimesecs`=45, `MovementType`=1, `wander_distance`=10 WHERE `id`=40174;

-- ---------------------------------------------------------------------------
-- S1.14 Spawn work — Legion's Rest event actors
-- Re-phase 341997 (cave-back Toshe) out of base into the phase-172 showdown:
-- fixes the double-Toshe bug (342021 tunnel Toshe stays base/169 as Rundown giver).
-- ---------------------------------------------------------------------------
UPDATE `creature` SET `PhaseId`=172 WHERE `guid`=341997;

DELETE FROM `creature` WHERE `guid` BETWEEN 9001200 AND 9001252;
INSERT INTO `creature` (`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseUseFlags`,`phaseMask`,`PhaseId`,`PhaseGroup`,`terrainSwapMap`,`modelid`,`equipment_id`,`position_x`,`position_y`,`position_z`,`orientation`,`spawntimesecs`,`wander_distance`,`currentwaypoint`,`curhealth`,`curmana`,`MovementType`,`npcflag`,`unit_flags`,`dynamicflags`,`ScriptName`,`VerifiedBuild`) VALUES
-- Spirit Vision tableau (phase 194, Abyssal Breach overlook, static scenery)
(9001200,40315,0,5145,5047,1,0,1,194,0,-1,0,0,-5930.06,5221.43,-1207.0,0.84,300,0,0,1,0,0,0,0,0,'',0),
(9001201,40315,0,5145,5047,1,0,1,194,0,-1,0,0,-5927.87,5225.93,-1206.0,0.80,300,0,0,1,0,0,0,0,0,'',0),
(9001202,40315,0,5145,5047,1,0,1,194,0,-1,0,0,-5934.56,5223.62,-1206.0,0.73,300,0,0,1,0,0,0,0,0,'',0),
(9001203,40315,0,5145,5047,1,0,1,194,0,-1,0,0,-5862.43,5316.30,-1202.0,4.18,300,0,0,1,0,0,0,0,0,'',0),
(9001204,40317,0,5145,5047,1,0,1,194,0,-1,0,0,-5894.56,5261.32,-1171.4,3.08,300,0,0,1,0,0,0,0,0,'',0),
(9001205,41652,0,5145,5047,1,0,1,194,0,-1,0,0,-5894.57,5256.15,-1168.5,2.88,300,0,0,1,0,0,0,0,0,'',0),
(9001206,41652,0,5145,5047,1,0,1,194,0,-1,0,0,-5896.58,5265.84,-1169.2,3.26,300,0,0,1,0,0,0,0,0,'',0),
(9001207,40318,0,5145,5047,1,0,1,194,0,-1,0,0,-5726.40,5275.10,-1240.3,2.90,300,0,0,1,0,0,0,0,0,'',0),
(9001208,40318,0,5145,5047,1,0,1,194,0,-1,0,0,-5731.90,5301.60,-1265.0,3.10,300,0,0,1,0,0,0,0,0,'',0),
(9001209,40318,0,5145,5047,1,0,1,194,0,-1,0,0,-5738.20,5322.40,-1287.6,3.30,300,0,0,1,0,0,0,0,0,'',0),
(9001210,40318,0,5145,5047,1,0,1,194,0,-1,0,0,-5744.70,5341.00,-1310.2,3.45,300,0,0,1,0,0,0,0,0,'',0),
(9001211,40318,0,5145,5047,1,0,1,194,0,-1,0,0,-5751.30,5357.80,-1334.8,3.60,300,0,0,1,0,0,0,0,0,'',0),
(9001212,40318,0,5145,5047,1,0,1,194,0,-1,0,0,-5757.60,5367.20,-1358.4,3.75,300,0,0,1,0,0,0,0,0,'',0),
(9001213,40318,0,5145,5047,1,0,1,194,0,-1,0,0,-5762.80,5330.50,-1382.1,3.20,300,0,0,1,0,0,0,0,0,'',0),
(9001214,40318,0,5145,5047,1,0,1,194,0,-1,0,0,-5766.10,5289.90,-1408.7,3.00,300,0,0,1,0,0,0,0,0,'',0),
(9001215,41227,0,5145,5047,1,0,1,194,0,-1,0,0,-5794.00,5290.00,-1196.0,3.142,300,0,0,1,0,0,0,0,0,'',0),
(9001216,41227,0,5145,5047,1,0,1,194,0,-1,0,0,-5797.71,5306.26,-1202.0,3.511,300,0,0,1,0,0,0,0,0,'',0),
(9001217,41227,0,5145,5047,1,0,1,194,0,-1,0,0,-5808.35,5320.32,-1203.9,3.881,300,0,0,1,0,0,0,0,0,'',0),
(9001218,41227,0,5145,5047,1,0,1,194,0,-1,0,0,-5824.48,5330.28,-1200.5,4.250,300,0,0,1,0,0,0,0,0,'',0),
(9001219,41227,0,5145,5047,1,0,1,194,0,-1,0,0,-5843.93,5334.81,-1194.0,4.620,300,0,0,1,0,0,0,0,0,'',0),
(9001220,41227,0,5145,5047,1,0,1,194,0,-1,0,0,-5864.05,5333.28,-1188.8,4.990,300,0,0,1,0,0,0,0,0,'',0),
(9001221,41227,0,5145,5047,1,0,1,194,0,-1,0,0,-5882.14,5325.91,-1188.6,5.359,300,0,0,1,0,0,0,0,0,'',0),
(9001222,41227,0,5145,5047,1,0,1,194,0,-1,0,0,-5895.76,5313.69,-1193.4,5.729,300,0,0,1,0,0,0,0,0,'',0),
(9001223,41227,0,5145,5047,1,0,1,194,0,-1,0,0,-5903.06,5298.27,-1200.0,6.098,300,0,0,1,0,0,0,0,0,'',0),
(9001224,41227,0,5145,5047,1,0,1,194,0,-1,0,0,-5903.06,5281.73,-1203.8,0.185,300,0,0,1,0,0,0,0,0,'',0),
(9001225,41227,0,5145,5047,1,0,1,194,0,-1,0,0,-5895.76,5266.31,-1202.4,0.554,300,0,0,1,0,0,0,0,0,'',0),
(9001226,41227,0,5145,5047,1,0,1,194,0,-1,0,0,-5882.14,5254.09,-1196.6,0.924,300,0,0,1,0,0,0,0,0,'',0),
(9001227,41227,0,5145,5047,1,0,1,194,0,-1,0,0,-5864.05,5246.72,-1190.4,1.294,300,0,0,1,0,0,0,0,0,'',0),
(9001228,41227,0,5145,5047,1,0,1,194,0,-1,0,0,-5843.93,5245.19,-1188.0,1.663,300,0,0,1,0,0,0,0,0,'',0),
(9001229,41227,0,5145,5047,1,0,1,194,0,-1,0,0,-5824.48,5249.72,-1191.1,2.033,300,0,0,1,0,0,0,0,0,'',0),
(9001230,41227,0,5145,5047,1,0,1,194,0,-1,0,0,-5808.35,5259.68,-1197.5,2.402,300,0,0,1,0,0,0,0,0,'',0),
(9001231,41227,0,5145,5047,1,0,1,194,0,-1,0,0,-5797.71,5273.74,-1202.9,2.772,300,0,0,1,0,0,0,0,0,'',0),
-- Legion's Rest defense event actors
(9001232,40163,0,5144,5006,1,0,1,169,0,-1,0,0,-5159.75,3989.76,-14.68,4.00,300,0,0,1,0,0,0,0,0,'',0),   -- Defense Controller (base phase: reachable by Gadra set-data; summons inherit 169)
(9001233,40161,0,5144,5006,1,0,1,172,0,-1,0,0,-5165.79,3975.01,-14.07,2.20,120,0,0,1,0,0,0,0,0,'',0),   -- Fathom-Lord Zin'jatar (showdown phase 172, 120s respawn)
(9001234,40375,0,5144,5006,1,0,1,172,0,-1,0,0,-5145.25,4000.94,-15.0,3.14,300,0,0,1,0,0,0,0,0,'',0),    -- Tsunami Bunny back wall 1
(9001235,40375,0,5144,5006,1,0,1,172,0,-1,0,0,-5145.07,3985.49,-15.0,3.14,300,0,0,1,0,0,0,0,0,'',0),    -- Tsunami Bunny back wall 2
(9001236,40375,0,5144,5006,1,0,1,172,0,-1,0,0,-5144.54,3972.47,-15.0,3.14,300,0,0,1,0,0,0,0,0,'',0),    -- Tsunami Bunny back wall 3
(9001237,40375,0,5144,5006,1,0,1,172,0,-1,0,0,-5144.30,3959.08,-15.0,3.14,300,0,0,1,0,0,0,0,0,'',0),    -- Tsunami Bunny back wall 4
(9001238,39389,0,5144,5006,1,0,1,169,0,-1,0,0,-5196.50,3978.00,-14.5,5.50,300,0,0,1,0,0,0,0,0,'',0),    -- Greater Earth Elemental guardian (base, fights waves)
(9001239,40831,0,5144,5006,1,0,1,169,0,-1,0,0,-5188.00,3966.50,-14.3,1.80,300,0,0,1,0,0,0,0,0,'',0),    -- Greater Fire Elemental guardian (base, fights waves)
-- ARC S2: Sambino hub ambience — Agitated Green Sand Crabs harassing the show
(9001240,40238,0,5144,4961,1,0,1,169,0,-1,0,0,-5859.70,4669.69,-512.97,5.50,300,0,0,1,0,0,0,0,0,'',0),
(9001241,40238,0,5144,4961,1,0,1,169,0,-1,0,0,-5863.56,4681.42,-512.95,2.50,300,0,0,1,0,0,0,0,0,'',0),
-- ARC S4: Silver Tide Hollow tent hub questgiver cast (phase 169)
(9001242,39881,0,5144,5005,1,0,1,169,0,-1,0,0,-6618.37,4282.48,-562.95,3.411,300,0,0,1,0,0,0,0,0,'',0), -- Wavespeaker Valoren
(9001243,40642,0,5144,5005,1,0,1,169,0,-1,0,0,-6598.71,4296.96,-562.70,0.401,300,0,0,1,0,0,0,0,0,'',0), -- Captain Taylor
(9001244,40643,0,5144,5005,1,0,1,169,0,-1,0,0,-6597.53,4296.34,-562.63,1.134,300,0,0,1,0,0,0,0,0,'',0), -- Admiral Dvorek
(9001245,40644,0,5144,5005,1,0,1,169,0,-1,0,0,-6604.19,4271.22,-562.35,3.089,300,0,0,1,0,0,0,0,0,'',0), -- Levia Dreamwaker
(9001246,40645,0,5144,5005,1,0,1,169,0,-1,0,0,-6609.81,4279.44,-562.72,3.482,300,0,0,1,0,0,0,0,0,'',0), -- Jorlan Trueblade
(9001247,41869,0,5144,5005,1,0,1,169,0,-1,0,0,-6612.51,4282.33,-563.04,0.30,300,0,0,1,0,0,0,0,0,'',0),  -- Erunak Stonespeaker (tent)
(9001248,41219,0,5144,5005,1,0,1,169,0,-1,0,0,-6606.34,4271.25,-562.23,1.50,300,0,0,1,0,0,0,0,0,'',0),  -- Nespirah Survivor (flavor)
(9001249,40920,0,5144,5005,1,0,1,169,0,-1,0,0,-6602.50,4269.00,-562.30,3.00,300,0,0,1,0,0,0,0,0,'',0),  -- Elendri Goldenbrow (H ender 27717; near Levia, pending H-sniff verify)
-- ARC S4: 25582 A Better Vantage proximity-credit triggers
(9001250,40963,0,5144,0,1,0,1,169,0,-1,0,0,-6658.40,4788.20,-595.10,0,300,0,0,1,0,0,0,0,0,'',0),        -- Northern Quel'Dormir Gardens scouted
(9001251,40964,0,5144,0,1,0,1,169,0,-1,0,0,-6788.90,4943.50,-553.60,0,300,0,0,1,0,0,0,0,0,'',0),        -- Tunnel west of Quel'Dormir Gardens scouted
(9001252,40965,0,5144,0,1,0,1,169,0,-1,0,0,-7167.10,4722.60,-566.20,0,300,0,0,1,0,0,0,0,0,'',0);        -- Structures south of Quel'Dormir Gardens scouted

-- ############################################################################
-- ARC S2 — TRANQUIL WASH + FATHOM-LORD (25215-25220, 25359/25360, 25217/25456, 25439-25442)
-- ############################################################################

-- ---------------------------------------------------------------------------
-- S2.1 25215 A Distracting Scent — spellclick corpse-drag
-- ---------------------------------------------------------------------------
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` IN (39911,39422);
INSERT INTO `npc_spellclick_spells` (`npc_entry`,`spell_id`,`cast_flags`,`user_type`) VALUES
(39911,76193,1,0),  -- Dead Zin'jatar Raider -> Summon Personal Dead Naga (40847)
(39422,86527,1,0);  -- Coilshell Sifter -> Loot Coilshell Sifter Ping (25219)
UPDATE `creature_template` SET `npcflag`=`npcflag`|16777216 WHERE `entry` IN (39911,39422);

-- See Quest Invis 1 (75577): reveal the corpses while on 25215 across the drag corridor; strip on leave/reward.
DELETE FROM `spell_area` WHERE `spell`=75577 AND `area` IN (4969,4963,4961);
INSERT INTO `spell_area` (`spell`,`area`,`quest_start`,`quest_end`,`aura_spell`,`racemask`,`gender`,`flags`,`quest_start_status`,`quest_end_status`) VALUES
(75577,4969,25215,0,0,0,2,3,10,11),
(75577,4963,25215,0,0,0,2,3,10,11),
(75577,4961,25215,0,0,0,2,3,10,11);
UPDATE `quest_template` SET `Flags`=`Flags`|0x400000 WHERE `ID`=25215;

-- 40847 Personal Dead Naga: follow the dragger; over Glimmerdeep Gorge (area 4963) credit + sink.
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=40847 AND `AIName`='';
DELETE FROM `smart_scripts` WHERE `entryorguid`=40847 AND `source_type`=0 AND `id` IN (0,1,2);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(40847,0,0,0,54,0,100,0,0,0,0,0,0,29,2,0,0,0,0,0,23,0,0,0,0,0,0,0,'Personal Dead Naga - just summoned - follow summoner'),
(40847,0,1,2,1,0,100,0,3000,3000,2000,2000,0,11,75522,2,0,0,0,0,1,0,0,0,0,0,0,0,'Personal Dead Naga - over Glimmerdeep Gorge - cast Dead Naga Kill Credit (39911 to summoner)'),
(40847,0,2,0,61,0,100,0,0,0,0,0,0,41,2500,0,0,0,0,0,1,0,0,0,0,0,0,0,'Personal Dead Naga - linked - sink/despawn');
-- Area gate for the credit tick (SAI id 1 -> SourceGroup id+1=2; ConditionTarget 1 = the corpse itself)
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=22 AND `SourceEntry`=40847;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(22,2,40847,0,0,23,1,4963,0,0,0,0,0,'','Personal Dead Naga credits only over Glimmerdeep Gorge (area 4963)');

-- ---------------------------------------------------------------------------
-- S2.2 25219 Don't be Shellfish — Coilshell Sifter response
-- ---------------------------------------------------------------------------
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=39422 AND `AIName`='';
DELETE FROM `smart_scripts` WHERE `entryorguid`=39422 AND `source_type`=0 AND `id` IN (0,1);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(39422,0,0,1,8,0,100,0,86527,0,1000,1000,0,11,74489,2,0,0,0,0,7,0,0,0,0,0,0,0,'Coilshell Sifter - hit by Loot Ping - create Coilshell Snail Meat (52975) for clicker'),
(39422,0,1,0,61,0,100,0,0,0,0,0,0,41,3000,0,0,0,0,0,1,0,0,0,0,0,0,0,'Coilshell Sifter - linked - despawn');
UPDATE `creature` SET `spawntimesecs`=60 WHERE `id`=39422;

-- ---------------------------------------------------------------------------
-- S2.3 25218 Undersea Inflation — balloon summon/fill/credit/despawn
-- 39418 already carries 54611 at 50% (WPP import) -> mark QuestRequired so it only drops on 25218.
-- Quest RewardSpell 75581 despawns the balloon natively (SAI receives the hit).
-- ---------------------------------------------------------------------------
UPDATE `creature_loot_template` SET `QuestRequired`=1 WHERE `Entry`=39418 AND `Item`=54611;
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry` IN (39882,40399) AND `AIName`='';
DELETE FROM `smart_scripts` WHERE `entryorguid`=39882 AND `source_type`=0 AND `id`=0;
DELETE FROM `smart_scripts` WHERE `entryorguid`=40399 AND `source_type`=0 AND `id` IN (0,1,2,3);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(39882,0,0,0,19,0,100,0,25218,1000,1000,0,0,134,75345,2,0,0,0,0,7,0,0,0,0,0,0,0,'Sambino - on accept Undersea Inflation - invoker summons Sambino''s Air Balloon'),
(40399,0,0,0,54,0,100,0,0,0,0,0,0,29,0,0,0,0,0,0,23,0,0,0,0,0,0,0,'Sambino''s Air Balloon - just summoned - follow owner'),
(40399,0,1,2,8,0,100,0,75346,0,2000,2000,0,11,86533,2,0,0,0,0,23,0,0,0,0,0,0,0,'Sambino''s Air Balloon - hit by Inflate - cast Inflated Air Balloon on owner'),
(40399,0,2,0,61,0,100,0,0,0,0,0,0,11,75386,2,0,0,0,0,23,0,0,0,0,0,0,0,'Sambino''s Air Balloon - linked - cast Balloon Kill Credit (40399) on owner'),
(40399,0,3,0,8,0,100,0,75581,0,1000,1000,0,41,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Sambino''s Air Balloon - hit by Despawn Air Balloon (25218 RewardSpell) - despawn');
-- Implicit-target conditions so 75346 (e1 mask 2) and 75581 hit the balloon
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry` IN (75346,75581);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(13,2,75346,0,0,31,0,3,40399,0,0,0,0,'','Inflate Air Balloon e1 hits Sambino''s Air Balloon'),
(13,1,75581,0,0,31,0,3,40399,0,0,0,0,'','Despawn Air Balloon hits Sambino''s Air Balloon');

-- ---------------------------------------------------------------------------
-- S2.4 25217 Totem Modification — StartItem + totem event
-- ---------------------------------------------------------------------------
UPDATE `quest_template` SET `StartItem`=53052 WHERE `ID`=25217;
DELETE FROM `creature_text` WHERE `CreatureID`=40233 AND `GroupID`=0;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(40233,0,0,'Sampling Successful!',16,0,100,0,0,0,0,0,0,'Sambino''s Modified Earthbind Totem - sampling done');
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=40233 AND `AIName`='';
DELETE FROM `smart_scripts` WHERE `entryorguid`=40233 AND `source_type`=0 AND `id`=0;
DELETE FROM `smart_scripts` WHERE `entryorguid`=4023300 AND `source_type`=9 AND `id` IN (0,1,2,3,4,5);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(40233,0,0,0,54,0,100,0,0,0,0,0,0,80,4023300,0,2,0,0,0,1,0,0,0,0,0,0,0,'Modified Earthbind Totem - just summoned - run sampling timeline'),
(4023300,9,0,0,0,0,100,0,5000,5000,0,0,0,12,40238,1,90000,0,0,0,1,0,0,0,0,0,0,0,'Totem timeline - +5s - summon Agitated Green Sand Crab'),
(4023300,9,1,0,0,0,100,0,10000,10000,0,0,0,12,40238,1,90000,0,0,0,1,0,0,0,0,0,0,0,'Totem timeline - +15s - summon second Agitated Green Sand Crab'),
(4023300,9,2,0,0,0,100,0,15000,15000,0,0,0,12,40239,1,90000,0,0,0,1,0,0,0,0,0,0,0,'Totem timeline - +30s - summon Enormous Sand Crab'),
(4023300,9,3,0,0,0,100,0,29000,29000,0,0,0,11,75010,2,0,0,0,0,1,0,0,0,0,0,0,0,'Totem timeline - +59s - cast Totem Kill Credit (40233 to master+party)'),
(4023300,9,4,0,0,0,100,0,200,200,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Totem timeline - emote Sampling Successful!'),
(4023300,9,5,0,0,0,100,0,4800,4800,0,0,0,41,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Totem timeline - despawn');
-- 40238: attack the totem when event-summoned; hub crabs harass Sambino's show OOC (append after existing id 0)
DELETE FROM `smart_scripts` WHERE `entryorguid`=40238 AND `source_type`=0 AND `id` IN (1,2,3);
DELETE FROM `smart_scripts` WHERE `entryorguid`=40239 AND `source_type`=0 AND `id`=2;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(40238,0,1,0,54,0,100,0,0,0,0,0,0,49,0,0,0,0,0,0,19,40233,15,0,0,0,0,0,'Agitated Green Sand Crab - event-summoned - attack the totem'),
(40238,0,2,0,1,0,100,0,8000,15000,12000,25000,0,11,79176,0,0,0,0,0,19,40230,15,0,0,0,0,0,'Agitated Green Sand Crab - OOC - Slap & Chop at Modified Earth Elemental (hub show)'),
(40238,0,3,0,1,0,100,0,20000,30000,20000,35000,0,11,79176,0,0,0,0,0,19,40227,20,0,0,0,0,0,'Agitated Green Sand Crab - OOC - Slap & Chop at Felice (hub show)'),
(40239,0,2,0,54,0,100,0,0,0,0,0,0,49,0,0,0,0,0,0,19,40233,15,0,0,0,0,0,'Enormous Sand Crab - event-summoned - attack the totem');

-- ---------------------------------------------------------------------------
-- S2.5 25441 Vortex — whirlpool vehicle
-- 75564 (item) -> missile 75112 -> summon 40277 at dest (props 496) — DBC-native.
-- 75104 self-aura pulses 75109 every 1s (rider trapping = C++ SpellScript, flagged in report).
-- ---------------------------------------------------------------------------
UPDATE `creature_template` SET `VehicleId`=735, `AIName`='SmartAI' WHERE `entry`=40277;
DELETE FROM `smart_scripts` WHERE `entryorguid`=40277 AND `source_type`=0 AND `id` IN (0,1,2);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(40277,0,0,1,54,0,100,0,0,0,0,0,0,11,75104,2,0,0,0,0,1,0,0,0,0,0,0,0,'Toshe''s Vortex - just summoned - apply Toshe''s Vortex pulse aura'),
(40277,0,1,2,61,0,100,0,0,0,0,0,0,89,8,0,0,0,0,0,1,0,0,0,0,0,0,0,'Toshe''s Vortex - linked - roam short random splines'),
(40277,0,2,0,61,0,100,0,0,0,0,0,0,41,26000,0,0,0,0,0,1,0,0,0,0,0,0,0,'Toshe''s Vortex - linked - despawn after ~26s');
-- 75109 pulse should pick Swarming Serpents (C++ handles ride/credit; conditions narrow the dummy's area targets)
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry`=75109;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(13,1,75109,0,0,31,0,3,40280,0,0,0,0,'','Toshe''s Vortex Trigger hits Swarming Serpents');

-- ---------------------------------------------------------------------------
-- S2.6 25440 Fathom-Lord Zin'jatar (weakened quest boss 40510)
-- Weakened = spawns at 25% strength: template HealthModifier 15 -> 3.75 (idempotent absolute;
-- retail-4.3.4 HP tuning from user pending). Addon aura 75573 already present.
-- ---------------------------------------------------------------------------
UPDATE `creature_template` SET `HealthModifier`=3.75, `AIName`='SmartAI' WHERE `entry`=40510;
UPDATE `creature` SET `spawntimesecs`=60 WHERE `guid`=343486;
DELETE FROM `creature_text` WHERE `CreatureID`=40510 AND `GroupID`=0;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(40510,0,0,'Ah, so you''ve left your hole?  The Lady will claim you soon enough.',12,0,100,0,0,21855,0,0,0,'Fathom-Lord Zin''jatar (weakened) - aggro');
DELETE FROM `smart_scripts` WHERE `entryorguid`=40510 AND `source_type`=0 AND `id` IN (0,1,2);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(40510,0,0,0,25,0,100,0,0,0,0,0,0,11,75573,2,0,0,0,0,1,0,0,0,0,0,0,0,'Fathom-Lord Zin''jatar (weakened) - on reset - cast Weakened'),
(40510,0,1,0,4,0,100,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Fathom-Lord Zin''jatar (weakened) - aggro - say The Lady will claim you'),
(40510,0,2,0,0,0,100,0,8000,10000,9000,12000,0,11,75571,0,0,0,0,0,2,0,0,0,0,0,0,0,'Fathom-Lord Zin''jatar (weakened) - IC - cast Wounding Strike (plausible, same DBC block)');

-- 25442 A Pearl of Wisdom: 54614 loot row already present at 100% (WPP import);
-- item-start gating is native via Item-sparse StartQuest. Nothing further needed.

-- ############################################################################
-- ARC S4 — SILVER TIDE HOLLOW HUB + NAR'SHOLA / QUEL'DORMIR RUINS
-- (25535-25540, 25579-25583, 27716/27717, 27393; hub spawns in creature block above)
-- ############################################################################

-- ---------------------------------------------------------------------------
-- S4.1 25582 A Better Vantage — proximity-credit triggers (spawns 9001250-52 above)
-- ---------------------------------------------------------------------------
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry` IN (40963,40964,40965) AND `AIName`='';
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (40963,40964,40965) AND `source_type`=0 AND `id`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(40963,0,0,0,10,0,100,0,1,35,5000,5000,1,33,40963,0,0,0,0,0,7,0,0,0,0,0,0,0,'Gardens scout trigger - player in LOS - give credit'),
(40964,0,0,0,10,0,100,0,1,35,5000,5000,1,33,40964,0,0,0,0,0,7,0,0,0,0,0,0,0,'Tunnel scout trigger - player in LOS - give credit'),
(40965,0,0,0,10,0,100,0,1,35,5000,5000,1,33,40965,0,0,0,0,0,7,0,0,0,0,0,0,0,'Structures scout trigger - player in LOS - give credit');
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=22 AND `SourceEntry` IN (40963,40964,40965);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(22,1,40963,0,0,9,0,25582,0,0,0,0,0,'','Scout credit only while A Better Vantage in progress'),
(22,1,40964,0,0,9,0,25582,0,0,0,0,0,'','Scout credit only while A Better Vantage in progress'),
(22,1,40965,0,0,9,0,25582,0,0,0,0,0,'','Scout credit only while A Better Vantage in progress');

-- ---------------------------------------------------------------------------
-- S4.2 25537 Art of Attraction — 75868 credit is DBC-native (KILL_CREDIT2 40654);
-- SAI only despawns the drained bunny. Implicit-target condition steers e0 dummy to the bunny.
-- ---------------------------------------------------------------------------
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=40654 AND `AIName`='';
DELETE FROM `smart_scripts` WHERE `entryorguid`=40654 AND `source_type`=0 AND `id`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(40654,0,0,0,8,0,100,0,75868,0,500,500,0,41,300,0,0,0,0,0,1,0,0,0,0,0,0,0,'Anemone Gas Bunny - chemical extraction - despawn');
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry`=75868;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(13,1,75868,0,0,31,0,3,40654,0,0,0,0,'','Anemone Chemical Extraction e0 dummy hits Anemone Gas Bunny');

-- ---------------------------------------------------------------------------
-- S4.3 25538 Odor Coater — diver credit + reaction texts; Anemone Frenzy behavior
-- 76033 -> forcecast 75918 -> diver summons 40710 (native). Credit is NOT in DBC -> SAI.
-- ---------------------------------------------------------------------------
DELETE FROM `creature_text` WHERE `CreatureID`=40646 AND `GroupID`=0;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(40646,0,0,'What the... disgusting!',12,0,100,0,0,0,0,0,0,'Glimmerdeep Diver - odor coated 1'),
(40646,0,1,'A fish?  For me?  It''s adorable.',12,0,100,0,0,0,0,0,0,'Glimmerdeep Diver - odor coated 2'),
(40646,0,2,'I don''t want to be ungrateful, but was that absolutely necessary?',12,0,100,0,0,0,0,0,0,'Glimmerdeep Diver - odor coated 3'),
(40646,0,3,'This will make me feel a tad safer from those murlocs.  Thank you.',12,0,100,0,0,0,0,0,0,'Glimmerdeep Diver - odor coated 4'),
(40646,0,4,'Hexascrub, right?  Of course.',12,0,100,0,0,0,0,0,0,'Glimmerdeep Diver - odor coated 5');
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=40646 AND `AIName`='';
DELETE FROM `smart_scripts` WHERE `entryorguid`=40646 AND `source_type`=0 AND `id` IN (0,1);
DELETE FROM `smart_scripts` WHERE `entryorguid`=40710 AND `source_type`=0 AND `id` IN (1,2,3);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(40646,0,0,1,8,0,100,0,76033,0,2000,2000,0,33,40646,0,0,0,0,0,7,0,0,0,0,0,0,0,'Glimmerdeep Diver - hit by Apply Anemone Chemicals - kill credit to player'),
(40646,0,1,0,61,0,100,0,0,0,0,0,0,1,0,0,0,0,0,0,7,0,0,0,0,0,0,0,'Glimmerdeep Diver - linked - random reaction say'),
(40710,0,1,2,54,0,100,0,0,0,0,0,0,11,75919,2,0,0,0,0,1,0,0,0,0,0,0,0,'Anemone Frenzy - just summoned - cast Anemone Fish Guardian'),
(40710,0,2,3,61,0,100,0,0,0,0,0,0,49,0,0,0,0,0,0,23,0,0,0,0,0,0,0,'Anemone Frenzy - linked - attack the summoning diver'),
(40710,0,3,0,61,0,100,0,0,0,0,0,0,41,15000,0,0,0,0,0,1,0,0,0,0,0,0,0,'Anemone Frenzy - linked - despawn after 15s');

-- ---------------------------------------------------------------------------
-- S4.4 25580 Swift Approach — 40877 Azsh'ir Monitor alert gimmick + dup-spawn cleanup
-- ---------------------------------------------------------------------------
DELETE FROM `creature_text` WHERE `CreatureID`=40877 AND `GroupID`=0;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(40877,0,0,'An Azsh''ir Monitor has spotted you!',16,0,100,0,0,0,0,0,0,'Azsh''ir Monitor - alert');
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=40877 AND `AIName`='';
DELETE FROM `smart_scripts` WHERE `entryorguid`=40877 AND `source_type`=0 AND `id`=0;
DELETE FROM `smart_scripts` WHERE `entryorguid`=4087700 AND `source_type`=9 AND `id` IN (0,1,2);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(40877,0,0,0,10,0,100,0,0,20,15000,30000,1,80,4087700,0,2,0,0,0,1,0,0,0,0,0,0,0,'Azsh''ir Monitor - hostile player in LOS - alert sequence'),
(4087700,9,0,0,0,0,100,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Monitor alert - emote An Azsh''ir Monitor has spotted you!'),
(4087700,9,1,0,0,0,100,0,0,0,0,0,0,11,76284,2,0,0,0,0,7,0,0,0,0,0,0,0,'Monitor alert - channel Dummy Channel alarm pose'),
(4087700,9,2,0,0,0,100,0,2000,2000,0,0,0,49,0,0,0,0,0,0,7,0,0,0,0,0,0,0,'Monitor alert - +2s - attack the spotted player');
-- Delete 11 WPP duplicate spawn rows (identical coords, keep lowest guid); light wander for feel
DELETE c FROM `creature` c JOIN (SELECT ROUND(`position_x`,1) rx, ROUND(`position_y`,1) ry, MIN(`guid`) mg FROM `creature` WHERE `id`=40877 GROUP BY ROUND(`position_x`,1), ROUND(`position_y`,1)) k
  ON ROUND(c.`position_x`,1)=k.rx AND ROUND(c.`position_y`,1)=k.ry AND c.`guid`<>k.mg WHERE c.`id`=40877;
UPDATE `creature` SET `MovementType`=1, `wander_distance`=8 WHERE `id`=40877;

-- ---------------------------------------------------------------------------
-- S4.5 25579 Caught Off-Guard — 39638 flee-at-low-HP (mirrors 39664 row; append after ids 0-2)
-- ---------------------------------------------------------------------------
DELETE FROM `smart_scripts` WHERE `entryorguid`=39638 AND `source_type`=0 AND `id`=3;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(39638,0,3,0,2,0,100,1,0,15,0,0,0,25,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Azsh''ir Patroller - flee at 15% HP');

-- ---------------------------------------------------------------------------
-- S4.6 27716/27717 Piece of the Past — faction-gate the starter drops
-- (drops already on 39638 @2.2% / 41227 @1.4% from WPP import; can-take-quest gating is native)
-- ---------------------------------------------------------------------------
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=1 AND `SourceGroup` IN (39638,41227) AND `SourceEntry` IN (62281,62282);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(1,39638,62281,0,0,6,0,469,0,0,0,0,0,'','Ancient Sword Pommel (A starter) - Alliance only'),
(1,39638,62282,0,0,6,0,67,0,0,0,0,0,'','Ancient Sword Pommel (H starter) - Horde only'),
(1,41227,62281,0,0,6,0,469,0,0,0,0,0,'','Ancient Sword Pommel (A starter) - Alliance only'),
(1,41227,62282,0,0,6,0,67,0,0,0,0,0,'','Ancient Sword Pommel (H starter) - Horde only');

-- ---------------------------------------------------------------------------
-- S4.7 25583 finale RP hook — Dvorek signals Valoren (Valoren SAI/texts owned by the Wavespeaker agent:
-- convention = set data field 1 value 1 on 39881 at 25583 reward; Dvorek's reply line imported here as his g0)
-- ---------------------------------------------------------------------------
DELETE FROM `creature_text` WHERE `CreatureID`=40643 AND `GroupID`=0;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(40643,0,0,'Of course.  It''s all yours.',12,0,100,0,0,0,0,0,0,'Admiral Dvorek - hands the shard to Valoren');
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=40643 AND `AIName`='';
DELETE FROM `smart_scripts` WHERE `entryorguid`=40643 AND `source_type`=0 AND `id`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(40643,0,0,0,20,0,100,0,25583,1000,1000,0,0,45,1,1,0,0,0,0,19,39881,50,0,0,0,0,0,'Admiral Dvorek - 25583 rewarded - set data 1 1 on Wavespeaker Valoren (starts shard RP)');

-- ---------------------------------------------------------------------------
-- S4.8 Quest text polish — request-items / offer-reward / details
-- ---------------------------------------------------------------------------
DELETE FROM `quest_request_items` WHERE `ID` IN (25539,25540,27716,27717,25583);
INSERT INTO `quest_request_items` (`ID`,`EmoteOnComplete`,`EmoteOnIncomplete`,`CompletionText`,`VerifiedBuild`) VALUES
(25539,1,0,'Has your clam-laden frolic proved fruitful?',0),
(25540,1,0,'You look to have come back to us with purpose. Was your trip successful?',0),
(27716,1,0,'Hah! Did you see the admiral giving me dirty looks and decide to throw some sympathy my way? I''ve pretty much lost hope of making him understand that these ruins offer us some opportunity to salvage some usefulness out of this whole mess.$B$BI''d actually hoped that you might turn up again - you seem like the only one I can rely on. Wait... what''s that you have there?',0),
(27717,1,0,'Hah! Did you see the admiral giving me dirty looks and decide to throw some sympathy my way? I''ve pretty much lost hope of making him understand that these ruins offer us some opportunity to salvage some usefulness out of this whole mess.$B$BI''d actually hoped that you might turn up again - you seem like the only one I can rely on. Wait... what''s that you have there?',0),
(25583,6,0,'Yes, $c?',0);
DELETE FROM `quest_offer_reward` WHERE `ID`=27393;
INSERT INTO `quest_offer_reward` (`ID`,`Emote1`,`Emote2`,`Emote3`,`Emote4`,`EmoteDelay1`,`EmoteDelay2`,`EmoteDelay3`,`EmoteDelay4`,`RewardText`,`VerifiedBuild`) VALUES
(27393,0,0,0,0,0,0,0,0,'The Wavespeaker was right. There''s something here.',0);
DELETE FROM `quest_details` WHERE `ID`=25535;
INSERT INTO `quest_details` (`ID`,`Emote1`,`Emote2`,`Emote3`,`Emote4`,`EmoteDelay1`,`EmoteDelay2`,`EmoteDelay3`,`EmoteDelay4`,`VerifiedBuild`) VALUES
(25535,1,0,0,0,0,0,0,0,0);

-- ============================================================================
-- END OF BATCH A
-- Native/no-op (verified, deliberately untouched): quest_poi (all 32 quests
-- already populated by WPP import), 25334 turn-in phasing (RewardSpell 74858),
-- 54614/62281/62282 loot rows, statue goobers + page texts (25581), clam GO
-- loot (25539), item-start 27716/27717/25442, GO 202714 relations (27393/25583),
-- orphan drafts 25625/25631-25635 already in disables.
-- ============================================================================
