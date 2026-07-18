-- ============================================================================
-- Kelp'thar Forest C++ event support (vashjir_kelpthar_forest.cpp)
-- Extra DB wiring required by the new scripts. Applied with the Kelpthar event scripts.
-- ============================================================================

-- ---------------------------------------------------------------------------
-- 1. ScriptName bindings for the new CreatureAIs
--    (40759/40770/40782 keep their SmartAI - the controller drives them)
-- ---------------------------------------------------------------------------
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_briny_cutter_battle_bunny' WHERE `entry`=40756;
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_zinjatar_abductor'         WHERE `entry`=40786;
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_zinjatar_abductor_player'  WHERE `entry`=40797;
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_erunak_rescue'             WHERE `entry`=40801;
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_naga_death_bunny'          WHERE `entry`=40605;
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_abyssal_seahorse'          WHERE `entry`=39996;

-- ---------------------------------------------------------------------------
-- 2. VehicleId for the personal player abductor (40786 already has 569 in DB)
--    76123 script effect boards the summoner into seat 0.
-- ---------------------------------------------------------------------------
UPDATE `creature_template` SET `VehicleId`=569 WHERE `entry`=40797;

-- ---------------------------------------------------------------------------
-- 3. SpellScript bindings
-- ---------------------------------------------------------------------------
DELETE FROM `spell_script_names` WHERE `spell_id` IN (86672,76123,74574,87217,87219,86332);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(86672, 'spell_vashjir_sea_legs_reward'),
(76123, 'spell_vashjir_force_creator_ride_abductor'),
(74574, 'spell_vashjir_forcecast_abyssal_ride'),
(87217, 'spell_vashjir_seahorse_rodeo_response'),
(87219, 'spell_vashjir_seahorse_rodeo_response'),
(86332, 'spell_vashjir_seahorse_rodeo_response');

-- ---------------------------------------------------------------------------
-- 4. Rescue-trio summon destinations
--    76127/77324/77326 are Effect 28 with TargetA=17 (TARGET_DEST_DB) -
--    without these rows the trio summons at the caster instead of the
--    sniffed rescue spots.
-- ---------------------------------------------------------------------------
DELETE FROM `spell_target_position` WHERE `ID` IN (76127,77324,77326);
INSERT INTO `spell_target_position` (`ID`, `EffectIndex`, `MapID`, `PositionX`, `PositionY`, `PositionZ`, `Orientation`, `VerifiedBuild`) VALUES
(76127, 0, 0, -4837.79, 3757.28, -111.09, 0.75, 0), -- Erunak Stonespeaker 40801
(77324, 0, 0, -4831.02, 3750.50, -113.63, 1.10, 0), -- Moanah 41241
(77326, 0, 0, -4845.48, 3765.32, -111.12, 0.35, 0); -- Rendel Firetongue 41244

-- ---------------------------------------------------------------------------
-- 5. Implicit-target conditions (SourceType 13) for area/nearby-entry spells
--    86328 Peck Pufferfish      (TargetA 38 nearby entry)  -> 39942 Abyssal Lure
--    86372 Erunak Success Ping  (TargetA 38 nearby entry)  -> 40105 Erunak (75538 eff1 forces this)
--    76128 Lava Bolt (rescue)   (TargetA 22/7 area entry)  -> 40797 player abductor
--    76110 Lava Bolt (battle)   (TargetA 22/7 area entry)  -> Zin'jatar raider entries
--                                (also fixes the existing 40736/40746 SAI casts)
-- ---------------------------------------------------------------------------
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry` IN (86328,86372,76128,76110);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 86328, 0, 0, 31, 0, 3, 39942, 0, 0, 0, 0, '', 'Peck Pufferfish - target Abyssal Lure'),
(13, 1, 86372, 0, 0, 31, 0, 3, 40105, 0, 0, 0, 0, '', 'Erunak Success Ping - target Erunak Stonespeaker'),
(13, 1, 76128, 0, 0, 31, 0, 3, 40797, 0, 0, 0, 0, '', 'Lava Bolt (rescue) - target Zin''jatar Abductor'),
(13, 1, 76110, 0, 0, 31, 0, 3, 40753, 0, 0, 0, 0, '', 'Lava Bolt (Briny Cutter) - target Zin''jatar Raider swarm'),
(13, 1, 76110, 0, 1, 31, 0, 3, 40759, 0, 0, 0, 0, '', 'Lava Bolt (Briny Cutter) - target Zin''jatar Raider pressure'),
(13, 1, 76110, 0, 2, 31, 0, 3, 40770, 0, 0, 0, 0, '', 'Lava Bolt (Briny Cutter) - target Zin''jatar Raider wave 2'),
(13, 1, 76110, 0, 3, 31, 0, 3, 40782, 0, 0, 0, 0, '', 'Lava Bolt (Briny Cutter) - target Zin''jatar Raider elite');
