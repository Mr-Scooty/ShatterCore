-- Remove four orphaned creature_addon rows (their spawns no longer exist); the loader
-- warns about these at every boot.
DELETE FROM `creature_addon` WHERE `guid` IN (254310,255860,255861,255864);
