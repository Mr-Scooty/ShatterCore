-- Lost Isles Mail Bird (39169), Shipwreck Shore (4721).
--
-- A later build-30706 import moved this ambient mail bird above the survivor
-- camp at 563.18, 3130.33.  Both the original 4.3.4 spawn data and the retail
-- sniff place it at the western end of the shore near 339, 3262 instead.
-- Restore the Cataclysm position while preserving the bird and its mail-bag
-- aura cycle.
UPDATE `creature`
SET `position_x`=338.5267,
    `position_y`=3262.624,
    `position_z`=20.64684,
    `orientation`=5.353576,
    `wander_distance`=5,
    `MovementType`=1,
    `VerifiedBuild`=15595
WHERE `guid`=389379
  AND `id`=39169
  AND `map`=648
  AND `phaseId`=170;
