-- Infrared = Infradead (14238): the quest was uncompletable - all 58 SI:7 Assassins carry
-- 68322 (stealth + invisibility type 8) via creature_addon, but nothing granted the player
-- detection. Retail (P3 sniff): accepting the quest self-casts 68338 "Orc Scout" (heat-vision
-- dummy aura + force-cast 68336 summoning a private Orc Scout guardian that fights along).
UPDATE `quest_template_addon` SET `SourceSpellID`=68338 WHERE `ID`=14238;

-- Reapply the heat vision on login/zone-in while the quest is active (area 4782 = the
-- assassin drop zone); the linked detect (69141) is managed by spell_lost_isles_heat_vision.
DELETE FROM `spell_area` WHERE `spell`=68338;
INSERT INTO `spell_area` (`spell`,`area`,`quest_start`,`quest_end`,`aura_spell`,`racemask`,`gender`,`flags`,`quest_start_status`,`quest_end_status`) VALUES
(68338,4782,14238,0,0,0,2,3,10,11);

DELETE FROM `spell_script_names` WHERE `spell_id`=68338;
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(68338,'spell_lost_isles_heat_vision');

-- SI:7 Assassins fight with Sinister Strike (14873, sniffed).
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=36092;
DELETE FROM `smart_scripts` WHERE `entryorguid`=36092 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(36092,0,0,0,0,0,100,0,3500,5000,6000,9000,11,14873,0,0,0,0,0,2,0,0,0,0,0,0,0,'SI:7 Assassin - In Combat - Cast Sinister Strike');

-- Orc Scout companion: follows its goblin and fights with Charge + Rend (sniffed).
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=36100;
DELETE FROM `smart_scripts` WHERE `entryorguid`=36100 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(36100,0,0,0,54,0,100,0,0,0,0,0,29,2,2,0,0,0,0,7,0,0,0,0,0,0,0,'Orc Scout - On Just Summoned - Follow Summoner'),
(36100,0,1,0,4,0,100,0,0,0,0,0,11,100,0,0,0,0,0,2,0,0,0,0,0,0,0,'Orc Scout - On Aggro - Cast Charge'),
(36100,0,2,0,0,0,100,0,3000,5000,8000,12000,11,11977,0,0,0,0,0,2,0,0,0,0,0,0,0,'Orc Scout - In Combat - Cast Rend');
