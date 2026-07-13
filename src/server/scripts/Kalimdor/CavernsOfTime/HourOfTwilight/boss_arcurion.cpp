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
 * Arcurion (54590) - first boss of Hour of Twilight.
 *
 * Choreography from retail sniffs (all timings observed twice, ±0.3s):
 *  - Frozen-leg Thrall (55779) walks from the canyon ledge down to the bowl
 *    overlook; his ready-check gossip fires "Show yourself!" - Arcurion
 *    materializes ~3s later (emerge visual 104767 on stalker 57197) and engages
 *    on his own ~11s after that. The rear Icewall (210049) closes for the fight.
 *  - Hand of Frost is chain-cast (2s cast, ~2.4s cycle); Chains of Frost first
 *    at +12s then every ~16s.
 *  - A Frozen Servitor (54600) spawns on one of the twenty 54598 rim points
 *    every 4s from +10s on; each lobs Icy Boulders (102198 telegraph -> native
 *    missile -> 102199 impact) at random players.
 *  - First Icy Tomb on Thrall at +30s; re-applied 30s after each tomb break (DBM).
 *  - At 30% the tomb shatters for good; Thrall pops Bloodlust/Molten Fury and
 *    burns the boss while Arcurion channels Torrent of Frost (104050: native
 *    channel + arena-wide boulder rain) until he dies.
 *  - Both Icewalls open 10s after the kill (instance handles it); Thrall walks
 *    down, blasts the exit wall with Lava Barrage, then ghost-wolf-runs around
 *    the frozen lake to hand the escort to Thrall 54972.
 */

#include "ScriptMgr.h"
#include "Containers.h"
#include "GameObject.h"
#include "InstanceScript.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "PassiveAI.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "SpellScript.h"
#include "TemporarySummon.h"
#include "hour_of_twilight.h"

namespace HourOfTwilight
{
namespace Arcurion
{
enum Spells
{
    SPELL_ARCURION_EMERGE           = 104767, // materialize visual, cast by stalker 57197
    SPELL_HAND_OF_FROST             = 102593, // chain-cast filler (2s cast)
    SPELL_CHAINS_OF_FROST           = 102582, // AoE damage + 10s root
    SPELL_ICY_TOMB                  = 103252, // freezes Thrall -> 103251 stun -> 103250 -> 103249 summons 54995
    SPELL_ICY_TOMB_STATE            = 103251,
    SPELL_ICY_TOMB_REMOVED          = 102722, // cast by the tomb on destruction
    SPELL_ICY_BOULDER_TELEGRAPH     = 102198, // 3s cast -> native missile -> 102199 impact at dest
    SPELL_TORRENT_OF_FROST          = 104050, // 3s cast -> channel 103962 + boulder-rain 104055/104058
    SPELL_THRALL_LAVA_BURST         = 102475, // 55779's support filler (2s cast)
    SPELL_THRALL_FREED              = 102108, // flare when the tomb breaks
    SPELL_THRALL_BLOODLUST          = 103834, // 30% burn package
    SPELL_THRALL_MOLTEN_FURY        = 103905,
    SPELL_THRALL_LAVA_BURST_BOSS    = 103923, // burn-phase nuke at the boss
    SPELL_THRALL_LAVA_BARRAGE       = 104540, // exit-wall blast
    SPELL_THRALL_GHOST_WOLF         = 2645
};

enum Events
{
    // Arcurion intro (out of combat)
    EVENT_INTRO_MATERIALIZE = 1,
    EVENT_INTRO_ENGAGE,

    // Arcurion combat
    EVENT_HAND_OF_FROST,
    EVENT_CHAINS_OF_FROST,
    EVENT_RIM_REINFORCEMENTS,
    EVENT_SUMMON_SERVITOR,
    EVENT_ICY_TOMB,

    // Servitor
    EVENT_BOULDER_VOLLEY,

    // Thrall 55779
    EVENT_THRALL_SUPPORT_CAST,
    EVENT_THRALL_SURROUNDED,
    EVENT_THRALL_BURN_BOSS,
    EVENT_THRALL_POST_FIGHT_TALK,
    EVENT_THRALL_BLAST_WALL,
    EVENT_THRALL_WOLF_RUN
};

enum Texts
{
    // Arcurion (canyon-voice groups 0-2 used by the leg-1 escort script)
    SAY_CANYON_INTRO        = 0, // 53797
    SAY_CANYON_AMBUSH       = 1, // 53798
    SAY_CANYON_ARRIVAL      = 2, // 53803
    SAY_MATERIALIZE         = 3, // 53818
    SAY_AGGRO               = 4, // 54495
    EMOTE_RIM_FORCES        = 5, // 54176
    EMOTE_FREEZE_THRALL     = 6, // 54199
    SAY_FREEZE_THRALL       = 7, // 54497
    SAY_TORRENT             = 8, // 54097
    SAY_DEATH               = 9, // 54502

    // Thrall 55779
    SAY_SHOW_YOURSELF       = 0, // 53049
    SAY_SURROUNDED          = 1, // 54177
    SAY_ALMOST_GOT_HIM      = 2, // 53963
    SAY_DISCOVERED          = 3, // 54294
    SAY_FOLLOW_ME           = 4  // 54446
};

enum Misc
{
    GOSSIP_MENU_READY_CHECK     = 13183,
    GOSSIP_MENU_LEAD_THE_WAY    = 13164,
    NPC_TEXT_READY_CHECK        = 18583,
    NPC_TEXT_LEAD_THE_WAY       = 18555,

    POINT_OVERLOOK              = 1,
    POINT_EXIT_WALL             = 2,
    POINT_WOLF_RUN              = 3,

    ACTION_REVEAL_ARCURION      = 101, // Thrall 55779 -> boss
    ACTION_RIM_FORCES           = 102  // boss -> Thrall 55779
};

// Thrall 55779: ledge -> Arcurion overlook (sniffed spline)
Position const ThrallOverlookPath[] =
{
    { 4832.924f, 126.993f, 84.853f },
    { 4813.428f, 106.922f, 79.033f },
    { 4801.502f,  94.177f, 75.337f },
    { 4792.127f,  83.901f, 72.342f },
    { 4786.657f,  77.741f, 70.716f }  // Icy Tomb spot
};

// Thrall 55779: overlook -> exit wall blast point (sniffed)
Position const ThrallExitPoint = { 4755.854f, 56.599f, 66.527f };

// Thrall 55779: ghost-wolf run around the frozen lake to the leg-2 handoff (sniffed spline)
Position const ThrallWolfRunPath[] =
{
    { 4728.788f,  38.575f, 65.349f },
    { 4688.065f,   9.570f, 65.352f },
    { 4671.627f,  30.047f, 68.875f },
    { 4659.198f,  47.991f, 74.264f },
    { 4650.908f,  71.128f, 80.529f },
    { 4658.952f, 119.205f, 91.882f },
    { 4652.482f, 154.643f, 96.895f },
    { 4630.868f, 191.936f, 97.649f },
    { 4615.846f, 235.611f, 95.230f },
    { 4606.137f, 283.642f, 94.437f },
    { 4606.574f, 315.611f, 95.649f },
    { 4583.181f, 347.971f, 94.691f },
    { 4544.504f, 367.521f, 84.175f },
    { 4511.203f, 396.679f, 70.634f },
    { 4488.254f, 437.713f, 59.345f },
    { 4476.983f, 458.408f, 55.708f },
    { 4449.565f, 459.800f, 49.081f },
    { 4420.513f, 455.337f, 40.190f },
    { 4408.250f, 461.342f, 36.436f }
};

struct boss_arcurion : public BossAI
{
    boss_arcurion(Creature* creature) : BossAI(creature, DATA_ARCURION), _torrentTriggered(false), _revealed(false)
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetImmuneToAll(true);
    }

    void JustAppeared() override
    {
        // Cache the sniffed rim points once per grid load.
        _rimPoints.clear();
        std::list<Creature*> points;
        me->GetCreatureListWithEntryInGrid(points, NPC_SERVITOR_SPAWN_POINT, 250.0f);
        for (Creature* point : points)
            _rimPoints.push_back(point->GetPosition());

        // Grid reload after the reveal (wipe recovery): skip the RP, open for pulls.
        if (instance->GetData(DATA_ESCORT_STAGE) >= STAGE_ARCURION_READY && instance->GetBossState(DATA_ARCURION) != DONE && me->IsVisible())
            OpenForPulls();
    }

    void DoAction(int32 action) override
    {
        if (action != ACTION_REVEAL_ARCURION || _revealed)
            return;
        _revealed = true;
        me->SetVisible(true);
        if (Creature* stalker = me->FindNearestCreature(NPC_ARCURION_SPAWN_VISUAL, 20.0f))
            stalker->CastSpell(stalker, SPELL_ARCURION_EMERGE, true);
        events.ScheduleEvent(EVENT_INTRO_MATERIALIZE, 3s);
        events.ScheduleEvent(EVENT_INTRO_ENGAGE, 14s);
    }

    void Reset() override
    {
        _Reset();
        _torrentTriggered = false;
        if (_revealed || (instance->GetData(DATA_ESCORT_STAGE) >= STAGE_ARCURION_READY && me->IsVisible()))
            OpenForPulls();
        if (GameObject* wall = instance->GetGameObject(DATA_ICEWALL_ARENA))
            wall->SetGoState(GO_STATE_ACTIVE);
    }

    void OpenForPulls()
    {
        _revealed = true;
        me->SetImmuneToAll(false);
        me->SetReactState(REACT_AGGRESSIVE);
    }

    void JustEngagedWith(Unit* who) override
    {
        BossAI::JustEngagedWith(who);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 1);
        // Lock the party in with him (sniffed: wall closes at his appearance).
        if (GameObject* wall = instance->GetGameObject(DATA_ICEWALL_ARENA))
            wall->SetGoState(GO_STATE_READY);
        events.ScheduleEvent(EVENT_HAND_OF_FROST, 1ms);
        events.ScheduleEvent(EVENT_CHAINS_OF_FROST, 12s);
        events.ScheduleEvent(EVENT_RIM_REINFORCEMENTS, 10s);
        events.ScheduleEvent(EVENT_ICY_TOMB, 30s);
    }

    void TombThrall()
    {
        if (_torrentTriggered)
            return;
        Creature* thrall = instance->GetCreature(DATA_THRALL_FROZEN);
        if (!thrall || thrall->HasAura(SPELL_ICY_TOMB_STATE))
            return;
        Talk(EMOTE_FREEZE_THRALL);
        Talk(SAY_FREEZE_THRALL);
        DoCast(thrall, SPELL_ICY_TOMB);
    }

    // Signalled by the Icy Tomb AI when the tomb is destroyed.
    void SetData(uint32 type, uint32 /*value*/) override
    {
        if (type == 0 && !_torrentTriggered && me->IsInCombat())
            events.ScheduleEvent(EVENT_ICY_TOMB, 30s); // DBM: re-tomb 30s after each break
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage) override
    {
        if (_torrentTriggered || !me->HealthBelowPctDamaged(30, damage))
            return;
        _torrentTriggered = true;
        events.Reset();
        me->InterruptNonMeleeSpells(false);

        // The tomb shatters for good and Thrall joins the burn.
        summons.DespawnEntry(NPC_ICY_TOMB);
        if (Creature* thrall = instance->GetCreature(DATA_THRALL_FROZEN))
        {
            thrall->RemoveAurasDueToSpell(SPELL_ICY_TOMB_STATE);
            if (thrall->IsAIEnabled())
                thrall->AI()->DoAction(ACTION_THRALL_FREED);
        }

        Talk(SAY_TORRENT);
        me->AttackStop();
        DoCastSelf(SPELL_TORRENT_OF_FROST);
    }

    void JustSummoned(Creature* summon) override
    {
        summons.Summon(summon);
        if (summon->GetEntry() == NPC_FROZEN_SERVITOR_SUMMON)
            DoZoneInCombat(summon);
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
        Talk(SAY_DEATH);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        if (Creature* thrall = instance->GetCreature(DATA_THRALL_FROZEN))
        {
            thrall->RemoveAurasDueToSpell(SPELL_ICY_TOMB_STATE);
            if (thrall->IsAIEnabled())
                thrall->AI()->DoAction(ACTION_ARCURION_DEAD);
        }
    }

    void EnterEvadeMode(EvadeReason why) override
    {
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        if (Creature* thrall = instance->GetCreature(DATA_THRALL_FROZEN))
            thrall->RemoveAurasDueToSpell(SPELL_ICY_TOMB_STATE);
        BossAI::EnterEvadeMode(why);
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        // The intro runs outside of combat.
        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_INTRO_MATERIALIZE:
                    Talk(SAY_MATERIALIZE);
                    continue;
                case EVENT_INTRO_ENGAGE:
                    Talk(SAY_AGGRO);
                    OpenForPulls();
                    DoZoneInCombat();
                    continue;
                default:
                    break;
            }

            if (!UpdateVictim())
                return;
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (eventId)
            {
                case EVENT_HAND_OF_FROST:
                    DoCastVictim(SPELL_HAND_OF_FROST);
                    events.Repeat(2400ms);
                    break;
                case EVENT_CHAINS_OF_FROST:
                    DoCastAOE(SPELL_CHAINS_OF_FROST);
                    events.Repeat(16s, 17s);
                    break;
                case EVENT_RIM_REINFORCEMENTS:
                    Talk(EMOTE_RIM_FORCES);
                    if (Creature* thrall = instance->GetCreature(DATA_THRALL_FROZEN))
                        if (thrall->IsAIEnabled())
                            thrall->AI()->DoAction(ACTION_RIM_FORCES);
                    events.ScheduleEvent(EVENT_SUMMON_SERVITOR, 1ms);
                    break;
                case EVENT_SUMMON_SERVITOR:
                    if (!_rimPoints.empty())
                        me->SummonCreature(NPC_FROZEN_SERVITOR_SUMMON,
                            Trinity::Containers::SelectRandomContainerElement(_rimPoints),
                            TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5s);
                    events.Repeat(4s);
                    break;
                case EVENT_ICY_TOMB:
                    TombThrall();
                    break;
                default:
                    break;
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
        }

        if (UpdateVictim() && !_torrentTriggered)
            DoMeleeAttackIfReady();
    }

private:
    std::vector<Position> _rimPoints;
    bool _torrentTriggered;
    bool _revealed;
};

// Rim add - stands on its ledge and lobs Icy Boulders at random players.
// 102198 (3s telegraph cast) natively launches the missile that triggers the
// 102199 impact at the snapshotted destination - fully DBC-driven.
struct npc_hot_frozen_servitor_summon : public ScriptedAI
{
    npc_hot_frozen_servitor_summon(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE); // never paths into the bowl
    }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        _events.ScheduleEvent(EVENT_BOULDER_VOLLEY, 2s, 4s);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);
        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;
        if (_events.ExecuteEvent() == EVENT_BOULDER_VOLLEY)
        {
            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 150.0f, true))
                DoCast(target, SPELL_ICY_BOULDER_TELEGRAPH);
            _events.Repeat(3400ms);
        }
    }

private:
    EventMap _events;
};

// The tomb: a target dummy that reports its destruction.
struct npc_hot_icy_tomb : public NullCreatureAI
{
    npc_hot_icy_tomb(Creature* creature) : NullCreatureAI(creature), _instance(creature->GetInstanceScript()) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        // The native chain has Thrall force-cast the summon - reclaim the tomb
        // for the enemy side so players can attack it.
        me->SetFaction(FACTION_MONSTER);
        me->SetImmuneToPC(false);
    }

    void JustDied(Unit* /*killer*/) override
    {
        DoCastAOE(SPELL_ICY_TOMB_REMOVED, true);
        if (Creature* thrall = _instance->GetCreature(DATA_THRALL_FROZEN))
        {
            thrall->RemoveAurasDueToSpell(SPELL_ICY_TOMB_STATE);
            thrall->CastSpell(thrall, SPELL_THRALL_FREED, true);
            if (thrall->IsAIEnabled())
                thrall->AI()->DoAction(ACTION_TOMB_DESTROYED);
        }
        if (Creature* arcurion = _instance->GetCreature(DATA_ARCURION))
            if (arcurion->IsAIEnabled())
                arcurion->AI()->SetData(0, 1); // schedule the re-tomb
        me->DespawnOrUnsummon(4s);
    }

private:
    InstanceScript* _instance;
};

// Thrall 55779 - walks to the overlook, ready-check gossip, support actor during
// Arcurion, exit-wall blast + ghost-wolf handoff run after the kill.
struct npc_hot_thrall_frozen : public ScriptedAI
{
    npc_hot_thrall_frozen(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript()), _atOverlook(false), _introPending(false) { }

    void JustAppeared() override
    {
        // Reloaded at the ready-check stage: stand at the overlook directly.
        if (_instance->GetData(DATA_ESCORT_STAGE) >= STAGE_ARCURION_READY && me->IsVisible())
        {
            me->NearTeleportTo(ThrallOverlookPath[4]);
            _atOverlook = true;
        }
    }

    void DoAction(int32 action) override
    {
        switch (action)
        {
            case ACTION_START_ESCORT_INTRO: // revealed at the ledge - walk down to the overlook
                me->SetWalk(true);
                me->GetMotionMaster()->MoveSmoothPath(POINT_OVERLOOK, ThrallOverlookPath, std::size(ThrallOverlookPath), true);
                break;
            case ACTION_RIM_FORCES:
                _events.ScheduleEvent(EVENT_THRALL_SURROUNDED, 1s);
                break;
            case ACTION_TOMB_DESTROYED: // players broke the tomb - resume support
                _events.ScheduleEvent(EVENT_THRALL_SUPPORT_CAST, 2s);
                break;
            case ACTION_THRALL_FREED: // boss at 30% - burn phase
                _events.Reset();
                Talk(SAY_ALMOST_GOT_HIM);
                DoCastSelf(SPELL_THRALL_BLOODLUST, true);
                DoCastSelf(SPELL_THRALL_MOLTEN_FURY, true);
                _events.ScheduleEvent(EVENT_THRALL_BURN_BOSS, 2s);
                break;
            case ACTION_ARCURION_DEAD:
                _events.Reset();
                _events.ScheduleEvent(EVENT_THRALL_POST_FIGHT_TALK, 7s);
                break;
            default:
                break;
        }
    }

    bool GossipHello(Player* player) override
    {
        bool arcurionDone = _instance->GetBossState(DATA_ARCURION) == DONE;
        uint32 menuId = arcurionDone ? GOSSIP_MENU_LEAD_THE_WAY : GOSSIP_MENU_READY_CHECK;
        uint32 textId = arcurionDone ? NPC_TEXT_LEAD_THE_WAY : NPC_TEXT_READY_CHECK;
        InitGossipMenuFor(player, menuId);
        AddGossipItemFor(player, menuId, 0, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
        SendGossipMenuFor(player, textId, me->GetGUID());
        return true;
    }

    bool GossipSelect(Player* player, uint32 /*menuId*/, uint32 /*gossipListId*/) override
    {
        CloseGossipMenuFor(player);
        me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        if (_instance->GetBossState(DATA_ARCURION) == DONE)
        {
            Talk(SAY_FOLLOW_ME);
            _events.ScheduleEvent(EVENT_THRALL_WOLF_RUN, 1s);
        }
        else
        {
            Talk(SAY_SHOW_YOURSELF);
            if (_atOverlook)
                RevealArcurion();
            else
                _introPending = true; // still walking - fire the reveal on arrival
        }
        return true;
    }

    void RevealArcurion()
    {
        _introPending = false;
        if (Creature* arcurion = _instance->GetCreature(DATA_ARCURION))
            if (arcurion->IsAIEnabled())
                arcurion->AI()->DoAction(ACTION_REVEAL_ARCURION);
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type != POINT_MOTION_TYPE && type != EFFECT_MOTION_TYPE)
            return;
        switch (id)
        {
            case POINT_OVERLOOK:
                _atOverlook = true;
                me->SetFacingTo(2.8f); // face the party
                if (_introPending)
                    RevealArcurion();
                break;
            case POINT_EXIT_WALL:
                me->SetFacingTo(3.67f); // face the exit wall
                _events.ScheduleEvent(EVENT_THRALL_BLAST_WALL, 1s);
                break;
            case POINT_WOLF_RUN:
                me->SetVisible(false);
                if (Creature* thrall = _instance->GetCreature(DATA_THRALL_GALAKROND))
                    thrall->SetVisible(true);
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
                case EVENT_THRALL_SURROUNDED:
                    Talk(SAY_SURROUNDED);
                    _events.ScheduleEvent(EVENT_THRALL_SUPPORT_CAST, 2s);
                    break;
                case EVENT_THRALL_SUPPORT_CAST:
                    // Free-cast Lava Burst at live rim servitors while not tombed.
                    if (!me->HasAura(SPELL_ICY_TOMB_STATE) && !me->HasUnitState(UNIT_STATE_CASTING))
                        if (Creature* servitor = me->FindNearestCreature(NPC_FROZEN_SERVITOR_SUMMON, 200.0f, true))
                            me->CastSpell(servitor, SPELL_THRALL_LAVA_BURST, false);
                    _events.Repeat(2400ms);
                    break;
                case EVENT_THRALL_BURN_BOSS:
                    if (!me->HasUnitState(UNIT_STATE_CASTING))
                        if (Creature* arcurion = _instance->GetCreature(DATA_ARCURION))
                            if (arcurion->IsAlive() && arcurion->IsInCombat())
                                me->CastSpell(arcurion, SPELL_THRALL_LAVA_BURST_BOSS, false);
                    _events.Repeat(2400ms);
                    break;
                case EVENT_THRALL_POST_FIGHT_TALK:
                    Talk(SAY_DISCOVERED);
                    me->SetWalk(false);
                    me->GetMotionMaster()->MovePoint(POINT_EXIT_WALL, ThrallExitPoint);
                    break;
                case EVENT_THRALL_BLAST_WALL:
                    if (Creature* stalker = me->FindNearestCreature(NPC_ICE_WALL_EXIT_STALKER, 100.0f))
                        me->CastSpell(stalker, SPELL_THRALL_LAVA_BARRAGE, true);
                    me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP); // "We have to keep moving; are you ready?"
                    break;
                case EVENT_THRALL_WOLF_RUN:
                    DoCastSelf(SPELL_THRALL_GHOST_WOLF, true);
                    me->SetWalk(false);
                    me->GetMotionMaster()->MoveSmoothPath(POINT_WOLF_RUN, ThrallWolfRunPath, std::size(ThrallWolfRunPath), false);
                    break;
                default:
                    break;
            }
        }
    }

private:
    InstanceScript* _instance;
    EventMap _events;
    bool _atOverlook;
    bool _introPending;
};

// Caps the native Icy Tomb summon chain (103251 -> periodic 103250 -> 103249) to one tomb.
class spell_arcurion_icy_tomb_summon : public SpellScript
{
    void PreventDuplicate(SpellEffIndex effIndex)
    {
        if (Unit* caster = GetCaster())
            if (caster->FindNearestCreature(NPC_ICY_TOMB, 10.0f, true))
                PreventHitDefaultEffect(effIndex);
    }

    void Register() override
    {
        OnEffectHit.Register(&spell_arcurion_icy_tomb_summon::PreventDuplicate, EFFECT_0, SPELL_EFFECT_SUMMON);
    }
};
} // namespace Arcurion
} // namespace HourOfTwilight

void AddSC_boss_arcurion()
{
    using namespace HourOfTwilight;
    using namespace HourOfTwilight::Arcurion;
    RegisterHourOfTwilightCreatureAI(boss_arcurion);
    RegisterHourOfTwilightCreatureAI(npc_hot_frozen_servitor_summon);
    RegisterHourOfTwilightCreatureAI(npc_hot_icy_tomb);
    RegisterHourOfTwilightCreatureAI(npc_hot_thrall_frozen);
    RegisterSpellScript(spell_arcurion_icy_tomb_summon);
}
