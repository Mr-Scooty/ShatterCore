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
#include "Containers.h"
#include "InstanceScript.h"
#include "Map.h"
#include "MapRefManager.h"
#include "ObjectAccessor.h"
#include "PassiveAI.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "TemporarySummon.h"

#include <algorithm>
#include <list>
#include <vector>

namespace WellOfEternity::QueenAzshara
{
enum Spells
{
    // Queen Azshara
    SPELL_SHROUD_OF_LUMINOSITY      = 102915, // full damage immunity - she is never killed, only "defeated"
    SPELL_DUMMY_NUKE                = 87235,  // observed 1.22 s filler at random players // verify in walkthrough
    SPELL_SERVANT_OF_THE_QUEEN      = 102334, // SET_VEHICLE_ID 1745 + AOE_CHARM + damage immunity on the puppet
    SPELL_TOTAL_OBEDIENCE           = 103241, // long interruptible hard cast, charms the whole group on completion
    SPELL_TOTAL_OBEDIENCE_AOE       = 110096, // companion self-AoE stun on completed cast
    SPELL_CLEAR_ALL_STATUS_AILMENTS = 105560,

    // Hand of the Queen
    SPELL_PUPPET_STRING             = 102319, // beam visual on the puppeted player
    SPELL_PUPPET_STRING_SELF        = 102333,

    // Enchanted Magus - fire (54882)
    SPELL_FIREBALL                  = 102265,
    SPELL_FIREBOMB                  = 102482,
    SPELL_BLAST_WAVE                = 102483,
    SPELL_FIRE_CHANNELING           = 110494,

    // Enchanted Magus - frost (54883)
    SPELL_ICE_FLING                 = 102478,
    SPELL_BLADES_OF_ICE             = 102467, // CHARGE_DEST + triggered 102468 weapon hit (both native)
    SPELL_BLADES_OF_ICE_AOE         = 102476,
    SPELL_COLDFLAME                 = 102465, // line start
    SPELL_COLDFLAME_LINE            = 102466, // marching destination casts
    SPELL_FROST_CHANNELING          = 110492,

    // Enchanted Magus - arcane (54884)
    SPELL_ARCANE_SHOCK              = 102463, // 5 s channel, pulses 102464 natively (periodic trigger, 1 s)
    SPELL_ARCANE_BOMB_DETONATION    = 102455,
    SPELL_ARCANE_BOMB_GROUND_VISUAL = 102460,
    SPELL_ARCANE_BOMB_AIR_VISUAL    = 109122,
    SPELL_ARCANE_CHANNELING         = 110495
};

enum Events
{
    // Queen Azshara
    EVENT_DUMMY_NUKE = 1,
    EVENT_ACTIVATE_MAGUS,
    EVENT_SERVANT_OF_THE_QUEEN,
    EVENT_TOTAL_OBEDIENCE,

    // Enchanted Magus
    EVENT_FIREBALL,
    EVENT_FIREBOMB,
    EVENT_BLAST_WAVE,
    EVENT_ICE_FLING,
    EVENT_BLADES_OF_ICE,
    EVENT_COLDFLAME,
    EVENT_ARCANE_SHOCK,
    EVENT_ARCANE_BOMB,

    // Arcane Bomb orb
    EVENT_DETONATE
};

enum Texts
{
    // Queen Azshara
    SAY_INTRO_1             = 0,
    SAY_INTRO_2             = 1,
    SAY_MAGUS_DEATH         = 2, // two flavor lines, random pick
    SAY_PUPPETS             = 3, // "Serve Azshara, puppets, and rejoice."
    EMOTE_TOTAL_OBEDIENCE   = 4, // raid boss emote interrupt warning
    SAY_INTERRUPT           = 5,
    SAY_DEFEAT              = 6,
    SAY_RIDERS              = 7,
    SAY_VAROTHEN_DISPOSE    = 8,

    // Enchanted Magus - groups 0-2 = first/second/third prayer line (all three on each entry)
    // Varo'then cameo (57117)
    SAY_AT_YOUR_SIDE        = 0
};

enum Actions
{
    // Enchanted Magus (file-local actions start at 10, see WOESharedActions)
    ACTION_MAGUS_RESET = 10
};

enum Data
{
    // Enchanted Magus
    DATA_MAGUS_IS_IDLE  = 1, // GetData: 1 while kneeling on the terrace
    DATA_MAGUS_ACTIVATE = 1, // SetData: value = prayer text group (0-2)

    // Queen Azshara
    DATA_MAGUS_DIED     = 1,

    // Hand of the Queen
    DATA_PUPPET_TARGET  = 1
};

enum Misc
{
    MAX_COUNCIL_MAGI    = 6,
    MAX_ACTIVE_MAGI     = 2
};

Position const VarothenBatCameoPos = { 3397.819f, -5225.655f, 251.321f, 5.7334f }; // 57117 bat-riding Varo'then vehicle
Position const VarothenCameoPos    = { 3403.721f, -5229.249f, 252.958f, 5.7381f }; // 57118

uint32 const MagusEntries[] = { NPC_ENCHANTED_MAGUS_FIRE, NPC_ENCHANTED_MAGUS_FROST, NPC_ENCHANTED_MAGUS_ARCANE };

uint32 GetChannelVisualForEntry(uint32 entry)
{
    switch (entry)
    {
        case NPC_ENCHANTED_MAGUS_FIRE:      return SPELL_FIRE_CHANNELING;
        case NPC_ENCHANTED_MAGUS_FROST:     return SPELL_FROST_CHANNELING;
        case NPC_ENCHANTED_MAGUS_ARCANE:    return SPELL_ARCANE_CHANNELING;
        default:                            return 0;
    }
}

Creature* FindAzshara(Creature* source, InstanceScript* instance)
{
    if (Creature* azshara = instance->GetCreature(BOSS_QUEEN_AZSHARA))
        return azshara;

    // The instance script may not expose Azshara through ObjectData yet - fall back to a grid scan
    return source->FindNearestCreature(NPC_QUEEN_AZSHARA, 250.f);
}

struct boss_queen_azshara : public BossAI
{
    boss_queen_azshara(Creature* creature) : BossAI(creature, BOSS_QUEEN_AZSHARA),
        _introStage(0), _activationCount(0), _finished(false), _outOfCombatTimer(1000) { }

    void JustAppeared() override
    {
        // She departed for good when the council fell - the Royal Cache & friendly RP are the instance script's job
        if (instance->GetBossState(BOSS_QUEEN_AZSHARA) == DONE)
        {
            me->DespawnOrUnsummon();
            return;
        }

        CollectCouncil();
    }

    void Reset() override
    {
        _Reset();
        _finished = false;
        _activationCount = 0;
        me->SetReactState(REACT_PASSIVE);
        me->MakeInterruptable(false);
        if (!me->HasAura(SPELL_SHROUD_OF_LUMINOSITY))
            DoCastSelf(SPELL_SHROUD_OF_LUMINOSITY, true);
    }

    void JustEngagedWith(Unit* who) override
    {
        BossAI::JustEngagedWith(who);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 1);

        CollectCouncil();
        ActivateNextMagus();

        events.ScheduleEvent(EVENT_DUMMY_NUKE, 1s + 200ms);
        events.ScheduleEvent(EVENT_ACTIVATE_MAGUS, 17s);            // DBM: first extra magus at 17 s, waves every 36 s
        events.ScheduleEvent(EVENT_SERVANT_OF_THE_QUEEN, 24s);      // DBM: first at 24 s
        events.ScheduleEvent(EVENT_TOTAL_OBEDIENCE, 36s);           // DBM: first at 36 s
    }

    void EnterEvadeMode(EvadeReason why) override
    {
        if (_finished)
            return;

        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        CleanupPuppets();
        DoCastAOE(SPELL_CLEAR_ALL_STATUS_AILMENTS, true);
        ResetCouncil();
        _activationCount = 0;
        me->MakeInterruptable(false);
        summons.DespawnAll();
        ScriptedAI::EnterEvadeMode(why);
    }

    void SetData(uint32 type, uint32 /*value*/) override
    {
        if (type != DATA_MAGUS_DIED || _finished || instance->GetBossState(BOSS_QUEEN_AZSHARA) != IN_PROGRESS)
            return;

        if (CountLivingMagi() == 0)
        {
            Finish();
            return;
        }

        Talk(SAY_MAGUS_DEATH);
        ActivateNextMagus();
        events.RescheduleEvent(EVENT_ACTIVATE_MAGUS, 36s);
    }

    void OnSpellCastFinished(SpellInfo const* spell, SpellFinishReason reason) override
    {
        if (spell->Id != SPELL_TOTAL_OBEDIENCE || _finished || instance->GetBossState(BOSS_QUEEN_AZSHARA) != IN_PROGRESS)
            return;

        me->MakeInterruptable(false);
        events.ScheduleEvent(EVENT_TOTAL_OBEDIENCE, 40s);

        if (reason == SPELL_FINISHED_CANCELED)
        {
            Talk(SAY_INTERRUPT);
            return;
        }

        if (reason != SPELL_FINISHED_SUCCESSFUL_CAST)
            return;

        // Interrupt failed: everybody becomes a puppet. 103241 lands its own charm aura on the group;
        // couple each player to a Hand of the Queen through Servant of the Queen and finish off
        // everyone who is still enslaved a few seconds later (retail 'interrupt or die').
        DoCastAOE(SPELL_TOTAL_OBEDIENCE_AOE, true);
        for (MapReference const& ref : me->GetMap()->GetPlayers())
            if (Player* player = ref.GetSource())
                if (player->IsAlive() && !player->IsGameMaster())
                    me->CastSpell(player, SPELL_SERVANT_OF_THE_QUEEN, true);

        scheduler.Schedule(6s, [this](TaskContext)
        {
            for (MapReference const& ref : me->GetMap()->GetPlayers())
                if (Player* player = ref.GetSource())
                    if (player->IsAlive() && (player->HasAura(SPELL_SERVANT_OF_THE_QUEEN) || player->HasAura(SPELL_TOTAL_OBEDIENCE)))
                        Unit::Kill(me, player, false);
        });
    }

    void UpdateAI(uint32 diff) override
    {
        scheduler.Update(diff);

        if (_finished)
            return;

        if (!me->IsEngaged())
            UpdateOutOfCombat(diff);

        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_DUMMY_NUKE:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 150.f, true))
                        DoCast(target, SPELL_DUMMY_NUKE); // verify in walkthrough - generic DBC name, observed as her 1.22 s filler
                    events.Repeat(1s + 200ms);
                    break;
                case EVENT_ACTIVATE_MAGUS:
                    ActivateNextMagus();
                    events.Repeat(36s);
                    break;
                case EVENT_SERVANT_OF_THE_QUEEN:
                    // 4.3.4 mechanic (not in the modern sniff): only ever one Hand at a time
                    if (me->FindNearestCreature(NPC_HAND_OF_THE_QUEEN, 250.f, true))
                    {
                        events.Repeat(10s);
                        break;
                    }
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, NonTankTargetSelector(me)))
                    {
                        Talk(SAY_PUPPETS);
                        DoCast(target, SPELL_SERVANT_OF_THE_QUEEN); // the 102334 aura script summons + couples the Hand
                    }
                    events.Repeat(30s);
                    break;
                case EVENT_TOTAL_OBEDIENCE:
                    // Stagger against Servant of the Queen so the casts do not overlap
                    if (events.GetTimeUntilEvent(EVENT_SERVANT_OF_THE_QUEEN) < 12 * IN_MILLISECONDS)
                        events.RescheduleEvent(EVENT_SERVANT_OF_THE_QUEEN, 15s);
                    Talk(SAY_PUPPETS);
                    Talk(EMOTE_TOTAL_OBEDIENCE);
                    me->MakeInterruptable(true); // she casts while damage-immune; school lockouts must still land
                    DoCastAOE(SPELL_TOTAL_OBEDIENCE);
                    // next cast is scheduled from OnSpellCastFinished (interrupt or completion)
                    break;
                default:
                    break;
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
        }
    }

private:
    void UpdateOutOfCombat(uint32 diff)
    {
        if (_outOfCombatTimer > diff)
        {
            _outOfCombatTimer -= diff;
            return;
        }
        _outOfCombatTimer = 1000;

        if (!me->IsAlive() || instance->GetBossState(BOSS_QUEEN_AZSHARA) == DONE)
            return;

        Player* nearest = nullptr;
        float nearestDist = 60.f;
        for (MapReference const& ref : me->GetMap()->GetPlayers())
        {
            Player* player = ref.GetSource();
            if (!player || !player->IsAlive() || player->IsGameMaster())
                continue;

            float dist = me->GetExactDist(player);
            if (dist < nearestDist)
            {
                nearest = player;
                nearestDist = dist;
            }
        }

        if (!nearest)
            return;

        if (_introStage == 0)
        {
            _introStage = 1;
            Talk(SAY_INTRO_1);
            scheduler.Schedule(10s, [this](TaskContext)
            {
                Talk(SAY_INTRO_2);
                _introStage = 2;
            });
        }
        else if (_introStage == 2 && nearestDist <= 40.f && me->IsWithinLOSInMap(nearest))
            DoZoneInCombat(); // nothing on the terrace is attackable pre-pull - proximity is the trigger
    }

    void CollectCouncil()
    {
        if (_magusGuids.size() == MAX_COUNCIL_MAGI)
            return;

        _magusGuids.clear();
        for (uint32 entry : MagusEntries)
        {
            std::list<Creature*> magi;
            me->GetCreatureListWithEntryInGrid(magi, entry, 200.f);
            for (Creature* magus : magi)
                _magusGuids.push_back(magus->GetGUID());
        }
    }

    uint8 CountLivingMagi()
    {
        uint8 count = 0;
        for (ObjectGuid guid : _magusGuids)
            if (Creature* magus = ObjectAccessor::GetCreature(*me, guid))
                if (magus->IsAlive())
                    ++count;
        return count;
    }

    void ActivateNextMagus()
    {
        std::vector<Creature*> idle;
        std::vector<uint32> activeEntries;
        for (ObjectGuid guid : _magusGuids)
        {
            Creature* magus = ObjectAccessor::GetCreature(*me, guid);
            if (!magus || !magus->IsAlive() || !magus->IsAIEnabled())
                continue;

            if (magus->AI()->GetData(DATA_MAGUS_IS_IDLE))
                idle.push_back(magus);
            else
                activeEntries.push_back(magus->GetEntry());
        }

        if (activeEntries.size() >= MAX_ACTIVE_MAGI)
            return;

        // The active pair is never of the same school
        idle.erase(std::remove_if(idle.begin(), idle.end(), [&activeEntries](Creature const* magus)
        {
            return std::find(activeEntries.begin(), activeEntries.end(), magus->GetEntry()) != activeEntries.end();
        }), idle.end());

        if (idle.empty())
            return;

        Creature* magus = Trinity::Containers::SelectRandomContainerElement(idle);
        magus->AI()->SetData(DATA_MAGUS_ACTIVATE, std::min<uint32>(_activationCount, 2));
        ++_activationCount;
    }

    void ResetCouncil()
    {
        for (ObjectGuid guid : _magusGuids)
        {
            Creature* magus = ObjectAccessor::GetCreature(*me, guid);
            if (!magus)
                continue;

            if (!magus->IsAlive())
                magus->Respawn(true);
            else if (magus->IsAIEnabled() && !magus->AI()->GetData(DATA_MAGUS_IS_IDLE))
                magus->AI()->DoAction(ACTION_MAGUS_RESET);
        }
    }

    void CleanupPuppets()
    {
        // Removing Servant of the Queen despawns the coupled Hands through the aura script
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SERVANT_OF_THE_QUEEN);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_TOTAL_OBEDIENCE);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_TOTAL_OBEDIENCE_AOE);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_PUPPET_STRING);

        uint32 const strayEntries[] = { NPC_HAND_OF_THE_QUEEN, NPC_ARCANE_BOMB_AIR, NPC_ARCANE_BOMB_GROUND };
        for (uint32 entry : strayEntries)
        {
            std::list<Creature*> strays;
            me->GetCreatureListWithEntryInGrid(strays, entry, 250.f);
            for (Creature* stray : strays)
                stray->DespawnOrUnsummon();
        }
    }

    void Finish()
    {
        _finished = true;
        events.Reset();
        me->MakeInterruptable(false);

        CleanupPuppets();
        Talk(SAY_DEFEAT); // DBM detects the kill on this yell
        DoCastAOE(SPELL_CLEAR_ALL_STATUS_AILMENTS, true);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        DoCastAOE(SPELL_AZSHARA_EVENT_CREDIT, true); // drives instance_encounters creditType 1 + quest credit
        instance->SetBossState(BOSS_QUEEN_AZSHARA, DONE); // no-death completion - set state directly
        me->CombatStop(true);

        // Departure RP - the Royal Cache, friendly handmaidens and bronze drakes are the instance script's job
        scheduler.Schedule(13s, [this](TaskContext)
        {
            Talk(SAY_RIDERS);
            if (Creature* bat = me->SummonCreature(NPC_VAROTHEN_SHADOWBAT_CAMEO, VarothenBatCameoPos, TEMPSUMMON_TIMED_DESPAWN, 25s))
            {
                bat->SetDisableGravity(true);
                _batCameoGuid = bat->GetGUID();
            }
            if (Creature* varothen = me->SummonCreature(NPC_VAROTHEN_CAMEO, VarothenCameoPos, TEMPSUMMON_TIMED_DESPAWN, 25s))
            {
                varothen->SetDisableGravity(true);
                _varothenCameoGuid = varothen->GetGUID();
            }
        });
        scheduler.Schedule(17s, [this](TaskContext)
        {
            if (Creature* bat = ObjectAccessor::GetCreature(*me, _batCameoGuid))
                if (bat->IsAIEnabled())
                    bat->AI()->Talk(SAY_AT_YOUR_SIDE);
        });
        scheduler.Schedule(24s, [this](TaskContext)
        {
            Talk(SAY_VAROTHEN_DISPOSE);
        });
        scheduler.Schedule(32s, [this](TaskContext)
        {
            if (Creature* bat = ObjectAccessor::GetCreature(*me, _batCameoGuid))
                bat->DespawnOrUnsummon();
            if (Creature* varothen = ObjectAccessor::GetCreature(*me, _varothenCameoGuid))
                varothen->DespawnOrUnsummon();
            me->DespawnOrUnsummon();
        });
    }

    uint8 _introStage;
    uint8 _activationCount;
    bool _finished;
    uint32 _outOfCombatTimer;
    std::vector<ObjectGuid> _magusGuids;
    ObjectGuid _batCameoGuid;
    ObjectGuid _varothenCameoGuid;
};

struct npc_enchanted_magus : public ScriptedAI
{
    npc_enchanted_magus(Creature* creature) : ScriptedAI(creature), _instance(creature->GetInstanceScript()), _active(false)
    {
        SetCombatMovement(false); // terrace casters - they fight from where they kneel
    }

    void Reset() override
    {
        if (_instance->GetBossState(BOSS_QUEEN_AZSHARA) == DONE)
        {
            me->DespawnOrUnsummon();
            return;
        }

        _active = false;
        _events.Reset();
        _scheduler.CancelAll();
        me->SetReactState(REACT_PASSIVE);
        me->SetImmuneToPC(true);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        if (uint32 visual = GetChannelVisualForEntry(me->GetEntry()))
            if (!me->HasAura(visual))
                DoCastSelf(visual, true); // kneeling channel cosmetic (also in creature_template_addon)
    }

    uint32 GetData(uint32 type) const override
    {
        if (type == DATA_MAGUS_IS_IDLE)
            return _active ? 0 : 1;

        return 0;
    }

    void SetData(uint32 type, uint32 value) override
    {
        if (type != DATA_MAGUS_ACTIVATE || _active)
            return;

        _active = true;
        if (uint32 visual = GetChannelVisualForEntry(me->GetEntry()))
            me->RemoveAurasDueToSpell(visual);
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        me->SetImmuneToPC(false);
        me->SetReactState(REACT_AGGRESSIVE);
        Talk(std::min<uint32>(value, 2));
        DoZoneInCombat();
        ScheduleSchoolEvents();
    }

    void DoAction(int32 action) override
    {
        if (action != ACTION_MAGUS_RESET || !_active)
            return;

        me->CombatStop(true);
        Reset();
    }

    void EnterEvadeMode(EvadeReason why) override
    {
        // A wipe resets the whole council through Azshara
        if (_active)
            if (Creature* azshara = FindAzshara(me, _instance))
                if (azshara->IsAIEnabled() && !azshara->IsInEvadeMode())
                    azshara->AI()->EnterEvadeMode(EVADE_REASON_OTHER);

        ScriptedAI::EnterEvadeMode(why);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (!_active)
            return;

        if (Creature* azshara = FindAzshara(me, _instance))
            if (azshara->IsAIEnabled())
                azshara->AI()->SetData(DATA_MAGUS_DIED, 1);
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);

        if (!UpdateVictim())
            return;

        _events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                // Fire
                case EVENT_FIREBALL:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 60.f, true))
                        DoCast(target, SPELL_FIREBALL);
                    _events.Repeat(2s + 500ms);
                    break;
                case EVENT_FIREBOMB:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 60.f, true))
                        DoCast(target, SPELL_FIREBOMB);
                    _events.Repeat(34s);
                    break;
                case EVENT_BLAST_WAVE:
                    if (SelectTarget(SELECT_TARGET_RANDOM, 0, 8.f, true))
                    {
                        DoCastSelf(SPELL_BLAST_WAVE);
                        _events.Repeat(25s);
                    }
                    else
                        _events.Repeat(5s);
                    break;
                // Frost
                case EVENT_ICE_FLING:
                    DoCastAOE(SPELL_ICE_FLING);
                    _events.Repeat(6s, 10s);
                    break;
                case EVENT_BLADES_OF_ICE:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 60.f, true))
                    {
                        DoCast(target, SPELL_BLADES_OF_ICE); // native CHARGE_DEST + triggered 102468
                        _scheduler.Schedule(1s + 200ms, [this](TaskContext)
                        {
                            DoCastSelf(SPELL_BLADES_OF_ICE_AOE, true); // arrival burst
                        });
                    }
                    _events.Repeat(20s);
                    break;
                case EVENT_COLDFLAME:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 60.f, true))
                    {
                        DoCast(target, SPELL_COLDFLAME);
                        float angle = me->GetAngle(target);
                        Position origin = me->GetPosition();
                        // March a line of Coldflame from the magus toward the target's position
                        _scheduler.Schedule(1s + 600ms, [this, angle, origin](TaskContext context)
                        {
                            float distance = 2.f * (context.GetRepeatCounter() + 1);
                            Position pos = origin;
                            pos.m_positionX += std::cos(angle) * distance;
                            pos.m_positionY += std::sin(angle) * distance;
                            me->UpdateGroundPositionZ(pos.m_positionX, pos.m_positionY, pos.m_positionZ);
                            me->CastSpell(pos, SPELL_COLDFLAME_LINE, true);
                            if (context.GetRepeatCounter() < 12)
                                context.Repeat(250ms);
                        });
                    }
                    _events.Repeat(25s);
                    break;
                // Arcane
                case EVENT_ARCANE_SHOCK:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 60.f, true))
                        DoCast(target, SPELL_ARCANE_SHOCK); // channel pulses 102464 natively
                    _events.Repeat(38s);
                    break;
                case EVENT_ARCANE_BOMB:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 60.f, true))
                    {
                        Position ground = target->GetPosition();
                        Position air = ground;
                        air.m_positionZ += 15.f;
                        me->SummonCreature(NPC_ARCANE_BOMB_GROUND, ground, TEMPSUMMON_TIMED_DESPAWN, 8s);
                        me->SummonCreature(NPC_ARCANE_BOMB_AIR, air, TEMPSUMMON_TIMED_DESPAWN, 8s);
                    }
                    _events.Repeat(36s);
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
    void ScheduleSchoolEvents()
    {
        switch (me->GetEntry())
        {
            case NPC_ENCHANTED_MAGUS_FIRE:
                _events.ScheduleEvent(EVENT_FIREBALL, 1ms);
                _events.ScheduleEvent(EVENT_FIREBOMB, 8s);
                _events.ScheduleEvent(EVENT_BLAST_WAVE, 10s);
                break;
            case NPC_ENCHANTED_MAGUS_FROST:
                _events.ScheduleEvent(EVENT_ICE_FLING, 6s);
                _events.ScheduleEvent(EVENT_BLADES_OF_ICE, 9s);
                _events.ScheduleEvent(EVENT_COLDFLAME, 15s);
                break;
            case NPC_ENCHANTED_MAGUS_ARCANE:
                _events.ScheduleEvent(EVENT_ARCANE_SHOCK, 10s);
                _events.ScheduleEvent(EVENT_ARCANE_BOMB, 18s);
                break;
            default:
                break;
        }
    }

    InstanceScript* _instance;
    bool _active;
    EventMap _events;
    TaskScheduler _scheduler;
};

struct npc_hand_of_the_queen : public ScriptedAI
{
    npc_hand_of_the_queen(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        // The puppet-master merely exists and is killed by the other players
        me->SetImmuneToPC(false);
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
    }

    void SetGUID(ObjectGuid const& guid, int32 id) override
    {
        if (id != DATA_PUPPET_TARGET)
            return;

        _playerGuid = guid;
        if (Player* player = ObjectAccessor::GetPlayer(*me, guid))
        {
            me->CastSpell(player, SPELL_RIDE_VEHICLE_HARDCODED, true); // rides ABOVE the player (player is the vehicle, 1745)
            me->CastSpell(player, SPELL_PUPPET_STRING, true);
            DoCastSelf(SPELL_PUPPET_STRING_SELF, true);
        }
        DoZoneInCombat();
    }

    ObjectGuid GetGUID(int32 id) const override
    {
        if (id == DATA_PUPPET_TARGET)
            return _playerGuid;

        return ObjectGuid::Empty;
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGuid))
        {
            player->RemoveAurasDueToSpell(SPELL_SERVANT_OF_THE_QUEEN);
            player->RemoveAurasDueToSpell(SPELL_TOTAL_OBEDIENCE);
            player->RemoveAurasDueToSpell(SPELL_PUPPET_STRING);
        }
    }

    void UpdateAI(uint32 /*diff*/) override { }

private:
    ObjectGuid _playerGuid;
};

struct npc_azshara_arcane_bomb : public NullCreatureAI
{
    npc_azshara_arcane_bomb(Creature* creature) : NullCreatureAI(creature) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        if (me->GetEntry() == NPC_ARCANE_BOMB_AIR)
        {
            me->SetDisableGravity(true);
            DoCastSelf(SPELL_ARCANE_BOMB_AIR_VISUAL, true);
        }
        else
        {
            DoCastSelf(SPELL_ARCANE_BOMB_GROUND_VISUAL, true);
            _events.ScheduleEvent(EVENT_DETONATE, 6s);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_DETONATE:
                    DoCastAOE(SPELL_ARCANE_BOMB_DETONATION);
                    if (Creature* air = me->FindNearestCreature(NPC_ARCANE_BOMB_AIR, 20.f))
                        air->DespawnOrUnsummon(500ms);
                    me->DespawnOrUnsummon(500ms);
                    break;
                default:
                    break;
            }
        }
    }

private:
    EventMap _events;
};

// 102334 - Servant of the Queen
class spell_azshara_servant_of_the_queen : public AuraScript
{
    void AfterApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetTarget();
        if (!caster)
            return;

        // Couple the puppet to a freshly risen Hand of the Queen
        if (Creature* hand = caster->SummonCreature(NPC_HAND_OF_THE_QUEEN, target->GetPosition(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5s))
            if (hand->IsAIEnabled())
                hand->AI()->SetGUID(target->GetGUID(), DATA_PUPPET_TARGET);
    }

    void AfterRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* target = GetTarget();
        target->RemoveAurasDueToSpell(SPELL_PUPPET_STRING);

        // The charm broke some other way (e.g. the player died) - the coupled Hand goes with it
        std::list<Creature*> hands;
        target->GetCreatureListWithEntryInGrid(hands, NPC_HAND_OF_THE_QUEEN, 200.f);
        for (Creature* hand : hands)
            if (hand->IsAlive() && hand->IsAIEnabled() && hand->AI()->GetGUID(DATA_PUPPET_TARGET) == target->GetGUID())
                hand->DespawnOrUnsummon();
    }

    void Register() override
    {
        AfterEffectApply.Register(&spell_azshara_servant_of_the_queen::AfterApply, EFFECT_0, SPELL_AURA_SET_VEHICLE_ID, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove.Register(&spell_azshara_servant_of_the_queen::AfterRemove, EFFECT_0, SPELL_AURA_SET_VEHICLE_ID, AURA_EFFECT_HANDLE_REAL);
    }
};
}

void AddSC_boss_queen_azshara()
{
    using namespace WellOfEternity;
    using namespace WellOfEternity::QueenAzshara;
    RegisterWellOfEternityCreatureAI(boss_queen_azshara);
    RegisterWellOfEternityCreatureAI(npc_enchanted_magus);
    RegisterWellOfEternityCreatureAI(npc_hand_of_the_queen);
    RegisterWellOfEternityCreatureAI(npc_azshara_arcane_bomb);
    RegisterSpellScript(spell_azshara_servant_of_the_queen);
}
