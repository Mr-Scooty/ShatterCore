# Gilneas (Worgen 1–13) in-game walkthrough checklist — Grandma Wahl → Rut'theran

Create a fresh Worgen. Everything through **Evacuation (14397)** was already working;
this checklist covers the newly implemented stretch. Expected phase transitions are
listed so drift is obvious immediately.

## Duskhaven evacuation (phase 183)
- [ ] 14398 Grandma Wahl → 14399 Grandma's Lost It Alright → **14400 I Can't Wear This:
      the "Grandma's Good Clothes" chest is now lootable** (was spawned in the wrong phase)
- [ ] 14401 Grandma's Cat (spellclick on Chance), 14403/14404 Hayward brothers,
      14406 Crowley Orchard → 14405 Escape By Sea, 14416 Hungry Ettin → 14463 Horses
      for Duskhaven, 14402 Ready to Go, side quest 14412 Washed Up

## Greymane Manor (183 → +184 → 186)
- [ ] **14465 To Greymane Manor**: on accept a Swift Mountain Horse coach appears and
      auto-boards you, then drives the switchback road to the manor gate (~30 s) and
      ejects. Queen Mia is visible at the manor (phase 184 stacks on 183). If you
      dismount midway, Gwen has a "Ride to Greymane Manor." gossip re-ride
- [ ] **14466 The King's Observatory**: turn-in flips the world to phase 186
      (Duskhaven destroyed)
- [ ] **14467 Alas, Gilneas!** (restored quest): offered by King Genn right after;
      completing it plays **cinematic 167** (Greymane Manor reveal)
- [ ] 24438 Exodus: Genn (now ph186) sends you to Prince Liam at the marsh; ogre
      ambushers visible at the stagecoach crash site during the quest (phase 194)

## Marsh → Stormglen → Blackwald (186)
- [ ] **24468 Stranded at the Marsh: Swamp Crocolisks now spawn** at the marsh
- [ ] **24472 Introductions Are in Order: Koroth's Banner GO** now stands at the ogre camp
- [ ] 24483 Stormglen; inn gives rest bonus (new tavern trigger)
- [ ] 24484 Pest Control, 24501 Queen-Sized Troubles, 24495 Pieces of the Past
- [ ] 24578 The Blackwald → **24616 Losing Your Tail**: walking the trail triggers the
      Dark Scout ambush (you are stunned, scout taunts + whispers the talisman hint;
      use Belysra's Talisman to break free, then kill the scout — now hostile)
- [ ] 24617 Tal'doren → 24627 At Our Doorstep → 24628 Preparations
- [ ] **24646 Take Back What's Ours**: at pickup Tobias/Darius exchange the Scythe
      warning lines; blowing the Horn of Tal'doren summons 3 Tal'doren Trackers who
      fight the Dark Rangers at the cabin
- [ ] 24593 Neither Human Nor Beast — **verify Two Forms (68996) is learned at turn-in**
- [ ] 24673/24672/24592 (Return to Stormglen / Onwards and Upwards / Betrayal at
      Tempest's Reach), 24675 Last Meal, 24674 Slaves to No One, 24676 Push Them Out

## The Battle for Gilneas City (186 → +187 while on quest → 190 on reward)
- [ ] **24904**: approach Prince Liam at the gates with the quest — the full staged
      battle runs (~10 min): Liam's 7-line speech → march → Lorna arrives with the
      Emberstone Cannons (rideable) → abomination block → Gorerot leaps off his perch
      (use the Damaged Catapults) → march to Greymane Court → Genn/Sylvanas
      confrontation → Liam takes the arrow → **credit granted to every eligible player
      within 100 yd**. Abandoning resets the city (phase 187 drops); Lorna's "We need
      to give this another try." gossip restarts an idle controller
- [ ] **24902 The Hunt For Sylvanas**: accept summons Tobias; he escorts you (waits if
      you fall behind), Forsaken General beat, canal jump, cathedral scene with
      Warhowl/Sylvanas/Crenshaw (private, per-player) — quest completes at the plague
      reveal line
- [ ] 24903 Vengeance or Survival (talk; phase 188 appears alongside 190)
- [ ] **24920 Slowing the Inevitable**: click the parked bat at the wall → bombing run
      vehicle (bomb on action bar) credits catapults/invaders
- [ ] 24678 Knee-Deep (torch is usable flavor), 24602 Laid to Rest
- [ ] **24679 Patriarch's Blessing**: use Blessed Offerings at Aderic's Tomb → credit +
      camera ride over the looping funeral scene (Genn/Lorna/Darius; phase 187 re-adds
      during the ride and drops on exit; you are teleported out at the end)

## Keel Harbor & Endgame (→189 → 191)
- [ ] 24680 Keel Harbor (188 drops, 189 adds on accept)
- [ ] **24681 They Have Allies, But So Do We**: Glaive Throwers are clickable; War
      Wolves now carry Wolfmaw Outrider riders (kill-8 satisfiable)
- [ ] 24677 Flank the Forsaken, 24575 Liberation Day
- [ ] **26706 Endgame**: accept summons a Hippogryph that flies you to the orc gunship
      (static, out in the bay, phase 191). Deck event: Lorna "Attack!" → deck fight →
      rappel order (ropes) → mid-deck fight → furnace room → Korm Bonegrind becomes
      attackable → on his death Lorna yells and **a Wyvern auto-summons + credits and
      flies you back** with the gunship explosion FX behind you
- [ ] Turn in 26706 to Lorna at Keel Harbor (ph191 copy) → **map exit unlocks**
- [ ] **14434 Rut'theran Village** at Admiral Nightwind: reward teleports you to
      Rut'theran Village (map 1)
- [ ] **26385 Breaking Waves of Change**: Genn Greymane now stands beside Krennan at
      Rut'theran and offers it. (28517's Howling Oak Genn spawn has a placeholder Z —
      check he isn't floating/buried in Darnassus and `.gob`/`.npc near` snap if so)

## Known intentional deviations from retail
- The Orc Gunship is a static backdrop (retail sails a taxi loop); its moving-transport
  row was removed from `transports`.
- Funeral scene is a shared looping scene on static actors (retail used per-player clones).
- Phase 190 is held until Keel Harbor is accepted (retail dropped it earlier but used
  multi-phase NPC copies this DB doesn't have).
- Wyvern return flight is longer than retail (~50 s vs 24 s) because the static ship
  is farther out than the drifted live one was.
- 25331 (duplicate Grandma's Cat) and 14415 (Apparitions in the Orchard) are cut
  content and stay disabled.
