-- ============================================================================
-- Abyssal Depths C++ bindings + support rows (vashjir_abyssal_depths.cpp)
-- DO NOT APPLY BLIND: coordinate with the SQL batch agents (spawns, creature_text,
-- SAI, phase_area rows are THEIR domain - contracts documented in comments below).
-- ============================================================================

-- ----------------------------------------------------------------------------
-- 1. ScriptName bindings
-- ----------------------------------------------------------------------------
-- 41840 "Merciless One" possessed player-double: scene director of Put It On.
-- VehicleId 1388 = Vehicle.dbc kit whose single seat 9269 is the ONLY seat in
-- the whole 4.3.4 DBC with AttachmentID 11 (head) - the faceless one (41814)
-- boards it via native 46598 Ride Vehicle Hardcoded.
UPDATE creature_template SET ScriptName = 'npc_abyssal_merciless_double', VehicleId = 1388 WHERE entry = 41840;

-- L'ghorek: runestone re-grant gossip (11607/0) + "L'ghorek Dies!" on 26181/26182
-- accept. NOTE for SQL agents: do NOT put SmartAI on 42197 - this C++ script owns
-- it. Needed from you: creature_text 42197 group 0 (Type 42 boss whisper
-- "L'ghorek Dies!"), gossip_menu/npc_text for 11607, conditions SourceType 15 on
-- gossip option 11607/0 (show only while 26154 or 26143 active), and deletion of
-- the duplicate spawn guid 348222.
UPDATE creature_template SET ScriptName = 'npc_abyssal_lghorek' WHERE entry = 42197;

-- Defending the Rift aftermath escorts (personal summons of RewardSpell 93268/93302)
UPDATE creature_template SET ScriptName = 'npc_abyssal_rift_escort' WHERE entry IN (50259, 50261);

-- ----------------------------------------------------------------------------
-- 2. Spell scripts
-- ----------------------------------------------------------------------------
DELETE FROM spell_script_names WHERE spell_id IN (94397, 93268, 93302);
INSERT INTO spell_script_names (spell_id, ScriptName) VALUES
(94397, 'spell_abyssal_put_it_on_reverse_cast'),
(93268, 'spell_abyssal_defending_the_rift_completion'),
(93302, 'spell_abyssal_defending_the_rift_completion');

-- ----------------------------------------------------------------------------
-- 3. Conditions: Put It On summon destinations
-- ----------------------------------------------------------------------------
-- Every scene summon spell uses implicit target 46 (TARGET_DEST_NEARBY_ENTRY);
-- the core HARD-FAILS the cast (SPELL_FAILED_BAD_IMPLICIT_TARGETS) without a
-- SourceType-13 OBJECT_ENTRY_GUID condition. Anchor the scene on the questgiver:
-- 41666 Engineer Hexascrub (A) / 41669 Fiasco Sizzlegrin (H). The shared
-- 77988/78008 get both anchors (ElseGroup OR).
DELETE FROM conditions WHERE SourceTypeOrReferenceId = 13
  AND SourceEntry IN (78021, 78083, 78085, 87752, 87760, 78022, 78084, 78086, 87787, 87789, 87792, 77988, 78008);
INSERT INTO conditions (SourceTypeOrReferenceId, SourceGroup, SourceEntry, SourceId, ElseGroup, ConditionTypeOrReference, ConditionTarget, ConditionValue1, ConditionValue2, ConditionValue3, NegativeCondition, ErrorType, ErrorTextId, ScriptName, Comment) VALUES
-- Alliance actors -> Hexascrub anchor
(13, 1, 78021, 0, 0, 31, 0, 3, 41666, 0, 0, 0, 0, '', 'Put It On: Summon Hexascrub double at Hexascrub'),
(13, 1, 78083, 0, 0, 31, 0, 3, 41666, 0, 0, 0, 0, '', 'Put It On: Summon Jorlan double at Hexascrub'),
(13, 1, 78085, 0, 0, 31, 0, 3, 41666, 0, 0, 0, 0, '', 'Put It On: Summon Foxy Topper double at Hexascrub'),
(13, 1, 87752, 0, 0, 31, 0, 3, 41666, 0, 0, 0, 0, '', 'Put It On: Summon Rallings double at Hexascrub'),
(13, 1, 87760, 0, 0, 31, 0, 3, 41666, 0, 0, 0, 0, '', 'Put It On: Summon Darkbreak Guard double at Hexascrub'),
-- Horde actors -> Sizzlegrin anchor
(13, 1, 78022, 0, 0, 31, 0, 3, 41669, 0, 0, 0, 0, '', 'Put It On: Summon Sizzlegrin double at Sizzlegrin'),
(13, 1, 78084, 0, 0, 31, 0, 3, 41669, 0, 0, 0, 0, '', 'Put It On: Summon Toldrek double at Sizzlegrin'),
(13, 1, 78086, 0, 0, 31, 0, 3, 41669, 0, 0, 0, 0, '', 'Put It On: Summon Gertrude double at Sizzlegrin'),
(13, 1, 87787, 0, 0, 31, 0, 3, 41669, 0, 0, 0, 0, '', 'Put It On: Summon Taley double at Sizzlegrin'),
(13, 1, 87789, 0, 0, 31, 0, 3, 41669, 0, 0, 0, 0, '', 'Put It On: Summon Nerius double at Sizzlegrin'),
(13, 1, 87792, 0, 0, 31, 0, 3, 41669, 0, 0, 0, 0, '', 'Put It On: Summon Cavern Grunt double at Sizzlegrin'),
-- shared Merciless One pair -> either anchor
(13, 1, 77988, 0, 0, 31, 0, 3, 41666, 0, 0, 0, 0, '', 'Put It On: Summon Merciless One controller (A anchor)'),
(13, 1, 77988, 0, 1, 31, 0, 3, 41669, 0, 0, 0, 0, '', 'Put It On: Summon Merciless One controller (H anchor)'),
(13, 1, 78008, 0, 0, 31, 0, 3, 41666, 0, 0, 0, 0, '', 'Put It On: Summon Merciless One double (A anchor)'),
(13, 1, 78008, 0, 1, 31, 0, 3, 41669, 0, 0, 0, 0, '', 'Put It On: Summon Merciless One double (H anchor)');

-- ----------------------------------------------------------------------------
-- 4. Torrent support (26154 / 26143)
-- ----------------------------------------------------------------------------
-- Sniffed SET_VEHICLE_REC_ID 1342 (verified present in 4.3.4 Vehicle.dbc).
-- 42325 bar = sniff-ordered pre-release set; 48620 = released set (26143).
-- npc_abyssal_rift C++ swaps 42325 -> 48620 on 26143 accept via EnterVehicle.
UPDATE creature_template SET VehicleId = 1342, spell1 = 78972, spell2 = 78968, spell3 = 79213, spell4 = 79012, spell5 = 90677, spell6 = 0 WHERE entry = 42325;
UPDATE creature_template SET VehicleId = 1342, spell1 = 79222, spell2 = 79223, spell3 = 78968, spell4 = 78972, spell5 = 79224, spell6 = 90677 WHERE entry = 48620;

-- 26143 RewardSpell 79052 "All that Rises: Quest Complete" is a serverside stub
-- with no effects in 4.3.4 - the C++ PlayerScript ejects/despawns the torrent on
-- reward instead. Never let the client be told to cast it.
UPDATE quest_template SET RewardSpell = 0, RewardDisplaySpell = 0 WHERE ID = 26143;

-- ============================================================================
-- CONTRACTS FOR THE SQL AGENTS (rows owned by them, consumed by this C++)
-- ============================================================================
-- creature_text group maps (BroadcastTextIds in WPP world.sql, 14987/14989/14993...):
--  41840 Merciless One double (shared A/H):
--    0 "I SEE YOU."  1 "DIE."  2 "YOUR SIMPLE MIND CANNOT GRASP WHAT IS TRANSPIRING."  (yells)
--  41837 Hexascrub double / 41852 Sizzlegrin double (H = adapted copies):
--    0 "Oh gods! It's not dead! RUN!"  1 "We're all going to die!"
--    2 "It's all your fault! It was supposed to be dead! Now it ate your brains!"  3 "Mommy!"
--  41884 Jorlan double / 41886 Toldrek double:
--    0 "What the... $n?"  1 "You fools stop running around! Face it!"
--    2 "Somebody knock that thing off of $n's head!"
--  41889 Foxy Topper double / 41887 Gertrude double:
--    0 "This is marbles and conkers!"  1 "Nutmegs don't fail me now!"
--    2 "You're all fore and aft for putting that on!"
--  42197 L'ghorek: 0 = Type 42 RaidBossWhisper "L'ghorek Dies!"
--  48620 Vengeful Torrent: 0 = whisper "%s's bindings have been released! Full powers unlocked."
--                          1 = yell "I am freed! Let us slay Hallazeal in Neptulon's name, $n. He lurks within the temple."
--  50259 Captain Taylor / 50261 Legionnaire Nazgrim (escort): 0 = say (emote 1)
--    "I'm going in after Erunak. Follow me, $n!" (H line = adapted mirror)
--
-- Gossip replay hook ("Hexascrub, let me see that merciless one again.",
-- menu 11535 option 0; Horde mirror 11536): SAI on 41666/41669 gossip select ->
-- invoker (player) casts the faction summon chain IN THIS ORDER, all triggered:
--   A: 78021, 78083, 78085, 87752, 87760, 77988, then 78008 LAST
--   H: 78022, 78084, 78086, 87787, 87789, 87792, 77988, then 78008 LAST
-- (78008's AI = scene director; it self-runs and needs no quest state - safe replay.)
--
-- Phase 233 is used by the C++ as the personal Put It On scene phase (added to
-- the player for ~40 s). It coexists with the AB4 breach-battle staging use of
-- 233 (different subzone, phase_area-driven).
--
-- Defending the Rift: 44490/44540 aftermath turn-in spawns belong to phase 234
-- (phase_area, quest state complete); the 50259/50261 escort is summoned by
-- RewardSpell 93268/93302 (script above) as a PRIVATE object - no spawns needed.
-- 26193/26194 battle credit (42819 x15 objective + KillCredit1 rows) per AB4.
