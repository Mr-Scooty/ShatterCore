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

#include "well_of_eternity.h"
#include "InstanceScript.h"
#include "Map.h"
#include "MapRefManager.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "PassiveAI.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellScript.h"

namespace WellOfEternity::Perotharn
{
enum Spells
{
    // Peroth'arn
    SPELL_CORRUPTING_TOUCH_ENABLER  = 104939, // self proc-aura, triggers 108101 on melee (also creature_template_addon)
    SPELL_CORRUPTING_TOUCH          = 108101, // the bolt: +10% damage taken, stacks, d20000
    SPELL_INCREASED_SCALE           = 105014, // 50% Increased Scale (intro growth)
    SPELL_CAMOUFLAGE                = 105341, // invisibility model + school immunity, infinite
    SPELL_CAMOUFLAGE_END            = 105541, // REMOVE_AURA 105341
    SPELL_FEL_FLAMES                = 108141, // dest cast -> 109824 (impact) + 108193 (summon 57329)
    SPELL_FEL_FLAMES_PERIODIC       = 108214, // stalker self-aura, pulses 108217 every 1 s
    SPELL_FEL_DECAY                 = 105544, // upfront + 1 s DoT; eff2 triggers 108124 on the victim
    SPELL_FEL_DECAY_HEAL_AURA       = 108124, // heal-reflect driver (scripted below)
    SPELL_FEL_SURGE                 = 108128, // shadow damage at the healer, bp = heal amount
    SPELL_DRAIN_ESSENCE             = 104905, // 4 s channeled AoE pacify/silence + 1 s ticks
    SPELL_FEL_ADDLED                = 105545, // stun/pacify on players during the drain window (d6000)
    SPELL_FEL_FIREBALL_AGGRO        = 105491, // 60 s hide-budget aura on players
    SPELL_STEALTH_DETECTION         = 93105,  // Invisibility and Stealth Detection (eyes)
    SPELL_ATTACK_ME_PEROTHARN       = 105509, // force-cast by the found player at the boss (threat + taunt)
    SPELL_FEL_FIREBALL_EXPLODE      = 105538, // eye self-destruct visual
    SPELL_EASY_PREY                 = 105493, // 8 s stun on the found player
    SPELL_FEL_QUICKENING            = 105526, // +100% attack speed, 15 s
    SPELL_ENFEEBLED                 = 105442, // self-stun + 25% damage taken, 15 s (hide budget expired)
    SPELL_ENDLESS_FRENZY            = 105521  // +25% damage done, natively removes 105442
};

enum Events
{
    EVENT_CORRUPTING_TOUCH = 1,
    EVENT_FEL_FLAMES,
    EVENT_FEL_DECAY,

    // Eye of Peroth'arn
    EVENT_EYE_WANDER = 1
};

enum Texts
{
    // Peroth'arn
    SAY_LEDGE_SENSE         = 0,  // "He is near, lurking in the shadows... I can sense it."
    SAY_LEDGE_FELGUARD      = 1,  // "You, Felguard.  Hold this area."
    SAY_LEDGE_COURTYARD     = 2,  // "The rest of you, secure the courtyard."
    SAY_INTRO               = 3,  // "Who shut down the portals? Clever little worms."
    SAY_ARRIVAL             = 4,  // "None will reach the palace without besting Peroth'arn, first of fel-touched!"
    SAY_AGGRO               = 5,  // "No mortal may stand before me and live!"
    SAY_DRAIN_ESSENCE       = 6,  // "Your essence... is MINE."
    SAY_SHADOWS             = 7,  // "The shadows serve ME, now..."
    EMOTE_VANISH            = 8,  // "%s vanishes into the shadows!"
    SAY_COWER               = 9,  // "Cower in hiding, heh."
    SAY_SPOTTED             = 10, // "I can see you."
    EMOTE_AMBUSH            = 11, // "%s ambushes his helpless prey!"
    SAY_FRENZY              = 12, // "ENOUGH! It is time to end this game!"
    SAY_DEATH               = 13, // "Nooooo... how can this be?"

    // Hunting Summon Stalker
    EMOTE_EYES_SEARCHING    = 0   // "The Eyes of Peroth'arn are looking for you."
};

enum Actions
{
    // File-local (shared header reserves values >= 10 for these)
    ACTION_EYE_SPOTTED          = 10
};

enum Points
{
    POINT_STAIR_BOTTOM = 1,
    POINT_FIREWALL_GATE,
    POINT_ARENA_CENTER
};

enum Phases
{
    PHASE_NONE = 0,
    PHASE_COMBAT,
    PHASE_DRAIN,        // Drain Essence transition (60%)
    PHASE_HIDE,         // camouflaged, eyes searching
    PHASE_ENFEEBLED     // hide budget expired, self-stunned
};

enum IntroStates
{
    INTRO_LEDGE = 0,    // DB spawn on the entrance ledge
    INTRO_GONE,         // ledge RP done, invisible
    INTRO_WALK,         // walking down from the palace stair
    INTRO_ARENA         // hostile at the arena, encounter ready
};

enum Misc
{
    TASK_GROUP_HIDE = 1,
    MAX_EYES        = 8
};

Position const IntroSpawnPosition   = { 3472.038f, -5106.056f, 213.680f, 2.1320f };
Position const StairBottomPosition  = { 3461.124f, -5088.549f, 213.597f };
Position const FirewallGatePosition = { 3392.934f, -4981.066f, 196.782f };
Position const ArenaCenterPosition  = { 3335.07f,  -4891.54f,  181.16f,  5.2883f };
Position const HuntingStalkerSpawn  = { 3335.072f, -4891.540f, 181.160f, 5.2883f };

struct EyeSpawn
{
    uint32 Entry;
    Position Pos;
};

EyeSpawn const EyeSpawns[MAX_EYES] =
{
    { NPC_EYE_OF_PEROTHARN_1, { 3342.711f, -4893.917f, 181.160f, 5.9815f } },
    { NPC_EYE_OF_PEROTHARN_1, { 3327.433f, -4889.163f, 181.160f, 2.8399f } },
    { NPC_EYE_OF_PEROTHARN_1, { 3332.695f, -4899.179f, 181.160f, 4.4107f } },
    { NPC_EYE_OF_PEROTHARN_1, { 3337.450f, -4883.901f, 181.160f, 1.2691f } },
    { NPC_EYE_OF_PEROTHARN_2, { 3338.792f, -4898.623f, 181.160f, 5.1960f } },
    { NPC_EYE_OF_PEROTHARN_2, { 3331.352f, -4884.458f, 181.160f, 2.0545f } },
    { NPC_EYE_OF_PEROTHARN_2, { 3327.990f, -4895.260f, 181.160f, 3.6253f } },
    { NPC_EYE_OF_PEROTHARN_2, { 3342.154f, -4887.820f, 181.160f, 0.4837f } }
};

constexpr float EyeDetectionRange   = 5.0f;
constexpr float EyeWanderRadius     = 12.0f;
constexpr float EyeWanderSpeed      = 0.85f;
constexpr float BossWanderRadius    = 25.0f;

struct boss_perotharn : public BossAI
{
    boss_perotharn(Creature* creature) : BossAI(creature, BOSS_PEROTHARN),
        _introState(INTRO_LEDGE), _phase(PHASE_NONE), _drainDone(false), _frenzied(false) { }

    void JustAppeared() override
    {
        if (instance->GetBossState(BOSS_PEROTHARN) == DONE)
        {
            me->DespawnOrUnsummon();
            return;
        }

        // Server restart / late grid load after the gauntlet was already completed:
        // skip the RP and wait for the group at the arena.
        if (_introState != INTRO_ARENA && instance->GetData(DATA_PORTALS_SHUT_DOWN) >= 3)
            SnapToArena();
    }

    void Reset() override
    {
        _Reset();
        _phase = PHASE_NONE;
        _drainDone = false;
        _frenzied = false;
        _preyGuid.Clear();
        ApplyIntroState();
    }

    void JustEngagedWith(Unit* who) override
    {
        BossAI::JustEngagedWith(who);
        Talk(SAY_AGGRO);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 1);
        instance->SetData(DATA_PLAYER_CAUGHT_BY_EYE, 0);

        _phase = PHASE_COMBAT;
        events.ScheduleEvent(EVENT_CORRUPTING_TOUCH, 2s);
        events.ScheduleEvent(EVENT_FEL_FLAMES, 5s);
        events.ScheduleEvent(EVENT_FEL_DECAY, 8s);
    }

    void EnterEvadeMode(EvadeReason why) override
    {
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        instance->SetData(DATA_PLAYER_CAUGHT_BY_EYE, 0);
        scheduler.CancelAll();
        summons.DespawnAll();
        me->RemoveAurasDueToSpell(SPELL_CAMOUFLAGE);
        me->RemoveAurasDueToSpell(SPELL_FEL_QUICKENING);
        me->RemoveAurasDueToSpell(SPELL_ENFEEBLED);
        me->RemoveAurasDueToSpell(SPELL_ENDLESS_FRENZY);
        StripPlayerAuras();
        ScriptedAI::EnterEvadeMode(why);
    }

    void JustDied(Unit* killer) override
    {
        BossAI::JustDied(killer);
        Talk(SAY_DEATH);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        StripPlayerAuras();
        NotifyGauntletIllidan(ACTION_ILLIDAN_PEROTHARN_DEAD);
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage) override
    {
        if (!_drainDone && _phase == PHASE_COMBAT && me->HealthBelowPctDamaged(60, damage))
        {
            _drainDone = true;
            StartDrainEssence();
        }

        if (!_frenzied && _phase != PHASE_HIDE && me->HealthBelowPctDamaged(20, damage))
        {
            _frenzied = true;
            Talk(SAY_FRENZY);
            DoCastSelf(SPELL_ENDLESS_FRENZY, true); // natively strips Enfeebled
            if (_phase == PHASE_ENFEEBLED)
                ResumeCombat(22s, 25s, nullptr);
        }
    }

    void SetGUID(ObjectGuid const& guid, int32 id) override
    {
        if (id == ACTION_EYE_SPOTTED)
            _preyGuid = guid;
    }

    void DoAction(int32 action) override
    {
        switch (action)
        {
            case ACTION_PEROTHARN_LEDGE_RP:
                if (_introState != INTRO_LEDGE)
                    break;

                Talk(SAY_LEDGE_SENSE);
                scheduler.Schedule(6s, [this](TaskContext)
                {
                    Talk(SAY_LEDGE_FELGUARD);
                });
                scheduler.Schedule(10s, [this](TaskContext)
                {
                    Talk(SAY_LEDGE_COURTYARD);
                });
                scheduler.Schedule(14s, [this](TaskContext)
                {
                    _introState = INTRO_GONE;
                    me->SetVisible(false);
                });
                break;
            case ACTION_PEROTHARN_INTRO:
                if (_introState == INTRO_LEDGE || _introState == INTRO_GONE)
                    StartIntro();
                break;
            case ACTION_EYE_SPOTTED:
                HandlePreySpotted();
                break;
            default:
                break;
        }
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type != POINT_MOTION_TYPE)
            return;

        switch (id)
        {
            case POINT_STAIR_BOTTOM:
                me->GetMotionMaster()->MovePoint(POINT_FIREWALL_GATE, FirewallGatePosition);
                break;
            case POINT_FIREWALL_GATE:
                me->GetMotionMaster()->MovePoint(POINT_ARENA_CENTER, ArenaCenterPosition);
                break;
            case POINT_ARENA_CENTER:
                FinishIntro();
                break;
            default:
                break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        scheduler.Update(diff);

        if (_introState != INTRO_ARENA)
            return;

        if (_phase == PHASE_DRAIN || _phase == PHASE_HIDE || _phase == PHASE_ENFEEBLED)
        {
            // Threat is reset and the react state passive during the transition -
            // detect a wipe manually so a dead party does not stall the encounter.
            if (!HasLivingTargetInArena())
                EnterEvadeMode(EVADE_REASON_NO_HOSTILES);
            return;
        }

        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_CORRUPTING_TOUCH:
                    DoCastVictim(SPELL_CORRUPTING_TOUCH, true);
                    events.Repeat(2s);
                    break;
                case EVENT_FEL_FLAMES:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.f, true))
                        DoCast(target, SPELL_FEL_FLAMES);
                    events.Repeat(8s + 400ms);
                    break;
                case EVENT_FEL_DECAY:
                {
                    Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 100.f, true);
                    if (!target)
                        target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.f, true);
                    if (target)
                        DoCast(target, SPELL_FEL_DECAY);
                    events.Repeat(17s);
                    break;
                }
                default:
                    break;
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
        }

        DoMeleeAttackIfReady();
    }

private:
    void ApplyIntroState()
    {
        switch (_introState)
        {
            case INTRO_LEDGE:
                me->SetVisible(true);
                MakeUnattackable();
                break;
            case INTRO_GONE:
                me->SetVisible(false);
                MakeUnattackable();
                break;
            case INTRO_WALK: // reset mid-walk should not happen; snap forward
                SnapToArena();
                break;
            case INTRO_ARENA:
                me->SetVisible(true);
                MakeAttackable();
                break;
            default:
                break;
        }
    }

    void MakeUnattackable()
    {
        me->SetImmuneToPC(true);
        me->SetImmuneToNPC(true);
        me->SetReactState(REACT_PASSIVE);
    }

    void MakeAttackable()
    {
        me->SetImmuneToPC(false);
        me->SetImmuneToNPC(false);
        me->SetReactState(REACT_AGGRESSIVE);
    }

    void SnapToArena()
    {
        _introState = INTRO_ARENA;
        me->NearTeleportTo(ArenaCenterPosition);
        me->SetHomePosition(ArenaCenterPosition);
        me->SetVisible(true);
        MakeAttackable();
    }

    void StartIntro()
    {
        _introState = INTRO_WALK;
        me->NearTeleportTo(IntroSpawnPosition);
        me->SetVisible(true);
        MakeUnattackable();
        DoCastSelf(SPELL_CORRUPTING_TOUCH_ENABLER, true);
        DoCastSelf(SPELL_INCREASED_SCALE, true);
        Talk(SAY_INTRO);
        me->SetWalk(true);

        scheduler.Schedule(2s, [this](TaskContext)
        {
            me->GetMotionMaster()->MovePoint(POINT_STAIR_BOTTOM, StairBottomPosition);
        });

        // He lurks unseen for the second half of the walk (sniff: +23 s)
        scheduler.Schedule(23s, [this](TaskContext)
        {
            if (_introState == INTRO_WALK)
                DoCastSelf(SPELL_CAMOUFLAGE, true);
        });
    }

    void FinishIntro()
    {
        _introState = INTRO_ARENA;
        me->SetWalk(false);
        me->SetHomePosition(ArenaCenterPosition);
        me->SetFacingTo(ArenaCenterPosition.GetOrientation());
        DoCastSelf(SPELL_CAMOUFLAGE_END, true);
        DoCastSelf(SPELL_CORRUPTING_TOUCH_ENABLER, true);
        Talk(SAY_ARRIVAL);
        MakeAttackable();
    }

    void StartDrainEssence()
    {
        _phase = PHASE_DRAIN;
        events.Reset();
        me->AttackStop();
        me->SetReactState(REACT_PASSIVE);
        me->GetMotionMaster()->Clear();
        me->StopMoving();
        me->InterruptNonMeleeSpells(false);

        Talk(SAY_DRAIN_ESSENCE);
        DoCastAOE(SPELL_DRAIN_ESSENCE);
        NotifyGauntletIllidan(ACTION_ILLIDAN_DRAIN_ESSENCE);

        scheduler.Schedule(4s, [this](TaskContext)
        {
            DoCastAOE(SPELL_FEL_ADDLED, true);
        });

        scheduler.Schedule(10s, [this](TaskContext)
        {
            StartHidePhase();
        });
    }

    void StartHidePhase()
    {
        _phase = PHASE_HIDE;
        Talk(EMOTE_VANISH);
        Talk(SAY_SHADOWS);
        me->InterruptNonMeleeSpells(true);
        DoCastSelf(SPELL_CAMOUFLAGE, true);
        ResetThreatList();
        me->GetMotionMaster()->MoveRandom(BossWanderRadius);

        me->SummonCreature(NPC_HUNTING_SUMMON_STALKER, HuntingStalkerSpawn, TEMPSUMMON_MANUAL_DESPAWN);
        for (EyeSpawn const& eye : EyeSpawns)
            me->SummonCreature(eye.Entry, eye.Pos, TEMPSUMMON_MANUAL_DESPAWN);

        // 60 s hide budget on every player
        for (MapReference const& ref : me->GetMap()->GetPlayers())
            if (Player* player = ref.GetSource())
                if (player->IsAlive() && !player->IsGameMaster())
                    me->CastSpell(player, SPELL_FEL_FIREBALL_AGGRO, true);

        scheduler.Schedule(5s, TASK_GROUP_HIDE, [this](TaskContext context)
        {
            Talk(SAY_COWER);
            context.Repeat(15s);
        });

        scheduler.Schedule(60s, TASK_GROUP_HIDE, [this](TaskContext)
        {
            EndHideExhausted();
        });
    }

    void HandlePreySpotted()
    {
        if (_phase != PHASE_HIDE)
            return;

        scheduler.CancelGroup(TASK_GROUP_HIDE);
        instance->SetData(DATA_PLAYER_CAUGHT_BY_EYE, 1); // voids Lazy Eye for this attempt

        Talk(SAY_SPOTTED);
        Talk(EMOTE_AMBUSH);
        EndHideCommon();
        DoCastSelf(SPELL_FEL_QUICKENING, true);

        Player* prey = ObjectAccessor::GetPlayer(*me, _preyGuid);
        if (prey)
        {
            DoCast(prey, SPELL_EASY_PREY, true);
            me->GetMotionMaster()->MoveJump(prey->GetPosition(), 25.f, 10.f);
        }

        // Post-hide cadence from the sniff: Fel Flames +22 s, Fel Decay +25 s
        ResumeCombat(22s, 25s, prey);
        NotifyGauntletIllidan(ACTION_ILLIDAN_HIDE_ENDED);
    }

    void EndHideExhausted()
    {
        if (_phase != PHASE_HIDE)
            return;

        scheduler.CancelGroup(TASK_GROUP_HIDE);
        EndHideCommon();
        me->StopMoving();
        DoCastSelf(SPELL_ENFEEBLED, true);
        _phase = PHASE_ENFEEBLED;
        NotifyGauntletIllidan(ACTION_ILLIDAN_HIDE_ENDED);

        // Attackable but stunned for the Enfeebled window (15 s), then combat resumes
        scheduler.Schedule(15s, [this](TaskContext)
        {
            if (_phase == PHASE_ENFEEBLED)
                ResumeCombat(22s, 25s, nullptr);
        });
    }

    // Shared spotted/exhausted cleanup: drop the camouflage, despawn the search party
    void EndHideCommon()
    {
        summons.DespawnEntry(NPC_EYE_OF_PEROTHARN_1);
        summons.DespawnEntry(NPC_EYE_OF_PEROTHARN_2);
        summons.DespawnEntry(NPC_HUNTING_SUMMON_STALKER);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_FEL_FIREBALL_AGGRO);
        me->GetMotionMaster()->Clear();
        DoCastSelf(SPELL_CAMOUFLAGE_END, true);
        DoCastSelf(SPELL_CORRUPTING_TOUCH_ENABLER, true);
    }

    void ResumeCombat(Milliseconds felFlamesIn, Milliseconds felDecayIn, Player* target)
    {
        _phase = PHASE_COMBAT;
        me->SetReactState(REACT_AGGRESSIVE);
        events.ScheduleEvent(EVENT_CORRUPTING_TOUCH, 2s);
        events.ScheduleEvent(EVENT_FEL_FLAMES, felFlamesIn);
        events.ScheduleEvent(EVENT_FEL_DECAY, felDecayIn);
        DoZoneInCombat();
        if (target)
            AttackStart(target);
    }

    void StripPlayerAuras()
    {
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_FEL_FIREBALL_AGGRO);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SHADOW_WALK);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SHADOW_WALK_PULSE);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SHADOW_AMBUSHER);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SHADOW_AMBUSHER_STEALTH);
    }

    void NotifyGauntletIllidan(int32 action)
    {
        if (Creature* illidan = instance->GetCreature(DATA_GAUNTLET_ILLIDAN))
            if (illidan->IsAIEnabled())
                illidan->AI()->DoAction(action);
    }

    bool HasLivingTargetInArena() const
    {
        for (MapReference const& ref : me->GetMap()->GetPlayers())
            if (Player* player = ref.GetSource())
                if (player->IsAlive() && !player->IsGameMaster() && me->IsWithinDist(player, 200.f, false))
                    return true;

        return false;
    }

    uint8 _introState;
    uint8 _phase;
    bool _drainDone;
    bool _frenzied;
    ObjectGuid _preyGuid;
};

struct npc_eye_of_perotharn : public ScriptedAI
{
    npc_eye_of_perotharn(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript()), _spotted(false)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void JustAppeared() override
    {
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        DoCastSelf(SPELL_STEALTH_DETECTION, true);
        me->SetSpeedRate(MOVE_WALK, EyeWanderSpeed);
        me->SetSpeedRate(MOVE_RUN, EyeWanderSpeed);
        me->SetWalk(true);
        _events.ScheduleEvent(EVENT_EYE_WANDER, 2s);
    }

    void MoveInLineOfSight(Unit* who) override
    {
        if (_spotted)
            return;

        Player* player = who->ToPlayer();
        if (!player || !player->IsAlive() || player->IsGameMaster())
            return;

        if (!me->IsWithinDist(player, EyeDetectionRange, false))
            return;

        Creature* perotharn = _instance->GetCreature(BOSS_PEROTHARN);
        if (!perotharn || !perotharn->IsAlive())
            return;

        _spotted = true;
        player->CastSpell(perotharn, SPELL_ATTACK_ME_PEROTHARN, true);
        DoCastSelf(SPELL_FEL_FIREBALL_EXPLODE, true);

        if (perotharn->IsAIEnabled())
        {
            perotharn->AI()->SetGUID(player->GetGUID(), ACTION_EYE_SPOTTED);
            perotharn->AI()->DoAction(ACTION_EYE_SPOTTED);
        }

        me->GetMotionMaster()->Clear();
        me->DespawnOrUnsummon(2s);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_EYE_WANDER:
                    me->GetMotionMaster()->MoveRandom(EyeWanderRadius);
                    break;
                default:
                    break;
            }
        }
    }

private:
    InstanceScript* _instance;
    EventMap _events;
    bool _spotted;
};

struct npc_perotharn_hunting_stalker : public NullCreatureAI
{
    npc_perotharn_hunting_stalker(Creature* creature) : NullCreatureAI(creature) { }

    void JustAppeared() override
    {
        Talk(EMOTE_EYES_SEARCHING);
    }
};

struct npc_perotharn_fel_flames : public NullCreatureAI
{
    npc_perotharn_fel_flames(Creature* creature) : NullCreatureAI(creature) { }

    void JustAppeared() override
    {
        // 108214 pulses the 108217 ground damage every second
        DoCastSelf(SPELL_FEL_FLAMES_PERIODIC, true);
        me->DespawnOrUnsummon(32s);
    }
};

// 108124 - Fel Decay Heal Aura: any heal received by the victim lashes back at the healer
class spell_perotharn_fel_decay_heal : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_FEL_SURGE });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetActor() && eventInfo.GetHealInfo() && eventInfo.GetHealInfo()->GetHeal();
    }

    void HandleProc(ProcEventInfo& eventInfo)
    {
        Unit* healer = eventInfo.GetActor();
        int32 damage = int32(eventInfo.GetHealInfo()->GetHeal()); // 100% of the heal amount

        Unit* caster = GetCaster(); // Peroth'arn (105544 eff2 trigger)
        if (!caster)
            caster = GetTarget();

        caster->CastSpell(healer, SPELL_FEL_SURGE, CastSpellExtraArgs(true).AddSpellBP0(damage));
    }

    void Register() override
    {
        DoCheckProc.Register(&spell_perotharn_fel_decay_heal::CheckProc);
        OnProc.Register(&spell_perotharn_fel_decay_heal::HandleProc);
    }
};
}

void AddSC_boss_perotharn()
{
    using namespace WellOfEternity;
    using namespace WellOfEternity::Perotharn;
    RegisterWellOfEternityCreatureAI(boss_perotharn);
    RegisterWellOfEternityCreatureAI(npc_eye_of_perotharn);
    RegisterWellOfEternityCreatureAI(npc_perotharn_hunting_stalker);
    RegisterWellOfEternityCreatureAI(npc_perotharn_fel_flames);
    RegisterSpellScript(spell_perotharn_fel_decay_heal);
}
