-- Ragnaros (Firelands): retail 4.3.4 health values
-- 10N  66,995,760 / 25N 200,987,280 / 10H  87,266,272 / 25H 290,486,744
-- (level 88 boss base health 85,892 x modifier)
UPDATE `creature_template` SET `HealthModifier`=780  WHERE `entry`=52409; -- 10 Normal
UPDATE `creature_template` SET `HealthModifier`=2340 WHERE `entry`=53797; -- 25 Normal
UPDATE `creature_template` SET `HealthModifier`=1016 WHERE `entry`=53798; -- 10 Heroic
UPDATE `creature_template` SET `HealthModifier`=3382 WHERE `entry`=53799; -- 25 Heroic
