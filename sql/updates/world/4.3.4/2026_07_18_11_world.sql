-- Fix: freed Imprisoned Soldier (41582) despawn listened on TEXT_OVER (52) instead of
-- MOVEMENTINFORM (34); it never fired. Params: MovementType 8 = POINT_MOTION_TYPE, PointID 1.
UPDATE `smart_scripts` SET `event_type`=34, `event_param1`=8, `event_param2`=1 WHERE `entryorguid`=41582 AND `source_type`=0 AND `id`=2;
