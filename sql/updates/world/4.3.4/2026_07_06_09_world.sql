--
-- Baleroc (Firelands) encounter completion: retail health, script bindings, Vital Spark/Flame procs.
--

-- Retail 4.3.4 health values (level 88 basehp3 85892 x modifier):
-- 10N 42,087,080 / 25N 133,304,384 / 10H 69,916,088 / 25H 195,576,084
UPDATE `creature_template` SET `HealthModifier` = 490 WHERE `entry` = 53494;
UPDATE `creature_template` SET `HealthModifier` = 1552 WHERE `entry` = 53587;
UPDATE `creature_template` SET `HealthModifier` = 814 WHERE `entry` = 53588;
UPDATE `creature_template` SET `HealthModifier` = 2277 WHERE `entry` = 53589;

-- Share the Pain (5830) is counted on fresh Torment beam applications (99255) now;
-- the old per-Tormented-application counter bindings are obsolete.
DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_baleroc_tormented_debuff';
DELETE FROM `spell_script_names` WHERE `spell_id` IN (99252, 99255, 99263);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(99252, 'spell_baleroc_blaze_of_glory'),
(99255, 'spell_baleroc_torment_beam'),
(99263, 'spell_baleroc_vital_flame');

-- Vital Spark / Vital Flame heal-taken procs on Torment (99256 + 25/heroic clones) and Blaze of Glory (99252).
-- ProcFlags 0x88800 = TAKE_HELPFUL_SPELL | TAKE_HELPFUL_ABILITY | TAKE_PERIODIC, SpellTypeMask 0x2 = heals.
DELETE FROM `spell_proc` WHERE `SpellId` IN (99252, 99256, 100230, 100231, 100232);
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(99252, 0, 0, 0, 0, 0, 0x88800, 0x2, 0x2, 0, 0, 0, 0, 100, 0, 0),
(99256, 0, 0, 0, 0, 0, 0x88800, 0x2, 0x2, 0, 0, 0, 0, 100, 0, 0),
(100230, 0, 0, 0, 0, 0, 0x88800, 0x2, 0x2, 0, 0, 0, 0, 100, 0, 0),
(100231, 0, 0, 0, 0, 0, 0x88800, 0x2, 0x2, 0, 0, 0, 0, 100, 0, 0),
(100232, 0, 0, 0, 0, 0, 0x88800, 0x2, 0x2, 0, 0, 0, 0, 100, 0, 0);
