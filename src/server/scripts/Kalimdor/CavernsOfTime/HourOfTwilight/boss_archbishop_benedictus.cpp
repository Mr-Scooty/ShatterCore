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
 * Archbishop Benedictus (54938) - final boss of Hour of Twilight.
 *
 * Choreography from retail sniffs + era DBM (timings observed twice):
 *  - He greets the party at the ramp ("Get inside, quickly!"), seals it behind
 *    them with Holy Wall on the Holy Shield stalker, walks back to the flooded
 *    platform, demands the Dragon Soul from Thrall and - 51.5s of dialogue later
 *    (DBM's TimerCombatStart matches to the decisecond) - turns hostile.
 *  - Phase 1 "the Light": Smite filler (104503, every 13.5s), Righteous Shear
 *    (46s), Purifying Light (3 orbs riding him as vehicle passengers, then
 *    hopping to players and detonating into persistent Purified pools), Wave of
 *    Virtue (a wave rider summoned at one of three sniffed west-edge lanes via
 *    the native spell_target_position spells, sweeping east at 10 yd/s).
 *    Thrall shears off orb volleys with Chain Lightning, dispels Righteous
 *    Shear, and shelters the party from waves with a Water Shell.
 *  - At 60%: Twilight Epiphany - the native 5s aura ends in a platform-wide
 *    blast (103755) and Engulfing Twilight (103762, retargeted onto Thrall),
 *    imprisoning him for the rest of the fight; Transform (103765) swaps
 *    Benedictus to his Twilight Father form (creature 54953 / model 38992).
 *  - Phase 2 mirrors the kit shadow-flavored with no Thrall support: orbs are
 *    attackable and must be shot down or all three leave pools; waves have no
 *    shell. The flooded floor's state controller (55445) drives the Seaping
 *    Light/Twilight ambience pulses throughout.
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
#include "SpellAuraEffects.h"
#include "SpellScript.h"
#include "TemporarySummon.h"
#include "hour_of_twilight.h"

namespace HourOfTwilight
{
namespace Benedictus
{
enum Spells
{
    // Benedictus RP
    SPELL_HOLY_WALL                 = 102629, // seals the ramp on the Holy Shield stalker

    // Phase 1
    SPELL_SMITE                     = 104503,
    SPELL_RIGHTEOUS_SHEAR           = 103151,
    SPELL_PURIFYING_LIGHT           = 103565, // summon visual aura for the orb volley
    SPELL_WAVE_OF_VIRTUE_SELF       = 103676, // announce on the boss
    SPELL_PURIFYING_LIGHT_VISUAL    = 103578, // orb spawn aura
    SPELL_PURIFYING_LIGHT_GROW      = 103579,
    SPELL_PURIFYING_BLAST_JUMP      = 103648, // orb hop -> triggers 103651 at the destination
    SPELL_PURIFYING_BLAST           = 103651, // blast + knockback + summons Purified pool 55427
    SPELL_PURIFIED_POOL             = 103654, // pool self-aura, ticks 103653
    SPELL_WAVE_OF_VIRTUE_RIDER      = 103678, // wave stalker self-aura, pulses 103684 every 0.25s
    SPELL_WAVE_OF_VIRTUE_PULSE      = 103684,

    // Transition
    SPELL_TWILIGHT_EPIPHANY         = 103754, // 5s aura; end-tick fires 103755 + 103762
    SPELL_ENGULFING_TWILIGHT        = 103762, // Thrall's prison (retargeted by spell script)
    SPELL_TRANSFORM                 = 103765, // native transform into 54953 (Twilight Father)

    // Phase 2
    SPELL_TWILIGHT_SMITE            = 104504,
    SPELL_TWILIGHT_SHEAR            = 103363,
    SPELL_CORRUPTING_TWILIGHT       = 103767,
    SPELL_WAVE_OF_TWILIGHT_SELF     = 103778,
    SPELL_CORRUPTING_VISUAL         = 103769,
    SPELL_CORRUPTING_GROW           = 103773,
    SPELL_TWILIGHT_BOLT_JUMP        = 103776, // orb hop -> triggers 103777 at the destination
    SPELL_TWILIGHT_BOLT             = 103777, // blast + knockback + summons Twilight pool 55468
    SPELL_TWILIGHT_POOL             = 103774, // pool self-aura, ticks 103775
    SPELL_WAVE_OF_TWILIGHT_RIDER    = 103780,
    SPELL_WAVE_OF_TWILIGHT_PULSE    = 103781,

    // Thrall support
    SPELL_THRALL_PULL_BUFF          = 108437,
    SPELL_THRALL_WATER_BOLT         = 108442,
    SPELL_CLEANSE_SPIRIT            = 103550,
    SPELL_CHAIN_LIGHTNING           = 103637, // destroys Purifying Light orbs
    SPELL_CHAIN_LIGHTNING_ARC       = 103638,
    SPELL_WATER_SHELL               = 103688, // cast on the Water Shell npc, pulses 103744
    SPELL_WATER_SHELL_BUFF          = 103744, // wave immunity marker + double damage

    // Earthen Shell Target (pool-state controller)
    SPELL_SEAPING_LIGHT             = 104516, // P1 self-aura, pulses 104528
    SPELL_SEAPING_TWILIGHT          = 104534  // P2 self-aura, pulses 104537
};

enum Events
{
    // Ramp / reveal RP (out of combat)
    EVENT_RAMP_TALK = 1,
    EVENT_RAMP_RETURN,
    EVENT_REVEAL_THRALL_REFUSE,     // Thrall: "I will NOT, Archbishop."
    EVENT_REVEAL_THIS_WAY,          // "I suppose it has to be this way, then."
    EVENT_REVEAL_THRALL_FIGUREHEAD, // Thrall: "You were a figurehead of the Light..."
    EVENT_REVEAL_ONLY_POWER,        // "There is only POWER!"
    EVENT_REVEAL_TRUE_MASTERS,      // "We serve the world's TRUE masters!"
    EVENT_REVEAL_HOSTILE,

    // Combat
    EVENT_SMITE,
    EVENT_SHEAR,
    EVENT_ORBS,
    EVENT_WAVE,
    EVENT_PHASE_TWO,

    // Orbs
    EVENT_ORB_DETACH,
    EVENT_ORB_DETONATE,

    // Wave rider
    EVENT_WAVE_LAUNCH,

    // Thrall
    EVENT_THRALL_WATER_BOLT,
    EVENT_THRALL_CLEANSE_SCAN,
    EVENT_THRALL_CHAIN_LIGHTNING
};

enum Texts
{
    // Benedictus
    SAY_GET_INSIDE          = 0, // 53254
    SAY_DEMAND_SOUL         = 1, // 53255
    SAY_THIS_WAY            = 2, // 53283
    SAY_ONLY_POWER          = 3, // 53278
    SAY_TRUE_MASTERS        = 4, // 54755
    SAY_AGGRO               = 5, // 56544
    SAY_WAVE_OF_VIRTUE      = 6, // 56542
    SAY_EPIPHANY            = 7, // 56541
    SAY_WAVE_OF_TWILIGHT    = 8, // 56543
    SAY_DEATH               = 9, // 56538
    EMOTE_WAVE_OF_VIRTUE    = 10, // 53877
    EMOTE_WAVE_OF_TWILIGHT  = 11, // 53896
    EMOTE_EPIPHANY          = 12, // 53904

    // Thrall 54971
    SAY_THRALL_REFUSE       = 0, // 53275
    SAY_THRALL_FIGUREHEAD   = 1  // 53279
};

enum Misc
{
    GOSSIP_MENU_THRALL          = 13363,
    NPC_TEXT_STAY_BY_MY_SIDE    = 18962,
    NPC_TEXT_IT_IS_DONE         = 18963,

    POINT_RAMP_TOP              = 1,
    POINT_FIGHT_ANCHOR          = 2,
    POINT_WAVE_END              = 3,

    DATA_ORBS_LAUNCHED          = 1,
    DATA_WAVE_INCOMING          = 2,

    PHASE_LIGHT                 = 1,
    PHASE_TWILIGHT              = 2
};

Position const RampTopPosition   = { 3724.686f, 289.068f, -92.481f };
Position const FightAnchor       = { 3547.737f, 272.793f, -115.975f, 0.121f };

// Benedictus' ramp walk (sniffed)
Position const RampPathUp[] =
{
    { 3564.04f, 274.84f, -115.97f },
    { 3627.09f, 281.59f, -120.15f },
    { 3654.42f, 283.12f, -120.17f },
    { 3673.20f, 284.52f, -118.41f },
    { 3701.12f, 286.70f, -105.07f },
    { 3724.686f, 289.068f, -92.481f }
};

Position const RampPathDown[] =
{
    { 3701.12f, 286.70f, -105.07f },
    { 3673.20f, 284.52f, -118.41f },
    { 3654.42f, 283.12f, -120.17f },
    { 3639.62f, 281.72f, -120.15f },
    { 3596.27f, 278.03f, -120.16f },
    { 3547.737f, 272.793f, -115.975f }
};

struct boss_archbishop_benedictus : public BossAI
{
    boss_archbishop_benedictus(Creature* creature) : BossAI(creature, DATA_ARCHBISHOP_BENEDICTUS),
        _transformed(false), _turnedHostile(false), _revealStarted(false), _metParty(false)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void JustAppeared() override
    {
        if (instance->GetBossState(DATA_ARCHBISHOP_BENEDICTUS) == DONE)
            return;
        // Reload after the reveal already played out: stand hostile at the anchor.
        if (instance->GetData(DATA_ESCORT_STAGE) >= STAGE_BENEDICTUS_READY)
            _metParty = true;
    }

    void DoAction(int32 action) override
    {
        switch (action)
        {
            case ACTION_BENEDICTUS_MEET_PARTY:
                if (_metParty)
                    break;
                _metParty = true;
                Talk(SAY_GET_INSIDE);
                me->SetWalk(false);
                me->GetMotionMaster()->MoveSmoothPath(POINT_RAMP_TOP, RampPathUp, std::size(RampPathUp), false);
                break;
            case ACTION_BENEDICTUS_REVEAL:
                StartReveal();
                break;
            default:
                break;
        }
    }

    void StartReveal()
    {
        if (_revealStarted || _turnedHostile)
            return;
        _revealStarted = true;
        Talk(SAY_DEMAND_SOUL);
        events.ScheduleEvent(EVENT_REVEAL_THRALL_REFUSE, 5300ms);
        events.ScheduleEvent(EVENT_REVEAL_THIS_WAY, 12600ms);
        events.ScheduleEvent(EVENT_REVEAL_THRALL_FIGUREHEAD, 23600ms);
        events.ScheduleEvent(EVENT_REVEAL_ONLY_POWER, 32200ms);
        events.ScheduleEvent(EVENT_REVEAL_TRUE_MASTERS, 41900ms);
        events.ScheduleEvent(EVENT_REVEAL_HOSTILE, 51500ms); // DBM TimerCombatStart 51.5s
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type != POINT_MOTION_TYPE && type != EFFECT_MOTION_TYPE)
            return;
        switch (id)
        {
            case POINT_RAMP_TOP:
                events.ScheduleEvent(EVENT_RAMP_RETURN, 12s);
                break;
            case POINT_FIGHT_ANCHOR:
                me->SetFacingTo(FightAnchor.GetOrientation());
                // Seal the ramp behind the party.
                if (Creature* shield = me->FindNearestCreature(NPC_HOLY_SHIELD, 200.0f))
                    me->CastSpell(shield, SPELL_HOLY_WALL, true);
                if (Creature* thrall = instance->GetCreature(DATA_THRALL_TITANS))
                    if (thrall->IsAIEnabled())
                        thrall->AI()->DoAction(ACTION_THRALL_ENTER_CHAMBER);
                break;
            default:
                break;
        }
    }

    // Reload fallback: the reveal is not persisted - re-run it when players walk in.
    void MoveInLineOfSight(Unit* who) override
    {
        BossAI::MoveInLineOfSight(who);
        if (!_revealStarted && !_turnedHostile && _metParty
            && instance->GetData(DATA_ESCORT_STAGE) >= STAGE_BENEDICTUS_READY
            && who->GetTypeId() == TYPEID_PLAYER && me->GetExactDist2d(who) < 45.0f)
            StartReveal();
    }

    void TurnHostile()
    {
        _turnedHostile = true;
        me->SetFaction(FACTION_MONSTER);
        me->SetImmuneToAll(false);
        me->SetReactState(REACT_AGGRESSIVE);
    }

    void Reset() override
    {
        _Reset();
        _transformed = false;
        me->RemoveAurasDueToSpell(SPELL_TRANSFORM);
        if (Creature* thrall = instance->GetCreature(DATA_THRALL_EPILOGUE))
            thrall->RemoveAurasDueToSpell(SPELL_ENGULFING_TWILIGHT);
        if (Creature* controller = GetController())
            if (controller->IsAIEnabled())
                controller->AI()->DoAction(ACTION_CONTROLLER_RESET);
        if (_turnedHostile)
        {
            // The betrayal is never replayed - players re-pull at the platform.
            me->SetFaction(FACTION_MONSTER);
            me->SetImmuneToAll(false);
            me->SetReactState(REACT_AGGRESSIVE);
        }
    }

    Creature* GetController() const
    {
        return me->FindNearestCreature(NPC_EARTHEN_SHELL_TARGET, 100.0f);
    }

    void JustEngagedWith(Unit* who) override
    {
        BossAI::JustEngagedWith(who);
        Talk(SAY_AGGRO);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 1);
        events.SetPhase(PHASE_LIGHT);
        events.ScheduleEvent(EVENT_SMITE, 6s, 0, PHASE_LIGHT);
        events.ScheduleEvent(EVENT_SHEAR, 8500ms, 0, PHASE_LIGHT);
        events.ScheduleEvent(EVENT_ORBS, 11s, 0, PHASE_LIGHT);
        events.ScheduleEvent(EVENT_WAVE, 30500ms, 0, PHASE_LIGHT);
        if (Creature* thrall = instance->GetCreature(DATA_THRALL_EPILOGUE))
            if (thrall->IsAIEnabled())
                thrall->AI()->DoAction(ACTION_BENEDICTUS_ENGAGED);
        if (Creature* controller = GetController())
            if (controller->IsAIEnabled())
                controller->AI()->DoAction(ACTION_CONTROLLER_ENGAGE);
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage) override
    {
        if (_transformed || !me->HealthBelowPctDamaged(60, damage))
            return;
        _transformed = true;
        events.Reset();
        me->InterruptNonMeleeSpells(false);
        Talk(SAY_EPIPHANY);
        Talk(EMOTE_EPIPHANY);
        // Native 5s aura: the end tick fires 103755 (platform blast) and 103762
        // (Engulfing Twilight, retargeted onto Thrall by spell script).
        DoCastSelf(SPELL_TWILIGHT_EPIPHANY);
        events.ScheduleEvent(EVENT_PHASE_TWO, 5s);
    }

    void LaunchOrbs()
    {
        bool twilight = _transformed;
        DoCastSelf(twilight ? SPELL_CORRUPTING_TWILIGHT : SPELL_PURIFYING_LIGHT, true);
        for (uint8 i = 0; i < 3; ++i)
        {
            Position pos = me->GetPosition();
            pos.m_positionY += float(i) - 1.0f;
            pos.m_positionZ += 1.5f;
            me->SummonCreature(twilight ? NPC_CORRUPTING_TWILIGHT : NPC_PURIFYING_LIGHT, pos, TEMPSUMMON_TIMED_DESPAWN, 20s);
        }
        if (!twilight)
            if (Creature* thrall = instance->GetCreature(DATA_THRALL_EPILOGUE))
                if (thrall->IsAIEnabled())
                    thrall->AI()->SetData(DATA_ORBS_LAUNCHED, 1);
    }

    void LaunchWave()
    {
        bool twilight = _transformed;
        Talk(twilight ? SAY_WAVE_OF_TWILIGHT : SAY_WAVE_OF_VIRTUE);
        Talk(twilight ? EMOTE_WAVE_OF_TWILIGHT : EMOTE_WAVE_OF_VIRTUE);
        DoCastSelf(twilight ? SPELL_WAVE_OF_TWILIGHT_SELF : SPELL_WAVE_OF_VIRTUE_SELF, true);
        // The locational spells summon the wave rider at one of three sniffed
        // west-edge lanes (spell_target_position, shipped in the world DB).
        uint32 const virtueLanes[3]   = { 103677, 103680, 103681 };
        uint32 const twilightLanes[3] = { 103782, 103783, 103784 };
        DoCastSelf((twilight ? twilightLanes : virtueLanes)[urand(0, 2)], true);
        if (!twilight)
            if (Creature* thrall = instance->GetCreature(DATA_THRALL_EPILOGUE))
                if (thrall->IsAIEnabled())
                    thrall->AI()->SetData(DATA_WAVE_INCOMING, 1);
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
        Talk(SAY_DEATH);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        if (Creature* thrall = instance->GetCreature(DATA_THRALL_EPILOGUE))
        {
            thrall->RemoveAurasDueToSpell(SPELL_ENGULFING_TWILIGHT);
            if (thrall->IsAIEnabled())
                thrall->AI()->DoAction(ACTION_THRALL_RELEASED);
        }
        if (Creature* controller = GetController())
            if (controller->IsAIEnabled())
                controller->AI()->DoAction(ACTION_CONTROLLER_RESET);
    }

    void EnterEvadeMode(EvadeReason why) override
    {
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        if (Creature* thrall = instance->GetCreature(DATA_THRALL_EPILOGUE))
            thrall->RemoveAurasDueToSpell(SPELL_ENGULFING_TWILIGHT);
        BossAI::EnterEvadeMode(why);
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        // Reveal RP runs outside of combat.
        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_RAMP_RETURN:
                    me->GetMotionMaster()->MoveSmoothPath(POINT_FIGHT_ANCHOR, RampPathDown, std::size(RampPathDown), false);
                    continue;
                case EVENT_REVEAL_THRALL_REFUSE:
                    if (Creature* thrall = instance->GetCreature(DATA_THRALL_EPILOGUE))
                        if (thrall->IsAIEnabled())
                            thrall->AI()->Talk(SAY_THRALL_REFUSE);
                    continue;
                case EVENT_REVEAL_THIS_WAY:
                    Talk(SAY_THIS_WAY);
                    continue;
                case EVENT_REVEAL_THRALL_FIGUREHEAD:
                    if (Creature* thrall = instance->GetCreature(DATA_THRALL_EPILOGUE))
                        if (thrall->IsAIEnabled())
                            thrall->AI()->Talk(SAY_THRALL_FIGUREHEAD);
                    continue;
                case EVENT_REVEAL_ONLY_POWER:
                    Talk(SAY_ONLY_POWER);
                    continue;
                case EVENT_REVEAL_TRUE_MASTERS:
                    Talk(SAY_TRUE_MASTERS);
                    continue;
                case EVENT_REVEAL_HOSTILE:
                    TurnHostile();
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
                case EVENT_SMITE:
                    DoCastVictim(_transformed ? SPELL_TWILIGHT_SMITE : SPELL_SMITE);
                    events.Repeat(13500ms);
                    break;
                case EVENT_SHEAR:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 100.0f, true))
                        DoCast(target, _transformed ? SPELL_TWILIGHT_SHEAR : SPELL_RIGHTEOUS_SHEAR);
                    else
                        DoCastVictim(_transformed ? SPELL_TWILIGHT_SHEAR : SPELL_RIGHTEOUS_SHEAR);
                    events.Repeat(46s);
                    break;
                case EVENT_ORBS:
                    LaunchOrbs();
                    events.Repeat(46s);
                    break;
                case EVENT_WAVE:
                    LaunchWave();
                    events.Repeat(48500ms);
                    break;
                case EVENT_PHASE_TWO:
                    events.SetPhase(PHASE_TWILIGHT);
                    DoCastSelf(SPELL_TRANSFORM, true);
                    if (Creature* thrall = instance->GetCreature(DATA_THRALL_EPILOGUE))
                        if (thrall->IsAIEnabled())
                            thrall->AI()->DoAction(ACTION_THRALL_IMPRISONED);
                    if (Creature* controller = GetController())
                        if (controller->IsAIEnabled())
                            controller->AI()->DoAction(ACTION_CONTROLLER_TWILIGHT);
                    events.ScheduleEvent(EVENT_ORBS, 1s);
                    events.ScheduleEvent(EVENT_WAVE, 9600ms);
                    events.ScheduleEvent(EVENT_SMITE, 10800ms);
                    events.ScheduleEvent(EVENT_SHEAR, 13200ms);
                    break;
                default:
                    break;
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
        }

        if (UpdateVictim())
            DoMeleeAttackIfReady();
    }

private:
    bool _transformed;
    bool _turnedHostile;
    bool _revealStarted;
    bool _metParty;
};

// Thrall 54971 - Benedictus fight ally, then the epilogue quest ender.
struct npc_hot_thrall_epilogue : public ScriptedAI
{
    npc_hot_thrall_epilogue(Creature* creature) : ScriptedAI(creature), _instance(creature->GetInstanceScript())
    {
        me->SetImmuneToNPC(true);
        me->SetReactState(REACT_PASSIVE);
    }

    void DoAction(int32 action) override
    {
        switch (action)
        {
            case ACTION_BENEDICTUS_ENGAGED:
                DoCastSelf(SPELL_THRALL_PULL_BUFF, true);
                _events.ScheduleEvent(EVENT_THRALL_WATER_BOLT, 3s);
                _events.ScheduleEvent(EVENT_THRALL_CLEANSE_SCAN, 2s);
                break;
            case ACTION_THRALL_IMPRISONED:
            case ACTION_THRALL_RELEASED:
                _events.Reset();
                me->InterruptNonMeleeSpells(false);
                break;
            default:
                break;
        }
    }

    void SetData(uint32 type, uint32 /*value*/) override
    {
        switch (type)
        {
            case DATA_ORBS_LAUNCHED:
                _events.ScheduleEvent(EVENT_THRALL_CHAIN_LIGHTNING, 2400ms);
                break;
            case DATA_WAVE_INCOMING:
                // Raise the Water Shell shelter right next to himself.
                if (Creature* shell = me->SummonCreature(NPC_WATER_SHELL, me->GetPositionX() + 3.0f, me->GetPositionY(), me->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 16000))
                    me->CastSpell(shell, SPELL_WATER_SHELL, true);
                break;
            default:
                break;
        }
    }

    bool GossipHello(Player* player) override
    {
        player->PrepareQuestMenu(me->GetGUID());
        bool done = _instance->GetBossState(DATA_ARCHBISHOP_BENEDICTUS) == DONE;
        SendGossipMenuFor(player, done ? NPC_TEXT_IT_IS_DONE : NPC_TEXT_STAY_BY_MY_SIDE, me->GetGUID());
        return true;
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);
        if (me->HasAura(SPELL_ENGULFING_TWILIGHT))
            return;

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_THRALL_WATER_BOLT:
                    if (!me->HasUnitState(UNIT_STATE_CASTING))
                        if (Creature* benedictus = _instance->GetCreature(DATA_ARCHBISHOP_BENEDICTUS))
                            if (benedictus->IsAlive() && benedictus->IsInCombat())
                                me->CastSpell(benedictus, SPELL_THRALL_WATER_BOLT, false);
                    _events.Repeat(3s);
                    break;
                case EVENT_THRALL_CLEANSE_SCAN:
                    for (auto const& ref : me->GetMap()->GetPlayers())
                        if (Player* player = ref.GetSource())
                            if (player->IsAlive() && player->HasAura(SPELL_RIGHTEOUS_SHEAR))
                            {
                                me->CastSpell(player, SPELL_CLEANSE_SPIRIT, true);
                                break;
                            }
                    _events.Repeat(4s);
                    break;
                case EVENT_THRALL_CHAIN_LIGHTNING:
                    if (Creature* orb = me->FindNearestCreature(NPC_PURIFYING_LIGHT, 100.0f, true))
                        me->CastSpell(orb, SPELL_CHAIN_LIGHTNING, false);
                    break;
                default:
                    break;
            }
        }
    }

private:
    InstanceScript* _instance;
    EventMap _events;
};

// Purifying Light (55377) / Corrupting Twilight (55467) - the orb volley: rise
// above Benedictus, then hop to a player and detonate into a persistent pool.
struct npc_hot_benedictus_orb : public ScriptedAI
{
    npc_hot_benedictus_orb(Creature* creature) : ScriptedAI(creature), _instance(creature->GetInstanceScript()), _detonated(false)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    bool IsTwilight() const { return me->GetEntry() == NPC_CORRUPTING_TWILIGHT; }

    void IsSummonedBy(Unit* summoner) override
    {
        DoCastSelf(IsTwilight() ? SPELL_CORRUPTING_VISUAL : SPELL_PURIFYING_LIGHT_VISUAL, true);
        DoCastSelf(IsTwilight() ? SPELL_CORRUPTING_GROW : SPELL_PURIFYING_LIGHT_GROW, true);
        me->SetDisableGravity(true);
        if (!IsTwilight())
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE); // only Thrall's Chain Lightning removes P1 orbs

        // Rise above the boss like the sniffed vehicle-seat fan.
        if (Unit* boss = summoner->ToUnit())
            me->GetMotionMaster()->MovePoint(0, boss->GetPositionX(), boss->GetPositionY() + frand(-4.0f, 4.0f), boss->GetPositionZ() + 9.0f);

        _events.ScheduleEvent(EVENT_ORB_DETONATE, 8s);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);
        if (_events.ExecuteEvent() == EVENT_ORB_DETONATE)
            Detonate();
    }

    void Detonate()
    {
        if (_detonated)
            return;
        _detonated = true;
        Unit* target = nullptr;
        if (Creature* benedictus = _instance->GetCreature(DATA_ARCHBISHOP_BENEDICTUS))
            if (benedictus->IsAIEnabled())
                target = benedictus->AI()->SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true);
        Position dest = target ? target->GetPosition() : me->GetPosition();
        dest.m_positionZ = -115.97f;
        me->GetMotionMaster()->MoveJump(dest, 20.0f, 15.0f, EVENT_JUMP);
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type != EFFECT_MOTION_TYPE || id != EVENT_JUMP)
            return;
        // Blast + knockback + native pool summon (55427 / 55468) at the landing point.
        me->CastSpell(me->GetPosition(), IsTwilight() ? SPELL_TWILIGHT_BOLT : SPELL_PURIFYING_BLAST, true);
        me->DespawnOrUnsummon(500ms);
    }

    void JustDied(Unit* /*killer*/) override
    {
        me->DespawnOrUnsummon(2s);
    }

private:
    InstanceScript* _instance;
    EventMap _events;
    bool _detonated;
};

// Purified (55427) / Twilight (55468) pools - persistent void zones left by orb blasts.
struct npc_hot_benedictus_pool : public NullCreatureAI
{
    npc_hot_benedictus_pool(Creature* creature) : NullCreatureAI(creature) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        DoCastSelf(me->GetEntry() == NPC_TWILIGHT_BLAST ? SPELL_TWILIGHT_POOL : SPELL_PURIFIED_POOL, true);
        // Tie the pool's lifetime to the encounter so wipes clean it up.
        if (InstanceScript* instance = me->GetInstanceScript())
            if (Creature* benedictus = instance->GetCreature(DATA_ARCHBISHOP_BENEDICTUS))
                if (benedictus->IsAIEnabled())
                    benedictus->AI()->JustSummoned(me);
    }
};

// Wave of Virtue (55441) / Wave of Twilight (55469) - summoned at a west-edge
// lane by the native locational spells, sweeps 100yd east at 10 yd/s.
struct npc_hot_benedictus_wave : public NullCreatureAI
{
    npc_hot_benedictus_wave(Creature* creature) : NullCreatureAI(creature) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        me->SetDisableGravity(true);
        DoCastSelf(me->GetEntry() == NPC_WAVE_OF_TWILIGHT ? SPELL_WAVE_OF_TWILIGHT_RIDER : SPELL_WAVE_OF_VIRTUE_RIDER, true);
        _events.ScheduleEvent(EVENT_WAVE_LAUNCH, 2500ms);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);
        if (_events.ExecuteEvent() == EVENT_WAVE_LAUNCH)
        {
            float orientation = me->GetOrientation();
            Position dest = me->GetPosition();
            dest.m_positionX += 100.0f * std::cos(orientation);
            dest.m_positionY += 100.0f * std::sin(orientation);
            me->GetMotionMaster()->MoveSmoothPath(POINT_WAVE_END, &dest, 1, false, true, 10.0f);
        }
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (id == POINT_WAVE_END)
            me->DespawnOrUnsummon();
    }

private:
    EventMap _events;
};

// Earthen Shell Target (55445) - the flooded floor's ambience/state controller.
struct npc_hot_benedictus_controller : public NullCreatureAI
{
    npc_hot_benedictus_controller(Creature* creature) : NullCreatureAI(creature) { }

    void DoAction(int32 action) override
    {
        switch (action)
        {
            case ACTION_CONTROLLER_ENGAGE:
                DoCastSelf(SPELL_SEAPING_LIGHT, true);
                break;
            case ACTION_CONTROLLER_TWILIGHT:
                me->RemoveAurasDueToSpell(SPELL_SEAPING_LIGHT);
                DoCastSelf(SPELL_SEAPING_TWILIGHT, true);
                break;
            case ACTION_CONTROLLER_RESET:
                me->RemoveAurasDueToSpell(SPELL_SEAPING_LIGHT);
                me->RemoveAurasDueToSpell(SPELL_SEAPING_TWILIGHT);
                break;
            default:
                break;
        }
    }
};

// 103762 Engulfing Twilight targets Thrall natively (TARGET_UNIT_SRC_AREA_ENTRY
// + a conditions row on creature 54971) - no spell script needed.

// 103684 / 103781 wave pulses - players sheltered in Thrall's Water Shell
// (carrying 103744) are immune to the Wave of Virtue.
class spell_benedictus_wave_pulse : public SpellScript
{
    void FilterTargets(std::list<WorldObject*>& targets)
    {
        targets.remove_if([](WorldObject* target)
        {
            Unit* unit = target->ToUnit();
            return unit && unit->HasAura(SPELL_WATER_SHELL_BUFF);
        });
    }

    void Register() override
    {
        OnObjectAreaTargetSelect.Register(&spell_benedictus_wave_pulse::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
        OnObjectAreaTargetSelect.Register(&spell_benedictus_wave_pulse::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ENEMY);
    }
};

// 103637 Chain Lightning - Thrall's orb-breaker arcs to a second orb (sniffed: hits 2 of 3).
class spell_benedictus_chain_lightning : public SpellScript
{
    void HandleInstakill(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* hit = GetHitUnit();
        if (!caster || !hit)
            return;
        if (Creature* firstOrb = hit->ToCreature())
        {
            std::list<Creature*> orbs;
            firstOrb->GetCreatureListWithEntryInGrid(orbs, NPC_PURIFYING_LIGHT, 40.0f);
            orbs.remove(firstOrb);
            orbs.remove_if([](Creature* orb) { return !orb->IsAlive(); });
            if (!orbs.empty())
            {
                orbs.sort(Trinity::ObjectDistanceOrderPred(firstOrb));
                Creature* second = orbs.front();
                caster->CastSpell(second, SPELL_CHAIN_LIGHTNING_ARC, true);
                Unit::Kill(caster, second);
            }
        }
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_benedictus_chain_lightning::HandleInstakill, EFFECT_0, SPELL_EFFECT_INSTAKILL);
    }
};
} // namespace Benedictus
} // namespace HourOfTwilight

void AddSC_boss_archbishop_benedictus()
{
    using namespace HourOfTwilight;
    using namespace HourOfTwilight::Benedictus;
    RegisterHourOfTwilightCreatureAI(boss_archbishop_benedictus);
    RegisterHourOfTwilightCreatureAI(npc_hot_thrall_epilogue);
    RegisterHourOfTwilightCreatureAI(npc_hot_benedictus_orb);
    RegisterHourOfTwilightCreatureAI(npc_hot_benedictus_pool);
    RegisterHourOfTwilightCreatureAI(npc_hot_benedictus_wave);
    RegisterHourOfTwilightCreatureAI(npc_hot_benedictus_controller);
    RegisterSpellScript(spell_benedictus_wave_pulse);
    RegisterSpellScript(spell_benedictus_chain_lightning);
}
