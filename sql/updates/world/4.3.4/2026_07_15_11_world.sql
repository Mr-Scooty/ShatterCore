-- Irresistible Pool Pony (24864): register the pony-aura lure script. Retail lures
-- nearby pool hatchlings automatically (serverside proximity spell 71920 is absent from
-- the 4.3.4 client data); the spellclick path remains as a fallback.
DELETE FROM `spell_script_names` WHERE `spell_id`=71914;
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(71914,'spell_lost_isles_pool_pony_aura');
