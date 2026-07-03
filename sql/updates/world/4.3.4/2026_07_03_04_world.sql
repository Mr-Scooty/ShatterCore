-- Kezan quest chain: Batch D - the bank heist quartet
-- 14121 Robbing Hoods / 14122 The Great Bank Heist / 14123 Waltz Right In /
-- 14124 Liberate the Kaja'mite

-- ----------------------------------------------------------------------------
-- 1) Slinky Sharpshiv says her villa-cap line on the correct quest (14123
--    Waltz Right In, not 14115).
-- ----------------------------------------------------------------------------
UPDATE `smart_scripts` SET `event_param1` = 14123, `comment` = 'Slinky Sharpshiv - On Quest 14123 Taken - Say Line 0' WHERE `entryorguid` = 34693 AND `source_type` = 0 AND `id` = 3;

-- ----------------------------------------------------------------------------
-- 2) Robbing Hoods: Stolen Loot (47530) only drops for players on the quest.
-- ----------------------------------------------------------------------------
UPDATE `creature_loot_template` SET `QuestRequired` = 1 WHERE `Entry` = 35234 AND `Item` = 47530;

-- ----------------------------------------------------------------------------
-- 3) The Great Bank Heist: the vault minigame vehicle.
--    Native chain: GO 195449 casts 67555 (script) -> player casts 67488 which
--    summons a personal 35486 (vehicle 476) and auto-boards via 67476.
--    Tool bar (spell1-5) is already on the template.
-- ----------------------------------------------------------------------------
UPDATE `creature_template` SET `VehicleId` = 476, `npcflag` = 0, `unit_flags2` = `unit_flags2` | 2048, `ScriptName` = 'npc_first_bank_vault' WHERE `entry` = 35486;
-- The vault is a summoned personal vehicle, not a questgiver (Sassy starts 14122).
DELETE FROM `creature_queststarter` WHERE `id` = 35486 AND `quest` = 14122;

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_kezan_vault_interact', 'spell_kezan_vault_tool', 'spell_kezan_mook_disguise');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(67555, 'spell_kezan_vault_interact'),
(67526, 'spell_kezan_vault_tool'), -- Amazing G-Ray
(67508, 'spell_kezan_vault_tool'), -- Blastcrackers
(67524, 'spell_kezan_vault_tool'), -- Ear-O-Scope
(67525, 'spell_kezan_vault_tool'), -- Infinifold Lockpick
(67522, 'spell_kezan_vault_tool'), -- Kaja'mite Drill
(67435, 'spell_kezan_mook_disguise');

-- Vault minigame texts (whispered to the passenger).
DELETE FROM `creature_text` WHERE `CreatureID` = 35486;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(35486, 0, 0, 'You are breaking into the vault to retrieve your Personal Riches!', 42, 0, 100, 0, 0, 0, 0, 0, 'FBoK Vault - intro 1'),
(35486, 1, 0, 'Use what is called for in your Goblin All-In-1-Der Belt below to crack open the vault!$B|TInterface\\Icons\\INV_Misc_EngGizmos_20.blp:64|t |TInterface\\Icons\\INV_Misc_Bomb_07.blp:64|t |TInterface\\Icons\\INV_Misc_Ear_NightElf_02.blp:64|t |TInterface\\Icons\\INV_Misc_EngGizmos_swissArmy.blp:64|t |TInterface\\Icons\\INV_Weapon_ShortBlade_21.blp:64|t', 42, 0, 100, 0, 0, 0, 0, 0, 'FBoK Vault - intro 2 (belt)'),
(35486, 2, 0, 'The vault will be cracked once the |cFFFF2222Vault Breaking progress bar reaches 100 percent!|r$B|TInterface\\Icons\\INV_Misc_coin_02.blp:64|t$BDoing the wrong thing at the wrong time will reduce the progress of the bar.', 42, 0, 100, 0, 0, 0, 0, 0, 'FBoK Vault - intro 3 (progress bar)'),
(35486, 3, 0, 'Good luck!', 42, 0, 100, 0, 0, 0, 0, 0, 'FBoK Vault - intro 4'),
(35486, 4, 0, 'Use your |cFFFF2222Amazing G-Ray!|r$B|TInterface\\Icons\\INV_Misc_EngGizmos_20.blp:64|t', 42, 0, 100, 0, 0, 0, 0, 0, 'FBoK Vault - prompt G-Ray'),
(35486, 5, 0, 'Use your |cFFFF2222Blastcrackers!|r$B|TInterface\\Icons\\INV_Misc_Bomb_07.blp:64|t', 42, 0, 100, 0, 0, 0, 0, 0, 'FBoK Vault - prompt Blastcrackers'),
(35486, 6, 0, 'Use your |cFFFF2222Ear-O-Scope!|r$B|TInterface\\Icons\\INV_Misc_Ear_NightElf_02.blp:64|t', 42, 0, 100, 0, 0, 0, 0, 0, 'FBoK Vault - prompt Ear-O-Scope'),
(35486, 7, 0, 'Use your |cFFFF2222Infinifold Lockpick!|r$B|TInterface\\Icons\\INV_Misc_EngGizmos_swissArmy.blp:64|t', 42, 0, 100, 0, 0, 0, 0, 0, 'FBoK Vault - prompt Infinifold Lockpick'),
(35486, 8, 0, 'Use your |cFFFF2222Kaja''mite Drill!|r$B|TInterface\\Icons\\INV_Weapon_ShortBlade_21.blp:64|t', 42, 0, 100, 0, 0, 0, 0, 0, 'FBoK Vault - prompt Kaja''mite Drill'),
(35486, 9, 0, 'Correct!', 42, 0, 100, 0, 0, 0, 0, 0, 'FBoK Vault - correct'),
(35486, 10, 0, 'Incorrect!', 42, 0, 100, 0, 0, 0, 0, 0, 'FBoK Vault - incorrect'),
(35486, 11, 0, 'Success! You have your Personal Riches!$B$B|TInterface\\Icons\\INV_Misc_coin_02.blp:64|t', 42, 0, 100, 0, 0, 0, 0, 0, 'FBoK Vault - success');

-- ----------------------------------------------------------------------------
-- 4) Waltz Right In: apply the Mook Disguise (67435) inside Gallywix's Villa
--    (area 4768) while the quest is active. The aura natively forces faction
--    960 (Villa Mooks) friendly and turns the player into vehicle 1362 so the
--    disguise rider can mount (handled by spell_kezan_mook_disguise).
-- ----------------------------------------------------------------------------
-- quest_start_status 10 = INCOMPLETE|COMPLETE: keep the disguise on after the
-- third chest is looted (quest complete) while still inside the villa.
DELETE FROM `spell_area` WHERE `spell` = 67435 AND `area` = 4768;
INSERT INTO `spell_area` (`spell`, `area`, `quest_start`, `quest_end`, `aura_spell`, `racemask`, `gender`, `flags`, `quest_start_status`, `quest_end_status`) VALUES
(67435, 4768, 14123, 14123, 0, 0, 2, 3, 10, 11);

-- ----------------------------------------------------------------------------
-- 5) Liberate the Kaja'mite: lootable Kaja'mite Chunk nodes (GO 195492) in the
--    mine during the heist era (phase 383). Positions from sniff.
-- ----------------------------------------------------------------------------
SET @OGUID := 9000300;
DELETE FROM `gameobject` WHERE `guid` BETWEEN @OGUID+0 AND @OGUID+11;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `VerifiedBuild`) VALUES
(@OGUID+0, 195492, 648, 4737, 4766, 1, 0, 1, 383, 0, -1, -8430.903, 1216.6945, 46.102367, 0.049289, 0, 0, 0.024642, 0.999696, 60, 255, 1, '', 15595),
(@OGUID+1, 195492, 648, 4737, 4766, 1, 0, 1, 383, 0, -1, -8432.795, 1218.118, 45.740704, 3.734984, 0, 0, -0.956305, 0.292372, 60, 255, 1, '', 15595),
(@OGUID+2, 195492, 648, 4737, 4766, 1, 0, 1, 383, 0, -1, -8433.132, 1220.1024, 46.119938, 2.042035, 0, 0, 0.852640, 0.522499, 60, 255, 1, '', 15595),
(@OGUID+3, 195492, 648, 4737, 4766, 1, 0, 1, 383, 0, -1, -8441.6455, 1189.382, 41.544994, 4.784686, 0, 0, -0.681037, 0.732249, 60, 255, 1, '', 15595),
(@OGUID+4, 195492, 648, 4737, 4766, 1, 0, 1, 383, 0, -1, -8447.051, 1190.9653, 41.783413, 3.734984, 0, 0, -0.956305, 0.292372, 60, 255, 1, '', 15595),
(@OGUID+5, 195492, 648, 4737, 4766, 1, 0, 1, 383, 0, -1, -8450.002, 1186.3004, 40.93982, 2.042035, 0, 0, 0.852640, 0.522499, 60, 255, 1, '', 15595),
(@OGUID+6, 195492, 648, 4737, 4766, 1, 0, 1, 383, 0, -1, -8487.564, 1225.9705, 45.39229, 2.568432, 0, 0, 0.959222, 0.282654, 60, 255, 1, '', 15595),
(@OGUID+7, 195492, 648, 4737, 4766, 1, 0, 1, 383, 0, -1, -8489.453, 1226.6927, 45.39229, 3.734984, 0, 0, -0.956305, 0.292372, 60, 255, 1, '', 15595),
(@OGUID+8, 195492, 648, 4737, 4766, 1, 0, 1, 383, 0, -1, -8485.599, 1226.9774, 45.39229, 2.042035, 0, 0, 0.852640, 0.522499, 60, 255, 1, '', 15595),
(@OGUID+9, 195492, 648, 4737, 4766, 1, 0, 1, 383, 0, -1, -8476.79, 1193.3524, 41.9336, 2.042035, 0, 0, 0.852640, 0.522499, 60, 255, 1, '', 15595),
(@OGUID+10, 195492, 648, 4737, 4766, 1, 0, 1, 383, 0, -1, -8480.888, 1199.9688, 41.8409, 3.734984, 0, 0, -0.956305, 0.292372, 60, 255, 1, '', 15595),
(@OGUID+11, 195492, 648, 4737, 4766, 1, 0, 1, 383, 0, -1, -8483.382, 1194.8906, 42.20798, 0, 0, 0, 0, 1, 60, 255, 1, '', 15595);

-- ----------------------------------------------------------------------------
-- 6) Robbing Hoods giver/ender: Megs Dreadshredder needs a heist-era spawn.
-- ----------------------------------------------------------------------------
SET @CGUID := 9000357;
DELETE FROM `creature` WHERE `guid` = @CGUID;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild`) VALUES
(@CGUID, 34874, 648, 4737, 4765, 1, 0, 1, 383, 0, -1, 0, 0, -8435.33, 1316.88, 102.632, 0.959931, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 15595); -- Megs Dreadshredder (heist era)
