-- Kezan quest chain: Batch B - party prep
-- "The New You" (14109/14110): fix the three party-outfit vendors.
-- Their TDB SmartAI casts the correct create-item spells (66780 -> 47045 Gappy,
-- 66781 -> 47046 Szabo, 66782 -> 47047 Missa) but listens on a dead gossip menu id (3).

-- action 85 (INVOKER_CAST): the create-item spells (66780/66781/66782) target
-- the caster, so the player must be the one casting them - action 11 would
-- create the item on the vendor instead.
UPDATE `smart_scripts` SET `event_param1` = 10620, `action_type` = 85 WHERE `entryorguid` = 35126 AND `source_type` = 0 AND `event_type` = 62;
UPDATE `smart_scripts` SET `event_param1` = 10622, `action_type` = 85 WHERE `entryorguid` = 35128 AND `source_type` = 0 AND `event_type` = 62;
UPDATE `smart_scripts` SET `event_param1` = 10624, `action_type` = 85 WHERE `entryorguid` = 35130 AND `source_type` = 0 AND `event_type` = 62;

-- Gate each option on The New You being active (either gender variant) and the
-- vendor's item not yet owned. Source 15 = GOSSIP_MENU_OPTION (SourceGroup = MenuID,
-- SourceEntry = OptionID); type 9 = QUESTTAKEN, type 2 = HAS_ITEM (negated).
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 15 AND `SourceGroup` IN (10620, 10622, 10624);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
-- Gappy Silvertooth (menu 10620) - Shiny Bling (47045)
(15, 10620, 0, 0, 1, 9, 0, 14109, 0, 0, 0, 0, 0, '', 'Gappy Silvertooth: show bling option while The New You (male) taken'),
(15, 10620, 0, 0, 1, 2, 0, 47045, 1, 0, 1, 0, 0, '', 'Gappy Silvertooth: hide bling option when player already has Shiny Bling'),
(15, 10620, 0, 0, 2, 9, 0, 14110, 0, 0, 0, 0, 0, '', 'Gappy Silvertooth: show bling option while The New You (female) taken'),
(15, 10620, 0, 0, 2, 2, 0, 47045, 1, 0, 1, 0, 0, '', 'Gappy Silvertooth: hide bling option when player already has Shiny Bling'),
-- Szabo (menu 10622) - Hip New Outfit (47046)
(15, 10622, 0, 0, 1, 9, 0, 14109, 0, 0, 0, 0, 0, '', 'Szabo: show outfit option while The New You (male) taken'),
(15, 10622, 0, 0, 1, 2, 0, 47046, 1, 0, 1, 0, 0, '', 'Szabo: hide outfit option when player already has Hip New Outfit'),
(15, 10622, 0, 0, 2, 9, 0, 14110, 0, 0, 0, 0, 0, '', 'Szabo: show outfit option while The New You (female) taken'),
(15, 10622, 0, 0, 2, 2, 0, 47046, 1, 0, 1, 0, 0, '', 'Szabo: hide outfit option when player already has Hip New Outfit'),
-- Missa Spekkies (menu 10624) - Cool Shades (47047)
(15, 10624, 0, 0, 1, 9, 0, 14109, 0, 0, 0, 0, 0, '', 'Missa Spekkies: show shades option while The New You (male) taken'),
(15, 10624, 0, 0, 1, 2, 0, 47047, 1, 0, 1, 0, 0, '', 'Missa Spekkies: hide shades option when player already has Cool Shades'),
(15, 10624, 0, 0, 2, 9, 0, 14110, 0, 0, 0, 0, 0, '', 'Missa Spekkies: show shades option while The New You (female) taken'),
(15, 10624, 0, 0, 2, 2, 0, 47047, 1, 0, 1, 0, 0, '', 'Missa Spekkies: hide shades option when player already has Cool Shades');
