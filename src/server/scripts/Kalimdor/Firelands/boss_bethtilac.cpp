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
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Beth'tilac, Firelands (10/25 Normal, 10/25 Heroic)
 *
 * Difficulty model (verified against 4.3.4 SpellDifficulty.dbc and Firelands sniffs):
 *  - Damage scaling: scripts always cast the 10N base ID; the core resolves the
 *    difficulty variant via SpellDifficulty (Ember Flare P1/P2, Venom Rain,
 *    Burning Acid, Boiling Spatter, Volatile Burst).
 *  - Heroic-only behavior (Fiery Web Spin, drone Fixate, Engorged Broodlings) is
 *    gated with IsHeroic(); spawn counts use RAID_MODE.
 *  - Timers follow the 4.3.4 DBM module: first Smoldering Devastation 82s after
 *    pull (8s cast, 90s cycle), spinners first 12s then 15s, spiderlings first
 *    12.5s then 30s, drone first 45s then 60s, phase two after the third
 *    Devastation with Widow's Kiss at ~47s then every 32s.
 *
 * Level model: the room is split by the web. Floor is at Z ~74, the web rim ring
 * (filament arrival, Spiderling Stalker markers) at Z ~110.5, and the web sags
 * funnel-like to Beth'tilac's perch at Z ~88.4 in the middle. Everything at
 * Z >= 87 counts as "on the web"; the same-level spell filters below and all
 * target selection use this single threshold.
 */

#include "ScriptMgr.h"
#include "firelands.h"
#include "Containers.h"
#include "InstanceScript.h"
#include "Map.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "PassiveAI.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "SpellAuraEffects.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "TemporarySummon.h"
#include "ThreatManager.h"
#include "Vehicle.h"

namespace Firelands::Bethtilac
{
enum Spells
{
    // Beth'tilac
    SPELL_VENOM_RAIN                = 99333,  // Ground raid damage while nobody shares her level. Difficulty chained.
    SPELL_EMBER_FLARE               = 98934,  // Web-level pulse during phase one. Difficulty chained.
    SPELL_EMBER_FLARE_GROUND        = 99859,  // Ground-level pulse during phase two. Difficulty chained.
    SPELL_METEOR_BURN               = 99076,  // Cast by the Web Rip controller: summons NPC 53450 + impact damage at dest
    SPELL_METEOR_FLAME              = 99039,  // Persistent flame left on the web at the impact point
    SPELL_SMOLDERING_DEVASTATION    = 99052,  // 8s cast, web level only; eff2 triggers her energize (99193)
    SPELL_CONSUME_BOSS              = 99857,  // Eats a Spiderling: eff2 = 10% max HP heal on caster (DBC native)
    SPELL_FRENZY                    = 99497,  // Phase two: +5% damage, stacking
    SPELL_THE_WIDOWS_KISS           = 99476,  // Periodic (2s) on the tank; each tick applies 99506 (scripted below)
    SPELL_WIDOWS_KISS_DEBUFF        = 99506,  // -10% healing received per stack + fire damage to allies around victim
    SPELL_ZERO_POWER_REGEN          = 72242,  // Freeze natural energy regen; the script drives the bar

    // Cinderweb Spinner
    SPELL_BURNING_ACID              = 98471,  // Also used by the drone. Difficulty chained.
    SPELL_FIERY_WEB_SPIN            = 97202,  // HEROIC: channeled stun on a ground player while hanging
    SPELL_WEB_CHANNEL_VISUAL        = 84283,  // Web strand beam up to the ceiling while hanging

    // Cinderweb Drone
    SPELL_BOILING_SPATTER           = 99463,  // Frontal cone. Difficulty chained.
    SPELL_CONSUME_DRONE             = 99304,  // Eats a Spiderling (heal handled in script, see ConsumeSpiderling)
    SPELL_LEECH_VENOM               = 99411,  // On Beth'tilac while an empty drone is attached: doubles her drain
    SPELL_FIXATE_DRONE              = 99526,  // HEROIC: threat lock (aura script below)
    SPELL_FIXATE_DRONE_SELF         = 99559,  // Companion self-aura (eff0 of 99526 is an unhandled script effect)

    // Cinderweb Spiderling
    SPELL_SEEPING_VENOM             = 97079,  // Leap onto a player within reach + fire DoT (99130 is the finder dummy)

    // Engorged Broodling (heroic)
    SPELL_BROODLING_FIXATE          = 100011, // Triggers 100014 (threat lock, aura script below)
    SPELL_VOLATILE_BURST            = 99990,  // Explosion; eff2 = native self-instakill. Difficulty chained.
    SPELL_VOLATILE_POISON           = 99276,  // Puddle aura, carried by an Invisible Man (54295): ticks 99278 (damage + 50% slow)

    // Spiderweb Filament
    SPELL_FILAMENT_VISUAL           = 97182,  // Web strand visual on the filament
};

enum Events
{
    // Beth'tilac
    EVENT_ENERGY_TICK = 1,
    EVENT_CHECK_WEB,
    EVENT_EMBER_FLARE,
    EVENT_METEOR_BURN,
    EVENT_DEVASTATION_FINISHED,
    EVENT_SPAWN_SPINNERS,
    EVENT_SPAWN_SPIDERLINGS,
    EVENT_SPAWN_DRONE,
    EVENT_SPAWN_BROODLINGS,
    EVENT_FRENZY,
    EVENT_WIDOWS_KISS,
    EVENT_CONSUME_CHECK,
    EVENT_CHECK_EVADE,

    // Cinderweb Spinner
    EVENT_SPINNER_BURNING_ACID,
    EVENT_FIERY_WEB_SPIN,

    // Cinderweb Drone
    EVENT_DRONE_ENERGY_TICK,
    EVENT_BOILING_SPATTER,
    EVENT_DRONE_BURNING_ACID,
    EVENT_DRONE_FIXATE,
    EVENT_DRONE_CONSUME_CHECK,

    // Cinderweb Spiderling
    EVENT_SPIDERLING_MOVE,
    EVENT_SPIDERLING_VENOM,

    // Engorged Broodling
    EVENT_BROODLING_CHECK_CONTACT,

    // Spiderweb Filament
    EVENT_FILAMENT_RISE,
};

enum Phases
{
    PHASE_NONE = 0,
    PHASE_ASCEND,
    PHASE_WEB,
    PHASE_DESCEND,
    PHASE_GROUND,
};

enum Points
{
    POINT_WEB = 1,
    POINT_GROUND,
    POINT_DRONE_ATTACH,
    POINT_FILAMENT_TOP,
    POINT_SPINNER_FALL,
};

enum Texts
{
    EMOTE_DEVASTATION   = 0, // "%s's smoldering body begins to flicker and combust!"
    EMOTE_SPIDERLINGS   = 1, // "Spiderlings have been roused from their nest!"
};

enum Misc
{
    MAX_FIRE_ENERGY         = 100,
    MAX_DRONE_ENERGY        = 90,
    DEVASTATION_COUNT       = 3,
    SPIDERLINGS_PER_CAVE    = 8,
};

// The bar empties in 82 seconds (100 ticks x 820ms), matching DBM's first
// Devastation at 82s and the 90s cycle (8s cast + 82s drain).
Milliseconds const EnergyTickPeriod = 820ms;

float const WEB_THRESHOLD_Z = 87.0f;   // ground tops out at ~86.2 (cave ledges), her perch is 88.4
float const WEB_RIM_Z       = 110.5f;  // filament arrival height (Spiderling Stalker ring)

Position const BethtilacPerchPos  = { 63.7014f, 387.3229f, 88.4215f, 1.9324f };
Position const BethtilacGroundPos = { 58.1748f, 397.6250f, 74.1896f, 1.9324f };
Position const DroneSpawnPos      = { 41.7000f, 371.6900f, 75.0400f, 0.0f };

// Spinner dangle points: the web rim marker ring (Spiderling Stalker spawns at
// Z > 100), lowered so the spinners hang below the web in ranged reach.
Position const SpinnerHangPositions[] =
{
    { 43.1927f, 379.504f, 90.0f, 3.49066f },
    { 56.7691f, 368.255f, 90.0f, 3.49066f },
    { 73.2760f, 372.050f, 90.0f, 3.49066f },
    { 45.1719f, 406.601f, 90.0f, 3.49066f },
    { 61.0295f, 405.193f, 90.0f, 3.49066f },
};

// Spiderling cave mouths: the four ground-ledge Spiderling Stalker spawns.
// 10-player sizes activate one cave per wave (rotating), 25-player all of them.
Position const CavePositions[] =
{
    {  23.1198f, 296.748f, 82.7261f, 0.855211f },
    { 134.8700f, 359.465f, 85.5022f, 3.50811f  },
    {  97.9861f, 451.188f, 86.2171f, 3.73500f  },
    {   4.66146f, 484.189f, 79.1088f, 5.65487f },
};

uint32 const NPC_VOLATILE_POISON_TRIGGER = 54295; // "Invisible Man", carries the puddle aura

inline bool IsOnWeb(WorldObject const* who)
{
    return who->GetPositionZ() >= WEB_THRESHOLD_Z;
}

// If the filament vehicle ride misbehaves (VehicleId 1711 seat data), flip this
// to fall back to an instant teleport onto the web rim on spellclick.
constexpr bool FilamentTeleportFallback = false;

struct boss_bethtilac : public BossAI
{
    boss_bethtilac(Creature* creature) : BossAI(creature, DATA_BETHTILAC),
        _phase(PHASE_NONE), _devastationCount(0), _caveIndex(0)
    {
        me->setActive(true);
    }

    void InitializeAI() override
    {
        BossAI::InitializeAI();
        me->SetPowerType(POWER_ENERGY);
        me->SetMaxPower(POWER_ENERGY, MAX_FIRE_ENERGY);
        me->SetPower(POWER_ENERGY, MAX_FIRE_ENERGY);
    }

    void Reset() override
    {
        _Reset();
        _phase = PHASE_NONE;
        _devastationCount = 0;
        _caveIndex = 0;
        me->SetReactState(REACT_AGGRESSIVE);
        me->SetDisableGravity(false);
        me->SetPower(POWER_ENERGY, MAX_FIRE_ENERGY);
    }

    void JustEngagedWith(Unit* who) override
    {
        BossAI::JustEngagedWith(who);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

        me->SetPower(POWER_ENERGY, MAX_FIRE_ENERGY);
        DoCastSelf(SPELL_ZERO_POWER_REGEN, true);

        // Climb to the web; the phase-one machinery starts on arrival.
        _phase = PHASE_ASCEND;
        me->AttackStop();
        me->SetReactState(REACT_PASSIVE);
        me->SetDisableGravity(true);
        me->GetMotionMaster()->MoveTakeoff(POINT_WEB, BethtilacPerchPos, 8.0f);

        events.ScheduleEvent(EVENT_SPAWN_SPINNERS, 12s);
        events.ScheduleEvent(EVENT_SPAWN_SPIDERLINGS, 12500ms);
        events.ScheduleEvent(EVENT_SPAWN_DRONE, 45s);
        if (IsHeroic())
            events.ScheduleEvent(EVENT_SPAWN_BROODLINGS, 30s);
        events.ScheduleEvent(EVENT_CHECK_EVADE, 5s);
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
    }

    void EnterEvadeMode(EvadeReason /*why*/) override
    {
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        summons.DespawnAll();
        _DespawnAtEvade();
    }

    void JustSummoned(Creature* summon) override
    {
        summons.Summon(summon);
        switch (summon->GetEntry())
        {
            case NPC_CINDERWEB_SPINNER:
            case NPC_CINDERWEB_DRONE:
            case NPC_CINDERWEB_SPIDERLING:
            case NPC_ENGORGED_BROODLING:
                DoZoneInCombat(summon);
                break;
            default: // filaments, web rips and other triggers stay out of combat
                break;
        }
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != POINT_MOTION_TYPE && type != EFFECT_MOTION_TYPE)
            return;

        switch (pointId)
        {
            case POINT_WEB:
                StartWebPhase();
                break;
            case POINT_GROUND:
                StartGroundPhase();
                break;
            default:
                break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (_phase == PHASE_GROUND)
        {
            if (!UpdateVictim())
                return;
        }
        else if (!me->IsInCombat())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_CHECK_EVADE:
                    if (!AnyPlayerAlive())
                    {
                        EnterEvadeMode(EVADE_REASON_NO_HOSTILES);
                        return;
                    }
                    events.ScheduleEvent(EVENT_CHECK_EVADE, 5s);
                    break;

                // ------------------------- Phase one: on the web
                case EVENT_ENERGY_TICK:
                    HandleEnergyTick();
                    break;
                case EVENT_CHECK_WEB:
                    HandleWebPresenceCheck();
                    events.ScheduleEvent(EVENT_CHECK_WEB, 2500ms);
                    break;
                case EVENT_EMBER_FLARE:
                    DoCastAOE(_phase == PHASE_GROUND ? SPELL_EMBER_FLARE_GROUND : SPELL_EMBER_FLARE);
                    events.ScheduleEvent(EVENT_EMBER_FLARE, 6s);
                    break;
                case EVENT_METEOR_BURN:
                    CastMeteorBurn();
                    events.ScheduleEvent(EVENT_METEOR_BURN, 15s);
                    break;
                case EVENT_DEVASTATION_FINISHED:
                    HandleDevastationFinished();
                    break;

                // ------------------------- Add spawners
                case EVENT_SPAWN_SPINNERS:
                    SpawnSpinnerWave();
                    events.ScheduleEvent(EVENT_SPAWN_SPINNERS, 15s);
                    break;
                case EVENT_SPAWN_SPIDERLINGS:
                    SpawnSpiderlingWave();
                    events.ScheduleEvent(EVENT_SPAWN_SPIDERLINGS, 30s);
                    break;
                case EVENT_SPAWN_DRONE:
                    me->SummonCreature(NPC_CINDERWEB_DRONE, DroneSpawnPos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000);
                    events.ScheduleEvent(EVENT_SPAWN_DRONE, 60s);
                    break;
                case EVENT_SPAWN_BROODLINGS:
                    SpawnBroodlings();
                    events.ScheduleEvent(EVENT_SPAWN_BROODLINGS, 25s);
                    break;

                // ------------------------- Phase two: the Frenzy
                case EVENT_FRENZY:
                    DoCastSelf(SPELL_FRENZY, true);
                    events.ScheduleEvent(EVENT_FRENZY, 5s);
                    break;
                case EVENT_WIDOWS_KISS:
                    DoCastVictim(SPELL_THE_WIDOWS_KISS);
                    events.ScheduleEvent(EVENT_WIDOWS_KISS, 32s);
                    break;
                case EVENT_CONSUME_CHECK:
                    ConsumeNearbySpiderlings();
                    events.ScheduleEvent(EVENT_CONSUME_CHECK, 1s);
                    break;
                default:
                    break;
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
        }

        if (_phase == PHASE_GROUND)
            DoMeleeAttackIfReady();
        else if (_phase == PHASE_WEB)
        {
            // Up top she only swings at whoever shares the web with her.
            if (Unit* victim = SelectWebVictim())
            {
                if (me->GetVictim() != victim)
                    me->Attack(victim, true);
                DoMeleeAttackIfReady();
            }
            else if (me->GetVictim())
                me->AttackStop();
        }
    }

private:
    uint8 _phase;
    uint8 _devastationCount;
    uint8 _caveIndex;

    bool AnyPlayerAlive() const
    {
        for (auto const& ref : me->GetMap()->GetPlayers())
            if (Player* player = ref.GetSource())
                if (player->IsAlive() && !player->IsGameMaster())
                    return true;
        return false;
    }

    bool AnyPlayerOnWeb() const
    {
        for (auto const& ref : me->GetMap()->GetPlayers())
            if (Player* player = ref.GetSource())
                if (player->IsAlive() && !player->IsGameMaster() && IsOnWeb(player))
                    return true;
        return false;
    }

    Unit* SelectWebVictim() const
    {
        Unit* best = nullptr;
        float bestThreat = -1.0f;
        for (auto const& ref : me->GetMap()->GetPlayers())
        {
            Player* player = ref.GetSource();
            if (!player || !player->IsAlive() || player->IsGameMaster() || !IsOnWeb(player))
                continue;
            float threat = me->GetThreatManager().GetThreat(player);
            if (threat > bestThreat)
            {
                bestThreat = threat;
                best = player;
            }
        }
        return best;
    }

    void StartWebPhase()
    {
        _phase = PHASE_WEB;
        me->SetDisableGravity(false);
        // Ground threat persists, but she never chases it; see UpdateAI.
        events.ScheduleEvent(EVENT_ENERGY_TICK, EnergyTickPeriod);
        events.ScheduleEvent(EVENT_CHECK_WEB, 2500ms);
        events.ScheduleEvent(EVENT_METEOR_BURN, 15s);
    }

    void HandleEnergyTick()
    {
        // An attached drone leeches her: the next Devastation comes twice as fast.
        int32 drain = me->HasAura(SPELL_LEECH_VENOM) ? 2 : 1;
        int32 energy = std::max<int32>(me->GetPower(POWER_ENERGY) - drain, 0);
        me->SetPower(POWER_ENERGY, energy);

        if (energy > 0)
        {
            events.ScheduleEvent(EVENT_ENERGY_TICK, EnergyTickPeriod);
            return;
        }

        // Stop the per-level loops for the 8s cast; the cast itself blocks the pump.
        events.CancelEvent(EVENT_EMBER_FLARE);
        events.CancelEvent(EVENT_METEOR_BURN);
        events.CancelEvent(EVENT_CHECK_WEB);

        Talk(EMOTE_DEVASTATION);
        DoCastAOE(SPELL_SMOLDERING_DEVASTATION);
        events.ScheduleEvent(EVENT_DEVASTATION_FINISHED, 8500ms);
    }

    void HandleDevastationFinished()
    {
        if (++_devastationCount >= DEVASTATION_COUNT)
        {
            StartDescend();
            return;
        }

        me->SetPower(POWER_ENERGY, MAX_FIRE_ENERGY);
        events.ScheduleEvent(EVENT_ENERGY_TICK, EnergyTickPeriod);
        events.ScheduleEvent(EVENT_CHECK_WEB, 1s);
        events.ScheduleEvent(EVENT_METEOR_BURN, 20s);
        events.RescheduleEvent(EVENT_SPAWN_SPINNERS, 12s); // fresh filaments for the next climb
    }

    void HandleWebPresenceCheck()
    {
        if (_phase != PHASE_WEB)
            return;

        if (AnyPlayerOnWeb())
        {
            if (events.GetTimeUntilEvent(EVENT_EMBER_FLARE) == std::numeric_limits<uint32>::max())
                events.ScheduleEvent(EVENT_EMBER_FLARE, 1s);
        }
        else
        {
            events.CancelEvent(EVENT_EMBER_FLARE);
            // Nobody up here with her: soak the ground instead.
            DoCastAOE(SPELL_VENOM_RAIN);
        }
    }

    void CastMeteorBurn()
    {
        if (_phase != PHASE_WEB)
            return;

        std::vector<Player*> webPlayers;
        for (auto const& ref : me->GetMap()->GetPlayers())
            if (Player* player = ref.GetSource())
                if (player->IsAlive() && !player->IsGameMaster() && IsOnWeb(player))
                    webPlayers.push_back(player);

        if (webPlayers.empty())
            return;

        Position dest = Trinity::Containers::SelectRandomContainerElement(webPlayers)->GetPosition();
        // The Web Rip controller casts Meteor Burn (summon + damage) at its feet;
        // she marks the burning hole in the web herself.
        me->SummonCreature(NPC_WEB_RIP, dest, TEMPSUMMON_TIMED_DESPAWN, 30000);
        me->CastSpell(dest, SPELL_METEOR_FLAME, true);
    }

    void StartDescend()
    {
        _phase = PHASE_DESCEND;
        events.CancelEvent(EVENT_ENERGY_TICK);
        events.CancelEvent(EVENT_CHECK_WEB);
        events.CancelEvent(EVENT_EMBER_FLARE);
        events.CancelEvent(EVENT_METEOR_BURN);
        events.CancelEvent(EVENT_SPAWN_SPINNERS);
        events.CancelEvent(EVENT_SPAWN_SPIDERLINGS);
        events.CancelEvent(EVENT_SPAWN_DRONE);
        events.CancelEvent(EVENT_SPAWN_BROODLINGS);

        // Leftover adds are not despawned: stray spiderlings become her snacks below.
        me->AttackStop();
        me->SetReactState(REACT_PASSIVE);
        me->SetDisableGravity(true);
        me->GetMotionMaster()->MoveLand(POINT_GROUND, BethtilacGroundPos, 8.0f);
    }

    void StartGroundPhase()
    {
        _phase = PHASE_GROUND;
        me->SetDisableGravity(false);
        me->GetThreatManager().ClearAllThreat(); // fresh start for the ground tanks
        me->SetReactState(REACT_AGGRESSIVE);
        DoZoneInCombat();

        events.ScheduleEvent(EVENT_FRENZY, 5s);
        events.ScheduleEvent(EVENT_EMBER_FLARE, 6s);
        events.ScheduleEvent(EVENT_WIDOWS_KISS, 47s);
        events.ScheduleEvent(EVENT_CONSUME_CHECK, 1s);
    }

    void ConsumeNearbySpiderlings()
    {
        std::list<Creature*> spiderlings;
        me->GetCreatureListWithEntryInGrid(spiderlings, NPC_CINDERWEB_SPIDERLING, 4.0f);
        for (Creature* spiderling : spiderlings)
        {
            if (!spiderling->IsAlive())
                continue;
            DoCast(spiderling, SPELL_CONSUME_BOSS, true); // 10% max HP heal via DBC eff2
            spiderling->DespawnOrUnsummon(500ms);
        }
    }

    void SpawnSpinnerWave()
    {
        uint8 const cap = RAID_MODE<uint8>(2, 5, 2, 5);

        std::list<Creature*> alive;
        me->GetCreatureListWithEntryInGrid(alive, NPC_CINDERWEB_SPINNER, 200.0f);
        alive.remove_if([](Creature* spinner) { return !spinner->IsAlive(); });
        if (alive.size() >= cap)
            return;

        uint8 toSpawn = cap - uint8(alive.size());
        for (uint8 i = 0; i < toSpawn; ++i)
        {
            Position const& pos = SpinnerHangPositions[urand(0, std::size(SpinnerHangPositions) - 1)];
            me->SummonCreature(NPC_CINDERWEB_SPINNER, pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000);
        }
    }

    void SpawnSpiderlingWave()
    {
        Talk(EMOTE_SPIDERLINGS);
        if (Is25ManRaid())
        {
            for (Position const& cave : CavePositions)
                SpawnSpiderlingsAt(cave);
        }
        else
        {
            SpawnSpiderlingsAt(CavePositions[_caveIndex]);
            _caveIndex = (_caveIndex + 1) % std::size(CavePositions);
        }
    }

    void SpawnSpiderlingsAt(Position const& cave)
    {
        for (uint8 i = 0; i < SPIDERLINGS_PER_CAVE; ++i)
        {
            Position pos = me->GetRandomPoint(cave, 4.0f);
            me->SummonCreature(NPC_CINDERWEB_SPIDERLING, pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
        }
    }

    void SpawnBroodlings()
    {
        if (Is25ManRaid())
        {
            for (Position const& cave : CavePositions)
                me->SummonCreature(NPC_ENGORGED_BROODLING, cave, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
        }
        else
            me->SummonCreature(NPC_ENGORGED_BROODLING, CavePositions[_caveIndex], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
    }
};

// Hangs below the web; drops (and frees its filament) when taunted or slain.
struct npc_cinderweb_spinner : public ScriptedAI
{
    npc_cinderweb_spinner(Creature* creature) : ScriptedAI(creature),
        _hanging(true), _filamentSpawned(false)
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetDisableGravity(true);
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        _events.ScheduleEvent(EVENT_SPINNER_BURNING_ACID, 3s, 8s);
        if (IsHeroic())
            _events.ScheduleEvent(EVENT_FIERY_WEB_SPIN, 12s, 20s);
    }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        DoCastSelf(SPELL_WEB_CHANNEL_VISUAL, true);
    }

    void SpellHit(WorldObject* caster, SpellInfo const* spellInfo) override
    {
        // Any taunt effect yanks the spinner off its thread.
        if (_hanging && (spellInfo->HasEffect(SPELL_EFFECT_ATTACK_ME) || spellInfo->HasAura(SPELL_AURA_MOD_TAUNT)))
            Drop(caster ? caster->ToUnit() : nullptr);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (_hanging)
            LeaveFilament(); // killed on the thread still frees the elevator
    }

    void UpdateAI(uint32 diff) override
    {
        if (!_hanging && !UpdateVictim())
            return;

        _events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_SPINNER_BURNING_ACID:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 60.0f, true))
                        DoCast(target, SPELL_BURNING_ACID);
                    _events.ScheduleEvent(EVENT_SPINNER_BURNING_ACID, 5s, 9s);
                    break;
                case EVENT_FIERY_WEB_SPIN: // heroic only, while hanging
                    if (_hanging)
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, [](Unit* target)
                            { return target->GetTypeId() == TYPEID_PLAYER && !IsOnWeb(target); }))
                            DoCast(target, SPELL_FIERY_WEB_SPIN);
                    _events.ScheduleEvent(EVENT_FIERY_WEB_SPIN, 20s, 30s);
                    break;
                default:
                    break;
            }
        }

        if (!_hanging)
            DoMeleeAttackIfReady();
    }

private:
    EventMap _events;
    bool _hanging;
    bool _filamentSpawned;

    void Drop(Unit* attacker)
    {
        _hanging = false;
        me->InterruptNonMeleeSpells(false); // ends Fiery Web Spin and the web beam
        _events.CancelEvent(EVENT_FIERY_WEB_SPIN);
        LeaveFilament();

        me->SetDisableGravity(false);
        me->GetMotionMaster()->MoveFall(POINT_SPINNER_FALL);
        me->SetReactState(REACT_AGGRESSIVE);
        if (attacker)
            AttackStart(attacker);
    }

    void LeaveFilament()
    {
        if (_filamentSpawned)
            return;
        _filamentSpawned = true;

        // The filament reaches the floor so the ground team can click it.
        Position pos = me->GetPosition();
        pos.m_positionZ = me->GetMap()->GetHeight(me->GetPhaseShift(), pos);
        me->SummonCreature(NPC_WEB_FILAMENT, pos, TEMPSUMMON_TIMED_DESPAWN, 60000);
    }
};

// Single-seat elevator: spellclick (98297) boards the player, the filament rises
// to the web rim and ejects them.
struct npc_spiderweb_filament : public ScriptedAI
{
    npc_spiderweb_filament(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetDisableGravity(true);
    }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        DoCastSelf(SPELL_FILAMENT_VISUAL, true);
    }

    void OnSpellClick(Unit* clicker, bool& /*result*/) override
    {
        if (!FilamentTeleportFallback)
            return;

        if (Player* player = clicker->ToPlayer())
        {
            player->NearTeleportTo(me->GetPositionX(), me->GetPositionY(), WEB_RIM_Z + 1.0f, player->GetOrientation());
            me->DespawnOrUnsummon(1s);
        }
    }

    void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
    {
        if (!apply || passenger->GetTypeId() != TYPEID_PLAYER)
            return;

        _events.ScheduleEvent(EVENT_FILAMENT_RISE, 500ms);
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != POINT_MOTION_TYPE || pointId != POINT_FILAMENT_TOP)
            return;

        if (Vehicle* vehicle = me->GetVehicleKit())
            vehicle->RemoveAllPassengers(); // drops the rider onto the web
        me->DespawnOrUnsummon(2s);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            if (eventId == EVENT_FILAMENT_RISE)
            {
                Position dest = me->GetPosition();
                dest.m_positionZ = WEB_RIM_Z;
                me->GetMotionMaster()->MovePoint(POINT_FILAMENT_TOP, dest, false);
            }
        }
    }

private:
    EventMap _events;
};

// Ground bruiser with its own energy bar; at zero it climbs the web and leeches
// the boss until killed.
struct npc_cinderweb_drone : public ScriptedAI
{
    npc_cinderweb_drone(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript()), _ascending(false)
    {
        me->SetPowerType(POWER_ENERGY);
        me->SetMaxPower(POWER_ENERGY, MAX_DRONE_ENERGY);
        me->SetPower(POWER_ENERGY, MAX_DRONE_ENERGY);
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        DoCastSelf(SPELL_ZERO_POWER_REGEN, true);
        _events.ScheduleEvent(EVENT_DRONE_ENERGY_TICK, 1s);
        _events.ScheduleEvent(EVENT_BOILING_SPATTER, 8s, 12s);
        _events.ScheduleEvent(EVENT_DRONE_BURNING_ACID, 6s, 10s);
        _events.ScheduleEvent(EVENT_DRONE_CONSUME_CHECK, 1s);
        if (IsHeroic())
            _events.ScheduleEvent(EVENT_DRONE_FIXATE, 15s, 25s);
    }

    void JustDied(Unit* /*killer*/) override
    {
        // Detach: her drain rate returns to normal.
        if (_instance)
            if (Creature* bethtilac = _instance->GetCreature(DATA_BETHTILAC))
                bethtilac->RemoveAurasDueToSpell(SPELL_LEECH_VENOM, me->GetGUID());
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if ((type != POINT_MOTION_TYPE && type != EFFECT_MOTION_TYPE) || pointId != POINT_DRONE_ATTACH)
            return;

        if (_instance)
            if (Creature* bethtilac = _instance->GetCreature(DATA_BETHTILAC))
                DoCast(bethtilac, SPELL_LEECH_VENOM, true);
    }

    void UpdateAI(uint32 diff) override
    {
        if (_ascending)
            return;

        if (!UpdateVictim())
            return;

        _events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_DRONE_ENERGY_TICK:
                {
                    int32 energy = std::max<int32>(me->GetPower(POWER_ENERGY) - 1, 0);
                    me->SetPower(POWER_ENERGY, energy);
                    if (energy <= 0)
                    {
                        StartAscend();
                        return;
                    }
                    _events.ScheduleEvent(EVENT_DRONE_ENERGY_TICK, 1s);
                    break;
                }
                case EVENT_BOILING_SPATTER:
                    DoCastVictim(SPELL_BOILING_SPATTER); // frontal cone - tanks face it away
                    _events.ScheduleEvent(EVENT_BOILING_SPATTER, 10s, 15s);
                    break;
                case EVENT_DRONE_BURNING_ACID:
                    // Used against players out of its reach.
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, [this](Unit* target)
                        { return target->GetTypeId() == TYPEID_PLAYER && !me->IsWithinMeleeRange(target); }))
                        DoCast(target, SPELL_BURNING_ACID);
                    _events.ScheduleEvent(EVENT_DRONE_BURNING_ACID, 6s, 10s);
                    break;
                case EVENT_DRONE_CONSUME_CHECK:
                    ConsumeSpiderling();
                    _events.ScheduleEvent(EVENT_DRONE_CONSUME_CHECK, 1s);
                    break;
                case EVENT_DRONE_FIXATE: // heroic only
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, [](Unit* target)
                        { return target->GetTypeId() == TYPEID_PLAYER && !IsOnWeb(target); }))
                    {
                        DoCastSelf(SPELL_FIXATE_DRONE_SELF, true);
                        DoCast(target, SPELL_FIXATE_DRONE, true); // threat lock via aura script
                    }
                    _events.ScheduleEvent(EVENT_DRONE_FIXATE, 25s, 35s);
                    break;
                default:
                    break;
            }
        }

        DoMeleeAttackIfReady();
    }

private:
    EventMap _events;
    InstanceScript* _instance;
    bool _ascending;

    void ConsumeSpiderling()
    {
        std::list<Creature*> spiderlings;
        me->GetCreatureListWithEntryInGrid(spiderlings, NPC_CINDERWEB_SPIDERLING, 4.0f);
        for (Creature* spiderling : spiderlings)
        {
            if (!spiderling->IsAlive())
                continue;
            DoCast(spiderling, SPELL_CONSUME_DRONE, true);
            // 100634 (triggered) carries the buff side; the 20% heal is guaranteed here.
            me->ModifyHealth(int64(me->CountPctFromMaxHealth(20)));
            spiderling->DespawnOrUnsummon(500ms);
        }
    }

    void StartAscend()
    {
        _ascending = true;
        _events.Reset();
        me->AttackStop();
        me->SetReactState(REACT_PASSIVE);
        me->SetDisableGravity(true);

        Position dest = BethtilacPerchPos;
        me->GetMotionMaster()->MoveTakeoff(POINT_DRONE_ATTACH, dest, 5.0f);
    }
};

// Swarm lemming: runs to the nearest drone (or to Beth'tilac once she is on the
// ground) to be eaten; leaps venomously onto players that stray too close.
struct npc_cinderweb_spiderling : public ScriptedAI
{
    npc_cinderweb_spiderling(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript())
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        _events.ScheduleEvent(EVENT_SPIDERLING_MOVE, 1s);
        _events.ScheduleEvent(EVENT_SPIDERLING_VENOM, 2s);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_SPIDERLING_MOVE:
                {
                    Creature* goal = FindConsumer();
                    if (goal && _goalGUID != goal->GetGUID())
                    {
                        _goalGUID = goal->GetGUID();
                        me->GetMotionMaster()->MoveFollow(goal, 0.5f, 0.0f);
                    }
                    else if (!goal && !_goalGUID.IsEmpty())
                    {
                        _goalGUID.Clear();
                        me->GetMotionMaster()->MoveRandom(8.0f);
                    }
                    _events.ScheduleEvent(EVENT_SPIDERLING_MOVE, 1500ms);
                    break;
                }
                case EVENT_SPIDERLING_VENOM:
                {
                    std::list<Player*> nearby;
                    me->GetPlayerListInGrid(nearby, 3.0f);
                    nearby.remove_if([](Player* player) { return !player->IsAlive() || player->IsGameMaster(); });
                    if (!nearby.empty())
                        DoCast(Trinity::Containers::SelectRandomContainerElement(nearby), SPELL_SEEPING_VENOM, true);
                    _events.ScheduleEvent(EVENT_SPIDERLING_VENOM, 4s);
                    break;
                }
                default:
                    break;
            }
        }
    }

private:
    EventMap _events;
    InstanceScript* _instance;
    ObjectGuid _goalGUID;

    Creature* FindConsumer() const
    {
        if (Creature* drone = me->FindNearestCreature(NPC_CINDERWEB_DRONE, 200.0f, true))
            return drone;

        if (_instance)
            if (Creature* bethtilac = _instance->GetCreature(DATA_BETHTILAC))
                if (bethtilac->IsAlive() && !IsOnWeb(bethtilac))
                    return bethtilac;

        return nullptr;
    }
};

// Heroic kamikaze: fixates a random ground player and detonates on contact with
// anyone, leaving a Volatile Poison puddle. Killing it at range prevents the blast.
struct npc_engorged_broodling : public ScriptedAI
{
    npc_engorged_broodling(Creature* creature) : ScriptedAI(creature), _exploded(false)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        AcquireTarget();
        _events.ScheduleEvent(EVENT_BROODLING_CHECK_CONTACT, 500ms);
    }

    void UpdateAI(uint32 diff) override
    {
        if (_exploded)
            return;

        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            if (eventId != EVENT_BROODLING_CHECK_CONTACT)
                continue;

            std::list<Player*> nearby;
            me->GetPlayerListInGrid(nearby, 3.0f);
            nearby.remove_if([](Player* player) { return !player->IsAlive() || player->IsGameMaster(); });
            if (!nearby.empty())
            {
                Explode();
                return;
            }

            // Re-acquire if the fixated player died or left the floor.
            Player* victim = ObjectAccessor::GetPlayer(*me, _victimGUID);
            if (!victim || !victim->IsAlive() || IsOnWeb(victim))
                AcquireTarget();

            _events.ScheduleEvent(EVENT_BROODLING_CHECK_CONTACT, 500ms);
        }
    }

private:
    EventMap _events;
    ObjectGuid _victimGUID;
    bool _exploded;

    void AcquireTarget()
    {
        std::vector<Player*> candidates;
        for (auto const& ref : me->GetMap()->GetPlayers())
            if (Player* player = ref.GetSource())
                if (player->IsAlive() && !player->IsGameMaster() && !IsOnWeb(player))
                    candidates.push_back(player);

        if (candidates.empty())
            return;

        Player* target = Trinity::Containers::SelectRandomContainerElement(candidates);
        _victimGUID = target->GetGUID();
        DoCast(target, SPELL_BROODLING_FIXATE, true); // triggers 100014 (threat lock)
        me->GetMotionMaster()->MoveFollow(target, 0.0f, 0.0f);
    }

    void Explode()
    {
        _exploded = true;
        // The puddle outlives the corpse: an invisible trigger carries the aura.
        if (Creature* trigger = me->SummonCreature(NPC_VOLATILE_POISON_TRIGGER, me->GetPosition(), TEMPSUMMON_TIMED_DESPAWN, 45000))
            trigger->CastSpell(trigger, SPELL_VOLATILE_POISON, true);
        DoCastAOE(SPELL_VOLATILE_BURST); // eff2 = self-instakill
    }
};

// Meteor Burn controller: summoned at the impact point, casts the meteor
// (summon + damage) at its feet and despawns with the burn.
struct npc_web_rip : public NullCreatureAI
{
    npc_web_rip(Creature* creature) : NullCreatureAI(creature) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        DoCastSelf(SPELL_METEOR_BURN, true);
    }
};

// 98934/99859 Ember Flare: hits only the level she is on.
class spell_bethtilac_ember_flare : public SpellScript
{
    void FilterTargets(std::list<WorldObject*>& targets)
    {
        bool casterOnWeb = IsOnWeb(GetCaster());
        targets.remove_if([casterOnWeb](WorldObject* target)
        {
            return IsOnWeb(target) != casterOnWeb;
        });
    }

    void Register() override
    {
        OnObjectAreaTargetSelect.Register(&spell_bethtilac_ember_flare::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
    }
};

// 99333 Venom Rain: cast from the web, soaks only the ground level.
class spell_bethtilac_venom_rain : public SpellScript
{
    void FilterTargets(std::list<WorldObject*>& targets)
    {
        targets.remove_if([](WorldObject* target)
        {
            return IsOnWeb(target);
        });
    }

    void Register() override
    {
        OnObjectAreaTargetSelect.Register(&spell_bethtilac_venom_rain::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
    }
};

// 99052 Smoldering Devastation: scorches only her own (web) level.
class spell_bethtilac_smoldering_devastation : public SpellScript
{
    void FilterTargets(std::list<WorldObject*>& targets)
    {
        targets.remove_if([](WorldObject* target)
        {
            return !IsOnWeb(target);
        });
    }

    void Register() override
    {
        OnObjectAreaTargetSelect.Register(&spell_bethtilac_smoldering_devastation::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
    }
};

// 99526 (drone) / 100014 (broodling) Fixate: hard threat lock for the aura's
// duration; the caster returns to its tank when the lock ends.
class spell_bethtilac_fixate : public AuraScript
{
    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Unit* caster = GetCaster())
            caster->GetThreatManager().AddThreat(GetTarget(), 50000000.0f, nullptr, true, true);
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Unit* caster = GetCaster())
            caster->GetThreatManager().AddThreat(GetTarget(), -50000000.0f, nullptr, true, true);
    }

    void Register() override
    {
        // Bound to both 99526 (dummy on EFFECT_1) and 100014 (dummy on EFFECT_0)
        AfterEffectApply.Register(&spell_bethtilac_fixate::OnApply, SpellEffIndex(EFFECT_FIRST_FOUND), SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove.Register(&spell_bethtilac_fixate::OnRemove, SpellEffIndex(EFFECT_FIRST_FOUND), SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 99476 The Widow's Kiss: the DBC periodic carries no trigger spell; each 2s
// tick stacks 99506 on the victim (-10% healing received per stack + fire
// damage to allies around them).
class spell_bethtilac_widows_kiss : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_WIDOWS_KISS_DEBUFF });
    }

    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        PreventDefaultAction(); // the DBC periodic has no trigger spell of its own
        GetTarget()->CastSpell(GetTarget(), SPELL_WIDOWS_KISS_DEBUFF, true);
    }

    void Register() override
    {
        OnEffectPeriodic.Register(&spell_bethtilac_widows_kiss::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

} // namespace Firelands::Bethtilac

void AddSC_boss_bethtilac()
{
    using namespace Firelands;
    using namespace Firelands::Bethtilac;
    RegisterFirelandsCreatureAI(boss_bethtilac);
    RegisterFirelandsCreatureAI(npc_cinderweb_spinner);
    RegisterFirelandsCreatureAI(npc_spiderweb_filament);
    RegisterFirelandsCreatureAI(npc_cinderweb_drone);
    RegisterFirelandsCreatureAI(npc_cinderweb_spiderling);
    RegisterFirelandsCreatureAI(npc_engorged_broodling);
    RegisterFirelandsCreatureAI(npc_web_rip);
    RegisterSpellScript(spell_bethtilac_ember_flare);
    RegisterSpellScript(spell_bethtilac_venom_rain);
    RegisterSpellScript(spell_bethtilac_smoldering_devastation);
    RegisterSpellScript(spell_bethtilac_fixate);
    RegisterSpellScript(spell_bethtilac_widows_kiss);
}
