-- Pterrordax Scavenger (36719): all Lost Isles spawns are airborne and use
-- the flying animation tier, but the creature had no flight movement template.
-- Without CAN_FLY the core applied gravity and the scavengers eventually fell
-- into the terrain or water below their correctly elevated spawn positions.
-- Retail uses airborne cyclic splines with flying movement for this creature.
DELETE FROM `creature_template_movement` WHERE `CreatureId`=36719;
INSERT INTO `creature_template_movement`
    (`CreatureId`,`Ground`,`Swim`,`Flight`,`Rooted`,`Random`)
VALUES
    (36719,0,0,2,0,0);
