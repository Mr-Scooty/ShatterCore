-- Pterrordax Scavenger (36719): correct the flight mode added in the previous
-- update. Flight=2 grants CAN_FLY but does not suppress gravity, so creatures
-- fall before their movement generator starts. Retail spawns use the
-- DISABLE_GRAVITY movement flag, represented by Flight=1 in this core.
-- Creature::CanFly treats either flight mode as flight-capable, so random and
-- path movement remain airborne with this setting.
DELETE FROM `creature_template_movement` WHERE `CreatureId`=36719;
INSERT INTO `creature_template_movement`
    (`CreatureId`,`Ground`,`Swim`,`Flight`,`Rooted`,`Random`)
VALUES
    (36719,0,0,1,0,0);
