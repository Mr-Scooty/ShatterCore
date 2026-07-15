-- The Slave Pits (25213): the accept-cast 89164 (Summon Footbomb Uniform, SourceSpellID)
-- summons at TARGET_DEST_NEARBY_ENTRY and had no anchor - the disguise the quest is about
-- was never created. Anchor it on Assistant Greely, the quest giver.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry`=89164;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(13,1,89164,0,0,31,0,3,38124,0,0,0,0,'','The Slave Pits: Summon Footbomb Uniform - dest anchored on Assistant Greely');
