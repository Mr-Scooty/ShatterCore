-- Shannox
UPDATE `creature_template` SET `ScriptName` = 'boss_shannox' WHERE `entry` = 53691;

-- Riplimb
UPDATE `creature_template` SET `ScriptName` = 'npc_riplimb' WHERE `entry` = 53694;

-- Rageface
UPDATE `creature_template` SET `ScriptName` = 'npc_rageface' WHERE `entry` = 53695;

-- Spear of Shannox (invisible trigger for mechanics)
UPDATE `creature_template` SET `ScriptName` = 'npc_spear_of_shannox', `modelid1` = 11686, `modelid2` = 0 WHERE `entry` = 53752;

-- Spear of Shannox (visible spear model on the ground - displayid 16925 is the spear)
-- unit_flags: UNIT_FLAG_NON_ATTACKABLE (0x2) | UNIT_FLAG_NOT_SELECTABLE (0x2000000)
UPDATE `creature_template` SET `modelid1` = 16925, `modelid2` = 0, `unit_flags` = 2|33554432 WHERE `entry` = 54112;

-- Spiral Flame (Magma Rupture fire patches - invisible trigger, visual from spell)
UPDATE `creature_template` SET `ScriptName` = 'npc_spiral_flame', `modelid1` = 11686, `modelid2` = 0 WHERE `entry` = 54276;

-- Immolation Trap
UPDATE `creature_template` SET `ScriptName` = 'npc_immolation_trap' WHERE `entry` = 53724;

-- Crystal Prison Trap
UPDATE `creature_template` SET `ScriptName` = 'npc_crystal_prison_trap' WHERE `entry` = 53713;

-- Crystal Prison (the prison that encases players - 2.8 million HP)
UPDATE `creature_template` SET `ScriptName` = 'npc_crystal_prison', `minlevel` = 88, `maxlevel` = 88, `faction` = 14 WHERE `entry` = 53819;

-- Shannox creature_text (voice lines)
DELETE FROM `creature_text` WHERE `CreatureID` = 53691;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
-- SAY_SPAWN (GroupID 0) - patrol/spawn text
(53691, 0, 0, 'Yes, I smell them too, Riplimb. Outsiders encroach on the Firelord''s private grounds. Find their trail. Find them for me, that I may dispense punishment!', 14, 0, 100, 0, 0, 24584, 52456, 0, 'Shannox - SAY_SPAWN'),
-- SAY_AGGRO (GroupID 1)
(53691, 1, 0, 'A-hah! The interlopers! Kill them. EAT THEM!', 14, 0, 100, 0, 0, 24565, 52440, 0, 'Shannox - SAY_AGGRO'),
-- SAY_SLAY (GroupID 2) - multiple kill lines
(53691, 2, 0, 'Now you stay dead!', 14, 0, 100, 0, 0, 24579, 52448, 0, 'Shannox - SAY_SLAY'),
(53691, 2, 1, 'The Firelord will be most pleased.', 14, 0, 100, 0, 0, 24580, 52449, 0, 'Shannox - SAY_SLAY'),
(53691, 2, 2, 'Dog food!', 14, 0, 100, 0, 0, 24578, 52451, 0, 'Shannox - SAY_SLAY'),
(53691, 2, 3, 'Fetch your supper!', 14, 0, 100, 0, 0, 24569, 52452, 0, 'Shannox - SAY_SLAY'),
-- SAY_RIPLIMB_DEAD (GroupID 3)
(53691, 3, 0, 'Riplimb! No... no! Oh, you terrible little beasts! How could you?!', 14, 0, 100, 0, 0, 24574, 52454, 0, 'Shannox - SAY_RIPLIMB_DEAD'),
-- SAY_RAGEFACE_DEAD (GroupID 4)
(53691, 4, 0, 'Oh, you murderers! Why? Why would you kill such a noble animal?', 14, 0, 100, 0, 0, 24575, 52455, 0, 'Shannox - SAY_RAGEFACE_DEAD'),
-- SAY_DEATH (GroupID 5)
(53691, 5, 0, 'Ohh... the pain. Lord of Fire, it hurts....', 14, 0, 100, 0, 0, 24568, 52441, 0, 'Shannox - SAY_DEATH');
