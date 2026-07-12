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

#include "end_time.h"
#include "Containers.h"
#include "InstanceScript.h"
#include "LFGMgr.h"
#include "Map.h"
#include "MapRefManager.h"
#include "MotionMaster.h"
#include "MoveSplineInit.h"
#include "ObjectAccessor.h"
#include "PassiveAI.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "TemporarySummon.h"

namespace EndTime::EchoOfTyrande
{
enum Spells
{
    // Echo of Tyrande
    SPELL_IN_SHADOW                     = 101841, // -90% damage taken shroud (creature_template_addon on her and all gauntlet adds)
    SPELL_MOONBOLT                      = 102193,
    SPELL_STARDUST                      = 102173,
    SPELL_MOONLANCE                     = 102151,
    SPELL_MOONLANCE_SPLIT               = 102152,
    SPELL_EYES_OF_THE_GODDESS           = 102606,
    SPELL_EYES_OF_THE_GODDESS_INSTANT   = 102608,
    SPELL_LUNAR_GUIDANCE                = 102472,
    SPELL_TEARS_OF_ELUNE                = 102241,
    SPELL_TEARS_OF_ELUNE_SCRIPT         = 102242,
    SPELL_TEARS_OF_ELUNE_MISSILE        = 102243,
    SPELL_DARK_MOONLIGHT                = 102414,
    SPELL_ACHIEVEMENT_TRACKER           = 102491, // proc on damage taken -> 102539 -> map event 29235
    SPELL_ACHIEVEMENT_CREDIT            = 102542,

    // Moonlance
    SPELL_MOONLANCE_VISUAL              = 102150, // periodic contact pulse (102149)

    // Eye of Elune
    SPELL_PIERCING_GAZE_OF_ELUNE        = 102182,

    // Pool of Moonlight
    SPELL_MOONLIT                       = 101946, // periodic, removes In Shadow from adds in the light (101842)

    // Gauntlet adds
    SPELL_DRAIN_POOL                    = 102002  // shrinks the pool
};

enum Events
{
    // Echo of Tyrande
    EVENT_MOONBOLT = 1,
    EVENT_STARDUST,
    EVENT_MOONLANCE,
    EVENT_EYES_OF_THE_GODDESS,
    EVENT_LUNAR_GUIDANCE,
    EVENT_TEARS_OF_ELUNE
};

enum Actions
{
    // Echo of Tyrande (instance wiring uses raw value 1)
    ACTION_START_GAUNTLET   = 1,
    ACTION_GAUNTLET_RESET   = 2,

    // Pool of Moonlight
    ACTION_POOL_FADE        = 1,

    // Gauntlet adds
    ACTION_ACTIVATE         = 1
};

enum Data
{
    DATA_POOL_INDEX         = 1,
    DATA_GAUNTLET_ADD_DIED  = 1,
    DATA_IS_DORMANT         = 1
};

enum Texts
{
    // Echo of Tyrande
    SAY_GAUNTLET_START      = 0,
    SAY_POOL_FADE_FIRST     = 1, // groups 1-4 - one line per faded pool
    EMOTE_DARK_MOONLIGHT    = 5,
    SAY_AGGRO               = 6,
    SAY_LUNAR_GUIDANCE_1    = 7,
    SAY_LUNAR_GUIDANCE_2    = 8,
    SAY_MOONLANCE           = 9,
    SAY_EYES_OF_THE_GODDESS = 10,
    SAY_TEARS_OF_ELUNE      = 11,
    SAY_DEATH               = 12,
    EMOTE_LUNAR_GUIDANCE    = 13,
    EMOTE_TEARS_OF_ELUNE    = 14,
    SAY_SLAY                = 15,

    // Pool of Moonlight (group = pool index, 5 = fade)
    EMOTE_POOL_FADES        = 5
};

enum MovePoints
{
    POINT_LANCE = 1
};

enum Misc
{
    TASK_GROUP_GAUNTLET     = 1,
    POOL_COUNT              = 5,
    MAX_WAVES_PER_POOL      = 3
};

Position const PoolPositions[POOL_COUNT] =
{
    { 2903.263f,  63.17882f,   3.245f },
    { 2862.8308f, 131.4618f,   3.184f },
    { 2756.5713f, 129.97049f,  5.582f },
    { 2695.441f,  28.796875f,  1.232f },
    { 2792.8176f, 1.9392362f,  2.463f }
};

struct GauntletWave
{
    Seconds Delay;
    uint8 Sentinels;
    uint8 Nightsabers;
    uint8 Huntresses;
};

GauntletWave const GauntletWaves[POOL_COUNT][MAX_WAVES_PER_POOL] =
{
    { {  0s, 0, 3, 0 }, {  2s, 0, 5, 0 }, { 25s, 0, 5, 0 } },
    { {  0s, 0, 2, 0 }, { 13s, 2, 3, 0 }, { 21s, 0, 1, 0 } },
    { {  0s, 2, 2, 0 }, {  2s, 0, 1, 0 }, { 11s, 5, 0, 0 } },
    { {  0s, 1, 0, 0 }, {  4s, 4, 0, 0 }, { 12s, 3, 0, 2 } },
    { { 14s, 0, 0, 4 }, { 21s, 0, 0, 1 }, { 29s, 0, 0, 3 } }
};

uint32 const NightsaberEntries[]    = { NPC_TIME_TWISTED_NIGHTSABER_1, NPC_TIME_TWISTED_NIGHTSABER_2, NPC_TIME_TWISTED_NIGHTSABER_3 };
constexpr float PoolLightRadius     = 13.f;
constexpr float EyeCircleRadius     = 34.6f;
constexpr float EyeSpeed            = 7.46f;
constexpr float LanceSpeed          = 7.5f;

struct boss_echo_of_tyrande : public BossAI
{
    boss_echo_of_tyrande(Creature* creature) : BossAI(creature, DATA_ECHO_OF_TYRANDE),
        _currentPool(0), _dispatchedWaves(0), _aliveWaveAdds(0), _lunarGuidanceCount(0), _tearsOfElune(false), _gauntletResetPending(false) { }

    void JustAppeared() override
    {
        me->SetControlled(true, UNIT_STATE_ROOT);
        if (instance->GetData(DATA_SHADOW_GAUNTLET) == DONE)
            MakeAttackable();
    }

    void JustEngagedWith(Unit* who) override
    {
        BossAI::JustEngagedWith(who);
        Talk(SAY_AGGRO);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 1);

        if (instance->GetData(DATA_MOON_GUARD_ELIGIBLE))
            DoCastAOE(SPELL_ACHIEVEMENT_CREDIT, true);
        else
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_ACHIEVEMENT_TRACKER);

        events.ScheduleEvent(EVENT_MOONBOLT, 1ms);
        events.ScheduleEvent(EVENT_STARDUST, 7s);
        events.ScheduleEvent(EVENT_MOONLANCE, 16s + 500ms);
        events.ScheduleEvent(EVENT_EYES_OF_THE_GODDESS, 30s);
    }

    void EnterEvadeMode(EvadeReason why) override
    {
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        summons.DespawnAll();
        ScriptedAI::EnterEvadeMode(why);
    }

    void Reset() override
    {
        _Reset();
        _lunarGuidanceCount = 0;
        _tearsOfElune = false;
        me->SetControlled(true, UNIT_STATE_ROOT);
    }

    void JustDied(Unit* killer) override
    {
        BossAI::JustDied(killer);
        Talk(SAY_DEATH, killer);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        me->RemoveAllDynObjects();
    }

    void KilledUnit(Unit* victim) override
    {
        if (victim->IsPlayer())
            Talk(SAY_SLAY, victim);
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage) override
    {
        if (_lunarGuidanceCount == 0 && me->HealthBelowPctDamaged(80, damage))
        {
            _lunarGuidanceCount = 1;
            events.ScheduleEvent(EVENT_LUNAR_GUIDANCE, 1ms);
        }
        else if (_lunarGuidanceCount == 1 && me->HealthBelowPctDamaged(55, damage))
        {
            _lunarGuidanceCount = 2;
            events.ScheduleEvent(EVENT_LUNAR_GUIDANCE, 1ms);
        }

        if (!_tearsOfElune && me->HealthBelowPctDamaged(30, damage))
        {
            _tearsOfElune = true;
            events.ScheduleEvent(EVENT_TEARS_OF_ELUNE, 1ms);
        }
    }

    void DoAction(int32 action) override
    {
        switch (action)
        {
            case ACTION_START_GAUNTLET:
                if (instance->GetData(DATA_SHADOW_GAUNTLET) != NOT_STARTED || me->IsInCombat())
                    break;

                instance->SetData(DATA_SHADOW_GAUNTLET, IN_PROGRESS);
                instance->SetData(DATA_MOON_GUARD_ELIGIBLE, 1);
                Talk(SAY_GAUNTLET_START);
                DoCastAOE(SPELL_ACHIEVEMENT_TRACKER, true);
                SummonPool(0);
                break;
            case ACTION_GAUNTLET_RESET:
                if (instance->GetData(DATA_SHADOW_GAUNTLET) != IN_PROGRESS || _gauntletResetPending)
                    break;

                _gauntletResetPending = true;
                scheduler.CancelGroup(TASK_GROUP_GAUNTLET);
                scheduler.Schedule(5s, [this](TaskContext)
                {
                    if (Creature* pool = ObjectAccessor::GetCreature(*me, _currentPoolGuid))
                        pool->DespawnOrUnsummon();

                    instance->SetData(DATA_SHADOW_GAUNTLET, NOT_STARTED);
                    _gauntletResetPending = false;
                });
                break;
            default:
                break;
        }
    }

    void SetData(uint32 type, uint32 /*value*/) override
    {
        if (type != DATA_GAUNTLET_ADD_DIED)
            return;

        if (_aliveWaveAdds)
            --_aliveWaveAdds;

        CheckPoolCleared();
    }

    void UpdateAI(uint32 diff) override
    {
        scheduler.Update(diff);

        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_MOONBOLT:
                    DoCastVictim(SPELL_MOONBOLT);
                    events.Repeat(2s + 500ms);
                    break;
                case EVENT_STARDUST:
                    DoCastAOE(SPELL_STARDUST);
                    events.Repeat(15s, 18s);
                    break;
                case EVENT_MOONLANCE:
                    if (_firstMoonlance)
                    {
                        _firstMoonlance = false;
                        Talk(SAY_MOONLANCE);
                    }
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 60.f, true))
                        DoCast(target, SPELL_MOONLANCE);
                    events.Repeat(10s, 14s);
                    break;
                case EVENT_EYES_OF_THE_GODDESS:
                    if (_firstEyes)
                    {
                        _firstEyes = false;
                        Talk(SAY_EYES_OF_THE_GODDESS);
                    }
                    DoCastSelf(SPELL_EYES_OF_THE_GODDESS_INSTANT, true);
                    DoCastSelf(SPELL_EYES_OF_THE_GODDESS);
                    events.Repeat(21s, 26s);
                    break;
                case EVENT_LUNAR_GUIDANCE:
                    Talk(_lunarGuidanceCount == 1 ? SAY_LUNAR_GUIDANCE_1 : SAY_LUNAR_GUIDANCE_2);
                    DoCastSelf(SPELL_LUNAR_GUIDANCE);
                    scheduler.Schedule(5s, [this](TaskContext)
                    {
                        Talk(EMOTE_LUNAR_GUIDANCE);
                    });
                    break;
                case EVENT_TEARS_OF_ELUNE:
                    Talk(SAY_TEARS_OF_ELUNE);
                    DoCastSelf(SPELL_TEARS_OF_ELUNE);
                    scheduler.Schedule(6s, [this](TaskContext)
                    {
                        Talk(EMOTE_TEARS_OF_ELUNE);
                    });
                    break;
                default:
                    break;
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
        }
    }

private:
    void MakeAttackable()
    {
        me->RemoveAurasDueToSpell(SPELL_IN_SHADOW);
        me->SetImmuneToPC(false);
        me->SetImmuneToNPC(false);
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    }

    void SummonPool(uint8 index)
    {
        _currentPool = index;
        _dispatchedWaves = 0;
        _aliveWaveAdds = 0;

        Creature* pool = me->SummonCreature(NPC_POOL_OF_MOONLIGHT, PoolPositions[index], TEMPSUMMON_MANUAL_DESPAWN);
        if (!pool)
            return;

        _currentPoolGuid = pool->GetGUID();
        if (pool->IsAIEnabled())
            pool->AI()->SetData(DATA_POOL_INDEX, index);

        for (uint8 i = 0; i < MAX_WAVES_PER_POOL; ++i)
        {
            GauntletWave const& wave = GauntletWaves[index][i];
            scheduler.Schedule(wave.Delay + 6s, TASK_GROUP_GAUNTLET, [this, index, i](TaskContext)
            {
                ActivateWave(index, i);
            });
        }
    }

    void ActivateWave(uint8 poolIndex, uint8 waveIndex)
    {
        GauntletWave const& wave = GauntletWaves[poolIndex][waveIndex];
        ActivateAdds({ NPC_TIME_TWISTED_SENTINEL }, wave.Sentinels, PoolPositions[poolIndex]);
        ActivateAdds({ NightsaberEntries[0], NightsaberEntries[1], NightsaberEntries[2] }, wave.Nightsabers, PoolPositions[poolIndex]);
        ActivateAdds({ NPC_TIME_TWISTED_HUNTRESS }, wave.Huntresses, PoolPositions[poolIndex]);

        ++_dispatchedWaves;
        CheckPoolCleared();
    }

    void ActivateAdds(std::initializer_list<uint32> entries, uint8 count, Position const& around)
    {
        if (!count)
            return;

        std::list<Creature*> candidates;
        for (uint32 entry : entries)
        {
            std::list<Creature*> found;
            me->GetCreatureListWithEntryInGrid(found, entry, 250.f);
            candidates.splice(candidates.end(), found);
        }

        candidates.remove_if([](Creature const* creature)
        {
            return !creature->IsAlive() || !creature->IsAIEnabled() || !creature->AI()->GetData(DATA_IS_DORMANT);
        });

        candidates.sort([&](Creature const* first, Creature const* second)
        {
            return first->GetExactDist2d(around) < second->GetExactDist2d(around);
        });

        for (Creature* add : candidates)
        {
            if (!count)
                break;

            add->AI()->DoAction(ACTION_ACTIVATE);
            ++_aliveWaveAdds;
            --count;
        }
    }

    void CheckPoolCleared()
    {
        if (instance->GetData(DATA_SHADOW_GAUNTLET) != IN_PROGRESS || _gauntletResetPending)
            return;

        if (_dispatchedWaves < MAX_WAVES_PER_POOL || _aliveWaveAdds)
            return;

        if (Creature* pool = ObjectAccessor::GetCreature(*me, _currentPoolGuid))
            if (pool->IsAIEnabled())
                pool->AI()->DoAction(ACTION_POOL_FADE);

        if (_currentPool + 1 < POOL_COUNT)
        {
            Talk(SAY_POOL_FADE_FIRST + _currentPool);
            SummonPool(_currentPool + 1);
        }
        else
            CompleteGauntlet();
    }

    void CompleteGauntlet()
    {
        instance->SetData(DATA_SHADOW_GAUNTLET, DONE);
        Talk(EMOTE_DARK_MOONLIGHT);
        MakeAttackable();
        me->CastSpell(me->GetPosition(), SPELL_DARK_MOONLIGHT, true);
    }

    uint8 _currentPool;
    uint8 _dispatchedWaves;
    uint8 _aliveWaveAdds;
    uint8 _lunarGuidanceCount;
    bool _tearsOfElune;
    bool _gauntletResetPending;
    bool _firstMoonlance = true;
    bool _firstEyes = true;
    ObjectGuid _currentPoolGuid;
};

struct npc_echo_of_tyrande_pool_of_moonlight : public NullCreatureAI
{
    npc_echo_of_tyrande_pool_of_moonlight(Creature* creature) : NullCreatureAI(creature), _poolIndex(0) { }

    void JustAppeared() override
    {
        // The pool spawns hidden and lights up a few seconds later
        me->SetVisible(false);
        _events.ScheduleEvent(1, 6s);
    }

    void SetData(uint32 type, uint32 value) override
    {
        if (type == DATA_POOL_INDEX)
            _poolIndex = uint8(value);
    }

    void DoAction(int32 action) override
    {
        if (action == ACTION_POOL_FADE)
        {
            Talk(EMOTE_POOL_FADES);
            me->DespawnOrUnsummon(500ms);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case 1:
                    me->SetVisible(true);
                    Talk(_poolIndex);
                    DoCastSelf(SPELL_MOONLIT, true);
                    break;
                default:
                    break;
            }
        }
    }

private:
    EventMap _events;
    uint8 _poolIndex;
};

struct npc_echo_of_tyrande_gauntlet_add : public ScriptedAI
{
    npc_echo_of_tyrande_gauntlet_add(Creature* creature) : ScriptedAI(creature), _dormant(true), _drainedPool(false), _shroudTimer(1000) { }

    void Reset() override
    {
        if (InstanceScript* instance = me->GetInstanceScript())
        {
            if (instance->GetData(DATA_SHADOW_GAUNTLET) == DONE)
            {
                me->DespawnOrUnsummon();
                return;
            }
        }

        _dormant = true;
        _drainedPool = false;
        me->SetReactState(REACT_PASSIVE);
        me->SetImmuneToPC(true);
        if (!me->HasAura(SPELL_IN_SHADOW))
            DoCastSelf(SPELL_IN_SHADOW, true);
    }

    uint32 GetData(uint32 type) const override
    {
        if (type == DATA_IS_DORMANT)
            return _dormant ? 1 : 0;

        return 0;
    }

    void DoAction(int32 action) override
    {
        if (action != ACTION_ACTIVATE || !_dormant)
            return;

        _dormant = false;
        me->SetImmuneToPC(false);
        me->SetReactState(REACT_AGGRESSIVE);
        DoCastSelf(SPELL_IN_SHADOW, true);
        DoZoneInCombat();
    }

    void EnterEvadeMode(EvadeReason why) override
    {
        // The party wiped - the whole gauntlet resets
        if (!_dormant)
            if (InstanceScript* instance = me->GetInstanceScript())
                if (Creature* tyrande = instance->GetCreature(DATA_ECHO_OF_TYRANDE))
                    if (tyrande->IsAIEnabled())
                        tyrande->AI()->DoAction(ACTION_GAUNTLET_RESET);

        ScriptedAI::EnterEvadeMode(why);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (_dormant)
            return;

        if (InstanceScript* instance = me->GetInstanceScript())
            if (Creature* tyrande = instance->GetCreature(DATA_ECHO_OF_TYRANDE))
                if (tyrande->IsAIEnabled())
                    tyrande->AI()->SetData(DATA_GAUNTLET_ADD_DIED, 1);
    }

    void UpdateAI(uint32 diff) override
    {
        UpdateShroud(diff);

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }

private:
    void UpdateShroud(uint32 diff)
    {
        if (_dormant)
            return;

        if (_shroudTimer > diff)
        {
            _shroudTimer -= diff;
            return;
        }

        _shroudTimer = 1000;

        Creature* pool = me->FindNearestCreature(NPC_POOL_OF_MOONLIGHT, PoolLightRadius);
        if (pool && pool->IsVisible())
        {
            // Drinking from the moonlight shrinks the pool. The Moonlit aura (101842) strips In Shadow while inside.
            if (!_drainedPool)
            {
                _drainedPool = true;
                DoCast(pool, SPELL_DRAIN_POOL, true);
            }
        }
        else if (!me->HasAura(SPELL_IN_SHADOW))
            DoCastSelf(SPELL_IN_SHADOW, true);
    }

    bool _dormant;
    bool _drainedPool;
    uint32 _shroudTimer;
};

struct npc_echo_of_tyrande_moonlance : public NullCreatureAI
{
    npc_echo_of_tyrande_moonlance(Creature* creature) : NullCreatureAI(creature) { }

    void IsSummonedBy(Unit* summoner) override
    {
        float orientation = summoner->GetOrientation();
        if (me->GetEntry() == NPC_MOONLANCE_SPLIT_LEFT)
            orientation = Position::NormalizeOrientation(orientation + float(M_PI_4));
        else if (me->GetEntry() == NPC_MOONLANCE_SPLIT_RIGHT)
            orientation = Position::NormalizeOrientation(orientation - float(M_PI_4));

        me->SetOrientation(orientation);
        DoCastSelf(SPELL_MOONLANCE_VISUAL, true);

        if (me->GetEntry() == NPC_MOONLANCE)
        {
            MoveForward(15.f);
            _events.ScheduleEvent(1, 1s + 770ms);
            me->DespawnOrUnsummon(5s);
        }
        else
        {
            MoveForward(40.f);
            me->DespawnOrUnsummon(7s + 200ms);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case 1:
                    DoCastSelf(SPELL_MOONLANCE_SPLIT, true);
                    break;
                default:
                    break;
            }
        }
    }

private:
    void MoveForward(float distance)
    {
        Position dest = me->GetPosition();
        dest.m_positionX += std::cos(me->GetOrientation()) * distance;
        dest.m_positionY += std::sin(me->GetOrientation()) * distance;

        Movement::MoveSplineInit init(me);
        init.MoveTo(dest.GetPositionX(), dest.GetPositionY(), dest.GetPositionZ(), false, true);
        init.SetVelocity(LanceSpeed);
        init.Launch();
    }

    EventMap _events;
};

struct npc_echo_of_tyrande_eye_of_elune : public NullCreatureAI
{
    npc_echo_of_tyrande_eye_of_elune(Creature* creature) : NullCreatureAI(creature) { }

    void IsSummonedBy(Unit* summoner) override
    {
        _center = summoner->GetPosition();
        _events.ScheduleEvent(1, 2s);
        _events.ScheduleEvent(2, 4s + 500ms);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case 1:
                    DoCastSelf(SPELL_PIERCING_GAZE_OF_ELUNE);
                    break;
                case 2:
                    StartOrbit();
                    break;
                default:
                    break;
            }
        }
    }

private:
    void StartOrbit()
    {
        bool clockwise = me->GetEntry() == NPC_EYE_OF_ELUNE_5;
        float startAngle = clockwise ? 1.43f : Position::NormalizeOrientation(1.43f + float(M_PI));
        float step = float(2 * M_PI) / 16.f * (clockwise ? -1.f : 1.f);

        Movement::PointsArray path;
        path.reserve(17);
        path.emplace_back(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
        for (uint8 i = 0; i < 16; ++i)
        {
            float angle = startAngle + step * i;
            path.emplace_back(_center.GetPositionX() + std::cos(angle) * EyeCircleRadius,
                              _center.GetPositionY() + std::sin(angle) * EyeCircleRadius,
                              _center.GetPositionZ());
        }

        Movement::MoveSplineInit init(me);
        init.MovebyPath(path);
        init.SetVelocity(EyeSpeed);
        init.Launch();
    }

    EventMap _events;
    Position _center;
};

// 102491 - Tyrande Achievement Tracker
class spell_echo_of_tyrande_achievement_tracker : public SpellScript
{
    void FilterTargets(std::list<WorldObject*>& targets)
    {
        // Moon Guard only tracks damage taken by the party's healers
        targets.remove_if([](WorldObject const* target)
        {
            Player const* player = target->ToPlayer();
            if (!player)
                return true;

            if (uint8 roles = sLFGMgr->GetRoles(player->GetGUID()))
                return (roles & lfg::PLAYER_ROLE_HEALER) == 0;

            switch (player->GetPrimaryTalentTree(player->GetActiveSpec()))
            {
                case TALENT_TREE_PALADIN_HOLY:
                case TALENT_TREE_PRIEST_DISCIPLINE:
                case TALENT_TREE_PRIEST_HOLY:
                case TALENT_TREE_SHAMAN_RESTORATION:
                case TALENT_TREE_DRUID_RESTORATION:
                    return false;
                default:
                    break;
            }

            return true;
        });
    }

    void Register() override
    {
        OnObjectAreaTargetSelect.Register(&spell_echo_of_tyrande_achievement_tracker::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
    }
};

// 102242 - Tears of Elune
class spell_echo_of_tyrande_tears_of_elune : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_TEARS_OF_ELUNE_MISSILE });
    }

    void HandleScriptEffect(SpellEffIndex /*effIndex*/)
    {
        Unit* target = GetHitUnit();
        target->CastSpell(target->GetPosition(), SPELL_TEARS_OF_ELUNE_MISSILE, true);
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_echo_of_tyrande_tears_of_elune::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};
}

void AddSC_boss_echo_of_tyrande()
{
    using namespace EndTime;
    using namespace EndTime::EchoOfTyrande;
    RegisterEndTimeCreatureAI(boss_echo_of_tyrande);
    RegisterEndTimeCreatureAI(npc_echo_of_tyrande_pool_of_moonlight);
    RegisterEndTimeCreatureAI(npc_echo_of_tyrande_gauntlet_add);
    RegisterEndTimeCreatureAI(npc_echo_of_tyrande_moonlance);
    RegisterEndTimeCreatureAI(npc_echo_of_tyrande_eye_of_elune);
    RegisterSpellScript(spell_echo_of_tyrande_achievement_tracker);
    RegisterSpellScript(spell_echo_of_tyrande_tears_of_elune);
}
