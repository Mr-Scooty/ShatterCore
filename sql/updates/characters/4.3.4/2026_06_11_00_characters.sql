--
-- Module system: allow module sql updates to be tracked in the updates table.
--
ALTER TABLE `updates`
  MODIFY `state` enum('RELEASED','ARCHIVED','MODULE') COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT 'RELEASED' COMMENT 'defines if an update is released, archived or owned by a module.';
