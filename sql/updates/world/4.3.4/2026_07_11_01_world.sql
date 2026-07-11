-- Madness of Deathwing follow-up: Blistering Tentacle AoE immunity,
-- retail Congealing Blood control immunities, and corrected Heroic parasite
-- spell bindings.

-- Blistering Tentacles can be attacked directly but are not valid implicit
-- area/chain targets. All difficulty rows carry the new core flags_extra bit.
UPDATE `creature_template`
SET `flags_extra` = `flags_extra` | 65536
WHERE `entry` IN (56188, 57978, 58142, 58143);

UPDATE `creature_template`
SET `AIName` = '', `ScriptName` = 'npc_madness_of_deathwing_blistering_tentacle'
WHERE `entry` = 56188;

-- Congealing Blood can be slowed, but retail did not allow roots, stuns, or
-- knockbacks. mechanic_immune_mask uses (1 << (mechanic - 1)):
-- root = 64, stun = 2048; flags_extra 0x40000000 blocks knockback effects.
UPDATE `creature_template`
SET `mechanic_immune_mask` = `mechanic_immune_mask` | 2112,
    `flags_extra` = `flags_extra` | 1073741824
WHERE `entry` IN (57798, 57980);

-- The inherited data also gave Elementium Terrors the Regenerative Bloods'
-- Degenerative Bite proc (105842). Retail spawn packets contain Tetanus only.
UPDATE `creature_template_addon`
SET `auras` = '106728'
WHERE `entry` IN (56710, 57971, 58127, 58128);

-- 106860 no longer needs its provisional AuraScript: its native 106886
-- energize is complete once the target condition below is present.
DELETE FROM `spell_script_names` WHERE `spell_id` IN (106860, 108601, 108787);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(108601, 'spell_madness_of_deathwing_corrupting_parasite_periodic'),
(108787, 'spell_madness_of_deathwing_parasitic_backlash');

-- Heroic Cauterize's 109045 pulse targets the attackable parasite. The
-- Blistering Tentacle variant (105569) already has its equivalent condition
-- in the base Dragon Soul data.
DELETE FROM `conditions`
WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 109045;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 109045, 0, 0, 31, 0, 3, 57479, 0, 0, 0, 0, '', 'Heroic Cauterize - Target Corrupting Parasite');

DELETE FROM `conditions`
WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 106886;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 106886, 0, 0, 31, 0, 3, 57962, 0, 0, 0, 0, '', 'Phase-two Cauterize - Drain Deathwing Corrupted Blood');

-- The four Aspects' Expose Weakness spells use a NEARBY_ENTRY selector and
-- therefore need the permitted limb entries on every difficulty variant.
DELETE FROM `conditions`
WHERE `SourceTypeOrReferenceId` = 13
  AND `SourceEntry` IN (106588,109582,109583,109584,106600,109619,109620,109621,106613,109637,109638,109639,106624,109728,109729,109730);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13,1,106588,0,0,31,0,3,56167,0,0,0,0,'','Expose Weakness (Alexstrasza) - Arm Tentacle 1'),
(13,1,106588,0,1,31,0,3,56846,0,0,0,0,'','Expose Weakness (Alexstrasza) - Arm Tentacle 2'),
(13,1,106588,0,2,31,0,3,56168,0,0,0,0,'','Expose Weakness (Alexstrasza) - Wing Tentacle'),
(13,1,109582,0,0,31,0,3,56167,0,0,0,0,'','Expose Weakness (Alexstrasza 25N) - Arm Tentacle 1'),
(13,1,109582,0,1,31,0,3,56846,0,0,0,0,'','Expose Weakness (Alexstrasza 25N) - Arm Tentacle 2'),
(13,1,109582,0,2,31,0,3,56168,0,0,0,0,'','Expose Weakness (Alexstrasza 25N) - Wing Tentacle'),
(13,1,109583,0,0,31,0,3,56167,0,0,0,0,'','Expose Weakness (Alexstrasza 10H) - Arm Tentacle 1'),
(13,1,109583,0,1,31,0,3,56846,0,0,0,0,'','Expose Weakness (Alexstrasza 10H) - Arm Tentacle 2'),
(13,1,109583,0,2,31,0,3,56168,0,0,0,0,'','Expose Weakness (Alexstrasza 10H) - Wing Tentacle'),
(13,1,109584,0,0,31,0,3,56167,0,0,0,0,'','Expose Weakness (Alexstrasza 25H) - Arm Tentacle 1'),
(13,1,109584,0,1,31,0,3,56846,0,0,0,0,'','Expose Weakness (Alexstrasza 25H) - Arm Tentacle 2'),
(13,1,109584,0,2,31,0,3,56168,0,0,0,0,'','Expose Weakness (Alexstrasza 25H) - Wing Tentacle'),
(13,1,106600,0,0,31,0,3,56167,0,0,0,0,'','Expose Weakness (Nozdormu) - Arm Tentacle 1'),
(13,1,106600,0,1,31,0,3,56846,0,0,0,0,'','Expose Weakness (Nozdormu) - Arm Tentacle 2'),
(13,1,106600,0,2,31,0,3,56168,0,0,0,0,'','Expose Weakness (Nozdormu) - Wing Tentacle'),
(13,1,109619,0,0,31,0,3,56167,0,0,0,0,'','Expose Weakness (Nozdormu 25N) - Arm Tentacle 1'),
(13,1,109619,0,1,31,0,3,56846,0,0,0,0,'','Expose Weakness (Nozdormu 25N) - Arm Tentacle 2'),
(13,1,109619,0,2,31,0,3,56168,0,0,0,0,'','Expose Weakness (Nozdormu 25N) - Wing Tentacle'),
(13,1,109620,0,0,31,0,3,56167,0,0,0,0,'','Expose Weakness (Nozdormu 10H) - Arm Tentacle 1'),
(13,1,109620,0,1,31,0,3,56846,0,0,0,0,'','Expose Weakness (Nozdormu 10H) - Arm Tentacle 2'),
(13,1,109620,0,2,31,0,3,56168,0,0,0,0,'','Expose Weakness (Nozdormu 10H) - Wing Tentacle'),
(13,1,109621,0,0,31,0,3,56167,0,0,0,0,'','Expose Weakness (Nozdormu 25H) - Arm Tentacle 1'),
(13,1,109621,0,1,31,0,3,56846,0,0,0,0,'','Expose Weakness (Nozdormu 25H) - Arm Tentacle 2'),
(13,1,109621,0,2,31,0,3,56168,0,0,0,0,'','Expose Weakness (Nozdormu 25H) - Wing Tentacle'),
(13,1,106613,0,0,31,0,3,56167,0,0,0,0,'','Expose Weakness (Ysera) - Arm Tentacle 1'),
(13,1,106613,0,1,31,0,3,56846,0,0,0,0,'','Expose Weakness (Ysera) - Arm Tentacle 2'),
(13,1,106613,0,2,31,0,3,56168,0,0,0,0,'','Expose Weakness (Ysera) - Wing Tentacle'),
(13,1,109637,0,0,31,0,3,56167,0,0,0,0,'','Expose Weakness (Ysera 25N) - Arm Tentacle 1'),
(13,1,109637,0,1,31,0,3,56846,0,0,0,0,'','Expose Weakness (Ysera 25N) - Arm Tentacle 2'),
(13,1,109637,0,2,31,0,3,56168,0,0,0,0,'','Expose Weakness (Ysera 25N) - Wing Tentacle'),
(13,1,109638,0,0,31,0,3,56167,0,0,0,0,'','Expose Weakness (Ysera 10H) - Arm Tentacle 1'),
(13,1,109638,0,1,31,0,3,56846,0,0,0,0,'','Expose Weakness (Ysera 10H) - Arm Tentacle 2'),
(13,1,109638,0,2,31,0,3,56168,0,0,0,0,'','Expose Weakness (Ysera 10H) - Wing Tentacle'),
(13,1,109639,0,0,31,0,3,56167,0,0,0,0,'','Expose Weakness (Ysera 25H) - Arm Tentacle 1'),
(13,1,109639,0,1,31,0,3,56846,0,0,0,0,'','Expose Weakness (Ysera 25H) - Arm Tentacle 2'),
(13,1,109639,0,2,31,0,3,56168,0,0,0,0,'','Expose Weakness (Ysera 25H) - Wing Tentacle'),
(13,1,106624,0,0,31,0,3,56167,0,0,0,0,'','Expose Weakness (Kalecgos) - Arm Tentacle 1'),
(13,1,106624,0,1,31,0,3,56846,0,0,0,0,'','Expose Weakness (Kalecgos) - Arm Tentacle 2'),
(13,1,106624,0,2,31,0,3,56168,0,0,0,0,'','Expose Weakness (Kalecgos) - Wing Tentacle'),
(13,1,109728,0,0,31,0,3,56167,0,0,0,0,'','Expose Weakness (Kalecgos 25N) - Arm Tentacle 1'),
(13,1,109728,0,1,31,0,3,56846,0,0,0,0,'','Expose Weakness (Kalecgos 25N) - Arm Tentacle 2'),
(13,1,109728,0,2,31,0,3,56168,0,0,0,0,'','Expose Weakness (Kalecgos 25N) - Wing Tentacle'),
(13,1,109729,0,0,31,0,3,56167,0,0,0,0,'','Expose Weakness (Kalecgos 10H) - Arm Tentacle 1'),
(13,1,109729,0,1,31,0,3,56846,0,0,0,0,'','Expose Weakness (Kalecgos 10H) - Arm Tentacle 2'),
(13,1,109729,0,2,31,0,3,56168,0,0,0,0,'','Expose Weakness (Kalecgos 10H) - Wing Tentacle'),
(13,1,109730,0,0,31,0,3,56167,0,0,0,0,'','Expose Weakness (Kalecgos 25H) - Arm Tentacle 1'),
(13,1,109730,0,1,31,0,3,56846,0,0,0,0,'','Expose Weakness (Kalecgos 25H) - Arm Tentacle 2'),
(13,1,109730,0,2,31,0,3,56168,0,0,0,0,'','Expose Weakness (Kalecgos 25H) - Wing Tentacle');
