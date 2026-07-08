-- Necessary Roughness / Fourth and Goal: keep the ride boat re-enterable.
--
-- Regression: after COMPLETING Necessary Roughness (objectives done, not yet turned
-- in) and dismounting, the parked prop stayed hidden and the boat could not be
-- re-entered. Two causes, both fixed here + in kezan_quests.cpp:
--  * the boat script only restored the see-invisibility (90161) when the quest was
--    INCOMPLETE, not COMPLETE, and only for Necessary Roughness (not Fourth and Goal);
--  * the spell_area that re-grants 90161 on relog/area-entry only covered NR while
--    INCOMPLETE. Widen it to all players in Kajaro Field so the parked prop is visible
--    before Necessary Roughness is accepted; spellclick conditions below keep it
--    non-enterable until the player has an active footbomb quest.
--  * the prop spellclick conditions used CONDITION_QUESTTAKEN (9), which only matches
--    INCOMPLETE in this core. Use CONDITION_QUESTSTATE (47) with the same status mask
--    so the visible prop remains enterable after objectives complete but before turn-in.
DELETE FROM `spell_area` WHERE `spell`=90161 AND `area`=4822;
INSERT INTO `spell_area`
  (`spell`,`area`,`quest_start`,`quest_end`,`aura_spell`,`racemask`,`gender`,`flags`,`quest_start_status`,`quest_end_status`) VALUES
  (90161, 4822, 0, 0, 0, 0, 2, 3, 0, 0); -- Kajaro Field, AUTOCAST+AUTOREMOVE

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=18 AND `SourceGroup`=48526 AND `SourceEntry` IN (70015,70075);
INSERT INTO `conditions`
  (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
  (18, 48526, 70015, 0, 0, 31, 0, 4,     0, 0, 0, 0, 0, '', 'Bilgewater Buccaneer prop: clicker must be a player'),
  (18, 48526, 70015, 0, 0, 47, 0, 24502, 10, 0, 0, 0, 0, '', 'Bilgewater Buccaneer prop: 70015 requires Necessary Roughness incomplete or complete'),
  (18, 48526, 70075, 0, 1, 31, 0, 4,     0, 0, 0, 0, 0, '', 'Bilgewater Buccaneer prop: clicker must be a player'),
  (18, 48526, 70075, 0, 1, 47, 0, 24503, 10, 0, 0, 0, 0, '', 'Bilgewater Buccaneer prop: 70075 requires Fourth and Goal (24503) incomplete or complete'),
  (18, 48526, 70075, 0, 2, 31, 0, 4,     0, 0, 0, 0, 0, '', 'Bilgewater Buccaneer prop: clicker must be a player'),
  (18, 48526, 70075, 0, 2, 47, 0, 28414, 10, 0, 0, 0, 0, '', 'Bilgewater Buccaneer prop: 70075 requires Fourth and Goal (28414) incomplete or complete');
