-- Fourth and Goal (28414/24503): the Deathwing flyover (48572) rendered as a
-- corpse gliding across the stadium.
--
-- Root cause: the client only plays DEATHWING.M2's airborne animation while the
-- unit's hover-anim state is active; without it the model falls back to its
-- limp (dead-looking) pose. The retail create block (Goblin P2 sniff, packet
-- 25037) carries PlayHoverAnim = true alongside MovementFlags 512
-- (DisableGravity) and AnimTier 3 - retail even resets the anim tier to 0
-- right after create (packet 25038), so the flying look is driven purely by
-- the hover anim, not the anim tier.
--
-- Our core derives the create-block PLAY_HOVER_ANIM bit from
-- MOVEMENTFLAG_HOVER (Object.cpp, BuildCreateUpdateBlockForPlayer), and that
-- flag is only applied at spawn from creature_template_movement (Ground = 2 =
-- Hover). 48572 had no row, so the create packet never told the client to use
-- the hover animation; the script-side SetCanFly/SetDisableGravity calls in
-- IsSummonedBy could not fix that bit retroactively. Flight = 1 additionally
-- lets the core keep gravity disabled from spawn (retail movement flag 512).
DELETE FROM `creature_template_movement` WHERE `CreatureId` = 48572;
INSERT INTO `creature_template_movement` (`CreatureId`, `Ground`, `Swim`, `Flight`, `Rooted`, `Random`, `InteractionPauseTimer`) VALUES
(48572, 2, 0, 1, 0, 0, NULL);

-- The dead-on-his-side pose itself: HealthModifier 1e9 x base HP 42 (level 1,
-- class 1) = ~3.35 billion health, which exceeds INT32_MAX. The 4.3.4 client
-- reads UNIT_FIELD_HEALTH as signed, sees a negative value and renders the
-- unit as a corpse - no anim tier / hover / stand state can override that.
-- Retail health for the flyover was 610,591,800 (P2 sniff create block);
-- 42 x 14537900 reproduces it exactly and stays far below 2^31.
UPDATE `creature_template` SET `HealthModifier` = 14537900 WHERE `entry` = 48572;
