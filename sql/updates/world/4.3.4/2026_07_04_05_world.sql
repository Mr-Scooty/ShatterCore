-- The Lost Isles (zone 4720): Part 5 - script bindings, SmartAI, conditions,
-- loot fixes and remaining spawns.

-- ----------------------------------------------------------------------------
-- 1) Script bindings.
-- ----------------------------------------------------------------------------
UPDATE `creature_template` SET `ScriptName` = 'npc_frightened_miner' WHERE `entry` = 35813;
UPDATE `creature_template` SET `ScriptName` = 'npc_weed_whacker_bunny' WHERE `entry` = 35903;
UPDATE `creature_template` SET `ScriptName` = 'npc_lost_isles_bastia' WHERE `entry` IN (36585, 39152);
UPDATE `creature_template` SET `ScriptName` = 'npc_lost_isles_gyrochoppa' WHERE `entry` = 36143;
UPDATE `creature_template` SET `ScriptName` = 'npc_lost_isles_cyclone' WHERE `entry` = 36178;
UPDATE `creature_template` SET `ScriptName` = 'npc_lost_isles_sling_rocket' WHERE `entry` IN (36505, 36514);
UPDATE `creature_template` SET `ScriptName` = 'npc_super_booster_rocket_boots' WHERE `entry` = 38802;
UPDATE `creature_template` SET `ScriptName` = 'boss_volcanoth' WHERE `entry` = 38855;
UPDATE `creature_template` SET `ScriptName` = 'npc_volcanoth_eruption_bunny' WHERE `entry` = 38985;
UPDATE `creature_template` SET `ScriptName` = 'npc_flying_bomber' WHERE `entry` = 38918;
UPDATE `creature_template` SET `ScriptName` = 'npc_alliance_paratrooper' WHERE `entry` = 39042;
UPDATE `creature_template` SET `ScriptName` = 'npc_pride_of_kezan' WHERE `entry` = 39074;
UPDATE `creature_template` SET `ScriptName` = 'npc_lost_isles_mine_cart' WHERE `entry` = 39329;
UPDATE `creature_template` SET `ScriptName` = 'boss_trade_prince_gallywix' WHERE `entry` = 39582;
UPDATE `creature_template` SET `ScriptName` = 'npc_thrall_gallywix_fight' WHERE `entry` = 39594;
UPDATE `creature_template` SET `ScriptName` = 'npc_ultimate_footbomb_uniform' WHERE `entry` = 39598;
UPDATE `creature_template` SET `ScriptName` = 'npc_lost_isles_battleworg' WHERE `entry` = 39611;
UPDATE `gameobject_template` SET `ScriptName` = 'go_rocket_sling' WHERE `entry` = 196439;
UPDATE `gameobject_template` SET `ScriptName` = 'go_platform_control_panel' WHERE `entry` = 202613;

DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
'spell_lost_isles_exploding_bananas', 'spell_lost_isles_ktc_snapflash', 'spell_lost_isles_snapflash_effect',
'spell_lost_isles_weed_whacker', 'spell_lost_isles_weed_whacker_aura', 'spell_lost_isles_remote_fireworks',
'spell_lost_isles_summon_mechashark', 'spell_lost_isles_pool_pony_click', 'spell_lost_isles_rocket_boots',
'spell_lost_isles_boot_stomp', 'spell_lost_isles_gunship_gun', 'spell_lost_isles_grenade',
'spell_lost_isles_kaja_cola', 'spell_lost_isles_soulstone', 'spell_lost_isles_escape_velocity', 'spell_lost_isles_captive_drain');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(67917, 'spell_lost_isles_exploding_bananas'),
(68280, 'spell_lost_isles_ktc_snapflash'),
(68296, 'spell_lost_isles_snapflash_effect'),
(68211, 'spell_lost_isles_weed_whacker'),
(68212, 'spell_lost_isles_weed_whacker_aura'),
(71170, 'spell_lost_isles_remote_fireworks'),
(71648, 'spell_lost_isles_summon_mechashark'),
(71919, 'spell_lost_isles_pool_pony_click'),
(71918, 'spell_lost_isles_pool_pony_click'),
(83115, 'spell_lost_isles_pool_pony_click'),
(83116, 'spell_lost_isles_pool_pony_click'),
(72891, 'spell_lost_isles_rocket_boots'),
(72886, 'spell_lost_isles_boot_stomp'),
(74958, 'spell_lost_isles_gunship_gun'),
(73477, 'spell_lost_isles_gunship_gun'),
(73425, 'spell_lost_isles_grenade'),
(73583, 'spell_lost_isles_kaja_cola'),
(73702, 'spell_lost_isles_soulstone'),
(73947, 'spell_lost_isles_escape_velocity'),
(72518, 'spell_lost_isles_captive_drain');

-- The Battleworg ride boards natively on accept (player-cast so the summon
-- belongs to the rider; dest anchored at the Kor'kron Loyalist).
UPDATE `quest_template_addon` SET `SourceSpellID` = 74031 WHERE `ID` = 25267;
UPDATE `quest_template` SET `Flags` = `Flags` | 0x100000 WHERE `ID` = 25267;

-- ----------------------------------------------------------------------------
-- 2) SmartAI.
-- ----------------------------------------------------------------------------
-- Goblin Escape Pods: using a pod rescues the survivor inside.
UPDATE `gameobject_template` SET `AIName` = 'SmartGameObjectAI' WHERE `entry` = 195188;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 195188 AND `source_type` = 1;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(195188, 1, 0, 0, 64, 0, 100, 0, 0, 0, 0, 0, 0, 33, 34748, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Goblin Escape Pod - On Used - Kill Credit Goblin Survivor');

-- Weed Whacker: Poison Spitter's native KillCredit1 (35897) already grants
-- the proxy credit on death; no SmartAI needed.
DELETE FROM `smart_scripts` WHERE `entryorguid` = 35896 AND `source_type` = 0;
UPDATE `creature_template` SET `AIName` = '' WHERE `entry` = 35896;

-- Town-In-A-Box Under Attack: Oomlot Warrior deaths credit the proxy
-- (SmartAI forwards the credit to B.C. Eliminator passengers).
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` = 38531;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 38531 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(38531, 0, 0, 0, 6, 0, 100, 0, 0, 0, 0, 0, 0, 33, 38536, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Oomlot Warrior - On Death - Kill Credit Town-In-A-Box defense');

-- Miner Troubles: the static Pygmy Witchdoctor's death is the quest credit.
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` = 35838;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 35838 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(35838, 0, 0, 0, 6, 0, 100, 0, 0, 0, 0, 0, 0, 33, 35816, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Pygmy Witchdoctor - On Death - Kill Credit Miner Troubles'),
(35838, 0, 1, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Pygmy Witchdoctor - On Aggro - Say Line 0 (once)');

-- Naga Hatchling followers: trail their rescuer, then swim off.
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` IN (44588, 44589, 44590, 44591);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (44588, 44589, 44590, 44591) AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(44588, 0, 0, 1, 54, 0, 100, 0, 0, 0, 0, 0, 0, 29, 1, 90, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Naga Hatchling - On Summoned - Follow summoner'),
(44588, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 41, 30000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Naga Hatchling - Linked - Despawn after 30s'),
(44589, 0, 0, 1, 54, 0, 100, 0, 0, 0, 0, 0, 0, 29, 1, 90, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Naga Hatchling - On Summoned - Follow summoner'),
(44589, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 41, 30000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Naga Hatchling - Linked - Despawn after 30s'),
(44590, 0, 0, 1, 54, 0, 100, 0, 0, 0, 0, 0, 0, 29, 1, 90, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Naga Hatchling - On Summoned - Follow summoner'),
(44590, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 41, 30000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Naga Hatchling - Linked - Despawn after 30s'),
(44591, 0, 0, 1, 54, 0, 100, 0, 0, 0, 0, 0, 0, 29, 1, 90, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Naga Hatchling - On Summoned - Follow summoner'),
(44591, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 41, 30000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Naga Hatchling - Linked - Despawn after 30s');

-- Wild Clucker Egg: hatches into the lootable egg chest (native loot 50239).
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` = 38195;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 38195 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(38195, 0, 0, 1, 1, 0, 100, 1, 5400, 5400, 0, 0, 0, 50, 201974, 300, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Wild Clucker Egg - OOC once - Summon egg chest'),
(38195, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 41, 2000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Wild Clucker Egg - Linked - Despawn');

-- Doc Zapnozzle's revive vignette (timings from the sniff).
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` = 36608;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 36608 AND `source_type` = 0;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 3660800 AND `source_type` = 9;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(36608, 0, 0, 0, 1, 0, 100, 0, 10000, 15000, 90000, 120000, 0, 80, 3660800, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Doc Zapnozzle - OOC - Run revive vignette'),
(36608, 0, 1, 0, 20, 0, 100, 0, 14239, 0, 0, 0, 0, 1, 5, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Doc Zapnozzle - On Quest 14239 Rewarded - Say farewell'),
(3660800, 9, 0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Doc vignette - Say 0'),
(3660800, 9, 1, 0, 0, 0, 100, 0, 5600, 5600, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Doc vignette - Say 1'),
(3660800, 9, 2, 0, 0, 0, 100, 0, 3100, 3100, 0, 0, 0, 11, 69085, 0, 0, 0, 0, 0, 19, 36600, 20, 0, 0, 0, 0, 0, 'Doc vignette - Zap Gizmo'),
(3660800, 9, 3, 0, 0, 0, 100, 0, 6500, 6500, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Doc vignette - Say 2'),
(3660800, 9, 4, 0, 0, 0, 100, 0, 5500, 5500, 0, 0, 0, 11, 69022, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Doc vignette - Jumper cables #1'),
(3660800, 9, 5, 0, 0, 0, 100, 0, 5700, 5700, 0, 0, 0, 1, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Doc vignette - Come on! Clear!'),
(3660800, 9, 6, 0, 0, 0, 100, 0, 3100, 3100, 0, 0, 0, 11, 69022, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Doc vignette - Jumper cables #2'),
(3660800, 9, 7, 0, 0, 0, 100, 0, 5700, 5700, 0, 0, 0, 1, 4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Doc vignette - Say 4');

-- Gizmo answers when Doc zaps him.
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` = 36600;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 36600 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(36600, 0, 0, 0, 8, 0, 100, 0, 69085, 0, 5000, 5000, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Geargrinder Gizmo - On Spellhit Zap - Say reply');

-- Foreman Dampwick: gossip re-summon of the escort miner.
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` = 35769;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 35769 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(35769, 0, 0, 1, 62, 0, 100, 0, 10677, 0, 0, 0, 0, 12, 35813, 1, 600000, 0, 0, 0, 8, 0, 0, 0, 492.4184, 2976.3213, 8.040207, 5.5267, 'Foreman Dampwick - Gossip - Summon Frightened Miner'),
(35769, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 72, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Foreman Dampwick - Linked - Close gossip');

-- Sassy @179: rocket launch via gossip.
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` = 36425;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 36425 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(36425, 0, 0, 1, 62, 0, 100, 0, 10808, 0, 0, 0, 0, 85, 68804, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Sassy Hardwrench - Gossip - Invoker casts Summon Sling Rocket'),
(36425, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 72, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Sassy Hardwrench - Linked - Close gossip');

-- Sassy 38928: gossip just closes (boarding via the bomber spellclick).
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` = 38928;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 38928 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(38928, 0, 0, 0, 62, 0, 100, 0, 11146, 0, 0, 0, 0, 72, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Sassy Hardwrench (bomber) - Gossip - Close');

-- Sassy 38387: Pride of Kezan launch and the set-sail departure.
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` = 38387;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 38387 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(38387, 0, 0, 1, 62, 0, 100, 0, 12582, 0, 0, 0, 0, 85, 74924, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Sassy Hardwrench - Gossip (set sail) - Invoker casts departure teleport'),
(38387, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 72, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Sassy Hardwrench - Linked - Close gossip'),
(38387, 0, 2, 3, 62, 0, 100, 0, 12582, 1, 0, 0, 0, 85, 73431, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Sassy Hardwrench - Gossip (Pride of Kezan) - Invoker casts gunship summon'),
(38387, 0, 3, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 72, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Sassy Hardwrench - Linked - Close gossip');

-- Thrall's camp: periodic Alliance paratrooper drops (sniffed air points).
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` = 38935;
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (38935, -392489) AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(-392489, 0, 0, 0, 1, 0, 100, 0, 10000, 20000, 60000, 90000, 0, 12, 39042, 1, 180000, 0, 0, 0, 8, 0, 0, 0, 1557.889, 2858.7173, 64.29019, 0, 'Thrall - OOC - Summon Alliance Paratrooper (drop 1)'),
(-392489, 0, 1, 0, 1, 0, 100, 0, 25000, 40000, 60000, 90000, 0, 12, 39042, 1, 180000, 0, 0, 0, 8, 0, 0, 0, 1791.7794, 2955.488, 153.94121, 0, 'Thrall - OOC - Summon Alliance Paratrooper (drop 2)'),
(-392489, 0, 2, 0, 1, 0, 100, 0, 40000, 60000, 60000, 90000, 0, 12, 39042, 1, 180000, 0, 0, 0, 8, 0, 0, 0, 1631.5173, 2961.7915, 183.86122, 0, 'Thrall - OOC - Summon Alliance Paratrooper (drop 3)'),
(-392489, 0, 3, 0, 1, 0, 100, 0, 55000, 75000, 60000, 90000, 0, 12, 39042, 1, 180000, 0, 0, 0, 8, 0, 0, 0, 1696.7979, 2972.9348, 114.64534, 0, 'Thrall - OOC - Summon Alliance Paratrooper (drop 4)'),
(-392489, 0, 4, 0, 1, 0, 100, 0, 70000, 90000, 60000, 90000, 0, 12, 39042, 1, 180000, 0, 0, 0, 8, 0, 0, 0, 1705.889, 2953.962, 91.21915, 0, 'Thrall - OOC - Summon Alliance Paratrooper (drop 5)');

-- ----------------------------------------------------------------------------
-- 3) Conditions: spell implicit-target anchors (source 13, type 31).
--    value1: 3 = unit, 5 = gameobject.
-- ----------------------------------------------------------------------------
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` IN (68280, 71857, 56576, 72886, 73477, 74958, 73430, 73532, 73746, 73991, 89164, 74031, 73601, 73609, 73611);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 68280, 0, 1, 31, 0, 3, 37872, 0, 0, 0, 0, '', 'KTC Snapflash targets vignette bunny 1'),
(13, 1, 68280, 0, 2, 31, 0, 3, 37895, 0, 0, 0, 0, '', 'KTC Snapflash targets vignette bunny 2'),
(13, 1, 68280, 0, 3, 31, 0, 3, 37896, 0, 0, 0, 0, '', 'KTC Snapflash targets vignette bunny 3'),
(13, 1, 68280, 0, 4, 31, 0, 3, 37897, 0, 0, 0, 0, '', 'KTC Snapflash targets vignette bunny 4'),
(13, 1, 71857, 0, 0, 31, 0, 5, 202133, 0, 0, 0, 0, '', 'Bilgewater banner plant anchors at Naga Banner'),
(13, 1, 56576, 0, 0, 31, 0, 5, 201972, 0, 0, 0, 0, '', 'Wild Clucker Egg summon anchors at the nest spell focus'),
(13, 1, 72886, 0, 1, 31, 0, 3, 38753, 0, 0, 0, 0, '', 'Rocket boot stomp hits Goblin Zombie'),
(13, 1, 72886, 0, 2, 31, 0, 3, 38813, 0, 0, 0, 0, '', 'Rocket boot stomp hits Goblin Zombie'),
(13, 1, 72886, 0, 3, 31, 0, 3, 38815, 0, 0, 0, 0, '', 'Rocket boot stomp hits Goblin Zombie'),
(13, 1, 72886, 0, 4, 31, 0, 3, 38816, 0, 0, 0, 0, '', 'Rocket boot stomp hits Goblin Zombie'),
(13, 1, 73477, 0, 0, 31, 0, 3, 39039, 0, 0, 0, 0, '', 'Gunship gun targets Gnomeregan Stealth Fighter'),
(13, 1, 74958, 0, 0, 31, 0, 3, 39039, 0, 0, 0, 0, '', 'Gunship missile targets Gnomeregan Stealth Fighter'),
(13, 1, 73430, 0, 0, 31, 0, 3, 38387, 0, 0, 0, 0, '', 'Pride of Kezan summon anchors at Sassy'),
(13, 1, 73532, 0, 0, 31, 0, 3, 39066, 0, 0, 0, 0, '', 'Bastia summon anchors at Kilag'),
(13, 1, 73746, 0, 0, 31, 0, 3, 39341, 0, 0, 0, 0, '', 'Mine Cart summon anchors at the parked cart'),
(13, 1, 73991, 0, 0, 31, 0, 3, 39592, 0, 0, 0, 0, '', 'Ultimate Footbomb Uniform summon anchors at the uniform prop'),
(13, 1, 89164, 0, 0, 31, 0, 3, 38124, 0, 0, 0, 0, '', 'Footbomb disguise summon anchors at Greely'),
(13, 1, 74031, 0, 0, 31, 0, 3, 39609, 0, 0, 0, 0, '', 'Battleworg summon anchors at the Kor''kron Loyalist'),
(13, 1, 73601, 0, 0, 31, 0, 3, 38441, 0, 0, 0, 0, '', 'Freed Ace summon anchors at captive Ace'),
(13, 1, 73609, 0, 0, 31, 0, 3, 38647, 0, 0, 0, 0, '', 'Freed Izzy summon anchors at captive Izzy'),
(13, 1, 73611, 0, 0, 31, 0, 3, 38746, 0, 0, 0, 0, '', 'Freed Gobber summon anchors at captive Gobber');

-- Spellclick quest gates (source 18): clicker must be a player on the quest.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 18 AND `SourceGroup` IN (38111, 38412, 44580, 38526, 38918, 39456, 39592);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(18, 38111, 71170, 0, 0, 31, 0, 4, 0, 0, 0, 0, 0, '', 'Wild Clucker: clicker must be a player'),
(18, 38111, 71170, 0, 0, 9, 0, 24671, 0, 0, 0, 0, 0, '', 'Wild Clucker: Cluster Cluck taken'),
(18, 38412, 71919, 0, 0, 31, 0, 4, 0, 0, 0, 0, 0, '', 'Naga Hatchling: clicker must be a player'),
(18, 38412, 71919, 0, 0, 9, 0, 24864, 0, 0, 0, 0, 0, '', 'Naga Hatchling: Irresistible Pool Pony taken'),
(18, 44580, 71919, 0, 0, 31, 0, 4, 0, 0, 0, 0, 0, '', 'Naga Hatchling: clicker must be a player'),
(18, 44580, 71919, 0, 0, 9, 0, 24864, 0, 0, 0, 0, 0, '', 'Naga Hatchling: Irresistible Pool Pony taken'),
(18, 38526, 72240, 0, 0, 31, 0, 4, 0, 0, 0, 0, 0, '', 'B.C. Eliminator: clicker must be a player'),
(18, 38526, 72240, 0, 0, 9, 0, 24901, 0, 0, 0, 0, 0, '', 'B.C. Eliminator: Town-In-A-Box Under Attack taken'),
(18, 38918, 46598, 0, 0, 31, 0, 4, 0, 0, 0, 0, 0, '', 'Flying Bomber: clicker must be a player'),
(18, 38918, 46598, 0, 0, 9, 0, 25023, 0, 0, 0, 0, 0, '', 'Flying Bomber: Old Friends taken'),
(18, 39456, 73947, 0, 0, 31, 0, 4, 0, 0, 0, 0, 0, '', 'Captured Goblin: clicker must be a player'),
(18, 39456, 73947, 0, 0, 9, 0, 25214, 0, 0, 0, 0, 0, '', 'Captured Goblin: Escape Velocity taken'),
(18, 39592, 73991, 0, 0, 31, 0, 4, 0, 0, 0, 0, 0, '', 'Footbomb Uniform: clicker must be a player'),
(18, 39592, 73991, 0, 0, 9, 0, 25251, 0, 0, 0, 0, 0, '', 'Footbomb Uniform: Final Confrontation taken');

-- ----------------------------------------------------------------------------
-- 4) Gossip: the Pride of Kezan launch option on Sassy's town menu.
-- ----------------------------------------------------------------------------
DELETE FROM `gossip_menu_option` WHERE `MenuID` = 12582 AND `OptionID` = 1;
INSERT INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcflag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES
(12582, 1, 0, 'I''m ready to fly the Pride of Kezan!', 0, 1, 1, 0, 0, 0, 0, NULL, 0, 15595);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 15 AND `SourceGroup` = 12582 AND `SourceEntry` = 1;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(15, 12582, 1, 0, 0, 9, 0, 25066, 0, 0, 0, 0, 0, '', 'Sassy: gunship option while The Pride of Kezan taken');

-- ----------------------------------------------------------------------------
-- 5) Loot fixes.
-- ----------------------------------------------------------------------------
-- Rockin' Powder drops from the Oostan pygmies.
DELETE FROM `creature_loot_template` WHERE `Entry` = 38811 AND `Item` = 52024;
INSERT INTO `creature_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`) VALUES
(38811, 52024, 0, 40, 1, 1, 0, 1, 1, 'Oostan Headhunter - Rockin'' Powder');

-- Guaranteed quest drops.
UPDATE `creature_loot_template` SET `Chance` = 100, `QuestRequired` = 1 WHERE `Entry` = 39194 AND `Item` = 52481; -- Blastshadow's Soulstone
UPDATE `creature_loot_template` SET `Chance` = 100, `QuestRequired` = 1 WHERE `Entry` = 39363 AND `Item` = 52531; -- Chip's heart
-- The Kaja'Cola Zero-One chests hold the IDEAS quest item, not the can.
UPDATE `gameobject_loot_template` SET `Item` = 52483 WHERE `Entry` = 28398 AND `Item` = 52484;

-- Quest items should only drop for players on the quest.
UPDATE `creature_loot_template` SET `QuestRequired` = 1 WHERE `Item` IN (46828, 49424, 50239, 50381, 50437, 52035, 52346, 52347, 52349, 52483, 52530, 52559);

-- ----------------------------------------------------------------------------
-- 6) Remaining spawns: the rideable Flying Bomber and the Oomlot Shaman
--    captors (positions from the sniff; shamans channel their captives).
-- ----------------------------------------------------------------------------
SET @CGUID := 9000410;
DELETE FROM `creature` WHERE `guid` BETWEEN @CGUID+0 AND @CGUID+12;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseUseFlags`, `phaseMask`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild`) VALUES
(@CGUID+0, 38918, 648, 4720, 0, 1, 0, 1, 183, 0, -1, 0, 0, 1151.07, 1115.43, 129.64, 2.11, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 15595), -- Flying Bomber (Old Friends)
(@CGUID+1, 38644, 648, 4720, 0, 1, 0, 1, 181, 0, -1, 0, 0, 746.6433, 1753.1249, 115.1671, 2.7227, 60, 0, 0, 0, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+2, 38644, 648, 4720, 0, 1, 0, 1, 181, 0, -1, 0, 0, 741.4469, 1704.887, 116.0247, 5.1661, 60, 0, 0, 0, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+3, 38644, 648, 4720, 0, 1, 0, 1, 181, 0, -1, 0, 0, 739.5894, 1730.8936, 114.0705, 0.9599, 60, 0, 0, 0, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+4, 38644, 648, 4720, 0, 1, 0, 1, 181, 0, -1, 0, 0, 713.2426, 1721.5835, 115.0807, 5.0267, 60, 0, 0, 0, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+5, 38644, 648, 4720, 0, 1, 0, 1, 181, 0, -1, 0, 0, 785.8434, 1730.247, 120.5601, 0.1921, 60, 0, 0, 0, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+6, 38644, 648, 4720, 0, 1, 0, 1, 181, 0, -1, 0, 0, 764.4905, 1732.1489, 118.7731, 2.4434, 60, 0, 0, 0, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+7, 38644, 648, 4720, 0, 1, 0, 1, 181, 0, -1, 0, 0, 771.1751, 1696.9703, 125.1255, 3.0891, 60, 0, 0, 0, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+8, 38644, 648, 4720, 0, 1, 0, 1, 181, 0, -1, 0, 0, 716.9445, 1664.5726, 121.2023, 4.9567, 60, 0, 0, 0, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+9, 38644, 648, 4720, 0, 1, 0, 1, 181, 0, -1, 0, 0, 768.9408, 1676.5234, 126.0962, 2.7926, 60, 0, 0, 0, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+10, 38644, 648, 4720, 0, 1, 0, 1, 181, 0, -1, 0, 0, 801.7067, 1698.5624, 125.7861, 4.7646, 60, 0, 0, 0, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+11, 38644, 648, 4720, 0, 1, 0, 1, 181, 0, -1, 0, 0, 696.4135, 1642.3203, 116.4784, 3.8573, 60, 0, 0, 0, 0, 0, 0, 0, 0, '', 15595),
(@CGUID+12, 38644, 648, 4720, 0, 1, 0, 1, 181, 0, -1, 0, 0, 774.2563, 1656.6321, 126.8149, 0.2967, 60, 0, 0, 0, 0, 0, 0, 0, 0, '', 15595);

-- Oomlot Shaman: channel the drain on the nearest captive until killed.
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` = 38644;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 38644 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(38644, 0, 0, 0, 1, 0, 100, 0, 2000, 2000, 15000, 15000, 0, 11, 72518, 0, 0, 0, 0, 0, 19, 38643, 10, 0, 0, 0, 0, 0, 'Oomlot Shaman - OOC - Channel drain on captive');
