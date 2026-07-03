-- Kezan quest chain: Batch A - footbomb tryouts arc
-- 24567 Report for Tryouts / 24488 The Replacements / 24502 Necessary Roughness /
-- 24503 Fourth and Goal / 24520 Give Sassy the News
--
-- Boarding is native 4.3.4 spell data:
--   70015 "Necessary Roughness: Summon Bilgewater Buccaneer" summons a personal
--   37179 at the parked prop (48526) and rides via BP 70016, which also grants
--   kill credit 48271. 70075 "Fourth and Goal: Summon Bilgewater Buccaneer"
--   summons 37213 at the caster (boarded by script).

-- ----------------------------------------------------------------------------
-- 1) Parked Bilgewater Buccaneer prop (48526): make it spell-clickable.
-- ----------------------------------------------------------------------------
UPDATE `creature_template` SET `npcflag` = `npcflag` | 16777216 WHERE `entry` = 48526;

DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 48526;
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(48526, 70015, 1, 0), -- Necessary Roughness: summon throw-buccaneer
(48526, 70075, 1, 0); -- Fourth and Goal: summon kick-buccaneer (re-boarding after dismount)

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 18 AND `SourceGroup` = 48526;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(18, 48526, 70015, 0, 0, 31, 0, 4, 0, 0, 0, 0, 0, '', 'Bilgewater Buccaneer prop: clicker must be a player'),
(18, 48526, 70015, 0, 0, 9, 0, 24502, 0, 0, 0, 0, 0, '', 'Bilgewater Buccaneer prop: 70015 requires Necessary Roughness taken'),
(18, 48526, 70075, 0, 1, 31, 0, 4, 0, 0, 0, 0, 0, '', 'Bilgewater Buccaneer prop: clicker must be a player'),
(18, 48526, 70075, 0, 1, 9, 0, 24503, 0, 0, 0, 0, 0, '', 'Bilgewater Buccaneer prop: 70075 requires Fourth and Goal (24503) taken'),
(18, 48526, 70075, 0, 2, 31, 0, 4, 0, 0, 0, 0, 0, '', 'Bilgewater Buccaneer prop: clicker must be a player'),
(18, 48526, 70075, 0, 2, 9, 0, 28414, 0, 0, 0, 0, 0, '', 'Bilgewater Buccaneer prop: 70075 requires Fourth and Goal (28414) taken');

-- 70015 effect 0 summons at TARGET_DEST_NEARBY_ENTRY: redirect the entry search
-- to the parked prop so the personal buccaneer appears on the pier.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 70015;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 70015, 0, 0, 31, 0, 3, 48526, 0, 0, 0, 0, '', 'Necessary Roughness summon: place buccaneer at the parked prop (48526)');

-- ----------------------------------------------------------------------------
-- 2) Creature templates: vehicles and script bindings.
--    VehicleIds verified against 4.3.4 Vehicle.dbc (582/579 single control seat).
-- ----------------------------------------------------------------------------
UPDATE `creature_template` SET `VehicleId` = 582, `ScriptName` = 'npc_bilgewater_buccaneer' WHERE `entry` = 37179;
UPDATE `creature_template` SET `VehicleId` = 579, `ScriptName` = 'npc_bilgewater_buccaneer' WHERE `entry` = 37213;
UPDATE `creature_template` SET `npcflag` = 0 WHERE `entry` = 37114;
UPDATE `creature_template` SET `ScriptName` = 'npc_fourth_and_goal_target' WHERE `entry` = 37203;
UPDATE `creature_template` SET `ScriptName` = 'npc_kezan_deathwing' WHERE `entry` = 48572;

-- ----------------------------------------------------------------------------
-- 3) Spell scripts (footbomb impacts grant the shark / goal credits).
-- ----------------------------------------------------------------------------
DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_kezan_footbomb_impact', 'spell_kezan_footbomb_kick_impact');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(69993, 'spell_kezan_footbomb_impact'),
(70052, 'spell_kezan_footbomb_kick_impact');

-- ----------------------------------------------------------------------------
-- 4) Texts: buccaneer instructions (whisper to driver) and Deathwing's flyover yell.
-- ----------------------------------------------------------------------------
DELETE FROM `creature_text` WHERE `CreatureID` IN (37179, 37213, 48572);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(37179, 0, 0, 'Throw your Footbombs at those Steamwheedle Sharks!', 42, 0, 100, 0, 0, 0, 0, 0, 'Bilgewater Buccaneer - boarding instruction (Necessary Roughness)'),
(37213, 0, 0, 'Kick the Footbomb between the smokestacks up and behind the opposing goal!', 42, 0, 100, 0, 0, 0, 0, 0, 'Bilgewater Buccaneer - boarding instruction (Fourth and Goal)'),
(48572, 0, 0, 'The sun has set on this mortal world, fools. Make peace with your end, for the hour of twilight falls!', 14, 0, 100, 0, 0, 0, 0, 3, 'Deathwing - Kezan flyover yell');

-- ----------------------------------------------------------------------------
-- 5) Fourth and Goal (28414, the auto-accept variant the chain now offers)
--    was missing its quest text rows; clone them from the dormant 24503.
-- ----------------------------------------------------------------------------
DELETE FROM `quest_details` WHERE `ID` = 28414;
INSERT INTO `quest_details` (`ID`, `Emote1`, `Emote2`, `Emote3`, `Emote4`, `EmoteDelay1`, `EmoteDelay2`, `EmoteDelay3`, `EmoteDelay4`, `VerifiedBuild`) VALUES
(28414, 397, 0, 0, 0, 0, 0, 0, 0, 15595);

DELETE FROM `quest_offer_reward` WHERE `ID` = 28414;
INSERT INTO `quest_offer_reward` (`ID`, `Emote1`, `Emote2`, `Emote3`, `Emote4`, `EmoteDelay1`, `EmoteDelay2`, `EmoteDelay3`, `EmoteDelay4`, `RewardText`, `VerifiedBuild`) VALUES
(28414, 5, 0, 0, 0, 1000, 0, 0, 0, 'Uh... you did it, kid. You REALLY did it! We won the game and....$B$BDID YOU SEE THAT DRAGON?!!!', 15595);
