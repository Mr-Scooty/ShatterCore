-- Wailing Caverns: give Mutanus the Devourer a proper boss script.
-- His summon point lies in the nightmare pool below the navmesh, so chasing failed and he
-- evaded back to the pool and idled. boss_mutanus_the_devourer walks him out on a forced
-- spline before engaging and adds his classic abilities (Thundercrack, Terrify, Naralex's
-- Nightmare). The script's JustDied replaces the SmartAI on-death instance data row.
UPDATE `creature_template` SET `AIName`='', `ScriptName`='boss_mutanus_the_devourer' WHERE `entry`=3654;
DELETE FROM `smart_scripts` WHERE `entryorguid`=3654 AND `source_type`=0;
