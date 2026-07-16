-- Capturing the Unknown (14031), Shipwreck Shore.
--
-- Retail casts 70683 when the quest is accepted. Its server-side behavior
-- grants one invisibility-detection aura for each of the four camera markers.
UPDATE `quest_template_addon`
SET `SourceSpellID`=70683
WHERE `ID`=14031;

DELETE FROM `spell_script_names`
WHERE `spell_id` IN (68279,68280,68296,68349,68936,68937,68943,70683);

INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(68279,'spell_lost_isles_snapflash_ai_cast'),
(68280,'spell_lost_isles_ktc_snapflash'),
(68296,'spell_lost_isles_snapflash_effect'),
(68349,'spell_lost_isles_snapflash_remove_detection'),
(68936,'spell_lost_isles_snapflash_remove_detection'),
(68937,'spell_lost_isles_snapflash_remove_detection'),
(68943,'spell_lost_isles_snapflash_remove_detection'),
(70683,'spell_lost_isles_capturing_unknown_accept');
