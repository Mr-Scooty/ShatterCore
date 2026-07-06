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
#include "ObjectAccessor.h"
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
#include <cmath>

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
    // Sinestra Phase 1 / 3
    SPELL_DRAINED                       = 89350,  // -40% damage done while in Phase 1
    SPELL_WRACK                         = 92955,  // 25-man
    SPELL_WRACK_10N                     = 89421,  // 10-man
    SPELL_FLAME_BREATH                  = 92944,  // 25-man
    SPELL_FLAME_BREATH_10N              = 90125,  // 10-man
    SPELL_TWILIGHT_SLICER               = 92852,  // Beam contact damage
    SPELL_TWILIGHT_SLICER_BEAM          = 92851,  // Beam visual channel between the orbs
    SPELL_TWILIGHT_PULSE                = 92958,  // Shadow Orb proximity damage
    SPELL_TWILIGHT_BLAST                = 89280,  // Anti-kiting nuke
    SPELL_CALL_FLAMES                   = 95855,  // Environmental effect at pull / phase transitions
    SPELL_PHASE_TRANSITION_VISUAL       = 64651,  // Sniffed companion visual to Call Flames

    // Twilight Whelps
    SPELL_TWILIGHT_SPIT                 = 92953,  // 25-man
    SPELL_TWILIGHT_SPIT_10N             = 89299,  // 10-man
    SPELL_TWILIGHT_ESSENCE_AURA         = 89284,  // Periodic trigger aura carried by the essence pool

    // Phase 2
    SPELL_MANA_BARRIER                  = 87299,  // Converts damage taken into mana loss (spell_sinestra_mana_barrier)
    SPELL_TWILIGHT_EXTINCTION_CHANNEL   = 86227,  // Visible channel while Extinction builds up
    SPELL_TWILIGHT_EXTINCTION_DMG       = 87945,  // ~475k shadow to the whole room, resolved after the channel
    SPELL_SINESTRA_DUEL_CHANNEL         = 87220,  // Sinestra's endless beam at the channel target
    SPELL_TWILIGHT_INFUSION             = 87655,  // Sinestra -> egg siphon
    SPELL_TWILIGHT_INFUSION_VISUAL      = 95564,  // Egg-side siphon visual
    SPELL_EGG_SIPHON_VISUAL             = 95789,  // Egg-side siphon visual
    SPELL_TWILIGHT_CARAPACE             = 87654,  // Egg absorb shield

    // Calen
    SPELL_CALEN_DUEL_CHANNEL            = 87221,  // Calen's endless beam at Sinestra
    SPELL_FIERY_BARRIER_PERIODIC        = 87229,  // Periodic dome application (spell_calen_fiery_barrier)
    SPELL_FIERY_BARRIER_PROTECTION      = 87231,  // -99% damage taken inside the dome
    SPELL_PYRRHIC_FOCUS                 = 87323,  // +500% healing taken, self-burn (spell_calen_pyrrhic_focus)
    SPELL_ESSENCE_OF_THE_RED            = 87946,  // +100% haste, 5% mana/s, 3 min

    // Twilight Spitecaller
    SPELL_UNLEASH_ESSENCE               = 90028,  // 10-man
    SPELL_UNLEASH_ESSENCE_25            = 92947,  // 25-man
    SPELL_INDOMITABLE                   = 90045,  // 10-man: knockback + 40k + CC immunity
    SPELL_INDOMITABLE_25                = 92946,  // 25-man
    SPELL_INDOMITABLE_BUFF              = 90044,  // Dispellable enrage component

    // Twilight Drake
    SPELL_TWILIGHT_BREATH               = 76817,  // 10-man
    SPELL_TWILIGHT_BREATH_25            = 92942,  // 25-man
    SPELL_ABSORB_ESSENCE                = 90107,  // +10% damage/health per absorbed pool

    // Misc
    SPELL_BERSERK                       = 26662
};

enum Events
{
    // Sinestra Phase 1 / 3
    EVENT_WRACK = 1,
    EVENT_FLAME_BREATH,
    EVENT_TWILIGHT_SLICER,
    EVENT_SPAWN_TWILIGHT_WHELPS,
    EVENT_CHECK_MELEE_RANGE,
    EVENT_BERSERK,

    // Sinestra Phase 2
    EVENT_EXTINCTION_RESOLVE,
    EVENT_ACTIVATE_CALEN,
    EVENT_MANA_CHECK,
    EVENT_FORCED_WINDOW,
    EVENT_CLOSE_CARAPACE_WINDOW,
    EVENT_SUMMON_SPITECALLER,
    EVENT_SUMMON_DRAKE,

    // Sinestra Phase 3
    EVENT_SLAY_CALEN,
    EVENT_GRANT_ESSENCE,
    EVENT_RESUME_ABILITIES,

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
    EVENT_CHECK_NEARBY_WHELPS,

    // Calen
    EVENT_CALEN_TAUNT,

    // Twilight Spitecaller
    EVENT_UNLEASH_ESSENCE,
    EVENT_ENGAGE_FAILSAFE,

    // Twilight Drake
    EVENT_TWILIGHT_BREATH,
    EVENT_ABSORB_ESSENCE_CHECK
};

enum Phases
{
    PHASE_1 = 1,
    PHASE_2 = 2,
    PHASE_3 = 3
};

// Encounter-internal actions. The cross-script actions (eggs, Calen) live in
// bastion_of_twilight.h as SinestraActions (values 10+).
enum Actions
{
    ACTION_WRACK_DISPELLED = 1,
    ACTION_MARK_AS_RESPAWNED = 2,
    ACTION_SET_PAIRED_ORB = 3,
    ACTION_SET_FIXATE_TARGET = 4
};

enum Points
{
    POINT_NONE = 0,
    POINT_ENTER_ROOM = 1,
    POINT_LAND = 2
};

// Positions taken from the 25H sniff.
Position const WhelpSpawnerPos[] =
{
    { -1003.56f, -588.10f, 455.98f },
    { -1146.20f, -684.11f, 459.44f },
    {  -837.81f, -761.04f, 466.38f },
    { -1139.64f, -668.27f, 457.50f },
    {  -961.92f, -592.00f, 453.75f }
};

Position const SpitecallerSpawnPos = { -1123.58f, -826.16f, 466.90f };
Position const SpitecallerEntryPos = { -1027.36f, -800.61f, 438.68f }; // room floor near the rear tunnel - verify in-game
Position const DrakePerchPos       = { -1195.29f, -614.29f, 500.28f };
Position const DrakeLandingPos     = { -1043.00f, -709.00f, 438.20f }; // main floor landing point - verify in-game
Position const CacheSpawnPos       = { -962.9202f, -749.7118f, 438.5929f, 4.031712f };

// Twilight Essence revival range for dead whelps
float const TWILIGHT_ESSENCE_REVIVAL_RANGE = 5.0f;

// Phase 2 mana economy. Mana drained per barrier tick is
// missingHealthPct * maxMana * MANA_DRAIN_FACTOR, i.e. the raid opens the
// first carapace window (100% -> 50% mana) after dealing 50 / MANA_DRAIN_FACTOR
// percent of her max health in damage.
float const MANA_DRAIN_FACTOR       = 4.0f;
float const WINDOW_OPEN_MANA_PCT    = 50.0f;
float const WINDOW_REFILL_MANA_PCT  = 90.0f;

struct boss_sinestra final : public BossAI
{
    boss_sinestra(Creature* creature) : BossAI(creature, DATA_SINESTRA)
    {
        Initialize();
        // She fights the entire encounter from her spot; ranged cheese is
        // punished by Twilight Blast instead of chasing.
        SetCombatMovement(false);
    }

    void Initialize()
    {
        _eggsDestroyed = 0;
        _windowOpen = false;
        _duelLost = false;
        _duelBanterDone = false;
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
        events.ScheduleEvent(EVENT_WRACK, 15s, 0, PHASE_1);
        events.ScheduleEvent(EVENT_FLAME_BREATH, 20s, 0, PHASE_1);
        events.ScheduleEvent(EVENT_TWILIGHT_SLICER, 30s, 0, PHASE_1);
        events.ScheduleEvent(EVENT_SPAWN_TWILIGHT_WHELPS, 16s, 0, PHASE_1);
        events.ScheduleEvent(EVENT_CHECK_MELEE_RANGE, 2s); // phase-agnostic, internally gated
        events.ScheduleEvent(EVENT_BERSERK, 15min);
    }

    void Reset() override
    {
        _Reset();
        Initialize();

        // Sinestra always appears at 60% health with Drained debuff
        me->SetHealth(me->GetMaxHealth() * 60 / 100);
        me->SetFullPower(POWER_MANA);
        me->SetReactState(REACT_AGGRESSIVE);
        DoCastSelf(SPELL_DRAINED, true);
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
        Talk(SAY_DEATH);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_WRACK);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_WRACK_10N);

        // Spawn Cache of the Broodmother loot chest (604800 seconds = 7 days)
        me->SummonGameObject(Is25ManRaid() ? GO_CACHE_OF_THE_BROODMOTHER_25H : GO_CACHE_OF_THE_BROODMOTHER_10H,
            CacheSpawnPos.GetPositionX(), CacheSpawnPos.GetPositionY(), CacheSpawnPos.GetPositionZ(), CacheSpawnPos.GetOrientation(), QuaternionData(), 604800);
    }

    void EnterEvadeMode(EvadeReason /*why*/) override
    {
        _EnterEvadeMode();
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        summons.DespawnAll();
        // _DespawnAtEvade sets the boss state to FAIL, which makes the
        // instance script respawn/reset the eggs and Calen.
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
                // Ensure whelps spawn without any immunity flags
                summon->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
                summon->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_ATTACKABLE_1);

                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    summon->AI()->AttackStart(target);
                break;
            case NPC_TWILIGHT_SPITECALLER:
            case NPC_TWILIGHT_DRAKE_SINESTRA:
                // They engage on their own once they have entered the room.
                break;
            default:
                break;
        }
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage) override
    {
        // Phase 1 -> 2 trip at 30%. Clamp the killing blow so she lands exactly
        // on the line and can never skip the transition.
        if (events.IsInPhase(PHASE_1))
        {
            if (me->HealthBelowPctDamaged(30, damage))
            {
                uint32 threshold = me->CountPctFromMaxHealth(30);
                damage = me->GetHealth() > threshold ? uint32(me->GetHealth() - threshold) : 0;
                StartPhaseTwo();
            }
            return;
        }

        // Phase 2: unkillable. The Mana Barrier heals the loss back and pays in
        // mana; this clamp only guards the one-shot edge case.
        if (events.IsInPhase(PHASE_2) && damage >= me->GetHealth())
            damage = uint32(me->GetHealth() - 1);
    }

    void DoAction(int32 action) override
    {
        switch (action)
        {
            case ACTION_EGG_DESTROYED:
                if (events.IsInPhase(PHASE_2) && ++_eggsDestroyed >= 2)
                    StartPhaseThree();
                break;
            case ACTION_CALEN_DEFEATED:
                if (events.IsInPhase(PHASE_2) && !_duelLost)
                    OnDuelLost();
                break;
            default:
                break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        // Phase 2 runs an endless duel channel; without this exception the
        // casting guard would starve every Phase 2 event.
        if (me->HasUnitState(UNIT_STATE_CASTING) && !events.IsInPhase(PHASE_2))
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_WRACK:
                    // Never a tank, never someone already afflicted.
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true, false, -int32(Is25ManRaid() ? SPELL_WRACK : SPELL_WRACK_10N)))
                        DoCast(target, Is25ManRaid() ? SPELL_WRACK : SPELL_WRACK_10N, true);
                    events.Repeat(events.IsInPhase(PHASE_3) ? 60s : 70s);
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
                    SummonWhelpWave();
                    events.Repeat(50s);
                    break;
                case EVENT_BERSERK:
                    DoCastSelf(SPELL_BERSERK, true);
                    break;
                case EVENT_CHECK_MELEE_RANGE:
                    // Anti-kiting mechanic: Cast Twilight Blast on targets out of melee range
                    if (!events.IsInPhase(PHASE_2))
                        if (Unit* victim = me->GetVictim())
                            if (!me->IsWithinMeleeRange(victim))
                                me->CastSpell(victim, SPELL_TWILIGHT_BLAST, true);
                    events.Repeat(2s);
                    break;

                // ---- Phase 2
                case EVENT_EXTINCTION_RESOLVE:
                    ResolveExtinction();
                    break;
                case EVENT_ACTIVATE_CALEN:
                    if (Creature* calen = instance->GetCreature(DATA_CALEN))
                        calen->AI()->DoAction(ACTION_CALEN_ENGAGE);
                    break;
                case EVENT_MANA_CHECK:
                    HandleManaCheck();
                    break;
                case EVENT_FORCED_WINDOW:
                    // Anti-deadlock failsafe: a stalled raid still gets a window.
                    OpenCarapaceWindow();
                    break;
                case EVENT_CLOSE_CARAPACE_WINDOW:
                    CloseCarapaceWindow();
                    break;
                case EVENT_SUMMON_SPITECALLER:
                    me->SummonCreature(NPC_TWILIGHT_SPITECALLER, SpitecallerSpawnPos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 20s);
                    events.Repeat(Is25ManRaid() ? 25s : 30s);
                    break;
                case EVENT_SUMMON_DRAKE:
                {
                    uint8 count = Is25ManRaid() ? 2 : 1;
                    for (uint8 i = 0; i < count; ++i)
                        me->SummonCreature(NPC_TWILIGHT_DRAKE_SINESTRA, DrakePerchPos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 20s);
                    events.Repeat(50s);
                    break;
                }

                // ---- Phase 3 transition
                case EVENT_SLAY_CALEN:
                    if (Creature* calen = instance->GetCreature(DATA_CALEN))
                    {
                        if (calen->IsAlive())
                        {
                            me->CastSpell(calen, SPELL_TWILIGHT_BLAST, true);
                            calen->AI()->DoAction(ACTION_CALEN_FINALE);
                        }
                    }
                    break;
                case EVENT_GRANT_ESSENCE:
                    // Primary path is Calen's own AoE cast in his finale; this
                    // covers players who were out of range or a missing Calen.
                    for (MapReference const& ref : me->GetMap()->GetPlayers())
                        if (Player* player = ref.GetSource())
                            if (player->IsAlive() && !player->IsGameMaster() && !player->HasAura(SPELL_ESSENCE_OF_THE_RED))
                                me->AddAura(SPELL_ESSENCE_OF_THE_RED, player);
                    break;
                case EVENT_RESUME_ABILITIES:
                    events.ScheduleEvent(EVENT_FLAME_BREATH, 20s, 0, PHASE_3);
                    events.ScheduleEvent(EVENT_WRACK, 15s, 0, PHASE_3);
                    events.ScheduleEvent(EVENT_TWILIGHT_SLICER, 30s, 0, PHASE_3);
                    events.ScheduleEvent(EVENT_SPAWN_TWILIGHT_WHELPS, 50s, 0, PHASE_3);
                    break;
                default:
                    break;
            }
        }

        DoMeleeAttackIfReady();
    }

private:
    void StartPhaseTwo()
    {
        events.SetPhase(PHASE_2);
        events.CancelEvent(EVENT_WRACK);
        events.CancelEvent(EVENT_FLAME_BREATH);
        events.CancelEvent(EVENT_TWILIGHT_SLICER);
        events.CancelEvent(EVENT_SPAWN_TWILIGHT_WHELPS);

        Talk(SAY_PHASE_2);
        me->InterruptNonMeleeSpells(true);
        me->AttackStop();
        me->SetReactState(REACT_PASSIVE);

        me->RemoveAurasDueToSpell(SPELL_DRAINED);
        // Heal to full BEFORE the barrier goes up - the barrier converts missing
        // health into mana loss, and the transition heal must not be billed.
        me->SetFullHealth();
        me->SetFullPower(POWER_MANA);
        DoCastSelf(SPELL_MANA_BARRIER, true);
        DoCastSelf(SPELL_PHASE_TRANSITION_VISUAL, true);
        DoCastSelf(SPELL_CALL_FLAMES, true);

        // Visible build-up; the damage resolves via EVENT_EXTINCTION_RESOLVE.
        DoCastSelf(SPELL_TWILIGHT_EXTINCTION_CHANNEL);
        events.ScheduleEvent(EVENT_EXTINCTION_RESOLVE, 10s, 0, PHASE_2);

        events.ScheduleEvent(EVENT_ACTIVATE_CALEN, 2s, 0, PHASE_2);
        events.ScheduleEvent(EVENT_MANA_CHECK, 1s, 0, PHASE_2);
        events.ScheduleEvent(EVENT_FORCED_WINDOW, 60s, 0, PHASE_2);
        events.ScheduleEvent(EVENT_SUMMON_SPITECALLER, 25s, 0, PHASE_2);
        events.ScheduleEvent(EVENT_SUMMON_DRAKE, 30s, 0, PHASE_2);
        events.ScheduleEvent(EVENT_SPAWN_TWILIGHT_WHELPS, 30s, 0, PHASE_2);
    }

    void ResolveExtinction()
    {
        me->InterruptNonMeleeSpells(true);
        // Lethal to everyone outside Calen's Fiery Barrier; players inside
        // carry the -99% damage taken aura (87231) and shrug it off.
        DoCastAOE(SPELL_TWILIGHT_EXTINCTION_DMG, true);

        if (_duelLost)
        {
            // The duel is lost: she loops Extinction until the raid falls.
            DoCastSelf(SPELL_TWILIGHT_EXTINCTION_CHANNEL);
            events.ScheduleEvent(EVENT_EXTINCTION_RESOLVE, 12s, 0, PHASE_2);
        }
        else
            BeginDuelChannel();
    }

    void BeginDuelChannel()
    {
        if (Creature* channelTarget = instance->GetCreature(DATA_SINESTRA_CHANNEL_TARGET))
        {
            me->SetFacingToObject(channelTarget);
            me->CastSpell(channelTarget, SPELL_SINESTRA_DUEL_CHANNEL);
        }
    }

    void HandleManaCheck()
    {
        uint32 maxMana = me->GetMaxPower(POWER_MANA);
        float manaPct = maxMana ? 100.0f * float(me->GetPower(POWER_MANA)) / float(maxMana) : 0.0f;

        if (_windowOpen)
        {
            // Siphoning the eggs: roughly 1% max mana per second flows back.
            if (manaPct < WINDOW_REFILL_MANA_PCT)
                me->ModifyPower(POWER_MANA, int32(maxMana / 100));
        }
        else if (manaPct < WINDOW_OPEN_MANA_PCT)
            OpenCarapaceWindow();

        // Out of mana with the duel still live: the barrier collapses outright
        // and she proceeds to Phase 3 (matches the retail zero-mana fallback).
        if (me->GetPower(POWER_MANA) == 0 && !_duelLost)
        {
            StartPhaseThree();
            return;
        }

        events.Repeat(1s);
    }

    void OpenCarapaceWindow()
    {
        if (_windowOpen || _duelLost || !events.IsInPhase(PHASE_2))
            return;

        _windowOpen = true;
        Talk(SAY_BARRIER_DISSIPATES);
        me->InterruptNonMeleeSpells(true);

        if (!_duelBanterDone)
        {
            _duelBanterDone = true;
            Talk(SAY_WEAKNESS_FOOL);
            if (Creature* calen = instance->GetCreature(DATA_CALEN))
                if (calen->IsAlive())
                    calen->AI()->Talk(SAY_CALEN_WEAKENING);
        }

        for (uint32 data : { DATA_EGG_LEFT, DATA_EGG_RIGHT })
        {
            if (Creature* egg = ObjectAccessor::GetCreature(*me, instance->GetGuidData(data)))
            {
                if (egg->IsAlive())
                {
                    egg->AI()->DoAction(ACTION_CARAPACE_DOWN);
                    me->CastSpell(egg, SPELL_TWILIGHT_INFUSION, true);
                }
            }
        }

        events.ScheduleEvent(EVENT_CLOSE_CARAPACE_WINDOW, 30s, 0, PHASE_2);
        events.RescheduleEvent(EVENT_FORCED_WINDOW, 60s, 0, PHASE_2);
    }

    void CloseCarapaceWindow()
    {
        if (!_windowOpen)
            return;

        _windowOpen = false;
        for (uint32 data : { DATA_EGG_LEFT, DATA_EGG_RIGHT })
            if (Creature* egg = ObjectAccessor::GetCreature(*me, instance->GetGuidData(data)))
                if (egg->IsAlive())
                    egg->AI()->DoAction(ACTION_CARAPACE_UP);

        if (!_duelLost && events.IsInPhase(PHASE_2))
            BeginDuelChannel();
    }

    void OnDuelLost()
    {
        // Calen fell: the carapace never drops again and she has infinite
        // staying power. Modeled as an Extinction loop until the raid wipes.
        _duelLost = true;
        if (_windowOpen)
            CloseCarapaceWindow();

        events.CancelEvent(EVENT_FORCED_WINDOW);
        events.CancelEvent(EVENT_CLOSE_CARAPACE_WINDOW);

        me->InterruptNonMeleeSpells(true);
        DoCastSelf(SPELL_TWILIGHT_EXTINCTION_CHANNEL);
        events.ScheduleEvent(EVENT_EXTINCTION_RESOLVE, 10s, 0, PHASE_2);
    }

    void StartPhaseThree()
    {
        events.SetPhase(PHASE_3);
        events.CancelEvent(EVENT_EXTINCTION_RESOLVE);
        events.CancelEvent(EVENT_MANA_CHECK);
        events.CancelEvent(EVENT_FORCED_WINDOW);
        events.CancelEvent(EVENT_CLOSE_CARAPACE_WINDOW);
        events.CancelEvent(EVENT_SUMMON_SPITECALLER);
        events.CancelEvent(EVENT_SUMMON_DRAKE);
        _windowOpen = false;

        Talk(SAY_PHASE_3);
        me->InterruptNonMeleeSpells(true);
        me->RemoveAurasDueToSpell(SPELL_MANA_BARRIER);
        me->SetFullHealth();
        me->SetReactState(REACT_AGGRESSIVE);
        DoCastSelf(SPELL_PHASE_TRANSITION_VISUAL, true);
        DoCastSelf(SPELL_CALL_FLAMES, true);

        events.ScheduleEvent(EVENT_SLAY_CALEN, 3s, 0, PHASE_3);
        events.ScheduleEvent(EVENT_GRANT_ESSENCE, 6s, 0, PHASE_3);
        // Grace period: she melees her tank but casts nothing while the raid
        // clears the leftover Phase 2 adds.
        events.ScheduleEvent(EVENT_RESUME_ABILITIES, 30s, 0, PHASE_3);
    }

    void SummonWhelpWave()
    {
        Talk(SAY_FEED_CHILDREN);

        uint32 whelpEntry = NPC_TWILIGHT_WHELP_10N;
        if (IsHeroic())
            whelpEntry = Is25ManRaid() ? NPC_TWILIGHT_WHELP_25H : NPC_TWILIGHT_WHELP_10H;
        else if (Is25ManRaid())
            whelpEntry = NPC_TWILIGHT_WHELP_25N;

        // One whelp per spawner point (positions from sniff).
        for (Position const& pos : WhelpSpawnerPos)
            me->SummonCreature(whelpEntry, pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 300s);
    }

    void SpawnTwilightSlicerOrbs()
    {
        // Use GetPlayerListInGrid instead of SelectTargetList to work with GM mode testing
        // SelectTargetList relies on threat list which is empty in GM mode
        std::list<Player*> players;
        me->GetPlayerListInGrid(players, 100.0f);

        players.remove_if([](Player* player) {
            return !player || !player->IsAlive();
        });

        if (players.empty())
            return;

        std::vector<Player*> targetVec(players.begin(), players.end());
        Trinity::Containers::RandomShuffle(targetVec);

        // If only 1 target (solo testing), use the same target for both orbs
        Unit* target1 = targetVec[0];
        Unit* target2 = (targetVec.size() >= 2) ? targetVec[1] : targetVec[0];

        Creature* orb1 = nullptr;
        Creature* orb2 = nullptr;

        if (target1)
        {
            Position spawnPos = target1->GetPosition();
            spawnPos.m_positionX += 5.0f;  // Offset slightly so orbs don't overlap
            orb1 = me->SummonCreature(NPC_SINESTRA_SHADOW_ORB, spawnPos, TEMPSUMMON_TIMED_DESPAWN, 16s);
            if (orb1 && orb1->AI())
                orb1->AI()->SetGUID(target1->GetGUID(), ACTION_SET_FIXATE_TARGET);
        }

        if (target2)
        {
            Position spawnPos = target2->GetPosition();
            spawnPos.m_positionX -= 5.0f;  // Offset in opposite direction
            orb2 = me->SummonCreature(NPC_SINESTRA_SHADOW_ORB, spawnPos, TEMPSUMMON_TIMED_DESPAWN, 16s);
            if (orb2 && orb2->AI())
                orb2->AI()->SetGUID(target2->GetGUID(), ACTION_SET_FIXATE_TARGET);
        }

        // Pair the orbs together (first orb tracks second orb for beam)
        if (orb1 && orb2 && orb1->AI())
            orb1->AI()->SetGUID(orb2->GetGUID(), ACTION_SET_PAIRED_ORB);
    }

    uint8 _eggsDestroyed;
    bool _windowOpen;
    bool _duelLost;
    bool _duelBanterDone;
};

// Calen (46277) - permanent spawn. Duels Sinestra during Phase 2 from his spawn
// position behind the Fiery Barrier and dies delivering Essence of the Red.
struct npc_sinestra_calen final : public ScriptedAI
{
    enum CalenState : uint8
    {
        CALEN_IDLE,
        CALEN_DUELING,
        CALEN_FINISHED
    };

    npc_sinestra_calen(Creature* creature) : ScriptedAI(creature), _instance(creature->GetInstanceScript()), _state(CALEN_IDLE), _powerWanesWarned(false)
    {
        SetCombatMovement(false);
    }

    void Reset() override
    {
        _events.Reset();
        _state = CALEN_IDLE;
        _powerWanesWarned = false;

        me->InterruptNonMeleeSpells(true);
        me->RemoveAurasDueToSpell(SPELL_PYRRHIC_FOCUS);
        me->SetStandState(UNIT_STAND_STATE_STAND);
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        me->SetReactState(REACT_PASSIVE);
        me->SetFullHealth();

        // The Fiery Barrier dome rides along as a creature_addon aura; restore
        // it in case an aura wipe removed it.
        if (!me->HasAura(SPELL_FIERY_BARRIER_PERIODIC))
            DoCastSelf(SPELL_FIERY_BARRIER_PERIODIC, true);
    }

    void DoAction(int32 action) override
    {
        switch (action)
        {
            case ACTION_CALEN_ENGAGE:
                if (_state != CALEN_IDLE)
                    break;
                _state = CALEN_DUELING;
                Talk(SAY_CALEN_INTRO);
                _events.ScheduleEvent(EVENT_CALEN_TAUNT, 4s);
                if (Creature* sinestra = _instance->GetCreature(DATA_SINESTRA))
                {
                    me->SetFacingToObject(sinestra);
                    me->CastSpell(sinestra, SPELL_CALEN_DUEL_CHANNEL);
                }
                DoCastSelf(SPELL_PYRRHIC_FOCUS, true);
                break;
            case ACTION_CALEN_FINALE:
                if (_state == CALEN_FINISHED)
                    break;
                _state = CALEN_FINISHED;
                me->InterruptNonMeleeSpells(true);
                me->RemoveAurasDueToSpell(SPELL_PYRRHIC_FOCUS);
                Talk(SAY_CALEN_LAST_POWER);
                DoCastAOE(SPELL_ESSENCE_OF_THE_RED, true);
                me->SetStandState(UNIT_STAND_STATE_DEAD);
                me->DespawnOrUnsummon(15s, 5min);
                break;
            case ACTION_CALEN_DEFEATED:
                // Sent by the Pyrrhic Focus burn when he runs dry.
                Defeated();
                break;
            case ACTION_CALEN_RESET:
                Reset();
                break;
            default:
                break;
        }
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage) override
    {
        if (_state != CALEN_DUELING)
        {
            damage = 0;
            return;
        }

        if (damage >= me->GetHealth())
        {
            damage = 0;
            Defeated();
            return;
        }

        if (!_powerWanesWarned && me->HealthBelowPctDamaged(25, damage))
        {
            _powerWanesWarned = true;
            Talk(SAY_CALEN_POWER_WANES);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_CALEN_TAUNT:
                    Talk(SAY_CALEN_AGGRO);
                    break;
                default:
                    break;
            }
        }
    }

private:
    void Defeated()
    {
        if (_state == CALEN_FINISHED)
            return;
        _state = CALEN_FINISHED;

        Talk(SAY_CALEN_DEATH);
        me->InterruptNonMeleeSpells(true);
        me->RemoveAurasDueToSpell(SPELL_PYRRHIC_FOCUS);
        me->SetStandState(UNIT_STAND_STATE_DEAD);

        if (Creature* sinestra = _instance->GetCreature(DATA_SINESTRA))
            if (sinestra->IsAIEnabled())
                sinestra->AI()->DoAction(ACTION_CALEN_DEFEATED);

        me->DespawnOrUnsummon(8s, 5min);
    }

    InstanceScript* _instance;
    EventMap _events;
    CalenState _state;
    bool _powerWanesWarned;
};

// Pulsing Twilight Egg (46842) - two permanent spawns. Impervious behind the
// Twilight Carapace except while Sinestra siphons them.
struct npc_sinestra_pulsing_twilight_egg final : public ScriptedAI
{
    npc_sinestra_pulsing_twilight_egg(Creature* creature) : ScriptedAI(creature), _instance(creature->GetInstanceScript())
    {
        SetCombatMovement(false);
        me->SetReactState(REACT_PASSIVE);
    }

    void Reset() override
    {
        me->SetFullHealth();
        ApplyCarapace(true);
    }

    void DoAction(int32 action) override
    {
        switch (action)
        {
            case ACTION_CARAPACE_DOWN:
                ApplyCarapace(false);
                break;
            case ACTION_CARAPACE_UP:
                ApplyCarapace(true);
                break;
            case ACTION_EGG_RESET:
                Reset();
                break;
            default:
                break;
        }
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (Creature* sinestra = _instance->GetCreature(DATA_SINESTRA))
            if (sinestra->IsAIEnabled())
                sinestra->AI()->DoAction(ACTION_EGG_DESTROYED);
    }

    void UpdateAI(uint32 /*diff*/) override { }

private:
    void ApplyCarapace(bool apply)
    {
        if (apply)
        {
            me->RemoveAurasDueToSpell(SPELL_TWILIGHT_INFUSION);
            me->RemoveAurasDueToSpell(SPELL_TWILIGHT_INFUSION_VISUAL);
            if (!me->HasAura(SPELL_TWILIGHT_CARAPACE))
                DoCastSelf(SPELL_TWILIGHT_CARAPACE, true);
            me->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_ALL, true);
        }
        else
        {
            me->RemoveAurasDueToSpell(SPELL_TWILIGHT_CARAPACE);
            me->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_ALL, false);
            DoCastSelf(SPELL_EGG_SIPHON_VISUAL, true);
            DoCastSelf(SPELL_TWILIGHT_INFUSION_VISUAL, true);
        }
    }

    InstanceScript* _instance;
};

// Twilight Spitecaller (48415) - Phase 2 add. Unleash Essence cannot be stopped
// by interrupts; hard crowd control instead triggers Indomitable.
struct npc_sinestra_twilight_spitecaller final : public ScriptedAI
{
    static uint32 constexpr HARD_CC_MASK =
        (1 << MECHANIC_STUN) | (1 << MECHANIC_FEAR) | (1 << MECHANIC_SILENCE) |
        (1 << MECHANIC_HORROR) | (1 << MECHANIC_FREEZE) | (1 << MECHANIC_BANISH);

    npc_sinestra_twilight_spitecaller(Creature* creature) : ScriptedAI(creature), _engaged(false) { }

    void Reset() override
    {
        // Unleash Essence is immune to conventional interrupts; the intended
        // counterplay is loss-of-control effects (polymorph, disorient, ...).
        me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_INTERRUPT_CAST, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_INTERRUPT, true);
    }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        // Untargetable until it has entered the combat area (retail hotfix).
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        me->SetReactState(REACT_PASSIVE);
        me->GetMotionMaster()->MovePoint(POINT_ENTER_ROOM, SpitecallerEntryPos);
        _events.ScheduleEvent(EVENT_ENGAGE_FAILSAFE, 8s);
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type == POINT_MOTION_TYPE && pointId == POINT_ENTER_ROOM)
            Engage();
    }

    void SpellHit(WorldObject* /*caster*/, SpellInfo const* spellInfo) override
    {
        if (!(spellInfo->GetAllEffectsMechanicMask() & HARD_CC_MASK))
            return;

        if (me->HasAura(SPELL_INDOMITABLE_BUFF))
            return;

        // Surge of will: purge the control, punish the raid. The buff is a
        // dispellable enrage granting CC immunity while present.
        me->RemoveAurasWithMechanic(HARD_CC_MASK);
        DoCastSelf(Is25ManRaid() ? SPELL_INDOMITABLE_25 : SPELL_INDOMITABLE, true);
        DoCastSelf(SPELL_INDOMITABLE_BUFF, true);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_ENGAGE_FAILSAFE:
                    Engage();
                    break;
                case EVENT_UNLEASH_ESSENCE:
                    if (me->GetVictim() && !me->HasUnitState(UNIT_STATE_CASTING))
                    {
                        DoCastAOE(Is25ManRaid() ? SPELL_UNLEASH_ESSENCE_25 : SPELL_UNLEASH_ESSENCE);
                        _events.Repeat(22s);
                    }
                    else
                        _events.Repeat(2s);
                    break;
                default:
                    break;
            }
        }

        if (!UpdateVictim())
            return;

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        DoMeleeAttackIfReady();
    }

private:
    void Engage()
    {
        if (_engaged)
            return;
        _engaged = true;

        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        me->SetReactState(REACT_AGGRESSIVE);
        DoZoneInCombat();
        _events.CancelEvent(EVENT_ENGAGE_FAILSAFE);
        _events.ScheduleEvent(EVENT_UNLEASH_ESSENCE, 8s, 12s);
    }

    EventMap _events;
    bool _engaged;
};

// Twilight Drake (48436) - Phase 2 add. Flies in from its perch, breathes on
// the tank and consumes Twilight Essence pools for a stacking buff.
struct npc_sinestra_twilight_drake final : public ScriptedAI
{
    npc_sinestra_twilight_drake(Creature* creature) : ScriptedAI(creature), _landed(false) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        me->SetDisableGravity(true);
        me->SetCanFly(true);
        me->SetReactState(REACT_PASSIVE);
        me->GetMotionMaster()->MoveLand(POINT_LAND, DrakeLandingPos);
        _events.ScheduleEvent(EVENT_ENGAGE_FAILSAFE, 10s);
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if ((type == EFFECT_MOTION_TYPE || type == POINT_MOTION_TYPE) && pointId == POINT_LAND)
            Land();
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_ENGAGE_FAILSAFE:
                    Land();
                    break;
                case EVENT_TWILIGHT_BREATH:
                    DoCastVictim(Is25ManRaid() ? SPELL_TWILIGHT_BREATH_25 : SPELL_TWILIGHT_BREATH);
                    _events.Repeat(12s, 16s);
                    break;
                case EVENT_ABSORB_ESSENCE_CHECK:
                    if (Creature* pool = me->FindNearestCreature(NPC_TWILIGHT_ESSENCE_POOL, 6.0f))
                    {
                        pool->DespawnOrUnsummon();
                        DoCastSelf(SPELL_ABSORB_ESSENCE, true);
                    }
                    _events.Repeat(1s);
                    break;
                default:
                    break;
            }
        }

        if (!UpdateVictim())
            return;

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        DoMeleeAttackIfReady();
    }

private:
    void Land()
    {
        if (_landed)
            return;
        _landed = true;

        me->SetCanFly(false);
        me->SetDisableGravity(false);
        me->SetReactState(REACT_AGGRESSIVE);
        DoZoneInCombat();
        _events.CancelEvent(EVENT_ENGAGE_FAILSAFE);
        _events.ScheduleEvent(EVENT_TWILIGHT_BREATH, 8s, 12s);
        _events.ScheduleEvent(EVENT_ABSORB_ESSENCE_CHECK, 1s);
    }

    EventMap _events;
    bool _landed;
};

// Twilight Whelp AI
struct npc_sinestra_twilight_whelp final : public ScriptedAI
{
    npc_sinestra_twilight_whelp(Creature* creature) : ScriptedAI(creature), _instance(creature->GetInstanceScript())
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
        if (_hasDroppedPool)
            return;
        _hasDroppedPool = true;

        // The pool is summoned by the boss so it lands in her summon list and
        // gets cleaned up on evade/kill.
        Creature* summoner = _instance ? _instance->GetCreature(DATA_SINESTRA) : nullptr;

        Creature* essence = nullptr;
        if (summoner)
            essence = summoner->SummonCreature(NPC_TWILIGHT_ESSENCE_POOL, me->GetPosition(), TEMPSUMMON_MANUAL_DESPAWN);
        else
            essence = me->SummonCreature(NPC_TWILIGHT_ESSENCE_POOL, me->GetPosition(), TEMPSUMMON_MANUAL_DESPAWN);

        if (essence)
        {
            // Make the essence non-interactive
            essence->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            essence->SetReactState(REACT_PASSIVE);
            essence->AttackStop();
            essence->StopMoving();
            essence->CastSpell(essence, SPELL_TWILIGHT_ESSENCE_AURA, true);
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
                    DoCastVictim(Is25ManRaid() ? SPELL_TWILIGHT_SPIT : SPELL_TWILIGHT_SPIT_10N);
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
        // Revived whelps do not drop a second pool.
        if (action == ACTION_MARK_AS_RESPAWNED)
            _hasDroppedPool = true;
    }

private:
    InstanceScript* _instance;
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
                break;
            case ACTION_SET_FIXATE_TARGET:
                _fixateTarget = guid;
                break;
            default:
                break;
        }
    }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        // Make the orb float and move fast (speed ~2.5 from sniff data)
        me->SetDisableGravity(true);
        me->SetCanFly(true);
        me->SetSpeed(MOVE_RUN, 2.5f);
        me->SetSpeed(MOVE_WALK, 2.5f);
        me->SetSpeed(MOVE_FLIGHT, 2.5f);

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
                    if (ObjectAccessor::GetUnit(*me, _fixateTarget))
                        _events.ScheduleEvent(EVENT_UPDATE_FIXATE_POSITION, 100ms);
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

// Wrack (89421 / 92955). The DoT ramps every tick; a dispel bounces it to the
// two nearest eligible allies with the remaining duration but a fresh ramp.
class spell_sinestra_wrack : public AuraScript
{
    static float constexpr WRACK_GROWTH_PER_TICK = 1.22f;
    static float constexpr WRACK_TICK_CAP = 60000.0f;

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_WRACK, SPELL_WRACK_10N });
    }

    void HandleUpdatePeriodic(AuraEffect* aurEff)
    {
        uint32 tick = std::max<uint32>(aurEff->GetTickNumber(), 1);
        float base = float(std::max(aurEff->GetBaseAmount(), 1));
        aurEff->SetAmount(int32(std::min(base * std::pow(WRACK_GROWTH_PER_TICK, float(tick - 1)), WRACK_TICK_CAP)));
    }

    void HandleDispel(DispelInfo* /*dispelInfo*/)
    {
        Unit* caster = GetCaster();
        Unit* host = GetUnitOwner();
        if (!caster || !host)
            return;

        int32 remaining = GetDuration();
        if (remaining <= 2000) // about to expire anyway: no bounce
            return;

        std::list<Player*> candidates;
        host->GetPlayerListInGrid(candidates, 15.0f);
        candidates.remove_if([&](Player* player)
        {
            if (!player || !player->IsAlive() || player->IsGameMaster())
                return true;
            if (player == host)
                return true;
            if (player->HasAura(SPELL_WRACK) || player->HasAura(SPELL_WRACK_10N))
                return true;
            if (player == caster->GetVictim()) // never a tank
                return true;
            return false;
        });

        candidates.sort([host](Player* a, Player* b)
        {
            return host->GetDistance2d(a) < host->GetDistance2d(b);
        });

        uint8 applied = 0;
        for (Player* ally : candidates)
        {
            if (applied >= 2)
                break;

            // A fresh aura restarts the tick counter, which resets the ramp -
            // exactly the retail semantics.
            if (Aura* bounced = caster->AddAura(GetId(), ally))
            {
                bounced->SetMaxDuration(remaining);
                bounced->SetDuration(remaining);
                ++applied;
            }
        }
    }

    void Register() override
    {
        OnEffectUpdatePeriodic.Register(&spell_sinestra_wrack::HandleUpdatePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
        AfterDispel.Register(&spell_sinestra_wrack::HandleDispel);
    }
};

// Mana Barrier (87299). Each tick heals Sinestra back to full and pays for the
// healed amount in mana. See MANA_DRAIN_FACTOR for the economy.
class spell_sinestra_mana_barrier : public AuraScript
{
    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        PreventDefaultAction();

        Unit* target = GetTarget();
        uint32 missing = target->GetMaxHealth() - target->GetHealth();
        if (!missing)
            return;

        uint32 maxMana = target->GetMaxPower(POWER_MANA);
        int32 drain = int32(float(missing) / float(target->GetMaxHealth()) * float(maxMana) * MANA_DRAIN_FACTOR);

        target->ModifyHealth(int32(missing));
        target->ModifyPower(POWER_MANA, -std::max(drain, 1));
    }

    void Register() override
    {
        OnEffectPeriodic.Register(&spell_sinestra_mana_barrier::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

// Fiery Barrier (87229, rides on Calen). Applies the -99% damage taken aura to
// every living player inside the dome twice a second.
class spell_calen_fiery_barrier : public AuraScript
{
    static float constexpr BARRIER_RADIUS = 10.0f;

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_FIERY_BARRIER_PROTECTION });
    }

    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        PreventDefaultAction();

        Unit* target = GetTarget();
        std::list<Player*> players;
        target->GetPlayerListInGrid(players, BARRIER_RADIUS);
        for (Player* player : players)
        {
            if (!player->IsAlive())
                continue;

            if (Aura* protection = target->AddAura(SPELL_FIERY_BARRIER_PROTECTION, player))
            {
                protection->SetMaxDuration(2500);
                protection->SetDuration(2500);
            }
        }
    }

    void Register() override
    {
        OnEffectPeriodic.Register(&spell_calen_fiery_barrier::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

// Pyrrhic Focus (87323). Calen burns himself out: 2% of his max health per
// second. Healers extend the duel; the +500% healing taken effect (EFFECT_1)
// comes straight from the DBC.
class spell_calen_pyrrhic_focus : public AuraScript
{
    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        PreventDefaultAction();

        Unit* target = GetTarget();
        uint32 burn = target->CountPctFromMaxHealth(2);
        if (target->GetHealth() <= burn)
        {
            if (Creature* calen = target->ToCreature())
                if (calen->IsAIEnabled())
                    calen->AI()->DoAction(ACTION_CALEN_DEFEATED);
        }
        else
            target->ModifyHealth(-int32(burn));
    }

    void Register() override
    {
        OnEffectPeriodic.Register(&spell_calen_pyrrhic_focus::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

}

void AddSC_boss_sinestra()
{
    using namespace BastionOfTwilight;
    using namespace BastionOfTwilight::Sinestra;

    RegisterBastionOfTwilightCreatureAI(boss_sinestra);
    RegisterBastionOfTwilightCreatureAI(npc_sinestra_calen);
    RegisterBastionOfTwilightCreatureAI(npc_sinestra_pulsing_twilight_egg);
    RegisterBastionOfTwilightCreatureAI(npc_sinestra_twilight_spitecaller);
    RegisterBastionOfTwilightCreatureAI(npc_sinestra_twilight_drake);
    RegisterBastionOfTwilightCreatureAI(npc_sinestra_twilight_whelp);
    RegisterBastionOfTwilightCreatureAI(npc_sinestra_twilight_essence);
    RegisterBastionOfTwilightCreatureAI(npc_sinestra_shadow_orb);
    RegisterSpellScript(spell_sinestra_wrack);
    RegisterSpellScript(spell_sinestra_mana_barrier);
    RegisterSpellScript(spell_calen_fiery_barrier);
    RegisterSpellScript(spell_calen_pyrrhic_focus);
}
