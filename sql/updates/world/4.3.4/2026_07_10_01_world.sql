-- Fourth and Goal (28414/24503): Deathwing flyover (48572) retail polish from
-- the Goblin P2 sniff create block.

-- Retail UnitFlags 0x2008300: IMMUNE_TO_PC | IMMUNE_TO_NPC | NOT_SELECTABLE
-- (we skip 0x8000, a modern-client swim-anim bit). Ours was 0, which left the
-- cinematic flyover selectable and attackable.
UPDATE `creature_template` SET `unit_flags` = 0x02000300 WHERE `entry` = 48572;

-- Retail DisplayID is 32809 (DEATHWING.M2 at display scale 2.0, with the
-- flyby sound kit attached); 33791 is the same model at scale 3.0 and no
-- sound kit. Confirmed present in the 4.3.4 CreatureDisplayInfo.dbc.
UPDATE `creature_template` SET `modelid1` = 32809 WHERE `entry` = 48572;

-- The flyby roars 23227-23230 fire as per-beat object sounds in the sniff
-- (leg 1 / leg 2 / circuit / 69988) and are now sent by the script; the leg-2
-- roar lands ~100ms before the yell text, so drop the yell's own copy of
-- 23228 to avoid playing it twice.
UPDATE `creature_text` SET `Sound` = 0 WHERE `CreatureID` = 48572 AND `GroupID` = 0;
