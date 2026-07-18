-- ############################################################################
-- Kelp'thar Forest batch 2 - script plumbing for the vashjir_kelpthar_forest.cpp
-- extensions (Budd shark arc 25657/25670/25743, Sanctuary 25794/25812/25887,
-- Horde battle mirror 25949). DO NOT APPLY while the worldserver is running.
-- Assumes the batch-1 import (VehicleIds, spawns, SAI, stp 76707/76819/76747,
-- 77376/77377/77429/77430/77432/77447/77723, phases 122/123/142/170) is live.
-- ############################################################################

-- ############ ScriptName bindings ############
UPDATE `creature_template` SET `ScriptName`='npc_gnaws_bait_bunny'          WHERE `entry`=41051;
UPDATE `creature_template` SET `ScriptName`='npc_gnaws_cloned_image'        WHERE `entry`=41085;
UPDATE `creature_template` SET `ScriptName`='npc_gnaws'                     WHERE `entry`=41057;
UPDATE `creature_template` SET `ScriptName`='npc_player_bait_bunny'         WHERE `entry`=41093;
UPDATE `creature_template` SET `ScriptName`='npc_gnaws_ii'                  WHERE `entry`=41098;
UPDATE `creature_template` SET `ScriptName`='npc_harpoon_chain_bunny'       WHERE `entry`=46460;
UPDATE `creature_template` SET `ScriptName`='npc_pewter_prophet'            WHERE `entry`=41192;
UPDATE `creature_template` SET `ScriptName`='npc_budd_farewell'             WHERE `entry`=46463;
UPDATE `creature_template` SET `ScriptName`='npc_watery_vision'             WHERE `entry`=41294;
UPDATE `creature_template` SET `ScriptName`='npc_dominated_great_shark'     WHERE `entry`=42013;
-- Horde battle controller shares the parameterized Alliance AI (entry switch in C++)
UPDATE `creature_template` SET `ScriptName`='npc_briny_cutter_battle_bunny' WHERE `entry`=41766;

-- Gnaws II: "Fire Harpoon Gun" button on the player-controlled vehicle bar
UPDATE `creature_template` SET `spell1`=76859 WHERE `entry`=41098;

-- ############ spell_script_names ############
DELETE FROM `spell_script_names` WHERE `spell_id` IN (76694,76744,76747,76795,76799,76854,76859,77281,77418,77433,78288,82581,86382);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(76694,'spell_vashjir_release_bait'),
(76744,'spell_vashjir_forcecast_bp_at_caster'),
(76799,'spell_vashjir_forcecast_bp_at_caster'),
(76854,'spell_vashjir_forcecast_bp_at_caster'),
(82581,'spell_vashjir_forcecast_bp_at_caster'),
(76747,'spell_vashjir_gnaws_kill_credit'),
(76795,'spell_vashjir_fastening_chain'),
(76859,'spell_vashjir_fire_harpoon_gun'),
(77281,'spell_vashjir_pewter_pounder_completion'),
(77418,'spell_vashjir_force_master_ride_vision'),
(77433,'spell_vashjir_spelunking_completion'),
(78288,'spell_vashjir_dominate_great_shark'),
(86382,'spell_vashjir_scrying');

-- ############ spell_target_position ############
-- 76747 "Gnaws Kill Credit Teleport See Invis": the dest-db teleport is EFFECT 2
-- (eff0 = script, eff1 = killcredit); the batch-1 import keyed it at index 0.
UPDATE `spell_target_position` SET `EffectIndex`=2 WHERE `ID`=76747 AND `EffectIndex`=0;

-- 86542 "Summon Harpoon Chain Bunny" (dest-db) was missing from batch 1
DELETE FROM `spell_target_position` WHERE `ID`=86542;
INSERT INTO `spell_target_position` (`ID`, `EffectIndex`, `MapID`, `PositionX`, `PositionY`, `PositionZ`, `Orientation`) VALUES
(86542, 0, 0, -4931.12, 3436.69, -115.79, 0);

-- ############ conditions ############
-- Implicit TARGET_UNIT_NEARBY_ENTRY (38) selections - required or target search fails.
-- SourceGroup = effect mask (eff0 -> 1, eff1 -> 2).
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry` IN (76745,76859,77275,77419,78290,78296,86599);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 76745, 0, 0, 31, 0, 3, 41051, 0, 0, 0, 0, '', 'Observe Gnaws - implicit target Bait Bunny'),
(13, 1, 76859, 0, 0, 31, 0, 3, 41094, 0, 0, 0, 0, '', 'Fire Harpoon Gun - implicit target Rusty Harpoon Gun Bunny'),
(13, 1, 77275, 0, 0, 31, 0, 3, 41192, 0, 0, 0, 0, '', 'Pound Pewter - implicit target The Pewter Prophet'),
(13, 1, 77419, 0, 0, 31, 0, 3, 41294, 0, 0, 0, 0, '', 'Ride Vehicle - implicit target Watery Vision'),
(13, 2, 78290, 0, 0, 31, 0, 3, 42013, 0, 0, 0, 0, '', 'Ride Famished Shark - implicit target Dominated Great Shark'),
(13, 1, 78296, 0, 0, 31, 0, 3, 41996, 0, 0, 0, 0, '', 'Eat Naga - implicit target Zin''jatar Guardian'),
(13, 1, 86599, 0, 0, 31, 0, 3, 46463, 0, 0, 0, 0, '', 'Ping Budd - implicit target Budd');

-- Dominate Creature item spell: explicit target must be a Famished Great Shark
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=17 AND `SourceEntry`=78287;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(17, 0, 78287, 0, 0, 31, 1, 3, 41998, 0, 0, 0, 0, '', 'Dominate Creature - target must be Famished Great Shark');

-- ############ H battle crew abduction yells (authored; A twins = 40734/40731) ############
DELETE FROM `creature_text` WHERE `CreatureID` IN (41797,41799);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(41797, 0, 0, 'No! Ahhhhh.....', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Hellscream''s Vanguard - abducted'),
(41799, 0, 0, 'Gah... heeelp!', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Hellscream''s Vanguard - abducted');
