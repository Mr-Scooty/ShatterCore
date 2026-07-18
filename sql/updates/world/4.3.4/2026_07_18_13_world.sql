-- ============================================================================
-- SHIMMERING EXPANSE - BATCH B (arcs S3 Nespirah / S5 Vision1+Reoccupation /
--                               S6 Battlemaiden2+Nazjar / S7 Finale)
-- Quests: 25890-25922 | 25760,25619/20/37/58/59,25747-25751 |
--         25752-25755,25858-25863,25892/93 | 25894-25898,25911,25626,
--         25896/25629/25860/25951,26005,26219
-- Guid blocks: creature 9001300-9001447, GO 9001060-9001081 (allocated caps:
--              9001300-9001449 / 9001060-9001099)
-- Custom PhaseIds used: 224 (surface rescue set), 228 (Voldrin's Hold staging)
-- Retail aura phases used: 170 (73974), 171 (77565), 172 (78263), 184 (78323)
-- phase_area quest phases: 179 (77359-equivalent), 180 (77665-equivalent)
-- Idempotent: every insert preceded by a scoped delete.
-- ============================================================================

-- ============================================================================
-- 1. QUEST CHAIN / TEMPLATE FIXES
-- ============================================================================
-- 25907/25908 wrongly chain to Horde 25989; retail Alliance chain -> 25909.
-- Positive ExclusiveGroup 25907 blocked parallel accept; retail runs both in
-- parallel and requires both for 25909 -> negative group.
UPDATE quest_template SET RewardNextQuest = 25909 WHERE ID IN (25907,25908) AND RewardNextQuest = 25989;
UPDATE quest_template_addon SET NextQuestID = 25909, ExclusiveGroup = -25907 WHERE ID IN (25907,25908);

-- 25890 "Nespirah": objective-less travel quest completed by areatrigger 5958.
UPDATE quest_template_addon SET SpecialFlags = SpecialFlags | 2 WHERE ID = 25890;
DELETE FROM areatrigger_involvedrelation WHERE id = 5958;
INSERT INTO areatrigger_involvedrelation (id, quest) VALUES (5958, 25890);

-- 25900 "Making Contact" / 25916 "Breaking Through": retail completes these
-- serverside with no client credit. Fork: convert to hidden kill-credit
-- objectives granted by the RP scripts (SAI action 33).
UPDATE quest_template SET RequiredNpcOrGo1 = 41531, RequiredNpcOrGoCount1 = 1,
  ObjectiveText1 = 'Witness Duarn''s attempt to speak with Nespirah' WHERE ID = 25900;
UPDATE quest_template SET RequiredNpcOrGo1 = 41633, RequiredNpcOrGoCount1 = 1,
  ObjectiveText1 = 'Listen to the Voice of Nespirah' WHERE ID = 25916;

-- Flags |= 0x400000 (UPDATE_PHASE_SHIFT) on quests that drive phase_area
-- conditions so the client re-requests phase on state change.
UPDATE quest_template SET Flags = Flags | 0x400000 WHERE ID IN (25752,25755,25898,25911,26005,26219);

-- 25898 retail SourceSpellID (See Quest Invis 5) - harmless with the phase_area
-- port, kept for fidelity.
UPDATE quest_template_addon SET SourceSpellID = 77861 WHERE ID = 25898;

-- Bogus RewardSpell 78492 (not in 4.3.4 Spell.dbc - verified) on 26017/26088.
UPDATE quest_template SET RewardSpell = 0 WHERE ID IN (26017,26088) AND RewardSpell = 78492;

-- ============================================================================
-- 2. QUEST TEXT TABLES (quest_details / quest_request_items) - sniffed values
-- ============================================================================
DELETE FROM quest_details WHERE ID IN (25890,25900,25907,25908,25909,25916,25917,25918,25919,25920,25921,25922,25747,25748,25749,25751,25752,25753,25754,25755,25760,25892,25893,25894,25895,25897,25898,25911,25626,26005,26219);
INSERT INTO quest_details (ID, Emote1, Emote2, Emote3, Emote4, EmoteDelay1, EmoteDelay2, EmoteDelay3, EmoteDelay4, VerifiedBuild) VALUES
(25890,1,1,1,0,0,0,0,0,0),
(25900,1,1,0,0,0,0,0,0,0),
(25907,1,1,0,0,0,0,0,0,0),
(25908,0,0,0,0,0,0,0,0,0),
(25909,1,1,0,0,0,0,0,0,0),
(25916,1,1,1,0,0,0,0,0,0),
(25917,1,1,1,0,0,0,0,0,0),
(25918,1,15,390,1,0,0,0,0,0),
(25919,0,0,0,0,0,0,0,0,0),
(25920,0,0,0,0,0,0,0,0,0),
(25921,0,0,0,0,0,0,0,0,0),
(25922,0,0,0,0,0,0,0,0,0),
(25747,1,0,0,0,0,0,0,0,0),
(25748,1,1,1,0,0,60,60,0,0),
(25749,1,0,0,0,0,0,0,0,0),
(25751,5,0,0,0,0,0,0,0,0),
(25752,1,0,0,0,0,0,0,0,0),
(25753,36,0,0,0,0,0,0,0,0),
(25754,1,0,0,0,0,0,0,0,0),
(25755,1,0,0,0,0,0,0,0,0),
(25760,1,0,0,0,0,0,0,0,0),
(25892,1,0,0,0,0,0,0,0,0),
(25893,1,0,0,0,0,0,0,0,0),
(25894,1,0,0,0,0,0,0,0,0),
(25895,1,0,0,0,0,0,0,0,0),
(25897,1,0,0,0,0,0,0,0,0),
(25898,1,0,0,0,0,0,0,0,0),
(25911,1,0,0,0,0,0,0,0,0),
(25626,1,0,0,0,0,0,0,0,0),
(26005,1,0,0,0,0,0,0,0,0),
(26219,1,0,0,0,0,0,0,0,0);

DELETE FROM quest_request_items WHERE ID IN (25751,25909,25918,25920,25895,25897);
INSERT INTO quest_request_items (ID, EmoteOnComplete, EmoteOnIncomplete, CompletionText, VerifiedBuild) VALUES
(25751,5,0,'Tick, tock, tick, tock. Always in such a hurry. No time for innovation. What I wouldn\'t give for a lab again!',0),
(25909,0,0,'I hope this works.  My skills as a shaman are limited, and I fear that this task may be too much for us.',0),
(25918,0,0,'In my youth, I was given some training on spearfighting.$b$b"Stick them with the pointy end," I think it was.',0),
(25920,0,0,'The naga must have some ulterior motive with these pearls.  If I had time to study them, I\'m certain I could figure it out.',0),
(25895,0,0,'A whole big box of amazing! That\'s what it is. I need more amazing in my life.$B$BDon\'t worry, I\'ll share.',0),
(25897,0,0,'The rope ties the whole plan together!',0);

-- ============================================================================
-- 3. ITEM STORE: 55171 Blade of the Naz'jar Battlemaiden
--    (present in client Item.db2, absent from Item-sparse.db2 -> StartItem of
--     25760/25755/25626 cannot function without a hotfix row)
-- ============================================================================
DELETE FROM hotfixes.item_sparse WHERE ID = 55171;
INSERT INTO hotfixes.item_sparse (ID, Quality, Flags1, Flags2, BuyCount, BuyPrice, SellPrice, InventoryType, AllowableClass, AllowableRace, ItemLevel, RequiredLevel, MaxCount, Stackable, DamageType, Delay, SpellID1, SpellTrigger1, SpellCharges1, SpellCooldown1, SpellCategory1, SpellCategoryCooldown1, SpellCooldown2, SpellCategoryCooldown2, SpellCooldown3, SpellCategoryCooldown3, SpellCooldown4, SpellCategoryCooldown4, SpellCooldown5, SpellCategoryCooldown5, Bonding, Display, Material, VerifiedBuild)
VALUES (55171, 1, 0, 0, 1, 0, 0, 0, -1, -1, 1, 0, 1, 1, 0, 0, 77292, 0, 0, 3000, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, 4, 'Blade of the Naz\'jar Battlemaiden', 1, 15595);

-- ============================================================================
-- 4. SPELL SUPPORT: spell_target_position / spell_area / spell_linked_spell
-- ============================================================================
-- TARGET_DEST_DB summons (TargA=17) - sniffed retail destinations:
DELETE FROM spell_target_position WHERE ID IN (77621,77936,77959,77322,79239);
INSERT INTO spell_target_position (ID, EffectIndex, MapID, PositionX, PositionY, PositionZ, Orientation, VerifiedBuild) VALUES
(77621, 0, 0, -6443.3, 4183.1, -422.6, 5.5, 0),   -- Summon Voice of Nespirah (41633)
(77959, 0, 0, -6443.3, 4183.1, -422.6, 5.5, 0),   -- Summon Voice of Nespirah II (41801)
(77936, 0, 0, -6468.9, 4166.3, -425.3, 0.2, 0),   -- Summon Erunak (41788)
(77322, 0, 0, -7006.82, 5072.01, -609.88, 1.0, 0),-- Summon Tamed Bombing Ray (41247)
(79239, 0, 0, -6877.94, 6032.79, -610.70, 4.6, 0);-- Move Alliance Occupants to Land (Darkbreak Cove)

-- 25890 tunnel escort: entering Nespirah (area 4962) with 25890 incomplete
-- casts 77963 (summons Erunak 41803 + dummy marker aura = no double-summon).
-- 41803's SAI summons Duarn 41532. Completion itself = areatrigger 5958.
DELETE FROM spell_area WHERE spell = 77963 AND area = 4962;
INSERT INTO spell_area (spell, area, quest_start, quest_end, aura_spell, racemask, gender, flags, quest_start_status, quest_end_status) VALUES
(77963, 4962, 25890, 25890, 0, 0, 2, 1, 8, 66);

-- 25922 escape seahorse: spellclick 77927 (kill credit 41776 native) has its
-- summon in a dummy effect -> link the Alliance summon-and-ride 77920.
DELETE FROM spell_linked_spell WHERE spell_trigger = 77927 AND spell_effect = 77920;
INSERT INTO spell_linked_spell (spell_trigger, spell_effect, type, comment) VALUES
(77927, 77920, 0, 'Summon Escape Seahorse (Master) -> Summon Escape Seahorse (Alliance)');

-- ============================================================================
-- 5. CONDITIONS
-- ============================================================================
-- 5a. Spell implicit-target conditions (source 13, nearby/area-entry targets)
DELETE FROM conditions WHERE SourceTypeOrReferenceId = 13 AND SourceEntry IN (77292,73974,77565,78264,77664,77554,77843,77641,77653,77654,78268,78329,77741);
INSERT INTO conditions (SourceTypeOrReferenceId, SourceGroup, SourceEntry, SourceId, ElseGroup, ConditionTypeOrReference, ConditionTarget, ConditionValue1, ConditionValue2, ConditionValue3, NegativeCondition, ErrorType, ErrorTextId, ScriptName, Comment) VALUES
(13,1,77292,0,0,31,0,3,41160,0,0,0,0,'','Blade of the Nazjar Battlemaiden targets Vision 1 bunny'),
(13,1,77292,0,1,31,0,3,41436,0,0,0,0,'','Blade of the Nazjar Battlemaiden targets Vision 2 bunny'),
(13,1,77292,0,2,31,0,3,41484,0,0,0,0,'','Blade of the Nazjar Battlemaiden targets Vision 3 bunny'),
(13,1,73974,0,0,31,0,3,41160,0,0,0,0,'','Nazjar Battlemaiden V1: summon at Vision 1 bunny'),
(13,1,77565,0,0,31,0,3,41436,0,0,0,0,'','Nazjar Battlemaiden V2: summon at Vision 2 bunny'),
(13,1,78264,0,0,31,0,3,41484,0,0,0,0,'','Nazjar Battlemaiden V3: summon at Vision 3 bunny'),
(13,1,77664,0,0,31,0,3,41494,0,0,0,0,'','Throw Rope targets Enslaved Alliance Pearl Miner'),
(13,1,77664,0,1,31,0,3,41495,0,0,0,0,'','Throw Rope targets Enslaved Horde Pearl Miner'),
(13,1,77554,0,0,31,0,3,41200,0,0,0,0,'','Nespirah Channel Beam targets Generic Bunny PRK'),
(13,2,77843,0,0,31,0,3,41200,0,0,0,0,'','Broken Resolve channel targets Generic Bunny PRK'),
(13,1,77641,0,0,31,0,3,41457,0,0,0,0,'','Kvaldir Execution Ping hits Executioner Verathress'),
(13,1,77653,0,0,31,0,3,41537,0,0,0,0,'','Subjugation targets Kvaldir High-Shaman'),
(13,1,77654,0,0,31,0,3,41537,0,0,0,0,'','Execution targets Kvaldir High-Shaman'),
(13,1,78268,0,0,31,0,4,0,0,0,0,0,'','Honor Guard Orders Credit hits players'),
(13,2,78268,0,0,31,0,4,0,0,0,0,0,'','Honor Guard Orders Credit hits players'),
(13,1,78329,0,0,31,0,4,0,0,0,0,0,'','Forcecast Phase Shift 9 hits players'),
(13,2,78329,0,0,31,0,4,0,0,0,0,0,'','Phase Shift 9 kill credit 42135 hits players'),
(13,1,77741,0,0,31,0,3,41572,0,0,0,0,'','Rescue Flare dummy hits Rescue Balloon');

-- 5b. Gossip option conditions (source 15)
DELETE FROM conditions WHERE SourceTypeOrReferenceId = 15 AND SourceGroup IN (11525,11477,11481,11515,11516,11517,11571,11572);
INSERT INTO conditions (SourceTypeOrReferenceId, SourceGroup, SourceEntry, SourceId, ElseGroup, ConditionTypeOrReference, ConditionTarget, ConditionValue1, ConditionValue2, ConditionValue3, NegativeCondition, ErrorType, ErrorTextId, ScriptName, Comment) VALUES
(15,11525,0,0,0,9,0,25900,0,0,0,0,0,'','Duarn opt0 - Making Contact taken'),
(15,11525,1,0,0,9,0,25916,0,0,0,0,0,'','Duarn opt1 - Breaking Through taken'),
(15,11477,0,0,0,9,0,25752,0,0,0,0,0,'','Hexascrub assault option - Swift Action taken'),
(15,11481,0,0,0,9,0,25753,0,0,0,0,0,'','Injured Volunteer rescue option - 25753 taken'),
(15,11515,0,0,0,9,0,25858,0,0,0,0,0,'','Zinjatar option - By Her Ladys Word taken'),
(15,11516,0,0,0,9,0,25858,0,0,0,0,0,'','Overseer option - By Her Ladys Word taken'),
(15,11517,0,0,0,9,0,25858,0,0,0,0,0,'','Lady Sirakess option - By Her Ladys Word taken'),
(15,11571,0,0,0,9,0,25896,0,0,0,0,0,'','Tide Priestess option - Devout Assembly taken'),
(15,11572,0,0,0,9,0,25896,0,0,0,0,0,'','Azrajar option - Devout Assembly taken');

-- 5c. Spellclick conditions (source 18)
DELETE FROM conditions WHERE SourceTypeOrReferenceId = 18 AND SourceGroup IN (41776,41520);
INSERT INTO conditions (SourceTypeOrReferenceId, SourceGroup, SourceEntry, SourceId, ElseGroup, ConditionTypeOrReference, ConditionTarget, ConditionValue1, ConditionValue2, ConditionValue3, NegativeCondition, ErrorType, ErrorTextId, ScriptName, Comment) VALUES
(18,41776,77927,0,0,9,0,25922,0,0,0,0,0,'','Escape seahorse click - Waking the Beast taken'),
(18,41520,77684,0,0,9,0,25909,0,0,0,0,0,'','Crab click - Capture the Crab taken'),
(18,41520,77684,0,0,1,1,77682,0,0,0,0,0,'','Crab click - crab netted (aura 77682)');

-- ============================================================================
-- 6. PHASING (phase_area + source-26 conditions)
-- Areas: 4967 Ruins of Vashjir, 4968 QuelDormir Terrace, 5090/5124 temple and
-- ridge subzones, 4966 Bielaran Ridge, 4969 Tranquil Wash, 4955 Voldrins Hold.
-- Phase 179 = terrace assault era (25752 taken -> 25755 rewarded)
-- Phase 180 = aftermath era (25755 rewarded)
-- Phase 224 = 25898 surface rescue set, phase 228 = 26005/26219 sub staging.
-- Aura-negatives keep vision content (170/171/172 rides) uncluttered.
-- ============================================================================
DELETE FROM phase_area WHERE PhaseId IN (179,180) AND AreaId IN (4967,4968,5090,5124,4966,4969);
DELETE FROM phase_area WHERE PhaseId = 224 AND AreaId IN (4966,4969);
DELETE FROM phase_area WHERE PhaseId = 228 AND AreaId IN (4955);
INSERT INTO phase_area (AreaId, PhaseId, Comment) VALUES
(4967,179,'Shimmering - ruins assault era'),
(4968,179,'Shimmering - ruins assault era'),
(5090,179,'Shimmering - ruins assault era'),
(5124,179,'Shimmering - ruins assault era'),
(4967,180,'Shimmering - ruins aftermath era'),
(4968,180,'Shimmering - ruins aftermath era'),
(5090,180,'Shimmering - ruins aftermath era'),
(5124,180,'Shimmering - ruins aftermath era'),
(4966,180,'Shimmering - ruins aftermath era (Bielaran/Tranquil camps)'),
(4969,180,'Shimmering - ruins aftermath era (Bielaran/Tranquil camps)'),
(4966,224,'Shimmering - 25898 surface rescue set'),
(4969,224,'Shimmering - 25898 surface rescue set'),
(4955,228,'Shimmering - Voldrins Hold sub staging');

DELETE FROM conditions WHERE SourceTypeOrReferenceId = 26 AND SourceGroup IN (179,180) AND SourceEntry IN (4967,4968,5090,5124,4966,4969);
DELETE FROM conditions WHERE SourceTypeOrReferenceId = 26 AND SourceGroup = 224 AND SourceEntry IN (4966,4969);
DELETE FROM conditions WHERE SourceTypeOrReferenceId = 26 AND SourceGroup = 228 AND SourceEntry IN (4955);
INSERT INTO conditions (SourceTypeOrReferenceId, SourceGroup, SourceEntry, SourceId, ElseGroup, ConditionTypeOrReference, ConditionTarget, ConditionValue1, ConditionValue2, ConditionValue3, NegativeCondition, ErrorType, ErrorTextId, ScriptName, Comment) VALUES
-- phase 179: 25752 taken/complete/rewarded AND 25755 not rewarded AND not in a vision
(26,179,4967,0,0,47,0,25752,74,0,0,0,0,'','P179: Swift Action started'),
(26,179,4967,0,0,8,0,25755,0,0,1,0,0,'','P179: Slaughter of Bielaran not rewarded'),
(26,179,4967,0,0,1,0,73974,0,0,1,0,0,'','P179: not in vision 1'),
(26,179,4967,0,0,1,0,77565,0,0,1,0,0,'','P179: not in vision 2'),
(26,179,4967,0,0,1,0,78264,0,0,1,0,0,'','P179: not in vision 3'),
(26,179,4968,0,0,47,0,25752,74,0,0,0,0,'','P179: Swift Action started'),
(26,179,4968,0,0,8,0,25755,0,0,1,0,0,'','P179: Slaughter of Bielaran not rewarded'),
(26,179,4968,0,0,1,0,73974,0,0,1,0,0,'','P179: not in vision 1'),
(26,179,4968,0,0,1,0,77565,0,0,1,0,0,'','P179: not in vision 2'),
(26,179,4968,0,0,1,0,78264,0,0,1,0,0,'','P179: not in vision 3'),
(26,179,5090,0,0,47,0,25752,74,0,0,0,0,'','P179: Swift Action started'),
(26,179,5090,0,0,8,0,25755,0,0,1,0,0,'','P179: Slaughter of Bielaran not rewarded'),
(26,179,5090,0,0,1,0,73974,0,0,1,0,0,'','P179: not in vision 1'),
(26,179,5090,0,0,1,0,77565,0,0,1,0,0,'','P179: not in vision 2'),
(26,179,5090,0,0,1,0,78264,0,0,1,0,0,'','P179: not in vision 3'),
(26,179,5124,0,0,47,0,25752,74,0,0,0,0,'','P179: Swift Action started'),
(26,179,5124,0,0,8,0,25755,0,0,1,0,0,'','P179: Slaughter of Bielaran not rewarded'),
(26,179,5124,0,0,1,0,73974,0,0,1,0,0,'','P179: not in vision 1'),
(26,179,5124,0,0,1,0,77565,0,0,1,0,0,'','P179: not in vision 2'),
(26,179,5124,0,0,1,0,78264,0,0,1,0,0,'','P179: not in vision 3'),
-- phase 180: 25755 rewarded AND not in a vision
(26,180,4967,0,0,8,0,25755,0,0,0,0,0,'','P180: Slaughter of Bielaran rewarded'),
(26,180,4967,0,0,1,0,77565,0,0,1,0,0,'','P180: not in vision 2'),
(26,180,4967,0,0,1,0,78264,0,0,1,0,0,'','P180: not in vision 3'),
(26,180,4968,0,0,8,0,25755,0,0,0,0,0,'','P180: Slaughter of Bielaran rewarded'),
(26,180,4968,0,0,1,0,77565,0,0,1,0,0,'','P180: not in vision 2'),
(26,180,4968,0,0,1,0,78264,0,0,1,0,0,'','P180: not in vision 3'),
(26,180,5090,0,0,8,0,25755,0,0,0,0,0,'','P180: Slaughter of Bielaran rewarded'),
(26,180,5090,0,0,1,0,77565,0,0,1,0,0,'','P180: not in vision 2'),
(26,180,5090,0,0,1,0,78264,0,0,1,0,0,'','P180: not in vision 3'),
(26,180,5124,0,0,8,0,25755,0,0,0,0,0,'','P180: Slaughter of Bielaran rewarded'),
(26,180,5124,0,0,1,0,77565,0,0,1,0,0,'','P180: not in vision 2'),
(26,180,5124,0,0,1,0,78264,0,0,1,0,0,'','P180: not in vision 3'),
(26,180,4966,0,0,8,0,25755,0,0,0,0,0,'','P180: Slaughter of Bielaran rewarded'),
(26,180,4966,0,0,1,0,77565,0,0,1,0,0,'','P180: not in vision 2'),
(26,180,4966,0,0,1,0,78264,0,0,1,0,0,'','P180: not in vision 3'),
(26,180,4969,0,0,8,0,25755,0,0,0,0,0,'','P180: Slaughter of Bielaran rewarded'),
(26,180,4969,0,0,1,0,77565,0,0,1,0,0,'','P180: not in vision 2'),
(26,180,4969,0,0,1,0,78264,0,0,1,0,0,'','P180: not in vision 3'),
-- phase 224: 25898 active OR (25898 rewarded AND 25911 not rewarded)
(26,224,4966,0,0,47,0,25898,10,0,0,0,0,'','P224: Honor and Privilege in log'),
(26,224,4966,0,1,8,0,25898,0,0,0,0,0,'','P224: Honor and Privilege rewarded'),
(26,224,4966,0,1,8,0,25911,0,0,1,0,0,'','P224: Welcome News not yet rewarded'),
(26,224,4969,0,0,47,0,25898,10,0,0,0,0,'','P224: Honor and Privilege in log'),
(26,224,4969,0,1,8,0,25898,0,0,0,0,0,'','P224: Honor and Privilege rewarded'),
(26,224,4969,0,1,8,0,25911,0,0,1,0,0,'','P224: Welcome News not yet rewarded'),
-- phase 228: 26005 started -> 26219 rewarded
(26,228,4955,0,0,47,0,26005,74,0,0,0,0,'','P228: A Breath of Fresh Air started'),
(26,228,4955,0,0,8,0,26219,0,0,1,0,0,'','P228: Full Circle not rewarded');

-- ============================================================================
-- 7. CREATURE TEMPLATE FIXES (vehicles, ability bars, AI hookup, npcflags)
-- ============================================================================
-- Vehicle RecIDs sniffed from create blocks, all verified in 4.3.4 Vehicle.dbc
UPDATE creature_template SET VehicleId = 694,  spell1 = 75678, spell2 = 76664 WHERE entry = 39584; -- Battlemaiden vision 1
UPDATE creature_template SET VehicleId = 812,  spell1 = 75678, spell2 = 75684, spell3 = 76619, spell4 = 76664, spell5 = 76569 WHERE entry = 41225; -- Battlemaiden vision 2
UPDATE creature_template SET VehicleId = 848 WHERE entry = 41986;  -- Battlemaiden vision 3 (empty pet bar - player uses own spellbook)
UPDATE creature_template SET VehicleId = 816 WHERE entry = 41247;  -- Tamed Bombing Ray (spell1 77330 already set)
UPDATE creature_template SET VehicleId = 840 WHERE entry = 41785;  -- Swiftfin escape seahorse
UPDATE creature_template SET VehicleId = 1355 WHERE entry = 48898; -- Tamed Seahorse 25892

-- SmartAI hookups (exclusive-arc entries + shared hub NPCs used per-guid)
UPDATE creature_template SET AIName = 'SmartAI' WHERE entry IN
(41531,41633,41801,41788,41803,41532,41494,41495,41785,41115,41049,41455,42071,42072,41457,41985,41980,41999,42057,42066,42077,41562,41535,42411,41221,42486,41436,41160,41484,41572,41247,41235,46470,41259,44413,42073,42075) AND AIName = '';
UPDATE creature_template SET AIName = 'SmartAI' WHERE entry IN (39881,40639,40645,40789) AND AIName = '';

-- Spellclick flags
UPDATE creature_template SET npcflag = npcflag | 16777216 WHERE entry IN (41520,48901);

-- npc_spellclick_spells: fix 41776 (86358 -> 77927), add crab + tamed seahorse
DELETE FROM npc_spellclick_spells WHERE npc_entry IN (41776,41520,48901);
INSERT INTO npc_spellclick_spells (npc_entry, spell_id, cast_flags, user_type) VALUES
(41776, 77927, 1, 0),  -- Swiftfin Seahorse: Summon Escape Seahorse (Master), clicker casts
(41520, 77684, 1, 0),  -- Deepseeker Crab: Loot Deepseeker Crab (creates item 56182)
(48901, 91172, 1, 0);  -- Tamed Seahorse: Summon Tamed Sea Horse (vehicle 48898)

-- vehicle accessory: Spinning Trident Bunny rides the Battlemaidens (WPP)
DELETE FROM vehicle_template_accessory WHERE entry IN (39584,41225) AND seat_id = 1;
INSERT INTO vehicle_template_accessory (entry, accessory_entry, seat_id, minion, description, summontype, summontimer) VALUES
(39584, 40596, 1, 1, 'Nazjar Battlemaiden V1 - Spinning Trident Bunny', 6, 30000),
(41225, 40596, 1, 1, 'Nazjar Battlemaiden V2 - Spinning Trident Bunny', 6, 30000);

-- Loot: quest-flag the Kvaldir rope / Idrakess pearl drops (rows exist at 75%)
UPDATE creature_loot_template SET QuestRequired = 1 WHERE Item = 56183 AND Entry IN (41569,41606);
UPDATE creature_loot_template SET QuestRequired = 1 WHERE Item = 56194 AND Entry IN (41607,41608);

-- Nespirah Abscess (25919): goober natively casts 77834 -> player casts 77750
-- (dummy 50% + dead ScriptEffect). GO-use SAI events are outside the fork's
-- safe enum set, so wire the ambusher via spell_linked_spell instead
-- (77750 -> 77719 summons 41579 at the nearby abscess; retail 50% roll becomes
-- 100% - flagged in report).
DELETE FROM spell_linked_spell WHERE spell_trigger = 77750 AND spell_effect = 77719;
INSERT INTO spell_linked_spell (spell_trigger, spell_effect, type, comment) VALUES
(77750, 77719, 0, 'Idrakess Ambusher 50pct Hit -> Summon Idrakess Ambusher');
DELETE FROM conditions WHERE SourceTypeOrReferenceId = 13 AND SourceEntry = 77719;
INSERT INTO conditions (SourceTypeOrReferenceId, SourceGroup, SourceEntry, SourceId, ElseGroup, ConditionTypeOrReference, ConditionTarget, ConditionValue1, ConditionValue2, ConditionValue3, NegativeCondition, ErrorType, ErrorTextId, ScriptName, Comment) VALUES
(13,1,77719,0,0,31,0,5,203309,0,0,0,0,'','Summon Idrakess Ambusher - dest at nearby Nespirah Abscess');

-- Reposition Engineer Hexascrub's existing spawn (wrong coords -6347/-682) to
-- the retail staging camp (giver of 25751, gossip target of 25752).
UPDATE creature SET zoneId = 5144, areaId = 4967, position_x = -6994.74, position_y = 5085.09, position_z = -609.92, orientation = 0.9 WHERE guid = 343891 AND id = 40639;

-- ============================================================================
-- 8. CREATURE SPAWNS (guid 9001300-9001447)
-- ============================================================================
DELETE FROM creature WHERE guid BETWEEN 9001300 AND 9001449;
DELETE FROM creature_addon WHERE guid BETWEEN 9001300 AND 9001449;
INSERT INTO creature (guid,id,map,zoneId,areaId,spawnMask,phaseUseFlags,phaseMask,PhaseId,PhaseGroup,terrainSwapMap,modelid,equipment_id,position_x,position_y,position_z,orientation,spawntimesecs,wander_distance,currentwaypoint,curhealth,curmana,MovementType,npcflag,unit_flags,dynamicflags,ScriptName,VerifiedBuild) VALUES
-- --- S3 Nespirah interior (phase 169 base) ---
(9001300,41531,0,5144,4962,1,0,1,169,0,-1,0,0,-6445.02,4177.84,-424.93,0.85,300,0,0,1,0,0,0,0,0,'',0),
(9001301,41540,0,5144,4962,1,0,1,169,0,-1,0,0,-6369.10,3641.40,-406.30,2.30,300,0,0,1,0,0,0,0,0,'',0),
(9001302,41541,0,5144,4962,1,0,1,169,0,-1,0,0,-6371.10,3639.80,-406.00,1.90,300,0,0,1,0,0,0,0,0,'',0),
(9001303,41542,0,5144,4962,1,0,1,169,0,-1,0,0,-6366.60,3628.80,-404.10,2.60,300,0,0,1,0,0,0,0,0,'',0),
(9001304,41802,0,5144,4962,1,0,1,169,0,-1,0,0,-6374.00,3644.00,-406.20,1.50,300,5,0,1,0,1,0,0,0,'',0),
(9001305,41810,0,5144,4962,1,0,1,169,0,-1,0,0,-6815.70,3760.40,-403.60,5.50,300,0,0,1,0,0,0,0,0,'',0),
(9001306,41811,0,5144,4962,1,0,1,169,0,-1,0,0,-6816.60,3761.80,-403.80,5.20,300,0,0,1,0,0,0,0,0,'',0),
(9001307,41812,0,5144,4962,1,0,1,169,0,-1,0,0,-6819.70,3759.10,-403.90,5.80,300,0,0,1,0,0,0,0,0,'',0),
(9001308,41813,0,5144,4962,1,0,1,169,0,-1,0,0,-6817.20,3774.30,-405.50,4.90,300,0,0,1,0,0,0,0,0,'',0),
(9001309,41776,0,5144,4962,1,0,1,169,0,-1,0,0,-6548.90,4242.40,-471.70,1.60,120,0,0,1,0,0,0,0,0,'',0),
-- --- S3 ledge camp = S5 cove = S7 ruins camp (phase 169, area 4969) ---
(9001310,40642,0,5144,4969,1,0,1,169,0,-1,0,0,-6598.71,4296.96,-562.70,3.70,300,0,0,1,0,0,0,0,0,'',0),
(9001311,40643,0,5144,4969,1,0,1,169,0,-1,0,0,-6597.53,4296.34,-562.63,3.90,300,0,0,1,0,0,0,0,0,'',0),
(9001312,40644,0,5144,4969,1,0,1,169,0,-1,0,0,-6604.20,4271.20,-562.30,1.40,300,0,0,1,0,0,0,0,0,'',0),
(9001313,40645,0,5144,4969,1,0,1,169,0,-1,0,0,-6620.20,4291.30,-562.70,5.90,300,0,0,1,0,0,0,0,0,'',0),
(9001314,39881,0,5144,4969,1,0,1,169,0,-1,0,0,-6618.37,4282.48,-562.95,0.40,300,0,0,1,0,0,0,0,0,'',0),
(9001315,41869,0,5144,4969,1,0,1,169,0,-1,0,0,-6612.50,4282.30,-563.00,3.30,300,0,0,1,0,0,0,0,0,'',0),
(9001316,40639,0,5144,4969,1,0,1,169,0,-1,0,0,-6595.40,4270.08,-562.20,2.10,300,0,0,1,0,0,0,0,0,'',0),
-- --- S5 vision-1 rendezvous + vision interior (phase 170) ---
(9001317,39881,0,5144,4967,1,0,1,169,0,-1,0,0,-7182.63,4712.50,-595.82,5.50,300,0,0,1,0,0,0,0,0,'',0),
(9001318,40978,0,5144,4967,1,0,1,169,0,-1,0,0,-7189.88,4713.69,-595.82,0.30,300,0,0,1,0,0,0,0,0,'',0),
(9001319,40978,0,5144,4967,1,0,1,170,0,-1,0,0,-7189.88,4713.69,-595.82,0.30,300,0,0,1,0,0,0,0,0,'',0),
(9001320,40640,0,5144,4967,1,0,1,170,0,-1,0,0,-6662.80,4770.30,-606.23,3.30,300,0,0,1,0,0,0,0,0,'',0),
(9001321,41050,0,5144,4967,1,0,1,170,0,-1,0,0,-6650.62,4790.52,-606.23,3.60,300,0,0,1,0,0,0,0,0,'',0),
(9001322,41050,0,5144,4968,1,0,1,170,0,-1,0,0,-7263.14,4876.62,-426.57,5.60,300,0,0,1,0,0,0,0,0,'',0),
(9001323,41049,0,5144,4968,1,0,1,170,0,-1,0,0,-7299.73,4914.34,-426.64,0.40,300,0,0,1,0,0,0,0,0,'',0),
(9001324,41115,0,5144,4968,1,0,1,170,0,-1,0,0,-7297.46,5295.96,-426.64,4.70,90,0,0,1,0,0,0,0,0,'',0),
(9001325,41108,0,5144,4968,1,0,1,170,0,-1,0,0,-7126.43,4909.24,-426.64,0,25,5,0,1,0,1,0,0,0,'',0),
(9001326,41108,0,5144,4968,1,0,1,170,0,-1,0,0,-6550.94,4798.23,-427.07,0,25,5,0,1,0,1,0,0,0,'',0),
(9001327,41108,0,5144,4968,1,0,1,170,0,-1,0,0,-6504.38,4921.29,-426.45,0,25,5,0,1,0,1,0,0,0,'',0),
(9001328,41108,0,5144,4968,1,0,1,170,0,-1,0,0,-6733.52,4893.25,-422.62,0,25,5,0,1,0,1,0,0,0,'',0),
(9001329,41108,0,5144,4968,1,0,1,170,0,-1,0,0,-6811.28,4962.00,-426.57,0,25,5,0,1,0,1,0,0,0,'',0),
(9001330,41108,0,5144,4968,1,0,1,170,0,-1,0,0,-6946.08,4949.85,-425.79,0,25,5,0,1,0,1,0,0,0,'',0),
(9001331,41108,0,5144,4968,1,0,1,170,0,-1,0,0,-6981.91,4996.19,-426.40,0,25,5,0,1,0,1,0,0,0,'',0),
(9001332,41108,0,5144,4968,1,0,1,170,0,-1,0,0,-7101.06,5035.93,-426.57,0,25,5,0,1,0,1,0,0,0,'',0),
(9001333,41108,0,5144,4968,1,0,1,170,0,-1,0,0,-7089.31,5141.51,-426.64,0,25,5,0,1,0,1,0,0,0,'',0),
(9001334,41108,0,5144,4968,1,0,1,170,0,-1,0,0,-7367.76,4919.51,-426.52,0,25,5,0,1,0,1,0,0,0,'',0),
(9001335,39602,0,5144,4967,1,0,1,170,0,-1,0,0,-7119.95,4672.89,-606.23,0,20,5,0,1,0,1,0,0,0,'',0),
(9001336,39602,0,5144,4967,1,0,1,170,0,-1,0,0,-7107.82,4727.75,-608.02,0,20,5,0,1,0,1,0,0,0,'',0),
(9001337,39602,0,5144,4967,1,0,1,170,0,-1,0,0,-7135.28,4771.12,-606.23,0,20,5,0,1,0,1,0,0,0,'',0),
(9001338,39602,0,5144,4967,1,0,1,170,0,-1,0,0,-7150.76,4769.04,-606.23,0,20,5,0,1,0,1,0,0,0,'',0),
(9001339,39602,0,5144,4967,1,0,1,170,0,-1,0,0,-7186.27,4707.50,-595.82,0,20,5,0,1,0,1,0,0,0,'',0),
-- --- S5 reoccupation (phase 169) ---
(9001340,40643,0,5144,4967,1,0,1,169,0,-1,0,0,-6944.86,5080.75,-609.97,0.90,300,0,0,1,0,0,0,0,0,'',0),
(9001341,40642,0,5144,4967,1,0,1,169,0,-1,0,0,-6966.41,5069.72,-609.92,0.40,300,0,0,1,0,0,0,0,0,'',0),
(9001342,41235,0,5144,4967,1,0,1,169,0,-1,0,0,-7008.60,4903.86,-561.97,0,120,0,0,1,0,0,0,0,0,'',0),
(9001343,41235,0,5144,4967,1,0,1,169,0,-1,0,0,-7256.49,4938.36,-557.98,0,120,0,0,1,0,0,0,0,0,'',0),
(9001344,41235,0,5144,4967,1,0,1,169,0,-1,0,0,-7012.12,4933.64,-562.06,0,120,0,0,1,0,0,0,0,0,'',0),
(9001345,41235,0,5144,4967,1,0,1,169,0,-1,0,0,-7174.97,5014.85,-558.38,0,120,0,0,1,0,0,0,0,0,'',0),
(9001346,41235,0,5144,4967,1,0,1,169,0,-1,0,0,-7009.00,5087.94,-516.56,0,120,0,0,1,0,0,0,0,0,'',0),
(9001347,41235,0,5144,4967,1,0,1,169,0,-1,0,0,-7214.59,5033.50,-520.36,0,120,0,0,1,0,0,0,0,0,'',0),
(9001348,41235,0,5144,4967,1,0,1,169,0,-1,0,0,-7003.44,5106.23,-561.97,0,120,0,0,1,0,0,0,0,0,'',0),
(9001349,46470,0,5144,4967,1,0,1,169,0,-1,0,0,-7218.84,4912.48,-557.97,0,120,0,0,1,0,0,0,0,0,'',0),
(9001350,46470,0,5144,4967,1,0,1,169,0,-1,0,0,-7087.79,4930.32,-585.30,0,120,0,0,1,0,0,0,0,0,'',0),
(9001351,46470,0,5144,4967,1,0,1,169,0,-1,0,0,-7224.02,5009.59,-554.16,0,120,0,0,1,0,0,0,0,0,'',0),
-- --- S6 assault (phase 179) / aftermath (phase 180) ---
(9001352,41562,0,5144,4967,1,0,1,180,0,-1,0,0,-6997.35,5070.87,-609.39,2.50,300,0,0,1,0,0,0,0,0,'',0),
(9001353,40643,0,5144,4968,1,0,1,179,0,-1,0,0,-7301.12,4784.32,-426.64,0.70,300,0,0,1,0,0,0,0,0,'',0),
(9001354,40642,0,5144,4968,1,0,1,179,0,-1,0,0,-7295.47,4785.99,-426.64,1.00,300,0,0,1,0,0,0,0,0,'',0),
(9001355,39881,0,5144,4968,1,0,1,179,0,-1,0,0,-7298.49,4779.09,-426.64,1.20,300,0,0,1,0,0,0,0,0,'',0),
(9001356,39881,0,5144,5090,1,0,1,180,0,-1,0,0,-7308.58,5247.91,-426.64,5.80,300,0,0,1,0,0,0,0,0,'',0),
(9001357,48901,0,5144,5090,1,0,1,180,0,-1,0,0,-7310.96,5249.91,-426.64,5.50,120,0,0,1,0,0,0,0,0,'',0),
(9001358,40789,0,5144,4968,1,0,1,179,0,-1,0,0,-7128.87,5183.72,-426.64,0,300,0,0,1,0,0,0,0,0,'',0),
(9001359,40789,0,5144,4967,1,0,1,179,0,-1,0,0,-6942.17,5083.07,-418.39,0,300,0,0,1,0,0,0,0,0,'',0),
(9001360,41259,0,5144,4968,1,0,1,179,0,-1,0,0,-7180.15,4905.10,-426.60,0,300,0,0,1,0,0,0,0,0,'',0),
(9001361,41259,0,5144,5090,1,0,1,179,0,-1,0,0,-7372.77,5246.82,-426.60,0,300,0,0,1,0,0,0,0,0,'',0),
(9001362,41281,0,5144,4968,1,0,1,179,0,-1,0,0,-7189.47,4939.62,-426.64,0,90,0,0,1,0,0,0,0,0,'',0),
(9001363,41281,0,5144,4968,1,0,1,179,0,-1,0,0,-7218.03,4943.19,-426.64,0,90,0,0,1,0,0,0,0,0,'',0),
(9001364,41281,0,5144,4968,1,0,1,179,0,-1,0,0,-7288.74,5015.55,-426.64,0,90,0,0,1,0,0,0,0,0,'',0),
(9001365,41281,0,5144,4968,1,0,1,179,0,-1,0,0,-7256.38,5251.55,-426.64,0,90,0,0,1,0,0,0,0,0,'',0),
(9001366,41281,0,5144,4968,1,0,1,179,0,-1,0,0,-7328.12,5150.49,-426.64,0,90,0,0,1,0,0,0,0,0,'',0),
(9001367,41281,0,5144,4968,1,0,1,179,0,-1,0,0,-7327.02,4983.78,-426.64,0,90,0,0,1,0,0,0,0,0,'',0),
(9001368,41249,0,5144,4967,1,0,1,179,0,-1,0,0,-7098.78,5082.09,-426.64,0,12,3,0,1,0,1,0,0,0,'',0),
(9001369,41249,0,5144,4967,1,0,1,179,0,-1,0,0,-6983.93,5154.03,-426.62,0,12,3,0,1,0,1,0,0,0,'',0),
(9001370,41249,0,5144,4967,1,0,1,179,0,-1,0,0,-6996.44,5154.01,-426.64,0,12,3,0,1,0,1,0,0,0,'',0),
(9001371,41249,0,5144,4967,1,0,1,179,0,-1,0,0,-7098.84,5062.86,-426.64,0,12,3,0,1,0,1,0,0,0,'',0),
(9001372,41249,0,5144,4967,1,0,1,179,0,-1,0,0,-7098.77,5071.20,-426.64,0,12,3,0,1,0,1,0,0,0,'',0),
(9001373,41249,0,5144,4967,1,0,1,179,0,-1,0,0,-6990.40,5154.00,-426.64,0,12,3,0,1,0,1,0,0,0,'',0),
(9001374,41249,0,5144,4967,1,0,1,179,0,-1,0,0,-6957.13,5001.74,-426.41,0,12,3,0,1,0,1,0,0,0,'',0),
(9001375,41249,0,5144,4967,1,0,1,179,0,-1,0,0,-6990.03,5146.24,-426.64,0,12,3,0,1,0,1,0,0,0,'',0),
(9001376,41249,0,5144,4967,1,0,1,179,0,-1,0,0,-6976.69,5146.27,-426.25,0,12,3,0,1,0,1,0,0,0,'',0),
(9001377,41249,0,5144,4967,1,0,1,179,0,-1,0,0,-6948.44,5001.43,-426.50,0,12,3,0,1,0,1,0,0,0,'',0),
(9001378,41249,0,5144,4967,1,0,1,179,0,-1,0,0,-7011.18,4979.34,-426.70,0,12,3,0,1,0,1,0,0,0,'',0),
(9001379,41250,0,5144,4967,1,0,1,179,0,-1,0,0,-7023.03,5149.60,-426.64,0,20,0,0,1,0,0,0,0,0,'',0),
(9001380,41250,0,5144,4967,1,0,1,179,0,-1,0,0,-6958.02,5076.16,-414.45,0,20,0,0,1,0,0,0,0,0,'',0),
(9001381,41250,0,5144,4967,1,0,1,179,0,-1,0,0,-7023.01,5006.69,-423.89,0,20,0,0,1,0,0,0,0,0,'',0),
(9001382,41250,0,5144,4967,1,0,1,179,0,-1,0,0,-6953.24,5090.21,-415.17,0,20,0,0,1,0,0,0,0,0,'',0),
(9001383,41250,0,5144,4967,1,0,1,179,0,-1,0,0,-6953.45,5062.62,-415.14,0,20,0,0,1,0,0,0,0,0,'',0),
(9001384,42549,0,5144,4967,1,0,1,179,0,-1,0,0,-6957.85,5114.97,-418.39,0,25,0,0,1,0,0,0,0,0,'',0),
(9001385,42549,0,5144,4967,1,0,1,179,0,-1,0,0,-6957.19,5140.99,-425.40,0,25,0,0,1,0,0,0,0,0,'',0),
(9001386,42549,0,5144,4967,1,0,1,179,0,-1,0,0,-6957.33,5013.52,-424.77,0,25,0,0,1,0,0,0,0,0,'',0),
(9001387,42549,0,5144,4967,1,0,1,179,0,-1,0,0,-7065.23,5146.13,-426.64,0,25,0,0,1,0,0,0,0,0,'',0),
-- --- S6 vision 2 (phase 171) ---
(9001388,42076,0,5144,5090,1,0,1,171,0,-1,0,0,-7292.67,5274.76,-426.64,4.60,300,0,0,1,0,0,0,0,0,'',0),
(9001389,42072,0,5144,4968,1,0,1,171,0,-1,0,0,-7299.61,5011.92,-426.64,1.50,300,0,0,1,0,0,0,0,0,'',0),
(9001390,41455,0,5144,4968,1,0,1,171,0,-1,0,0,-7334.91,4894.31,-426.64,1.00,300,0,0,1,0,0,0,0,0,'',0),
(9001391,42071,0,5144,4968,1,0,1,171,0,-1,0,0,-7278.28,4779.72,-426.64,1.40,300,0,0,1,0,0,0,0,0,'',0),
(9001392,42074,0,5144,4966,1,0,1,171,0,-1,0,0,-7328.35,4440.54,-276.56,0.30,300,0,0,1,0,0,0,0,0,'',0),
(9001393,42076,0,5144,4966,1,0,1,171,0,-1,0,0,-7311.66,4431.67,-276.56,0.10,300,0,0,1,0,0,0,0,0,'',0),
(9001394,41476,0,5144,4966,1,0,1,171,0,-1,0,0,-7296.67,4434.12,-276.56,3.20,300,0,0,1,0,0,0,0,0,'',0),
(9001395,41457,0,5144,4966,1,0,1,171,0,-1,0,0,-7510.17,3908.86,-218.06,2.40,300,0,0,1,0,0,0,0,0,'',0),
(9001396,41537,0,5144,4966,1,0,1,171,0,-1,0,0,-7549.03,3833.99,-211.48,0.90,90,0,0,1,0,0,0,0,0,'',0),
(9001397,41105,0,5144,4966,1,0,1,171,0,-1,0,0,-7308.44,4333.54,-272.65,0,20,5,0,1,0,1,0,0,0,'',0),
(9001398,41105,0,5144,4966,1,0,1,171,0,-1,0,0,-7339.02,4332.69,-269.71,0,20,5,0,1,0,1,0,0,0,'',0),
(9001399,41105,0,5144,4966,1,0,1,171,0,-1,0,0,-7351.19,4286.52,-267.58,0,20,5,0,1,0,1,0,0,0,'',0),
(9001400,41105,0,5144,4966,1,0,1,171,0,-1,0,0,-7328.59,4253.02,-269.38,0,20,5,0,1,0,1,0,0,0,'',0),
(9001401,41105,0,5144,4966,1,0,1,171,0,-1,0,0,-7282.18,4248.15,-269.45,0,20,5,0,1,0,1,0,0,0,'',0),
(9001402,41105,0,5144,4966,1,0,1,171,0,-1,0,0,-7400.40,4207.14,-253.98,0,20,5,0,1,0,1,0,0,0,'',0),
(9001403,41105,0,5144,4966,1,0,1,171,0,-1,0,0,-7424.64,4177.10,-247.97,0,20,5,0,1,0,1,0,0,0,'',0),
(9001404,41105,0,5144,4966,1,0,1,171,0,-1,0,0,-7502.14,4165.80,-238.05,0,20,5,0,1,0,1,0,0,0,'',0),
(9001405,41451,0,5144,4966,1,0,1,171,0,-1,0,0,-7229.18,4330.20,-272.06,0,45,5,0,1,0,1,0,0,0,'',0),
(9001406,41451,0,5144,4966,1,0,1,171,0,-1,0,0,-7407.76,4285.66,-262.76,0,45,5,0,1,0,1,0,0,0,'',0),
(9001407,41221,0,5144,4966,1,0,1,171,0,-1,0,0,-7280.70,4420.50,-276.60,0,300,0,0,1,0,0,0,0,0,'',0),
-- --- S7 vision 3 temple stage (phase 172) ---
(9001408,41456,0,5144,5124,1,0,1,172,0,-1,0,0,-7274.84,5075.10,-242.35,3.50,300,0,0,1,0,0,0,0,0,'',0),
(9001409,42077,0,5144,5124,1,0,1,172,0,-1,0,0,-7209.65,5074.42,-267.67,3.10,300,0,0,1,0,0,0,0,0,'',0),
(9001410,41980,0,5144,5124,1,0,1,172,0,-1,0,0,-6734.10,5069.38,-345.09,3.10,60,0,0,1,0,0,0,0,0,'',0),
(9001411,41985,0,5144,5124,1,0,1,172,0,-1,0,0,-7299.74,5005.63,-266.06,0,60,0,0,1,0,0,0,0,0,'',0),
(9001412,41985,0,5144,5124,1,0,1,172,0,-1,0,0,-7299.53,5142.87,-266.06,0,60,0,0,1,0,0,0,0,0,'',0),
(9001413,41985,0,5144,5124,1,0,1,172,0,-1,0,0,-7199.11,4993.10,-267.18,0,60,0,0,1,0,0,0,0,0,'',0),
(9001414,41985,0,5144,5124,1,0,1,172,0,-1,0,0,-7368.52,5075.79,-267.68,0,60,0,0,1,0,0,0,0,0,'',0),
(9001415,41985,0,5144,5124,1,0,1,172,0,-1,0,0,-7253.10,4975.85,-284.78,0,60,0,0,1,0,0,0,0,0,'',0),
(9001416,41985,0,5144,5124,1,0,1,172,0,-1,0,0,-7199.12,5158.89,-267.32,0,60,0,0,1,0,0,0,0,0,'',0),
(9001417,41999,0,5144,5124,1,0,1,172,0,-1,0,0,-7237.77,5126.02,-267.68,0,60,0,0,1,0,0,0,0,0,'',0),
(9001418,41999,0,5144,5124,1,0,1,172,0,-1,0,0,-7252.09,5019.20,-267.68,0,60,0,0,1,0,0,0,0,0,'',0),
(9001419,41999,0,5144,5124,1,0,1,172,0,-1,0,0,-7199.74,5124.17,-284.76,0,60,0,0,1,0,0,0,0,0,'',0),
(9001420,41999,0,5144,5124,1,0,1,172,0,-1,0,0,-7200.24,5023.46,-284.74,0,60,0,0,1,0,0,0,0,0,'',0),
(9001421,41999,0,5144,5124,1,0,1,172,0,-1,0,0,-7341.56,5145.22,-267.68,0,60,0,0,1,0,0,0,0,0,'',0),
(9001422,41999,0,5144,5124,1,0,1,172,0,-1,0,0,-7273.46,5178.93,-267.68,0,60,0,0,1,0,0,0,0,0,'',0),
(9001423,41999,0,5144,5124,1,0,1,172,0,-1,0,0,-7341.50,5005.00,-267.68,0,60,0,0,1,0,0,0,0,0,'',0),
(9001424,41999,0,5144,5124,1,0,1,172,0,-1,0,0,-7273.50,4971.00,-267.68,0,60,0,0,1,0,0,0,0,0,'',0),
(9001425,42057,0,5144,5124,1,0,1,172,0,-1,0,0,-7187.45,5073.74,-261.96,0,20,8,0,1,0,1,0,0,0,'',0),
(9001426,42057,0,5144,5124,1,0,1,172,0,-1,0,0,-7216.44,4997.39,-265.81,0,20,8,0,1,0,1,0,0,0,'',0),
(9001427,42057,0,5144,5124,1,0,1,172,0,-1,0,0,-7295.63,4989.40,-266.06,0,20,8,0,1,0,1,0,0,0,'',0),
(9001428,42057,0,5144,5124,1,0,1,172,0,-1,0,0,-7209.94,5139.61,-265.95,0,20,8,0,1,0,1,0,0,0,'',0),
(9001429,42057,0,5144,5124,1,0,1,172,0,-1,0,0,-7306.71,4990.13,-266.06,0,20,8,0,1,0,1,0,0,0,'',0),
(9001430,42057,0,5144,5124,1,0,1,172,0,-1,0,0,-7209.81,5005.82,-266.22,0,20,8,0,1,0,1,0,0,0,'',0),
(9001431,42058,0,5144,5124,1,0,1,172,0,-1,0,0,-7213.67,5001.21,-266.22,0,30,8,0,1,0,1,0,0,0,'',0),
(9001432,42058,0,5144,5124,1,0,1,172,0,-1,0,0,-7216.81,5146.24,-266.38,0,30,8,0,1,0,1,0,0,0,'',0),
(9001433,42066,0,5144,5124,1,0,1,172,0,-1,0,0,-7294.89,5098.37,-247.54,0,60,0,0,1,0,0,0,0,0,'',0),
(9001434,42066,0,5144,5124,1,0,1,172,0,-1,0,0,-7323.85,5064.58,-247.52,0,60,0,0,1,0,0,0,0,0,'',0),
(9001435,44413,0,5144,5124,1,0,1,172,0,-1,0,0,-7243.91,5075.46,-265.14,0,300,0,0,1,0,0,0,0,0,'',0),
(9001436,40789,0,5144,5124,1,0,1,172,0,-1,0,0,-7297.13,5075.54,-270.13,0,300,0,0,1,0,0,0,0,0,'',0),
(9001437,40789,0,5144,4968,1,0,1,172,0,-1,0,0,-7300.68,4823.61,-284.88,0,300,0,0,1,0,0,0,0,0,'',0),
-- --- S7 bridge counter-push set (phase 184, granted by 78329 -> 78323) ---
(9001438,42077,0,5144,4968,1,0,1,184,0,-1,0,0,-7298.30,4619.70,-284.80,1.60,300,0,0,1,0,0,0,0,0,'',0),
(9001439,42073,0,5144,4968,1,0,1,184,0,-1,0,0,-7271.78,4859.83,-284.88,4.70,300,0,0,1,0,0,0,0,0,'',0),
(9001440,42075,0,5144,4968,1,0,1,184,0,-1,0,0,-7313.31,4874.46,-284.88,4.70,300,0,0,1,0,0,0,0,0,'',0),
(9001441,42063,0,5144,4968,1,0,1,184,0,-1,0,0,-7290.37,4513.70,-261.04,1.60,120,0,0,1,0,0,0,0,0,'',0),
-- --- S7 Bielaran/Tranquil camp (180) + surface rescue (224) + sub staging (228) ---
(9001442,41535,0,5144,4966,1,0,1,180,0,-1,0,0,-7358.99,3919.82,-235.55,0.60,300,0,0,1,0,0,0,0,0,'',0),
(9001443,40645,0,5144,4966,1,0,1,180,0,-1,0,0,-7360.67,3917.43,-235.22,0.80,300,0,0,1,0,0,0,0,0,'',0),
(9001444,40645,0,5144,4966,1,0,1,224,0,-1,0,0,-7373.00,3855.00,0.50,1.20,300,0,0,1,0,0,0,0,0,'',0),
(9001445,41572,0,5144,4966,1,0,1,224,0,-1,0,0,-7378.00,3862.00,6.00,4.20,300,0,0,1,0,0,0,0,0,'',0),
(9001446,42411,0,5144,4955,1,0,1,228,0,-1,0,0,-7210.13,3919.68,3.93,3.10,300,0,0,1,0,0,0,0,0,'',0),
(9001447,42486,0,5144,4955,1,0,1,228,0,-1,0,0,-7218.50,3907.50,4.00,0,300,0,0,1,0,0,0,0,0,'',0);

-- Wounded posture for injured actors
INSERT INTO creature_addon (guid, waypointPathId, cyclicSplinePathId, mount, StandState, AnimTier, VisFlags, SheathState, PvPFlags, emote, aiAnimKit, movementAnimKit, meleeAnimKit, visibilityDistanceType, auras) VALUES
(9001352,0,0,0,8,0,0,0,0,0,0,0,0,0,''),
(9001362,0,0,0,8,0,0,0,0,0,0,0,0,0,''),
(9001363,0,0,0,8,0,0,0,0,0,0,0,0,0,''),
(9001364,0,0,0,8,0,0,0,0,0,0,0,0,0,''),
(9001365,0,0,0,8,0,0,0,0,0,0,0,0,0,''),
(9001366,0,0,0,8,0,0,0,0,0,0,0,0,0,''),
(9001367,0,0,0,8,0,0,0,0,0,0,0,0,0,'');

-- ============================================================================
-- 9. GAMEOBJECT SPAWNS (guid 9001060-9001081)
-- ============================================================================
DELETE FROM gameobject WHERE guid BETWEEN 9001060 AND 9001099;
INSERT INTO gameobject (guid,id,map,zoneId,areaId,spawnMask,phaseUseFlags,phaseMask,PhaseId,PhaseGroup,terrainSwapMap,position_x,position_y,position_z,orientation,rotation0,rotation1,rotation2,rotation3,spawntimesecs,animprogress,state,ScriptName,VerifiedBuild) VALUES
-- 203185 Narshola Ward goobers (25658, vision phase 170; native Data1/Data10)
(9001060,203185,0,5144,4968,1,0,1,170,0,-1,-7272.18,4720.05,-426.72,0,0,0,0,1,300,255,1,'',0),
(9001061,203185,0,5144,4968,1,0,1,170,0,-1,-6581.89,4826.87,-427.15,0,0,0,0,1,300,255,1,'',0),
(9001062,203185,0,5144,4968,1,0,1,170,0,-1,-6513.81,4876.60,-427.15,0,0,0,0,1,300,255,1,'',0),
(9001063,203185,0,5144,4968,1,0,1,170,0,-1,-7185.80,4871.79,-426.72,0,0,0,0,1,300,255,1,'',0),
(9001064,203185,0,5144,4968,1,0,1,170,0,-1,-6710.57,4933.28,-426.72,0,0,0,0,1,300,255,1,'',0),
(9001065,203185,0,5144,4968,1,0,1,170,0,-1,-6837.70,4940.32,-426.67,0,0,0,0,1,300,255,1,'',0),
(9001066,203185,0,5144,4968,1,0,1,170,0,-1,-6772.30,4988.72,-425.16,0,0,0,0,1,300,255,1,'',0),
(9001067,203185,0,5144,4968,1,0,1,170,0,-1,-6845.04,5002.97,-405.59,0,0,0,0,1,300,255,1,'',0),
(9001068,203185,0,5144,4968,1,0,1,170,0,-1,-6867.88,5005.59,-423.98,0,0,0,0,1,300,255,1,'',0),
(9001069,203185,0,5144,4968,1,0,1,170,0,-1,-6957.97,5034.99,-418.47,0,0,0,0,1,300,255,1,'',0),
(9001070,203185,0,5144,4968,1,0,1,170,0,-1,-7007.72,5001.50,-405.59,0,0,0,0,1,300,255,1,'',0),
(9001071,203185,0,5144,4968,1,0,1,170,0,-1,-6926.18,5117.07,-418.47,0,0,0,0,1,300,255,1,'',0),
(9001072,203185,0,5144,4968,1,0,1,170,0,-1,-7031.09,5006.65,-423.98,0,0,0,0,1,300,255,1,'',0),
(9001073,203185,0,5144,4968,1,0,1,170,0,-1,-7096.21,5032.67,-426.72,0,0,0,0,1,300,255,1,'',0),
(9001074,203185,0,5144,4968,1,0,1,170,0,-1,-7191.31,4940.64,-426.72,0,0,0,0,1,300,255,1,'',0),
(9001075,203185,0,5144,4968,1,0,1,170,0,-1,-7156.80,4971.87,-426.72,0,0,0,0,1,300,255,1,'',0),
(9001076,203185,0,5144,4968,1,0,1,170,0,-1,-7258.34,4847.74,-426.72,0,0,0,0,1,300,255,1,'',0),
(9001077,203185,0,5144,4968,1,0,1,170,0,-1,-7330.03,4815.77,-426.72,0,0,0,0,1,300,255,1,'',0),
(9001078,203185,0,5144,4968,1,0,1,170,0,-1,-7324.08,4931.77,-426.72,0,0,0,0,1,300,255,1,'',0),
-- 203305 Crucible of Nazsharin - vision 2 copy (25862 ender / 25863 starter)
(9001079,203305,0,5144,4966,1,0,1,171,0,-1,-7664.86,3904.78,-196.23,0,0,0,0,1,300,255,1,'',0),
-- 203403 Survival Kit Remnants (25898 surface chest -> item 56188 flare gun)
(9001080,203403,0,5144,4966,1,0,1,224,0,-1,-7370.50,3852.00,0.30,0,0,0,0,1,120,255,1,'',0),
-- 205062 Boarding Plank (Voldrins Hold staging scenery)
(9001081,205062,0,5144,4955,1,0,1,228,0,-1,-7218.00,3905.00,3.90,0,0,0,0,1,300,255,1,'',0);

-- ============================================================================
-- 10. CREATURE_TEXT (imported from WPP world.sql dumps P2/P3/P4/P5;
--     UNKNOWN -> SoundType 0 / BroadcastTextId 0; 40789 renumbered to
--     groups 1-4 because DB already owns (40789,0))
-- ============================================================================
DELETE FROM creature_text WHERE CreatureID IN (41494,41531,41532,41633,41731,41788,41801,41803,39584,39881,40643,40978,41115,41235,46470,40639,41105,41281,41451,41455,41457,41562,42071,42072,42076,40645,41456,41535,41566,41980,41985,41986,41999,42060,42063,42073,42075,42077,42488,44413,48423,48429) AND CreatureID <> 40789;
DELETE FROM creature_text WHERE CreatureID = 41495 AND GroupID IN (0,1,2);
DELETE FROM creature_text WHERE CreatureID = 40789 AND GroupID IN (1,2,3,4);
INSERT INTO creature_text (CreatureID, GroupID, ID, Text, Type, Language, Probability, Emote, Duration, Sound, SoundType, BroadcastTextId, TextRange, comment) VALUES
-- S3 Nespirah
(41494,0,0,'You\'ve saved my life!',12,7,100,0,0,0,0,0,0,'Enslaved Alliance Pearl Miner - rope rescue'),
(41495,0,0,'Kek ogar lok moth\'aga!',12,1,100,0,0,0,0,0,0,'Enslaved Horde Pearl Miner - rope rescue'),
(41495,1,0,'Ogg maka ha thok zaga kagg aaz mu kazreth...',12,1,100,0,0,0,0,0,0,'Enslaved Horde Pearl Miner - rope rescue'),
(41495,2,0,'Throm ogg!',12,1,100,0,0,0,0,0,0,'Enslaved Horde Pearl Miner - rope rescue'),
(41531,0,0,'Spirit of Life, lend me your aid!',14,0,100,15,0,0,0,0,0,'Earthmender Duarn - Making Contact'),
(41531,1,0,'Grant me the clarity to speak with this creature!',14,0,100,0,0,0,0,0,0,'Earthmender Duarn - Making Contact'),
(41531,2,0,'Earthmender Duarn shuffles uncomfortably.',16,0,100,0,0,0,0,0,0,'Earthmender Duarn - Making Contact'),
(41531,3,0,'Nothing is happening... something must be preventing me from speaking with her.',12,0,100,1,0,0,0,0,0,'Earthmender Duarn - Making Contact'),
(41532,0,0,'What\'s this over here?',12,0,100,0,0,0,0,0,0,'Earthmender Duarn - tunnel escort'),
(41633,0,0,'Kkkk... aaahhh...!',14,0,100,0,0,0,0,0,0,'Voice of Nespirah - Breaking Through'),
(41633,1,0,'What\'s happening?  Can you hear me?',12,0,100,0,0,0,0,0,0,'Voice of Nespirah - Breaking Through'),
(41633,2,0,'I can\'t see you, but I can sense your presence.  I\'ll explain what I can.',12,0,100,0,0,0,0,0,0,'Voice of Nespirah - Breaking Through'),
(41633,3,0,'The naga are suppressing my will.  I can feel what they\'re doing, but I am powerless to act.',12,0,100,0,0,0,0,0,0,'Voice of Nespirah - Breaking Through'),
(41633,4,0,'They mine my pearls, they carve my body, and they distort my mind.  I can feel my will breaking, strangers... they mean to use me for war!',12,0,100,0,0,0,0,0,0,'Voice of Nespirah - Breaking Through'),
(41633,5,0,'I will NOT be a tool of the naga.  I will resist as long as I can, but I can\'t last forever.  Please, someone, stop them!',14,0,100,0,0,0,0,0,0,'Voice of Nespirah - Breaking Through'),
(41731,0,0,'You are too late, you foolsss!  The ancient one\'sss will is broken!  The processs is nearly complete!',14,0,100,1,0,0,0,0,0,'Overseer Idrakess - aggro'),
(41731,1,0,'Now, you will watch as I complete the ritual and turn the beast against you!',14,0,100,1,0,0,0,0,0,'Overseer Idrakess - ritual'),
(41731,2,0,'Slay the Idra\'kess Mistresses to destroy the shield!',41,0,100,0,0,0,0,0,0,'Overseer Idrakess - shield emote'),
(41731,3,0,'Aghhh!  My sssphere!',14,0,100,0,0,0,0,0,0,'Overseer Idrakess - sphere broken'),
(41731,4,0,'Nespirah is waking up! Use the Nespirah Fluid to kill Idra\'kess.',41,0,100,0,0,0,0,0,0,'Overseer Idrakess - fluid emote'),
(41788,0,0,'I have secured escape for us, $n.  You are to come with me.',12,0,100,0,0,0,0,0,0,'Erunak Stonespeaker - Waking the Beast'),
(41788,1,0,'Duarn - excellent work.  Do not dally long within Nespirah\'s shell.  You know as well as I do the dangers posed by an angry demigod.',12,0,100,0,0,0,0,0,0,'Erunak Stonespeaker - Waking the Beast'),
(41788,2,0,'Now, follow me!',14,0,100,0,0,0,0,0,0,'Erunak Stonespeaker - Waking the Beast'),
(41788,3,0,'Board the seahorse, friend.  You probably do not want to be near Nespirah when she turns on the naga.',12,0,100,0,0,0,0,0,0,'Erunak Stonespeaker - Waking the Beast'),
(41801,0,0,'K-k-k-k-kkkah!  I am freed from the naga\'s servitude, and my will is once again my own!',14,0,100,0,0,0,0,0,0,'Voice of Nespirah - Waking the Beast'),
(41801,1,0,'My body is already starting to fight the naga infection.  Please, I ask you to get to safety before I unleash my wrath!',14,0,100,0,0,0,0,0,0,'Voice of Nespirah - Waking the Beast'),
(41801,2,0,'You may leave now, friend.  The tauren\'s friend Erunak should be waiting outside with seahorses.  In fact, it looks like he is here now.',12,0,100,0,0,0,0,0,0,'Voice of Nespirah - Waking the Beast'),
(41801,3,0,'Worry not, Erunak.  The tauren will be safe.',12,0,100,0,0,0,0,0,0,'Voice of Nespirah - Waking the Beast'),
(41803,0,0,'This job is well-suited for you, Duarn.  I must attend to matters outside.',12,0,100,0,0,0,0,0,0,'Erunak Stonespeaker - tunnel escort'),
(41803,1,0,'Good luck to you too, $n.',12,0,100,0,0,0,0,0,0,'Erunak Stonespeaker - tunnel escort'),
-- S5 vision 1 / reoccupation
(39584,0,0,'A single prong. Hardly enough to stop me from slaughtering hundreds more.',12,0,100,0,0,21809,0,0,0,'Nazjar Battlemaiden V1 - intro reply'),
(39584,1,0,'Further attuning yourself with the Battlemaiden, you have gained a new ability!',42,0,100,0,0,0,0,0,0,'Nazjar Battlemaiden V1 - new ability whisper'),
(39881,0,0,'Let me have a look at that shard, would you Admiral?  I could feel its power radiating from the other side of the cave.',12,0,100,1,0,0,0,0,0,'Wavespeaker Valoren - shard RP'),
(39881,1,0,'Thank you.',12,0,100,2,0,0,0,0,0,'Wavespeaker Valoren - shard RP'),
(39881,2,0,'Erunak, take a look at this. This looks like a sufficient focus for the vision magics I was speaking to you about.',12,0,100,1,0,0,0,0,0,'Wavespeaker Valoren - shard RP'),
(39881,3,0,'Erunak examines the shard and nods in agreement.',16,0,100,0,0,0,0,0,0,'Wavespeaker Valoren - shard RP'),
(39881,4,0,'I will prepare the enchantment right away then. This may finally shed some light on the means and intentions of the naga.',12,0,100,1,0,0,0,0,0,'Wavespeaker Valoren - shard RP'),
(39881,5,0,'Welcome back, $n. That took longer than expected. Learn anything useful?',12,0,100,1,0,0,0,0,0,'Wavespeaker Valoren - vision 1 exit'),
(40643,0,0,'Of course.  It\'s all yours.',12,0,100,0,0,0,0,0,0,'Admiral Dvorek - shard RP reply'),
(40978,0,0,'I could hear the sound of your trident breaking from the gardens! The skulls of these Kvaldir are as strong as rock.',12,0,100,1,0,21670,0,0,0,'Fathom-Stalker Azjentus - vision intro'),
(40978,1,0,'Indeed. I doubt much would.',12,0,100,1,0,21671,0,0,0,'Fathom-Stalker Azjentus - vision intro'),
(41115,0,0,'Attack the Sea Witches - stop their spells!',14,0,100,0,0,21404,0,0,0,'Varkul the Unrelenting'),
(41115,1,0,'Attack them now, brothers!',14,0,100,0,0,21402,0,0,0,'Varkul the Unrelenting'),
(41115,2,0,'Kvaldir never die... this city is ours...',14,0,100,0,0,21408,0,0,0,'Varkul the Unrelenting - death'),
(41235,0,0,'\'Bout time.',12,0,100,5,0,0,0,0,0,'Alliance Lookout - restock'),
(41235,1,0,'Good to know they\'ve still got our backs. Thanks!',12,0,100,5,0,0,0,0,0,'Alliance Lookout - restock'),
(41235,2,0,'We\'ll keep you covered. Good luck with the assault!',12,0,100,5,0,0,0,0,0,'Alliance Lookout - restock'),
(41235,3,0,'Good timing - I was almost out.',12,0,100,5,0,0,0,0,0,'Alliance Lookout - restock'),
(46470,0,0,'Their attacks are getting more frequent. You better keep moving.',12,0,100,5,0,0,0,0,0,'Alliance Lookout - restock'),
(46470,1,0,'\'Bout time.',12,0,100,5,0,0,0,0,0,'Alliance Lookout - restock'),
-- S6 assault / vision 2  (40789 WPP P4 g0/g1 -> g3/g4, P5 g0/g1 -> g1/g2)
(40639,0,0,'Explode\'m good!',12,0,100,0,0,0,0,0,0,'Engineer Hexascrub - bombing run'),
(40789,1,0,'The crucible looks abandoned and powerless.$BPerhaps there is a place nearby where you can attune yourself to the Battlemaiden...',42,0,100,0,0,0,0,0,0,'CSA bunny - crucible whisper (25626)'),
(40789,2,0,'You\'ve succeeded in defending the bridge. The summon ritual has been completed.',41,0,100,0,0,0,0,0,0,'CSA bunny - bridge complete (25951)'),
(40789,3,0,'The assault looks like it did almost no damage at all. They\'re already back in formation!',42,0,100,0,0,0,0,0,0,'CSA bunny - NW terrace scout (25754)'),
(40789,4,0,'The naga priestesses look to be summoning a near endless stream of reinforcements!',42,0,100,0,0,0,0,0,0,'CSA bunny - tunnel scout (25754)'),
(41105,0,0,'I will crush the breath from your lungs!',12,0,100,0,0,0,0,0,0,'Kvaldir Limbripper'),
(41281,0,0,'You don\'t have to tell me twice.',12,0,100,0,0,0,0,0,0,'Injured Assault Volunteer'),
(41281,1,0,'On my way. Thank you.',12,0,100,0,0,0,0,0,0,'Injured Assault Volunteer'),
(41281,2,0,'That is *exactly* what I wanted to hear. Thanks.',12,0,100,0,0,0,0,0,0,'Injured Assault Volunteer'),
(41281,3,0,'It was a glorious assault. I\'ll give it that.',12,0,100,0,0,0,0,0,0,'Injured Assault Volunteer'),
(41281,4,0,'Looks like I owe you another one.',12,0,100,0,0,0,0,0,0,'Injured Assault Volunteer'),
(41451,0,0,'These waters will be your grave. ',12,0,100,0,0,0,0,0,0,'Kvaldir Sandterror'),
(41455,0,0,'You shall have your reinforcements, Battlemaiden. I had already intended to collect Kvaldir to labor in Nespirah, so your timing is opportune.',12,0,100,0,0,0,0,0,0,'Overseer Idrakess - 25858 reply'),
(41457,0,0,'On your knees!',14,0,100,0,0,0,0,0,0,'Executioner Verathress'),
(41457,1,0,'Let this be a lesson to all Kvaldir. You are weak! Facing Azshara brings only death.',14,0,100,0,0,0,0,0,0,'Executioner Verathress'),
(41457,2,0,'Slink back to your pathetic mist. Return and we will slaughter every last one of you.',14,0,100,0,0,0,0,0,0,'Executioner Verathress'),
(41562,0,0,'They attacked while you were gone... the Admiral didn\'t make it. The rest retreated to the cave, you should do the same.',12,0,100,0,0,0,0,0,0,'Injured Lookout - 25892'),
(42071,0,0,'I will send some of my priestesses to aid you. Find the artifact quickly so that I can begin summoning our allies for the battles to come.',12,0,100,0,0,0,0,0,0,'Lady Sirakess - 25858 reply'),
(42072,0,0,'You will have some of my best men, do not worry Battlemaiden. Return to us quickly - I am eager to begin hunting the Tidehunter.',12,0,100,0,0,0,0,0,0,'Fathom-Lord Zinjatar - 25858 reply'),
(42076,0,0,'The Kvaldir have been purged from the city. Now we can begin our preparations.',12,0,100,0,0,21806,0,0,0,'Lady Nazjar - vision 2 intro'),
(42076,1,0,'Battlemaiden, I have a task that requires your attention.',12,0,100,0,0,21807,0,0,0,'Lady Nazjar - vision 2 intro'),
-- S7 finale
(40645,0,0,'Fresh air! By the light, I missed it. We have some ships just in sight to the north, $n - fire the flare in their direction.',12,0,100,4,0,0,0,0,0,'Jorlan Trueblade - surface'),
(40645,1,0,'Ha ha! There\'s no way they\'ll miss that, cannon fire or not.',12,0,100,4,0,0,0,0,0,'Jorlan Trueblade - flare fired'),
(41456,0,0,'The Kvaldir have returned to take the city back from us, Battlemaiden. They will need you outside.',12,0,100,0,0,0,0,0,0,'Lady Sirakess - 25860'),
(41535,0,0,'Up you go!',12,0,100,0,0,0,0,0,0,'Engineer Hexascrub - 25898 accept'),
(41566,0,0,'You got shiny?',12,0,100,0,0,0,0,0,0,'Muckskin Scrounger'),
(41566,1,0,'What\'s it got in its pockets? Hmm?',12,0,100,0,0,0,0,0,0,'Muckskin Scrounger'),
(41980,0,0,'Of course. I will finish shortly and meet the Mistress at the temple. Thank you, Battlemaiden.',12,0,100,0,0,0,0,0,0,'Fathom-Caller Azrajar - 25896'),
(41985,0,0,'I see. Thank you for informing me personally, Battlemaiden.',12,0,100,0,0,0,0,0,0,'Sirakess Tide Priestess - 25896'),
(41985,1,0,'I will head there immediately Battlemaiden. Thank you.',12,0,100,0,0,0,0,0,0,'Sirakess Tide Priestess - 25896'),
(41985,2,0,'The Tidehunter will fall.',12,0,100,0,0,0,0,0,0,'Sirakess Tide Priestess - 25896'),
(41985,3,0,'I will find the Mistress of the Tides immediately. Thank you for delivering this message, Battlemaiden.',12,0,100,0,0,0,0,0,0,'Sirakess Tide Priestess - 25896'),
(41986,0,0,'Fall back for the ritual. Our Lady awaits.',12,0,100,0,0,0,0,0,0,'Nazjar Battlemaiden V3 - 25629'),
(41986,1,0,'The Mistress of the Tides will need your defense back at the temple. Make your way there immediately.',12,0,100,0,0,0,0,0,0,'Nazjar Battlemaiden V3 - 25629'),
(41986,2,0,'The Lady has requested you back at the temple, Honor Guard.',12,0,100,0,0,0,0,0,0,'Nazjar Battlemaiden V3 - 25629'),
(41986,3,0,'Back to the temple. I will pick up the slack here.',12,0,100,0,0,0,0,0,0,'Nazjar Battlemaiden V3 - 25629'),
(41986,4,0,'Return to the temple immediately. The ritual will begin soon.',12,0,100,0,0,0,0,0,0,'Nazjar Battlemaiden V3 - 25629'),
(41999,0,0,'On my way.',12,0,100,0,0,0,0,0,0,'Nazjar Honor Guard - 25629'),
(41999,1,0,'Of course, my lady.',12,0,100,0,0,0,0,0,0,'Nazjar Honor Guard - 25629'),
(41999,2,0,'As you say, my lady.',12,0,100,0,0,0,0,0,0,'Nazjar Honor Guard - 25629'),
(41999,3,0,'I will not delay.',12,0,100,0,0,0,0,0,0,'Nazjar Honor Guard - 25629'),
(41999,4,0,'By your order.',12,0,100,0,0,0,0,0,0,'Nazjar Honor Guard - 25629'),
(41999,5,0,'At once, my lady.',12,0,100,0,0,0,0,0,0,'Nazjar Honor Guard - 25629'),
(42060,0,0,'A curse upon your kind, trespasser! ',12,0,100,0,0,0,0,0,0,'Kvaldir Skinflayer'),
(42063,0,0,'You should have left this city to the waves.',14,0,100,0,0,21525,0,0,0,'Hagrim Hopebreaker - aggro'),
(42063,1,0,'Your race is a disease upon the sea.',14,0,100,0,0,21526,0,0,0,'Hagrim Hopebreaker'),
(42073,0,0,'Your march to your deaths! Azshara shall see you wiped from the seas!',14,0,100,0,0,21852,0,0,0,'Fathom-Lord Zinjatar - bridge'),
(42073,1,0,'Slaughter them all! Show no mercy, brothers!',14,0,100,0,0,21854,0,0,0,'Fathom-Lord Zinjatar - bridge'),
(42075,0,0,'Back to the sands with you, wretch!',12,0,100,0,0,21666,0,0,0,'Fathom-Stalker Azjentus - bridge'),
(42077,0,0,'Move forward! Hold the bridge!',12,0,100,0,0,21796,0,0,0,'Lady Nazjar - bridge push'),
(42077,1,0,'Cut them down, Battlemaiden!',14,0,100,0,0,21798,0,0,0,'Lady Nazjar - bridge push'),
(42077,2,0,'Our allies have arrived! Push them back over the edge, my brethren.',12,0,100,0,0,21797,0,0,0,'Lady Nazjar - bridge push'),
(42488,0,0,'The Pincer X2 will be arriving shortly! Get your gear and form up, boys!',14,0,100,0,0,0,0,0,0,'Chief Engineer Yoon - 26219'),
(42488,1,0,'The Pincer X2 has docked. All aboard!',14,0,100,0,0,0,0,0,0,'Chief Engineer Yoon - 26219'),
(44413,0,0,'Kvaldir are pouring across the bridge! Battlemaiden, Lady Naz\'jar calls for you!',12,0,100,0,0,21665,0,0,0,'Fathom-Stalker Azjentus - 25951 hook'),
(48423,0,0,'Take her in slowly, number two.',12,0,100,1,0,21179,0,0,0,'Captain Glovaal - voyage'),
(48423,1,0,'Steady, number two...',12,0,100,0,0,21180,0,0,0,'Captain Glovaal - voyage'),
(48423,2,0,'Fire! Fire! Blow it out of the water!',12,0,100,5,0,21181,0,0,0,'Captain Glovaal - voyage'),
(48423,3,0,'Don\'t worry, number two. We\'ll get another chance. And we\'ll be ready.',12,0,100,1,0,21182,0,0,0,'Captain Glovaal - voyage'),
(48429,0,0,'We are approaching the cavern, Captain.',12,0,100,1,0,21532,0,0,0,'First Lieutenant Wiley - voyage'),
(48429,1,0,'The reports were accurate! The beast is here...',12,0,100,5,0,21533,0,0,0,'First Lieutenant Wiley - voyage'),
(48429,2,0,'The beast escaped! By Mekkatorque\'s moustache, it was fast!',12,0,100,5,0,21534,0,0,0,'First Lieutenant Wiley - voyage');

-- ============================================================================
-- 11. SMART SCRIPTS
--     Exclusive entries use ids 0-19 (existing upstream combat rows kept where
--     sane); shared hub entries (40639) use ids 20-39; per-guid scripts run on
--     batch-B-owned guids only. Timed actionlists: entry*100+n.
-- ============================================================================
DELETE FROM smart_scripts WHERE source_type = 0 AND entryorguid IN (41633,41801,41788,41803,41532,41494,41495,41115,41049,41160,41436,41484,41235,46470,41247,42072,41455,42071,41457,41985,41980,41999,42057,42066,42077,41562,41535,42411,41572,41531,41785);
DELETE FROM smart_scripts WHERE source_type = 0 AND entryorguid = 41520 AND id BETWEEN 1 AND 19;
DELETE FROM smart_scripts WHERE source_type = 0 AND entryorguid = 41795 AND id BETWEEN 4 AND 19;
DELETE FROM smart_scripts WHERE source_type = 0 AND entryorguid = 41731 AND id BETWEEN 3 AND 19;
DELETE FROM smart_scripts WHERE source_type = 0 AND entryorguid = 41537 AND id BETWEEN 4 AND 19;
DELETE FROM smart_scripts WHERE source_type = 0 AND entryorguid = 42063 AND id BETWEEN 2 AND 19;
DELETE FROM smart_scripts WHERE source_type = 0 AND entryorguid = 41281 AND id BETWEEN 1 AND 19;
DELETE FROM smart_scripts WHERE source_type = 0 AND entryorguid = 40639 AND id BETWEEN 20 AND 39;
DELETE FROM smart_scripts WHERE source_type = 0 AND entryorguid IN (-9001317,-9001358,-9001359,-9001360,-9001361,-9001407,-9001435,-9001436,-9001437,-9001439,-9001440,-9001444,-9001447);
DELETE FROM smart_scripts WHERE source_type = 1 AND entryorguid = 203309;
DELETE FROM smart_scripts WHERE source_type = 9 AND entryorguid IN (4153100,4153101,4153102,4153200,4163300,4180100,4178800,4180300,4173103,4173104,4111500,4128101,4153701,4145700,4198500,4198000,4199900,4157200,4248600,4078920);

INSERT INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, event_param5, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES
-- ---- 41531 Earthmender Duarn: 25900 RP / 25916 Voice summon / 25922 finale ----
(41531,0,0,0,62,0,100,0,11525,0,0,0,0,80,4153100,0,0,0,0,0,1,0,0,0,0,0,0,0,'Duarn - gossip opt0 - Making Contact RP'),
(41531,0,1,0,62,0,100,0,11525,1,0,0,0,80,4153101,0,0,0,0,0,1,0,0,0,0,0,0,0,'Duarn - gossip opt1 - Breaking Through'),
(41531,0,2,0,19,0,100,0,25922,0,0,0,0,80,4153102,0,0,0,0,0,1,0,0,0,0,0,0,0,'Duarn - 25922 accepted - Waking the Beast'),
(4153100,9,0,0,0,0,100,0,0,0,0,0,0,72,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Duarn 25900 - close gossip'),
(4153100,9,1,0,0,0,100,0,700,700,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Duarn 25900 - Spirit of Life'),
(4153100,9,2,0,0,0,100,0,4000,4000,0,0,0,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,'Duarn 25900 - Grant me clarity'),
(4153100,9,3,0,0,0,100,0,4800,4800,0,0,0,1,2,0,0,0,0,0,1,0,0,0,0,0,0,0,'Duarn 25900 - shuffles emote'),
(4153100,9,4,0,0,0,100,0,1500,1500,0,0,0,1,3,0,0,0,0,0,1,0,0,0,0,0,0,0,'Duarn 25900 - nothing happening'),
(4153100,9,5,0,0,0,100,0,0,0,0,0,0,33,41531,0,0,0,0,0,7,0,0,0,0,0,0,0,'Duarn 25900 - event credit'),
(4153101,9,0,0,0,0,100,0,0,0,0,0,0,72,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Duarn 25916 - close gossip'),
(4153101,9,1,0,0,0,100,0,500,500,0,0,0,11,77804,0,0,0,0,0,7,0,0,0,0,0,0,0,'Duarn 25916 - forcecast Summon Voice of Nespirah'),
(4153102,9,0,0,0,0,100,0,0,0,0,0,0,41,0,0,0,0,0,0,19,41633,30,0,0,0,0,0,'Duarn 25922 - despawn old Voice'),
(4153102,9,1,0,0,0,100,0,500,500,0,0,0,11,77945,0,0,0,0,0,7,0,0,0,0,0,0,0,'Duarn 25922 - forcecast Summon Erunak + Voice II'),
-- ---- 41633 Voice of Nespirah (25916 RP, summoned by 77621) ----
(41633,0,0,0,54,0,100,0,0,0,0,0,0,80,4163300,0,0,0,0,0,1,0,0,0,0,0,0,0,'Voice of Nespirah - just summoned'),
(4163300,9,0,0,0,0,100,0,1000,1000,0,0,0,11,77554,0,0,0,0,0,1,0,0,0,0,0,0,0,'Voice 25916 - channel beam at PRK bunny'),
(4163300,9,1,0,0,0,100,0,2800,2800,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Voice 25916 - Kkkk aaahhh'),
(4163300,9,2,0,0,0,100,0,3300,3300,0,0,0,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,'Voice 25916 - can you hear me'),
(4163300,9,3,0,0,0,100,0,4000,4000,0,0,0,1,2,0,0,0,0,0,1,0,0,0,0,0,0,0,'Voice 25916 - cannot see you'),
(4163300,9,4,0,0,0,100,0,6400,6400,0,0,0,1,3,0,0,0,0,0,1,0,0,0,0,0,0,0,'Voice 25916 - naga suppressing'),
(4163300,9,5,0,0,0,100,0,5700,5700,0,0,0,1,4,0,0,0,0,0,1,0,0,0,0,0,0,0,'Voice 25916 - they mine my pearls'),
(4163300,9,6,0,0,0,100,0,6400,6400,0,0,0,33,41633,0,0,0,0,0,7,0,0,0,0,0,0,0,'Voice 25916 - event credit'),
(4163300,9,7,0,0,0,100,0,100,100,0,0,0,1,5,0,0,0,0,0,1,0,0,0,0,0,0,0,'Voice 25916 - I will NOT be a tool'),
-- ---- 41801 Voice of Nespirah II (25922, summoned by 77959) ----
(41801,0,0,0,54,0,100,0,0,0,0,0,0,80,4180100,0,0,0,0,0,1,0,0,0,0,0,0,0,'Voice II - just summoned'),
(4180100,9,0,0,0,0,100,0,1000,1000,0,0,0,11,77554,0,0,0,0,0,1,0,0,0,0,0,0,0,'Voice II - channel beam'),
(4180100,9,1,0,0,0,100,0,1500,1500,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Voice II - I am freed'),
(4180100,9,2,0,0,0,100,0,5600,5600,0,0,0,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,'Voice II - fighting the infection'),
(4180100,9,3,0,0,0,100,0,6600,6600,0,0,0,1,2,0,0,0,0,0,1,0,0,0,0,0,0,0,'Voice II - you may leave now'),
(4180100,9,4,0,0,0,100,0,14600,14600,0,0,0,1,3,0,0,0,0,0,1,0,0,0,0,0,0,0,'Voice II - worry not Erunak'),
-- ---- 41788 Erunak (25922 escort to the seahorse, summoned by 77936) ----
(41788,0,0,0,54,0,100,0,0,0,0,0,0,80,4178800,0,0,0,0,0,1,0,0,0,0,0,0,0,'Erunak 25922 - just summoned'),
(4178800,9,0,0,0,0,100,0,21000,21000,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Erunak 25922 - secured escape'),
(4178800,9,1,0,0,0,100,0,4200,4200,0,0,0,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,'Erunak 25922 - Duarn excellent work'),
(4178800,9,2,0,0,0,100,0,1900,1900,0,0,0,33,41531,0,0,0,0,0,7,0,0,0,0,0,0,0,'Erunak 25922 - listen credit'),
(4178800,9,3,0,0,0,100,0,3700,3700,0,0,0,1,2,0,0,0,0,0,1,0,0,0,0,0,0,0,'Erunak 25922 - now follow me'),
(4178800,9,4,0,0,0,100,0,500,500,0,0,0,69,1,0,1,0,0,0,8,0,0,0,-6473.81,4167.97,-425.82,0,'Erunak 25922 - wp1'),
(4178800,9,5,0,0,0,100,0,3500,3500,0,0,0,69,2,0,1,0,0,0,8,0,0,0,-6487.10,4159.76,-422.51,0,'Erunak 25922 - wp2'),
(4178800,9,6,0,0,0,100,0,3000,3000,0,0,0,69,3,0,1,0,0,0,8,0,0,0,-6511.77,4148.33,-430.75,0,'Erunak 25922 - wp3'),
(4178800,9,7,0,0,0,100,0,3500,3500,0,0,0,69,4,0,1,0,0,0,8,0,0,0,-6536.72,4153.68,-435.10,0,'Erunak 25922 - wp4'),
(4178800,9,8,0,0,0,100,0,3200,3200,0,0,0,69,5,0,1,0,0,0,8,0,0,0,-6549.32,4178.83,-436.44,0,'Erunak 25922 - wp5'),
(4178800,9,9,0,0,0,100,0,2000,2000,0,0,0,69,6,0,1,0,0,0,8,0,0,0,-6555.89,4186.90,-437.14,0,'Erunak 25922 - wp6'),
(4178800,9,10,0,0,0,100,0,3200,3200,0,0,0,69,7,0,1,0,0,0,8,0,0,0,-6550.15,4193.50,-455.98,0,'Erunak 25922 - wp7'),
(4178800,9,11,0,0,0,100,0,5000,5000,0,0,0,69,8,0,1,0,0,0,8,0,0,0,-6547.02,4231.60,-470.96,0,'Erunak 25922 - wp8 valve'),
(4178800,9,12,0,0,0,100,0,6000,6000,0,0,0,1,3,0,0,0,0,0,1,0,0,0,0,0,0,0,'Erunak 25922 - board the seahorse'),
(4178800,9,13,0,0,0,100,0,120000,120000,0,0,0,41,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Erunak 25922 - despawn'),
-- ---- 41803/41532 tunnel escort (25890, summoned via spell_area 77963) ----
(41803,0,0,0,54,0,100,0,0,0,0,0,0,80,4180300,0,0,0,0,0,1,0,0,0,0,0,0,0,'Erunak 25890 - just summoned'),
(4180300,9,0,0,0,0,100,0,500,500,0,0,0,12,41532,1,240000,0,0,0,1,0,0,0,0,0,0,0,'Erunak 25890 - summon Duarn escort'),
(4180300,9,1,0,0,0,100,0,1000,1000,0,0,0,69,1,0,1,0,0,0,8,0,0,0,-6280.00,4230.00,-390.00,0,'Erunak 25890 - wp1 tunnel'),
(4180300,9,2,0,0,0,100,0,14000,14000,0,0,0,69,2,0,1,0,0,0,8,0,0,0,-6403.90,4110.00,-433.50,0,'Erunak 25890 - wp2 tunnel'),
(4180300,9,3,0,0,0,100,0,12000,12000,0,0,0,69,3,0,1,0,0,0,8,0,0,0,-6418.71,4114.03,-427.81,0,'Erunak 25890 - wp3 trigger plane'),
(4180300,9,4,0,0,0,100,0,4000,4000,0,0,0,69,4,0,1,0,0,0,8,0,0,0,-6435.37,4162.76,-425.27,0,'Erunak 25890 - wp4'),
(4180300,9,5,0,0,0,100,0,5000,5000,0,0,0,69,5,0,1,0,0,0,8,0,0,0,-6444.00,4176.00,-424.90,0,'Erunak 25890 - wp5 chamber'),
(4180300,9,6,0,0,0,100,0,4000,4000,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Erunak 25890 - well-suited for you'),
(4180300,9,7,0,0,0,100,0,5500,5500,0,0,0,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,'Erunak 25890 - good luck to you'),
(4180300,9,8,0,0,0,100,0,4000,4000,0,0,0,41,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Erunak 25890 - despawn'),
(41532,0,0,0,54,0,100,0,0,0,0,0,0,80,4153200,0,0,0,0,0,1,0,0,0,0,0,0,0,'Duarn escort 25890 - just summoned'),
(4153200,9,0,0,0,0,100,0,1500,1500,0,0,0,69,1,0,1,0,0,0,8,0,0,0,-6282.00,4228.00,-390.00,0,'Duarn escort - wp1'),
(4153200,9,1,0,0,0,100,0,14000,14000,0,0,0,69,2,0,1,0,0,0,8,0,0,0,-6406.00,4108.00,-433.50,0,'Duarn escort - wp2'),
(4153200,9,2,0,0,0,100,0,6000,6000,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Duarn escort - whats this over here'),
(4153200,9,3,0,0,0,100,0,6000,6000,0,0,0,69,3,0,1,0,0,0,8,0,0,0,-6420.00,4116.00,-427.80,0,'Duarn escort - wp3'),
(4153200,9,4,0,0,0,100,0,4000,4000,0,0,0,69,4,0,1,0,0,0,8,0,0,0,-6437.00,4164.00,-425.20,0,'Duarn escort - wp4'),
(4153200,9,5,0,0,0,100,0,5000,5000,0,0,0,69,5,0,1,0,0,0,8,0,0,0,-6447.00,4174.50,-424.90,0,'Duarn escort - wp5'),
(4153200,9,6,0,0,0,100,0,30000,30000,0,0,0,41,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Duarn escort - despawn'),
-- ---- 25907 miners / 25909 crab ----
(41494,0,0,1,8,0,100,0,77664,0,10000,10000,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Alliance Pearl Miner - hit by Throw Rope - talk'),
(41494,0,1,2,61,0,100,0,0,0,0,0,0,33,41555,0,0,0,0,0,7,0,0,0,0,0,0,0,'Alliance Pearl Miner - rescue credit'),
(41494,0,2,0,61,0,100,0,0,0,0,0,0,41,3500,0,0,0,0,0,1,0,0,0,0,0,0,0,'Alliance Pearl Miner - despawn'),
(41495,0,0,1,8,0,100,0,77664,0,10000,10000,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Horde Pearl Miner - hit by Throw Rope - talk'),
(41495,0,1,2,61,0,100,0,0,0,0,0,0,33,41555,0,0,0,0,0,7,0,0,0,0,0,0,0,'Horde Pearl Miner - rescue credit'),
(41495,0,2,0,61,0,100,0,0,0,0,0,0,41,3500,0,0,0,0,0,1,0,0,0,0,0,0,0,'Horde Pearl Miner - despawn'),
(41520,0,1,0,8,0,100,0,77682,0,15000,15000,0,41,15000,0,0,0,0,0,1,0,0,0,0,0,0,0,'Deepseeker Crab - netted - despawn after loot window'),
-- ---- 25921 Overseer Idrakess encounter ----
(41795,0,5,0,0,0,100,0,5000,8000,9000,12000,0,11,77841,0,0,0,0,0,19,41731,40,0,0,0,0,0,'Idrakess Mistress - maintain Impenetrable Sphere (IC)'),
(41795,0,6,0,1,0,100,0,5000,5000,12000,12000,0,11,77841,0,0,0,0,0,19,41731,40,0,0,0,0,0,'Idrakess Mistress - maintain Impenetrable Sphere (OOC)'),
(41795,0,7,8,6,0,100,0,0,0,0,0,0,28,77841,0,0,0,0,0,19,41731,60,0,0,0,0,0,'Idrakess Mistress - death - strip sphere'),
(41795,0,8,0,61,0,100,0,0,0,0,0,0,45,1,1,0,0,0,0,19,41731,60,0,0,0,0,0,'Idrakess Mistress - death - notify Overseer'),
(41795,0,9,0,1,0,100,0,3000,3000,9000,9000,0,11,77626,0,0,0,0,0,19,41200,150,0,0,0,0,0,'Idrakess Mistress - Suppression Bolt at heart bunny'),
(41731,0,3,0,4,0,100,0,0,0,0,0,0,80,4173103,0,0,0,0,0,1,0,0,0,0,0,0,0,'Overseer Idrakess - aggro RP'),
(41731,0,4,0,0,0,100,1,40000,45000,0,0,0,80,4173104,0,0,0,0,0,1,0,0,0,0,0,0,0,'Overseer Idrakess - Nespirah wakes (fluid phase)'),
(41731,0,5,0,38,0,100,0,1,1,5000,5000,0,1,3,0,0,0,0,0,1,0,0,0,0,0,0,0,'Overseer Idrakess - sphere broken yell'),
(4173103,9,0,0,0,0,100,0,2500,2500,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Overseer - too late fools'),
(4173103,9,1,0,0,0,100,0,5000,5000,0,0,0,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,'Overseer - complete the ritual'),
(4173103,9,2,0,0,0,100,0,0,0,0,0,0,11,77843,0,0,0,0,0,1,0,0,0,0,0,0,0,'Overseer - Broken Resolve channel'),
(4173103,9,3,0,0,0,100,0,1500,1500,0,0,0,1,2,0,0,0,0,0,1,0,0,0,0,0,0,0,'Overseer - slay the mistresses emote'),
(4173104,9,0,0,0,0,100,0,0,0,0,0,0,1,4,0,0,0,0,0,1,0,0,0,0,0,0,0,'Overseer - Nespirah waking emote'),
(4173104,9,1,0,0,0,100,0,500,500,0,0,0,11,88429,0,0,0,0,0,1,0,0,0,0,0,0,0,'Overseer - summon Nespirah Fluid'),
-- ---- Vision transform bunnies (blade 77292 -> per-vision forcecast) ----
(41160,0,0,0,8,0,100,0,77292,0,5000,5000,0,11,77293,0,0,0,0,0,7,0,0,0,0,0,0,0,'V1 bunny - blade hit - forcecast 73974'),
(41436,0,0,0,8,0,100,0,77292,0,5000,5000,0,11,77566,0,0,0,0,0,7,0,0,0,0,0,0,0,'V2 bunny - blade hit - forcecast 77565'),
(41484,0,0,1,8,0,100,0,77292,0,5000,5000,0,11,78265,0,0,0,0,0,7,0,0,0,0,0,0,0,'V3 bunny - blade hit - forcecast 78264'),
(41484,0,1,0,61,0,100,0,0,0,0,0,0,85,78263,2,0,0,0,0,7,0,0,0,0,0,0,0,'V3 bunny - grant phase 172 (78332 script substitute)'),
-- ---- 25659 vision-1 exit at Zinjatar turn-in ----
(41049,0,0,1,20,0,100,0,25659,0,0,0,0,28,73974,0,0,0,0,0,7,0,0,0,0,0,0,0,'Zinjatar - 25659 rewarded - remove Battlemaiden aura'),
(41049,0,1,0,61,0,100,0,0,0,0,0,0,41,0,0,0,0,0,0,19,39584,20,0,0,0,0,0,'Zinjatar - 25659 rewarded - despawn V1 vehicle'),
-- ---- Varkul the Unrelenting (25659) ----
(41115,0,0,0,4,0,100,0,0,0,0,0,0,80,4111500,0,0,0,0,0,1,0,0,0,0,0,0,0,'Varkul - aggro yells'),
(41115,0,1,0,0,0,100,0,8000,12000,15000,20000,0,11,76881,0,0,0,0,0,2,0,0,0,0,0,0,0,'Varkul - Throw Harpoon'),
(41115,0,2,0,0,0,100,0,20000,25000,30000,40000,0,11,76895,0,0,0,0,0,1,0,0,0,0,0,0,0,'Varkul - Colossal Crash'),
(41115,0,3,0,0,0,100,1,75000,90000,0,0,0,11,76862,0,0,0,0,0,1,0,0,0,0,0,0,0,'Varkul - Enrage'),
(41115,0,4,0,6,0,100,0,0,0,0,0,0,1,2,0,0,0,0,0,1,0,0,0,0,0,0,0,'Varkul - death yell'),
(4111500,9,0,0,0,0,100,0,2000,2000,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Varkul - attack the Sea Witches'),
(4111500,9,1,0,0,0,100,0,60000,60000,0,0,0,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,'Varkul - attack them now'),
-- ---- 25749 lookouts ----
(41235,0,0,1,8,0,100,0,77313,0,5000,5000,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Alliance Lookout - restocked talk'),
(41235,0,1,0,61,0,100,0,0,0,0,0,0,33,41235,0,0,0,0,0,7,0,0,0,0,0,0,0,'Alliance Lookout - restock credit'),
(46470,0,0,1,8,0,100,0,77313,0,5000,5000,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Alliance Lookout 46470 - restocked talk'),
(46470,0,1,0,61,0,100,0,0,0,0,0,0,33,41235,0,0,0,0,0,7,0,0,0,0,0,0,0,'Alliance Lookout 46470 - restock credit'),
-- ---- 25752 bombing run ----
(40639,0,20,21,62,0,100,0,11477,0,0,0,0,72,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Hexascrub - assault gossip - close'),
(40639,0,21,22,61,0,100,0,0,0,0,0,0,11,77325,0,0,0,0,0,7,0,0,0,0,0,0,0,'Hexascrub - forcecast Ruins Assault (summon+ride ray)'),
(40639,0,22,0,61,0,100,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Hexascrub - Explodem good'),
(41247,0,0,0,8,0,100,0,77342,0,0,0,0,41,1500,0,0,0,0,0,1,0,0,0,0,0,0,0,'Bombing Ray - run finished - despawn/eject'),
-- ---- 25753 volunteers ----
(41281,0,1,0,62,0,100,0,11481,0,0,0,0,80,4128101,0,0,0,0,0,1,0,0,0,0,0,0,0,'Injured Volunteer - rescue gossip'),
(4128101,9,0,0,0,0,100,0,0,0,0,0,0,72,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Volunteer - close gossip'),
(4128101,9,1,0,0,0,100,0,300,300,0,0,0,11,77398,0,0,0,0,0,7,0,0,0,0,0,0,0,'Volunteer - rescue credit spell'),
(4128101,9,2,0,0,0,100,0,500,500,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Volunteer - talk'),
(4128101,9,3,0,0,0,100,0,2500,2500,0,0,0,41,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Volunteer - swim off (despawn)'),
-- ---- 25858 gossip credits (vision 2) ----
(42072,0,0,1,62,0,100,0,11515,0,0,0,0,72,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Zinjatar V2 - close gossip'),
(42072,0,1,2,61,0,100,0,0,0,0,0,0,11,77633,0,0,0,0,0,7,0,0,0,0,0,0,0,'Zinjatar V2 - credit 41049'),
(42072,0,2,0,61,0,100,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Zinjatar V2 - reply'),
(41455,0,0,1,62,0,100,0,11516,0,0,0,0,72,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Overseer V2 - close gossip'),
(41455,0,1,2,61,0,100,0,0,0,0,0,0,11,77634,0,0,0,0,0,7,0,0,0,0,0,0,0,'Overseer V2 - credit 41455'),
(41455,0,2,0,61,0,100,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Overseer V2 - reply'),
(42071,0,0,1,62,0,100,0,11517,0,0,0,0,72,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Lady Sirakess V2 - close gossip'),
(42071,0,1,2,61,0,100,0,0,0,0,0,0,11,77635,0,0,0,0,0,7,0,0,0,0,0,0,0,'Lady Sirakess V2 - credit 41456'),
(42071,0,2,0,61,0,100,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Lady Sirakess V2 - reply'),
-- ---- 25861 execution event ----
(41537,0,4,0,0,0,100,0,2000,3000,4000,4000,0,11,77641,0,0,0,0,0,1,0,0,0,0,0,0,0,'High-Shaman - execution ping (IC)'),
(41537,0,5,0,1,0,100,0,5000,5000,6000,6000,0,11,77641,0,0,0,0,0,1,0,0,0,0,0,0,0,'High-Shaman - execution ping (OOC)'),
(41537,0,6,0,38,0,100,1,2,1,0,0,0,80,4153701,0,0,0,0,0,1,0,0,0,0,0,0,0,'High-Shaman - subjugated by Verathress'),
(4153701,9,0,0,0,0,100,0,0,0,0,0,0,33,41457,0,0,0,0,0,2,0,0,0,0,0,0,0,'High-Shaman - credit dragging player'),
(4153701,9,1,0,0,0,100,0,200,200,0,0,0,45,3,1,0,0,0,0,19,41457,40,0,0,0,0,0,'High-Shaman - trigger Subjugation'),
(4153701,9,2,0,0,0,100,0,13000,13000,0,0,0,45,4,1,0,0,0,0,19,41457,40,0,0,0,0,0,'High-Shaman - trigger Execution'),
(41457,0,0,0,8,0,100,0,77641,0,3000,3000,0,45,2,1,0,0,0,0,7,0,0,0,0,0,0,0,'Verathress - ping received - notify shaman'),
(41457,0,1,2,38,0,100,0,3,1,0,0,0,11,77653,0,0,0,0,0,7,0,0,0,0,0,0,0,'Verathress - Subjugation'),
(41457,0,2,0,61,0,100,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Verathress - on your knees'),
(41457,0,3,0,38,0,100,0,4,1,0,0,0,80,4145700,0,0,0,0,0,1,0,0,0,0,0,0,0,'Verathress - execution sequence'),
(4145700,9,0,0,0,0,100,0,0,0,0,0,0,11,77654,0,0,0,0,0,7,0,0,0,0,0,0,0,'Verathress - Execution instakill'),
(4145700,9,1,0,0,0,100,0,300,300,0,0,0,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,'Verathress - let this be a lesson'),
(4145700,9,2,0,0,0,100,0,3500,3500,0,0,0,1,2,0,0,0,0,0,1,0,0,0,0,0,0,0,'Verathress - slink back to your mist'),
-- ---- 25896 temple gossip credits (vision 3) ----
(41985,0,0,0,62,0,100,0,11571,0,0,0,0,80,4198500,0,0,0,0,0,1,0,0,0,0,0,0,0,'Tide Priestess - orders gossip'),
(4198500,9,0,0,0,0,100,0,0,0,0,0,0,72,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Tide Priestess - close'),
(4198500,9,1,0,0,0,100,0,300,300,0,0,0,11,78266,0,0,0,0,0,7,0,0,0,0,0,0,0,'Tide Priestess - orders credit'),
(4198500,9,2,0,0,0,100,0,500,500,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Tide Priestess - reply'),
(4198500,9,3,0,0,0,100,0,1500,1500,0,0,0,69,1,0,0,0,0,0,8,0,0,0,-7274.00,5070.00,-243.00,0,'Tide Priestess - head to temple'),
(4198500,9,4,0,0,0,100,0,12000,12000,0,0,0,41,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Tide Priestess - despawn'),
(41980,0,0,0,62,0,100,0,11572,0,0,0,0,80,4198000,0,0,0,0,0,1,0,0,0,0,0,0,0,'Azrajar - orders gossip'),
(4198000,9,0,0,0,0,100,0,0,0,0,0,0,72,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Azrajar - close'),
(4198000,9,1,0,0,0,100,0,300,300,0,0,0,11,78267,0,0,0,0,0,7,0,0,0,0,0,0,0,'Azrajar - orders credit'),
(4198000,9,2,0,0,0,100,0,500,500,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Azrajar - reply'),
(4198000,9,3,0,0,0,100,0,10000,10000,0,0,0,41,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Azrajar - despawn'),
-- ---- 25629 honor guards (proximity credit via 78268 AoE) ----
(41999,0,0,0,10,0,100,0,1,15,30000,30000,0,80,4199900,0,0,0,0,0,1,0,0,0,0,0,0,0,'Honor Guard - battlemaiden nearby (friendly)'),
(41999,0,1,0,10,0,100,0,0,15,30000,30000,0,80,4199900,0,0,0,0,0,1,0,0,0,0,0,0,0,'Honor Guard - battlemaiden nearby (hostile-flagged rider)'),
(4199900,9,0,0,0,0,100,0,0,0,0,0,0,11,78268,0,0,0,0,0,1,0,0,0,0,0,0,0,'Honor Guard - orders credit AoE'),
(4199900,9,1,0,0,0,100,0,300,300,0,0,0,1,0,0,0,0,0,0,19,41986,20,0,0,0,0,0,'Honor Guard - battlemaiden line'),
(4199900,9,2,0,0,0,100,0,2500,2500,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Honor Guard - reply'),
(4199900,9,3,0,0,0,100,0,1500,1500,0,0,0,69,1,0,0,0,0,0,8,0,0,0,-7250.00,5075.00,-267.00,0,'Honor Guard - fall back to temple'),
(4199900,9,4,0,0,0,100,0,15000,15000,0,0,0,41,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Honor Guard - despawn'),
-- ---- 25860 waves / defenders ----
(42057,0,0,0,0,0,100,0,5000,8000,9000,14000,0,11,39047,0,0,0,0,0,2,0,0,0,0,0,0,0,'Kvaldir Bonesnapper - Cleave'),
(42066,0,0,0,0,0,100,0,2000,3000,3500,5000,0,11,78587,0,0,0,0,0,2,0,0,0,0,0,0,0,'Nazjar Spitfire - Spitfire Arrow'),
-- ---- 25951 bridge finale ----
(42063,0,2,0,6,0,100,0,0,0,0,0,0,33,42063,0,0,0,0,0,7,0,0,0,0,0,0,0,'Hagrim - death credit (backstop for E90 path)'),
(42063,0,3,0,4,0,100,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Hagrim - aggro yell'),
(42063,0,4,0,0,0,100,1,25000,30000,0,0,0,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,'Hagrim - disease upon the sea'),
(42077,0,0,1,20,0,100,0,25951,0,0,0,0,28,78264,0,0,0,0,0,7,0,0,0,0,0,0,0,'Lady Nazjar - 25951 rewarded - remove summon aura'),
(42077,0,1,2,61,0,100,0,0,0,0,0,0,28,78263,0,0,0,0,0,7,0,0,0,0,0,0,0,'Lady Nazjar - remove phase 172 aura'),
(42077,0,2,3,61,0,100,0,0,0,0,0,0,28,78323,0,0,0,0,0,7,0,0,0,0,0,0,0,'Lady Nazjar - remove phase 184 aura'),
(42077,0,3,0,61,0,100,0,0,0,0,0,0,41,0,0,0,0,0,0,19,41986,30,0,0,0,0,0,'Lady Nazjar - despawn V3 vehicle'),
-- ---- Hexascrub 25898 / surface set ----
(41535,0,0,0,19,0,100,0,25898,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Hexascrub - Up you go'),
(41572,0,0,0,8,0,100,0,77741,0,10000,10000,0,80,4157200,0,0,0,0,0,1,0,0,0,0,0,0,0,'Rescue Balloon - flare hit'),
(4157200,9,0,0,0,0,100,0,5200,5200,0,0,0,33,41572,0,0,0,0,0,7,0,0,0,0,0,0,0,'Rescue Balloon - flare credit'),
(4157200,9,1,0,0,0,100,0,300,300,0,0,0,1,1,0,0,0,0,0,19,40645,60,0,0,0,0,0,'Rescue Balloon - Jorlan cheers'),
-- ---- 26219 voyage-lite ----
(42411,0,0,0,19,0,100,0,26219,0,0,0,0,1,0,0,0,0,0,0,19,42488,50,0,0,0,0,0,'Capt Taylor - Yoon announces Pincer X2');

-- per-guid scripts (batch-B owned guids)
INSERT INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, event_param5, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES
(-9001317,0,0,0,10,0,100,0,1,30,120000,120000,0,1,5,0,1,0,0,0,7,0,0,0,0,0,0,0,'Valoren rendezvous - welcome back'),
(-9001358,0,0,1,10,0,100,0,1,45,60000,60000,0,1,3,0,1,0,0,0,7,0,0,0,0,0,0,0,'CSA NW scout - whisper'),
(-9001358,0,1,0,61,0,100,0,0,0,0,0,0,33,41303,0,0,0,0,0,7,0,0,0,0,0,0,0,'CSA NW scout - credit 41303'),
(-9001359,0,0,1,10,0,100,0,1,45,60000,60000,0,1,4,0,1,0,0,0,7,0,0,0,0,0,0,0,'CSA tunnel scout - whisper'),
(-9001359,0,1,0,61,0,100,0,0,0,0,0,0,33,41304,0,0,0,0,0,7,0,0,0,0,0,0,0,'CSA tunnel scout - credit 41304'),
(-9001360,0,0,0,10,0,100,0,1,45,120000,120000,0,11,77344,0,0,0,0,0,7,0,0,0,0,0,0,0,'Bombing Run Controller S - finish run'),
(-9001361,0,0,0,10,0,100,0,1,45,120000,120000,0,11,77344,0,0,0,0,0,7,0,0,0,0,0,0,0,'Bombing Run Controller N - finish run'),
(-9001407,0,0,1,10,0,100,0,1,25,15000,15000,0,11,77284,0,0,0,0,0,7,0,0,0,0,0,0,0,'V2 exit trigger - kill credit 41221'),
(-9001407,0,1,2,61,0,100,0,0,0,0,0,0,28,77565,0,0,0,0,0,7,0,0,0,0,0,0,0,'V2 exit trigger - remove battlemaiden aura'),
(-9001407,0,2,0,61,0,100,0,0,0,0,0,0,41,0,0,0,0,0,0,19,41225,30,0,0,0,0,0,'V2 exit trigger - despawn V2 vehicle'),
(-9001435,0,0,0,10,0,100,0,1,35,600000,600000,0,1,0,0,1,0,0,0,7,0,0,0,0,0,0,0,'Azjentus 44413 - Kvaldir pouring whisper'),
(-9001436,0,0,1,10,0,100,0,1,40,60000,60000,0,33,41982,0,0,0,0,0,7,0,0,0,0,0,0,0,'CSA crucible - 25626 temple credit'),
(-9001436,0,1,0,61,0,100,0,0,0,0,0,0,1,1,0,1,0,0,0,7,0,0,0,0,0,0,0,'CSA crucible - abandoned whisper'),
(-9001437,0,0,0,10,0,100,0,1,45,90000,90000,0,80,4078920,0,0,0,0,0,1,0,0,0,0,0,0,0,'CSA east bridge - 25951 hold timer'),
(-9001439,0,0,0,10,0,100,0,0,50,300000,300000,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Zinjatar bridge - yell'),
(-9001439,0,1,0,10,0,100,0,1,50,300000,300000,0,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,'Zinjatar bridge - yell 2'),
(-9001440,0,0,0,10,0,100,0,0,50,300000,300000,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Azjentus bridge - yell'),
(-9001440,0,1,0,10,0,100,0,1,50,300000,300000,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Azjentus bridge - yell (nonhostile)'),
(-9001444,0,0,1,10,0,100,0,1,35,60000,60000,0,1,0,0,1,0,0,0,7,0,0,0,0,0,0,0,'Surface Jorlan - fresh air'),
(-9001444,0,1,0,61,0,100,0,0,0,0,0,0,33,40645,0,0,0,0,0,7,0,0,0,0,0,0,0,'Surface Jorlan - swim-up credit'),
(-9001447,0,0,0,10,0,100,0,1,12,180000,180000,0,80,4248600,0,0,0,0,0,1,0,0,0,0,0,0,0,'Boarding trigger - 26219 voyage');
INSERT INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, event_param5, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES
(4078920,9,0,0,0,0,100,0,60000,60000,0,0,0,11,78329,0,0,0,0,0,1,0,0,0,0,0,0,0,'CSA east bridge - phase 184 + credit 42135'),
(4248600,9,0,0,0,0,100,0,0,0,0,0,0,33,42486,0,0,0,0,0,7,0,0,0,0,0,0,0,'Voyage - boarding credit'),
(4248600,9,1,0,0,0,100,0,1500,1500,0,0,0,1,1,0,0,0,0,0,19,42488,60,0,0,0,0,0,'Voyage - Yoon all aboard'),
(4248600,9,2,0,0,0,100,0,12000,12000,0,0,0,33,42487,0,0,0,0,0,7,0,0,0,0,0,0,0,'Voyage - cavern credit'),
(4248600,9,3,0,0,0,100,0,3000,3000,0,0,0,85,79239,2,0,0,0,0,7,0,0,0,0,0,0,0,'Voyage - teleport to Darkbreak Cove');

-- 11b. SAI event conditions (source 22; SourceGroup = id+1)
DELETE FROM conditions WHERE SourceTypeOrReferenceId = 22 AND SourceEntry IN (41160,41436,41484,41562,41999,-9001317,-9001358,-9001359,-9001360,-9001361,-9001407,-9001435,-9001436,-9001437,-9001444,-9001447);
INSERT INTO conditions (SourceTypeOrReferenceId, SourceGroup, SourceEntry, SourceId, ElseGroup, ConditionTypeOrReference, ConditionTarget, ConditionValue1, ConditionValue2, ConditionValue3, NegativeCondition, ErrorType, ErrorTextId, ScriptName, Comment) VALUES
(22,1,41160,0,0,9,0,25760,0,0,0,0,0,'','V1 bunny reacts only on 25760'),
(22,1,41436,0,0,9,0,25755,0,0,0,0,0,'','V2 bunny reacts only on 25755'),
(22,1,41484,0,0,9,0,25626,0,0,0,0,0,'','V3 bunny reacts only on 25626'),
(22,1,41562,0,0,9,0,25892,0,0,0,0,0,'','Injured Lookout - 25892 taken'),
(22,1,41999,0,0,9,0,25629,0,0,0,0,0,'','Honor Guard - 25629 taken'),
(22,2,41999,0,0,9,0,25629,0,0,0,0,0,'','Honor Guard - 25629 taken (hostile path)'),
(22,1,-9001317,0,0,47,0,25760,2,0,0,0,0,'','Valoren rendezvous - 25760 complete'),
(22,1,-9001358,0,0,9,0,25754,0,0,0,0,0,'','NW scout - 25754 taken'),
(22,1,-9001359,0,0,9,0,25754,0,0,0,0,0,'','Tunnel scout - 25754 taken'),
(22,1,-9001360,0,0,47,0,25752,2,0,0,0,0,'','Bomb controller S - 25752 complete'),
(22,1,-9001361,0,0,47,0,25752,2,0,0,0,0,'','Bomb controller N - 25752 complete'),
(22,1,-9001407,0,0,9,0,25755,0,0,0,0,0,'','V2 exit - 25755 taken'),
(22,1,-9001435,0,0,9,0,25951,0,0,0,0,0,'','44413 whisper - 25951 taken'),
(22,1,-9001436,0,0,9,0,25626,0,0,0,0,0,'','Crucible - 25626 taken'),
(22,1,-9001437,0,0,9,0,25951,0,0,0,0,0,'','East bridge - 25951 taken'),
(22,1,-9001444,0,0,9,0,25898,0,0,0,0,0,'','Surface Jorlan - 25898 taken'),
(22,1,-9001447,0,0,9,0,26219,0,0,0,0,0,'','Boarding trigger - 26219 taken');
-- ============================================================================
-- 41562/41281 wounded lookout SAI trigger for 25892 (entry-level; only my
-- spawns of 41562 exist)
-- ============================================================================
INSERT INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, event_param5, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES
(41562,0,0,1,10,0,100,0,1,30,30000,30000,0,1,0,0,1,0,0,0,7,0,0,0,0,0,0,0,'Injured Lookout - the Admiral did not make it'),
(41562,0,1,0,61,0,100,0,0,0,0,0,0,33,41562,0,0,0,0,0,7,0,0,0,0,0,0,0,'Injured Lookout - 25892 credit');
-- ---- 41785 escape seahorse ride (path 884, ExactPathFlying nodes) ----
INSERT INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, event_param5, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES
(41785,0,0,0,54,0,100,0,0,0,0,0,0,80,4178500,0,0,0,0,0,1,0,0,0,0,0,0,0,'Escape Seahorse - just summoned');
DELETE FROM smart_scripts WHERE source_type = 9 AND entryorguid = 4178500;
INSERT INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, event_param5, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES
(4178500,9,0,0,0,0,100,0,500,500,0,0,0,59,1,0,0,0,0,0,1,0,0,0,0,0,0,0,'Escape Seahorse - set run'),
(4178500,9,1,0,0,0,100,0,200,200,0,0,0,69,1,0,1,0,0,0,8,0,0,0,-6551.63,4248.40,-475.55,0,'Escape Seahorse - wp1'),
(4178500,9,2,0,0,0,100,0,3500,3500,0,0,0,69,2,0,1,0,0,0,8,0,0,0,-6550.71,4281.13,-475.55,0,'Escape Seahorse - wp2 valve'),
(4178500,9,3,0,0,0,100,0,4500,4500,0,0,0,69,3,0,1,0,0,0,8,0,0,0,-6548.23,4351.03,-475.55,0,'Escape Seahorse - wp3'),
(4178500,9,4,0,0,0,100,0,5000,5000,0,0,0,69,4,0,1,0,0,0,8,0,0,0,-6545.46,4417.79,-475.55,0,'Escape Seahorse - wp4'),
(4178500,9,5,0,0,0,100,0,6000,6000,0,0,0,69,5,0,1,0,0,0,8,0,0,0,-6544.46,4508.55,-475.55,0,'Escape Seahorse - wp5 lip'),
(4178500,9,6,0,0,0,100,0,5000,5000,0,0,0,69,6,0,1,0,0,0,8,0,0,0,-6586.73,4547.96,-517.41,0,'Escape Seahorse - wp6 over shell'),
(4178500,9,7,0,0,0,100,0,4500,4500,0,0,0,69,7,0,1,0,0,0,8,0,0,0,-6628.25,4524.40,-546.83,0,'Escape Seahorse - wp7 descent'),
(4178500,9,8,0,0,0,100,0,4500,4500,0,0,0,69,8,0,1,0,0,0,8,0,0,0,-6631.98,4485.71,-574.83,0,'Escape Seahorse - wp8'),
(4178500,9,9,0,0,0,100,0,4500,4500,0,0,0,69,9,0,1,0,0,0,8,0,0,0,-6618.48,4438.34,-591.22,0,'Escape Seahorse - wp9'),
(4178500,9,10,0,0,0,100,0,4500,4500,0,0,0,69,10,0,1,0,0,0,8,0,0,0,-6611.28,4367.85,-582.53,0,'Escape Seahorse - wp10'),
(4178500,9,11,0,0,0,100,0,4500,4500,0,0,0,69,11,0,1,0,0,0,8,0,0,0,-6607.28,4310.52,-564.53,0,'Escape Seahorse - wp11 ledge camp'),
(4178500,9,12,0,0,0,100,0,4000,4000,0,0,0,41,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Escape Seahorse - despawn/eject at camp');

-- ============================================================================
-- END OF BATCH B
-- Not SQL-expressible here (C++ needed, see batch report): vision remote
-- auto-complete (25659), 76569 reinforcement ping, charmed-vehicle kill credit
-- verification (25859/25752), real Pincer X2 transport (GO 203620/TaxiPath
-- 2208), 80674 backup-credit script, 78332 ScriptEffect (substituted via SAI
-- invoker-cast of 78263).
-- ============================================================================
