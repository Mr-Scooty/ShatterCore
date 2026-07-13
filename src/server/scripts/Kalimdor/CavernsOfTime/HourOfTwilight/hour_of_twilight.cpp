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

/*
 * Hour of Twilight (map 940) - Thrall escort framework, canyon/road/gauntlet
 * trash choreography, the Life Warden flight and the exit teleporter.
 *
 * All waypoints, hold positions and bark beats are taken from retail sniffs
 * (see the per-leg comments); the modern client still runs the 4.3.4 stalker-
 * creature implementation, so everything maps 1:1 onto era entries.
 */

#include "ScriptMgr.h"
#include "CellImpl.h"
#include "CombatAI.h"
#include "Containers.h"
#include "GameObject.h"
#include "GridNotifiersImpl.h"
#include "InstanceScript.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "PassiveAI.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "SpellScript.h"
#include "TemporarySummon.h"
#include "Vehicle.h"
#include "hour_of_twilight.h"

namespace HourOfTwilight
{
enum EscortSpells
{
    SPELL_THRALL_LAVA_BURST_CANYON  = 102475, // 54548 / 55779 filler (2s cast)
    SPELL_THRALL_HEALING_WAVE       = 103641,
    SPELL_THRALL_LAVA_BURST_ROAD    = 107980, // 54972 / 54634 filler
    SPELL_THRALL_TOTEM_TRIGGER      = 103819, // force-casts 108374 Rising Fire Totem
    SPELL_THRALL_ANCESTRAL_SPIRIT   = 103947, // revives the downed Life Warden
    SPELL_TOTEM_RISING_FIRE_SPAWN   = 103813,
    SPELL_TOTEM_RISING_FIRE_PULSE   = 103817, // +5% damage/health to allies, stacks
    SPELL_CANYON_BOULDER_VOLLEY     = 105433, // 54555 telegraph -> native missile -> 102199
    SPELL_ELEMENTAL_IMPALE          = 104019,
    SPELL_ASSASSIN_STEALTH          = 102921,
    SPELL_ASSASSIN_GARROTE          = 102925,
    SPELL_ASSASSIN_GARROTE_SILENCE  = 102926,
    SPELL_ASSASSIN_BACKSTAB         = 102924,
    SPELL_ASSASSIN_EVISCERATE       = 102967,
    SPELL_PERMANENT_FEIGN_DEATH     = 29266,
    SPELL_TELEPORTER_TRIGGER        = 108925
};

enum EscortEvents
{
    // Thrall 54548 (canyon)
    EVENT_CANYON_ARCURION_VOICE = 1,
    EVENT_CANYON_START_WALK,
    EVENT_CANYON_AMBUSH_SAY,
    EVENT_CANYON_RESUME_CHECK,
    EVENT_CANYON_HANDOFF,
    EVENT_ESCORT_SUPPORT_CAST,

    // Thrall 54972 (road)
    EVENT_ROAD_BARK,
    EVENT_ROAD_RESUME_CHECK,
    EVENT_ROAD_TOTEM,
    EVENT_ROAD_POST_ASIRA_TALK,
    EVENT_ROAD_WALK_TO_WARDEN,
    EVENT_ROAD_HEAL_WARDEN,
    EVENT_ROAD_BOARD_WARDEN,

    // Thrall 54634 (gauntlet)
    EVENT_GAUNTLET_RESUME_CHECK,
    EVENT_GAUNTLET_SLIME_RAIN,

    // Life Wardens
    EVENT_WARDEN_FLY_IN_SECOND,
    EVENT_WARDEN_LAND,
    EVENT_TAXI_WHISPER,

    // Assassin
    EVENT_ASSASSIN_BACKSTAB,
    EVENT_ASSASSIN_EVISCERATE,

    // Canyon trash
    EVENT_CANYON_TRASH_CAST
};

enum EscortTexts
{
    // Thrall 54548
    SAY_CANYON_INTRO        = 0, // 53046 "Heroes, we have the Dragon Soul..."
    SAY_CANYON_FOUND_US     = 1, // 53826 "How did they find us?..."
    SAY_CANYON_WHAT_MAGIC   = 2, // 53050 "What magic is this?"
    SAY_CANYON_LOOK_OUT     = 3, // 54489 "Look out!"
    SAY_CANYON_KEEP_MOVING  = 4, // 53048 "Hurry - we must keep moving."
    SAY_CANYON_ANOTHER      = 5, // 54490 "Another ambush. Watch your backs!"
    SAY_CANYON_BREATHER     = 6, // 54491 "Take a moment to catch your breath..."

    // Arcurion canyon-voice groups (boss_arcurion.cpp texts 0-2)
    SAY_ARCURION_VOICE_INTRO    = 0, // 53797
    SAY_ARCURION_VOICE_AMBUSH   = 1, // 53798
    SAY_ARCURION_VOICE_ARRIVAL  = 2, // 53803

    // Thrall 54972
    SAY_ROAD_BEWARE         = 0, // 53553 "Beware. Enemies approach!"
    SAY_ROAD_LET_NONE       = 1, // 53821 "Let none stand in our way."
    SAY_ROAD_LET_THEM_COME  = 2, // 53823 "Let them come!"
    SAY_ROAD_DRAKES_MEET    = 3, // 53857 "Alexstrasza's drakes should meet us here..."
    SAY_ROAD_ABOVE_US       = 4, // 53875 "Up there, above us!"
    SAY_ROAD_ASSASSIN       = 5, // 53913 (spoken during Asira's arrival)
    SAY_ROAD_NOT_STOPPED    = 6, // 53968 (spoken during Asira's arrival)
    SAY_ROAD_WELL_DONE      = 7, // 53986 "Well done, Let us see to our friend..."
    SAY_ROAD_FLY_AHEAD      = 8, // 53987 "The rest of the drakes should be here shortly..."

    // Life Warden taxi
    WHISPER_TAXI_LOOK_THERE = 0, // 54036

    // Dark Haze
    EMOTE_OLD_GODS          = 0  // 53085 "Spawn of the Old Gods materialize nearby!"
};

enum EscortMisc
{
    POINT_CANYON_WAVE_2     = 3,
    POINT_CANYON_END        = 6,
    POINT_ROAD_SEGMENT      = 100,
    POINT_WARDEN_SIDE       = 200,
    POINT_WARDEN_FLY_IN     = 201,
    POINT_WARDEN_FLY_OUT    = 202,
    POINT_TAXI_FLIGHT       = 203,
    POINT_GAUNTLET_SEGMENT  = 300,
    POINT_CHAMBER_WALK      = 400
};

// ---------------------------------------------------------------------------
// Leg 1 - The Crystal Vice (Thrall 54548)
// ---------------------------------------------------------------------------

Position const CanyonPath[] =
{
    { 4928.077f, 288.696f, 96.869f },
    { 4909.420f, 231.151f, 98.840f },
    { 4900.027f, 219.134f, 99.282f }, // ambush hold (33s on retail, hostile-gated here)
    { 4892.324f, 205.212f, 99.738f },
    { 4873.719f, 170.536f, 98.773f },
    { 4864.338f, 159.285f, 97.460f }  // handoff to Thrall 55779
};

// Activation helper: reveal and unleash pre-placed (hidden) ambushers near a point.
void ActivateAmbushers(Creature* owner, uint32 entry, Position const& center, float radius)
{
    std::list<Creature*> ambushers;
    owner->GetCreatureListWithEntryInGrid(ambushers, entry, 150.0f);
    for (Creature* ambusher : ambushers)
    {
        if (!ambusher->IsAlive() || ambusher->GetExactDist2d(&center) > radius || ambusher->IsVisible())
            continue;
        ambusher->SetVisible(true);
        ambusher->SetImmuneToPC(false);
        ambusher->SetReactState(REACT_AGGRESSIVE);
        if (ambusher->IsAIEnabled())
            ambusher->AI()->DoZoneInCombat();
    }
}

bool HostilesAliveNear(Creature* who, float radius)
{
    std::list<Creature*> hostiles;
    for (uint32 entry : { NPC_FROZEN_SERVITOR, NPC_CRYSTALLINE_ELEMENTAL, NPC_FROZEN_SHARD,
                          NPC_TWILIGHT_ASSASSIN, NPC_TWILIGHT_RANGER, NPC_TWILIGHT_SHADOW_WALKER,
                          NPC_TWILIGHT_THUG, NPC_TWILIGHT_BRUISER,
                          NPC_FACELESS_BRUTE, NPC_FACELESS_SHADOW_WEAVER, NPC_SHADOW_BORER, NPC_CORRUPTED_SLIME })
    {
        std::list<Creature*> found;
        who->GetCreatureListWithEntryInGrid(found, entry, radius);
        for (Creature* creature : found)
            if (creature->IsAlive() && creature->IsInCombat())
                return true;
    }
    return false;
}

struct npc_hot_thrall_entrance : public ScriptedAI
{
    npc_hot_thrall_entrance(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript()), _pointIndex(0), _started(false) { }

    bool GossipSelect(Player* player, uint32 /*menuId*/, uint32 /*gossipListId*/) override
    {
        CloseGossipMenuFor(player);
        if (_started)
            return true;
        _started = true;
        me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        _instance->SetData(DATA_ESCORT_STAGE, STAGE_CANYON_ESCORT);
        me->GetMap()->LoadGrid(4764.04f, 61.8f); // make sure Arcurion's grid (canyon voice) is live
        Talk(SAY_CANYON_INTRO);
        _events.ScheduleEvent(EVENT_CANYON_ARCURION_VOICE, 8s);
        _events.ScheduleEvent(EVENT_CANYON_START_WALK, 29s);
        _events.ScheduleEvent(EVENT_ESCORT_SUPPORT_CAST, 12s);
        return true;
    }

    void ArcurionVoice(uint8 group)
    {
        if (Creature* arcurion = _instance->GetCreature(DATA_ARCURION))
            if (arcurion->IsAIEnabled())
                arcurion->AI()->Talk(group);
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type != POINT_MOTION_TYPE)
            return;
        switch (id)
        {
            case POINT_CANYON_WAVE_2:
                // The flanking ambush: two servitors + a Crystalline Elemental.
                Talk(SAY_CANYON_WHAT_MAGIC);
                _events.ScheduleEvent(EVENT_CANYON_AMBUSH_SAY, 3500ms);
                break;
            case POINT_CANYON_END:
                Talk(SAY_CANYON_BREATHER);
                _events.ScheduleEvent(EVENT_CANYON_HANDOFF, 5s);
                break;
            default:
                MoveNext();
                break;
        }
    }

    void MoveNext()
    {
        if (_pointIndex >= std::size(CanyonPath))
            return;
        me->SetWalk(true);
        me->GetMotionMaster()->MovePoint(_pointIndex + 1, CanyonPath[_pointIndex]);
        ++_pointIndex;
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);
        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_CANYON_ARCURION_VOICE:
                    ArcurionVoice(SAY_ARCURION_VOICE_INTRO);
                    // First pair creeps in from behind the party at the entrance.
                    ActivateAmbushers(me, NPC_FROZEN_SERVITOR, { 4929.0f, 267.9f, 97.5f }, 15.0f);
                    break;
                case EVENT_CANYON_START_WALK:
                    Talk(SAY_CANYON_FOUND_US);
                    MoveNext();
                    break;
                case EVENT_CANYON_AMBUSH_SAY:
                    Talk(SAY_CANYON_LOOK_OUT);
                    ArcurionVoice(SAY_ARCURION_VOICE_AMBUSH);
                    ActivateAmbushers(me, NPC_FROZEN_SERVITOR, { 4908.5f, 228.4f, 99.1f }, 15.0f);
                    ActivateAmbushers(me, NPC_CRYSTALLINE_ELEMENTAL, { 4888.5f, 198.5f, 100.0f }, 15.0f);
                    _events.ScheduleEvent(EVENT_CANYON_RESUME_CHECK, 5s);
                    break;
                case EVENT_CANYON_RESUME_CHECK:
                    if (HostilesAliveNear(me, 60.0f))
                    {
                        _events.Repeat(2s);
                        break;
                    }
                    Talk(SAY_CANYON_KEEP_MOVING);
                    _events.ScheduleEvent(EVENT_CANYON_AMBUSH_SAY + 100, 2400ms);
                    break;
                case EVENT_CANYON_AMBUSH_SAY + 100:
                    // The shard cluster near the ice wall assembles.
                    Talk(SAY_CANYON_ANOTHER);
                    ActivateAmbushers(me, NPC_CRYSTALLINE_ELEMENTAL, { 4881.5f, 143.4f, 103.0f }, 15.0f);
                    ActivateAmbushers(me, NPC_FROZEN_SHARD, { 4881.5f, 143.4f, 103.0f }, 15.0f);
                    MoveNext();
                    break;
                case EVENT_CANYON_HANDOFF:
                    ArcurionVoice(SAY_ARCURION_VOICE_ARRIVAL);
                    _instance->SetData(DATA_ESCORT_STAGE, STAGE_ARCURION_READY);
                    if (Creature* frozen = _instance->GetCreature(DATA_THRALL_FROZEN))
                        if (frozen->IsAIEnabled())
                            frozen->AI()->DoAction(ACTION_START_ESCORT_INTRO);
                    break;
                case EVENT_ESCORT_SUPPORT_CAST:
                    if (!me->HasUnitState(UNIT_STATE_CASTING))
                    {
                        Unit* target = nullptr;
                        for (uint32 entry : { NPC_FROZEN_SERVITOR, NPC_CRYSTALLINE_ELEMENTAL, NPC_FROZEN_SHARD })
                            if (Creature* hostile = me->FindNearestCreature(entry, 60.0f, true))
                                if (hostile->IsVisible() && !hostile->IsImmuneToPC())
                                {
                                    target = hostile;
                                    break;
                                }
                        if (target)
                            me->CastSpell(target, SPELL_THRALL_LAVA_BURST_CANYON, false);
                    }
                    _events.Repeat(2400ms);
                    break;
                default:
                    break;
            }
        }
    }

private:
    InstanceScript* _instance;
    EventMap _events;
    uint32 _pointIndex;
    bool _started;
};

// Canyon ambushers (54555 Frozen Servitor, 55559 Crystalline Elemental, 55563
// Frozen Shard) - hidden and inert until their ambush beat triggers them.
struct npc_hot_canyon_ambusher : public ScriptedAI
{
    npc_hot_canyon_ambusher(Creature* creature) : ScriptedAI(creature)
    {
        if (creature->IsAlive())
        {
            creature->SetImmuneToPC(true);
            creature->SetReactState(REACT_PASSIVE);
        }
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        if (me->GetEntry() == NPC_FROZEN_SERVITOR)
            _events.ScheduleEvent(EVENT_CANYON_TRASH_CAST, 3s, 6s);
        else if (me->GetEntry() == NPC_CRYSTALLINE_ELEMENTAL)
            _events.ScheduleEvent(EVENT_CANYON_TRASH_CAST, 2s, 4s);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;
        _events.Update(diff);
        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;
        if (_events.ExecuteEvent() == EVENT_CANYON_TRASH_CAST)
        {
            if (me->GetEntry() == NPC_FROZEN_SERVITOR)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    DoCast(target, SPELL_CANYON_BOULDER_VOLLEY);
                _events.Repeat(6s, 8s);
            }
            else
            {
                DoCastVictim(SPELL_ELEMENTAL_IMPALE);
                _events.Repeat(2500ms, 4s);
            }
        }
        DoMeleeAttackIfReady();
    }

private:
    EventMap _events;
};

// ---------------------------------------------------------------------------
// Leg 2 - Galakrond's Rest (Thrall 54972)
// ---------------------------------------------------------------------------

struct RoadNode
{
    Position Dest;
    uint32 HoldMs;   // fixed pause at the node (RP pacing)
    int8 BarkGroup;  // creature_text group to fire on arrival, -1 = none
};

// Key nodes from the sniffed 26-point path (combat wander pruned).
RoadNode const RoadPath[] =
{
    { { 4395.671f, 431.016f,  21.900f }, 0,     -1 },
    { { 4380.550f, 415.682f,   9.304f }, 0,     -1 },
    { { 4366.536f, 405.658f,  -0.384f }, 0,     -1 },
    { { 4358.142f, 403.080f,  -4.024f }, 0,     -1 },
    { { 4348.767f, 404.155f,  -6.455f }, 0,     -1 },
    { { 4340.767f, 404.217f,  -8.083f }, 0,     SAY_ROAD_BEWARE },
    { { 4323.827f, 402.809f,  -8.083f }, 6000,  -1 }, // assassin ambush hold
    { { 4332.630f, 405.300f,  -8.083f }, 0,     -1 },
    { { 4336.102f, 417.059f,  -8.221f }, 0,     -1 },
    { { 4345.627f, 433.729f,  -7.513f }, 0,     SAY_ROAD_LET_NONE },
    { { 4347.064f, 442.507f,  -7.598f }, 0,     -1 },
    { { 4343.200f, 453.622f,  -7.867f }, 0,     -1 },
    { { 4341.415f, 464.012f,  -7.889f }, 0,     -1 },
    { { 4331.226f, 479.354f,  -8.112f }, 0,     -1 },
    { { 4324.190f, 479.269f,  -8.036f }, 4000,  -1 }, // camp hold
    { { 4326.598f, 505.323f,  -8.514f }, 0,     -1 },
    { { 4320.588f, 516.983f,  -9.463f }, 0,     -1 },
    { { 4316.045f, 526.188f,  -8.860f }, 0,     -1 },
    { { 4315.932f, 535.594f,  -8.911f }, 0,     -1 },
    { { 4317.026f, 544.061f,  -8.249f }, 0,     -1 },
    { { 4322.195f, 557.490f,  -7.794f }, 0,     SAY_ROAD_LET_THEM_COME }, // ranger camp
    { { 4305.413f, 568.809f,  -7.343f }, 4000,  -1 },
    { { 4285.710f, 568.135f,  -6.950f }, 5000,  SAY_ROAD_DRAKES_MEET },
    { { 4286.546f, 572.031f,  -6.717f }, 0,     SAY_ROAD_ABOVE_US } // Life Warden fly-in + Asira RP
};

Position const WardenCrashSite  = { 4285.797f, 602.125f, -6.739f, 4.4f };
Position const ThrallWardenSide = { 4285.980f, 595.128f, -6.729f };

struct npc_hot_thrall_galakrond : public ScriptedAI
{
    npc_hot_thrall_galakrond(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript()), _pointIndex(0), _started(false), _asiraFighting(false) { }

    bool GossipSelect(Player* player, uint32 /*menuId*/, uint32 /*gossipListId*/) override
    {
        CloseGossipMenuFor(player);
        if (_started)
            return true;
        _started = true;
        me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        _instance->SetData(DATA_ESCORT_STAGE, STAGE_GALAKROND_ESCORT);
        _events.ScheduleEvent(EVENT_ESCORT_SUPPORT_CAST, 5s);
        MoveNext();
        return true;
    }

    void MoveNext()
    {
        if (_pointIndex >= std::size(RoadPath))
            return;
        me->SetWalk(false);
        me->GetMotionMaster()->MovePoint(POINT_ROAD_SEGMENT + _pointIndex, RoadPath[_pointIndex].Dest);
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type != POINT_MOTION_TYPE || id != POINT_ROAD_SEGMENT + _pointIndex)
            return;

        RoadNode const& node = RoadPath[_pointIndex];
        if (node.BarkGroup >= 0)
            Talk(uint8(node.BarkGroup));

        ++_pointIndex;
        if (_pointIndex >= std::size(RoadPath))
        {
            // Arrived at the clearing: the Life Warden crashes in with Asira.
            _instance->SetData(DATA_ESCORT_STAGE, STAGE_ASIRA_READY);
            return;
        }
        _events.ScheduleEvent(EVENT_ROAD_RESUME_CHECK, node.HoldMs ? Milliseconds(node.HoldMs) : 1ms);
    }

    void DoAction(int32 action) override
    {
        switch (action)
        {
            case ACTION_ASIRA_ENGAGED:
                _asiraFighting = true;
                _events.ScheduleEvent(EVENT_ROAD_TOTEM, 10s);
                break;
            case ACTION_ASIRA_DEAD:
                _asiraFighting = false;
                _events.Reset();
                _events.ScheduleEvent(EVENT_ROAD_POST_ASIRA_TALK, 7s);
                break;
            default:
                break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);
        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_ROAD_RESUME_CHECK:
                    if (HostilesAliveNear(me, 40.0f))
                    {
                        _events.Repeat(2s);
                        break;
                    }
                    MoveNext();
                    break;
                case EVENT_ESCORT_SUPPORT_CAST:
                {
                    if (!me->HasUnitState(UNIT_STATE_CASTING))
                    {
                        Unit* target = nullptr;
                        if (_asiraFighting)
                            target = _instance->GetCreature(DATA_ASIRA_DAWNSLAYER);
                        if (!target || !target->IsAlive() || !target->IsInCombat())
                        {
                            for (uint32 entry : { NPC_TWILIGHT_ASSASSIN, NPC_TWILIGHT_RANGER, NPC_TWILIGHT_SHADOW_WALKER, NPC_TWILIGHT_THUG, NPC_TWILIGHT_BRUISER })
                                if (Creature* hostile = me->FindNearestCreature(entry, 40.0f, true))
                                    if (hostile->IsInCombat())
                                    {
                                        target = hostile;
                                        break;
                                    }
                        }
                        if (target)
                            me->CastSpell(target, SPELL_THRALL_LAVA_BURST_ROAD, false);
                    }
                    _events.Repeat(6s);
                    break;
                }
                case EVENT_ROAD_TOTEM:
                    if (_asiraFighting)
                    {
                        DoCastSelf(SPELL_THRALL_TOTEM_TRIGGER);
                        _events.Repeat(23s);
                    }
                    break;
                case EVENT_ROAD_POST_ASIRA_TALK:
                    Talk(SAY_ROAD_WELL_DONE);
                    _events.ScheduleEvent(EVENT_ROAD_WALK_TO_WARDEN, 5s);
                    break;
                case EVENT_ROAD_WALK_TO_WARDEN:
                    me->SetWalk(true);
                    me->GetMotionMaster()->MovePoint(POINT_WARDEN_SIDE, ThrallWardenSide);
                    _events.ScheduleEvent(EVENT_ROAD_HEAL_WARDEN, 10s);
                    break;
                case EVENT_ROAD_HEAL_WARDEN:
                    Talk(SAY_ROAD_FLY_AHEAD);
                    if (Creature* warden = _instance->GetCreature(DATA_LIFE_WARDEN_THRALLS))
                        me->CastSpell(warden, SPELL_THRALL_ANCESTRAL_SPIRIT, false);
                    _events.ScheduleEvent(EVENT_ROAD_BOARD_WARDEN, 5s);
                    break;
                case EVENT_ROAD_BOARD_WARDEN:
                    if (Creature* warden = _instance->GetCreature(DATA_LIFE_WARDEN_THRALLS))
                    {
                        warden->RemoveAurasDueToSpell(SPELL_PERMANENT_FEIGN_DEATH);
                        warden->SetStandState(UNIT_STAND_STATE_STAND);
                        me->EnterVehicle(warden, 0);
                        if (warden->IsAIEnabled())
                            warden->AI()->DoAction(ACTION_ASIRA_DEAD); // fly out
                    }
                    break;
                default:
                    break;
            }
        }
    }

private:
    InstanceScript* _instance;
    EventMap _events;
    uint32 _pointIndex;
    bool _started;
    bool _asiraFighting;
};

// Twilight Assassin (55106) - stealthed at fixed ambush points; pounces when the
// escort passes.
struct npc_hot_twilight_assassin : public ScriptedAI
{
    npc_hot_twilight_assassin(Creature* creature) : ScriptedAI(creature)
    {
        creature->SetReactState(REACT_DEFENSIVE);
    }

    void Reset() override
    {
        DoCastSelf(SPELL_ASSASSIN_STEALTH, true);
    }

    void MoveInLineOfSight(Unit* who) override
    {
        if (me->IsInCombat() || !who->IsAlive())
            return;
        if (who->GetTypeId() != TYPEID_PLAYER && who->GetEntry() != NPC_THRALL_GALAKROND)
            return;
        if (me->GetExactDist2d(who) > 9.0f)
            return;
        me->RemoveAurasDueToSpell(SPELL_ASSASSIN_STEALTH);
        me->CastSpell(who, SPELL_ASSASSIN_GARROTE, true);
        me->CastSpell(who, SPELL_ASSASSIN_GARROTE_SILENCE, true);
        AttackStart(who);
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        me->RemoveAurasDueToSpell(SPELL_ASSASSIN_STEALTH);
        _events.ScheduleEvent(EVENT_ASSASSIN_BACKSTAB, 6s);
        _events.ScheduleEvent(EVENT_ASSASSIN_EVISCERATE, 10s);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;
        _events.Update(diff);
        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_ASSASSIN_BACKSTAB:
                    DoCastVictim(SPELL_ASSASSIN_BACKSTAB);
                    _events.Repeat(7s, 9s);
                    break;
                case EVENT_ASSASSIN_EVISCERATE:
                    DoCastVictim(SPELL_ASSASSIN_EVISCERATE);
                    _events.Repeat(12s, 15s);
                    break;
                default:
                    break;
            }
        }
        DoMeleeAttackIfReady();
    }

private:
    EventMap _events;
};

// ---------------------------------------------------------------------------
// Life Warden flight
// ---------------------------------------------------------------------------

// The crashed drake (55415): flies in carrying Asira, crash-lands, lies wounded
// through the fight, then carries Thrall away once revived.
Position const WardenFlyInPath1[] =
{
    { 4138.424f, 554.477f, 66.606f },
    { 4164.514f, 549.056f, 68.717f },
    { 4240.319f, 500.962f, 24.712f },
    { 4268.984f, 530.255f, 19.400f },
    { 4248.996f, 558.036f, 20.000f }
};

Position const WardenFlyInPath2[] =
{
    { 4241.496f, 568.299f, 19.969f },
    { 4252.358f, 583.127f, 16.015f },
    { 4273.138f, 565.504f,  8.949f },
    { 4286.611f, 604.894f, 11.789f }
};

Position const WardenFlyOutPath[] =
{
    { 4206.213f, 472.639f, 73.647f },
    { 4138.598f, 523.495f, 73.647f },
    { 4054.506f, 549.823f, 73.647f }
};

struct npc_hot_life_warden_thrall : public VehicleAI
{
    npc_hot_life_warden_thrall(Creature* creature) : VehicleAI(creature), _instance(creature->GetInstanceScript()) { }

    void JustAppeared() override
    {
        uint32 stage = _instance->GetData(DATA_ESCORT_STAGE);
        if (stage >= STAGE_ASIRA_READY && stage < STAGE_ASIRA_DONE && me->IsVisible())
        {
            // Reload mid-encounter: lie wounded at the crash site.
            me->NearTeleportTo(WardenCrashSite);
            DoCastSelf(SPELL_PERMANENT_FEIGN_DEATH, true);
        }
    }

    void DoAction(int32 action) override
    {
        switch (action)
        {
            case ACTION_ASIRA_ARRIVES:
                me->SetCanFly(true);
                me->SetDisableGravity(true);
                me->GetMotionMaster()->MoveSmoothPath(POINT_WARDEN_FLY_IN, WardenFlyInPath1, std::size(WardenFlyInPath1), false, true);
                break;
            case ACTION_ASIRA_DEAD: // Thrall aboard - fly out
                me->SetCanFly(true);
                me->SetDisableGravity(true);
                me->GetMotionMaster()->MoveSmoothPath(POINT_WARDEN_FLY_OUT, WardenFlyOutPath, std::size(WardenFlyOutPath), false, true);
                break;
            default:
                break;
        }
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type != POINT_MOTION_TYPE && type != EFFECT_MOTION_TYPE)
            return;
        switch (id)
        {
            case POINT_WARDEN_FLY_IN:
                if (!_secondLeg)
                {
                    _secondLeg = true;
                    me->GetMotionMaster()->MoveSmoothPath(POINT_WARDEN_FLY_IN, WardenFlyInPath2, std::size(WardenFlyInPath2), false, true);
                }
                else
                    me->GetMotionMaster()->MoveLand(POINT_WARDEN_LAND_ID, WardenCrashSite);
                break;
            case POINT_WARDEN_LAND_ID:
            {
                me->SetCanFly(false);
                me->SetDisableGravity(false);
                DoCastSelf(SPELL_PERMANENT_FEIGN_DEATH, true);
                // Asira leaps off the crashing drake.
                if (Creature* asira = _instance->GetCreature(DATA_ASIRA_DAWNSLAYER))
                {
                    asira->SetVisible(true);
                    if (asira->IsAIEnabled())
                        asira->AI()->DoAction(ACTION_ASIRA_ARRIVES);
                }
                break;
            }
            case POINT_WARDEN_FLY_OUT:
                if (Vehicle* kit = me->GetVehicleKit())
                    kit->RemoveAllPassengers();
                if (Creature* thrall = _instance->GetCreature(DATA_THRALL_GALAKROND))
                    thrall->SetVisible(false);
                me->DespawnOrUnsummon(1s);
                break;
            default:
                break;
        }
    }

private:
    static constexpr uint32 POINT_WARDEN_LAND_ID = 210;
    InstanceScript* _instance;
    bool _secondLeg = false;
};

// Taxi drakes (55549): lie wounded on the rim; spell-click (103989) mounts the
// player and the drake carries them across the Dragon Wastes.
Position const TaxiFlightPath[] =
{
    { 4246.517f, 555.960f, 29.400f },
    { 4168.066f, 551.575f, 72.780f },
    { 4075.568f, 554.639f, 79.715f },
    { 4006.476f, 495.167f, 57.447f },
    { 3959.408f, 390.859f, 50.039f },
    { 3931.731f, 306.415f, 50.039f },
    { 3920.138f, 284.849f, 50.039f },
    { 3908.778f, 292.264f, 50.039f },
    { 3933.524f, 311.519f, 50.010f }
};

Position const TaxiLandingPosition = { 3933.524f, 311.519f, 11.81f };

struct npc_hot_life_warden_taxi : public VehicleAI
{
    npc_hot_life_warden_taxi(Creature* creature) : VehicleAI(creature) { }

    void Reset() override
    {
        DoCastSelf(SPELL_PERMANENT_FEIGN_DEATH, true);
    }

    void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply) override
    {
        if (!apply || who->GetTypeId() != TYPEID_PLAYER)
            return;
        me->RemoveAurasDueToSpell(SPELL_PERMANENT_FEIGN_DEATH);
        me->SetStandState(UNIT_STAND_STATE_STAND);
        me->SetCanFly(true);
        me->SetDisableGravity(true);
        me->GetMotionMaster()->MoveSmoothPath(POINT_TAXI_FLIGHT, TaxiFlightPath, std::size(TaxiFlightPath), false, true);
        _events.ScheduleEvent(EVENT_TAXI_WHISPER, 15s);
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type != POINT_MOTION_TYPE && type != EFFECT_MOTION_TYPE)
            return;
        if (id == POINT_TAXI_FLIGHT)
            me->GetMotionMaster()->MoveLand(POINT_TAXI_FLIGHT + 1, TaxiLandingPosition);
        else if (id == POINT_TAXI_FLIGHT + 1)
        {
            if (Vehicle* kit = me->GetVehicleKit())
                kit->RemoveAllPassengers();
            me->DespawnOrUnsummon(6s);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);
        if (_events.ExecuteEvent() == EVENT_TAXI_WHISPER)
        {
            if (Vehicle* kit = me->GetVehicleKit())
                for (auto const& [seatId, seat] : kit->Seats)
                    if (Unit* passenger = ObjectAccessor::GetUnit(*me, seat.Passenger.Guid))
                        if (Player* player = passenger->ToPlayer())
                            Talk(WHISPER_TAXI_LOOK_THERE, player);
        }
    }

private:
    EventMap _events;
};

// ---------------------------------------------------------------------------
// Leg 3 - Path of the Titans (Thrall 54634)
// ---------------------------------------------------------------------------

struct GauntletNode
{
    Position Dest;
    bool PackHold; // fight a faceless pack here
};

GauntletNode const GauntletPath[] =
{
    { { 3921.281f, 278.665f,   9.279f }, false },
    { { 3892.984f, 278.773f,   1.343f }, true  }, // pack 1
    { { 3834.076f, 280.135f, -21.924f }, true  }, // pack 2
    { { 3802.290f, 288.762f, -41.971f }, false },
    { { 3762.020f, 289.790f, -64.817f }, false },
    { { 3754.838f, 291.243f, -70.732f }, true  }  // pack 3, then Benedictus meets the party
};

Position const ChamberWalkPath[] =
{
    { 3628.100f, 281.830f, -120.140f },
    { 3594.270f, 277.800f, -120.160f },
    { 3571.000f, 275.300f, -118.000f },
    { 3563.037f, 274.587f, -115.975f }
};

struct npc_hot_thrall_titans : public ScriptedAI
{
    npc_hot_thrall_titans(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript()), _pointIndex(0), _started(false) { }

    bool GossipSelect(Player* player, uint32 /*menuId*/, uint32 /*gossipListId*/) override
    {
        CloseGossipMenuFor(player);
        if (_started)
            return true;
        _started = true;
        me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        me->RemoveAurasDueToSpell(102550); // template-addon Root Thrall - anchored until the escort starts
        _instance->SetData(DATA_ESCORT_STAGE, STAGE_TITANS_ESCORT);
        _events.ScheduleEvent(EVENT_ESCORT_SUPPORT_CAST, 8s);
        _events.ScheduleEvent(EVENT_GAUNTLET_SLIME_RAIN, 10s);
        MoveNext();
        return true;
    }

    void MoveNext()
    {
        if (_pointIndex >= std::size(GauntletPath))
            return;
        me->SetWalk(false);
        me->GetMotionMaster()->MovePoint(POINT_GAUNTLET_SEGMENT + _pointIndex, GauntletPath[_pointIndex].Dest);
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type != POINT_MOTION_TYPE && type != EFFECT_MOTION_TYPE)
            return;

        if (id == POINT_CHAMBER_WALK)
        {
            // Handoff to the fight-side Thrall (54971) and start the betrayal.
            me->SetVisible(false);
            if (Creature* thrall = _instance->GetCreature(DATA_THRALL_EPILOGUE))
                thrall->SetVisible(true);
            if (Creature* benedictus = _instance->GetCreature(DATA_ARCHBISHOP_BENEDICTUS))
                if (benedictus->IsAIEnabled())
                    benedictus->AI()->DoAction(ACTION_BENEDICTUS_REVEAL);
            return;
        }

        if (id != int32(POINT_GAUNTLET_SEGMENT + _pointIndex))
            return;

        GauntletNode const& node = GauntletPath[_pointIndex];
        ++_pointIndex;

        if (node.PackHold)
        {
            // "Spawn of the Old Gods materialize nearby!"
            if (Creature* haze = me->FindNearestCreature(NPC_DARK_HAZE, 60.0f))
                if (haze->IsAIEnabled())
                    haze->AI()->Talk(EMOTE_OLD_GODS);
            _events.ScheduleEvent(EVENT_GAUNTLET_RESUME_CHECK, 4s);
        }
        else if (_pointIndex < std::size(GauntletPath))
            MoveNext();

        if (_pointIndex >= std::size(GauntletPath))
            _events.ScheduleEvent(EVENT_GAUNTLET_RESUME_CHECK, 4s);
    }

    void DoAction(int32 action) override
    {
        if (action != ACTION_THRALL_ENTER_CHAMBER)
            return;
        _events.Reset();
        me->SetWalk(false);
        me->GetMotionMaster()->MoveSmoothPath(POINT_CHAMBER_WALK, ChamberWalkPath, std::size(ChamberWalkPath), false);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);
        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_GAUNTLET_RESUME_CHECK:
                    if (HostilesAliveNear(me, 45.0f))
                    {
                        _events.Repeat(2s);
                        break;
                    }
                    if (_pointIndex < std::size(GauntletPath))
                        MoveNext();
                    else
                        // Gauntlet cleared - Benedictus greets the party at the ramp.
                        _instance->SetData(DATA_ESCORT_STAGE, STAGE_BENEDICTUS_READY);
                    break;
                case EVENT_ESCORT_SUPPORT_CAST:
                    if (!me->HasUnitState(UNIT_STATE_CASTING))
                    {
                        for (uint32 entry : { NPC_FACELESS_BRUTE, NPC_FACELESS_SHADOW_WEAVER, NPC_SHADOW_BORER, NPC_CORRUPTED_SLIME })
                            if (Creature* hostile = me->FindNearestCreature(entry, 45.0f, true))
                                if (hostile->IsInCombat())
                                {
                                    me->CastSpell(hostile, SPELL_THRALL_LAVA_BURST_ROAD, false);
                                    break;
                                }
                    }
                    _events.Repeat(6s);
                    break;
                case EVENT_GAUNTLET_SLIME_RAIN:
                    // Corrupted Slimes drip from the temple all the way down (sniffed ambience).
                    if (_pointIndex < std::size(GauntletPath))
                    {
                        Position pos = me->GetPosition();
                        pos.m_positionX += frand(-12.0f, 12.0f);
                        pos.m_positionY += frand(-12.0f, 12.0f);
                        pos.m_positionZ += 12.0f;
                        me->SummonCreature(NPC_CORRUPTED_SLIME, pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30s);
                        _events.Repeat(9s, 14s);
                    }
                    break;
                default:
                    break;
            }
        }
    }

private:
    InstanceScript* _instance;
    EventMap _events;
    uint32 _pointIndex;
    bool _started;
};

// Rising Fire Totem (55474) - Thrall's snowball buff during the Asira fight.
struct npc_hot_rising_fire_totem : public NullCreatureAI
{
    npc_hot_rising_fire_totem(Creature* creature) : NullCreatureAI(creature) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        DoCastSelf(SPELL_TOTEM_RISING_FIRE_SPAWN, true);
        _scheduler = 0;
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler += diff;
        if (_scheduler >= 5000)
        {
            _scheduler = 0;
            DoCastSelf(SPELL_TOTEM_RISING_FIRE_PULSE, true);
        }
    }

private:
    uint32 _scheduler = 0;
};

// 108925 Teleporter Trigger - the exit portal's goober spell.
class spell_hot_teleporter_trigger : public SpellScript
{
    void HandleScriptEffect(SpellEffIndex /*effIndex*/)
    {
        Player* player = GetHitPlayer();
        if (!player)
            return;
        if (player->GetTeam() == ALLIANCE)
            player->TeleportTo(0, -8833.38f, 628.62f, 94.0f, 1.06f);   // Stormwind
        else
            player->TeleportTo(1, 1569.97f, -4397.41f, 16.06f, 0.54f); // Orgrimmar
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_hot_teleporter_trigger::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};
} // namespace HourOfTwilight

void AddSC_hour_of_twilight()
{
    using namespace HourOfTwilight;
    RegisterHourOfTwilightCreatureAI(npc_hot_thrall_entrance);
    RegisterHourOfTwilightCreatureAI(npc_hot_canyon_ambusher);
    RegisterHourOfTwilightCreatureAI(npc_hot_thrall_galakrond);
    RegisterHourOfTwilightCreatureAI(npc_hot_twilight_assassin);
    RegisterHourOfTwilightCreatureAI(npc_hot_life_warden_thrall);
    RegisterHourOfTwilightCreatureAI(npc_hot_life_warden_taxi);
    RegisterHourOfTwilightCreatureAI(npc_hot_thrall_titans);
    RegisterHourOfTwilightCreatureAI(npc_hot_rising_fire_totem);
    RegisterSpellScript(spell_hot_teleporter_trigger);
}
