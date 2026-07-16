-- Morale Boost (25122): merge the prisoners' "idea" one-liners into one random group each
-- (they were one-line groups, so the rescue yell could never pick randomly), and give the
-- prisoners SmartAI so the yells can play.
UPDATE `creature_text` SET `ID`=`GroupID`, `GroupID`=0 WHERE `CreatureID` IN (38409,38745) AND `GroupID`<>0;
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry` IN (38409,38745) AND `ScriptName`='';
