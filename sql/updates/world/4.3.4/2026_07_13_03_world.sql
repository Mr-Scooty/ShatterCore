-- Kezan: "The Great Bank Heist" (14122) - the five vault-cracking tools could not be cast,
-- the vault console could be driven around, and the feigned heist-phase Bank Tellers kept
-- yelling "Next!".
-- Retail (Goblin_P2 sniff): the seated player casts the tools at the vault
-- (TARGET_UNIT_VEHICLE, handled by a CMSG_PET_CAST_SPELL redirect in core); the vault answers
-- with 67493/67494 (+/-10 mana) and its mana bar is the progress bar (max 100, no passive
-- regen, -5 decay every ~5s); the console spawns rooted (create block: Root|DisableGravity)
-- so the controlling player can turn but not drive it.

-- Vault console 35486: mana class + no passive power regen (unit_flags2 0x800 would refill
-- the progress bar), retail immune/unselectable flags.
UPDATE `creature_template` SET `unit_class`=8, `unit_flags`=0x2000300, `unit_flags2`=0 WHERE `entry`=35486;

-- Rooted at spawn, like the Necessary Roughness boats.
DELETE FROM `creature_template_movement` WHERE `CreatureId`=35486;
INSERT INTO `creature_template_movement` (`CreatureId`, `Ground`, `Swim`, `Flight`, `Rooted`, `Random`, `InteractionPauseTimer`) VALUES
(35486, NULL, NULL, NULL, 1, NULL, NULL);

-- 67494 "Power Incorrect" is a -10 mana energize; the core drops negative energizes.
DELETE FROM `spell_script_names` WHERE `spell_id`=67494;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(67494, 'spell_kezan_vault_power_incorrect');

-- Heist-phase (383) FBoK Bank Tellers 35120 lie feigned dead (creature_addon aura 29266) but
-- still ran the entry SmartAI: OOC "Next!" yell + gossip. Guid-scoped SAI replaces the entry
-- script for these two spawns and clears their npcflag (retail: NpcFlags 0).
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (-253140,-253146) AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(-253140, 0, 0, 0, 63, 0, 100, 0, 0, 0, 0, 0, 0, 81, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'FBoK Bank Teller (heist corpse) - Just Created - Set npcflag none'),
(-253146, 0, 0, 0, 63, 0, 100, 0, 0, 0, 0, 0, 0, 81, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'FBoK Bank Teller (heist corpse) - Just Created - Set npcflag none');
