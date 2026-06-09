/*
* This file is part of the ShatterCore Project. See AUTHORS file for Copyright information
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 2 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "Containers.h"
#include "Creature.h"
#include "CreatureAIImpl.h"
#include "InstanceScript.h"
#include "Map.h"
#include "MotionMaster.h"
#include "ObjectMgr.h"
#include "PassiveAI.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellAuras.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "SpellMgr.h"
#include "TemporarySummon.h"
#include "Unit.h"
#include "bastion_of_twilight.h"

namespace BastionOfTwilight::Sinestra
{
enum Texts
{
    // Sinestra
    SAY_AGGRO                       = 0, // "We were fools to entrust an imbecile like Cho'gall with such a sacred duty! I will deal with you intruders myself!"
    SAY_FEED_CHILDREN               = 1, // "Feed, children! Take your fill from their meaty husks!"
    SAY_PHASE_2                     = 2, // "I tire of this. Do you see this clutch amidst which you stand? I have nurtured the spark within them, but that life-force is and always will be mine. Behold, power beyond your comprehension!"
    SAY_WEAKNESS_FOOL               = 3, // "You mistake this for weakness? Fool!"
    SAY_BARRIER_DISSIPATES          = 4, // "The barrier protecting the Pulsing Twilight Eggs dissipates as Sinestra harnesses their power!"
    SAY_PHASE_3                     = 5, // "Enough! Drawing upon this source will set us back months. You should feel honored to be worthy of its expenditure. Now... die!"
    SAY_DEATH                       = 6, // "Deathwing! I have fallen.... The brood... is lost."

    // Calen
    SAY_CALEN_INTRO                 = 0, // "Heroes, you are not alone in this dark place!"
    SAY_CALEN_AGGRO                 = 1, // "Sintharia, your master owes me a great debt -- one that I intend to extract from his consort's hide!"
    SAY_CALEN_POWER_WANES           = 2, // "Heroes! My power wanes...."
    SAY_CALEN_DEATH                 = 3, // "All is lost.... Forgive me, my Queen...."
    SAY_CALEN_WEAKENING             = 4, // "You are weakening, Sintharia! Accept the inevitable!"
    SAY_CALEN_LAST_POWER            = 5  // "The fires dim, champions.... Take this, the last of my power. Succeed where I have failed. Avenge me. Avenge the world...."
};

enum Spells
{
    // Sinestra Phase 1
    SPELL_DRAINED                       = 89350,  // Boss debuff - 40% damage reduction in Phase 1
    SPELL_WRACK                         = 92955,  // 89421 in 10-man, periodic shadow damage that increases each tick
    SPELL_WRACK_10N                     = 89421,
    SPELL_FLAME_BREATH                  = 92944,  // 90125 in 10-man, raid-wide fire damage
    SPELL_FLAME_BREATH_10N              = 90125,
    // Shadow Orb / Twilight Slicer spells (from Wowhead)
    SPELL_TWILIGHT_SLICER               = 92852,  // 100 yd range, instant - Fires beam of twilight energy
    SPELL_TWILIGHT_SLICER_BEAM          = 92851,  // 200 yd range, Channeled - Beam damage between orbs
    SPELL_TWILIGHT_PULSE                = 92958,  // Shadow Orb AoE damage pulse every 1 sec
    SPELL_TWILIGHT_BLAST                = 89280,  // Anti-kiting nuke
    SPELL_CALL_FLAMES                   = 95855,  // Environmental effect at pull

    // Twilight Whelps
    SPELL_TWILIGHT_SPIT                 = 92953,  // Shadow bolt + debuff
    SPELL_TWILIGHT_ESSENCE_AURA         = 89284,  // Visual aura cast by Twilight Essence on itself

    // Phase 2 Transition
    SPELL_MANA_BARRIER                  = 87299,  // Shield during transition
    SPELL_TWILIGHT_EXTINCTION           = 87945,  // 86226 in 10-man
    SPELL_TWILIGHT_EXTINCTION_10N       = 86226,

    // Calen
    SPELL_FIERY_BARRIER                 = 87231,  // Protection barrier

    // Misc
    SPELL_BERSERK                       = 26662
};

enum Events
{
    // Sinestra Phase 1
    EVENT_WRACK = 1,
    EVENT_FLAME_BREATH,
    EVENT_TWILIGHT_SLICER,
    EVENT_SPAWN_TWILIGHT_WHELPS,
    EVENT_CHECK_MELEE_RANGE,
    EVENT_BERSERK,

    // Twilight Whelps
    EVENT_TWILIGHT_SPIT,

    // Shadow Orbs
    EVENT_START_FIXATE,
    EVENT_UPDATE_FIXATE_POSITION,
    EVENT_START_BEAM,
    EVENT_BEAM_TICK,
    EVENT_TWILIGHT_PULSE,
    EVENT_DESPAWN_ORB,

    // Twilight Essence
    EVENT_CHECK_NEARBY_WHELPS
};

enum Phases
{
    PHASE_1 = 1,
    PHASE_2 = 2,
    PHASE_3 = 3
};

enum Actions
{
    ACTION_WRACK_DISPELLED = 1,
    ACTION_MARK_AS_RESPAWNED = 2,
    ACTION_SET_PAIRED_ORB = 3,
    ACTION_SET_FIXATE_TARGET = 4
};

enum Points
{
    POINT_NONE = 0
};

enum NPCs
{
    NPC_TWILIGHT_WHELP_25H              = 47265,
    NPC_TWILIGHT_WHELP_25N              = 48049,
    NPC_TWILIGHT_WHELP_10H              = 48048,
    NPC_TWILIGHT_WHELP_10N              = 48047,
    NPC_SHADOW_ORB                      = 49863,
    NPC_TWILIGHT_ESSENCE                = 48018,
    NPC_CALEN                           = 46277,
    NPC_PULSING_TWILIGHT_EGG            = 46842
};


Position const SinestraSpawnPos = { -1130.75f, -817.314f, 467.747f, 5.191174f };

// Twilight Essence revival range for dead whelps
float const TWILIGHT_ESSENCE_REVIVAL_RANGE = 5.0f;

struct boss_sinestra final : public BossAI
{
    boss_sinestra(Creature* creature) : BossAI(creature, DATA_SINESTRA)
    {
        Initialize();
    }

    void Initialize()
    {
        _wrackActive = false;
    }

    void JustEngagedWith(Unit* who) override
    {
        BossAI::JustEngagedWith(who);
        Talk(SAY_AGGRO);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

        // Ensure Sinestra starts at 60% health with Drained debuff in Phase 1
        me->SetHealth(me->GetMaxHealth() * 60 / 100);
        DoCastSelf(SPELL_DRAINED, true);
        DoCastSelf(SPELL_CALL_FLAMES, true);

        events.SetPhase(PHASE_1);
        events.ScheduleEvent(EVENT_WRACK, 5s, 0, PHASE_1);
        events.ScheduleEvent(EVENT_FLAME_BREATH, 20s, 0, PHASE_1);
        events.ScheduleEvent(EVENT_TWILIGHT_SLICER, 30s, 0, PHASE_1);
        events.ScheduleEvent(EVENT_SPAWN_TWILIGHT_WHELPS, 50s, 0, PHASE_1);
        events.ScheduleEvent(EVENT_CHECK_MELEE_RANGE, 2s, 0, PHASE_1);
        events.ScheduleEvent(EVENT_BERSERK, 10min);
    }

    void Reset() override
    {
        _Reset();
        Initialize();

        // Sinestra always appears at 60% health with Drained debuff
        me->SetHealth(me->GetMaxHealth() * 60 / 100);
        DoCastSelf(SPELL_DRAINED, true);
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
        Talk(SAY_DEATH);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

        // Spawn Cache of the Broodmother loot chest (604800 seconds = 7 days)
        me->SummonGameObject(GO_CACHE_OF_THE_BROODMOTHER, -962.9202f, -749.7118f, 438.5929f, 4.031712f, QuaternionData(), 604800);
    }

    void EnterEvadeMode(EvadeReason /*why*/) override
    {
        _EnterEvadeMode();
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        summons.DespawnAll();
        _DespawnAtEvade();
    }

    void JustSummoned(Creature* summon) override
    {
        summons.Summon(summon);

        switch (summon->GetEntry())
        {
            case NPC_TWILIGHT_WHELP_10N:
            case NPC_TWILIGHT_WHELP_10H:
            case NPC_TWILIGHT_WHELP_25N:
            case NPC_TWILIGHT_WHELP_25H:
                Talk(SAY_FEED_CHILDREN);

                // Ensure whelps spawn without any immunity flags
                summon->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
                summon->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_ATTACKABLE_1);

                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    summon->AI()->AttackStart(target);
                break;
            default:
                break;
        }
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage) override
    {
        // Phase 2 transition at 30% health
        if (me->HealthBelowPctDamaged(30, damage) && events.IsInPhase(PHASE_1))
        {
            events.SetPhase(PHASE_2);
            events.CancelEvent(EVENT_WRACK);
            events.CancelEvent(EVENT_FLAME_BREATH);
            events.CancelEvent(EVENT_TWILIGHT_SLICER);
            events.CancelEvent(EVENT_SPAWN_TWILIGHT_WHELPS);

            me->RemoveAurasDueToSpell(SPELL_DRAINED);
            DoCastSelf(SPELL_MANA_BARRIER, true);
            me->SetHealth(me->GetMaxHealth()); // Heals to 100%

            Talk(SAY_PHASE_2);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_WRACK:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    {
                        DoCast(target, Is25ManRaid() ? SPELL_WRACK : SPELL_WRACK_10N);
                        _wrackActive = true;
                    }
                    // Wrack reschedules itself after the previous one expires (handled by aura removal)
                    break;
                case EVENT_FLAME_BREATH:
                    DoCastAOE(Is25ManRaid() ? SPELL_FLAME_BREATH : SPELL_FLAME_BREATH_10N);
                    events.Repeat(20s);
                    break;
                case EVENT_TWILIGHT_SLICER:
                    SpawnTwilightSlicerOrbs();
                    events.Repeat(30s);
                    break;
                case EVENT_SPAWN_TWILIGHT_WHELPS:
                {
                    // Spawn 5 whelps from eggs around the room
                    uint32 whelpEntry = NPC_TWILIGHT_WHELP_10N;
                    if (IsHeroic())
                        whelpEntry = Is25ManRaid() ? NPC_TWILIGHT_WHELP_25H : NPC_TWILIGHT_WHELP_10H;
                    else if (Is25ManRaid())
                        whelpEntry = NPC_TWILIGHT_WHELP_25N;

                    for (uint8 i = 0; i < 5; ++i)
                    {
                        // Spawn whelps around the boss in a circle
                        float angle = (i * 2 * M_PI / 5);
                        float x = me->GetPositionX() + 30.0f * cos(angle);
                        float y = me->GetPositionY() + 30.0f * sin(angle);
                        float z = me->GetPositionZ();
                        me->SummonCreature(whelpEntry, x, y, z, 0.0f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 300000);
                    }
                    events.Repeat(50s, 60s);
                    break;
                }
                case EVENT_BERSERK:
                    DoCastSelf(SPELL_BERSERK, true);
                    break;
                case EVENT_CHECK_MELEE_RANGE:
                    // Anti-kiting mechanic: Cast Twilight Blast on targets out of melee range
                    if (Unit* victim = me->GetVictim())
                    {
                        if (!me->IsWithinMeleeRange(victim))
                        {
                            // DEBUG: Log that we're attempting to cast
                            TC_LOG_DEBUG("scripts", "Sinestra: Target out of melee range, casting Twilight Blast");

                            // Cast Twilight Blast - use triggered to bypass restrictions
                            me->CastSpell(victim, SPELL_TWILIGHT_BLAST, true);
                        }
                    }
                    events.ScheduleEvent(EVENT_CHECK_MELEE_RANGE, 2s, 0, events.IsInPhase(PHASE_1) ? PHASE_1 : PHASE_3);
                    break;
                default:
                    break;
            }
        }

        DoMeleeAttackIfReady();
    }

private:
    void SpawnTwilightSlicerOrbs()
    {
        TC_LOG_ERROR("scripts.sinestra", "SpawnTwilightSlicerOrbs called");

        // Use GetPlayerListInGrid instead of SelectTargetList to work with GM mode testing
        // SelectTargetList relies on threat list which is empty in GM mode
        std::list<Player*> players;
        me->GetPlayerListInGrid(players, 100.0f);

        // Filter to only alive players
        players.remove_if([](Player* player) {
            return !player || !player->IsAlive();
        });

        TC_LOG_ERROR("scripts.sinestra", "Twilight Slicer: Found {} players in range", players.size());

        if (players.empty())
        {
            TC_LOG_ERROR("scripts.sinestra", "Twilight Slicer: No players found, aborting");
            return;
        }

        std::vector<Player*> targetVec(players.begin(), players.end());

        // Shuffle for random selection
        Trinity::Containers::RandomShuffle(targetVec);

        // If only 1 target (solo testing), use the same target for both orbs
        Unit* target1 = targetVec[0];
        Unit* target2 = (targetVec.size() >= 2) ? targetVec[1] : targetVec[0];

        Creature* orb1 = nullptr;
        Creature* orb2 = nullptr;

        // Spawn first orb near first target
        if (target1)
        {
            Position spawnPos = target1->GetPosition();
            spawnPos.m_positionX += 5.0f;  // Offset slightly so orbs don't overlap
            TC_LOG_ERROR("scripts.sinestra", "Attempting to spawn Shadow Orb 1 (NPC {}) at ({}, {}, {})",
                NPC_SHADOW_ORB, spawnPos.GetPositionX(), spawnPos.GetPositionY(), spawnPos.GetPositionZ());

            orb1 = me->SummonCreature(NPC_SHADOW_ORB, spawnPos, TEMPSUMMON_TIMED_DESPAWN, 16s);
            if (orb1)
            {
                TC_LOG_ERROR("scripts.sinestra", "Shadow Orb 1 spawned successfully");
                if (orb1->AI())
                    orb1->AI()->SetGUID(target1->GetGUID(), ACTION_SET_FIXATE_TARGET);
            }
            else
            {
                TC_LOG_ERROR("scripts.sinestra", "FAILED to spawn Shadow Orb 1! Check creature_template for entry {}", NPC_SHADOW_ORB);
            }
        }

        // Spawn second orb near second target
        if (target2)
        {
            Position spawnPos = target2->GetPosition();
            spawnPos.m_positionX -= 5.0f;  // Offset in opposite direction
            TC_LOG_ERROR("scripts.sinestra", "Attempting to spawn Shadow Orb 2 (NPC {}) at ({}, {}, {})",
                NPC_SHADOW_ORB, spawnPos.GetPositionX(), spawnPos.GetPositionY(), spawnPos.GetPositionZ());

            orb2 = me->SummonCreature(NPC_SHADOW_ORB, spawnPos, TEMPSUMMON_TIMED_DESPAWN, 16s);
            if (orb2)
            {
                TC_LOG_ERROR("scripts.sinestra", "Shadow Orb 2 spawned successfully");
                if (orb2->AI())
                    orb2->AI()->SetGUID(target2->GetGUID(), ACTION_SET_FIXATE_TARGET);
            }
            else
            {
                TC_LOG_ERROR("scripts.sinestra", "FAILED to spawn Shadow Orb 2! Check creature_template for entry {}", NPC_SHADOW_ORB);
            }
        }

        // Pair the orbs together (first orb tracks second orb for beam)
        if (orb1 && orb2 && orb1->AI())
        {
            orb1->AI()->SetGUID(orb2->GetGUID(), ACTION_SET_PAIRED_ORB);
            TC_LOG_ERROR("scripts.sinestra", "Shadow Orbs paired for Twilight Slicer beam");
        }
    }

    bool _wrackActive;
};

// Twilight Whelp AI
struct npc_sinestra_twilight_whelp final : public ScriptedAI
{
    npc_sinestra_twilight_whelp(Creature* creature) : ScriptedAI(creature)
    {
        Initialize();
    }

    void Initialize()
    {
        _hasDroppedPool = false;
    }

    void Reset() override
    {
        _events.Reset();
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        _events.ScheduleEvent(EVENT_TWILIGHT_SPIT, 2s);
    }

    void JustDied(Unit* /*killer*/) override
    {
        // First death: spawn Twilight Essence pool at whelp's location
        if (!_hasDroppedPool)
        {
            _hasDroppedPool = true;

            // Spawn the Twilight Essence NPC at the whelp's death location
            Position pos = me->GetPosition();
            TC_LOG_DEBUG("scripts.sinestra", "Twilight Whelp JustDied: Spawning Twilight Essence at ({}, {}, {})", pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ());

            if (Creature* essence = me->SummonCreature(NPC_TWILIGHT_ESSENCE, pos, TEMPSUMMON_MANUAL_DESPAWN))
            {
                TC_LOG_DEBUG("scripts.sinestra", "Twilight Essence spawned successfully, applying visual aura");
                // Make the essence non-interactive
                essence->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                essence->SetReactState(REACT_PASSIVE);
                essence->AttackStop();
                essence->StopMoving();
                // Cast the visual aura on itself
                essence->CastSpell(essence, SPELL_TWILIGHT_ESSENCE_AURA, true);
            }
            else
            {
                TC_LOG_ERROR("scripts.sinestra", "Failed to spawn Twilight Essence NPC {} at whelp death location!", NPC_TWILIGHT_ESSENCE);
            }
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        _events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_TWILIGHT_SPIT:
                    DoCastVictim(SPELL_TWILIGHT_SPIT);
                    _events.Repeat(3s, 5s);
                    break;
                default:
                    break;
            }
        }

        DoMeleeAttackIfReady();
    }

    void DoAction(int32 action) override
    {
        if (action == ACTION_MARK_AS_RESPAWNED)
        {
            // Mark this whelp as respawned - it won't drop a pool on next death
            _hasDroppedPool = true;
            TC_LOG_DEBUG("scripts.sinestra", "Twilight Whelp marked as respawned - will not drop pool on death");
        }
    }

private:
    EventMap _events;
    bool _hasDroppedPool;
};

// Twilight Essence AI - handles reviving nearby dead Twilight Whelps
struct npc_sinestra_twilight_essence final : public ScriptedAI
{
    npc_sinestra_twilight_essence(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        // Start checking for nearby dead whelps
        _events.ScheduleEvent(EVENT_CHECK_NEARBY_WHELPS, 1s);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_CHECK_NEARBY_WHELPS:
                    ReviveNearbyWhelps();
                    _events.Repeat(2s);
                    break;
                default:
                    break;
            }
        }
    }

private:
    void ReviveNearbyWhelps()
    {
        // Find all dead Twilight Whelps within range
        std::list<Creature*> whelps;
        me->GetCreatureListWithEntryInGrid(whelps, NPC_TWILIGHT_WHELP_10N, TWILIGHT_ESSENCE_REVIVAL_RANGE);
        me->GetCreatureListWithEntryInGrid(whelps, NPC_TWILIGHT_WHELP_10H, TWILIGHT_ESSENCE_REVIVAL_RANGE);
        me->GetCreatureListWithEntryInGrid(whelps, NPC_TWILIGHT_WHELP_25N, TWILIGHT_ESSENCE_REVIVAL_RANGE);
        me->GetCreatureListWithEntryInGrid(whelps, NPC_TWILIGHT_WHELP_25H, TWILIGHT_ESSENCE_REVIVAL_RANGE);

        for (Creature* whelp : whelps)
        {
            if (!whelp->IsAlive())
            {
                TC_LOG_DEBUG("scripts.sinestra", "Twilight Essence reviving dead whelp at ({}, {}, {})",
                    whelp->GetPositionX(), whelp->GetPositionY(), whelp->GetPositionZ());

                // Properly revive the whelp in place
                whelp->setDeathState(JUST_RESPAWNED);
                whelp->SetFullHealth();
                whelp->SetReactState(REACT_AGGRESSIVE);
                whelp->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                whelp->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);

                // Mark it as respawned so it won't drop a pool on next death
                if (whelp->AI())
                {
                    whelp->AI()->DoAction(ACTION_MARK_AS_RESPAWNED);
                    whelp->AI()->Reset();
                }

                // Put it back in combat - find a player target
                if (Player* target = whelp->SelectNearestPlayer(100.0f))
                    whelp->AI()->AttackStart(target);

                // Display the revival emote
                whelp->TextEmote("%s is revived by the commingled essences!", whelp, true);

                // Despawn this essence pool after reviving a whelp
                me->DespawnOrUnsummon(500ms);
                return;
            }
        }
    }

    EventMap _events;
};

// Shadow Orb AI (for Twilight Slicer mechanic)
// Two orbs spawn, each fixating a random player
// They pulse shadow damage (Twilight Pulse) around them
// A beam between them (Twilight Slicer) damages players caught in it
struct npc_sinestra_shadow_orb final : public ScriptedAI
{
    npc_sinestra_shadow_orb(Creature* creature) : ScriptedAI(creature)
    {
        Initialize();
        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    }

    void Initialize()
    {
        _fixateTarget = ObjectGuid::Empty;
        _pairedOrb = ObjectGuid::Empty;
        _isFirstOrb = false;
    }

    void SetGUID(ObjectGuid const& guid, int32 id) override
    {
        switch (id)
        {
            case ACTION_SET_PAIRED_ORB:
                _pairedOrb = guid;
                _isFirstOrb = true; // The orb that receives the paired GUID is the "first" one responsible for beam
                TC_LOG_DEBUG("scripts.sinestra", "Shadow Orb {} received paired orb GUID", me->GetGUID().ToString());
                break;
            case ACTION_SET_FIXATE_TARGET:
                _fixateTarget = guid;
                TC_LOG_DEBUG("scripts.sinestra", "Shadow Orb {} received fixate target GUID", me->GetGUID().ToString());
                break;
            default:
                break;
        }
    }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        TC_LOG_ERROR("scripts.sinestra", "Shadow Orb IsSummonedBy called - orb spawned successfully");

        // Make the orb float and move fast (speed ~2.5 from sniff data)
        me->SetDisableGravity(true);
        me->SetCanFly(true);
        me->SetSpeed(MOVE_RUN, 2.5f);
        me->SetSpeed(MOVE_WALK, 2.5f);
        me->SetSpeed(MOVE_FLIGHT, 2.5f);

        // Schedule events
        _events.ScheduleEvent(EVENT_START_FIXATE, 200ms);  // Start following quickly
        _events.ScheduleEvent(EVENT_TWILIGHT_PULSE, 1s);   // Start pulsing every 1 sec
        _events.ScheduleEvent(EVENT_START_BEAM, 3s);       // Beam starts after 3s
        _events.ScheduleEvent(EVENT_DESPAWN_ORB, 15s);     // Despawn after 15s
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_START_FIXATE:
                    if (Unit* target = ObjectAccessor::GetUnit(*me, _fixateTarget))
                    {
                        TC_LOG_ERROR("scripts.sinestra", "Shadow Orb fixating on player {}", target->GetName());
                        // Start periodic movement updates toward target
                        _events.ScheduleEvent(EVENT_UPDATE_FIXATE_POSITION, 100ms);
                    }
                    else
                    {
                        TC_LOG_ERROR("scripts.sinestra", "Shadow Orb EVENT_START_FIXATE: Could not find fixate target!");
                    }
                    break;

                case EVENT_UPDATE_FIXATE_POSITION:
                    if (Unit* target = ObjectAccessor::GetUnit(*me, _fixateTarget))
                    {
                        // Use MovePoint to create smooth spline movement toward target
                        me->GetMotionMaster()->MovePoint(0, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());
                    }
                    _events.Repeat(500ms);  // Update movement target every 500ms
                    break;

                case EVENT_TWILIGHT_PULSE:
                    // Cast Twilight Pulse - AoE damage to players within 5 yards
                    CastTwilightPulse();
                    _events.Repeat(1s);
                    break;

                case EVENT_START_BEAM:
                    // Only the first orb handles beam damage to avoid double-damage
                    if (_isFirstOrb)
                    {
                        // Cast triggered beam visual so it doesn't interrupt MoveFollow
                        if (Creature* otherOrb = ObjectAccessor::GetCreature(*me, _pairedOrb))
                            me->CastSpell(otherOrb, SPELL_TWILIGHT_SLICER_BEAM, true);  // 92851 - triggered to not stop movement

                        _events.ScheduleEvent(EVENT_BEAM_TICK, 300ms);  // 0.3 second ticks per Wowhead
                    }
                    break;

                case EVENT_BEAM_TICK:
                    DamagePlayersInBeam();
                    _events.Repeat(300ms);  // 0.3 second ticks
                    break;

                case EVENT_DESPAWN_ORB:
                    me->DespawnOrUnsummon();
                    break;

                default:
                    break;
            }
        }
    }

private:
    void CastTwilightPulse()
    {
        // Twilight Pulse hits all players within 5 yards of the orb
        std::list<Player*> players;
        me->GetPlayerListInGrid(players, 5.0f);

        for (Player* player : players)
        {
            if (!player->IsAlive())
                continue;

            me->CastSpell(player, SPELL_TWILIGHT_PULSE, true);
        }
    }

    void DamagePlayersInBeam()
    {
        Creature* otherOrb = ObjectAccessor::GetCreature(*me, _pairedOrb);
        if (!otherOrb || !otherOrb->IsAlive())
            return;

        // Get positions of both orbs
        Position pos1 = me->GetPosition();
        Position pos2 = otherOrb->GetPosition();

        // Find all players and check if they're in the beam
        std::list<Player*> players;
        me->GetPlayerListInGrid(players, 100.0f);

        for (Player* player : players)
        {
            if (!player->IsAlive())
                continue;

            // Check if player is between the two orbs (within the beam)
            if (IsInBeam(player, pos1, pos2))
            {
                // Deal Twilight Slicer damage (SpellID 92852)
                me->CastSpell(player, SPELL_TWILIGHT_SLICER, true);
            }
        }
    }

    bool IsInBeam(Unit* target, Position const& orbPos1, Position const& orbPos2)
    {
        // Beam width tolerance (how wide the beam is) - approximately player hitbox width
        float const BEAM_WIDTH = 3.0f;

        Position targetPos = target->GetPosition();

        // Calculate the line segment between the two orbs
        float dx = orbPos2.GetPositionX() - orbPos1.GetPositionX();
        float dy = orbPos2.GetPositionY() - orbPos1.GetPositionY();
        float lineLength = std::sqrt(dx * dx + dy * dy);

        if (lineLength < 0.1f)
            return false;

        // Calculate the perpendicular distance from target to the line
        float tx = targetPos.GetPositionX() - orbPos1.GetPositionX();
        float ty = targetPos.GetPositionY() - orbPos1.GetPositionY();

        // Project target position onto the line
        float t = (tx * dx + ty * dy) / (lineLength * lineLength);

        // Check if projection falls between the two orbs
        if (t < 0.0f || t > 1.0f)
            return false;

        // Calculate closest point on line to target
        float closestX = orbPos1.GetPositionX() + t * dx;
        float closestY = orbPos1.GetPositionY() + t * dy;

        // Calculate distance from target to closest point
        float distX = targetPos.GetPositionX() - closestX;
        float distY = targetPos.GetPositionY() - closestY;
        float distance = std::sqrt(distX * distX + distY * distY);

        // Check if within beam width
        return distance <= BEAM_WIDTH;
    }

    EventMap _events;
    ObjectGuid _fixateTarget;
    ObjectGuid _pairedOrb;
    bool _isFirstOrb;
};

// Wrack spell script - handles dispel jumping
class spell_sinestra_wrack : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_WRACK, SPELL_WRACK_10N });
    }

    void HandleDispel(DispelInfo* dispelInfo)
    {
        if (Unit* target = GetTarget())
        {
            uint32 remainingDuration = GetDuration();

            // Find 2 nearest allies to jump to
            std::list<Player*> nearbyPlayers;
            target->GetPlayerListInGrid(nearbyPlayers, 15.0f);

            if (!nearbyPlayers.empty())
            {
                Trinity::Containers::RandomResize(nearbyPlayers, std::min<size_t>(2, nearbyPlayers.size()));

                for (Player* ally : nearbyPlayers)
                {
                    if (ally != target && ally->IsAlive())
                    {
                        if (Aura* newWrack = ally->AddAura(GetSpellInfo()->Id, ally))
                        {
                            newWrack->SetDuration(remainingDuration);
                            newWrack->SetMaxDuration(remainingDuration);
                        }
                    }
                }
            }

            if (Unit* dispeller = dispelInfo->GetDispeller()->ToUnit())
            {
            }
        }
    }

    void Register() override
    {
        OnDispel.Register(&spell_sinestra_wrack::HandleDispel);
    }
};

}

void AddSC_boss_sinestra()
{
    using namespace BastionOfTwilight;
    using namespace BastionOfTwilight::Sinestra;

    RegisterBastionOfTwilightCreatureAI(boss_sinestra);
    RegisterBastionOfTwilightCreatureAI(npc_sinestra_twilight_whelp);
    RegisterBastionOfTwilightCreatureAI(npc_sinestra_twilight_essence);
    RegisterBastionOfTwilightCreatureAI(npc_sinestra_shadow_orb);
    RegisterSpellScript(spell_sinestra_wrack);
}
