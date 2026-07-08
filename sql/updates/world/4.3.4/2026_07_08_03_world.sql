-- Necessary Roughness (24502) / Fourth and Goal (28414) footbomb follow-up fixes.

-- =========================================================================
-- ISSUE 1: duplicate boat. The parked prop 48526 and the summoned ride boat
-- 37179 share model 26559 and overlap exactly. A CONTROL_VEHICLE driver is the
-- ride boat's charmer, so Unit::IsAlwaysVisibleFor always shows them the ride
-- boat; the prop must therefore be hidden FROM THE DRIVER to avoid the double.
-- Retail keeps the prop invisible (90366, MOD_INVISIBILITY type 12) and grants
-- questers a see-invisibility (66143) that is dropped on boarding. 66143 is absent
-- from our client Spell.dbc, so we substitute the client-present hidden type-12
-- detect 90161 (MOD_INVISIBILITY_DETECT type 12, value 1000, DO_NOT_DISPLAY).
-- It is self-cast on NR accept (NR carries PLAYER_CAST_ACCEPT 0x100000) for the
-- immediate reveal, and re-applied via spell_area on area re-entry / relog; the
-- boat script (PassengerBoarded) strips it on boarding so only one boat renders.
-- =========================================================================
UPDATE `creature_template_addon` SET `auras`='90366' WHERE `entry`=48526;
UPDATE `creature_addon`          SET `auras`='90366' WHERE `guid`=252174;
UPDATE `quest_template_addon`    SET `SourceSpellID`=90161 WHERE `ID`=24502;

DELETE FROM `spell_area` WHERE `spell`=90161 AND `area`=4822;
INSERT INTO `spell_area`
  (`spell`,`area`,`quest_start`,`quest_end`,`aura_spell`,`racemask`,`gender`,`flags`,`quest_start_status`,`quest_end_status`) VALUES
  (90161, 4822, 24502, 0, 0, 0, 2, 3, 8, 0); -- area Kajaro Field, while NR INCOMPLETE (status bit 3), AUTOCAST+AUTOREMOVE

-- =========================================================================
-- ISSUE 2: movement lock. Both ride boats are ROOTED in retail (CreateObject
-- MovementFlags 1536 = DisableGravity+Root) so the player may only turn/aim, not
-- drive. Add rooted movement templates; Creature::UpdateEntry then applies
-- SetControlled(ROOT) at spawn. Turning, footbomb casting and dismount are
-- unaffected (the seat keeps CAN_CONTROL / CAN_CAST / CAN_ENTER_OR_EXIT).
-- =========================================================================
DELETE FROM `creature_template_movement` WHERE `CreatureId` IN (37179,37213);
INSERT INTO `creature_template_movement` (`CreatureId`,`Rooted`) VALUES (37179,1),(37213,1);

-- =========================================================================
-- ISSUE 3: Fourth and Goal re-entry. Retail summons the kick boat by self-casting
-- 70075 when F&G is accepted, via the quest's SourceSpell (both variants carry
-- PLAYER_CAST_ACCEPT). Our SourceSpellID was 0, so nothing summoned the kick boat
-- (the old OnQuestStatusChange auto-summon was skipped because the player is still
-- seated on the throw boat when NR auto-completes). Set the SourceSpell on the
-- auto-accept variant NR grants (28414) and the dormant manual variant (24503).
-- =========================================================================
UPDATE `quest_template_addon` SET `SourceSpellID`=70075 WHERE `ID` IN (24503,28414);
