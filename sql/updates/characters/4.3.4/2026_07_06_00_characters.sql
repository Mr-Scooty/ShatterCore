-- Raid Finder per-boss weekly loot lockouts (purged on the weekly reset)
CREATE TABLE IF NOT EXISTS `character_lfr_lockout` (
  `guid` INT UNSIGNED NOT NULL COMMENT 'Character Global Unique Identifier',
  `bossId` INT UNSIGNED NOT NULL COMMENT 'Boss creature_template entry',
  `killTime` INT UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`guid`, `bossId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='Raid Finder weekly loot lockouts';
