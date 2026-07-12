-- Kezan: "Liberate the Kaja'mite" (14124) - Kablooey Bombs (48768, spell 67682) did nothing.
-- Spell 67682 EFFECT_0 is SPELL_EFFECT_ACTIVATE_OBJECT (action Open) on gameobjects within 5yd
-- of the blast; the deposit goober 195488 has no goober data, so the core's Use() path produced
-- nothing visible, nothing spawned the chunks and the quest could not progress.
-- Retail (Goblin_P2 sniff): the bombed deposit plays custom anim 0, despawns (respawn 120s per
-- the sniffed spawn row) and three Kaja'mite Chunk chests (195492, quest loot 48766) spawn
-- scattered 2-4yd around it. Scripted in spell_kezan_kablooey_bombs.
DELETE FROM `spell_script_names` WHERE `spell_id`=67682;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(67682, 'spell_kezan_kablooey_bombs');

-- Restrict the blast's gameobject area target to the bombable deposit goober so nearby
-- chests/decor are not "opened" by the default ACTIVATE_OBJECT handling.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry`=67682;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 67682, 0, 31, 0, 5, 195488, 0, 0, 0, 0, '', 'Kablooey! implicit gameobject target: only Kaja''mite Deposit goobers');

-- The 12 static Kaja'mite Chunk (195492) spawns were dynamic bombing results captured by the
-- sniff, not world spawns; chunks only exist after a deposit is blown up.
DELETE FROM `gameobject` WHERE `id`=195492;

-- Deposits respawn 120s after being bombed (sniffed spawn data; was 300).
UPDATE `gameobject` SET `spawntimesecs`=120 WHERE `id`=195488;
