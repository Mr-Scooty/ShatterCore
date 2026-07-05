--
-- Death Knight faction handoff — capital-city guards heckle the returning Death Knight.
-- Quests 13188 "Where Kings Walk" (Alliance -> King Varian) / 13189 "Warchief's Blessing"
-- (Horde -> Garrosh).
--
-- Bind the guard-reaction SpellScript to the two "Return to <capital>" spells the DK's ride
-- aura (58530 Stormwind / 58551 Orgrimmar, already granted via spell_area after quest 13165)
-- periodically emits as an AoE. The guard creature_text (groups 2-5) and the spell_area rows
-- are already present in the DB; only the script binding was missing.
--
DELETE FROM `spell_script_names` WHERE `spell_id` IN (58533, 58552) AND `ScriptName` = 'spell_chapter5_return_to_capital';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(58533, 'spell_chapter5_return_to_capital'),
(58552, 'spell_chapter5_return_to_capital');

--
-- Remove a stale, incorrect gate. Five SMART_EVENT conditions (SourceType 22, SourceGroup 1 =
-- smart event id 0) were gating the guards' ordinary out-of-combat wave/salute greeting
-- (event 0 -> action list 6800) behind completion of DK quest 13188/13189 -- so Stormwind and
-- Orgrimmar guards (entries 68/1756/1976/3296/14304) would only greet players who had finished
-- the Death Knight starting zone. The heckle is now driven entirely by the SpellScript above
-- (which does its own guard-entry check), so these conditions are unnecessary and wrong.
--
DELETE FROM `conditions`
  WHERE `SourceTypeOrReferenceId` = 22 AND `SourceGroup` = 1 AND `SourceId` = 0
    AND `SourceEntry` IN (68, 1756, 1976, 3296, 14304)
    AND `ConditionTypeOrReference` = 28 AND `ConditionValue1` IN (13188, 13189);
