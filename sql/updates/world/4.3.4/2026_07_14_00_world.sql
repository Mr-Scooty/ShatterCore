-- 447 (14125): retail Gasbot event + KTC Headquarters fire phasing (P2 sniff).
--
-- Sniffed flow: using the Gasbot Control Panel makes the PLAYER cast 70253
-- (panel goober playerCast) -> 70254 "Gasbot Master" -> 70255 despawns stale
-- bots + 70252 summons a fresh one at the panel. Four Gasbot Gas Targets
-- (37599) board it (vehicle_template_accessory, already present). At t+1.5s it
-- casts 70256 "Gasbot Gas Stream" (caster + passengers) and walks up the HQ
-- steps; at the top it casts 70259 "Gasbot Explosion" + 70260 credit-to-master,
-- and the 23 "447 Fire" objects appear. It is destroyed 1.6s later.

-- ----------------------------------------------------------------------------
-- 1) The 23 "447 Fire" spawns are the burnt-HQ state: they were visible for the
--    whole evacuation era (phase 384), so the tower was ablaze before the
--    player ever touched the quest. Move them to a dedicated phase that only
--    activates once 447's objectives are complete (or the quest is rewarded).
-- ----------------------------------------------------------------------------
UPDATE `gameobject` SET `PhaseId` = 385 WHERE `id` = 201745 AND `map` = 648;

DELETE FROM `phase_area` WHERE `AreaId` = 4737 AND `PhaseId` = 385;
INSERT INTO `phase_area` (`AreaId`, `PhaseId`, `Comment`) VALUES
(4737, 385, 'Kezan - KTC Headquarters ablaze (447 complete)');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 26 AND `SourceGroup` = 385 AND `SourceEntry` = 4737;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(26, 385, 4737, 1, 28, 0, 14125, 0, 0, 0, 0, 0, '', 'Kezan phase 385: 447 objectives complete'),
(26, 385, 4737, 2, 8, 0, 14125, 0, 0, 0, 0, 0, '', 'Kezan phase 385: 447 rewarded');

-- ----------------------------------------------------------------------------
-- 2) The control panel is a native player-cast spellcaster goober (Data0 =
--    70253, Data6 = playerCast); the bespoke GameObjectScript is gone.
-- ----------------------------------------------------------------------------
UPDATE `gameobject_template` SET `Data6` = 1, `ScriptName` = '' WHERE `entry` = 201736;

-- ----------------------------------------------------------------------------
-- 3) Spell chain scripts.
-- ----------------------------------------------------------------------------
DELETE FROM `spell_script_names` WHERE `spell_id` IN (70220, 70223, 70239, 70240, 70247, 70248, 70252, 70253, 70254, 70255, 70260);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(70220, 'spell_kezan_arson_despawn'),
(70223, 'spell_kezan_arson_master'),
(70239, 'spell_kezan_arson_master'),
(70240, 'spell_kezan_arson_despawn'),
(70247, 'spell_kezan_arson_master'),
(70248, 'spell_kezan_arson_despawn'),
(70252, 'spell_kezan_summon_gasbot'),
(70253, 'spell_kezan_gasbot_panel'),
(70254, 'spell_kezan_gasbot_master'),
(70255, 'spell_kezan_arson_despawn'),
(70260, 'spell_kezan_gasbot_credit');

-- The despawn spells hit TARGET_UNIT_SRC_AREA_ENTRY, the summons (and the cigar
-- missile 70251) aim TARGET_DEST_NEARBY_ENTRY: all need implicit-target
-- conditions to pick their entry (summons anchor on their house goober).
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` IN (70198, 70220, 70240, 70241, 70248, 70249, 70251, 70252, 70255);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 70255, 0, 31, 0, 3, 37598, 0, 0, 0, 0, '', '447: Despawn Gasbot - only hit Gasbots'),
(13, 1, 70220, 0, 31, 0, 3, 37561, 0, 0, 0, 0, '', '447: Despawn Overloaded Generator - only hit Overloaded Generators'),
(13, 1, 70240, 0, 31, 0, 3, 37590, 0, 0, 0, 0, '', '447: Despawn Stove Leak - only hit Stove Leaks'),
(13, 1, 70248, 0, 31, 0, 3, 37594, 0, 0, 0, 0, '', '447: Despawn Smoldering Bed - only hit Smoldering Beds'),
(13, 1, 70252, 0, 31, 0, 5, 201736, 0, 0, 0, 0, '', '447: Summon Gasbot - dest anchored at the Gasbot Control Panel'),
(13, 1, 70198, 0, 31, 0, 5, 201735, 0, 0, 0, 0, '', '447: Summon Overloaded Generator - dest anchored at the Defective Generator'),
(13, 1, 70241, 0, 31, 0, 5, 201733, 0, 0, 0, 0, '', '447: Summon Stove Leak - dest anchored at the Leaky Stove'),
(13, 1, 70249, 0, 31, 0, 5, 201734, 0, 0, 0, 0, '', '447: Summon Smoldering Bed - dest anchored at the Flammable Bed'),
(13, 1, 70251, 0, 31, 0, 5, 201734, 0, 0, 0, 0, '', '447: Smoldering Bed Precast (cigar missile) - dest anchored at the Flammable Bed');

-- ----------------------------------------------------------------------------
-- 4) Arson houses (sniff): same native master chain as the Gasbot. Each goober
--    natively grants its credit (Data1) and force-casts its master via Data10
--    (70197/70238/70245); the master script despawns and re-summons the
--    smoldering prop, which self-casts its visual. The props are per-use player
--    summons, so the static pre-lit markers and the placeholder SAI go away.
-- ----------------------------------------------------------------------------
DELETE FROM `creature` WHERE `guid` IN (9000358, 9000359, 9000360);
UPDATE `gameobject_template` SET `AIName` = '' WHERE `entry` IN (201733, 201734, 201735);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (201733, 201734, 201735) AND `source_type` = 1;

DELETE FROM `smart_scripts` WHERE `entryorguid` IN (37561, 37590, 37594) AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(37561, 0, 0, 0, 54, 0, 100, 0, 0, 0, 0, 0, 0, 11, 70226, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Overloaded Generator - On Just Summoned - Cast 447: Overloaded Generator Visual'),
(37590, 0, 0, 0, 54, 0, 100, 0, 0, 0, 0, 0, 0, 11, 70236, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Stove Leak - On Just Summoned - Cast 447: Stove Leak Visual'),
(37594, 0, 0, 0, 54, 0, 100, 0, 0, 0, 0, 0, 0, 11, 70250, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Smoldering Bed - On Just Summoned - Cast 447: Smoldering Bed Visual');
