-- ============================================================================
-- Shimmering Expanse C++ script bindings + support rows
-- Pairs with src/server/scripts/EasternKingdoms/Vashjir/vashjir_shimmering_expanse.cpp
-- DO NOT APPLY until the worldserver build containing the module is deployed.
--
-- NOT included here (owned by the SQL/spawn agents - see the arc reports):
--   * all creature/GO spawns, creature_text rows, SmartAI, gossip, quest_*
--     rows, loot, spell_area / phase ladders, item_sparse hotfixes.
--
-- Spawns my scripts REQUIRE from the spawn agents (with exact placement):
--   * 40163 Intro Cave Defense Controller Bunny at -5159.75 3989.76 -14.68,
--     spawn with PhaseId 171 ONLY (AI joins 172 itself).
--   * 40161 Fathom-Lord Zin'jatar at -5165.79 3975.01 -14.07, PhaseId 172,
--     spawntimesecs ~30; 4x 40375 Tsunami Bunny back wall, PhaseId 172.
--   * 42135 "Defend the Bridge Quad Credit" x1 at mid-bridge
--     (-7300.4 4870.9 -284.9), spawn with PhaseId 183 ONLY (AI joins 184) -
--     it doubles as the Final Judgement event controller.
--   * 41982 Quel'Dormir Temple Credit Bunny x1 at the crucible
--     (-7270 5075 -270), base phase 169 (proximity credit for 25626 obj 1).
--   * 40789 Generic Controller Bunny near the crucible and near the south
--     bridge end (whisper mouthpieces; groups 0/1, see text table below).
--   * 48423 Captain Glovaal + 48429 First Lieutenant Wiley inside the static
--     submarine hull at ~ -7234 3838 -66 (phase 228) for the voyage RP.
--   * 41776 Swiftfin Seahorse (spellclick) at -6548.9 4242.4 -471.7.
--   * 42488 Chief Engineer Yoon already spawned (guid 345885).
--
-- creature_text groups the module drives (rows owned by the text agents):
--   40398: 0 breach / 1 workin' together / 2 disturbin' / 3 ready-ta-leave
--   40161: 0 aggro yell (snd 21857) / 1 boss emote (type 41) / 2 concede (snd 21858)
--   41532: 0 "What's this over here?"
--   41803: 0 well-suited / 1 "Good luck to you too, $n."
--   39584: 0 "A single prong..." / 1 whisper "Further attuning yourself..."
--   40978: 0 "...trident breaking..." (snd 21670) / 1 "Indeed. I doubt much would." (21671)
--   42077: 0 move forward / 1 cut them down / 2 allies arrived (snds 21796-98)
--   42073: 0 march to your deaths (21852) / 1 slaughter them all (21854)
--   42075: 0 back to the sands (21666)
--   42060: 0 curse upon your kind
--   42063: 0 left this city to the waves (21525) / 1 disease upon the sea (21526)
--   40789: 0 crucible whisper / 1 bridge-defended whisper (type 42)
--   42488: 0 arriving shortly / 1 docked, all aboard
--   48429: 0 approaching / 1 reports accurate / 2 beast escaped (21532-34)
--   48423: 0 take her in / 1 steady / 2 fire! / 3 don't worry (21179-82)
--   40645: 0 "Fresh air!..." / 1 "Ha ha! There's no way they'll miss that..."
-- ============================================================================

-- ---------------------------------------------------------------------------
-- creature_template script bindings + vehicle ids
-- ---------------------------------------------------------------------------
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_vashjir_spirit_of_gadra' WHERE `entry`=40398;
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_vashjir_cave_defense_controller' WHERE `entry`=40163;
UPDATE `creature_template` SET `AIName`='', `ScriptName`='boss_vashjir_fathom_lord_zinjatar' WHERE `entry`=40161;
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_vashjir_toshes_vortex', `VehicleId`=735 WHERE `entry`=40277;
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_vashjir_nespirah_escort' WHERE `entry` IN (41532,41803);
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_vashjir_escape_seahorse', `VehicleId`=840 WHERE `entry` IN (41785,41778);
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_vashjir_battlemaiden', `VehicleId`=694, `spell1`=75678, `spell2`=76664 WHERE `entry`=39584;
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_vashjir_battlemaiden', `VehicleId`=812, `spell1`=75678, `spell2`=75684, `spell3`=76619, `spell4`=76664, `spell5`=76569 WHERE `entry`=41225;
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_vashjir_battlemaiden', `VehicleId`=848 WHERE `entry`=41986;
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_vashjir_war_party' WHERE `entry` IN (44421,44422,44423);
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_vashjir_temple_credit_bunny' WHERE `entry`=41982;
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_vashjir_bridge_controller' WHERE `entry`=42135;
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_vashjir_chief_engineer_yoon' WHERE `entry`=42488;

-- Support vehicle ids for the SAI-owned arcs (sniff create-block RecIDs; the
-- spawn agents may also carry these - idempotent):
UPDATE `creature_template` SET `VehicleId`=816 WHERE `entry`=41247;   -- Tamed Bombing Ray (25752)
UPDATE `creature_template` SET `VehicleId`=1355 WHERE `entry`=48898;  -- Tamed Seahorse (25892)

-- ---------------------------------------------------------------------------
-- spell_script_names
-- ---------------------------------------------------------------------------
DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
'spell_vashjir_spirit_trance_aura','spell_vashjir_spirit_vision_timer','spell_vashjir_naga_wave_engine',
'spell_vashjir_toshes_vortex_trigger','spell_vashjir_summon_escape_seahorse',
'spell_vashjir_blade_of_the_battlemaiden','spell_vashjir_battlemaiden_final_phase',
'spell_vashjir_battlemaiden_backup_credit','spell_vashjir_battlemaiden_vision_exit',
'spell_vashjir_naga_reinforcement_ping','spell_vashjir_rescue_flare');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(74386, 'spell_vashjir_spirit_trance_aura'),          -- Spirit Trance: expire -> 74385
(81811, 'spell_vashjir_spirit_vision_timer'),         -- expire -> 75482 (Spirit of Gadra)
(74848, 'spell_vashjir_naga_wave_engine'),            -- suppress E1 periodic (controller owns waves)
(75109, 'spell_vashjir_toshes_vortex_trigger'),       -- vortex pulse: grab serpents
(77927, 'spell_vashjir_summon_escape_seahorse'),      -- faction-split BP summon
(77292, 'spell_vashjir_blade_of_the_battlemaiden'),   -- blade -> transform bunny forcecast
(78332, 'spell_vashjir_battlemaiden_final_phase'),    -- -> 78263 (phase 172)
(80674, 'spell_vashjir_battlemaiden_backup_credit'),  -- -> 77283 backstop
(77283, 'spell_vashjir_battlemaiden_vision_exit'),
(77284, 'spell_vashjir_battlemaiden_vision_exit'),
(77285, 'spell_vashjir_battlemaiden_vision_exit'),
(76569, 'spell_vashjir_naga_reinforcement_ping'),
(77741, 'spell_vashjir_rescue_flare');

-- ---------------------------------------------------------------------------
-- areatrigger script (Nespirah throat tunnel, DBC trigger 5958)
-- The script summons the Duarn/Erunak escort pair and completes 25890 itself
-- (returns true - no areatrigger_involvedrelation row needed).
-- ---------------------------------------------------------------------------
DELETE FROM `areatrigger_scripts` WHERE `entry`=5958;
INSERT INTO `areatrigger_scripts` (`entry`, `ScriptName`) VALUES (5958, 'at_nespirah_tunnel');

-- ---------------------------------------------------------------------------
-- conditions - spell implicit targets (SourceType 13)
-- ---------------------------------------------------------------------------
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry` IN (77292,73974,77565,78264,75312,78268);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
-- 77292 Blade of the Naz'jar Battlemaiden: E0 nearby-entry = the transform bunnies
(13, 1, 77292, 0, 0, 31, 0, 3, 41160, 0, 0, 0, 0, '', 'Blade of the Battlemaiden - target Transform Bunny Vision 1'),
(13, 1, 77292, 0, 1, 31, 0, 3, 41436, 0, 0, 0, 0, '', 'Blade of the Battlemaiden - target Transform Bunny Vision 2'),
(13, 1, 77292, 0, 2, 31, 0, 3, 41484, 0, 0, 0, 0, '', 'Blade of the Battlemaiden - target Transform Bunny Vision 3'),
-- Battlemaiden summons: E0 dest-nearby-entry = summon at the transform bunny
(13, 1, 73974, 0, 0, 31, 0, 3, 41160, 0, 0, 0, 0, '', 'Naz''jar Battlemaiden V1 - summon dest at Vision 1 bunny'),
(13, 1, 77565, 0, 0, 31, 0, 3, 41436, 0, 0, 0, 0, '', 'Naz''jar Battlemaiden V2 - summon dest at Vision 2 bunny'),
(13, 1, 78264, 0, 0, 31, 0, 3, 41484, 0, 0, 0, 0, '', 'Naz''jar Battlemaiden V3 - summon dest at Vision 3 bunny'),
-- 75312 Tsunami Knockback: dest-area-entry -> players (knockback + native KC 40161)
(13, 7, 75312, 0, 0, 31, 0, 4, 0, 0, 0, 0, 0, '', 'Tsunami Knockback - hit players'),
-- 78268 Naz'jar Honor Guard Orders Credit (SAI-cast): hit players + the V3 vehicle
(13, 3, 78268, 0, 0, 31, 0, 4, 0, 0, 0, 0, 0, '', 'Honor Guard Orders Credit - hit players'),
(13, 3, 78268, 0, 1, 31, 0, 3, 41986, 0, 0, 0, 0, '', 'Honor Guard Orders Credit - hit Battlemaiden vehicle');

-- ---------------------------------------------------------------------------
-- Swiftfin escape seahorse spellclick (25922) - the DB row currently points at
-- 86358; the escape event needs 77927 (KC2 41776 native + scripted BP summon).
-- ---------------------------------------------------------------------------
UPDATE `creature_template` SET `npcflag` = `npcflag` | 16777216 WHERE `entry`=41776;
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry`=41776;
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(41776, 77927, 1, 0);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=18 AND `SourceGroup`=41776;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(18, 41776, 77927, 0, 0, 9, 0, 25922, 0, 0, 0, 0, 0, '', 'Swiftfin Seahorse click - only on Waking the Beast');
