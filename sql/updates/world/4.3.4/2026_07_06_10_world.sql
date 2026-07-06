-- Majordomo Staghelm (Firelands) retail health values
-- Level 88, exp 3 -> creature_classlevelstats basehp3 = 85892
-- 10N 51,019,848 | 25N 178,569,468 | 10H 124,715,184 | 25H 480,136,280
UPDATE `creature_template` SET `HealthModifier`=594  WHERE `entry`=52571; -- 10 Normal
UPDATE `creature_template` SET `HealthModifier`=2079 WHERE `entry`=53856; -- 25 Normal
UPDATE `creature_template` SET `HealthModifier`=1452 WHERE `entry`=53857; -- 10 Heroic
UPDATE `creature_template` SET `HealthModifier`=5590 WHERE `entry`=53858; -- 25 Heroic
