-- ============================================================================
-- Vashj'ir intro ship event - support data for vashjir_intro.cpp
-- (Call of Duty 14482 Alliance / 25924 Horde)
-- Applied with the intro ship event scripts.
-- ============================================================================

-- ----------------------------------------------------------------------------
-- 1) Transport script bindings + ship phasing
--    TransportScript is resolved via gameobject_template.ScriptName.
--    phaseid 170 = intro ship phase: only quest players (PlayerScript adds
--    phases 171+170 on accept) see the mercenary ships.
-- ----------------------------------------------------------------------------
UPDATE gameobject_template SET ScriptName='transport_vashjir_ship_a' WHERE entry=197195;
UPDATE gameobject_template SET ScriptName='transport_vashjir_ship_h' WHERE entry=203466;
UPDATE transports SET phaseid=170 WHERE entry IN (197195,203466);

-- ----------------------------------------------------------------------------
-- 2) Creature AI bindings
-- ----------------------------------------------------------------------------
UPDATE creature_template SET ScriptName='npc_vashjir_ship_controller' WHERE entry=40559;
UPDATE creature_template SET ScriptName='npc_vashjir_captain_taylor'  WHERE entry=42103;
UPDATE creature_template SET ScriptName='npc_vashjir_budd'            WHERE entry=39480;
UPDATE creature_template SET ScriptName='npc_vashjir_vehicle_pad'     WHERE entry=42202;
UPDATE creature_template SET ScriptName='npc_vashjir_grab_tentacle'   WHERE entry IN (36826,36835,36846,39620,39652,39661,42208);
UPDATE creature_template SET ScriptName='npc_vashjir_intro_tentacle'  WHERE entry=36878;
UPDATE creature_template SET ScriptName='npc_vashjir_submerge_bunny'  WHERE entry=36901;

-- ----------------------------------------------------------------------------
-- 3) Quest completion via script
--    Both quests have zero objectives; SPECIAL_FLAGS_EXPLORATION_OR_EVENT (2)
--    defers completion to Player::AreaExploredOrEventHappens, fired by the
--    TransportScript when the ship approaches the wreck stop (retail:
--    SMSG_QUEST_UPDATE_COMPLETE at 22:39:51, ~30 s before the grab).
-- ----------------------------------------------------------------------------
UPDATE quest_template_addon SET SpecialFlags=2 WHERE ID IN (14482,25924);

-- ----------------------------------------------------------------------------
-- 4) Spell implicit-target conditions (TARGET_UNIT_NEARBY_ENTRY = 38 and
--    TARGET_UNIT_SRC_AREA_ENTRY = 7 need entry conditions to select targets).
--    ConditionTypeOrReference 31 = CONDITION_OBJECT_ENTRY_GUID
--    (ConditionValue1 = TypeID: 3 unit / 4 player, ConditionValue2 = entry).
-- ----------------------------------------------------------------------------
DELETE FROM conditions WHERE SourceTypeOrReferenceId=13 AND SourceEntry IN
    (75633,78739,78749,78752,78760,78762,69394,69408,74067,74131,69414,69396,69407,74068,74130);
INSERT INTO conditions (SourceTypeOrReferenceId,SourceGroup,SourceEntry,SourceId,ElseGroup,ConditionTypeOrReference,ConditionTarget,ConditionValue1,ConditionValue2,ConditionValue3,NegativeCondition,ErrorType,ErrorTextId,ScriptName,Comment) VALUES
-- ship phase pulse hits players only
(13,1,75633,0,0,31,0,4,0,0,0,0,0,'','Phase 1 Intro Aura - target players aboard'),
-- jump-overboard ride spells -> splash pad 42202
(13,1,78739,0,0,31,0,3,42202,0,0,0,0,'','Budd Ride Vehicle - target Budd''s Vehicle Bunny'),
(13,1,78749,0,0,31,0,3,42202,0,0,0,0,'','Soldier Ride Vehicle seat 2 - target splash pad'),
(13,1,78752,0,0,31,0,3,42202,0,0,0,0,'','Recruit Ride Vehicle seat 2 - target splash pad'),
(13,1,78760,0,0,31,0,3,42202,0,0,0,0,'','Soldier Ride Vehicle seat 3 - target splash pad'),
(13,1,78762,0,0,31,0,3,42202,0,0,0,0,'','Soldier Ride Vehicle seat 3 - target splash pad'),
-- tentacle grab force-casts -> named crew victims
(13,1,69394,0,0,31,0,3,36818,0,0,0,0,'','Tentacle vs. Grembul - target Captain Grembul'),
(13,1,69408,0,0,31,0,3,36821,0,0,0,0,'','Tentacle vs. Belindah - target Belindah'),
(13,1,74067,0,0,31,0,3,39447,0,0,0,0,'','Tentacle vs Samir - target Captain Samir'),
(13,1,74131,0,0,31,0,3,39460,0,0,0,0,'','Tentacle vs Adarra - target Adarrah'),
-- knockback hits Billy + Bannon
(13,1,69414,0,0,31,0,3,36820,0,0,0,0,'','Tentacle Knockback - target Billyclub Billy'),
(13,1,69414,0,1,31,0,3,36819,0,0,0,0,'','Tentacle Knockback - target Crewman Bannon'),
-- forced ride spells cast BY the victims -> their tentacles
(13,1,69396,0,0,31,0,3,36826,0,0,0,0,'','Ride Tentacle A - Grembul rides 36826'),
(13,1,69407,0,0,31,0,3,36835,0,0,0,0,'','Ride Tentacle B - Belindah rides 36835'),
(13,1,74068,0,0,31,0,3,39620,0,0,0,0,'','Ride Tentacle D - Samir rides 39620'),
(13,1,74130,0,0,31,0,3,39652,0,0,0,0,'','Ride Tentacle E - Adarrah rides 39652');

-- ----------------------------------------------------------------------------
-- 5) Wreck-site phase-179 ambiance (per-player scene, shared phase).
--    Wreck GO 203746 (guid 9001000) is already spawned in phase 179;
--    Ozumat + drowning crew + Zin'jatar raiders were missing.
--    Ozumat/Grembul coords are sniffed; the scattered drowners/raiders use
--    plausible positions around the wreck (retail scattered ~14+14+20).
-- ----------------------------------------------------------------------------
DELETE FROM creature WHERE guid BETWEEN 9001100 AND 9001121;
INSERT INTO creature (guid,id,map,zoneId,areaId,spawnMask,phaseUseFlags,phaseMask,PhaseId,PhaseGroup,modelid,equipment_id,position_x,position_y,position_z,orientation,spawntimesecs,wander_distance,currentwaypoint,curhealth,curmana,MovementType,npcflag,unit_flags,dynamicflags,ScriptName,VerifiedBuild) VALUES
(9001100,51276,0,4815,5056,1,0,1,179,0,0,0,-4635.368,3861.425,4.587,4.73,300,0,0,1,0,0,0,0,0,'',0),      -- Ozumat on the wreck
(9001101,36996,0,4815,5056,1,0,1,179,0,0,0,-4683.05,3800.48,-68.61,1.2,300,0,0,1,0,0,0,0,0,'',0),         -- Captain Grembul (drowning)
(9001102,37001,0,4815,5056,1,0,1,179,0,0,0,-4650,3820,-30,2.1,300,0,0,1,0,0,0,0,0,'',0),
(9001103,37001,0,4815,5056,1,0,1,179,0,0,0,-4670,3850,-55,0.4,300,0,0,1,0,0,0,0,0,'',0),
(9001104,37001,0,4815,5056,1,0,1,179,0,0,0,-4620,3830,-45,3.6,300,0,0,1,0,0,0,0,0,'',0),
(9001105,37001,0,4815,5056,1,0,1,179,0,0,0,-4655,3880,-25,5.1,300,0,0,1,0,0,0,0,0,'',0),
(9001106,37001,0,4815,5056,1,0,1,179,0,0,0,-4610,3870,-60,1.9,300,0,0,1,0,0,0,0,0,'',0),
(9001107,37001,0,4815,5056,1,0,1,179,0,0,0,-4690,3830,-80,2.7,300,0,0,1,0,0,0,0,0,'',0),
(9001108,39662,0,4815,5056,1,0,1,179,0,0,0,-4640,3800,-40,0.8,300,0,0,1,0,0,0,0,0,'',0),
(9001109,39662,0,4815,5056,1,0,1,179,0,0,0,-4665,3815,-70,4.3,300,0,0,1,0,0,0,0,0,'',0),
(9001110,39662,0,4815,5056,1,0,1,179,0,0,0,-4600,3850,-35,2.4,300,0,0,1,0,0,0,0,0,'',0),
(9001111,39662,0,4815,5056,1,0,1,179,0,0,0,-4630,3900,-50,5.7,300,0,0,1,0,0,0,0,0,'',0),
(9001112,39662,0,4815,5056,1,0,1,179,0,0,0,-4680,3870,-40,1.1,300,0,0,1,0,0,0,0,0,'',0),
(9001113,39662,0,4815,5056,1,0,1,179,0,0,0,-4645,3845,-90,3.2,300,0,0,1,0,0,0,0,0,'',0),
(9001114,43176,0,4815,5056,1,0,1,179,0,0,0,-4655,3835,-50,0.6,300,5,0,1,0,1,0,0,0,'',0),                 -- raiders wander
(9001115,43176,0,4815,5056,1,0,1,179,0,0,0,-4625,3855,-65,2.9,300,5,0,1,0,1,0,0,0,'',0),
(9001116,43176,0,4815,5056,1,0,1,179,0,0,0,-4600,3880,-45,4.8,300,5,0,1,0,1,0,0,0,'',0),
(9001117,43176,0,4815,5056,1,0,1,179,0,0,0,-4670,3800,-55,1.5,300,5,0,1,0,1,0,0,0,'',0),
(9001118,37008,0,4815,5056,1,0,1,179,0,0,0,-4645,3860,-35,2.2,300,0,0,1,0,0,0,0,0,'',0),                 -- vehicle raiders w/ crewman accessory
(9001119,37008,0,4815,5056,1,0,1,179,0,0,0,-4615,3840,-55,4.1,300,0,0,1,0,0,0,0,0,'',0),
(9001120,37008,0,4815,5056,1,0,1,179,0,0,0,-4660,3890,-60,0.9,300,0,0,1,0,0,0,0,0,'',0);

-- Vehicle raiders carry a drowning crewman in their clutches (sniffed accessory)
DELETE FROM vehicle_template_accessory WHERE entry=37008;
INSERT INTO vehicle_template_accessory (entry,accessory_entry,seat_id,minion,description,summontype,summontimer) VALUES
(37008,37001,0,1,'Zin''jatar Raider - Drowning Crewman',8,30000);

-- ----------------------------------------------------------------------------
-- 6) Movement kits for the underwater event actors
--    36901 submerge bunny descends on a fly-spline; 40587/36878 swim.
-- ----------------------------------------------------------------------------
DELETE FROM creature_template_movement WHERE CreatureId IN (36901,40587,36878);
INSERT INTO creature_template_movement (CreatureId,Ground,Swim,Flight,Rooted,Random,InteractionPauseTimer) VALUES
(36901,0,1,1,0,0,NULL),
(40587,1,1,0,0,0,NULL),
(36878,1,1,0,0,0,NULL);

-- ----------------------------------------------------------------------------
-- NOTES (no rows needed):
-- * spell_target_position rows for 69459/69522/75726/75680 (personal-chain
--   summons) and 73727 (Alliance cave teleport) already exist in this DB.
-- * 73728 (Horde teleport) has a row too, but its teleport effect targets
--   TARGET_DEST_CHANNEL_CASTER (106), which the loader/executor cannot bind to
--   spell_target_position - the script teleports Horde players directly to the
--   73728 coords (-4608.16, 3981.21, -70.8, o2.217) instead.
-- * Harbor NPCs 42021/42022/42059/42094/42095/42096/42103 (guids
--   9001044-9001056, phases 171/170) and wreck GO 203746 (guid 9001000,
--   phase 179) are already spawned.
-- * creature_text for 39460/39478/39480/40601/42021/42022/42059/42095/42096/
--   42103 is already imported in sniff group order.
-- ============================================================================
