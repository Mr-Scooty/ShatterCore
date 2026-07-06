-- Lord Rhyolith: retail health values on all transformation chains
--
-- Retail 4.3.4: 13,113,000 / 39,939,780 / 19,927,177 / 60,020,883
-- HealthModifier = target HP / creature_classlevelstats basehp3 (level 88 = 85892)
-- The 53772 (phase two/death) chain already carried these exact values; the spawn
-- chain 52558 governs the live pool (UpdateEntry is called without stat updates),
-- so bring it and the transitional 54192/54199 chains in line.
UPDATE `creature_template` SET `HealthModifier` = 152.669 WHERE `entry` IN (52558, 54192, 54199); -- 10N
UPDATE `creature_template` SET `HealthModifier` = 465     WHERE `entry` IN (52559, 54193, 54200); -- 25N
UPDATE `creature_template` SET `HealthModifier` = 232.003 WHERE `entry` IN (52560, 54194, 54201); -- 10H
UPDATE `creature_template` SET `HealthModifier` = 698.795 WHERE `entry` IN (52561, 54195, 54202); -- 25H
