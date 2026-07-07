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
#include "InstanceScript.h"
#include "Map.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
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

namespace DragonSoul::Yorsahj
{
enum Texts
{
    // Each Shath'Yar yell is followed by its translation, whispered to the raid
    SAY_INTRO           = 0,  // "Ak'agthshi ma uhnish, ak'uq shg'cul vwahuhn! ..."
    SAY_AGGRO           = 2,  // "Iilth qi'uothk shn'ma yeh'glu Shath'Yar! H'IWN IILTH!"
    SAY_SUMMON_FIRST    = 4,  // three paired variants: yells 4/6/8, whispers 5/7/9
    SAY_SLAY_FIRST      = 10, // three paired variants: yells 10/12/14, whispers 11/13/15
    SAY_DEATH           = 16  // "Ez, Shuul'wah! Sk'woth'gl yu'gaz yoh'ghyl iilth!"
};

enum Spells
{
    // Yor'sahj
    SPELL_VOID_BOLT                 = 104849, // forks: 108383 / 108384 / 108385 (direct hit + 12s DoT on the tank)
    SPELL_VOID_BOLT_AOE             = 105416, // forks: 109549 / 109550 / 109551 (raid hit while Glowing Blood is up)
    SPELL_BERSERK                   = 26662,

    // Blood of Shu'ma buffs gained when a globule is consumed (60s)
    SPELL_BLACK_BLOOD_OF_SHUMA      = 104894,
    SPELL_SHADOWED_BLOOD_OF_SHUMA   = 104896, // purple
    SPELL_CRIMSON_BLOOD_OF_SHUMA    = 104897, // red
    SPELL_ACIDIC_BLOOD_OF_SHUMA     = 104898, // green
    SPELL_COBALT_BLOOD_OF_SHUMA     = 105027, // blue
    SPELL_GLOWING_BLOOD_OF_SHUMA    = 104901, // yellow: +50% attack/cast speed, doubled ability rate

    // Granted abilities
    SPELL_SEARING_BLOOD             = 105033, // red    - forks: 108356 / 108357 / 108358
    SPELL_SEARING_BLOOD_LFR         = 108218, // red    - Raid Finder values (fork: 108363)
    SPELL_DIGESTIVE_ACID            = 105573, // green  - forks: 108350 / 108351 / 108352 (damage + 4yd splash at dest)
    SPELL_DIGESTIVE_ACID_LFR        = 108419, // green  - Raid Finder values, no splash (fork: 109543)
    SPELL_DEEP_CORRUPTION           = 105171, // purple - AoE-applied 25s debuff, procs on taken heals
    SPELL_DEEP_CORRUPTION_COUNTER   = 103628, // purple - per-player stack counter (detonates at 5)
    SPELL_DEEP_CORRUPTION_DAMAGE    = 105173, // purple - forks: 108347 / 108348 / 108349 (player self-cast)
    SPELL_CORRUPTED_MINIONS         = 105636, // black  - 25 player: 109558; 1s ticks of 105637 (summon 56265)
    SPELL_SUMMON_MANA_VOID          = 105034, // blue   - summons 56231
    SPELL_MANA_VOID_DRAIN           = 105530, // blue   - 4s power burn on all mana users
    SPELL_MANA_DIFFUSION            = 105539, // blue   - energize on Void death (value scripted to full mana)
    SPELL_MANA_DIFFUSION_LFR        = 108228, // blue   - Raid Finder: percent-based, huge radius
    SPELL_PSYCHIC_SLICE             = 105671, // forks: 108353 / 108354 / 108355

    // Globules
    SPELL_FUSING_VAPORS_HEAL        = 103635, // 5% max health heal on the other globules (also 108233)
    SPELL_FUSING_VAPORS_IMMUNITY    = 105904, // all-schools immunity shell once a globule dies
    SPELL_SPAWNING_POOL             = 105600  // pad visual while dormant
};

enum Colors : uint8
{
    COLOR_BLACK = 0,
    COLOR_BLUE,
    COLOR_GREEN,
    COLOR_PURPLE,
    COLOR_RED,
    COLOR_YELLOW,
    MAX_COLORS
};

enum Events
{
    EVENT_VOID_BOLT = 1,
    EVENT_COLOR_COMBINATION,
    EVENT_SEARING_BLOOD,
    EVENT_DIGESTIVE_ACID,
    EVENT_CORRUPTED_MINIONS,
    EVENT_SUMMON_MANA_VOID,
    EVENT_BERSERK
};

enum Actions
{
    ACTION_GLOBULE_DIED = 1,
    ACTION_FUSING_VAPORS_IMMUNITY
};

enum Data
{
    DATA_SPAWN_GLOBULE_SET = 1, // spell script -> boss AI (value: Color Combination spell id)
    DATA_GLOBULE_CONSUMED  = 2, // globule AI -> boss AI (value: color)
    DATA_GLOBULE_COLOR     = 3  // boss AI -> globule AI (value: color)
};

enum Points
{
    POINT_CONSUME = 1
};

namespace
{
// Globule walk: attackable 7s after the Color Combination cast, consumed at
// +34.5s (retail DBM timers) - so 27.5s of travel time from the spawn pads
constexpr float GlobuleTravelSeconds = 27.5f;

// Searing Blood strikes the farthest targets harder (community-accepted fit)
constexpr float SearingBloodDamagePerYard = 0.02f;

// Where the globules are consumed - the boss stands here
Position const ConsumptionCenter = { -1764.30f, -3035.05f, -182.469f };

struct GlobuleInfo
{
    uint32 Entry;
    uint32 BuffSpell;
    Position SpawnPos;
};

// Fixed color-matched spawn pads by the room walls (retail sniff positions)
GlobuleInfo const GlobuleData[MAX_COLORS] =
{
    /*COLOR_BLACK */ { NPC_DARK_GLOBULE,     SPELL_BLACK_BLOOD_OF_SHUMA,    { -1808.226f, -3136.740f, -173.480f } },
    /*COLOR_BLUE  */ { NPC_COBALT_GLOBULE,   SPELL_COBALT_BLOOD_OF_SHUMA,   { -1722.604f, -3137.158f, -173.390f } },
    /*COLOR_GREEN */ { NPC_ACIDIC_GLOBULE,   SPELL_ACIDIC_BLOOD_OF_SHUMA,   { -1723.757f, -2935.328f, -174.029f } },
    /*COLOR_PURPLE*/ { NPC_SHADOWED_GLOBULE, SPELL_SHADOWED_BLOOD_OF_SHUMA, { -1663.894f, -3077.130f, -174.479f } },
    /*COLOR_RED   */ { NPC_CRIMSON_GLOBULE,  SPELL_CRIMSON_BLOOD_OF_SHUMA,  { -1662.964f, -2992.283f, -173.514f } },
    /*COLOR_YELLOW*/ { NPC_GLOWING_GLOBULE,  SPELL_GLOWING_BLOOD_OF_SHUMA,  { -1863.986f, -2993.090f, -174.110f } }
};

struct ColorCombination
{
    uint32 SpellId;
    uint8 Colors[3];
    uint8 HeroicExtra;
};

// The six fixed retail combinations (DBM data). Heroic adds a fourth color.
ColorCombination const Combinations[] =
{
    { 105420, { COLOR_PURPLE, COLOR_GREEN,  COLOR_BLUE   }, COLOR_BLACK  },
    { 105435, { COLOR_GREEN,  COLOR_RED,    COLOR_BLACK  }, COLOR_BLUE   },
    { 105436, { COLOR_GREEN,  COLOR_YELLOW, COLOR_RED    }, COLOR_BLACK  },
    { 105437, { COLOR_PURPLE, COLOR_BLUE,   COLOR_YELLOW }, COLOR_GREEN  },
    { 105439, { COLOR_BLUE,   COLOR_BLACK,  COLOR_YELLOW }, COLOR_PURPLE },
    { 105440, { COLOR_PURPLE, COLOR_RED,    COLOR_BLACK  }, COLOR_YELLOW }
};

bool IsHeroicYorsahj(Map const* map)
{
    return map->GetDifficulty() == RAID_DIFFICULTY_10MAN_HEROIC
        || map->GetDifficulty() == RAID_DIFFICULTY_25MAN_HEROIC;
}

bool IsLFR(InstanceScript const* instance)
{
    return instance && instance->IsLFR();
}

// Raid Finder tuning: reduced health (from the LFR stats templates) and
// reduced outgoing damage, applied on top of the 25 player normal profile
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
}

struct boss_yorsahj : public BossAI
{
    boss_yorsahj(Creature* creature) : BossAI(creature, DATA_YORSAHJ_THE_UNSLEEPING)
    {
        SetCombatMovement(false); // the boss never leaves the room center
    }

    void Reset() override
    {
        _Reset();
        _scheduler.CancelAll();
        _pendingGlobules = 0;
        _lastCombination = -1;
        me->SetReactState(REACT_AGGRESSIVE);
        ApplyLFRHealth(me, instance, NPC_YORSAHJ_LFR_STATS);
    }

    void DamageDealt(Unit* /*victim*/, uint32& damage, DamageEffectType damageType) override
    {
        // Direct spell hits are excluded: Searing Blood and Digestive Acid use
        // their dedicated Raid Finder spells, Void Bolt is cut in its spell
        // script. This handles melee and the Void Bolt DoT ticks.
        if (damageType != SPELL_DIRECT_DAMAGE)
            ApplyLFRDamageReduction(instance, damage);
    }

    void MoveInLineOfSight(Unit* who) override
    {
        if (!_introDone && who->GetTypeId() == TYPEID_PLAYER && !who->ToPlayer()->IsGameMaster() && me->IsWithinDistInMap(who, 70.0f))
        {
            _introDone = true;
            TalkPairToRaid(SAY_INTRO);
        }

        BossAI::MoveInLineOfSight(who);
    }

    void JustEngagedWith(Unit* who) override
    {
        BossAI::JustEngagedWith(who);
        TalkPairToRaid(SAY_AGGRO);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

        events.ScheduleEvent(EVENT_VOID_BOLT, 2s);
        events.ScheduleEvent(EVENT_COLOR_COMBINATION, 22s);
        events.ScheduleEvent(EVENT_BERSERK, 10min);
    }

    void KilledUnit(Unit* victim) override
    {
        if (victim->GetTypeId() == TYPEID_PLAYER && roll_chance_i(50))
            TalkPairToRaid(SAY_SLAY_FIRST + urand(0, 2) * 2);
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
        TalkPairToRaid(SAY_DEATH);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        ReportTasteTheRainbow();
        CleanupEncounter();
    }

    void EnterEvadeMode(EvadeReason /*why*/) override
    {
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        CleanupEncounter();
        _EnterEvadeMode();
        summons.DespawnAll();
        me->GetMotionMaster()->MoveTargetedHome();
        _DespawnAtEvade();
    }

    void SetData(uint32 type, uint32 data) override
    {
        switch (type)
        {
            case DATA_SPAWN_GLOBULE_SET: // from the Color Combination spell script
                SpawnGlobuleSet(data);
                break;
            case DATA_GLOBULE_CONSUMED:  // from the globule AI, data = color
                if (data < MAX_COLORS)
                    DoCastSelf(GlobuleData[data].BuffSpell, true);
                OnGlobuleResolved();
                break;
            default:
                break;
        }
    }

    void DoAction(int32 action) override
    {
        if (action != ACTION_GLOBULE_DIED)
            return;

        // Fusing Vapors: the moment one globule dies, the survivors become immune
        for (ObjectGuid guid : summons)
            if (Creature* globule = ObjectAccessor::GetCreature(*me, guid))
                if (globule->IsAlive() && IsGlobuleEntry(globule->GetEntry()))
                    globule->AI()->DoAction(ACTION_FUSING_VAPORS_IMMUNITY);

        OnGlobuleResolved();
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);

        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_VOID_BOLT:
                    DoCastVictim(SPELL_VOID_BOLT);
                    if (me->HasAura(SPELL_GLOWING_BLOOD_OF_SHUMA))
                    {
                        DoCastAOE(SPELL_VOID_BOLT_AOE, true);
                        events.Repeat(3s);
                    }
                    else
                        events.Repeat(6s);
                    break;
                case EVENT_COLOR_COMBINATION:
                    StartColorCombination();
                    break;
                case EVENT_SEARING_BLOOD:
                    if (!me->HasAura(SPELL_CRIMSON_BLOOD_OF_SHUMA))
                        break;
                    DoCastAOE(IsLFR(instance) ? SPELL_SEARING_BLOOD_LFR : SPELL_SEARING_BLOOD);
                    events.Repeat(me->HasAura(SPELL_GLOWING_BLOOD_OF_SHUMA) ? 3500ms : 6s);
                    break;
                case EVENT_DIGESTIVE_ACID:
                    if (!me->HasAura(SPELL_ACIDIC_BLOOD_OF_SHUMA))
                        break;
                    // The damage spell carries the 4 yard splash around its target;
                    // the spell script trims the splash and values on Raid Finder
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 150.0f, true))
                        DoCast(target, SPELL_DIGESTIVE_ACID);
                    events.Repeat(me->HasAura(SPELL_GLOWING_BLOOD_OF_SHUMA) ? 3500ms : 8300ms);
                    break;
                case EVENT_CORRUPTED_MINIONS:
                    if (!me->HasAura(SPELL_BLACK_BLOOD_OF_SHUMA))
                        break;
                    DoCastSelf(SPELL_CORRUPTED_MINIONS, true);
                    // Glowing Blood: a second wave follows
                    if (_minionWaves && --_minionWaves)
                        events.Repeat(15s);
                    break;
                case EVENT_SUMMON_MANA_VOID:
                    if (!me->HasAura(SPELL_COBALT_BLOOD_OF_SHUMA))
                        break;
                    DoCastSelf(SPELL_SUMMON_MANA_VOID, true);
                    // Glowing Blood: a second Mana Void follows
                    if (_manaVoids && --_manaVoids)
                        events.Repeat(12s);
                    break;
                case EVENT_BERSERK:
                    DoCastSelf(SPELL_BERSERK, true);
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
    static bool IsGlobuleEntry(uint32 entry)
    {
        for (GlobuleInfo const& info : GlobuleData)
            if (info.Entry == entry)
                return true;
        return false;
    }

    // Yells in Shath'Yar, then whispers the translation to every raid member
    void TalkPairToRaid(uint32 yellGroup)
    {
        Talk(yellGroup);
        _scheduler.Schedule(3s, [this, yellGroup](TaskContext /*context*/)
        {
            for (MapReference const& ref : me->GetMap()->GetPlayers())
                if (Player* player = ref.GetSource())
                    Talk(yellGroup + 1, player);
        });
    }

    void StartColorCombination()
    {
        // The boss turns passive at the room center until the set resolves;
        // Void Bolt stays quiet for 42 seconds (retail timing)
        me->AttackStop();
        me->SetReactState(REACT_PASSIVE);
        events.CancelEvent(EVENT_VOID_BOLT);
        events.ScheduleEvent(EVENT_VOID_BOLT, 42s);
        events.ScheduleEvent(EVENT_COLOR_COMBINATION, IsHeroicYorsahj(me->GetMap()) ? 75s : 90s);

        TalkPairToRaid(SAY_SUMMON_FIRST + urand(0, 2) * 2);

        int32 combination;
        do
            combination = irand(0, int32(std::size(Combinations)) - 1);
        while (combination == _lastCombination);
        _lastCombination = combination;

        // The spell's script effect reports back through DATA_SPAWN_GLOBULE_SET
        DoCastSelf(Combinations[combination].SpellId);
    }

    void SpawnGlobuleSet(uint32 combinationSpellId)
    {
        auto itr = std::find_if(std::begin(Combinations), std::end(Combinations),
            [combinationSpellId](ColorCombination const& combination) { return combination.SpellId == combinationSpellId; });
        if (itr == std::end(Combinations))
            return;

        bool const heroic = IsHeroicYorsahj(me->GetMap());
        _pendingGlobules = 0;

        auto spawnGlobule = [this](uint8 color)
        {
            Position pos = GlobuleData[color].SpawnPos;
            pos.SetOrientation(pos.GetAngle(ConsumptionCenter));
            if (Creature* globule = me->SummonCreature(GlobuleData[color].Entry, pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5s))
            {
                globule->AI()->SetData(DATA_GLOBULE_COLOR, color);
                ++_pendingGlobules;
            }
        };

        for (uint8 color : itr->Colors)
            spawnGlobule(color);
        if (heroic)
            spawnGlobule(itr->HeroicExtra);
    }

    void OnGlobuleResolved()
    {
        if (!_pendingGlobules || --_pendingGlobules)
            return;

        // Set resolved - resume combat with whatever bloods were absorbed
        me->SetReactState(REACT_AGGRESSIVE);
        if (Unit* victim = me->GetVictim())
            AttackStart(victim);

        bool const glowing = me->HasAura(SPELL_GLOWING_BLOOD_OF_SHUMA);

        if (me->HasAura(SPELL_CRIMSON_BLOOD_OF_SHUMA))
            events.ScheduleEvent(EVENT_SEARING_BLOOD, 5s);

        if (me->HasAura(SPELL_ACIDIC_BLOOD_OF_SHUMA))
            events.ScheduleEvent(EVENT_DIGESTIVE_ACID, 8s);

        if (me->HasAura(SPELL_BLACK_BLOOD_OF_SHUMA))
        {
            _minionWaves = glowing ? 2 : 1;
            events.ScheduleEvent(EVENT_CORRUPTED_MINIONS, 1s);
        }

        if (me->HasAura(SPELL_COBALT_BLOOD_OF_SHUMA))
        {
            _manaVoids = glowing ? 2 : 1;
            events.ScheduleEvent(EVENT_SUMMON_MANA_VOID, 3s);
        }

        if (me->HasAura(SPELL_SHADOWED_BLOOD_OF_SHUMA))
        {
            // Deep Corruption covers the raid for the entire 60s window (the
            // debuff lasts 25s and is reapplied, catching battle rezzes too)
            DoCastAOE(SPELL_DEEP_CORRUPTION, true);
            _scheduler.Schedule(20s, [this](TaskContext context)
            {
                if (!me->HasAura(SPELL_SHADOWED_BLOOD_OF_SHUMA))
                    return;
                DoCastAOE(SPELL_DEEP_CORRUPTION, true);
                context.Repeat(20s);
            });
        }
    }

    // Taste the Rainbow!: report the color pairs empowering the boss at the kill
    void ReportTasteTheRainbow()
    {
        auto empowered = [this](uint8 color) { return me->HasAura(GlobuleData[color].BuffSpell); };

        uint32 mask = 0;
        if (empowered(COLOR_BLACK) && empowered(COLOR_YELLOW))
            mask |= RAINBOW_BIT_BLACK_YELLOW;
        if (empowered(COLOR_RED) && empowered(COLOR_GREEN))
            mask |= RAINBOW_BIT_RED_GREEN;
        if (empowered(COLOR_BLACK) && empowered(COLOR_BLUE))
            mask |= RAINBOW_BIT_BLACK_BLUE;
        if (empowered(COLOR_PURPLE) && empowered(COLOR_YELLOW))
            mask |= RAINBOW_BIT_PURPLE_YELLOW;

        if (mask)
            instance->SetData(DATA_YORSAHJ_TASTE_THE_RAINBOW, mask);
    }

    void CleanupEncounter()
    {
        _scheduler.CancelAll();
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_DEEP_CORRUPTION);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_DEEP_CORRUPTION_COUNTER);
    }

    TaskScheduler _scheduler;
    uint8 _pendingGlobules = 0;
    uint8 _minionWaves = 0;
    uint8 _manaVoids = 0;
    int32 _lastCombination = -1;
    bool _introDone = false;
};

struct npc_yorsahj_globule : public ScriptedAI
{
    npc_yorsahj_globule(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript())
    {
        SetCombatMovement(false);
        me->SetReactState(REACT_PASSIVE);
    }

    void JustAppeared() override
    {
        ApplyLFRHealth(me, _instance, NPC_YORSAHJ_GLOBULE_LFR_STATS);
        DoCastSelf(SPELL_SPAWNING_POOL, true);

        // Dormant in its pool for 7 seconds, then attackable and moving
        _scheduler.Schedule(7s, [this](TaskContext /*context*/)
        {
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            DoZoneInCombat();

            float const speed = std::max(me->GetExactDist(ConsumptionCenter), 1.0f) / GlobuleTravelSeconds;
            me->GetMotionMaster()->MovePoint(POINT_CONSUME, ConsumptionCenter, true, speed);
        });
    }

    void SetData(uint32 type, uint32 data) override
    {
        if (type == DATA_GLOBULE_COLOR)
            _color = data;
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage) override
    {
        if (_consumed)
        {
            damage = 0;
            return;
        }

        // Fusing Vapors: dropping below half heals every other living globule
        if (!_vaporsFired && me->HealthBelowPctDamaged(50, damage))
        {
            _vaporsFired = true;
            DoCastAOE(SPELL_FUSING_VAPORS_HEAL, true);
        }
    }

    void DoAction(int32 action) override
    {
        if (action != ACTION_FUSING_VAPORS_IMMUNITY || _consumed || !me->IsAlive())
            return;

        DoCastSelf(SPELL_FUSING_VAPORS_IMMUNITY, true);
        me->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_ALL, true);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (_consumed)
            return;

        if (Creature* yorsahj = _instance->GetCreature(DATA_YORSAHJ_THE_UNSLEEPING))
            if (yorsahj->IsAlive() && yorsahj->IsAIEnabled())
                yorsahj->AI()->DoAction(ACTION_GLOBULE_DIED);
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != POINT_MOTION_TYPE || pointId != POINT_CONSUME || _consumed || !me->IsAlive())
            return;

        // Consumed: Yor'sahj absorbs this globule's blood
        _consumed = true;
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

        if (Creature* yorsahj = _instance->GetCreature(DATA_YORSAHJ_THE_UNSLEEPING))
            if (yorsahj->IsAlive() && yorsahj->IsAIEnabled())
                yorsahj->AI()->SetData(DATA_GLOBULE_CONSUMED, _color);

        me->DespawnOrUnsummon(500ms);
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

private:
    InstanceScript* _instance;
    TaskScheduler _scheduler;
    uint32 _color = MAX_COLORS;
    bool _vaporsFired = false;
    bool _consumed = false;
};

struct npc_yorsahj_forgotten_one : public ScriptedAI
{
    npc_yorsahj_forgotten_one(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript()) { }

    void JustAppeared() override
    {
        ApplyLFRHealth(me, _instance, NPC_YORSAHJ_FORGOTTEN_ONE_LFR_STATS);
        DoZoneInCombat();

        // Short grace window, then fixate on a random raid member
        _scheduler.Schedule(1500ms, [this](TaskContext /*context*/)
        {
            Fixate();
        });

        _scheduler.Schedule(4s, 7s, [this](TaskContext context)
        {
            if (Unit* victim = me->GetVictim())
                DoCast(victim, SPELL_PSYCHIC_SLICE);
            context.Repeat(6s, 9s);
        });
    }

    void DamageDealt(Unit* /*victim*/, uint32& damage, DamageEffectType /*damageType*/) override
    {
        ApplyLFRDamageReduction(_instance, damage);
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);

        if (!UpdateVictim())
            return;

        // Fixation target died - pick a new one
        if (Unit* victim = me->GetVictim())
            if (_fixateGUID && victim->GetGUID() != _fixateGUID)
                Fixate();

        DoMeleeAttackIfReady();
    }

private:
    void Fixate()
    {
        Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 150.0f, true);
        if (!target)
            return;

        _fixateGUID = target->GetGUID();
        ResetThreatList();
        AddThreat(target, 50000000.0f);
        AttackStart(target);
    }

    InstanceScript* _instance;
    TaskScheduler _scheduler;
    ObjectGuid _fixateGUID;
};

struct npc_yorsahj_mana_void : public ScriptedAI
{
    npc_yorsahj_mana_void(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript())
    {
        SetCombatMovement(false);
        me->SetReactState(REACT_PASSIVE);
    }

    void JustAppeared() override
    {
        ApplyLFRHealth(me, _instance, NPC_YORSAHJ_MANA_VOID_LFR_STATS);
        DoZoneInCombat();
        DoCastAOE(SPELL_MANA_VOID_DRAIN);
    }

    void JustDied(Unit* /*killer*/) override
    {
        // Mana Diffusion: the leeched mana is released to nearby mana users
        // (30 yards; the Raid Finder variant is percent-based and room-wide)
        me->CastSpell(me, IsLFR(_instance) ? SPELL_MANA_DIFFUSION_LFR : SPELL_MANA_DIFFUSION, true);
    }

    void UpdateAI(uint32 /*diff*/) override { }

private:
    InstanceScript* _instance;
};

// 105420, 105435, 105436, 105437, 105439, 105440 - Color Combination
class spell_yorsahj_color_combination : public SpellScript
{
    void HandleAfterCast()
    {
        Creature* caster = GetCaster()->ToCreature();
        if (!caster || !caster->IsAIEnabled())
            return;

        caster->AI()->SetData(DATA_SPAWN_GLOBULE_SET, GetSpellInfo()->Id);
    }

    void Register() override
    {
        AfterCast.Register(&spell_yorsahj_color_combination::HandleAfterCast);
    }
};

// 105171 - Deep Corruption
class spell_yorsahj_deep_corruption : public AuraScript
{
    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetHealInfo() && eventInfo.GetHealInfo()->GetHeal();
    }

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& /*eventInfo*/)
    {
        // 103628 has no stacking data in the DBC - count the heals ourselves
        PreventDefaultAction();

        Unit* target = GetTarget();
        uint8 stacks = 0;
        if (Aura const* counter = target->GetAura(SPELL_DEEP_CORRUPTION_COUNTER))
            stacks = counter->GetStackAmount();

        if (stacks >= 4) // this heal is the fifth - detonate into the raid
        {
            target->RemoveAurasDueToSpell(SPELL_DEEP_CORRUPTION_COUNTER);
            target->CastSpell(target, SPELL_DEEP_CORRUPTION_DAMAGE, true);
        }
        else
            target->CastSpell(target, SPELL_DEEP_CORRUPTION_COUNTER,
                CastSpellExtraArgs(TRIGGERED_FULL_MASK).AddSpellMod(SPELLVALUE_AURA_STACK, stacks + 1));
    }

    void Register() override
    {
        DoCheckProc.Register(&spell_yorsahj_deep_corruption::CheckProc);
        OnEffectProc.Register(&spell_yorsahj_deep_corruption::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

// 105173, 108347, 108348, 108349 - Deep Corruption (detonation, self-cast by the player)
class spell_yorsahj_deep_corruption_explosion : public SpellScript
{
    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        uint32 damage = GetHitDamage();
        ApplyLFRDamageReduction(GetCaster()->GetInstanceScript(), damage);
        SetHitDamage(damage);
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_yorsahj_deep_corruption_explosion::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 105033, 108356, 108357, 108358, 108218, 108363 - Searing Blood
class spell_yorsahj_searing_blood : public SpellScript
{
    void FilterTargets(std::list<WorldObject*>& targets)
    {
        Unit* caster = GetCaster();
        InstanceScript const* instance = caster->GetInstanceScript();

        // Strikes the farthest players: 8 on 25 player, 3 on 10 player and Raid Finder
        uint32 const maxTargets = (!IsLFR(instance) && caster->GetMap()->Is25ManRaid()) ? 8 : 3;

        targets.sort([caster](WorldObject* left, WorldObject* right)
        {
            return caster->GetExactDist2d(left) > caster->GetExactDist2d(right);
        });

        if (targets.size() > maxTargets)
            targets.resize(maxTargets);
    }

    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        // Damage grows with the target's distance from the boss
        float const dist = GetCaster()->GetExactDist2d(GetHitUnit());
        SetHitDamage(int32(GetHitDamage() * (1.0f + dist * SearingBloodDamagePerYard)));
    }

    void Register() override
    {
        OnObjectAreaTargetSelect.Register(&spell_yorsahj_searing_blood::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        OnObjectAreaTargetSelect.Register(&spell_yorsahj_searing_blood::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
        OnEffectHitTarget.Register(&spell_yorsahj_searing_blood::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 105573, 108350, 108351, 108352 - Digestive Acid
class spell_yorsahj_digestive_acid : public SpellScript
{
    void FilterTargets(std::list<WorldObject*>& targets)
    {
        // Raid Finder: no splash - only the targeted player is hit
        if (!IsLFR(GetCaster()->GetInstanceScript()))
            return;

        WorldObject* primary = GetExplTargetWorldObject();
        targets.clear();
        if (primary)
            targets.push_back(primary);
    }

    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        // Raid Finder uses the dedicated spell's (108419) lower values
        if (!IsLFR(GetCaster()->GetInstanceScript()))
            return;

        if (SpellInfo const* lfrInfo = sSpellMgr->GetSpellInfo(SPELL_DIGESTIVE_ACID_LFR))
            SetHitDamage(lfrInfo->Effects[EFFECT_0].CalcValue(GetCaster()));
    }

    void Register() override
    {
        OnObjectAreaTargetSelect.Register(&spell_yorsahj_digestive_acid::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
        OnEffectHitTarget.Register(&spell_yorsahj_digestive_acid::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 103635, 108233 - Fusing Vapors (heal pulse)
class spell_yorsahj_fusing_vapors : public SpellScript
{
    void FilterTargets(std::list<WorldObject*>& targets)
    {
        // Heals only the other living globules
        Unit* caster = GetCaster();
        targets.remove_if([caster](WorldObject* target)
        {
            Creature* creature = target->ToCreature();
            if (!creature || creature == caster || !creature->IsAlive())
                return true;

            switch (creature->GetEntry())
            {
                case NPC_ACIDIC_GLOBULE:
                case NPC_SHADOWED_GLOBULE:
                case NPC_GLOWING_GLOBULE:
                case NPC_CRIMSON_GLOBULE:
                case NPC_COBALT_GLOBULE:
                case NPC_DARK_GLOBULE:
                    return false;
                default:
                    return true;
            }
        });
    }

    void Register() override
    {
        OnObjectAreaTargetSelect.Register(&spell_yorsahj_fusing_vapors::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
    }
};

// 105539 - Mana Diffusion
class spell_yorsahj_mana_diffusion : public SpellScript
{
    void HandleEnergize(SpellEffIndex /*effIndex*/)
    {
        // The DBC value is a placeholder - restore the full mana pool
        if (Unit* target = GetHitUnit())
            SetEffectValue(int32(target->GetMaxPower(POWER_MANA)));
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_yorsahj_mana_diffusion::HandleEnergize, EFFECT_0, SPELL_EFFECT_ENERGIZE);
    }
};

// 104849, 108383, 108384, 108385, 105416, 109549, 109550, 109551 - Void Bolt
class spell_yorsahj_void_bolt_lfr : public SpellScript
{
    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        uint32 damage = GetHitDamage();
        ApplyLFRDamageReduction(GetCaster()->GetInstanceScript(), damage);
        SetHitDamage(damage);
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_yorsahj_void_bolt_lfr::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};
}

void AddSC_boss_yorsahj()
{
    using namespace DragonSoul;
    using namespace DragonSoul::Yorsahj;

    RegisterDragonSoulCreatureAI(boss_yorsahj);
    RegisterDragonSoulCreatureAI(npc_yorsahj_globule);
    RegisterDragonSoulCreatureAI(npc_yorsahj_forgotten_one);
    RegisterDragonSoulCreatureAI(npc_yorsahj_mana_void);

    RegisterSpellScript(spell_yorsahj_color_combination);
    RegisterSpellScript(spell_yorsahj_deep_corruption);
    RegisterSpellScript(spell_yorsahj_deep_corruption_explosion);
    RegisterSpellScript(spell_yorsahj_searing_blood);
    RegisterSpellScript(spell_yorsahj_digestive_acid);
    RegisterSpellScript(spell_yorsahj_fusing_vapors);
    RegisterSpellScript(spell_yorsahj_mana_diffusion);
    RegisterSpellScript(spell_yorsahj_void_bolt_lfr);
}
