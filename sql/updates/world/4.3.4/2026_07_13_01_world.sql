-- End Time (map 938) audit fixes + one Well of Eternity credit-routing fix.
-- Findings cross-checked against 4.3.4 DBM (EndTime/*.lua), the 4.3.4 client
-- DBCs and the fork's spell-targeting core.

--
-- Echo of Baine: the totem is the encounter's core pickup - without the
-- spellclick npcflag the client never offers the click (the script strips the
-- flag when the totem lands on Baine, so it expects it present).
--
UPDATE `creature_template` SET `npcflag`=`npcflag`|16777216 WHERE `entry`=54434;

--
-- Echo of Tyrande: the first Eyes of the Goddess pair (102181 summons
-- 54594/54597) had no AI binding - only the repeat pair (54941/54942) was
-- scripted, leaving the first orbit inert. The unused middle pair gets the
-- same AI for safety (102605 -> 54939/54940).
--
UPDATE `creature_template` SET `ScriptName`='npc_echo_of_tyrande_eye_of_elune' WHERE `entry` IN (54594,54597,54939,54940);

--
-- Echo of Tyrande: player-only implicit-target conditions. These four ship
-- with TARGET_CHECK_ENTRY targeting and no conditions - the core then accepts
-- any unit: Tyrande stunned/nuked by her own Moonlance, split lances hitting
-- each other, Stardust hitting all creatures, Tears of Elune targeting
-- everything. Siblings (101401/102491/102542/101842) already had rows.
--
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry` IN (102173,102149,102183,102242);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 102173, 0, 0, 31, 0, 4, 0, 0, 0, 0, 0, '', 'Stardust targets players'),
(13, 3, 102149, 0, 0, 31, 0, 4, 0, 0, 0, 0, 0, '', 'Moonlance contact pulse targets players'),
(13, 3, 102183, 0, 0, 31, 0, 4, 0, 0, 0, 0, 0, '', 'Piercing Gaze of Elune targets players'),
(13, 1, 102242, 0, 0, 31, 0, 4, 0, 0, 0, 0, 0, '', 'Tears of Elune selector targets players');

--
-- Well of Eternity: Queen Azshara's encounter credit (instance_encounters
-- 1273, creditType 1 spell 94981) only reaches the encounter system through
-- spell_gen_dungeon_credit - the spell had no binding, so the encounter and
-- LFG progress never completed. She casts it herself (creature caster), so
-- the generic script applies cleanly.
--
DELETE FROM `spell_script_names` WHERE `spell_id`=94981;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(94981, 'spell_gen_dungeon_credit');
