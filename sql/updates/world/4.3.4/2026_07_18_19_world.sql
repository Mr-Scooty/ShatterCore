-- Abyssal reconciliation: the C++ module owns the Put It On scene on quest accept
-- (player_abyssal_depths casts the chain; npc_abyssal_merciless_double directs) - the SAI
-- accept trigger would double-fire it. The gossip replay path stays (same summon chain,
-- C++ director takes over; the 123460 completion cast no-ops on a rewarded quest).
DELETE FROM `smart_scripts` WHERE `entryorguid`=41666 AND `source_type`=0 AND `id`=0;
-- Rift escort: 93268/93302 SpellScripts summon the dive Taylor/Nazgrim; remove the SAI double.
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (44490,44540) AND `source_type`=0 AND `id`=0;
