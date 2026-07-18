-- ============================================================================
-- Shimmering Expanse C++ follow-up bindings: HORDE mirrors 25963 / 25972
-- Pairs with the Horde parameterization pass on vashjir_shimmering_expanse.cpp.
-- Apply AFTER shimmering_cpp.sql. DO NOT APPLY before the new build deploys.
--
-- C++ changes this file supports:
--   * spell 77741 (Rescue Flare) now serves 25898 (A, spotter 40645) AND
--     25972 (H, spotter 40921); shared balloon credit 41572.
--   * PlayerScript casts the 77861 backstop on 25972 accept too.
--   * Battlemaiden vision gating accepts the Horde mirror quests
--     25957 / 25966 / 26135 (same bunnies, vehicles, credits, item 55171).
--   * Temple credit bunny 41982 credits 26135 as well as 25626.
--   * KillRewarder audit result: possessed-vehicle kills (25859, vehicle
--     41225) and player-summon kills (25752/25963 bombing rays) natively
--     credit the controlling player (Unit::Kill ->
--     GetCharmerOrOwnerPlayerOrPlayerItself; LowerPlayerDamageReq accepts
--     player-controlled and player-summoned attackers). NO credit relay
--     was added - none is needed.
--
-- Spawn agents still owe (nothing in guid range 9001700+ yet):
--   * 40921 Blood Guard Toldrek surface copy near the shared balloon
--     (balloon 41572 is at -7378 3862 +6, phase 224; suggest Toldrek at
--     ~ -7370 3858 +0.5, PhaseId 224) - he is 25972's obj-1 proximity credit
--     (SAI: OOC-LoS + 25972 incomplete -> KC 40921 + text group 0) AND the
--     25973 "Welcome News" starter/25972 ender.
--   * Horde hub spawns: 40916 Captain Vilethorn (25963 starter/ender),
--     41770 Fiasco Sizzlegrin (25972-line giver); 40918 Fiasco assault copy
--     already spawned (guid 344771, -6924.83 4067.73 -467.35).
--   * 40918 gossip SAI (menu 11534): on select -> cast 78053 on player.
--     78053 = Forcecast Ruins Assault Horde: native chain -> 78051 (summon
--     Horde ray 41868 at dest-db [row present], aura 261 PhaseId 181 - the
--     HORDE ruins phase, NOT 179 - and KC 40918). Bombing targets
--     41249/41250/42549 and explosives 77330 are shared with 25752.
--
-- creature_text contract additions (text agents):
--   40921: group 0 = surface greeting/"fire the flare" line (H mirror of
--          40645 g0), group 1 = flare-success line (H mirror of 40645 g1;
--          spoken by my 77741 script) - SAME group numbers as 40645.
-- ============================================================================

-- ---------------------------------------------------------------------------
-- Horde bombing ray (mirror of 41247): vehicle 816 + shared explosives bar
-- ---------------------------------------------------------------------------
UPDATE `creature_template` SET `VehicleId`=816, `spell1`=77330 WHERE `entry`=41868;

-- ---------------------------------------------------------------------------
-- Phase 224 (surface rescue set: spotter + shared balloon 41572) - extend the
-- existing A-side window (ElseGroups 0/1) with the Horde window:
--   ElseGroup 2: 25972 in log (complete|incomplete)
--   ElseGroup 3: 25972 rewarded AND 25973 (H Welcome News) not yet rewarded
-- ---------------------------------------------------------------------------
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=26 AND `SourceGroup`=224 AND `ElseGroup` IN (2,3);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(26, 224, 4966, 0, 2, 47, 0, 25972, 10, 0, 0, 0, 0, '', 'P224: Honor and Privilege (H) in log'),
(26, 224, 4966, 0, 3,  8, 0, 25972,  0, 0, 0, 0, 0, '', 'P224: Honor and Privilege (H) rewarded'),
(26, 224, 4966, 0, 3,  8, 0, 25973,  0, 0, 1, 0, 0, '', 'P224: Welcome News (H) not yet rewarded'),
(26, 224, 4969, 0, 2, 47, 0, 25972, 10, 0, 0, 0, 0, '', 'P224: Honor and Privilege (H) in log'),
(26, 224, 4969, 0, 3,  8, 0, 25972,  0, 0, 0, 0, 0, '', 'P224: Honor and Privilege (H) rewarded'),
(26, 224, 4969, 0, 3,  8, 0, 25973,  0, 0, 1, 0, 0, '', 'P224: Welcome News (H) not yet rewarded');
