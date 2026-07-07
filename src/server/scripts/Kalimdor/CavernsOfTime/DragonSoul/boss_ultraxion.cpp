/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
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

#include "dragon_soul.h"
#include "Containers.h"
#include "GameObject.h"
#include "InstanceScript.h"
#include "Map.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "PhasingHandler.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "TaskScheduler.h"
#include "TemporarySummon.h"

namespace DragonSoul::Ultraxion
{
enum Texts
{
    // Ultraxion (55294)
    SAY_INTRO                   = 0,  // "I am the beginning of the end..."
    SAY_AGGRO                   = 1,  // "For this moment ALONE was I made..."
    EMOTE_TWILIGHT_SHIFT        = 2,  // "A monstrous force pulls you into the twilight realm!"
    SAY_HOUR_OF_TWILIGHT        = 3,  // "Now is the hour of twilight!"
    SAY_FADING_LIGHT            = 4,  // "The final shred of light fades..."
    SAY_UNSTABLE_MONSTROSITY    = 5,  // two variants ("Lord Deathwing, your gift..." / "Through the pain and fire...")
    SAY_SLAY                    = 6,  // three variants
    SAY_DEATH                   = 7,
    SAY_TWILIGHT_ERUPTION       = 8,  // "I WILL DRAG YOU WITH ME INTO FLAME AND DARKNESS!"
    EMOTE_SHIELD_DESTROYED      = 9,  // unsoaked Hour of Twilight
    EMOTE_SHIELDS_SHATTER       = 10, // last warning before the failure eruption
    EMOTE_MORE_UNSTABLE         = 11, // Unstable Monstrosity speeds up
    EMOTE_ERUPTION_INCOMING     = 12, // "...instability is causing a massive [Twilight Eruption]!"
    EMOTE_MAXIMUM_INSTABILITY   = 13, // final Monstrosity step

    // Aspects (56630 Alexstrasza / 56665 Ysera / 56664 Kalecgos / 56666 Nozdormu / 56667 Thrall)
    SAY_ASPECT_GIFT             = 0,  // per-aspect flavor yell for their gift
    EMOTE_ASPECT_GIFT           = 1,  // per-aspect raid warning with the spell link

    // Ysera extras
    SAY_YSERA_GAUNTLET_START    = 2,  // "Heroes, we must place this burden on your shoulders once again..."
    SAY_YSERA_PULL_WARNING      = 3,  // "I sense a great disturbance in the balance approaching..."
    SAY_YSERA_WIPE              = 4,  // "I have awakened only to sleep once again."

    // Alexstrasza extras
    SAY_ALEXSTRASZA_BRING_DOWN  = 2,  // "They... are my clutch no longer. Bring them down."
    SAY_ALEXSTRASZA_WIPE        = 3,  // "They have failed us sister."

    // Deathwing (55971)
    SAY_DEATHWING_GAUNTLET_1    = 0,  // "It is good to see you again, Alexstrasza..."
    SAY_DEATHWING_GAUNTLET_2    = 1,  // "Twisting your pitiful whelps..."
    SAY_DEATHWING_GAUNTLET_END  = 2,  // "Mere whelps, experiments..."
    SAY_DEATHWING_ULTIMATE      = 3,  // "Nefarian, Onyxia, Sinestra... they were nothing..."
    SAY_DEATHWING_HOUR          = 4   // "The Hour of Twilight is nigh..."
};

enum Spells
{
    // Twilight realm and Heroic Will
    SPELL_TWILIGHT_SHIFT_AOE        = 106369, // force-casts 106368 on the raid
    SPELL_TWILIGHT_SHIFT            = 106368, // screen effect + phase 16 (corrected in SpellMgr)
    SPELL_HEROIC_WILL_GRANT         = 105554, // aura 293 -> OverrideSpellData 313 -> extra action button
    SPELL_HEROIC_WILL               = 106108, // the button: 5s pacify + leaves the twilight realm

    // Hour of Twilight
    SPELL_HOUR_OF_TWILIGHT          = 106371, // 5s cast; forks 109415/109416/109417
    SPELL_HOUR_OF_TWILIGHT_DAMAGE   = 103327, // triggered blast on twilight realm players
    SPELL_LOOMING_DARKNESS_MISSILE  = 109231, // triggered by 103327; LFR forgiveness path
    SPELL_LOOMING_DARKNESS          = 106498, // 120s LFR debuff

    // Fading Light
    SPELL_FADING_LIGHT              = 105925, // tank; forks 110070/110069/110068
    SPELL_FADING_LIGHT_RAID         = 109075, // heroic non-tanks; forks 110080/110079/110078
    SPELL_FADING_LIGHT_KILL         = 105926, // forks 110075/110074/110073

    // Unstable Monstrosity (all difficulties, 6s -> 1s over six minutes)
    SPELL_UNSTABLE_MONSTROSITY_6S   = 106372,
    SPELL_UNSTABLE_MONSTROSITY_5S   = 106376,
    SPELL_UNSTABLE_MONSTROSITY_4S   = 106377,
    SPELL_UNSTABLE_MONSTROSITY_3S   = 106378,
    SPELL_UNSTABLE_MONSTROSITY_2S   = 106379,
    SPELL_UNSTABLE_MONSTROSITY_1S   = 106380,
    SPELL_TWILIGHT_INSTABILITY      = 106375, // forks 109182/109183/109184

    // Enrage and punish
    SPELL_TWILIGHT_ERUPTION         = 106388, // 5s cast, raid-wide instakill
    SPELL_TWILIGHT_BURST            = 106415, // no-melee-victim punish

    // Aspect assistance
    SPELL_LAST_DEFENDER_OF_AZEROTH  = 106218, // Thrall; script effect fans out the per-class auras
    SPELL_LAST_DEFENDER_WARRIOR     = 106080,
    SPELL_LAST_DEFENDER_DRUID       = 106224,
    SPELL_LAST_DEFENDER_PALADIN     = 106226,
    SPELL_LAST_DEFENDER_DEATH_KNIGHT= 106227,
    SPELL_TIMELOOP                  = 105984, // cheat death
    SPELL_TIMELOOP_HEAL             = 105992,
    SPELL_GIFT_OF_LIFE              = 105896, // crystal buffs (goober Data10)
    SPELL_GIFT_OF_LIFE_HEROIC       = 109340,
    SPELL_ESSENCE_OF_DREAMS         = 105900,
    SPELL_ESSENCE_OF_DREAMS_HEROIC  = 109342,
    SPELL_ESSENCE_OF_DREAMS_HEAL    = 105996,
    SPELL_SOURCE_OF_MAGIC           = 105903,
    SPELL_SOURCE_OF_MAGIC_HEROIC    = 109346,

    // Gauntlet
    SPELL_WARD_OF_EARTH             = 108161, // Thrall
    SPELL_ASPECT_SHIELD_NOZDORMU    = 108160,
    SPELL_ASPECT_SHIELD_KALECGOS    = 108162,
    SPELL_ASPECT_SHIELD_ALEXSTRASZA = 108163,
    SPELL_ASPECT_SHIELD_YSERA       = 108164,
    SPELL_TWILIGHT_ESCAPE           = 109904, // drakes flee
    SPELL_TWILIGHT_FLAMES           = 105579, // drake strafing breath
    SPELL_TWILIGHT_BREATH_1         = 105555,
    SPELL_TWILIGHT_BREATH_2         = 105556
};

enum Events
{
    EVENT_HOUR_OF_TWILIGHT = 1,
    EVENT_MONSTROSITY_STEP,
    EVENT_CRYSTAL_GIFT_OF_LIFE,
    EVENT_CRYSTAL_ESSENCE_OF_DREAMS,
    EVENT_CRYSTAL_SOURCE_OF_MAGIC,
    EVENT_TIMELOOP,
    EVENT_TWILIGHT_ERUPTION,
    EVENT_MELEE_CHECK
};

enum Actions
{
    ACTION_START_INTRO           = 1, // gauntlet controller -> boss: fly-in RP
    ACTION_HOUR_NOT_SOAKED       = 2, // spell script -> boss: nobody took the blast
    ACTION_GAUNTLET_END          = 3, // controller -> drakes: escape
    ACTION_CRYSTAL_CLAIMED_BASE  = 10 // + crystal index, spell script -> boss
};

enum GuidDataIds
{
    GUID_HOUR_OF_TWILIGHT_HIT = 1 // spell script -> boss: achievement tracking
};

enum DataIds
{
    DATA_CRYSTAL_CLAIMED_BASE = 1 // + crystal index (0..2)
};

enum Points
{
    POINT_FLY_IN = 1,
    POINT_DRAKE_RING,
    POINT_DRAKE_STRAFE,
    POINT_DRAKE_ESCAPE
};

enum CrystalIndex : uint8
{
    CRYSTAL_GIFT_OF_LIFE     = 0,
    CRYSTAL_ESSENCE_OF_DREAMS = 1,
    CRYSTAL_SOURCE_OF_MAGIC  = 2
};

// Positions from the retail sniff; the fly-in is identical on all difficulties
Position const UltraxionSpawnPos  = { -1564.0f,  -2369.0f,  250.0833f, 3.281219f };
Position const UltraxionAnchorPos = { -1707.89f, -2384.9f,  353.84f,   3.247313f };
Position const PlatformCenter     = { -1786.0f,  -2393.0f,  341.44f,   0.0f };

// Crystals appear in front of their aspect (hand-placed; absent from sniffs)
Position const CrystalPositions[3] =
{
    { -1780.35f, -2398.61f, 341.44f, 2.1468f }, // Gift of Life - Alexstrasza
    { -1777.45f, -2392.35f, 341.44f, 3.2114f }, // Essence of Dreams - Ysera
    { -1789.87f, -2398.05f, 341.44f, 0.8552f }  // Source of Magic - Kalecgos
};

constexpr uint32 CrystalGameObjects[3] = { GO_GIFT_OF_LIFE, GO_ESSENCE_OF_DREAMS, GO_SOURCE_OF_MAGIC };
constexpr uint32 CrystalBuffs[3]       = { SPELL_GIFT_OF_LIFE, SPELL_ESSENCE_OF_DREAMS, SPELL_SOURCE_OF_MAGIC };
constexpr uint32 CrystalBuffsHeroic[3] = { SPELL_GIFT_OF_LIFE_HEROIC, SPELL_ESSENCE_OF_DREAMS_HEROIC, SPELL_SOURCE_OF_MAGIC_HEROIC };

uint32 const MonstrositySteps[6] =
{
    SPELL_UNSTABLE_MONSTROSITY_6S, SPELL_UNSTABLE_MONSTROSITY_5S, SPELL_UNSTABLE_MONSTROSITY_4S,
    SPELL_UNSTABLE_MONSTROSITY_3S, SPELL_UNSTABLE_MONSTROSITY_2S, SPELL_UNSTABLE_MONSTROSITY_1S
};

uint32 const FadingLightAll[8] = { 105925, 110070, 110069, 110068, 109075, 110080, 110079, 110078 };

// Gauntlet flight geometry (from the sniff flight envelope)
constexpr float DrakeRingRadius   = 70.0f;
constexpr float DrakeRingZ        = 352.0f;
constexpr float DrakeStrafeZ      = 346.0f;
constexpr float DrakeFlightSpeed  = 2.4f;   // speed rate, ~16.8 yd/s
constexpr uint32 GauntletFailsafeMs = 253000; // DBM event timer

struct UltraxionTuning
{
    uint8  FadingLightExtraTargets; // heroic: extra non-tank Fading Lights per application
    uint8  UnsoakedTolerance;       // unsoaked Hours of Twilight before the aspects fall
    uint8  DrakeKillTarget;         // gauntlet drake kills to end the pre-event
    uint8  DrakeMaxAirborne;        // simultaneous gauntlet drakes
};

bool IsHeroicUltraxion(Map const* map)
{
    return map->GetDifficulty() == RAID_DIFFICULTY_10MAN_HEROIC
        || map->GetDifficulty() == RAID_DIFFICULTY_25MAN_HEROIC;
}

bool IsLFR(InstanceScript const* instance)
{
    return instance && instance->IsLFR();
}

UltraxionTuning const& GetTuning(InstanceScript const* instance, Map const* map)
{
    static UltraxionTuning const lfr = { 0, 2, 15, 12 };
    static UltraxionTuning const n10 = { 0, 2, 15, 8  };
    static UltraxionTuning const n25 = { 0, 2, 15, 12 };
    static UltraxionTuning const h10 = { 1, 2, 15, 8  };
    static UltraxionTuning const h25 = { 3, 2, 15, 12 };

    if (IsLFR(instance))
        return lfr;

    switch (map->GetDifficulty())
    {
        case RAID_DIFFICULTY_25MAN_NORMAL: return n25;
        case RAID_DIFFICULTY_10MAN_HEROIC: return h10;
        case RAID_DIFFICULTY_25MAN_HEROIC: return h25;
        default:                           return n10;
    }
}

void ApplyLFRHealth(Creature* creature, InstanceScript const* instance, uint32 statsEntry)
{
    if (!IsLFR(instance))
        return;

    CreatureTemplate const* lfrStats = sObjectMgr->GetCreatureTemplate(statsEntry);
    if (!lfrStats)
        return;

    if (CreatureBaseStats const* baseStats = sObjectMgr->GetCreatureBaseStats(creature->getLevel(), lfrStats->unit_class))
    {
        creature->SetMaxHealth(baseStats->GenerateHealth(lfrStats));
        creature->SetFullHealth();
    }
}

void ApplyLFRDamageReduction(InstanceScript const* instance, uint32& damage)
{
    if (IsLFR(instance))
        damage = damage * LFR_DAMAGE_PCT / 100;
}

bool IsInTwilightRealm(Unit const* unit)
{
    return unit->HasAura(SPELL_TWILIGHT_SHIFT);
}

// Alive, non-GM players currently inside the twilight realm
std::vector<Player*> GetTwilightRealmPlayers(Map* map)
{
    std::vector<Player*> players;
    for (MapReference const& ref : map->GetPlayers())
    {
        Player* player = ref.GetSource();
        if (player && player->IsAlive() && !player->IsGameMaster() && IsInTwilightRealm(player))
            players.push_back(player);
    }
    return players;
}
}

namespace DragonSoul::Ultraxion
{
struct boss_ultraxion : public BossAI
{
    boss_ultraxion(Creature* creature) : BossAI(creature, DATA_ULTRAXION) { }

    void Reset() override
    {
        _Reset();
        _scheduler.CancelAll();
        _monstrosityStep = 0;
        _unsoakedHits = 0;
        _hourOfTwilightHits.clear();
        _crystalClaimed = { };
        _crystalGuids = { };

        me->SetDisableGravity(true);
        me->SetHover(true);
        me->SetControlled(true, UNIT_STATE_ROOT);

        ApplyLFRHealth(me, instance, NPC_ULTRAXION_LFR_STATS);
    }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        me->SetSpeedRate(MOVE_FLIGHT, 2.5f);
        Reset();
    }

    void DamageDealt(Unit* /*victim*/, uint32& damage, DamageEffectType damageType) override
    {
        // Direct spell hits carry their own Raid Finder cuts in the spell
        // scripts; this handles melee and periodic ticks
        if (damageType != SPELL_DIRECT_DAMAGE)
            ApplyLFRDamageReduction(instance, damage);
    }

    void DoAction(int32 action) override
    {
        if (action == ACTION_START_INTRO)
        {
            // Fresh from the gauntlet: taunt the raid, then fly to the platform
            me->SetControlled(false, UNIT_STATE_ROOT);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);

            _scheduler.Schedule(1s, [this](TaskContext /*context*/)
            {
                Talk(SAY_INTRO);
            });
            _scheduler.Schedule(12s, [this](TaskContext /*context*/)
            {
                Talk(SAY_AGGRO);
                me->GetMotionMaster()->MovePoint(POINT_FLY_IN, UltraxionAnchorPos, false);
            });
        }
        else if (action == ACTION_HOUR_NOT_SOAKED)
        {
            ++_unsoakedHits;
            if (_unsoakedHits > GetTuning(instance, me->GetMap()).UnsoakedTolerance)
            {
                // The aspects can take no more: the twilight consumes everything
                Talk(EMOTE_SHIELDS_SHATTER);
                Talk(SAY_TWILIGHT_ERUPTION);
                events.CancelEvent(EVENT_TWILIGHT_ERUPTION);
                me->InterruptNonMeleeSpells(false);
                DoCastAOE(SPELL_TWILIGHT_ERUPTION);
            }
            else
                Talk(EMOTE_SHIELD_DESTROYED);
        }
        else if (action >= ACTION_CRYSTAL_CLAIMED_BASE && action < ACTION_CRYSTAL_CLAIMED_BASE + 3)
        {
            uint8 index = uint8(action - ACTION_CRYSTAL_CLAIMED_BASE);
            _crystalClaimed[index] = true;
            DespawnCrystal(index);
        }
    }

    uint32 GetData(uint32 type) const override
    {
        if (type >= DATA_CRYSTAL_CLAIMED_BASE && type < DATA_CRYSTAL_CLAIMED_BASE + 3)
            return _crystalClaimed[type - DATA_CRYSTAL_CLAIMED_BASE] ? 1 : 0;
        return 0;
    }

    void SetGUID(ObjectGuid const& guid, int32 id) override
    {
        // Minutes to Midnight: fail once anyone eats a second Hour of Twilight
        if (id == GUID_HOUR_OF_TWILIGHT_HIT)
            if (++_hourOfTwilightHits[guid] >= 2)
                instance->SetData(DATA_ULTRAXION_ACHIEVEMENT_FAILED, 1);
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != POINT_MOTION_TYPE || pointId != POINT_FLY_IN)
            return;

        me->SetFacingTo(UltraxionAnchorPos.GetOrientation());
        me->SetHomePosition(UltraxionAnchorPos);
        me->SetControlled(true, UNIT_STATE_ROOT);
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
        me->SetReactState(REACT_AGGRESSIVE);
        DoZoneInCombat();
    }

    void JustEngagedWith(Unit* who) override
    {
        BossAI::JustEngagedWith(who);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 1);
        Talk(EMOTE_TWILIGHT_SHIFT);

        // Drag the raid into the twilight realm and grant the button
        DoCastAOE(SPELL_TWILIGHT_SHIFT_AOE, true);
        DoCastAOE(SPELL_HEROIC_WILL_GRANT, true);

        // The boss fights from inside the realm; the whole platform phases
        PhasingHandler::ResetPhaseShift(me);
        PhasingHandler::AddPhase(me, PHASE_TWILIGHT_REALM, true);

        // Thrall shields the last defenders of Azeroth
        if (Creature* thrall = instance->GetCreature(DATA_THRALL_ULTRAXION))
        {
            thrall->AI()->Talk(SAY_ASPECT_GIFT);
            thrall->AI()->Talk(EMOTE_ASPECT_GIFT);
            thrall->CastSpell(me, SPELL_LAST_DEFENDER_OF_AZEROTH, true);
        }

        // Unstable Monstrosity ticks on all difficulties, accelerating hourly
        DoCastSelf(MonstrositySteps[0], true);
        _monstrosityStep = 1;

        events.ScheduleEvent(EVENT_HOUR_OF_TWILIGHT, 45500ms);
        events.ScheduleEvent(EVENT_MONSTROSITY_STEP, 1min);
        events.ScheduleEvent(EVENT_CRYSTAL_GIFT_OF_LIFE, 80s);
        events.ScheduleEvent(EVENT_CRYSTAL_ESSENCE_OF_DREAMS, 155s);
        events.ScheduleEvent(EVENT_CRYSTAL_SOURCE_OF_MAGIC, 215s);
        events.ScheduleEvent(EVENT_TIMELOOP, 5min);
        events.ScheduleEvent(EVENT_TWILIGHT_ERUPTION, 6min);
        events.ScheduleEvent(EVENT_MELEE_CHECK, 10s);

        // Dead players who release must not linger phased into the realm
        _scheduler.Schedule(2s, [this](TaskContext context)
        {
            for (MapReference const& ref : me->GetMap()->GetPlayers())
            {
                Player* player = ref.GetSource();
                if (player && player->isDead() && player->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
                {
                    player->RemoveAurasDueToSpell(SPELL_TWILIGHT_SHIFT);
                    player->RemoveAurasDueToSpell(SPELL_HEROIC_WILL_GRANT);
                }
            }
            context.Repeat(2s);
        });
    }

    void KilledUnit(Unit* victim) override
    {
        if (victim->GetTypeId() == TYPEID_PLAYER && roll_chance_i(50))
            Talk(SAY_SLAY);
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
        Talk(SAY_DEATH);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        CleanupEncounter();
    }

    void EnterEvadeMode(EvadeReason why) override
    {
        // The entire raid leaves the realm for five seconds on every Hour of
        // Twilight; threat references merely go offline. Never evade for that.
        if (why == EVADE_REASON_NO_HOSTILES && IsAnyPlayerAliveInCombat())
            return;

        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        _EnterEvadeMode();
        CleanupEncounter();
        summons.DespawnAll();

        if (Creature* ysera = instance->GetCreature(DATA_YSERA_ULTRAXION))
            ysera->AI()->Talk(SAY_YSERA_WIPE);

        // The gauntlet stays cleared; the controller respawns the boss
        me->DespawnOrUnsummon();
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);

        if (!me->IsEngaged())
            return;

        // Keep timers running even while every player is outside the realm
        // (no selectable victim); victim selection refreshes on its own
        UpdateVictim();

        events.Update(diff);
        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_HOUR_OF_TWILIGHT:
                    Talk(SAY_HOUR_OF_TWILIGHT);
                    DoCastAOE(SPELL_HOUR_OF_TWILIGHT);
                    events.Repeat(45500ms);
                    break;
                case EVENT_MONSTROSITY_STEP:
                    if (_monstrosityStep < 6)
                    {
                        me->RemoveAurasDueToSpell(MonstrositySteps[_monstrosityStep - 1]);
                        DoCastSelf(MonstrositySteps[_monstrosityStep], true);
                        Talk(SAY_UNSTABLE_MONSTROSITY);
                        Talk(_monstrosityStep == 5 ? EMOTE_MAXIMUM_INSTABILITY : EMOTE_MORE_UNSTABLE);
                        if (++_monstrosityStep < 6)
                            events.Repeat(1min);
                    }
                    break;
                case EVENT_CRYSTAL_GIFT_OF_LIFE:
                    SummonCrystal(CRYSTAL_GIFT_OF_LIFE, DATA_ALEXSTRASZA_ULTRAXION);
                    break;
                case EVENT_CRYSTAL_ESSENCE_OF_DREAMS:
                    SummonCrystal(CRYSTAL_ESSENCE_OF_DREAMS, DATA_YSERA_ULTRAXION);
                    break;
                case EVENT_CRYSTAL_SOURCE_OF_MAGIC:
                    SummonCrystal(CRYSTAL_SOURCE_OF_MAGIC, DATA_KALECGOS_ULTRAXION);
                    break;
                case EVENT_TIMELOOP:
                {
                    Creature* nozdormu = instance->GetCreature(DATA_NOZDORMU_ULTRAXION);
                    Unit* caster = nozdormu ? static_cast<Unit*>(nozdormu) : me;
                    if (nozdormu)
                    {
                        nozdormu->AI()->Talk(SAY_ASPECT_GIFT);
                        nozdormu->AI()->Talk(EMOTE_ASPECT_GIFT);
                    }
                    caster->CastSpell(nullptr, SPELL_TIMELOOP, true);
                    break;
                }
                case EVENT_TWILIGHT_ERUPTION:
                    Talk(EMOTE_ERUPTION_INCOMING);
                    Talk(SAY_TWILIGHT_ERUPTION);
                    DoCastAOE(SPELL_TWILIGHT_ERUPTION);
                    break;
                case EVENT_MELEE_CHECK:
                    // Punish exploit setups with no reachable victim. During a
                    // full Heroic Will window this hits nobody (phase filtered).
                    if (!me->GetVictim() || !me->IsWithinMeleeRange(me->GetVictim()))
                        DoCastAOE(SPELL_TWILIGHT_BURST);
                    events.Repeat(6s);
                    break;
                default:
                    break;
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
        }

        DoMeleeAttackIfReady();
    }

private:
    bool IsAnyPlayerAliveInCombat() const
    {
        for (MapReference const& ref : me->GetMap()->GetPlayers())
        {
            Player* player = ref.GetSource();
            if (player && player->IsAlive() && !player->IsGameMaster() && player->IsInCombatWith(me))
                return true;
        }
        return false;
    }

    void SummonCrystal(uint8 index, uint32 aspectData)
    {
        if (Creature* aspect = instance->GetCreature(aspectData))
        {
            aspect->AI()->Talk(SAY_ASPECT_GIFT);
            aspect->AI()->Talk(EMOTE_ASPECT_GIFT);
        }

        // Summoned by the boss: the crystal inherits the twilight phase
        if (GameObject* crystal = me->SummonGameObject(CrystalGameObjects[index], CrystalPositions[index], QuaternionData::fromEulerAnglesZYX(CrystalPositions[index].GetOrientation(), 0.0f, 0.0f), 0))
            _crystalGuids[index] = crystal->GetGUID();
    }

    void DespawnCrystal(uint8 index)
    {
        if (GameObject* crystal = ObjectAccessor::GetGameObject(*me, _crystalGuids[index]))
            crystal->Delete();
        _crystalGuids[index].Clear();
    }

    void CleanupEncounter()
    {
        for (uint32 spellId : { SPELL_TWILIGHT_SHIFT, SPELL_HEROIC_WILL_GRANT, SPELL_HEROIC_WILL,
            SPELL_TIMELOOP, SPELL_LOOMING_DARKNESS,
            SPELL_GIFT_OF_LIFE, SPELL_GIFT_OF_LIFE_HEROIC,
            SPELL_ESSENCE_OF_DREAMS, SPELL_ESSENCE_OF_DREAMS_HEROIC,
            SPELL_SOURCE_OF_MAGIC, SPELL_SOURCE_OF_MAGIC_HEROIC })
            instance->DoRemoveAurasDueToSpellOnPlayers(spellId);

        for (uint32 spellId : FadingLightAll)
            instance->DoRemoveAurasDueToSpellOnPlayers(spellId);

        for (uint8 i = 0; i < 3; ++i)
            DespawnCrystal(i);

        PhasingHandler::ResetPhaseShift(me);
    }

    TaskScheduler _scheduler;
    uint8 _monstrosityStep = 0;
    uint8 _unsoakedHits = 0;
    std::unordered_map<ObjectGuid, uint8> _hourOfTwilightHits;
    std::array<bool, 3> _crystalClaimed = { };
    std::array<ObjectGuid, 3> _crystalGuids = { };
};

// 56305 - Ultraxion Gauntlet: runs the Deathwing RP, the Twilight Assaulter
// waves and Ultraxion's arrival. Rooted high above the platform center.
struct npc_ultraxion_gauntlet : public ScriptedAI
{
    npc_ultraxion_gauntlet(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript()), _summons(creature) { }

    void Reset() override
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetDisableGravity(true);

        _scheduler.CancelAll();
        _running = false;
        _hostile = false;
        _drakeKills = 0;
        _drakesSpawned = 0;

        // Watch for arriving players (gauntlet) or a missing boss (rebuild
        // after a wipe or server restart once the gauntlet was cleared)
        _scheduler.Schedule(2s, [this](TaskContext context)
        {
            if (_instance->GetBossState(DATA_HAGARA_THE_STORMBINDER) == DONE
                && _instance->GetBossState(DATA_ULTRAXION) != DONE
                && !_instance->GetCreature(DATA_ULTRAXION))
            {
                if (_instance->GetData(DATA_ULTRAXION_GAUNTLET_DONE))
                {
                    if (IsAnyPlayerNearby(120.0f))
                        SummonUltraxion(false);
                }
                else if (!_running && IsAnyPlayerNearby(60.0f))
                    StartGauntlet();
            }
            context.Repeat(2s);
        });
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

    void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
    {
        if (summon->GetEntry() == BOSS_ULTRAXION)
            return;

        ++_drakeKills;
        if (_running && _drakeKills >= GetTuning(_instance, me->GetMap()).DrakeKillTarget)
            EndGauntlet();
        else if (_running && _hostile)
            _scheduler.Schedule(8s, [this](TaskContext /*context*/)
            {
                if (_running)
                    SpawnDrakeWave();
            });
    }

    void JustSummoned(Creature* summon) override
    {
        _summons.Summon(summon);
    }

private:
    bool IsAnyPlayerNearby(float range) const
    {
        Position const& center = PlatformCenter;
        for (MapReference const& ref : me->GetMap()->GetPlayers())
        {
            Player* player = ref.GetSource();
            if (player && player->IsAlive() && !player->IsGameMaster()
                && player->GetExactDist2d(center.GetPositionX(), center.GetPositionY()) < range
                && std::abs(player->GetPositionZ() - PlatformCenter.GetPositionZ()) < 40.0f)
                return true;
        }
        return false;
    }

    void TalkHelper(uint32 dataId, uint8 group)
    {
        if (Creature* speaker = _instance->GetCreature(dataId))
            speaker->AI()->Talk(group);
    }

    void StartGauntlet()
    {
        _running = true;
        _hostile = false;
        _drakeKills = 0;
        _drakesSpawned = 0;

        TalkHelper(DATA_YSERA_ULTRAXION, SAY_YSERA_GAUNTLET_START);

        // Deathwing gloats from his perch while his clutch swarms the summit
        _scheduler.Schedule(10s, [this](TaskContext /*context*/) { TalkHelper(DATA_DEATHWING_ULTRAXION, SAY_DEATHWING_GAUNTLET_1); });
        _scheduler.Schedule(20s, [this](TaskContext /*context*/) { TalkHelper(DATA_DEATHWING_ULTRAXION, SAY_DEATHWING_GAUNTLET_2); });

        _scheduler.Schedule(5s, [this](TaskContext /*context*/)
        {
            uint8 count = GetTuning(_instance, me->GetMap()).DrakeMaxAirborne;
            for (uint8 i = 0; i < count; ++i)
                SpawnDrake();
        });

        _scheduler.Schedule(36s, [this](TaskContext /*context*/)
        {
            TalkHelper(DATA_ALEXSTRASZA_ULTRAXION, SAY_ALEXSTRASZA_BRING_DOWN);
            MakeDrakesAttackable();

            // The aspects shield themselves against the assault
            CastAspectShield(DATA_NOZDORMU_ULTRAXION, SPELL_ASPECT_SHIELD_NOZDORMU);
            CastAspectShield(DATA_KALECGOS_ULTRAXION, SPELL_ASPECT_SHIELD_KALECGOS);
            CastAspectShield(DATA_ALEXSTRASZA_ULTRAXION, SPELL_ASPECT_SHIELD_ALEXSTRASZA);
            CastAspectShield(DATA_YSERA_ULTRAXION, SPELL_ASPECT_SHIELD_YSERA);
            CastAspectShield(DATA_THRALL_ULTRAXION, SPELL_WARD_OF_EARTH);
        });

        // Failsafe: the assault lifts on its own eventually
        _scheduler.Schedule(Milliseconds(GauntletFailsafeMs), [this](TaskContext /*context*/)
        {
            if (_running)
                EndGauntlet();
        });
    }

    void CastAspectShield(uint32 dataId, uint32 spellId)
    {
        if (Creature* aspect = _instance->GetCreature(dataId))
            aspect->CastSpell(aspect, spellId, true);
    }

    void SpawnDrake()
    {
        // Alternate the four flight templates; 25 player raids mix in the
        // sturdier 57795 variant
        uint32 entry = NPC_TWILIGHT_ASSAULTER_N + (_drakesSpawned % 4);
        if (Is25ManRaid() && (_drakesSpawned % 3) == 2)
            entry = 57795;
        ++_drakesSpawned;

        float angle = frand(0.0f, 2.0f * float(M_PI));
        Position spawn = PlatformCenter;
        spawn.m_positionX += std::cos(angle) * (DrakeRingRadius + 40.0f);
        spawn.m_positionY += std::sin(angle) * (DrakeRingRadius + 40.0f);
        spawn.m_positionZ = DrakeRingZ + frand(0.0f, 15.0f);

        if (Creature* drake = me->SummonCreature(entry, spawn, TEMPSUMMON_MANUAL_DESPAWN))
            if (!_hostile)
                drake->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_SELECTABLE);
    }

    bool Is25ManRaid() const
    {
        Difficulty difficulty = me->GetMap()->GetDifficulty();
        return difficulty == RAID_DIFFICULTY_25MAN_NORMAL || difficulty == RAID_DIFFICULTY_25MAN_HEROIC;
    }

    void SpawnDrakeWave()
    {
        uint8 airborne = 0;
        for (ObjectGuid guid : _summons)
            if (Creature* summon = ObjectAccessor::GetCreature(*me, guid))
                if (summon->IsAlive() && summon->GetEntry() != BOSS_ULTRAXION)
                    ++airborne;

        if (airborne < GetTuning(_instance, me->GetMap()).DrakeMaxAirborne)
            SpawnDrake();
    }

    void MakeDrakesAttackable()
    {
        _hostile = true;
        for (ObjectGuid guid : _summons)
            if (Creature* drake = ObjectAccessor::GetCreature(*me, guid))
                if (drake->IsAlive() && drake->GetEntry() != BOSS_ULTRAXION)
                    drake->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_SELECTABLE);
    }

    void EndGauntlet()
    {
        _running = false;
        _hostile = false;
        _instance->SetData(DATA_ULTRAXION_GAUNTLET_DONE, 1);

        // Survivors slip back into the twilight
        for (ObjectGuid guid : _summons)
            if (Creature* drake = ObjectAccessor::GetCreature(*me, guid))
                if (drake->IsAlive() && drake->GetEntry() != BOSS_ULTRAXION)
                    drake->AI()->DoAction(ACTION_GAUNTLET_END);

        TalkHelper(DATA_DEATHWING_ULTRAXION, SAY_DEATHWING_GAUNTLET_END);
        _scheduler.Schedule(14s, [this](TaskContext /*context*/) { TalkHelper(DATA_DEATHWING_ULTRAXION, SAY_DEATHWING_ULTIMATE); });
        _scheduler.Schedule(27s, [this](TaskContext /*context*/) { TalkHelper(DATA_DEATHWING_ULTRAXION, SAY_DEATHWING_HOUR); });
        _scheduler.Schedule(39s, [this](TaskContext /*context*/) { TalkHelper(DATA_YSERA_ULTRAXION, SAY_YSERA_PULL_WARNING); });
        _scheduler.Schedule(46s, [this](TaskContext /*context*/) { SummonUltraxion(true); });
    }

    void SummonUltraxion(bool withIntro)
    {
        if (_instance->GetCreature(DATA_ULTRAXION))
            return;

        Position const& pos = withIntro ? UltraxionSpawnPos : UltraxionAnchorPos;
        if (Creature* ultraxion = me->SummonCreature(BOSS_ULTRAXION, pos, TEMPSUMMON_MANUAL_DESPAWN))
            if (withIntro)
                ultraxion->AI()->DoAction(ACTION_START_INTRO);
    }

    InstanceScript* _instance;
    SummonList _summons;
    TaskScheduler _scheduler;
    bool _running = false;
    bool _hostile = false;
    uint8 _drakeKills = 0;
    uint32 _drakesSpawned = 0;
};

// 56249-56252, 57795 - Twilight Assaulter: circles the summit and strafes the
// platform with twilight flames until brought down
struct npc_ultraxion_twilight_assaulter : public ScriptedAI
{
    npc_ultraxion_twilight_assaulter(Creature* creature) : ScriptedAI(creature) { }

    void Reset() override
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetDisableGravity(true);
        me->SetSpeedRate(MOVE_FLIGHT, DrakeFlightSpeed);

        _escaping = false;
        _scheduler.CancelAll();
        _scheduler.Schedule(1s, [this](TaskContext /*context*/) { NextLeg(); });

        // Strafing runs scorch a random defender
        _scheduler.Schedule(10s, 15s, [this](TaskContext context)
        {
            if (!_escaping)
                if (Unit* target = SelectRandomPlayer())
                    DoCast(target, SPELL_TWILIGHT_FLAMES);
            context.Repeat(9s, 16s);
        });
    }

    void DoAction(int32 action) override
    {
        if (action != ACTION_GAUNTLET_END)
            return;

        _escaping = true;
        _scheduler.CancelAll();
        me->InterruptNonMeleeSpells(false);
        DoCastSelf(SPELL_TWILIGHT_ESCAPE, true);

        Position away = me->GetPosition();
        away.m_positionX += std::cos(me->GetOrientation()) * 250.0f;
        away.m_positionY += std::sin(me->GetOrientation()) * 250.0f;
        away.m_positionZ += 60.0f;
        me->GetMotionMaster()->MovePoint(POINT_DRAKE_ESCAPE, away, false);
        me->DespawnOrUnsummon(8s);
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != POINT_MOTION_TYPE || _escaping)
            return;

        if (pointId == POINT_DRAKE_RING || pointId == POINT_DRAKE_STRAFE)
            _scheduler.Schedule(Milliseconds(urand(300, 900)), [this](TaskContext /*context*/) { NextLeg(); });
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

private:
    Player* SelectRandomPlayer() const
    {
        std::vector<Player*> players;
        for (MapReference const& ref : me->GetMap()->GetPlayers())
        {
            Player* player = ref.GetSource();
            if (player && player->IsAlive() && !player->IsGameMaster()
                && player->GetExactDist2d(PlatformCenter.GetPositionX(), PlatformCenter.GetPositionY()) < 60.0f
                && std::abs(player->GetPositionZ() - PlatformCenter.GetPositionZ()) < 40.0f)
                players.push_back(player);
        }
        if (players.empty())
            return nullptr;
        return Trinity::Containers::SelectRandomContainerElement(players);
    }

    void NextLeg()
    {
        if (_escaping)
            return;

        // Mostly circle the summit; sometimes cut straight across the platform
        Position dest = PlatformCenter;
        if (roll_chance_i(35))
        {
            dest.m_positionX += frand(-25.0f, 25.0f);
            dest.m_positionY += frand(-25.0f, 25.0f);
            dest.m_positionZ = DrakeStrafeZ + frand(0.0f, 4.0f);
            me->GetMotionMaster()->MovePoint(POINT_DRAKE_STRAFE, dest, false);
        }
        else
        {
            float angle = me->GetPosition().GetAngle(&PlatformCenter) + float(M_PI) + frand(0.5f, 1.2f);
            dest.m_positionX += std::cos(angle) * DrakeRingRadius;
            dest.m_positionY += std::sin(angle) * DrakeRingRadius;
            dest.m_positionZ = DrakeRingZ + frand(-4.0f, 10.0f);
            me->GetMotionMaster()->MovePoint(POINT_DRAKE_RING, dest, false);
        }
    }

    TaskScheduler _scheduler;
    bool _escaping = false;
};

void SelectAllRaidPlayers(Unit const* caster, std::list<WorldObject*>& targets)
{
    targets.clear();
    for (MapReference const& ref : caster->GetMap()->GetPlayers())
    {
        Player* player = ref.GetSource();
        if (player && player->IsAlive() && !player->IsGameMaster())
            targets.push_back(player);
    }
}

// 106369 - Twilight Shift (area missile) and 105554 - Heroic Will (grant):
// the DBC entry-check targeting has no conditions; select the raid by hand
class spell_ultraxion_twilight_shift_aoe : public SpellScript
{
    void FilterTargets(std::list<WorldObject*>& targets)
    {
        SelectAllRaidPlayers(GetCaster(), targets);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect.Register(&spell_ultraxion_twilight_shift_aoe::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
    }
};

// 105984 - Timeloop: one area cast covers the raid (entry-check targeting)
class spell_ultraxion_timeloop_targets : public SpellScript
{
    void FilterTargets(std::list<WorldObject*>& targets)
    {
        SelectAllRaidPlayers(GetCaster(), targets);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect.Register(&spell_ultraxion_timeloop_targets::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        OnObjectAreaTargetSelect.Register(&spell_ultraxion_timeloop_targets::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENTRY);
    }
};

// 106108 - Heroic Will
class spell_ultraxion_heroic_will : public SpellScript
{
    SpellCastResult CheckCast()
    {
        // Only usable to leave the twilight realm
        if (!GetCaster()->HasAura(SPELL_TWILIGHT_SHIFT))
            return SPELL_FAILED_NOT_HERE;
        return SPELL_CAST_OK;
    }

    void Register() override
    {
        OnCheckCast.Register(&spell_ultraxion_heroic_will::CheckCast);
    }
};

class spell_ultraxion_heroic_will_AuraScript : public AuraScript
{
    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (GetTargetApplication()->GetRemoveMode() == AuraRemoveFlags::ByDeath)
            return;

        // Five seconds of respite are over: back into the twilight realm
        Unit* target = GetTarget();
        InstanceScript const* instance = target->GetInstanceScript();
        if (instance && instance->GetBossState(DATA_ULTRAXION) == IN_PROGRESS && target->IsAlive())
            target->CastSpell(target, SPELL_TWILIGHT_SHIFT, true);
    }

    void Register() override
    {
        AfterEffectRemove.Register(&spell_ultraxion_heroic_will_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_PACIFY_SILENCE, AURA_EFFECT_HANDLE_REAL);
    }
};

// 106371, 109415, 109416, 109417 - Hour of Twilight (cast + Fading Light aura)
class spell_ultraxion_hour_of_twilight : public SpellScript
{
    void PreventShiftback(SpellEffIndex effIndex)
    {
        // The native EFFECT_0 trigger (106174) strips Heroic Will from the
        // whole raid at impact; if the blast missile resolved later, dodgers
        // would be pulled back in and killed. Rely on the 5s expiry instead.
        PreventHitDefaultEffect(effIndex);
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_ultraxion_hour_of_twilight::PreventShiftback, EFFECT_0, SPELL_EFFECT_TRIGGER_SPELL);
    }
};

class spell_ultraxion_hour_of_twilight_AuraScript : public AuraScript
{
    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        // The DBC periodic would cast Fading Light at the aura holder (the
        // boss itself); pick the real targets by hand
        PreventDefaultAction();

        Unit* caster = GetTarget();
        Creature* ultraxion = caster->ToCreature();
        if (!ultraxion || !ultraxion->IsAIEnabled())
            return;

        ultraxion->AI()->Talk(SAY_FADING_LIGHT);

        if (Unit* victim = ultraxion->GetThreatManager().GetCurrentVictim())
            ultraxion->CastSpell(victim, SPELL_FADING_LIGHT, true);

        InstanceScript const* instance = ultraxion->GetInstanceScript();
        uint8 extraTargets = GetTuning(instance, ultraxion->GetMap()).FadingLightExtraTargets;
        if (!extraTargets)
            return;

        std::vector<Player*> candidates = GetTwilightRealmPlayers(ultraxion->GetMap());
        Trinity::Containers::EraseIf(candidates, [ultraxion](Player* player)
        {
            if (player == ultraxion->GetThreatManager().GetCurrentVictim())
                return true;
            for (uint32 spellId : FadingLightAll)
                if (player->HasAura(spellId))
                    return true;
            return false;
        });

        if (candidates.size() > extraTargets)
            Trinity::Containers::RandomResize(candidates, extraTargets);

        for (Player* target : candidates)
            ultraxion->CastSpell(target, SPELL_FADING_LIGHT_RAID, true);
    }

    void Register() override
    {
        OnEffectPeriodic.Register(&spell_ultraxion_hour_of_twilight_AuraScript::HandlePeriodic, EFFECT_2, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

// 103327 - Hour of Twilight (blast on everyone still in the realm)
class spell_ultraxion_hour_of_twilight_damage : public SpellScript
{
    void FilterTargets(std::list<WorldObject*>& targets)
    {
        // Entry-check targeting is unreliable without conditions rows; the
        // blast strikes everyone still inside the twilight realm
        targets.clear();
        for (MapReference const& ref : GetCaster()->GetMap()->GetPlayers())
        {
            Player* player = ref.GetSource();
            if (player && player->IsAlive() && !player->IsGameMaster() && IsInTwilightRealm(player))
                targets.push_back(player);
        }
        _hitCount = targets.size();
    }

    void FilterMissileTargets(std::list<WorldObject*>& targets)
    {
        SelectAllRaidPlayers(GetCaster(), targets);
        targets.remove_if([](WorldObject* target)
        {
            Unit* unit = target->ToUnit();
            return !unit || !IsInTwilightRealm(unit);
        });
    }

    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        uint32 damage = GetHitDamage();
        ApplyLFRDamageReduction(GetCaster()->GetInstanceScript(), damage);
        SetHitDamage(damage);

        // Minutes to Midnight bookkeeping
        if (Creature* ultraxion = GetCaster()->ToCreature())
            if (Unit* target = GetHitUnit())
                ultraxion->AI()->SetGUID(target->GetGUID(), GUID_HOUR_OF_TWILIGHT_HIT);
    }

    void SuppressLoomingDarkness(SpellEffIndex effIndex)
    {
        // The Looming Darkness chain (force-cast 109231) is Raid Finder
        // forgiveness only
        InstanceScript const* instance = GetCaster()->GetInstanceScript();
        if (!IsLFR(instance))
            PreventHitDefaultEffect(effIndex);
    }

    void AfterCastHandler()
    {
        if (_hitCount)
            return;

        // Nobody soaked: the blast strikes the aspects instead
        if (Creature* ultraxion = GetCaster()->ToCreature())
            if (ultraxion->IsAIEnabled())
                ultraxion->AI()->DoAction(ACTION_HOUR_NOT_SOAKED);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect.Register(&spell_ultraxion_hour_of_twilight_damage::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        OnObjectAreaTargetSelect.Register(&spell_ultraxion_hour_of_twilight_damage::FilterMissileTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENTRY);
        OnEffectHitTarget.Register(&spell_ultraxion_hour_of_twilight_damage::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        OnEffectHitTarget.Register(&spell_ultraxion_hour_of_twilight_damage::SuppressLoomingDarkness, EFFECT_1, SPELL_EFFECT_FORCE_CAST);
        AfterCast.Register(&spell_ultraxion_hour_of_twilight_damage::AfterCastHandler);
    }

    size_t _hitCount = 0;
};

// 109231 - Looming Darkness (Raid Finder: the first offense is forgiven)
class spell_ultraxion_looming_darkness_missile : public SpellScript
{
    void HandleKill(SpellEffIndex effIndex)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        if (!target->HasAura(SPELL_LOOMING_DARKNESS))
        {
            PreventHitDefaultEffect(effIndex);
            caster->CastSpell(target, SPELL_LOOMING_DARKNESS, true);
        }
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_ultraxion_looming_darkness_missile::HandleKill, EFFECT_0, SPELL_EFFECT_INSTAKILL);
    }
};

// 105925 + forks, 109075 + forks - Fading Light
class spell_ultraxion_fading_light : public AuraScript
{
    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        // The real timer is rolled per application
        int32 duration = int32(urand(5000, 10000));
        GetAura()->SetMaxDuration(duration);
        GetAura()->SetDuration(duration);
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (GetTargetApplication()->GetRemoveMode() != AuraRemoveFlags::Expired)
            return;

        Unit* target = GetTarget();
        Unit* caster = GetCaster();
        if (!caster || !target->IsAlive())
            return;

        // Sheltered outside the realm: survives. Caught inside: consumed.
        if (!IsInTwilightRealm(target))
            return;

        if (IsLFR(target->GetInstanceScript()))
            caster->CastSpell(target, SPELL_LOOMING_DARKNESS, true);
        else
            caster->CastSpell(target, SPELL_FADING_LIGHT_KILL, true);
    }

    void Register() override
    {
        AfterEffectApply.Register(&spell_ultraxion_fading_light::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove.Register(&spell_ultraxion_fading_light::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 105925 + forks - Fading Light (tank cast): EFFECT_1 would immediately spray
// the raid variant onto the whole raid on every difficulty; the heroic extra
// targets are chosen by the Hour of Twilight script instead
class spell_ultraxion_fading_light_tank : public SpellScript
{
    void PreventRaidSpray(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_ultraxion_fading_light_tank::PreventRaidSpray, EFFECT_1, SPELL_EFFECT_TRIGGER_SPELL);
    }
};

// 109075 + forks - Fading Light (raid): confine the area targeting to the
// player picked by the Hour of Twilight script
class spell_ultraxion_fading_light_raid : public SpellScript
{
    void FilterTargets(std::list<WorldObject*>& targets)
    {
        WorldObject* primary = GetExplTargetUnit();
        targets.clear();
        if (primary)
            targets.push_back(primary);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect.Register(&spell_ultraxion_fading_light_raid::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
    }
};

// 106372, 106376, 106377, 106378, 106379, 106380 - Unstable Monstrosity
class spell_ultraxion_unstable_monstrosity : public AuraScript
{
    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        // The DBC trigger chain dead-ends in an unscripted script effect;
        // strike one random soul still inside the realm directly
        PreventDefaultAction();

        Unit* caster = GetTarget();
        std::vector<Player*> candidates = GetTwilightRealmPlayers(caster->GetMap());
        if (candidates.empty())
            return;

        Player* target = Trinity::Containers::SelectRandomContainerElement(candidates);
        caster->CastSpell(target, SPELL_TWILIGHT_INSTABILITY, true);
    }

    void Register() override
    {
        OnEffectPeriodic.Register(&spell_ultraxion_unstable_monstrosity::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

// 106375, 109182, 109183, 109184 - Twilight Instability
class spell_ultraxion_twilight_instability : public SpellScript
{
    void FilterTargets(std::list<WorldObject*>& targets)
    {
        // Single victim, chosen by the Unstable Monstrosity script
        WorldObject* primary = GetExplTargetUnit();
        targets.clear();
        if (primary)
            targets.push_back(primary);
    }

    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        uint32 damage = GetHitDamage();
        ApplyLFRDamageReduction(GetCaster()->GetInstanceScript(), damage);
        SetHitDamage(damage);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect.Register(&spell_ultraxion_twilight_instability::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
        OnEffectHitTarget.Register(&spell_ultraxion_twilight_instability::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 106415 - Twilight Burst
class spell_ultraxion_twilight_burst : public SpellScript
{
    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        uint32 damage = GetHitDamage();
        ApplyLFRDamageReduction(GetCaster()->GetInstanceScript(), damage);
        SetHitDamage(damage);
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_ultraxion_twilight_burst::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 106218 - Last Defender of Azeroth (Thrall): each tank-capable class gets
// its own cooldown blessing
class spell_ultraxion_last_defender : public SpellScript
{
    void HandleScript(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        for (MapReference const& ref : caster->GetMap()->GetPlayers())
        {
            Player* player = ref.GetSource();
            if (!player || !player->IsAlive() || player->IsGameMaster())
                continue;

            uint32 spellId = 0;
            switch (player->getClass())
            {
                case CLASS_WARRIOR:      spellId = SPELL_LAST_DEFENDER_WARRIOR; break;
                case CLASS_DRUID:        spellId = SPELL_LAST_DEFENDER_DRUID; break;
                case CLASS_PALADIN:      spellId = SPELL_LAST_DEFENDER_PALADIN; break;
                case CLASS_DEATH_KNIGHT: spellId = SPELL_LAST_DEFENDER_DEATH_KNIGHT; break;
                default: continue;
            }
            caster->CastSpell(player, spellId, true);
        }
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_ultraxion_last_defender::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

// 105984 - Timeloop: the next killing blow heals instead
class spell_ultraxion_timeloop : public AuraScript
{
    void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
    {
        amount = -1;
    }

    void Absorb(AuraEffect* /*aurEff*/, DamageInfo& dmgInfo, uint32& absorbAmount)
    {
        Unit* target = GetTarget();
        if (dmgInfo.GetDamage() < target->GetHealth())
            return;

        absorbAmount = dmgInfo.GetDamage();
        target->CastSpell(target, SPELL_TIMELOOP_HEAL, CastSpellExtraArgs(true).AddSpellBP0(int32(target->GetMaxHealth())));
        Remove();
    }

    void Register() override
    {
        DoEffectCalcAmount.Register(&spell_ultraxion_timeloop::CalculateAmount, EFFECT_1, SPELL_AURA_SCHOOL_ABSORB);
        OnEffectAbsorb.Register(&spell_ultraxion_timeloop::Absorb, EFFECT_1);
    }
};

// 105896 / 105900 / 105903 - the crystal gifts (cast by the goober on the user)
class spell_ultraxion_crystal_gift : public SpellScript
{
public:
    spell_ultraxion_crystal_gift(uint8 crystalIndex) : _crystalIndex(crystalIndex) { }

private:
    Creature* GetUltraxion() const
    {
        if (InstanceScript* instance = GetCaster()->GetInstanceScript())
            return instance->GetCreature(DATA_ULTRAXION);
        return nullptr;
    }

    SpellCastResult CheckCast()
    {
        // One gift per crystal: first click wins
        if (Creature* ultraxion = GetUltraxion())
            if (ultraxion->AI()->GetData(DATA_CRYSTAL_CLAIMED_BASE + _crystalIndex))
                return SPELL_FAILED_CASTER_AURASTATE;
        return SPELL_CAST_OK;
    }

    void HandleAfterCast()
    {
        Unit* caster = GetCaster();

        if (Creature* ultraxion = GetUltraxion())
            if (ultraxion->IsAIEnabled())
                ultraxion->AI()->DoAction(ACTION_CRYSTAL_CLAIMED_BASE + _crystalIndex);

        // On heroic the gift carries the empowered component as well
        if (IsHeroicUltraxion(caster->GetMap()))
            caster->CastSpell(caster, CrystalBuffsHeroic[_crystalIndex], true);
    }

    void Register() override
    {
        OnCheckCast.Register(&spell_ultraxion_crystal_gift::CheckCast);
        AfterCast.Register(&spell_ultraxion_crystal_gift::HandleAfterCast);
    }

    uint8 _crystalIndex;
};

class spell_ultraxion_gift_of_life : public spell_ultraxion_crystal_gift
{
public:
    spell_ultraxion_gift_of_life() : spell_ultraxion_crystal_gift(CRYSTAL_GIFT_OF_LIFE) { }
};

class spell_ultraxion_essence_of_dreams : public spell_ultraxion_crystal_gift
{
public:
    spell_ultraxion_essence_of_dreams() : spell_ultraxion_crystal_gift(CRYSTAL_ESSENCE_OF_DREAMS) { }
};

class spell_ultraxion_source_of_magic : public spell_ultraxion_crystal_gift
{
public:
    spell_ultraxion_source_of_magic() : spell_ultraxion_crystal_gift(CRYSTAL_SOURCE_OF_MAGIC) { }
};

// 105900 / 109342 - Essence of Dreams: every heal is mirrored to the raid
class spell_ultraxion_essence_of_dreams_mirror : public AuraScript
{
    bool CheckProc(ProcEventInfo& eventInfo)
    {
        if (_mirroring)
            return false;

        HealInfo* healInfo = eventInfo.GetHealInfo();
        if (!healInfo || !healInfo->GetHeal())
            return false;

        // Never mirror the mirror
        SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
        return !spellInfo || spellInfo->Id != SPELL_ESSENCE_OF_DREAMS_HEAL;
    }

    void OnProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        _mirroring = true;
        GetTarget()->CastSpell(GetTarget(), SPELL_ESSENCE_OF_DREAMS_HEAL,
            CastSpellExtraArgs(true).AddSpellBP0(int32(eventInfo.GetHealInfo()->GetHeal())));
        _mirroring = false;
    }

    void Register() override
    {
        DoCheckProc.Register(&spell_ultraxion_essence_of_dreams_mirror::CheckProc);
        OnEffectProc.Register(&spell_ultraxion_essence_of_dreams_mirror::OnProc, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
    }

    bool _mirroring = false;
};
}

void AddSC_boss_ultraxion()
{
    using namespace DragonSoul;
    using namespace DragonSoul::Ultraxion;

    RegisterDragonSoulCreatureAI(boss_ultraxion);
    RegisterDragonSoulCreatureAI(npc_ultraxion_gauntlet);
    RegisterDragonSoulCreatureAI(npc_ultraxion_twilight_assaulter);

    RegisterSpellScript(spell_ultraxion_twilight_shift_aoe);
    RegisterSpellAndAuraScriptPair(spell_ultraxion_heroic_will, spell_ultraxion_heroic_will_AuraScript);
    RegisterSpellAndAuraScriptPair(spell_ultraxion_hour_of_twilight, spell_ultraxion_hour_of_twilight_AuraScript);
    RegisterSpellScript(spell_ultraxion_hour_of_twilight_damage);
    RegisterSpellScript(spell_ultraxion_looming_darkness_missile);
    RegisterSpellScript(spell_ultraxion_fading_light);
    RegisterSpellScript(spell_ultraxion_fading_light_tank);
    RegisterSpellScript(spell_ultraxion_fading_light_raid);
    RegisterSpellScript(spell_ultraxion_unstable_monstrosity);
    RegisterSpellScript(spell_ultraxion_twilight_instability);
    RegisterSpellScript(spell_ultraxion_twilight_burst);
    RegisterSpellScript(spell_ultraxion_last_defender);
    RegisterSpellAndAuraScriptPair(spell_ultraxion_timeloop_targets, spell_ultraxion_timeloop);
    RegisterSpellScript(spell_ultraxion_gift_of_life);
    RegisterSpellScript(spell_ultraxion_essence_of_dreams);
    RegisterSpellScript(spell_ultraxion_source_of_magic);
    RegisterSpellScript(spell_ultraxion_essence_of_dreams_mirror);
}
