-- Well of Eternity (map 939) - dungeon framework, courtyard gauntlet, teleporters, achievements
-- Part 1: instance/gauntlet plumbing (boss scripts land in the same update set)

--
-- Script bindings: gauntlet + shared
--
UPDATE `creature_template` SET `ScriptName`='npc_woe_illidan_gauntlet', `npcflag`=`npcflag`|1 WHERE `entry`=55500;
UPDATE `creature_template` SET `ScriptName`='npc_woe_legion_demon_door_guard' WHERE `entry`=55503;
UPDATE `creature_template` SET `ScriptName`='npc_woe_legion_demon_marching' WHERE `entry`=54500;
UPDATE `creature_template` SET `ScriptName`='npc_woe_bronze_drake', `npcflag`=`npcflag`|16777216 WHERE `entry`=57107;
UPDATE `creature_template` SET `ScriptName`='npc_woe_legion_army_doomguard' WHERE `entry`=55700;
UPDATE `gameobject_template` SET `ScriptName`='go_woe_portal_energy_focus' WHERE `entry` IN (209366,209447,209448);
UPDATE `gameobject_template` SET `ScriptName`='go_woe_time_transit_device' WHERE `entry` IN (209997,209998,209999,210000);

--
-- Bronze drake taxi (terrace -> shores)
--
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry`=57107;
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(57107, 93970, 1, 0);

--
-- Time Transit Device network - teleport destinations (TARGET_DEST_DB)
--
DELETE FROM `spell_target_position` WHERE `ID` IN (107686,107687,107688,107689,107690,107691,107934,107979);
INSERT INTO `spell_target_position` (`ID`, `EffectIndex`, `MapID`, `PositionX`, `PositionY`, `PositionZ`, `Orientation`) VALUES
(107934, 0, 939, 3227.02, -4998.09, 194.093, 5.9),   -- Teleport Players - Courtyard Entrance
(107690, 0, 939, 3489.83, -5013.55, 197.617, 4.05),  -- Teleport Players - Azshara's Palace
(107979, 0, 939, 3487.29, -5194.99, 229.949, 4.79),  -- Teleport Players - Azshara's Overlook
(107691, 0, 939, 3062.66, -5561.66, 18.125, 6.05),   -- Teleport Players - Well of Eternity
(107686, 0, 939, 3287.86, -4985.94, 181.16, 5.03),   -- Teleport Players - Courtyard Portal 1
(107687, 0, 939, 3442.87, -4890.34, 181.16, 1.13),   -- Teleport Players - Courtyard Portal 2
(107688, 0, 939, 3474.24, -4856.20, 194.13, 1.90),   -- Teleport Players - Courtyard Portal 3
(107689, 0, 939, 3335.07, -4891.54, 181.16, 5.28);   -- Teleport Players - Courtyard Boss

--
-- Transit device gossip (menu 13326)
--
DELETE FROM `gossip_menu` WHERE `MenuID`=13326;
INSERT INTO `gossip_menu` (`MenuID`, `TextID`) VALUES
(13326, 18848); -- 'Select your destination.' (shared with End Time's device)
DELETE FROM `gossip_menu_option` WHERE `MenuID`=13326;
INSERT INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcflag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES
(13326, 0, 0, 'Teleport to the Courtyard Entrance.', 0, 1, 1, 0, 0, 0, 0, NULL, 0),
(13326, 1, 0, 'Teleport to Azshara''s Palace.', 0, 1, 1, 0, 0, 0, 0, NULL, 0),
(13326, 2, 0, 'Teleport to Azshara''s Overlook.', 0, 1, 1, 0, 0, 0, 0, NULL, 0),
(13326, 3, 0, 'Teleport to the Well of Eternity.', 0, 1, 1, 0, 0, 0, 0, NULL, 0);

--
-- Loot cache spawn groups (manual, spawned by the instance script)
--
DELETE FROM `spawn_group_template` WHERE `groupId` IN (470,471);
INSERT INTO `spawn_group_template` (`groupId`, `groupName`, `groupFlags`) VALUES
(470, 'Well of Eternity - Royal Cache', 4),
(471, 'Well of Eternity - Minor Cache of the Aspects', 4);
DELETE FROM `spawn_group` WHERE `groupId` IN (470,471);
INSERT INTO `spawn_group` (`groupId`, `spawnType`, `spawnId`) VALUES
(470, 1, 224492), -- Royal Cache 210025
(471, 1, 224494); -- Minor Cache of the Aspects 209541

--
-- Achievements: Lazy Eye (6127, criteria 18618) / That's Not Canon! (6070, criteria 18363)
-- type 18 = instance-script check (same wiring as End Time's Severed Ties)
--
DELETE FROM `achievement_criteria_data` WHERE `criteria_id` IN (18618, 18363) AND `type`=18;
INSERT INTO `achievement_criteria_data` (`criteria_id`, `type`, `value1`, `value2`, `ScriptName`) VALUES
(18618, 18, 0, 0, ''),
(18363, 18, 0, 0, '');

--
-- LFG: fix the missing teleport-in position for dungeon 437
--
UPDATE `lfg_dungeon_template` SET `position_x`=3238.55, `position_y`=-4998.39, `position_z`=194.093, `orientation`=2.35619 WHERE `dungeonId`=437;

--
-- Voracious Felhound shore packs are re-seeded from exact sniffed positions
-- in Part 4 (Mannoroth) below.
--

--
-- Palace/approach trash SmartAI (kits from sniffed casts)
--
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry` IN (54747, 54589, 56579, 54645, 54612, 55654, 56078);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (54747, 54589, 56579, 54645, 54612, 55654, 56078) AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
-- Eye of the Legion: Fel Lightning + Fel Flames channel
(54747, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 102361, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Eye of the Legion - IC - Cast Fel Lightning'),
(54747, 0, 1, 0, 0, 0, 100, 0, 10000, 15000, 20000, 25000, 11, 102356, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Eye of the Legion - IC - Cast Fel Flames'),
-- Enchanted Highmistress (both entries): Fireball filler, Frostbolt, Blizzard
(54589, 0, 0, 0, 0, 0, 100, 0, 0, 2500, 3500, 4500, 11, 102265, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Enchanted Highmistress - IC - Cast Fireball'),
(54589, 0, 1, 0, 0, 0, 100, 0, 6000, 9000, 12000, 15000, 11, 102266, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Enchanted Highmistress - IC - Cast Frostbolt'),
(54589, 0, 2, 0, 0, 0, 100, 0, 12000, 16000, 24000, 30000, 11, 102267, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Enchanted Highmistress - IC - Cast Blizzard'),
(56579, 0, 0, 0, 0, 0, 100, 0, 0, 2500, 3500, 4500, 11, 102265, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Enchanted Highmistress - IC - Cast Fireball'),
(56579, 0, 1, 0, 0, 0, 100, 0, 6000, 9000, 12000, 15000, 11, 102266, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Enchanted Highmistress - IC - Cast Frostbolt'),
(56579, 0, 2, 0, 0, 0, 100, 0, 12000, 16000, 24000, 30000, 11, 102267, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Enchanted Highmistress - IC - Cast Blizzard'),
-- Royal Handmaiden: Piercing Thorns, Choking Perfume, Sweet Lullaby + archive quest credit
(54645, 0, 0, 0, 0, 0, 100, 0, 1000, 3000, 4000, 6000, 11, 102239, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Royal Handmaiden - IC - Cast Piercing Thorns'),
(54645, 0, 1, 0, 0, 0, 100, 0, 8000, 12000, 15000, 20000, 11, 102233, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Royal Handmaiden - IC - Cast Choking Perfume'),
(54645, 0, 2, 0, 0, 0, 100, 0, 15000, 20000, 25000, 30000, 11, 102245, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Royal Handmaiden - IC - Cast Sweet Lullaby'),
(54645, 0, 3, 0, 6, 0, 100, 0, 0, 0, 0, 0, 33, 57857, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Royal Handmaiden - On Death - Quest Credit'),
-- Eternal Champion: Sheen of Elune on aggro
(54612, 0, 0, 0, 4, 0, 100, 0, 0, 0, 0, 0, 11, 102258, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Eternal Champion - On Aggro - Cast Sheen of Elune'),
-- Corrupted Arcanist: Arcane Annihilation
(55654, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 9000, 14000, 11, 107865, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Corrupted Arcanist - IC - Cast Arcane Annihilation'),
-- Abyssal Doombringer (far shore): Abyssal Flamethrower ambience
(56078, 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 105218, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Abyssal Doombringer - On Reset - Cast Abyssal Flamethrower');

--
-- Marching Legion Demon quest credit is handled in C++ (npc_woe_legion_demon_marching has a script);
-- give the same credit from the second static Legion Demon entry via its C++ JustDied hook.
--

--
-- Illidan gauntlet escort texts (55500, groups 0-28 match npc_woe_illidan_gauntlet enum)
--
DELETE FROM `creature_text` WHERE `CreatureID`=55500;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(55500, 0, 0, 'Over here, in the shadows.', 12, 0, 100, 0, 0, 26076, 54274, 0, 'Illidan - Intro 1'),
(55500, 1, 0, 'I think we stand a better chance fighting alongside one another.', 12, 0, 100, 0, 0, 26525, 54277, 0, 'Illidan - Intro 2'),
(55500, 2, 0, 'We now hide in shadows, hidden from our enemies.', 12, 0, 100, 0, 0, 26054, 54705, 0, 'Illidan - Cloak'),
(55500, 3, 0, 'Come with me if you''d like to live long enough to see me save this world!', 12, 0, 100, 0, 0, 26065, 54284, 0, 'Illidan - Escort start'),
(55500, 4, 0, 'I''ve seen a single Guardian Demon slaughter a hundred elves.  Tread lightly.', 12, 0, 100, 0, 0, 26068, 54296, 0, 'Illidan - Guardian warning'),
(55500, 5, 0, 'I will hold them back so we can get past. Be ready.', 12, 0, 100, 0, 0, 26063, 54297, 0, 'Illidan - Wall of Shadow'),
(55500, 6, 0, 'My magic is fading. I''m going through!', 12, 0, 100, 0, 0, 26064, 54299, 0, 'Illidan - Magic fading'),
(55500, 7, 0, 'Attack. I don''t like to be kept waiting.', 12, 0, 100, 0, 0, 26081, 54304, 0, 'Illidan - Attack'),
(55500, 8, 0, 'Death to the Legion!', 12, 0, 100, 0, 0, 26056, 54444, 0, 'Illidan - Death to the Legion'),
(55500, 9, 0, 'Destroy the portal energy focus!', 12, 0, 100, 0, 0, 26105, 54710, 0, 'Illidan - Destroy focus'),
(55500, 10, 0, 'We''re leaving. Stay close.', 12, 0, 100, 0, 0, 26055, 54511, 0, 'Illidan - Leaving'),
(55500, 11, 0, 'They come endlessly from the palace.', 12, 0, 100, 0, 0, 26069, 54510, 0, 'Illidan - Endless'),
(55500, 12, 0, 'I''ll let you have the first kill.  Don''t make me regret that.', 12, 0, 100, 0, 0, 26082, 54308, 0, 'Illidan - First kill'),
(55500, 13, 0, 'Smash the crystal. We need to move.', 12, 0, 100, 0, 0, 26104, 54709, 0, 'Illidan - Smash crystal'),
(55500, 14, 0, 'The stench of sulfur and brimstone... These portals are as foul as the demons themselves.', 12, 0, 100, 0, 0, 26070, 54512, 0, 'Illidan - Stench'),
(55500, 15, 0, 'Cut this one down from the shadows.', 12, 0, 100, 0, 0, 26071, 54514, 0, 'Illidan - Cut down'),
(55500, 16, 0, 'Let us shut down this final portal and finish this.', 12, 0, 100, 0, 0, 26072, 54736, 0, 'Illidan - Final portal'),
(55500, 17, 0, 'Destroy the crystal so we can move on.', 12, 0, 100, 0, 0, 26103, 54468, 0, 'Illidan - Destroy crystal 3'),
(55500, 18, 0, 'The demons should all be leaving. We will be at the palace in no time.', 12, 0, 100, 0, 0, 26073, 54750, 0, 'Illidan - Demons leaving'),
(55500, 19, 0, 'The demons are no longer pouring from the palace. We can move ahead.', 12, 0, 100, 0, 0, 26074, 54572, 0, 'Illidan - No longer pouring'),
(55500, 20, 0, 'Too easy.', 12, 0, 100, 0, 0, 26075, 54573, 0, 'Illidan - Too easy'),
(55500, 21, 0, 'Another demon, ready to be slaughtered.', 12, 0, 100, 0, 0, 26050, 54574, 0, 'Illidan - Another demon'),
(55500, 22, 0, 'Nothing will stop me. Not even you, demon.', 12, 0, 100, 0, 0, 26049, 54829, 0, 'Illidan - Nothing stops'),
(55500, 23, 0, 'Your magic is pathetic. Let me show you mine.', 12, 0, 100, 0, 0, 26053, 54775, 0, 'Illidan - Drain counter'),
(55500, 24, 0, 'Return to the shadows!', 12, 0, 100, 0, 0, 26048, 54831, 0, 'Illidan - Return to shadows'),
(55500, 25, 0, 'My strength returns.', 12, 0, 100, 0, 0, 26102, 54832, 0, 'Illidan - Strength returns'),
(55500, 26, 0, 'The hunter became the prey.', 12, 0, 100, 0, 0, 26051, 54838, 0, 'Illidan - Hunter prey'),
(55500, 27, 0, 'You did well, but for now I must continue alone.  Good hunting.', 12, 0, 100, 0, 0, 26052, 54839, 0, 'Illidan - Farewell'),
(55500, 28, 0, 'Waiting to attack...', 12, 0, 100, 0, 0, 26100, 54302, 0, 'Illidan - Waiting');

--
-- Guardian Demon portal-jump yells (54927, used by SmartAI-free ambient RP; kept for reference)
--
DELETE FROM `creature_text` WHERE `CreatureID`=54927;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(54927, 0, 0, 'There! Go! GO!!!', 14, 0, 100, 0, 0, 26038, 53405, 0, 'Guardian Demon - Portal jump'),
(54927, 1, 0, 'The portal is closing! Hurry!', 14, 0, 100, 0, 0, 26037, 53406, 0, 'Guardian Demon - Portal closing');

--
-- Bronze drake flight text (57107)
--
DELETE FROM `creature_text` WHERE `CreatureID`=57107;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(57107, 0, 0, 'Heroes! We have been sent by Nozdormu! Quickly, on our backs so that we may carry you to the Well!', 14, 0, 100, 0, 0, 26032, 55914, 0, 'Bronze Drake - Heroes');

-- ====================================================================
-- Part 2: Peroth'arn encounter
-- ====================================================================
-- Well of Eternity (map 939): Peroth'arn encounter (boss_perotharn.cpp)
-- Verified against 11.2.5 sniff (original 4.3.4 spell IDs), 4.3.4 DBM timers and Spell.dbc decodes.

-- 1. Creature script bindings
UPDATE `creature_template` SET `ScriptName`='boss_perotharn' WHERE `entry`=55085;
UPDATE `creature_template` SET `ScriptName`='npc_eye_of_perotharn' WHERE `entry` IN (55868,55879);
UPDATE `creature_template` SET `ScriptName`='npc_perotharn_fel_flames' WHERE `entry`=57329;
UPDATE `creature_template` SET `ScriptName`='npc_perotharn_hunting_stalker' WHERE `entry`=56248;

-- 2. Spell script bindings (108124 Fel Decay Heal Aura -> heal-reflect driver)
DELETE FROM `spell_script_names` WHERE `ScriptName`='spell_perotharn_fel_decay_heal';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(108124, 'spell_perotharn_fel_decay_heal');

-- 3. Proc data: 108124 is a plain dummy aura on the Fel Decay victim - it must proc
--    on heals TAKEN (direct + periodic). Same shape as Baleroc's Torment heal procs.
--    ProcFlags 0x88800 = TAKE_HELPFUL_SPELL | TAKE_HELPFUL_ABILITY | TAKE_PERIODIC, SpellTypeMask 0x2 = heals.
DELETE FROM `spell_proc` WHERE `SpellId`=108124;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(108124, 0, 0, 0, 0, 0, 0x88800, 0x2, 0x2, 0, 0, 0, 0, 100, 0, 0);

-- 4. Spawn cleanup
--    Peroth'arn has two DB spawns (entrance ledge + palace stair top). The script owns the
--    stair-top appearance (NearTeleportTo on ACTION_PEROTHARN_INTRO) - keep only the ledge spawn.
--    Two spawns would also break the instance ObjectData accessor (BOSS_PEROTHARN).
DELETE FROM `creature` WHERE `guid`=358839 AND `id`=55085;
--    The Hunting Summon Stalker (ring center raid-emote owner) is summoned by the boss script
--    at each hide phase - remove the static spawn.
DELETE FROM `creature` WHERE `guid`=358857 AND `id`=56248;

-- 5. Template tweaks
--    Fel Flames Stalker ships as faction 35 (friendly) - its 1 s ground pulse (108214 -> 108217,
--    TARGET_UNIT_SRC_AREA_ENEMY) could never hit players. Match the boss's hostile faction.
UPDATE `creature_template` SET `faction`=14 WHERE `entry`=57329;

-- NOTE (no change needed): creature_template_addon 55085 already carries aura 104939
--      (Corrupting Touch proc enabler); the script re-applies it after each Camouflage End.
-- NOTE (no change needed): 55868/55879 unit flags (NON_ATTACKABLE | NOT_SELECTABLE) and
--      stealth detection 93105 are applied by npc_eye_of_perotharn at spawn.
-- NOTE: 104939 (Corrupting Touch enabler, PROC_TRIGGER_SPELL -> 108101) needs NO spell_proc row -
--      the script casts the 108101 bolt on a fixed 2 s combat event instead (sniffed cadence).

-- 6. Creature texts (exact sniffed strings/sounds via broadcast_text; groups match the Texts enum)
DELETE FROM `creature_text` WHERE `CreatureID` IN (55085,56248);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 55085, 0, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 3, 'Perotharn - SAY_LEDGE_SENSE' FROM `broadcast_text` WHERE `ID`=54683;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 55085, 1, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 3, 'Perotharn - SAY_LEDGE_FELGUARD' FROM `broadcast_text` WHERE `ID`=54685;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 55085, 2, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 3, 'Perotharn - SAY_LEDGE_COURTYARD' FROM `broadcast_text` WHERE `ID`=54687;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 55085, 3, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 3, 'Perotharn - SAY_INTRO' FROM `broadcast_text` WHERE `ID`=54833;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 55085, 4, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 3, 'Perotharn - SAY_ARRIVAL' FROM `broadcast_text` WHERE `ID`=54834;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 55085, 5, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Perotharn - SAY_AGGRO' FROM `broadcast_text` WHERE `ID`=54778;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 55085, 6, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Perotharn - SAY_DRAIN_ESSENCE' FROM `broadcast_text` WHERE `ID`=54546;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 55085, 7, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Perotharn - SAY_SHADOWS' FROM `broadcast_text` WHERE `ID`=54816;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 55085, 8, 0, IF(`Text`='', `Text1`, `Text`), 41, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Perotharn - EMOTE_VANISH' FROM `broadcast_text` WHERE `ID`=55983;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 55085, 9, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Perotharn - SAY_COWER' FROM `broadcast_text` WHERE `ID`=54819;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 55085, 10, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Perotharn - SAY_SPOTTED' FROM `broadcast_text` WHERE `ID`=54790;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 55085, 11, 0, IF(`Text`='', `Text1`, `Text`), 41, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Perotharn - EMOTE_AMBUSH' FROM `broadcast_text` WHERE `ID`=56047;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 55085, 12, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Perotharn - SAY_FRENZY' FROM `broadcast_text` WHERE `ID`=54812;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 55085, 13, 0, IF(`Text`='', `Text1`, `Text`), 12, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Perotharn - SAY_DEATH' FROM `broadcast_text` WHERE `ID`=56991;

INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 56248, 0, 0, IF(`Text`='', `Text1`, `Text`), 41, 0, 100, 0, 0, `SoundEntriesID`, `ID`, 0, 'Hunting Summon Stalker - EMOTE_EYES_SEARCHING' FROM `broadcast_text` WHERE `ID`=55984;

-- ====================================================================
-- Part 3: Queen Azshara encounter
-- ====================================================================
-- ============================================================================
-- Well of Eternity (map 939) - Queen Azshara encounter
-- Companion SQL for src/server/scripts/Kalimdor/CavernsOfTime/WellOfEternity/boss_queen_azshara.cpp
-- Texts/sounds/BroadcastTextIds/emotes sniffed (11.2.5 run, IDs verified against 4.3.4 DBC).
-- ============================================================================

-- ----------------------------------------------------------------------------
-- ScriptNames
-- ----------------------------------------------------------------------------
UPDATE `creature_template` SET `ScriptName`='boss_queen_azshara' WHERE `entry`=54853;
UPDATE `creature_template` SET `ScriptName`='npc_enchanted_magus' WHERE `entry` IN (54882,54883,54884);
UPDATE `creature_template` SET `ScriptName`='npc_hand_of_the_queen' WHERE `entry`=54728;
UPDATE `creature_template` SET `ScriptName`='npc_azshara_arcane_bomb' WHERE `entry` IN (54864,54865);

-- ----------------------------------------------------------------------------
-- Spell scripts
-- ----------------------------------------------------------------------------
DELETE FROM `spell_script_names` WHERE `ScriptName`='spell_azshara_servant_of_the_queen';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(102334, 'spell_azshara_servant_of_the_queen');

-- ----------------------------------------------------------------------------
-- 94981 Azshara Event Credit ships in the client Spell.dbc/SpellEffect.dbc - no serverside rows needed.
-- KILL_CREDIT (effect 90) misc 51314 at all players (targets 22/7), per DBC decode.
-- instance_encounters row 1273 (creditType 1, creditEntry 94981) already exists.
-- ----------------------------------------------------------------------------

-- ----------------------------------------------------------------------------
-- Creature texts - Queen Azshara 54853
-- Groups: 0/1 intro RP, 2 magus-death flavor (random of 2), 3 "Serve Azshara" (Servant + Total Obedience),
--         4 Total Obedience raid-warning (type 41), 5 interrupt, 6 defeat (DBM kill trigger - exact string!),
--         7 "Riders, to me!", 8 Varo'then farewell (type 12 say).
-- ----------------------------------------------------------------------------
DELETE FROM `creature_text` WHERE `CreatureID`=54853;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(54853, 0, 0, 'Ah, welcome. You are here to join us in the coming celebration? No? A pity.', 14, 0, 100, 274, 0, 26013, 55919, 0, 'Queen Azshara - SAY_INTRO_1'),
(54853, 1, 0, 'I have no time for such diversions. Keepers of Eternity, will you stand for your queen?', 14, 0, 100, 274, 0, 26027, 53300, 0, 'Queen Azshara - SAY_INTRO_2'),
(54853, 2, 0, 'Still these strangers would oppose your queen''s will. Who will stop them?', 14, 0, 100, 1, 0, 26028, 53301, 0, 'Queen Azshara - SAY_MAGUS_DEATH'),
(54853, 2, 1, 'I beseech of you, my beloved subjects: Put an end to these miscreants.', 14, 0, 100, 5, 0, 26029, 53299, 0, 'Queen Azshara - SAY_MAGUS_DEATH'),
(54853, 3, 0, 'Serve Azshara, puppets, and rejoice.', 14, 0, 100, 273, 0, 26026, 55920, 0, 'Queen Azshara - SAY_PUPPETS'),
(54853, 4, 0, '%s begins to transform everybody into puppets! You must interrupt her!', 41, 0, 100, 0, 0, 0, 0, 0, 'Queen Azshara - EMOTE_TOTAL_OBEDIENCE'),
(54853, 5, 0, 'Bold of you, to strike a queen. A lesser monarch might be enraged.', 14, 0, 100, 5, 0, 26014, 53305, 0, 'Queen Azshara - SAY_INTERRUPT'),
(54853, 6, 0, 'Enough! As much as I adore playing hostess, I have more pressing matters to attend to.', 14, 0, 100, 5, 0, 26017, 53280, 0, 'Queen Azshara - SAY_DEFEAT (DBM kill detection)'),
(54853, 7, 0, 'Riders, to me!', 14, 0, 100, 25, 0, 26018, 53281, 0, 'Queen Azshara - SAY_RIDERS'),
(54853, 8, 0, 'My noble Varo''then, do return and dispose of this murderous band.', 12, 0, 100, 25, 0, 26019, 55910, 0, 'Queen Azshara - SAY_VAROTHEN_DISPOSE');

-- ----------------------------------------------------------------------------
-- Creature texts - Enchanted Magus 54882/54883/54884
-- Groups 0-2 = first/second/third activation prayer (any school can be any slot,
-- so all three lines live on every entry; the boss passes the group via SetData).
-- ----------------------------------------------------------------------------
DELETE FROM `creature_text` WHERE `CreatureID` IN (54882,54883,54884);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(54882, 0, 0, 'I pray that the Light of a Thousand Moons will grant me this honor.', 14, 0, 100, 0, 0, 26042, 53285, 0, 'Enchanted Magus - SAY_PRAYER_1'),
(54882, 1, 0, 'Yes, Light of Lights! My life is yours!', 14, 0, 100, 0, 0, 26513, 53286, 0, 'Enchanted Magus - SAY_PRAYER_2'),
(54882, 2, 0, 'The Flower of Life calls upon me. I WILL NOT fail you, my Queen.', 14, 0, 100, 0, 0, 26047, 53287, 0, 'Enchanted Magus - SAY_PRAYER_3'),
(54883, 0, 0, 'I pray that the Light of a Thousand Moons will grant me this honor.', 14, 0, 100, 0, 0, 26042, 53285, 0, 'Enchanted Magus - SAY_PRAYER_1'),
(54883, 1, 0, 'Yes, Light of Lights! My life is yours!', 14, 0, 100, 0, 0, 26513, 53286, 0, 'Enchanted Magus - SAY_PRAYER_2'),
(54883, 2, 0, 'The Flower of Life calls upon me. I WILL NOT fail you, my Queen.', 14, 0, 100, 0, 0, 26047, 53287, 0, 'Enchanted Magus - SAY_PRAYER_3'),
(54884, 0, 0, 'I pray that the Light of a Thousand Moons will grant me this honor.', 14, 0, 100, 0, 0, 26042, 53285, 0, 'Enchanted Magus - SAY_PRAYER_1'),
(54884, 1, 0, 'Yes, Light of Lights! My life is yours!', 14, 0, 100, 0, 0, 26513, 53286, 0, 'Enchanted Magus - SAY_PRAYER_2'),
(54884, 2, 0, 'The Flower of Life calls upon me. I WILL NOT fail you, my Queen.', 14, 0, 100, 0, 0, 26047, 53287, 0, 'Enchanted Magus - SAY_PRAYER_3');

-- ----------------------------------------------------------------------------
-- Creature texts - Captain Varo'then bat cameo 57117 (departure RP)
-- ----------------------------------------------------------------------------
DELETE FROM `creature_text` WHERE `CreatureID`=57117;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(57117, 0, 0, 'At your side, my queen!', 14, 0, 100, 0, 0, 26136, 55912, 0, 'Captain Varo''then (cameo) - SAY_AT_YOUR_SIDE');

-- ============================================================================
-- TEMPLATE / DATA NOTES (not applied here - flag for review or walkthrough):
--
-- 1. 54728 Hand of the Queen: faction is already 14 (OK). HealthModifier is
--    currently 1 (~43k @85) - retail heroic is ~150k; suggest HealthModifier
--    ~3.5, tune during the walkthrough.
-- 2. 87235 "Dummy Nuke" (Azshara filler, sniffed at 1.22 s cadence): generic
--    DBC name - verify during walkthrough that it deals damage; if the DBC
--    entry is a no-op, replace with a scripted nuke or spell_dbc override.
-- 3. 57117 creature_template.name is "Shadowbat"; the sniff object_names call
--    it "Captain Varo'then" (bat-riding vehicle version, VehicleId 1919).
--    Consider renaming so the departure RP yell shows the right speaker.
-- 4. 54864/54865 "Hammer of Divinity" orbs have unit_flags 0 in the template;
--    the script sets NON_ATTACKABLE|NOT_SELECTABLE on summon, no DB change
--    strictly required.
-- 5. 54853 creature loot (lootid 54853) is redundant on retail: Azshara is
--    never killed - loot comes from the Royal Cache GO 210025 (summoned by the
--    INSTANCE script on boss state DONE, gameobject loot 40353).
-- 6. instance_well_of_eternity should register { NPC_QUEEN_AZSHARA,
--    BOSS_QUEEN_AZSHARA } in its creature ObjectData so
--    instance->GetCreature(BOSS_QUEEN_AZSHARA) resolves (the boss/magus
--    scripts fall back to a grid scan if it does not).
-- 7. 54853 mechanic_immune_mask must NOT include MECHANIC_INTERRUPT - Total
--    Obedience (103241) must remain interruptible. The script also toggles
--    Creature::MakeInterruptable around the cast as a safety.
-- 8. creature_template_addon for 54882/54883/54884 already carries the idle
--    channel visuals (110494/110492/110495); the script re-applies them on
--    reset, so no addon change is needed.
-- ============================================================================

-- Template tweaks from the Azshara review:
UPDATE `creature_template` SET `name`='Captain Varo''then' WHERE `entry`=57117;
UPDATE `creature_template` SET `HealthModifier`=3.5 WHERE `entry`=54728; -- Hand of the Queen ~150k, tune in walkthrough

-- ====================================================================
-- Part 4: Mannoroth & Captain Varo'then finale
-- ====================================================================
-- Well of Eternity (map 939): Mannoroth & Captain Varo'then finale
-- (boss_mannoroth_and_varothen.cpp)
-- Sources: 11.2.5 retail sniff (original 4.3.4 IDs), 4.3.4 DBM Mannoroth.lua,
-- 4.3.4 Spell.dbc decodes. All statements idempotent (DELETE+INSERT / UPDATE).

-- ============================================================================
-- 1. Creature script bindings
-- ============================================================================
UPDATE `creature_template` SET `ScriptName`='boss_mannoroth' WHERE `entry`=54969;
UPDATE `creature_template` SET `ScriptName`='boss_captain_varothen' WHERE `entry`=55419;
UPDATE `creature_template` SET `ScriptName`='npc_woe_illidan_finale' WHERE `entry`=55532;
UPDATE `creature_template` SET `ScriptName`='npc_woe_tyrande' WHERE `entry`=55524;
UPDATE `creature_template` SET `ScriptName`='npc_woe_malfurion' WHERE `entry`=55570;
UPDATE `creature_template` SET `ScriptName`='npc_varothens_magical_blade' WHERE `entry`=55837;
UPDATE `creature_template` SET `ScriptName`='npc_woe_embedded_blade' WHERE `entry`=55838;
UPDATE `creature_template` SET `ScriptName`='npc_dreadlord_debilitator' WHERE `entry`=55762;
-- One AI, behavior keyed by entry: Devastator / Felhound / Felguard / Infernal
UPDATE `creature_template` SET `ScriptName`='npc_woe_portal_demon' WHERE `entry` IN (55739,56001,56002,56036);
-- Fel Firestorm ground fire (55502 is shared with Peroth'arn's kit; behavior
-- there is identical - 103892 pulse + timed despawn)
UPDATE `creature_template` SET `ScriptName`='npc_woe_fel_flames' WHERE `entry`=55502;

-- ============================================================================
-- 2. Varo'then's Magical Blade: spellclick pickup (sniff: 55837 -> 104818)
-- ============================================================================
UPDATE `creature_template` SET `npcflag`=`npcflag`|16777216 WHERE `entry`=55837; -- UNIT_NPC_FLAG_SPELLCLICK
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry`=55837;
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(55837, 104818, 1, 0); -- caster = player (hurls the blade at Mannoroth)

-- ============================================================================
-- 3. Mannoroth vehicle accessories (sniff: VehicleId 584 already on template)
--    Seats 1-6 = 55839 Mannoroth Cosmetic Strike Point.
--    Seat 0 (55838 Embedded Blade) is deliberately NOT an accessory - the
--    blade boards mid-fight when the sword is thrown (script-summoned).
-- ============================================================================
DELETE FROM `vehicle_template_accessory` WHERE `entry`=54969;
INSERT INTO `vehicle_template_accessory` (`entry`, `accessory_entry`, `seat_id`, `minion`, `description`, `summontype`, `summontimer`) VALUES
(54969, 55839, 1, 1, 'Mannoroth - Cosmetic Strike Point', 8, 0),
(54969, 55839, 2, 1, 'Mannoroth - Cosmetic Strike Point', 8, 0),
(54969, 55839, 3, 1, 'Mannoroth - Cosmetic Strike Point', 8, 0),
(54969, 55839, 4, 1, 'Mannoroth - Cosmetic Strike Point', 8, 0),
(54969, 55839, 5, 1, 'Mannoroth - Cosmetic Strike Point', 8, 0),
(54969, 55839, 6, 1, 'Mannoroth - Cosmetic Strike Point', 8, 0);

-- ============================================================================
-- 4. Spell script bindings
-- ============================================================================
DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_woe_blessing_of_elune','spell_mannoroth_magistrike_arc');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(103918, 'spell_woe_blessing_of_elune'),    -- proc handler: attacks vs lesser demons -> 103919
(105524, 'spell_mannoroth_magistrike_arc'); -- deterministic 105523 victim-arc + 104822 blade pulse

-- ============================================================================
-- 5. Proc data: 103918 Blessing of Elune is a plain PROC_TRIGGER_SPELL aura -
--    it must proc on any damage DONE (melee/ranged/spell/periodic); the aura
--    script filters for the lesser-demon entries.
--    ProcFlags 0x51154 = DONE_MELEE_AUTO | DONE_SPELL_MELEE | DONE_RANGED_AUTO
--    | DONE_SPELL_RANGED | DONE_SPELL_NEG | DONE_SPELL_MAGIC_NEG | DONE_PERIODIC.
-- ============================================================================
DELETE FROM `spell_proc` WHERE `SpellId`=103918;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(103918, 0, 0, 0, 0, 0, 0x51154, 0x1, 0x2, 0, 0, 0, 0, 100, 0, 0);

-- ============================================================================
-- 6. Implicit-target conditions. The encounter's area/nearby-entry spells ship
--    with TARGET_CHECK_ENTRY targeting and rely on serverside restrictions:
--    without these rows the NEARBY_ENTRY casts whiff and the SRC_AREA_ENTRY
--    pulses hit every unit in range.
--    SourceType 13 = SPELL_IMPLICIT_TARGET, SourceGroup = effect mask,
--    ConditionType 31 = OBJECT_ENTRY_GUID (Value1: 3 = unit, 4 = player).
-- ============================================================================
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry` IN (104961,105523,104822,104678,105041,104817,104818,104625,104648,103918,103954,105009,105576,109546,105093,104688,105073,105075,104387);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
-- nearby-entry casts
(13, 1, 104961, 0, 0, 31, 0, 3, 55419, 0, 0, 0, 0, '', 'Fel Drain - instakill Captain Varo''then'),
(13, 1, 105523, 0, 0, 31, 0, 3, 54969, 0, 0, 0, 0, '', 'Magistrike Arc - victim arcs 1,000,000 into Mannoroth'),
(13, 1, 104822, 0, 0, 31, 0, 3, 54969, 0, 0, 0, 0, '', 'Magistrike Arc - Embedded Blade pulse hits Mannoroth'),
(13, 1, 104678, 0, 0, 31, 0, 3, 55524, 0, 0, 0, 0, '', 'Debilitating Flay - channel on Tyrande'),
(13, 1, 105041, 0, 0, 31, 0, 3, 54020, 0, 0, 0, 0, '', 'Nether Tear - portal opener at GP Bunny (flying, huge)'),
(13, 1, 104817, 0, 0, 31, 0, 3, 54969, 0, 0, 0, 0, '', 'Varo''then''s Magical Blade - hurl script hits Mannoroth'),
(13, 1, 104818, 0, 0, 31, 0, 3, 54969, 0, 0, 0, 0, '', 'Varo''then''s Magical Blade - pickup missile at Mannoroth'),
-- src-area-entry cosmetics at the GP bunnies
(13, 1, 104625, 0, 0, 31, 0, 3, 45979, 0, 0, 0, 0, '', 'Nether Portal - cosmetic missiles at GP bunnies'),
(13, 1, 104648, 0, 0, 31, 0, 3, 45979, 0, 0, 0, 0, '', 'Nether Portal - dummy aura on GP bunnies'),
-- src-area-entry buffs restricted to players
(13, 1, 103918, 0, 0, 31, 0, 4, 0, 0, 0, 0, 0, '', 'Blessing of Elune - players only'),
(13, 3, 103954, 0, 0, 31, 0, 4, 0, 0, 0, 0, 0, '', 'Waters of Eternity zone - players only'),
(13, 7, 105009, 0, 0, 31, 0, 4, 0, 0, 0, 0, 0, '', 'Gift of Sargeras - players only'),
(13, 1, 105576, 0, 0, 31, 0, 4, 0, 0, 0, 0, 0, '', 'Mannoroth Achievement Spell - players only'),
(13, 1, 109546, 0, 0, 31, 0, 4, 0, 0, 0, 0, 0, '', 'Hand of Elune farewell - players only'),
-- src-area-entry damage restricted to its intended victims
(13, 1, 105093, 0, 0, 31, 0, 4, 0, 0, 0, 0, 0, '', 'Fel Fire Nova - players only (allies are not toasted)'),
(13, 1, 104688, 0, 0, 31, 0, 3, 55739, 0, 0, 0, 0, '', 'Lunar Shot AoE - Doomguard Devastators'),
(13, 1, 105073, 0, 0, 31, 0, 3, 55739, 0, 0, 0, 0, '', 'Wrath of Elune - Devastators'),
(13, 1, 105073, 0, 1, 31, 0, 3, 56001, 0, 0, 0, 0, '', 'Wrath of Elune - Felhounds'),
(13, 1, 105073, 0, 2, 31, 0, 3, 56002, 0, 0, 0, 0, '', 'Wrath of Elune - Felguard'),
(13, 1, 105073, 0, 3, 31, 0, 3, 56036, 0, 0, 0, 0, '', 'Wrath of Elune - Infernals'),
(13, 1, 105075, 0, 0, 31, 0, 3, 55739, 0, 0, 0, 0, '', 'Wrath of Elune (final) - Devastators'),
(13, 1, 105075, 0, 1, 31, 0, 3, 56001, 0, 0, 0, 0, '', 'Wrath of Elune (final) - Felhounds'),
(13, 1, 105075, 0, 2, 31, 0, 3, 56002, 0, 0, 0, 0, '', 'Wrath of Elune (final) - Felguard'),
(13, 1, 105075, 0, 3, 31, 0, 3, 56036, 0, 0, 0, 0, '', 'Wrath of Elune (final) - Infernals'),
(13, 1, 104387, 0, 0, 31, 0, 3, 56073, 0, 0, 0, 0, '', 'Aura of Immolation - Voracious Felhounds'),
(13, 1, 104387, 0, 1, 31, 0, 3, 55519, 0, 0, 0, 0, '', 'Aura of Immolation - Doomguard Annihilators'),
(13, 1, 104387, 0, 2, 31, 0, 3, 55426, 0, 0, 0, 0, '', 'Aura of Immolation - Highguard Elite'),
(13, 1, 104387, 0, 3, 31, 0, 3, 55453, 0, 0, 0, 0, '', 'Aura of Immolation - Shadowbat vehicles'),
(13, 1, 104387, 0, 4, 31, 0, 3, 55465, 0, 0, 0, 0, '', 'Aura of Immolation - Shadowbat mirrors');

-- ====
-- (sections 7/8 omitted: achievement criteria 18363 and spawn group 471 are
--  seeded in Part 1 of this update, reusing existing GO spawn 224494)
-- ====
-- 9. Spawn cleanup: the Devastator stream (125x in the sniff) and both
--    Dreadlord Debilitators are event summons owned by the boss script - the
--    TDB anchor spawns would stand around idle (and break the flay cycle by
--    channeling out of turn).
-- ============================================================================
DELETE FROM `creature` WHERE `id` IN (55739,55762) AND `map`=939;

-- ============================================================================
-- 10. Voracious Felhound population: the sniff shows 24 persistent pack
--     spawns along the shore, TDB ships only 2. Re-seed the full set
--     (positions from CreateObject1 blocks; guid block 9000860+).
-- ============================================================================
DELETE FROM `creature` WHERE `id`=56073 AND `map`=939;
INSERT INTO `creature` (`guid`, `id`, `map`, `spawnMask`, `PhaseId`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `MovementType`) VALUES
(9000860, 56073, 939, 2, 169, 3450.425, -5708.649, 14.000, 0.0000, 7200, 5, 1),
(9000861, 56073, 939, 2, 169, 3452.088, -5711.449, 14.256, 5.2484, 7200, 5, 1),
(9000862, 56073, 939, 2, 169, 3452.161, -5711.404, 14.245, 5.2749, 7200, 0, 0),
(9000863, 56073, 939, 2, 169, 3463.971, -5753.199, 18.173, 4.1840, 7200, 5, 1),
(9000864, 56073, 939, 2, 169, 3463.285, -5752.614, 18.114, 3.9047, 7200, 5, 1),
(9000865, 56073, 939, 2, 169, 3463.387, -5752.717, 18.100, 3.9498, 7200, 0, 0),
(9000866, 56073, 939, 2, 169, 3464.071, -5803.048, 19.006, 5.8766, 7200, 5, 1),
(9000867, 56073, 939, 2, 169, 3463.125, -5804.322, 18.660, 5.3836, 7200, 5, 1),
(9000868, 56073, 939, 2, 169, 3463.736, -5803.676, 18.830, 5.6567, 7200, 0, 0),
(9000869, 56073, 939, 2, 169, 3492.086, -5719.409, 16.621, 2.4537, 7200, 5, 1),
(9000870, 56073, 939, 2, 169, 3492.025, -5719.485, 16.653, 2.4842, 7200, 5, 1),
(9000871, 56073, 939, 2, 169, 3491.355, -5721.388, 16.691, 3.1221, 7200, 0, 0),
(9000872, 56073, 939, 2, 169, 3522.440, -5736.825, 18.366, 5.4605, 7200, 5, 1),
(9000873, 56073, 939, 2, 169, 3516.973, -5734.161, 18.391, 3.0646, 7200, 5, 1),
(9000874, 56073, 939, 2, 169, 3519.176, -5737.480, 18.213, 4.3906, 7200, 0, 0),
(9000875, 56073, 939, 2, 169, 3562.885, -5661.405, 11.355, 1.3841, 7200, 5, 1),
(9000876, 56073, 939, 2, 169, 3562.851, -5661.399, 11.355, 1.3950, 7200, 5, 1),
(9000877, 56073, 939, 2, 169, 3567.012, -5667.049, 12.278, 5.7945, 7200, 0, 0),
(9000878, 56073, 939, 2, 169, 3576.559, -5711.080, 18.810, 2.3700, 7200, 5, 1),
(9000879, 56073, 939, 2, 169, 3577.186, -5710.578, 18.827, 2.1193, 7200, 5, 1),
(9000880, 56073, 939, 2, 169, 3579.887, -5710.275, 18.693, 1.2466, 7200, 0, 0),
(9000881, 56073, 939, 2, 169, 3584.381, -5768.717, 20.893, 2.1228, 7200, 5, 1),
(9000882, 56073, 939, 2, 169, 3592.193, -5769.526, 21.906, 0.2974, 7200, 5, 1),
(9000883, 56073, 939, 2, 169, 3589.622, -5765.998, 21.360, 0.9863, 7200, 0, 0);

-- ============================================================================
-- 11. Template corrections (factions from the sniff create blocks)
-- ============================================================================
-- Event adds must be hostile (sniff: fac 90 on all wave demons)
UPDATE `creature_template` SET `faction`=90 WHERE `entry` IN (55739,55762,56001,56002,56036);
-- Fel Flames ground fire is hostile (fac 14) or its 103891 pulse can never hit players
UPDATE `creature_template` SET `faction`=14 WHERE `entry`=55502;
-- Blade props: ground sword friendly-clickable (35), embedded blade rides Mannoroth (1771)
UPDATE `creature_template` SET `faction`=35 WHERE `entry` IN (55837,55839);
UPDATE `creature_template` SET `faction`=1771 WHERE `entry`=55838;
-- Mannoroth ignores taunts outright (Illidan's lock is threat-seeded + watchdogged;
-- his 6 s 104461 casts stay as flavor). CREATURE_FLAG_EXTRA_NO_TAUNT = 0x100.
UPDATE `creature_template` SET `flags_extra`=`flags_extra`|0x100 WHERE `entry`=54969;
-- Dreadlord Debilitator is taunt-proof too (the AI also applies spell immunities)
UPDATE `creature_template` SET `flags_extra`=`flags_extra`|0x100 WHERE `entry`=55762;

-- ============================================================================
-- 12. Texts (sniffed strings/sounds/BroadcastTextIds verbatim).
--     SoundType 1 = PLAY_OBJECT_SOUND (the pre-fight portal dialogue used
--     object sounds 54103-54110 and has no BroadcastTextID).
-- ============================================================================
DELETE FROM `creature_text` WHERE `CreatureID` IN (54969,55419,55532,55524,55570,55837,57913);
DELETE FROM `creature_text` WHERE `CreatureID`=56102 AND `GroupID`=4; -- groups 0-3 = Dragon Soul Madness aspect

INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
-- Mannoroth (54969)
(54969, 0, 0, 'Varo''then, see that I am not disrupted by this rabble!', 14, 0, 100, 0, 0, 26480, 0, 54118, 0, 'Mannoroth - shore RP'),
(54969, 1, 0, 'Come Stormrage, and I will show you what happens to those that betray the lord of the Legion!', 14, 0, 100, 0, 0, 26478, 0, 54223, 0, 'Mannoroth - aggro'),
(54969, 2, 0, '%s begins to cast |cFFF00000|Hspell:88972|h[Fel Firestorm]|h|r!', 41, 0, 100, 0, 0, 0, 0, 0, 0, 'Mannoroth - fel firestorm warning'),
(54969, 3, 0, '[Demonic] Amanare maev il azgalada zila ashj ashj zila enkil!', 14, 0, 100, 0, 0, 26488, 0, 56644, 0, 'Mannoroth - nether portal'),
(54969, 4, 0, 'Rrraaaghhh!!', 14, 0, 100, 0, 0, 26482, 0, 54555, 0, 'Mannoroth - blade embedded'),
(54969, 5, 0, 'Lord Sargeras, I will not fail you! Sweep your molten fist through this world, so that it may be reborn in flames and darkness!', 14, 0, 100, 0, 0, 26481, 0, 54256, 0, 'Mannoroth - stage three'),
(54969, 6, 0, 'Felguard pour forth from the demon portal!', 41, 0, 100, 0, 0, 0, 0, 0, 0, 'Mannoroth - felguard warning'),
(54969, 7, 0, 'Yes...yes! I can feel his burning eyes upon me, he is close...so close! And then your world will be unmade, your lives as nothing!', 14, 0, 100, 0, 0, 26484, 0, 54259, 0, 'Mannoroth - burning eyes'),
(54969, 8, 0, 'Infernals rain from the sky!', 41, 0, 100, 0, 0, 0, 0, 0, 0, 'Mannoroth - infernal warning'),
(54969, 9, 0, 'No...no! This victory will not be ripped from my grasp! I will not return to him in failure! I will not be torn from this pitiful world! No...NOOOOOOOO!!!', 14, 0, 100, 0, 0, 26479, 0, 54271, 0, 'Mannoroth - death throes'),

-- Captain Varo'then (55419)
(55419, 0, 0, 'Highguard, to arms! For your queen! For Azshara!', 14, 0, 100, 0, 0, 26137, 0, 54119, 0, 'Varo''then - highguard wave'),
(55419, 1, 0, 'For you, Azshara.', 12, 0, 100, 0, 0, 26134, 0, 54225, 0, 'Varo''then - aggro'),
(55419, 2, 0, 'Light of lights...I have failed you. I am sorry...my Azshara...', 12, 0, 100, 0, 0, 26135, 0, 54253, 0, 'Varo''then - death'),

-- Illidan Stormrage (55532)
(55532, 0, 0, 'Can you close the portal, brother?', 12, 0, 100, 0, 0, 54103, 1, 0, 0, 'Illidan finale - intro 1'),
(55532, 1, 0, 'Very well, we shall break it for you.', 12, 0, 100, 0, 0, 54104, 1, 0, 0, 'Illidan finale - intro 2'),
(55532, 2, 0, 'Let them come.', 12, 0, 100, 0, 0, 54105, 1, 0, 0, 'Illidan finale - intro 3'),
(55532, 3, 0, 'Weak, pitiful creatures. Hardly worthy of being called a legion.', 12, 0, 100, 0, 0, 26088, 0, 54111, 0, 'Illidan finale - shore road'),
(55532, 4, 0, 'Oh this will be fun...', 12, 0, 100, 0, 0, 26089, 0, 54112, 0, 'Illidan finale - vial 1'),
(55532, 5, 0, 'Wait, I have an idea.', 12, 0, 100, 0, 0, 26090, 0, 54113, 0, 'Illidan finale - vial 2'),
(55532, 6, 0, 'What our people could not.', 12, 0, 100, 0, 0, 26091, 0, 54116, 0, 'Illidan finale - vial 3'),
(55532, 7, 0, '%s splashes the Waters of Eternity over himself!', 41, 0, 100, 0, 0, 0, 0, 0, 0, 'Illidan finale - waters emote (GUESSED TEXT - the sniff shows an emote here but its string was not captured)'),
(55532, 8, 0, 'Yes...YES. I can feel the raw power of the Well of Eternity! It is time to end this charade!', 12, 0, 100, 0, 0, 26092, 0, 54117, 0, 'Illidan finale - vial 4'),
(55532, 9, 0, 'They are not where they appear to be!  Strike in an area, it is the only way to uncover the real one!', 12, 0, 100, 0, 0, 26093, 0, 54121, 0, 'Illidan finale - mirror hint'),
(55532, 10, 0, 'Handle Varo''then. Mannoroth is mine.', 12, 0, 100, 0, 0, 26094, 0, 54184, 0, 'Illidan finale - staging'),
(55532, 11, 0, 'The sword has pierced his infernal armor! Strike him down!', 14, 0, 100, 0, 0, 26095, 0, 54254, 0, 'Illidan finale - sword pierced'),
(55532, 12, 0, 'A massive demonic portal opens nearby!', 41, 0, 100, 0, 0, 0, 0, 0, 0, 'Illidan finale - stage three portal warning'),
(55532, 13, 0, 'He is still connected to the Well somehow! Focus your attacks on Mannoroth, we must disrupt his concentration!', 14, 0, 100, 0, 0, 26099, 0, 54257, 0, 'Illidan finale - stage three'),
(55532, 14, 0, 'The artifact!', 14, 0, 100, 0, 0, 26059, 0, 56630, 0, 'Illidan finale - the artifact'),
(55532, 15, 0, 'Brother. A timely arrival...', 12, 0, 100, 0, 0, 26060, 0, 56631, 0, 'Illidan finale - epilogue 1'),
(55532, 16, 0, 'Aye. It''s been twisted and turned by too many spells. The fuss we - especially you - made with the portal was too much! The same spell that sent the Burning Legion back into their foul realm now works on the well! It''s devouring itself and taking its surroundings with it! Fascinating, isn''t it?', 12, 0, 100, 0, 0, 26061, 0, 56632, 0, 'Illidan finale - epilogue 2'),
(55532, 17, 0, 'If you''ve a way out of here, we should probably use it! I''ve tried casting myself and Tyrande out of here, but the well is too much in flux!', 12, 0, 100, 0, 0, 26062, 0, 56633, 0, 'Illidan finale - epilogue 3'),

-- Tyrande Whisperwind (55524)
(55524, 0, 0, 'He knows what we attempt. We have not much time, the forest crawls with his demons.', 12, 0, 100, 0, 0, 54108, 1, 0, 0, 'Tyrande - intro 1'),
(55524, 1, 0, 'Mother moon, guide us through this darkness.', 12, 0, 100, 0, 0, 54110, 1, 0, 0, 'Tyrande - intro 2'),
(55524, 2, 0, 'Illidan, what is in that vial? What are you doing?', 12, 0, 100, 0, 0, 25995, 0, 54114, 0, 'Tyrande - vial'),
(55524, 3, 0, 'I cannot strike them!  What is this demon magic?', 12, 0, 100, 0, 0, 25996, 0, 54120, 0, 'Tyrande - mirror wave'),
(55524, 4, 0, 'I will handle the demons. Elune, guide my arrows!', 12, 0, 100, 0, 0, 25997, 0, 54227, 0, 'Tyrande - lane start'),
(55524, 5, 0, 'Light of Elune, save me!', 14, 0, 100, 0, 0, 25998, 0, 54229, 0, 'Tyrande - flayed'),
(55524, 6, 0, 'Tyrande is overwhelmed! Use the Blessing of Elune to drive the demons back!', 41, 0, 100, 0, 0, 0, 0, 0, 0, 'Tyrande - overwhelmed warning'),
(55524, 7, 0, 'Tyrande can hold her own once again!', 41, 0, 100, 0, 0, 0, 0, 0, 0, 'Tyrande - freed emote'),
(55524, 8, 0, 'I will hold them back for now!', 14, 0, 100, 0, 0, 25999, 0, 54231, 0, 'Tyrande - freed'),
(55524, 9, 0, 'Illidan, I am out of arrows! Moon goddess, protect us from the darkness, that we may see your light again another night!', 12, 0, 100, 0, 0, 26000, 0, 54258, 0, 'Tyrande - out of arrows'),
(55524, 10, 0, 'Tyrande is imbued with the shining light of the Moon Goddess!', 41, 0, 100, 0, 0, 0, 0, 0, 0, 'Tyrande - imbued emote'),
(55524, 11, 0, 'There are too many of them!', 14, 0, 100, 22, 0, 26004, 0, 54609, 0, 'Tyrande - too many'),
(55524, 12, 0, 'Tyrande collapses!  The Light of Elune winks out!', 41, 0, 100, 0, 0, 0, 0, 0, 0, 'Tyrande - collapse emote'),
(55524, 13, 0, 'Malfurion, he has done it! The portal is collapsing!', 12, 0, 100, 0, 0, 26003, 0, 54270, 0, 'Tyrande - encounter end (DBM kill line)'),
(55524, 14, 0, 'Malfurion...', 12, 0, 100, 0, 0, 25989, 0, 56620, 0, 'Tyrande - epilogue 1'),
(55524, 15, 0, 'By the very edge...', 12, 0, 100, 1, 0, 25990, 0, 56621, 0, 'Tyrande - epilogue 2'),
(55524, 16, 0, 'I do not know who you are, but I thank you. Without your aid, our world would be...I do not wish to think about it. Moon goddess light your path.', 12, 0, 100, 1, 0, 25991, 0, 56622, 0, 'Tyrande - epilogue 3'),

-- Malfurion Stormrage (55570)
(55570, 0, 0, 'It is being maintained by the will of a powerful demon...Mannoroth.', 12, 0, 100, 0, 0, 54106, 1, 0, 0, 'Malfurion - intro 1'),
(55570, 1, 0, 'I cannot break his will alone...', 12, 0, 100, 0, 0, 54107, 1, 0, 0, 'Malfurion - intro 2'),
(55570, 2, 0, 'The Dragon Soul''s link to the portal has been broken!  The Soul plummets downwards towards the Well!', 41, 0, 100, 0, 0, 0, 0, 0, 0, 'Malfurion - soul plummets emote'),
(55570, 3, 0, 'Hush, Tyrande. Where is Illidan?', 12, 0, 100, 1, 0, 26490, 0, 56637, 0, 'Malfurion - epilogue 1'),
(55570, 4, 0, 'Illidan! The well is out of control!', 14, 0, 100, 22, 0, 26491, 0, 56638, 0, 'Malfurion - epilogue 2'),
(55570, 5, 0, 'Not if we''re caught up in it! Why weren''t you running? What have you been doing with your hand in the Well?', 12, 0, 100, 6, 0, 26492, 0, 56639, 0, 'Malfurion - epilogue 3'),
(55570, 6, 0, 'This way!', 14, 0, 100, 22, 0, 26493, 0, 56640, 0, 'Malfurion - epilogue 4'),

-- Varo'then's Magical Blade (55837)
(55837, 0, 0, 'Varo''then''s magical sword falls to the ground!', 41, 0, 100, 0, 0, 0, 0, 0, 0, 'Magical Blade - falls'),

-- Chromie (57913)
(57913, 0, 0, 'Did I miss anything? Oh WOW!', 12, 0, 100, 0, 0, 0, 0, 0, 0, 'Chromie - arrival'),
(57913, 1, 0, 'We''ve gathered up some items from this time period. I hope this helps!', 12, 0, 100, 0, 0, 0, 0, 0, 0, 'Chromie - loot'),

-- Nozdormu finale (56102) - GROUP 4: groups 0-3 belong to the DS Madness aspect script
(56102, 4, 0, 'The Dragon Soul is safe once again. Quickly, into the time portal, before this world sunders!', 14, 0, 100, 457, 0, 25960, 0, 54272, 3, 'Nozdormu - WoE finale (zone-wide, he hovers over the well)');

-- ============================================================================
-- NOTES (no automatic change - verify/tune during the walkthrough)
-- ============================================================================
-- 1. HP/damage tuning (per project convention, retail values come from the
--    user; modern sniff numbers are stat-squished):
--      * 54969 Mannoroth HealthModifier is 1200 in TDB - the fight is won via
--        1,000,000 Magistrike Arc procs every 3.5 s, so his pool defines the
--        burn length. // tune
--      * 55524 Tyrande HealthModifier 50: she must survive ~60 s of capped
--        (8x) Devastator melee while flayed. Raise if she folds. // tune
--      * Fel Drain threshold is 10% of Mannoroth's max health of cumulative
--        player damage while Varo'then lives (boss_mannoroth.cpp). // tune
-- 2. Allied trio 55532/55524/55570 (factions 1770/1770/35) must be friendly
--    to players and hostile to the Legion factions (14/16/90/1771/2387).
--    Faction 35 Malfurion never fights - no change needed. Illidan/Tyrande
--    hold their own lanes; if 1770 turns out not to be hostile to 1771/90,
--    swap them to 1802 (friendly, attacks demons) during the walkthrough.
-- 3. Illidan's 104746 Demonic Sight (90% dodge) is applied by the script on
--    spawn; alternatively add it to creature_template_addon.auras for 55532.
-- 4. 55838 Embedded Blade / 55839 Strike Points should be unattackable props;
--    templates already flag them - verify players cannot nuke the seat-0
--    blade (if they can, add UNIT_FLAG_NON_ATTACKABLE 33554432+2 on 55838).
-- 5. The instance script owns spawn group 470 (Royal Cache, Azshara) - only
--    471 is seeded here. If 470 is not seeded by the Azshara/instance
--    fragment, add its gameobject row analogously.
-- 6. 57913 Chromie / 57864 Alurmi exit-teleport gossip (menus 13362/13361)
--    are out of scope here - verify they exist before shipping.
-- 7. 56102 is shared with Dragon Soul Madness (ScriptName
--    npc_madness_of_deathwing_dragon_aspect). On map 939 that AI factory
--    rejects the creature and the default AI takes over - Talk(4) still
--    works. Do NOT clear its ScriptName (it would break Dragon Soul).
-- 8. Voracious Felhound packs (section 10) sit on the far shore east of the
--    arena; the escort road itself is contested by the 15 Doomguard
--    Annihilators (55519) which the script also fights through.
-- 9. Peroth'arn's fragment (sql_perotharn.sql) does not add the Lazy Eye
--    criteria gate; if achievement 6127 grants unconditionally, add:
--    achievement_criteria_data (18618, 18, 0, 0, '') - the instance script
--    already implements the check.
