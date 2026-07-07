-- Kezan: parked Goblin Hot Rod (49131) and Goblin Trike (49132) are pure decoration on
-- retail: no vehicle kit, UNIT_FLAG_NOT_SELECTABLE (0x2000000), no spellclick (sniffed:
-- NpcFlags 0, HasVehicleCreate False, Flags 33554432 - same as Goblin Epic Trike 49133).
-- "Keys to the Hot Rod" (91551) summons a fresh Hot Rod (34840) instead; handled in kezan.cpp.
UPDATE `creature_template` SET `VehicleId`=0, `npcflag`=0, `unit_flags`=33554432, `ScriptName`='' WHERE `entry` IN (49131,49132);
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry`=49131;
DELETE FROM `vehicle_template_accessory` WHERE `entry` IN (49131,49132);
