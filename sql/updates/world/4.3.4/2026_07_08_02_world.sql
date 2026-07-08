-- Fix "Necessary Roughness" (Kezan quest 24502).
--
-- Part A: Coach Crosscheck (37106) had no bark on accepting Necessary Roughness.
-- His creature_text group 1 ("Get into that shredder and win the game...",
-- BroadcastText 49026) was bound to accepting The Replacements (24488) instead of
-- Necessary Roughness (24502). Re-point it (sniff: the say fires as 24502 is accepted).
UPDATE `smart_scripts`
  SET `event_param1`=24502,
      `comment`='Coach Crosscheck - On Quest ''Necessary Roughness'' Taken - Say Line 1'
WHERE `entryorguid`=37106 AND `source_type`=0 AND `id`=1;

-- Part B: the clickable field boat 48526 (spellclick 70015 -> summon+ride the
-- shredder 37179) carries invisibility aura 90366. Retail reveals it with see-invis
-- spell 66143, which is absent from this core's Spell.dbc, so the prop is unclickable
-- and the quest cannot start. Drop the aura so quest-havers can see and click it
-- (the spellclick stays gated by conditions: quest 24502 taken + clicker is a player).
-- Also un-hides the prop for Fourth and Goal (28414), which shares it (spellclick 70075).
UPDATE `creature_template_addon` SET `auras`='' WHERE `entry`=48526;
UPDATE `creature_addon` ca JOIN `creature` c ON c.`guid`=ca.`guid`
   SET ca.`auras`='' WHERE c.`id`=48526 AND ca.`auras`='90366';
