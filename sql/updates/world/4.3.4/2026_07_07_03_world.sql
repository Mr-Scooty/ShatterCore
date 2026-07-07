-- The Replacements: sniffed Replacement Parts spawns and static Goblin Trike cleanup.
DELETE FROM `gameobject_template` WHERE `entry` = 201603;
INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `IconName`, `castBarCaption`, `unk1`, `size`, `Data0`, `Data1`, `Data3`, `Data14`, `AIName`, `ScriptName`, `VerifiedBuild`) VALUES
(201603, 3, 9116, 'Replacement Parts', '', 'Collecting', '', 1.5, 1818, 27737, 1, 19676, '', '', 15595);

DELETE FROM `gameobject_template_addon` WHERE `entry` = 201603;
INSERT INTO `gameobject_template_addon` (`entry`, `faction`, `flags`, `mingold`, `maxgold`, `artkit0`, `artkit1`, `artkit2`, `artkit3`, `artkit4`) VALUES
(201603, 0, 4, 0, 0, 0, 0, 0, 0, 0);

DELETE FROM `gameobject_questitem` WHERE `GameObjectEntry` = 201603;
INSERT INTO `gameobject_questitem` (`GameObjectEntry`, `Idx`, `ItemId`, `VerifiedBuild`) VALUES
(201603, 0, 49752, 15595);

DELETE FROM `gameobject_loot_template` WHERE `Entry` = 27737 AND `Item` = 49752;
INSERT INTO `gameobject_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `IsCurrency`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`) VALUES
(27737, 49752, 0, 100, 1, 0, 1, 0, 1, 1, 'Replacement Parts');

SET @OGUID := 9000700;
DELETE FROM `gameobject` WHERE `guid` BETWEEN @OGUID+0 AND @OGUID+6;
DELETE FROM `gameobject`
WHERE `id` = 201603
  AND `map` = 648
  AND (
    (`position_x` BETWEEN -8183.0 AND -8181.0 AND `position_y` BETWEEN 1320.0 AND 1321.0) OR
    (`position_x` BETWEEN -8173.0 AND -8171.0 AND `position_y` BETWEEN 1261.0 AND 1262.0) OR
    (`position_x` BETWEEN -8052.0 AND -8050.0 AND `position_y` BETWEEN 1358.0 AND 1360.0) OR
    (`position_x` BETWEEN -8069.0 AND -8067.0 AND `position_y` BETWEEN 1463.0 AND 1465.0) OR
    (`position_x` BETWEEN -8051.0 AND -8049.0 AND `position_y` BETWEEN 1495.0 AND 1497.0) OR
    (`position_x` BETWEEN -8067.0 AND -8064.0 AND `position_y` BETWEEN 1517.0 AND 1519.0) OR
    (`position_x` BETWEEN -8123.0 AND -8121.0 AND `position_y` BETWEEN 1553.0 AND 1555.0)
  );
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `VerifiedBuild`) VALUES
(@OGUID+0, 201603, 648, 4737, 4765, 1, 0, 1, 169, 0, -1, -8182.295410, 1320.612915, 27.540461, 5.410522, 0, 0, -0.422618, 0.906308, 120, 255, 1, '', 15595),
(@OGUID+1, 201603, 648, 4737, 4765, 1, 0, 1, 169, 0, -1, -8172.399902, 1261.540039, 25.082399, 0.785397, 0, 0, 0.382683, 0.923880, 120, 255, 1, '', 15595),
(@OGUID+2, 201603, 648, 4737, 0, 1, 0, 1, 169, 0, -1, -8051.166504, 1359.067749, 5.333935, 6.003934, 0, 0, -0.139173, 0.990268, 120, 255, 1, '', 15595),
(@OGUID+3, 201603, 648, 4737, 4767, 1, 0, 1, 169, 0, -1, -8068.350098, 1464.130005, 9.477340, 1.361356, 0, 0, 0.629320, 0.777146, 120, 255, 1, '', 15595),
(@OGUID+4, 201603, 648, 4737, 4767, 1, 0, 1, 169, 0, -1, -8050.520020, 1495.859985, 10.088200, 3.351047, 0, 0, -0.994521, 0.104536, 120, 255, 1, '', 15595),
(@OGUID+5, 201603, 648, 4737, 4767, 1, 0, 1, 169, 0, -1, -8065.586914, 1517.956665, 9.112565, 4.049168, 0, 0, -0.898793, 0.438373, 120, 255, 1, '', 15595),
(@OGUID+6, 201603, 648, 4737, 4767, 1, 0, 1, 169, 0, -1, -8122.080078, 1554.290039, 11.031000, 1.186823, 0, 0, 0.559193, 0.829038, 120, 255, 1, '', 15595);

-- Static Goblin Trikes in the sniff are not spellclick vehicles.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 18 AND `SourceGroup` = 49132 AND `SourceEntry` = 46598;
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 49132 AND `spell_id` = 46598;
UPDATE `creature_template` SET `npcflag` = `npcflag` & ~16777216 WHERE `entry` = 49132;
