--
-- Gilneas (Worgen starter zone) — vehicle setup + quest accept-casts
-- VehicleIds are sniff-RecID ground truth, verified against 4.3.4 Vehicle.dbc.
--

UPDATE `creature_template` SET `VehicleId` = 542 WHERE `entry` = 36741;   -- Swift Mountain Horse (14465 coach)
UPDATE `creature_template` SET `VehicleId` = 866 WHERE `entry` = 37927;   -- Glaive Thrower (24681)
UPDATE `creature_template` SET `VehicleId` = 641 WHERE `entry` = 38540;   -- Captured Riding Bat (24920)
UPDATE `creature_template` SET `VehicleId` = 641 WHERE `entry` = 43751;   -- Hippogryph (26706)
UPDATE `creature_template` SET `VehicleId` = 983 WHERE `entry` = 43713;   -- Wyvern (26706)
UPDATE `creature_template` SET `VehicleId` = 1450 WHERE `entry` = 51083;  -- Gilneas Funeral Camera (24679)
-- Scene extras (retail-correct convoy/coach units)
UPDATE `creature_template` SET `VehicleId` = 542 WHERE `entry` = 38765;   -- Stout Mountain Horse
UPDATE `creature_template` SET `VehicleId` = 958 WHERE `entry` = 43336;   -- Stagecoach Harness
UPDATE `creature_template` SET `VehicleId` = 959 WHERE `entry` = 43337;   -- Stagecoach Carriage

-- Glaive Thrower is entered by click (sniffed click spell 68503 = CONTROL_VEHICLE);
-- both click-vehicles also need UNIT_NPC_FLAG_SPELLCLICK or the client never sends
-- the spellclick opcode
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 37927;
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(37927, 68503, 1, 0);
UPDATE `creature_template` SET `npcflag` = `npcflag` | 16777216 WHERE `entry` IN (37927, 38615);

-- Gate the click-vehicles on their quests (parked bat 38615 -> summon 38540 already wired via 72472)
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 18 AND `SourceGroup` IN (37927, 38615);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(18, 38615, 72472, 0, 0, 9, 0, 24920, 0, 0, 0, 0, 0, '', 'Captured Riding Bat - requires Slowing the Inevitable taken'),
(18, 37927, 68503, 0, 0, 9, 0, 24681, 0, 0, 0, 0, 0, '', 'Glaive Thrower - requires They Have Allies, But So Do We taken');

-- Every Riding War Wolf carries a Wolfmaw Outrider (24681 kill credit source; 35 ph189 spawns)
DELETE FROM `vehicle_template_accessory` WHERE `entry` = 37939;
INSERT INTO `vehicle_template_accessory` (`entry`, `accessory_entry`, `seat_id`, `minion`, `description`, `summontype`, `summontimer`) VALUES
(37939, 37938, 0, 1, 'Riding War Wolf - Wolfmaw Outrider', 6, 30000);

-- Native accept-casts (summon-with-ride-back chains verified in 4.3.4 SpellEffect.dbc)
UPDATE `quest_template_addon` SET `SourceSpellID` = 69255 WHERE `ID` = 14465;  -- summon coach 36741, ride 69254 (+phase 184 aura)
UPDATE `quest_template_addon` SET `SourceSpellID` = 72470 WHERE `ID` = 24902;  -- summon Tobias Mistmantle 38507
UPDATE `quest_template_addon` SET `SourceSpellID` = 81877 WHERE `ID` = 26706;  -- summon Hippogryph 43751, auto-ride
