-- Wailing Caverns: Naralex awakening event could not be started.
-- The "Let the event begin!" gossip option lived on menu 201, which is the gossip menu of the
-- sleeping Naralex (3679) - an unscripted NPC - so clicking it did nothing. The event is actually
-- driven by Muyoh <Disciple of Naralex> (3678, npc_disciple_of_naralex), whose script injects the
-- option from the DB once the four fanglords are dead. Muyoh's own menu (202) already carries the
-- same option row, so delete it from Naralex's menu; Naralex then only shows his flavor text
-- (698, "Naralex sleeps again!") with no dead-end option.
DELETE FROM `gossip_menu_option` WHERE `MenuID`=201 AND `OptionID`=0;
