-- Vashj'ir zone infrastructure: Sea Legs spell_area, vehicle kits, spellclicks,
-- vehicle accessories, intro quest chain fixes, Horde wake-up teleport position.

-- 1) Sea Legs (73701) zone-wide aura, granted once the faction "Sea Legs" quest is rewarded.
--    75966 is applied via SPELL_AURA_LINKED on 73701 - do not add it here.
--    Zone 5146 is the parent zone for all of Vashj'ir (GetZoneId() everywhere in the zone).
DELETE FROM `spell_area` WHERE `spell`=73701 AND `area`=5146;
INSERT INTO `spell_area` (`spell`, `area`, `quest_start`, `quest_end`, `aura_spell`, `racemask`, `gender`, `flags`, `quest_start_status`, `quest_end_status`) VALUES
(73701, 5146, 24432, 0, 0, 0, 2, 1, 64, 64), -- Alliance: Sea Legs (24432) rewarded
(73701, 5146, 25929, 0, 0, 0, 2, 1, 64, 64); -- Horde: Sea Legs (25929) rewarded

-- 2) Vehicle kits verified against retail sniff create-blocks (RecID) + 4.3.4 Vehicle.dbc
UPDATE `creature_template` SET `VehicleId`=559 WHERE `entry`=36878; -- Sea Monster Tentacle (player grab)
UPDATE `creature_template` SET `VehicleId`=561 WHERE `entry`=36901; -- Submerge Bunny (drag-down, seats 1-3)
UPDATE `creature_template` SET `VehicleId`=569 WHERE `entry`=37001; -- Drowning Crewman
UPDATE `creature_template` SET `VehicleId`=738 WHERE `entry` IN (40351,40353); -- Meatstick 00/01
UPDATE `creature_template` SET `VehicleId`=812 WHERE `entry`=40596; -- Spinning Trident Bunny
UPDATE `creature_template` SET `VehicleId`=778 WHERE `entry`=40761; -- Secondary Diving Tank
UPDATE `creature_template` SET `VehicleId`=803 WHERE `entry`=41048; -- Booby-Trapped Bait
UPDATE `creature_template` SET `VehicleId`=808 WHERE `entry` IN (41150,41154); -- Gnaws Blood/Harpoon Bunny
UPDATE `creature_template` SET `VehicleId`=845 WHERE `entry`=41907; -- Humphrey Digsong
UPDATE `creature_template` SET `VehicleId`=1342 WHERE `entry`=42361; -- Ascended Zealot
UPDATE `creature_template` SET `VehicleId`=946 WHERE `entry`=43279; -- Ammo Cart Bunny
UPDATE `creature_template` SET `VehicleId`=1173 WHERE `entry`=45847; -- S.A.F.E. Operative
UPDATE `creature_template` SET `VehicleId`=804 WHERE `entry`=46403; -- Gnaws Mouth Blood Bunny

-- 3) Spellclicks from retail sniff
DELETE FROM `npc_spellclick_spells` WHERE (`npc_entry`=39996 AND `spell_id`=86324) OR (`npc_entry`=40499 AND `spell_id`=75525);
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(39996, 86324, 1, 0), -- Abyssal Seahorse
(40499, 75525, 1, 0); -- Puffer Hatchling

-- 4) Vehicle accessories from retail sniff (carriers with verified kits only)
DELETE FROM `vehicle_template_accessory` WHERE `entry` IN (37008,41157,40223);
INSERT INTO `vehicle_template_accessory` (`entry`, `accessory_entry`, `seat_id`, `minion`, `description`, `summontype`, `summontimer`) VALUES
(37008, 37001, 0, 0, 'Zin''jatar Raider - Drowning Crewman', 6, 30000),
(41157, 41150, 1, 1, 'Gnaws'' Corpse - Gnaws Blood Bunny', 6, 30000),
(41157, 41154, 2, 1, 'Gnaws'' Corpse - Gnaws Harpoon Bunny', 6, 30000),
(40223, 40351, 1, 1, 'Speckled Sea Turtle - Meatstick 00', 6, 30000),
(40223, 40353, 2, 1, 'Speckled Sea Turtle - Meatstick 01', 6, 30000);

-- 5) Alliance "To the Depths" (28827) gate on "The Eye of the Storm" (28826), mirroring Horde 28816<-28805
UPDATE `quest_template_addon` SET `PrevQuestID`=28826 WHERE `ID`=28827;

-- 6) Missing request-items text for 28826 (from retail sniff WPP dump)
DELETE FROM `quest_request_items` WHERE `ID`=28826;
INSERT INTO `quest_request_items` (`ID`, `EmoteOnComplete`, `EmoteOnIncomplete`, `CompletionText`, `VerifiedBuild`) VALUES
(28826, 0, 0, 'Yes, $c?', 0);

-- 7) Horde wake-up teleport (73728 "Teleport Horde Player"), mirror of Alliance 73727.
--    Placed in front of Horde-cave Erunak (41618 @ -4611.18, 3985.19, -70.44, o 5.358).
DELETE FROM `spell_target_position` WHERE `ID`=73728;
INSERT INTO `spell_target_position` (`ID`, `EffectIndex`, `MapID`, `PositionX`, `PositionY`, `PositionZ`, `Orientation`, `VerifiedBuild`) VALUES
(73728, 0, 0, -4608.16, 3981.21, -70.8, 2.217, 0);
