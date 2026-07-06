-- Shannox encounter completion: 10/25 Normal + 10/25 Heroic
--
-- Retail 4.3.4 health values, HealthModifier = target HP / creature_classlevelstats basehp3
--   basehp3: level 88 = 85892, level 87 = 82994, level 85 = 77490 (all units are unit_class 1)
-- Do NOT tune these against the modern-client sniffs - post-squish stats are unusable here.

-- Shannox: 24,049,760 / 81,597,400 / 33,669,664 / 114,236,360
UPDATE `creature_template` SET `HealthModifier` = 280  WHERE `entry` = 53691; -- 10N
UPDATE `creature_template` SET `HealthModifier` = 950  WHERE `entry` = 53979; -- 25N
UPDATE `creature_template` SET `HealthModifier` = 392  WHERE `entry` = 54079; -- 10H
UPDATE `creature_template` SET `HealthModifier` = 1330 WHERE `entry` = 54080; -- 25H

-- Riplimb: 9,295,328 / 32,533,648 / 4,647,664 / 16,266,824
-- (Heroic values low by design: he collapses and reanimates instead of dying)
UPDATE `creature_template` SET `HealthModifier` = 112  WHERE `entry` = 53694; -- 10N
UPDATE `creature_template` SET `HealthModifier` = 392  WHERE `entry` = 53980; -- 25N
UPDATE `creature_template` SET `HealthModifier` = 56   WHERE `entry` = 54077; -- 10H
UPDATE `creature_template` SET `HealthModifier` = 196  WHERE `entry` = 54078; -- 25H

-- Rageface: 9,295,328 / 32,533,648 / 41,497,000 / 145,239,500
-- (Heroic Rageface CAN die permanently - only Riplimb reanimates per the dungeon journal -
--  but his health is deliberately prohibitive; he is only hit to break Face Rage)
UPDATE `creature_template` SET `HealthModifier` = 112  WHERE `entry` = 53695; -- 10N
UPDATE `creature_template` SET `HealthModifier` = 392  WHERE `entry` = 53981; -- 25N
UPDATE `creature_template` SET `HealthModifier` = 500  WHERE `entry` = 54075; -- 10H
UPDATE `creature_template` SET `HealthModifier` = 1750 WHERE `entry` = 54076; -- 25H

-- Crystal Prison: 150k / 450k / 300k / 900k (script no longer hardcodes 2.8M)
UPDATE `creature_template` SET `HealthModifier` = 1.75    WHERE `entry` = 53819; -- 10N (level 88)
UPDATE `creature_template` SET `HealthModifier` = 5.81    WHERE `entry` = 53820; -- 25N (level 85)
UPDATE `creature_template` SET `HealthModifier` = 3.87    WHERE `entry` = 53821; -- 10H (level 85)
UPDATE `creature_template` SET `HealthModifier` = 11.61   WHERE `entry` = 53822; -- 25H (level 85)

-- Face Rage victim aura (99947): ramping maul damage + break/cleanup notification
DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_shannox_face_rage';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (99947, 'spell_shannox_face_rage');

-- 'Fetch your supper!' is Shannox's Hurl Spear yell (sniff: "Shannox to Spear of Shannox"),
-- not a player-kill line - move it from the SAY_SLAY group (2) to SAY_HURL_SPEAR (6)
DELETE FROM `creature_text` WHERE `CreatureID` = 53691 AND `GroupID` = 2 AND `ID` = 3;
DELETE FROM `creature_text` WHERE `CreatureID` = 53691 AND `GroupID` = 6;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(53691, 6, 0, 'Fetch your supper!', 14, 0, 100, 0, 0, 24569, 52452, 0, 'Shannox - SAY_HURL_SPEAR');

-- Fallback ONLY if Feeding Frenzy (100655) does not proc from its DBC data in testing:
-- proc on successful melee auto-attacks (ProcFlags 0x4 = DONE_MELEE_AUTO_ATTACK,
-- HitMask 0x3 = NORMAL | CRITICAL), 100% chance.
-- DELETE FROM `spell_proc` WHERE `SpellId` = 100655;
-- INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
-- (100655, 0, 0, 0, 0, 0, 0x4, 0, 0, 0x3, 0, 0, 0, 100, 0, 0);
