-- Keep Training Dummy (48304) in The Lost Isles stationary.
UPDATE `creature`
SET `MovementType` = 0
WHERE `id` = 48304
  AND `map` = 648
  AND `zoneId` = 4737
  AND `areaId` = 4765
  AND `MovementType` <> 0;
