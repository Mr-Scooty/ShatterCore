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

#include "ScriptMgr.h"
#include "CellImpl.h"
#include "DBCStores.h"
#include "firelands.h"
#include "GridNotifiersImpl.h"
#include "InstanceScript.h"
#include "Map.h"
#include "MotionMaster.h"
#include "MoveSplineInit.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "PassiveAI.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "TemporarySummon.h"

namespace Firelands::Alysrazor
{
enum Texts
{
    // Egg Pile
    EMOTE_CRACKING_EGGS         = 0,    // The Molten Eggs begin to crack and splinter!
};

enum Spells
{
    // Harbinger of Flame
    SPELL_FIRE_IT_UP                            = 100093,
    SPELL_FIEROBLAST_TRASH                      = 100094,
    SPELL_FIEROCLAST_BARRAGE                    = 100095,
    SPELL_FIRE_CHANNELING                       = 100109,

    // Blazing Monstrosity
    SPELL_RIDE_MONSTROSITY                      = 93970,
    SPELL_SHARE_HEALTH_LEFT                     = 101502,
    SPELL_SHARE_HEALTH_RIGHT                    = 101503,
    SPELL_SLEEP_ULTRA_HIGH_PRIORITY             = 99480,
    SPELL_GENERIC_DUMMY_CAST                    = 100088,
    SPELL_LEFT_SIDE_SMACK_L                     = 100076,
    SPELL_RIGHT_SIDE_SMACK_L                    = 100078,
    SPELL_HEAD_BONK_L                           = 100080,
    SPELL_TICKLE_L                              = 100082,
    SPELL_KNOCKBACK_RIGHT                       = 100084,
    SPELL_KNOCKBACK_LEFT                        = 100085,
    SPELL_KNOCKBACK_FORWARD                     = 100086,
    SPELL_KNOCKBACK_BACK                        = 100087,
    SPELL_HEAD_BONK_R                           = 100089,
    SPELL_LEFT_SIDE_SMACK_R                     = 100090,
    SPELL_RIGHT_SIDE_SMACK_R                    = 100091,
    SPELL_TICKLE_R                              = 100092,
    SPELL_MOLTEN_BARRAGE_EFFECT_L               = 100071,
    SPELL_MOLTEN_BARRAGE_LEFT                   = 100072,
    SPELL_MOLTEN_BARRAGE_RIGHT                  = 100073,
    SPELL_MOLTEN_BARRAGE_EFFECT_R               = 100074,
    SPELL_MOLTEN_BARRAGE_VISUAL                 = 100075,
    SPELL_AGGRO_CLOSEST                         = 100462,
    SPELL_INVISIBILITY_AND_STEALTH_DETECTION    = 18950,

    // Egg Pile
    SPELL_SUMMON_SMOULDERING_HATCHLING          = 100096,
    SPELL_MOLTEN_EGG_TRASH_CALL_L               = 100097,
    SPELL_MOLTEN_EGG_TRASH_CALL_R               = 100098,
    SPELL_ALYSRAZOR_COSMETIC_EGG_XPLOSION       = 100099,
};

#define SPELL_SHARE_HEALTH          (me->GetEntry() == NPC_BLAZING_MONSTROSITY_LEFT ? SPELL_SHARE_HEALTH_LEFT : SPELL_SHARE_HEALTH_RIGHT)
#define SPELL_MOLTEN_BARRAGE        (me->GetEntry() == NPC_BLAZING_MONSTROSITY_LEFT ? SPELL_MOLTEN_BARRAGE_LEFT : SPELL_MOLTEN_BARRAGE_RIGHT)
#define SPELL_MOLTEN_BARRAGE_EFFECT (me->GetEntry() == NPC_BLAZING_MONSTROSITY_LEFT ? SPELL_MOLTEN_BARRAGE_EFFECT_L : SPELL_MOLTEN_BARRAGE_EFFECT_R)

enum Events
{
    // Blazing Monstrosity
    EVENT_START_SPITTING                = 1,
    EVENT_CONTINUE_SPITTING             = 2,

    // Harbinger of Flame
    EVENT_FIEROBLAST                    = 1,
    EVENT_FIEROCLAST_BARRAGE            = 2,

    // Egg Pile
    EVENT_SUMMON_SMOULDERING_HATCHLING  = 1,
};

enum MiscData
{
    MODEL_INVISIBLE_STALKER     = 11686,
    ANIM_KIT_BIRD_WAKE          = 1469,
    ANIM_KIT_BIRD_TURN          = 1473,
};

class RespawnEggEvent : public BasicEvent
{
    public:
        explicit RespawnEggEvent(Creature* egg) : _egg(egg) { }

        bool Execute(uint64 /*time*/, uint32 /*diff*/)
        {
            _egg->RestoreDisplayId();
            return true;
        }

    private:
        Creature* _egg;
};

class MoltenEggCheck
{
    public:
        explicit MoltenEggCheck(Creature* pile) : _eggPile(pile) { }

        bool operator()(Unit* object) const
        {
            if (object->GetEntry() != NPC_MOLTEN_EGG_TRASH)
                return false;

            if (object->GetDisplayId() != object->GetNativeDisplayId())
                return false;

            if (_eggPile->GetDistance2d(object) > 20.0f)
                return false;

            return true;
        }

    private:
        Creature* _eggPile;
};

class TrashRespawnWorker
{
    public:
        void operator()(Creature* creature) const
        {
            switch (creature->GetEntry())
            {
                case NPC_BLAZING_MONSTROSITY_LEFT:
                case NPC_BLAZING_MONSTROSITY_RIGHT:
                case NPC_EGG_PILE:
                case NPC_HARBINGER_OF_FLAME:
                case NPC_MOLTEN_EGG_TRASH:
                    if (!creature->IsAlive())
                        creature->Respawn(true);
                    break;
                case NPC_SMOULDERING_HATCHLING:
                    creature->DespawnOrUnsummon();
                    break;
            }
        }
};

static void AlysrazorTrashEvaded(Creature* creature)
{
    TrashRespawnWorker check;
    Trinity::CreatureWorker<TrashRespawnWorker> worker(creature, check);
    Cell::VisitGridObjects(creature, worker, SIZE_OF_GRIDS);
}

class npc_harbinger_of_flame : public CreatureScript
{
    public:
        npc_harbinger_of_flame() : CreatureScript("npc_harbinger_of_flame") { }

        struct npc_harbinger_of_flameAI : public ScriptedAI
        {
            npc_harbinger_of_flameAI(Creature* creature) : ScriptedAI(creature)
            {
            }

            void JustEngagedWith(Unit* /*target*/) override
            {
                if (Creature* bird = ObjectAccessor::GetCreature(*me, me->GetChannelObjectGuid()))
                    DoZoneInCombat(bird);

                me->InterruptSpell(CURRENT_CHANNELED_SPELL);
                _events.Reset();
                _events.ScheduleEvent(EVENT_FIEROBLAST, 1);
                _events.ScheduleEvent(EVENT_FIEROCLAST_BARRAGE, 6000);
            }

            void JustReachedHome() override
            {
                AlysrazorTrashEvaded(me);
            }

            void MoveInLineOfSight(Unit* unit) override
            {
                if (me->IsInCombat())
                    return;

                if (!unit->IsCharmedOwnedByPlayerOrPlayer())
                    return;

                ScriptedAI::MoveInLineOfSight(unit);
            }

            void UpdateAI(uint32 diff) override
            {
                if (!me->IsInCombat())
                    if (!me->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
                        if (Creature* fireBird = me->FindNearestCreature((me->GetHomePosition().GetPositionY() > -275.0f ? NPC_BLAZING_MONSTROSITY_LEFT : NPC_BLAZING_MONSTROSITY_RIGHT), 100.0f))
                            DoCast(fireBird, SPELL_FIRE_CHANNELING);

                if (!UpdateVictim())
                    return;

                _events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_FIEROBLAST:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, false, true, -SPELL_RIDE_MONSTROSITY))
                                DoCast(target, SPELL_FIEROBLAST_TRASH);
                            _events.RescheduleEvent(EVENT_FIEROBLAST, 500);  // cast time is longer, but thanks to UNIT_STATE_CASTING check it won't trigger more often (need this because this creature gets a stacking haste aura)
                            break;
                        case EVENT_FIEROCLAST_BARRAGE:
                            DoCastAOE(SPELL_FIEROCLAST_BARRAGE);
                            _events.ScheduleEvent(EVENT_FIEROCLAST_BARRAGE, urand(9000, 12000));
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

        private:
            EventMap _events;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_harbinger_of_flameAI(creature);
        }
};

class npc_blazing_monstrosity : public CreatureScript
{
    public:
        npc_blazing_monstrosity() : CreatureScript("npc_blazing_monstrosity") { }

        struct npc_blazing_monstrosityAI : public PassiveAI
        {
            npc_blazing_monstrosityAI(Creature* creature) : PassiveAI(creature), _summons(creature)
            {
            }

            void EnterEvadeMode(EvadeReason why) override
            {
                _summons.DespawnAll();
                _events.Reset();
                PassiveAI::EnterEvadeMode(why);
            }

            void JustDied(Unit* /*killer*/) override
            {
                _summons.DespawnAll();
                _events.Reset();
            }

            void JustReachedHome() override
            {
                AlysrazorTrashEvaded(me);
            }

            void JustEngagedWith(Unit* /*target*/) override
            {
                DoZoneInCombat();
                me->RemoveAurasDueToSpell(SPELL_SLEEP_ULTRA_HIGH_PRIORITY);
                me->PlayOneShotAnimKitId(ANIM_KIT_BIRD_WAKE);
                _events.Reset();
                _events.ScheduleEvent(EVENT_START_SPITTING, 6000);
                _events.ScheduleEvent(EVENT_CONTINUE_SPITTING, 9000);
            }

            void PassengerBoarded(Unit* passenger, int8 /*seat*/, bool apply) override
            {
                if (!apply)
                    return;

                // Our passenger is another vehicle (boardable by players)
                DoCast(passenger, SPELL_SHARE_HEALTH, true);
                passenger->SetFaction(35);
                passenger->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

                // Hack to relocate vehicle on vehicle so exiting players are not moved under map
                Movement::MoveSplineInit init(passenger);
                init.DisableTransportPathTransformations();
                init.MoveTo(0.6654003f, 0.0f, 1.9815f);
                init.SetFacing(0.0f);
                init.Launch();
            }

            void JustSummoned(Creature* summon) override
            {
                _summons.Summon(summon);
            }

            void SummonedCreatureDespawn(Creature* summon) override
            {
                _summons.Despawn(summon);
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
                        case EVENT_START_SPITTING:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, false, true, -SPELL_RIDE_MONSTROSITY))
                                DoCast(target, SPELL_MOLTEN_BARRAGE);
                            break;
                        case EVENT_CONTINUE_SPITTING:
                            DoCastAOE(SPELL_MOLTEN_BARRAGE_EFFECT);
                            if (Creature* egg = me->FindNearestCreature(NPC_EGG_PILE, 100.0f))
                                egg->AI()->DoAction(me->GetEntry());
                            break;
                    }
                }
            }

        private:
            SummonList _summons;
            EventMap _events;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_blazing_monstrosityAI(creature);
        }
};

class npc_molten_barrage : public CreatureScript
{
    public:
        npc_molten_barrage() : CreatureScript("npc_molten_barrage") { }

        struct npc_molten_barrageAI : public NullCreatureAI
        {
            npc_molten_barrageAI(Creature* creature) : NullCreatureAI(creature) { }

            void AttackStart(Unit* target) override
            {
                if (target)
                    me->GetMotionMaster()->MoveFollow(target, 0.0f, 0.0f, MOTION_SLOT_IDLE);
            }

            void IsSummonedBy(Unit* /*summoner*/) override
            {
                DoCastAOE(SPELL_AGGRO_CLOSEST, true);
                DoCast(me, SPELL_MOLTEN_BARRAGE_VISUAL);
                DoCast(me, SPELL_INVISIBILITY_AND_STEALTH_DETECTION, true);
            }

            void MovementInform(uint32 movementType, uint32 /*pointId*/) override
            {
                if (movementType != EFFECT_MOTION_TYPE)
                    return;

                DoCastAOE(SPELL_AGGRO_CLOSEST);
                me->ClearUnitState(UNIT_STATE_CANNOT_TURN);
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_molten_barrageAI(creature);
        }
};

class npc_egg_pile : public CreatureScript
{
    public:
        npc_egg_pile() : CreatureScript("npc_egg_pile") { }

        struct npc_egg_pileAI : public CreatureAI
        {
            npc_egg_pileAI(Creature* creature) : CreatureAI(creature)
            {
            }

            void AttackStart(Unit* /*target*/) override { }

            void Reset() override
            {
                me->SetReactState(REACT_PASSIVE);
                _events.Reset();
                _callHatchlingSpell = 0;
            }

            void JustDied(Unit* /*killer*/) override
            {
                _events.Reset();
                std::list<Creature*> eggs;
                GetCreatureListWithEntryInGrid(eggs, me, NPC_MOLTEN_EGG_TRASH, 20.0f);
                for (std::list<Creature*>::const_iterator itr = eggs.begin(); itr != eggs.end(); ++itr)
                    (*itr)->CastSpell(*itr, SPELL_ALYSRAZOR_COSMETIC_EGG_XPLOSION, TRIGGERED_FULL_MASK);

                DoCast(me, SPELL_ALYSRAZOR_COSMETIC_EGG_XPLOSION, true);
            }

            void JustReachedHome() override
            {
                AlysrazorTrashEvaded(me);
            }

            void DoAction(int32 action) override
            {
                if (action != NPC_BLAZING_MONSTROSITY_LEFT &&
                    action != NPC_BLAZING_MONSTROSITY_RIGHT)
                    return;

                if (action == NPC_BLAZING_MONSTROSITY_LEFT)
                    Talk(EMOTE_CRACKING_EGGS);

                _callHatchlingSpell = (action == NPC_BLAZING_MONSTROSITY_LEFT) ? SPELL_MOLTEN_EGG_TRASH_CALL_L : SPELL_MOLTEN_EGG_TRASH_CALL_R;
                DoZoneInCombat();
                _events.Reset();
                _events.ScheduleEvent(EVENT_SUMMON_SMOULDERING_HATCHLING, 1);
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
                        case EVENT_SUMMON_SMOULDERING_HATCHLING:
                        {
                            std::list<Creature*> eggs;
                            MoltenEggCheck check(me);
                            Trinity::CreatureListSearcher<MoltenEggCheck> searcher(me, eggs, check);
                            Cell::VisitGridObjects(me, searcher, 20.0f);

                            if (!eggs.empty())
                            {
                                Creature* egg = Trinity::Containers::SelectRandomContainerElement(eggs);
                                egg->CastSpell(egg, SPELL_SUMMON_SMOULDERING_HATCHLING, TRIGGERED_FULL_MASK);
                                egg->SetDisplayId(MODEL_INVISIBLE_STALKER);
                                egg->m_Events.AddEvent(new RespawnEggEvent(egg), egg->m_Events.CalculateTime(5000));
                            }

                            if (_callHatchlingSpell)
                                DoCastAOE(_callHatchlingSpell, true);
                            _events.ScheduleEvent(EVENT_SUMMON_SMOULDERING_HATCHLING, urand(6000, 10000));
                            break;
                        }
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

        private:
            EventMap _events;
            uint32 _callHatchlingSpell =0;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_egg_pileAI(creature);
        }
};

class spell_alysrazor_cosmetic_egg_xplosion : public SpellScriptLoader
{
    public:
        spell_alysrazor_cosmetic_egg_xplosion() : SpellScriptLoader("spell_alysrazor_cosmetic_egg_xplosion") { }

        class spell_alysrazor_cosmetic_egg_xplosion_SpellScript : public SpellScript
        {
            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                if (!sCreatureDisplayInfoStore.LookupEntry(MODEL_INVISIBLE_STALKER))
                    return false;
                return true;
            }

            void HandleExplosion(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                GetHitUnit()->SetDisplayId(MODEL_INVISIBLE_STALKER);
                if (Creature* creature = GetHitCreature())
                    creature->DespawnOrUnsummon(4000);
            }

            void Register() override
            {
                OnEffectHitTarget.Register(&spell_alysrazor_cosmetic_egg_xplosion_SpellScript::HandleExplosion, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_alysrazor_cosmetic_egg_xplosion_SpellScript();
        }
};

class spell_alysrazor_turn_monstrosity : public SpellScriptLoader
{
    public:
        spell_alysrazor_turn_monstrosity() : SpellScriptLoader("spell_alysrazor_turn_monstrosity") { }

        class spell_alysrazor_turn_monstrosity_SpellScript : public SpellScript
        {
            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo({ SPELL_GENERIC_DUMMY_CAST, SPELL_KNOCKBACK_RIGHT, SPELL_KNOCKBACK_LEFT, SPELL_KNOCKBACK_FORWARD, SPELL_KNOCKBACK_BACK });
            }

            void KnockBarrage(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                GetHitUnit()->GetMotionMaster()->MoveIdle();
                if (TempSummon* summ = GetHitUnit()->ToTempSummon())
                    if (Unit* summoner = summ->GetSummoner())
                        GetHitUnit()->CastSpell(summoner, SPELL_GENERIC_DUMMY_CAST, TRIGGERED_FULL_MASK);

                float angle = 0.0f;
                if (Unit* bird = GetCaster()->GetVehicleBase())
                {
                    bird->SetOrientationTowards(GetHitUnit());
                    angle = bird->GetOrientation();
                }

                uint32 spellId = 0;
                switch (GetSpellInfo()->Id)
                {
                    case SPELL_RIGHT_SIDE_SMACK_R:
                    case SPELL_RIGHT_SIDE_SMACK_L:
                        spellId = SPELL_KNOCKBACK_RIGHT;
                        angle -= float(M_PI) * 0.5f;
                        break;
                    case SPELL_LEFT_SIDE_SMACK_R:
                    case SPELL_LEFT_SIDE_SMACK_L:
                        spellId = SPELL_KNOCKBACK_LEFT;
                        angle += float(M_PI) * 0.5f;
                        break;
                    case SPELL_HEAD_BONK_R:
                    case SPELL_HEAD_BONK_L:
                        spellId = SPELL_KNOCKBACK_FORWARD;
                        break;
                    case SPELL_TICKLE_R:
                    case SPELL_TICKLE_L:
                        spellId = SPELL_KNOCKBACK_BACK;
                        angle -= float(M_PI);
                        break;
                }

                // Cannot wait for object update to process facing spline, it's needed in next spell cast
                GetHitUnit()->SetOrientation(angle);
                GetHitUnit()->SetFacingTo(angle);
                GetHitUnit()->AddUnitState(UNIT_STATE_CANNOT_TURN);
                GetHitUnit()->CastSpell(GetHitUnit(), spellId, TRIGGERED_FULL_MASK);
            }

            void TurnBird(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                GetHitUnit()->PlayOneShotAnimKitId(ANIM_KIT_BIRD_TURN);
            }

            void Register() override
            {
                OnEffectHitTarget.Register(&spell_alysrazor_turn_monstrosity_SpellScript::KnockBarrage, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
                OnEffectHitTarget.Register(&spell_alysrazor_turn_monstrosity_SpellScript::TurnBird, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_alysrazor_turn_monstrosity_SpellScript();
        }
};

class spell_alysrazor_aggro_closest : public SpellScriptLoader
{
    public:
        spell_alysrazor_aggro_closest() : SpellScriptLoader("spell_alysrazor_aggro_closest") { }

        class spell_alysrazor_aggro_closest_SpellScript : public SpellScript
        {
            bool Load() override
            {
                return GetCaster()->GetTypeId() == TYPEID_UNIT;
            }

            void HandleEffect(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                float curThreat = GetCaster()->GetThreatManager().GetThreat(GetHitUnit(), true);
                GetCaster()->GetThreatManager().AddThreat(GetHitUnit(), -curThreat + 50000.0f / std::min(1.0f, GetCaster()->GetDistance(GetHitUnit())), GetSpellInfo(), true, true);
            }

            void UpdateThreat()
            {
                GetCaster()->ClearUnitState(UNIT_STATE_CASTING);
                GetCaster()->GetAI()->AttackStart(GetCaster()->ToCreature()->SelectVictim());
            }

            void Register() override
            {
                OnEffectHitTarget.Register(&spell_alysrazor_aggro_closest_SpellScript::HandleEffect, EFFECT_0, SPELL_EFFECT_DUMMY);
                AfterCast.Register(&spell_alysrazor_aggro_closest_SpellScript::UpdateThreat);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_alysrazor_aggro_closest_SpellScript();
        }
};

class spell_alysrazor_fieroblast : public SpellScriptLoader
{
    public:
        spell_alysrazor_fieroblast() : SpellScriptLoader("spell_alysrazor_fieroblast") { }

        class spell_alysrazor_fieroblast_SpellScript : public SpellScript
        {
            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo({ SPELL_FIRE_IT_UP });
            }

            void FireItUp()
            {
                GetCaster()->CastSpell(GetCaster(), SPELL_FIRE_IT_UP, TRIGGERED_FULL_MASK);
            }

            void Register() override
            {
                AfterCast.Register(&spell_alysrazor_fieroblast_SpellScript::FireItUp);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_alysrazor_fieroblast_SpellScript();
        }
};

// ========================================================================
// ===== Alysrazor encounter (boss 52530) =================================
// ========================================================================
// Timers sourced from the 4.3.4 DBM module (authoritative), geometry from
// Firelands sniffs (flight ring, fissures, spawn heights). See
// sql/updates/world/4.3.4/2026_07_06_08_world.sql for the DB side.

enum EncounterSpells
{
    // Intro
    SPELL_SMOLDERING_ROOTS              = 100559,

    // Boss - stage control (pull cosmetics/energy setup are the sniffed encounter spells)
    SPELL_ZERO_POWER_ZERO_REGEN         = 99905,   // "Zero Power + Zero Regen" (encounter variant)
    SPELL_ENERGY_FULL                   = 99920,   // "Alysrazor Energy Full"
    SPELL_FIRE_HAWK_SMOKE               = 100712,  // pull cosmetic
    SPELL_ANIM_REPLACEMENT_SET          = 99595,   // pull cosmetic
    SPELL_PULL_FIRESTORM                = 99605,   // engage blast: raid damage + knockback, triggers 99606 pulses
    SPELL_BURNOUT                       = 99432,   // +50% damage taken (DBC), transform
    SPELL_IGNITED                       = 99922,   // native +2 energy/s + Blazing Buffet pulse
    SPELL_FULL_POWER                    = 99925,   // native raid damage + knockback
    SPELL_BLAZING_CLAW                  = 99843,   // self aura, pulses 99844 frontal every 1.5s natively
    SPELL_FIRESTORM                     = 100744,  // channel, periodic trigger -> 100745 raid damage
    SPELL_MOLTING                       = 99464,   // self aura, natively triggers 99465 feather summons; 25N variant 100698
    SPELL_BERSERK                       = 26662,

    // Player flight system (alternate-power driven: bar aura 101410, clicks
    // grant 97128 -> 98734 energize +1 feather power; bar click casts 98624)
    SPELL_MOLTEN_FEATHER_AURA           = 97128,
    SPELL_MOLTEN_FEATHER_BAR            = 101410,
    SPELL_WINGS_FLY_CARRIER             = 98619,   // FLY + flight speed, periodically re-applied by 98624
    SPELL_WINGS_OF_FLAME                = 98624,
    SPELL_BLAZING_POWER                 = 99461,   // stacking haste, triggered on players by orb aura 99462
    SPELL_BLAZING_POWER_PULSE           = 99462,   // orb self-aura, pulses 99461 every 500ms
    SPELL_ALYSRAS_RAZOR                 = 100029,

    // Stage 1 hazards and adds
    SPELL_INCENDIARY_CLOUD              = 99426,
    SPELL_INCENDIARY_CLOUD_DAMAGE       = 99427,
    SPELL_FIEROBLAST                    = 101223,
    SPELL_BRUSHFIRE_SUMMON              = 98884,
    SPELL_BRUSHFIRE_DAMAGE              = 98885,
    SPELL_IMPRINTED                     = 99389,
    SPELL_HUNGRY                        = 99361,
    SPELL_TANTRUM                       = 99362,
    SPELL_SATIATED                      = 99359,   // heroic variants via chain 3841
    SPELL_GUSHING_WOUND_APPLY           = 100024,  // chain 3829
    SPELL_GUSHING_WOUND                 = 99308,
    SPELL_LAVA_SPEW                     = 99335,

    // Stage 2
    SPELL_FIERY_VORTEX_DAMAGE           = 99794,
    SPELL_FIERY_TORNADO_DAMAGE          = 99816,   // chain 3832
    SPELL_HARSH_WINDS                   = 100640,

    // Stage 3
    SPELL_IGNITION                      = 99919,

    // Heroic
    SPELL_CATACLYSM                     = 100761,  // 5s cast, SEND_EVENT; raid damage script-driven
    SPELL_HERALD_RITUAL                 = 99199,   // Ritual of the Flame channel visual
    SPELL_EXPLOSIVE_SUICIDE             = 91738,   // Herald dies after Cataclysm
    SPELL_METEOR_CALL                   = 99564,   // cosmetic fire portal
    SPELL_METEORIC_IMPACT               = 99558,

    // Intro (Majordomo Staghelm 54015)
    SPELL_STAGHELM_TRANSFORM            = 100565   // "Fandral Transform" cat exit
};

enum EncounterTexts
{
    // Alysrazor (52530)
    SAY_AGGRO               = 0,
    SAY_SKIES_ARE_MINE      = 1,
    EMOTE_FLY_CIRCLE        = 2,
    EMOTE_FIERY_VORTEX      = 3,
    EMOTE_BURNOUT           = 4,
    SAY_BURNOUT             = 5,
    EMOTE_IGNITED           = 6,
    EMOTE_FULL_POWER        = 7,
    SAY_FULL_POWER          = 8,
    SAY_FIRESTORM           = 9,
    SAY_DEATH               = 10,
    SAY_KILL                = 11,

    // Majordomo Staghelm intro (54015)
    SAY_INTRO_1             = 0,
    SAY_INTRO_2             = 1,
    SAY_INTRO_3             = 2,

    // Blazing Talon Initiate / Herald / Broodmother / Egg / Worm
    SAY_ADD_SPAWN           = 0
};

enum EncounterEvents
{
    // Intro / first-cycle opener (boss side; sniff-timed low pass)
    EVENT_INTRO_CROSS = 100,     // keep clear of trash event ids in this file
    EVENT_INTRO_ENGAGE,
    EVENT_OPENING_CLAW,
    EVENT_MOLT_PASS,
    EVENT_RING_ASCEND,

    // Stage 1
    EVENT_MOLTING,
    EVENT_INITIATE_WAVE,
    EVENT_BROODMOTHERS,
    EVENT_LAVA_WORMS,
    EVENT_FLIGHT_HAZARD,
    EVENT_SUMMON_HERALD,
    EVENT_METEOR,
    EVENT_FIRESTORM,
    EVENT_RESUME_FLIGHT,
    EVENT_STAGE_TWO,

    // Stage 2
    EVENT_STAGE_THREE,

    // Stage 3 / 4
    EVENT_ENERGY_TICK,
    EVENT_BLAZING_CLAW,

    // Global
    EVENT_BERSERK,
    EVENT_CHECK_EVADE
};

enum EncounterPhases
{
    PHASE_NONE      = 0,
    PHASE_INTRO     = 1,
    PHASE_FLIGHT    = 2,
    PHASE_TORNADO   = 3,
    PHASE_BURNOUT   = 4,
    PHASE_REIGNITED = 5
};

enum EncounterPoints
{
    POINT_INTRO_FLYBY   = 10,
    POINT_INTRO_LAND,
    POINT_TAKEOFF,
    POINT_LAND_BURNOUT,
    POINT_INITIATE_LAND,
    POINT_EGG_LAND,
    POINT_BROODMOTHER_PERCH,
    POINT_CLAWSHAPER_LAND,
    POINT_METEOR_LAND,
    POINT_METEOR_ROLL_END
};

enum EncounterData
{
    // AI-level data (achievement + stage 3 channel bookkeeping)
    DATA_BARREL_ROLL        = 1,
    DATA_IGNITION_CHANNELS  = 2
};

enum EncounterActions
{
    ACTION_BARREL_ROLL_FAIL = 1,
    ACTION_INTRO_DONE       = 2
};

enum EncounterMisc
{
    MAX_FIRE_ENERGY     = 100,
    IGNITED_ENERGY      = 50,
    SIDE_BOTH           = 0,
    SIDE_EAST           = 1,
    SIDE_WEST           = 2
};

// Sniff-verified flight geometry: 16-point cyclic circle, clockwise,
// 24.4 s per lap (12.79 yd/s), constant height.
Position const FlightRingCenter     = { -37.66f, -279.50f, 130.2321f };
float const FLIGHT_RING_RADIUS      = 50.0f;
float const FLIGHT_RING_VELOCITY    = 12.79f;
float const BLAZING_POWER_Z         = 130.44f;   // orbs sit on the ring
float const ARENA_GROUND_Z          = 54.7f;

Position const ArenaCenterGround    = { -41.40f, -271.26f, 54.69f, 0.0f };
Position const AlysrazorIntroSpawn  = { 90.07f, -390.44f, 21.55f, 2.409f };
Position const VortexHoverPos       = { -37.66f, -279.50f, 85.0f };

// Sniff-verified lava fissure positions (worm spawns)
Position const LavaWormPositions[] =
{
    { -71.64f, -272.52f, 55.48f, 0.0f },
    { -45.65f, -306.56f, 54.86f, 0.0f },
    { -36.89f, -246.40f, 54.78f, 0.0f },
    { -11.64f, -278.01f, 53.11f, 0.0f }
};

// Initiate wave cadence [DBM]: first wave at 27s (13.5s on later cycles),
// deltas below between subsequent waves, sides Both,Both,E,W,E,W.
uint32 const InitiateWaveDelaysNormal[] = { 31, 31, 21, 21, 21 };
uint32 const InitiateWaveDelaysHeroic[] = { 22, 63, 21, 21, 40 };
uint8 const InitiateWaveSides[]         = { SIDE_BOTH, SIDE_BOTH, SIDE_EAST, SIDE_WEST, SIDE_EAST, SIDE_WEST };
uint8 const MAX_INITIATE_WAVES          = 6;

struct boss_alysrazor : public BossAI
{
    boss_alysrazor(Creature* creature) : BossAI(creature, DATA_ALYSRAZOR),
        _cycleCount(0), _initiateWave(0), _moltCount(0), _firestormCount(0),
        _ignitionChannels(0), _barrelRoll(true), _ignited(false)
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
        _cycleCount = 0;
        _initiateWave = 0;
        _moltCount = 0;
        _firestormCount = 0;
        _ignitionChannels = 0;
        _barrelRoll = true;
        _ignited = false;
        me->SetReactState(REACT_PASSIVE);
        SetFlying(true);
        me->SetPower(POWER_ENERGY, MAX_FIRE_ENERGY);
    }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        // Summoned by the Staghelm intro at the terrace mouth; takes off,
        // crosses into the arena and engages while flying [sniff timeline].
        SetFlying(true);
        events.SetPhase(PHASE_INTRO);
        Position ascent = me->GetPosition();
        ascent.m_positionZ += 100.0f;
        me->GetMotionMaster()->MoveTakeoff(POINT_INTRO_FLYBY, ascent, 20.0f);
        events.ScheduleEvent(EVENT_INTRO_CROSS, 2500ms, 0, PHASE_INTRO);
    }

    void JustEngagedWith(Unit* who) override
    {
        BossAI::JustEngagedWith(who);
        Talk(SAY_AGGRO);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

        // Energy/power setup + pull cosmetics (sniffed engage volley)
        me->SetPower(POWER_ENERGY, MAX_FIRE_ENERGY);
        DoCastSelf(SPELL_ZERO_POWER_ZERO_REGEN, true);
        DoCastSelf(SPELL_ENERGY_FULL, true);
        DoCastSelf(SPELL_FIRE_HAWK_SMOKE, true);
        DoCastSelf(SPELL_ANIM_REPLACEMENT_SET, true);
        DoCastAOE(SPELL_MOLTEN_FEATHER_BAR, true);  // feather alt-power bar on the raid
        DoCastAOE(SPELL_PULL_FIRESTORM, true);      // engage blast, natively pulses 99606

        events.ScheduleEvent(EVENT_BERSERK, 10min);
        events.ScheduleEvent(EVENT_CHECK_EVADE, 5s);
        ScheduleStageOne(true);
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
        Talk(SAY_DEATH);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        DespawnMoltenFeathers();
    }

    void KilledUnit(Unit* victim) override
    {
        if (victim->IsPlayer())
            Talk(SAY_KILL);
    }

    void EnterEvadeMode(EvadeReason /*why*/) override
    {
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        DespawnMoltenFeathers();
        summons.DespawnAll();
        _DespawnAtEvade();
    }

    void DoAction(int32 action) override
    {
        switch (action)
        {
            case ACTION_BARREL_ROLL_FAIL:
                _barrelRoll = false;
                break;
            default:
                break;
        }
    }

    uint32 GetData(uint32 type) const override
    {
        switch (type)
        {
            case DATA_BARREL_ROLL:
                return _barrelRoll ? 1 : 0;
            case DATA_IGNITION_CHANNELS:
                return _ignitionChannels;
            default:
                return 0;
        }
    }

    void SetData(uint32 type, uint32 value) override
    {
        if (type == DATA_IGNITION_CHANNELS)
        {
            if (value)
                ++_ignitionChannels;
            else if (_ignitionChannels > 0)
                --_ignitionChannels;
        }
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != POINT_MOTION_TYPE && type != EFFECT_MOTION_TYPE)
            return;

        switch (pointId)
        {
            case POINT_TAKEOFF:
                StartFlightLoop();
                break;
            case POINT_LAND_BURNOUT:
                BeginBurnout();
                break;
            default:
                break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!events.IsInPhase(PHASE_INTRO) && !UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = events.ExecuteEvent())
            HandleEvent(eventId);

        if (events.IsInPhase(PHASE_REIGNITED) || (events.IsInPhase(PHASE_FLIGHT) && !me->IsFlying()))
            DoMeleeAttackIfReady();
    }

private:
    void HandleEvent(uint32 eventId)
    {
        switch (eventId)
        {
            // ----- Intro / first-cycle opener [sniff timeline] -----------
            case EVENT_INTRO_CROSS:
                // Cross into the arena at altitude and engage mid-flight
                me->GetMotionMaster()->MovePoint(POINT_INTRO_FLYBY, 5.97f, -310.50f, 79.50f, false);
                events.ScheduleEvent(EVENT_INTRO_ENGAGE, 3s, 0, PHASE_INTRO);
                break;
            case EVENT_INTRO_ENGAGE:
                events.SetPhase(PHASE_NONE);
                DoZoneInCombat();
                break;
            case EVENT_OPENING_CLAW:
                // +14.6s: claw pulse while descending for the feather pass
                DoCastSelf(SPELL_BLAZING_CLAW, true);
                me->GetMotionMaster()->MovePoint(0, 5.97f, -310.50f, 65.58f, false);
                break;
            case EVENT_MOLT_PASS:
                // +16s: low pass toward the west rim, feathers trail behind
                // (99464 natively volleys 99465 -> summons 53089 at dest)
                DoCastSelf(SPELL_MOLTING, true);
                ++_moltCount;
                me->GetMotionMaster()->MovePoint(0, -75.60f, -246.92f, 70.23f, false);
                break;
            case EVENT_RING_ASCEND:
            {
                Position ringPoint = NearestRingPoint();
                me->GetMotionMaster()->MoveTakeoff(POINT_TAKEOFF, ringPoint, 15.0f);
                break;
            }

            // ----- Stage 1 ----------------------------------------------
            case EVENT_MOLTING:
                DoCastSelf(SPELL_MOLTING, true);
                ++_moltCount;
                if (!IsHeroic() && _moltCount < 3)
                    events.ScheduleEvent(EVENT_MOLTING, 60s, 0, PHASE_FLIGHT);
                break;
            case EVENT_INITIATE_WAVE:
                SummonInitiateWave(InitiateWaveSides[_initiateWave]);
                ++_initiateWave;
                if (_initiateWave < MAX_INITIATE_WAVES)
                    events.ScheduleEvent(EVENT_INITIATE_WAVE, Seconds(IsHeroic() ? InitiateWaveDelaysHeroic[_initiateWave - 1] : InitiateWaveDelaysNormal[_initiateWave - 1]), 0, PHASE_FLIGHT);
                break;
            case EVENT_BROODMOTHERS:
                SummonBroodmothers();
                events.ScheduleEvent(EVENT_BROODMOTHERS, 50s, 0, PHASE_FLIGHT);
                break;
            case EVENT_LAVA_WORMS:
                SummonLavaWorms();
                events.ScheduleEvent(EVENT_LAVA_WORMS, 60s, 0, PHASE_FLIGHT);
                break;
            case EVENT_FLIGHT_HAZARD:
                SpawnFlightHazard();
                events.ScheduleEvent(EVENT_FLIGHT_HAZARD, 2s, 0, PHASE_FLIGHT);
                break;
            case EVENT_SUMMON_HERALD:
                // One Herald per Cataclysm: ritual -> 5s cast -> suicide [sniff]
                SummonHerald();
                events.ScheduleEvent(EVENT_SUMMON_HERALD, 31s, 0, PHASE_FLIGHT);
                break;
            case EVENT_METEOR:
                SummonMeteor();
                break;
            case EVENT_FIRESTORM:
                Talk(SAY_FIRESTORM);
                me->StopMoving();
                DoCastSelf(SPELL_FIRESTORM);
                ++_firestormCount;
                events.ScheduleEvent(EVENT_RESUME_FLIGHT, 11s, 0, PHASE_FLIGHT);
                if (_firestormCount % 2 != 0) // two per flight stage [DBM]
                {
                    events.ScheduleEvent(EVENT_METEOR, 73s, 0, PHASE_FLIGHT);
                    events.ScheduleEvent(EVENT_FIRESTORM, 83s, 0, PHASE_FLIGHT);
                }
                break;
            case EVENT_RESUME_FLIGHT:
                StartFlightLoop();
                break;
            case EVENT_STAGE_TWO:
                BeginStageTwo();
                break;

            // ----- Stage 2 ----------------------------------------------
            case EVENT_STAGE_THREE:
                BeginStageThreeDescent();
                break;

            // ----- Stage 3 / 4 ------------------------------------------
            case EVENT_ENERGY_TICK:
                HandleEnergyTick();
                break;
            case EVENT_BLAZING_CLAW:
                // Self aura; pulses the 99844 frontal cleave every 1.5s natively.
                // Re-applied periodically so the pressure never lapses in Stage 4.
                DoCastSelf(SPELL_BLAZING_CLAW, true);
                events.ScheduleEvent(EVENT_BLAZING_CLAW, 15s, 0, PHASE_REIGNITED);
                break;

            // ----- Global -----------------------------------------------
            case EVENT_BERSERK:
                DoCastSelf(SPELL_BERSERK, true);
                break;
            case EVENT_CHECK_EVADE:
                if (!AnyPlayerAlive())
                {
                    EnterEvadeMode(EVADE_REASON_NO_HOSTILES);
                    return;
                }
                events.ScheduleEvent(EVENT_CHECK_EVADE, 5s);
                break;
            default:
                break;
        }
    }

    void ScheduleStageOne(bool firstCycle)
    {
        events.SetPhase(PHASE_FLIGHT);
        _initiateWave = 0;
        _moltCount = 0;
        _firestormCount = 0;
        me->SetPower(POWER_ENERGY, MAX_FIRE_ENERGY);

        if (firstCycle)
        {
            // Sniff-timed opener: claw pulse at +14.6s during the descent,
            // molt/feather pass at +16s, then ascend to the flight ring
            events.ScheduleEvent(EVENT_OPENING_CLAW, 14500ms, 0, PHASE_FLIGHT);
            events.ScheduleEvent(EVENT_MOLT_PASS, 16s, 0, PHASE_FLIGHT);
            events.ScheduleEvent(EVENT_RING_ASCEND, 26s, 0, PHASE_FLIGHT);
            if (!IsHeroic())
                events.ScheduleEvent(EVENT_MOLTING, 76s, 0, PHASE_FLIGHT); // molt pass was #1
        }
        else
            events.ScheduleEvent(EVENT_MOLTING, 16s, 0, PHASE_FLIGHT);
        events.ScheduleEvent(EVENT_INITIATE_WAVE, firstCycle ? 27s : 13500ms, 0, PHASE_FLIGHT);
        events.ScheduleEvent(EVENT_FLIGHT_HAZARD, firstCycle ? 33s : 5s, 0, PHASE_FLIGHT);
        events.ScheduleEvent(EVENT_LAVA_WORMS, 43s, 0, PHASE_FLIGHT);

        // Broodmothers drop eggs ~10s before the DBM hatch bars
        if (IsHeroic())
            events.ScheduleEvent(EVENT_BROODMOTHERS, firstCycle ? 32s : 12s, 0, PHASE_FLIGHT);
        else
            events.ScheduleEvent(EVENT_BROODMOTHERS, firstCycle ? 37s : 22s, 0, PHASE_FLIGHT);

        if (IsHeroic())
        {
            events.ScheduleEvent(EVENT_SUMMON_HERALD, firstCycle ? 20s : 10s, 0, PHASE_FLIGHT);
            events.ScheduleEvent(EVENT_METEOR, firstCycle ? 84s : 60s, 0, PHASE_FLIGHT);
            events.ScheduleEvent(EVENT_FIRESTORM, firstCycle ? 94s : 70s, 0, PHASE_FLIGHT);
            events.ScheduleEvent(EVENT_STAGE_TWO, firstCycle ? 243s : 225s, 0, PHASE_FLIGHT);
        }
        else
            events.ScheduleEvent(EVENT_STAGE_TWO, firstCycle ? 196s : 179s, 0, PHASE_FLIGHT);
    }

    void SetFlying(bool apply)
    {
        me->SetCanFly(apply);
        me->SetDisableGravity(apply);
        if (!apply)
            me->SetHomePosition(me->GetPosition());
    }

    bool AnyPlayerAlive() const
    {
        for (auto const& ref : me->GetMap()->GetPlayers())
            if (Player* player = ref.GetSource())
                if (player->IsAlive() && !player->IsGameMaster())
                    return true;
        return false;
    }

    Position NearestRingPoint() const
    {
        float angle = FlightRingCenter.GetAngle(me);
        return Position(FlightRingCenter.GetPositionX() + FLIGHT_RING_RADIUS * std::cos(angle),
                        FlightRingCenter.GetPositionY() + FLIGHT_RING_RADIUS * std::sin(angle),
                        FlightRingCenter.GetPositionZ());
    }

    void StartFlightLoop()
    {
        Talk(EMOTE_FLY_CIRCLE);
        me->GetMotionMaster()->MoveCirclePath(FlightRingCenter.GetPositionX(), FlightRingCenter.GetPositionY(),
            FlightRingCenter.GetPositionZ(), FLIGHT_RING_RADIUS, /*clockwise*/ true, 16, FLIGHT_RING_VELOCITY);
    }

    // Spawn hazards parametrically along the ring behind the boss - never
    // from me->GetPosition() (client-side extrapolation detaches trailing
    // spawns from the visible flight trail).
    void SpawnFlightHazard()
    {
        if (!me->IsFlying())
            return;

        float bossAngle = FlightRingCenter.GetAngle(me);
        // Motion is clockwise (angle decreasing), so "behind" is angle + offset
        float angle = bossAngle + float(M_PI) / 8.0f;

        if (++_hazardCounter % 2 == 0)
        {
            float x = FlightRingCenter.GetPositionX() + FLIGHT_RING_RADIUS * std::cos(angle);
            float y = FlightRingCenter.GetPositionY() + FLIGHT_RING_RADIUS * std::sin(angle);
            me->SummonCreature(NPC_BLAZING_POWER, x, y, BLAZING_POWER_Z, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 20000);
        }
        else
        {
            float radius = frand(42.0f, 58.0f);
            float x = FlightRingCenter.GetPositionX() + radius * std::cos(angle);
            float y = FlightRingCenter.GetPositionY() + radius * std::sin(angle);
            me->SummonCreature(NPC_INCENDIARY_CLOUD, x, y, frand(118.0f, 142.0f), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 20000);
        }
    }

    void DespawnMoltenFeathers()
    {
        std::list<Creature*> feathers;
        GetCreatureListWithEntryInGrid(feathers, me, NPC_MOLTEN_FEATHER, 200.0f);
        for (Creature* feather : feathers)
            feather->DespawnOrUnsummon();
    }

    // ----- Stage 1 summon helpers (implemented with Stage 1 adds) -------
    void SummonInitiateWave(uint8 side);
    void SummonBroodmothers();
    void SummonLavaWorms();
    void SummonHerald();
    void SummonMeteor();

    // ----- Stage transitions --------------------------------------------
    void BeginStageTwo()
    {
        events.SetPhase(PHASE_TORNADO);
        Talk(SAY_SKIES_ARE_MINE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        me->InterruptNonMeleeSpells(false);
        me->GetMotionMaster()->Clear();
        me->GetMotionMaster()->MovePoint(0, VortexHoverPos, false);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_WINGS_OF_FLAME);
        summons.DespawnEntry(NPC_HERALD_OF_THE_BURNING_END);
        SummonTornadoField();
        events.ScheduleEvent(EVENT_STAGE_THREE, 33500ms, 0, PHASE_TORNADO);
    }

    void SummonTornadoField(); // implemented with Stage 2 support NPCs

    void BeginStageThreeDescent()
    {
        events.SetPhase(PHASE_BURNOUT);
        summons.DespawnEntry(NPC_FIERY_TORNADO);
        summons.DespawnEntry(NPC_FIERY_VORTEX);
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        me->GetMotionMaster()->MoveLand(POINT_LAND_BURNOUT, ArenaCenterGround);
    }

    void BeginBurnout()
    {
        SetFlying(false);
        Talk(SAY_BURNOUT);
        Talk(EMOTE_BURNOUT);
        me->SetPower(POWER_ENERGY, 0);
        _ignited = false;
        DoCastSelf(SPELL_BURNOUT);
        SummonClawshapers();
        events.ScheduleEvent(EVENT_ENERGY_TICK, 1s);
    }

    void SummonClawshapers(); // implemented with Stage 3 support NPCs

    void HandleEnergyTick()
    {
        // Regen is native: each Ignition channel feeds +1/s (aura 21), Ignited
        // itself +2/s. This tick only monitors thresholds, plus a slow base
        // creep so a clawshaper-less Burnout can't stall forever (tunable).
        if (events.IsInPhase(PHASE_BURNOUT) && _ignitionChannels == 0 && (++_energyTickParity % 2) == 0)
            me->SetPower(POWER_ENERGY, std::min<int32>(me->GetPower(POWER_ENERGY) + 1, MAX_FIRE_ENERGY));

        if (!_ignited && me->GetPower(POWER_ENERGY) >= IGNITED_ENERGY)
        {
            _ignited = true;
            me->RemoveAurasDueToSpell(SPELL_BURNOUT);
            DoCastSelf(SPELL_IGNITED);
            Talk(EMOTE_IGNITED);
            events.SetPhase(PHASE_REIGNITED);
            me->SetReactState(REACT_AGGRESSIVE);
            events.ScheduleEvent(EVENT_BLAZING_CLAW, 2s, 0, PHASE_REIGNITED);
        }

        if (me->GetPower(POWER_ENERGY) >= MAX_FIRE_ENERGY)
        {
            BeginFullPower();
            return;
        }

        events.ScheduleEvent(EVENT_ENERGY_TICK, 1s);
    }

    void BeginFullPower()
    {
        Talk(EMOTE_FULL_POWER);
        Talk(SAY_FULL_POWER);
        DoCastSelf(SPELL_FULL_POWER);
        summons.DespawnEntry(NPC_BLAZING_TALON_CLAWSHAPER);
        me->AttackStop();
        me->SetReactState(REACT_PASSIVE);
        SetFlying(true);
        ++_cycleCount;
        Position ringPoint = NearestRingPoint();
        me->GetMotionMaster()->MoveTakeoff(POINT_TAKEOFF, ringPoint, 15.0f);
        ScheduleStageOne(false);
    }

    uint8 _cycleCount;
    uint8 _initiateWave;
    uint8 _moltCount;
    uint8 _firestormCount;
    uint8 _broodmotherSide = 0;
    uint8 _wormSetIndex = 0;
    uint32 _hazardCounter = 0;
    uint32 _energyTickParity = 0;
    int32 _ignitionChannels;
    bool _barrelRoll;
    bool _ignited;
};

// Majordomo Staghelm intro RP (54015): roots the raid, calls Alysrazor in,
// transforms and leaps away. Combat starts 35.5s after Smoldering Roots [DBM].
struct npc_alysrazor_majordomo_staghelm_intro : public ScriptedAI
{
    npc_alysrazor_majordomo_staghelm_intro(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript()), _started(false) { }

    void Reset() override
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void MoveInLineOfSight(Unit* who) override
    {
        if (_started || !who->IsPlayer() || me->GetDistance(who) > 50.0f)
            return;

        if (_instance->GetBossState(DATA_ALYSRAZOR) == DONE || _instance->GetBossState(DATA_ALYSRAZOR) == IN_PROGRESS)
            return;

        StartIntro();
    }

    void DoAction(int32 action) override
    {
        if (action == ACTION_RESET_ALYSRAZOR_INTRO)
        {
            _started = false;
            me->SetVisible(true);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_INTRO_ROOTS:
                    DoCastAOE(SPELL_SMOLDERING_ROOTS);
                    break;
                case EVENT_INTRO_YELL_2:
                    Talk(SAY_INTRO_2);
                    break;
                case EVENT_INTRO_YELL_3:
                    Talk(SAY_INTRO_3);
                    break;
                case EVENT_INTRO_TRANSFORM:
                    DoCastSelf(SPELL_STAGHELM_TRANSFORM, true);
                    break;
                case EVENT_INTRO_LEAVE:
                    me->SetVisible(false);
                    break;
                case EVENT_INTRO_SUMMON_BOSS:
                    _instance->instance->SummonCreature(BOSS_ALYSRAZOR, AlysrazorIntroSpawn);
                    break;
                default:
                    break;
            }
        }
    }

private:
    enum IntroEvents
    {
        EVENT_INTRO_ROOTS = 1,
        EVENT_INTRO_YELL_2,
        EVENT_INTRO_YELL_3,
        EVENT_INTRO_TRANSFORM,
        EVENT_INTRO_LEAVE,
        EVENT_INTRO_SUMMON_BOSS
    };

    // Sniffed timeline (relative to the first yell): roots +3.5s, second
    // yell +8.5s, third yell +24.4s, Fandral Transform +27.8s, Alysrazor
    // created +35.2s; combat begins moments later.
    void StartIntro()
    {
        _started = true;
        Talk(SAY_INTRO_1);
        _events.ScheduleEvent(EVENT_INTRO_ROOTS, 3500ms);
        _events.ScheduleEvent(EVENT_INTRO_YELL_2, 8500ms);
        _events.ScheduleEvent(EVENT_INTRO_YELL_3, 24400ms);
        _events.ScheduleEvent(EVENT_INTRO_TRANSFORM, 27800ms);
        _events.ScheduleEvent(EVENT_INTRO_LEAVE, 30s);
        _events.ScheduleEvent(EVENT_INTRO_SUMMON_BOSS, 35s);
    }

    InstanceScript* _instance;
    EventMap _events;
    bool _started;
};

// ------------------------------------------------------------------------
// Side geometry [sniff-verified]. Entry-pair "1" (broodmother 53680 -> egg
// 53681 -> hatchling 53509) works the Y-positive perch; pair "2" (53900 ->
// 53899 -> 53898) the Y-negative perch. Broodmothers lift off near the
// center, fly up to their perch and release the egg, which sails down to a
// fixed ground landing near the center.
// ------------------------------------------------------------------------
Position const BroodmotherLiftoffPos[2] =
{
    { -50.44f, -266.25f, 77.33f, 0.0f },
    { -30.96f, -285.06f, 77.26f, 0.0f }
};

Position const BroodmotherPerchPos[2] =
{
    {   9.53f, -228.78f, 148.04f, 0.0f },
    { -38.35f, -355.38f, 147.97f, 0.0f }
};

Position const EggLandPositions[2] =
{
    { -46.96f, -266.20f, 55.04f, 0.0f },
    { -33.06f, -287.83f, 53.90f, 0.0f }
};

// Initiates spawn airborne (Z~97) and land at fixed points [sniff]
struct InitiateSpawnInfo
{
    uint32 Entry;
    Position AirSpawn;
    Position Landing;
};

InitiateSpawnInfo const InitiateSpawns[4] =
{
    { NPC_BLAZING_TALON_INITIATE_1, { -17.28f, -342.27f, 96.70f, 0.0f }, { -23.79f, -323.36f, 52.92f, 0.0f } },
    { NPC_BLAZING_TALON_INITIATE_1, {  23.16f, -267.84f, 96.72f, 0.0f }, {   3.31f, -270.27f, 54.10f, 0.0f } },
    { NPC_BLAZING_TALON_INITIATE_2, { -66.04f, -214.96f, 97.67f, 0.0f }, { -58.88f, -233.64f, 55.44f, 0.0f } },
    { NPC_BLAZING_TALON_INITIATE_2, { -106.54f, -292.63f, 96.63f, 0.0f }, { -87.22f, -287.45f, 55.25f, 0.0f } }
};

Position const HeraldSpawnPos = { -41.57f, -275.87f, 54.44f, 1.5f };   // arena center [sniff]

float const TornadoLaneRadii[3]     = { 15.0f, 27.0f, 40.0f };
uint8 const TornadoLaneCounts[3]    = { 4, 6, 8 };
float const TORNADO_REVOLUTION_SECS = 24.0f;
float const TORNADO_Z               = 55.5f;

// ----- boss_alysrazor summon helper definitions --------------------------

void boss_alysrazor::SummonInitiateWave(uint8 side)
{
    // Entry pair 1 (53896, table slots 0-1) plays "East", pair 2 (53369,
    // slots 2-3) plays "West" for the DBM side rotation. 10-player: one
    // initiate per active side (alternating slot); 25-player: both slots.
    for (uint8 s = 0; s < 2; ++s)
    {
        if (side != SIDE_BOTH && side != s + 1)
            continue;

        if (Is25ManRaid())
        {
            me->SummonCreature(InitiateSpawns[s * 2].Entry, InitiateSpawns[s * 2].AirSpawn);
            me->SummonCreature(InitiateSpawns[s * 2 + 1].Entry, InitiateSpawns[s * 2 + 1].AirSpawn);
        }
        else
            me->SummonCreature(InitiateSpawns[s * 2 + (_initiateWave % 2)].Entry, InitiateSpawns[s * 2 + (_initiateWave % 2)].AirSpawn);
    }
}

void boss_alysrazor::SummonBroodmothers()
{
    if (Is25ManRaid())
    {
        me->SummonCreature(NPC_BLAZING_BROODMOTHER_1, BroodmotherLiftoffPos[0]);
        me->SummonCreature(NPC_BLAZING_BROODMOTHER_2, BroodmotherLiftoffPos[1]);
    }
    else
    {
        // 10-player: one side per delivery, alternating
        uint8 side = _broodmotherSide++ % 2;
        me->SummonCreature(side == 0 ? NPC_BLAZING_BROODMOTHER_1 : NPC_BLAZING_BROODMOTHER_2, BroodmotherLiftoffPos[side]);
    }
}

void boss_alysrazor::SummonLavaWorms()
{
    uint8 count = Is25ManRaid() ? 4 : 2;
    uint8 first = _wormSetIndex++ % 4;
    for (uint8 i = 0; i < count; ++i)
        me->SummonCreature(NPC_PLUMP_LAVA_WORM, LavaWormPositions[(first + i) % 4]);
}

void boss_alysrazor::SummonHerald()
{
    me->SummonCreature(NPC_HERALD_OF_THE_BURNING_END, HeraldSpawnPos);
}

void boss_alysrazor::SummonMeteor()
{
    Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 60.0f, true);
    if (!target)
        target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true);
    if (target)
        me->SummonCreature(NPC_METEOR_CALLER, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 15000);
}

void boss_alysrazor::SummonTornadoField()
{
    Talk(EMOTE_FIERY_VORTEX);
    me->SummonCreature(NPC_FIERY_VORTEX, ArenaCenterGround);

    for (uint8 lane = 0; lane < 3; ++lane)
    {
        float radius = TornadoLaneRadii[lane];
        uint8 count = TornadoLaneCounts[lane];
        for (uint8 i = 0; i < count; ++i)
        {
            float angle = i * 2.0f * float(M_PI) / count;
            float x = FlightRingCenter.GetPositionX() + radius * std::cos(angle);
            float y = FlightRingCenter.GetPositionY() + radius * std::sin(angle);
            if (Creature* tornado = me->SummonCreature(NPC_FIERY_TORNADO, x, y, TORNADO_Z, 0.0f))
                tornado->GetMotionMaster()->MoveCirclePath(FlightRingCenter.GetPositionX(), FlightRingCenter.GetPositionY(),
                    TORNADO_Z, radius, lane % 2 == 0, 16, 2.0f * float(M_PI) * radius / TORNADO_REVOLUTION_SECS);
        }
    }
}

void boss_alysrazor::SummonClawshapers()
{
    uint8 count = Is25ManRaid() ? 4 : 2;
    for (uint8 i = 0; i < count; ++i)
    {
        float angle = i * 2.0f * float(M_PI) / count;
        float x = ArenaCenterGround.GetPositionX() + 10.0f * std::cos(angle);
        float y = ArenaCenterGround.GetPositionY() + 10.0f * std::sin(angle);
        me->SummonCreature(NPC_BLAZING_TALON_CLAWSHAPER, x, y, ARENA_GROUND_Z + 40.0f, angle + float(M_PI));
    }
}

// ----- Stage 1 support NPCs ----------------------------------------------

// Blazing Talon Initiate (53896/53369): flies in, lands, casts interruptible
// Fieroblast (uninterrupted -> stacking Fire It Up self-haste via spell
// script) and periodic Brushfire.
struct npc_alysrazor_blazing_talon_initiate : public ScriptedAI
{
    npc_alysrazor_blazing_talon_initiate(Creature* creature) : ScriptedAI(creature) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetCanFly(true);
        me->SetDisableGravity(true);
        // Fly to the sniffed landing point nearest to our air spawn
        Position const* landing = &InitiateSpawns[0].Landing;
        float best = 1.0e10f;
        for (InitiateSpawnInfo const& info : InitiateSpawns)
        {
            if (info.Entry != me->GetEntry())
                continue;
            float dist = me->GetExactDist2dSq(info.AirSpawn.GetPositionX(), info.AirSpawn.GetPositionY());
            if (dist < best)
            {
                best = dist;
                landing = &info.Landing;
            }
        }
        me->GetMotionMaster()->MoveLand(POINT_INITIATE_LAND, *landing);
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != EFFECT_MOTION_TYPE || pointId != POINT_INITIATE_LAND)
            return;

        me->SetCanFly(false);
        me->SetDisableGravity(false);
        me->SetReactState(REACT_AGGRESSIVE);
        Talk(SAY_ADD_SPAWN);
        DoZoneInCombat();
        _events.ScheduleEvent(EVENT_INITIATE_FIEROBLAST, 2s);
        _events.ScheduleEvent(EVENT_INITIATE_BRUSHFIRE, 15s);
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
                case EVENT_INITIATE_FIEROBLAST:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                        DoCast(target, SPELL_FIEROBLAST);
                    // Short reschedule; UNIT_STATE_CASTING guard paces actual casts
                    // (the creature gains stacking haste when left uninterrupted)
                    _events.RescheduleEvent(EVENT_INITIATE_FIEROBLAST, 500ms);
                    break;
                case EVENT_INITIATE_BRUSHFIRE:
                    DoCastSelf(SPELL_BRUSHFIRE_SUMMON);
                    _events.ScheduleEvent(EVENT_INITIATE_BRUSHFIRE, 15s, 20s);
                    break;
                default:
                    break;
            }
        }

        DoMeleeAttackIfReady();
    }

private:
    enum InitiateEvents
    {
        EVENT_INITIATE_FIEROBLAST = 1,
        EVENT_INITIATE_BRUSHFIRE
    };

    EventMap _events;
};

// Brushfire (53372): crawls in a straight line, burning everything it passes
// through (periodic 98885 on self; hits flag the barrel-roll fail).
struct npc_alysrazor_brushfire : public PassiveAI
{
    npc_alysrazor_brushfire(Creature* creature) : PassiveAI(creature) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        DoCastSelf(SPELL_BRUSHFIRE_DAMAGE, true);
        float angle = frand(0.0f, 2.0f * float(M_PI));
        Position dest = me->GetPosition();
        dest.m_positionX += 40.0f * std::cos(angle);
        dest.m_positionY += 40.0f * std::sin(angle);
        dest.m_positionZ = me->GetMap()->GetHeight(me->GetPhaseShift(), dest.GetPositionX(), dest.GetPositionY(), dest.GetPositionZ() + 5.0f);
        me->SetWalk(true);
        me->GetMotionMaster()->MovePoint(0, dest);
        me->DespawnOrUnsummon(20s);
    }
};

// Blazing Broodmother (53680/53900): lifts off near the center, flies up to
// its rim perch and releases a Molten Egg, which sails down to the arena
// floor [sniff geometry].
struct npc_alysrazor_blazing_broodmother : public PassiveAI
{
    npc_alysrazor_blazing_broodmother(Creature* creature) : PassiveAI(creature) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        me->SetCanFly(true);
        me->SetDisableGravity(true);
        DoCastSelf(SPELL_FIRE_HAWK_SMOKE, true);
        Talk(SAY_ADD_SPAWN);
        uint8 side = SideIndex();
        me->GetMotionMaster()->MovePoint(POINT_BROODMOTHER_PERCH, BroodmotherPerchPos[side], false);
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != POINT_MOTION_TYPE || pointId != POINT_BROODMOTHER_PERCH)
            return;

        uint32 eggEntry = me->GetEntry() == NPC_BLAZING_BROODMOTHER_1 ? NPC_MOLTEN_EGG_1 : NPC_MOLTEN_EGG_2;
        uint8 eggs = me->GetMap()->Is25ManRaid() ? 2 : 1;
        for (uint8 i = 0; i < eggs; ++i)
            me->SummonCreature(eggEntry, me->GetPositionX() + i * 3.0f, me->GetPositionY(), me->GetPositionZ() - 4.0f, 0.0f);

        me->DespawnOrUnsummon(15s);
    }

private:
    uint8 SideIndex() const
    {
        return me->GetEntry() == NPC_BLAZING_BROODMOTHER_1 ? 0 : 1;
    }
};

// Molten Egg (53681/53899): sails from the rim perch down to its fixed
// landing point, cracks, hatches into a Voracious Hatchling ~10s later.
struct npc_alysrazor_molten_egg : public PassiveAI
{
    npc_alysrazor_molten_egg(Creature* creature) : PassiveAI(creature) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        uint8 side = me->GetEntry() == NPC_MOLTEN_EGG_1 ? 0 : 1;
        Position land = EggLandPositions[side];
        // Small scatter so 25-player double drops don't stack perfectly
        land.m_positionX += frand(-4.0f, 4.0f);
        land.m_positionY += frand(-4.0f, 4.0f);
        me->SetCanFly(true);
        me->SetDisableGravity(true);
        me->GetMotionMaster()->MovePoint(POINT_EGG_LAND, land, false);
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != POINT_MOTION_TYPE || pointId != POINT_EGG_LAND)
            return;

        me->SetCanFly(false);
        me->SetDisableGravity(false);
        _events.ScheduleEvent(EVENT_EGG_CRACK, 5s);
        _events.ScheduleEvent(EVENT_EGG_HATCH, 10s);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_EGG_CRACK:
                    Talk(SAY_ADD_SPAWN);
                    break;
                case EVENT_EGG_HATCH:
                {
                    uint32 hatchling = me->GetEntry() == NPC_MOLTEN_EGG_1 ? NPC_VORACIOUS_HATCHLING_1 : NPC_VORACIOUS_HATCHLING_2;
                    me->SummonCreature(hatchling, me->GetPosition());
                    me->DespawnOrUnsummon(1s);
                    break;
                }
                default:
                    break;
            }
        }
    }

private:
    enum EggEvents
    {
        EVENT_EGG_CRACK = 1,
        EVENT_EGG_HATCH
    };

    EventMap _events;
};

// Voracious Hatchling (53509/53898): imprints on the nearest player and only
// ever attacks it; throws Tantrums when hungry, calmed by eating lava worms.
// Heroic: applies Gushing Wound to low-health victims.
struct npc_alysrazor_voracious_hatchling : public ScriptedAI
{
    npc_alysrazor_voracious_hatchling(Creature* creature) : ScriptedAI(creature) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        me->SetReactState(REACT_AGGRESSIVE);
        DoZoneInCombat();
        ImprintOnNearest();
        _events.ScheduleEvent(EVENT_HATCHLING_HUNGRY, 15s);
    }

    bool CanAIAttack(Unit const* target) const override
    {
        return _imprintTarget.IsEmpty() || target->GetGUID() == _imprintTarget;
    }

    void MoveInLineOfSight(Unit* who) override
    {
        if (_satiated || who->GetEntry() != NPC_PLUMP_LAVA_WORM || !me->IsWithinDist(who, 5.0f))
            return;

        // Om nom nom
        who->ToCreature()->DespawnOrUnsummon();
        me->RemoveAurasDueToSpell(SPELL_TANTRUM);
        me->RemoveAurasDueToSpell(SPELL_HUNGRY);
        DoCastSelf(SPELL_SATIATED);
        _satiated = true;
        _events.CancelEvent(EVENT_HATCHLING_TANTRUM);
        _events.ScheduleEvent(EVENT_HATCHLING_SATIATED_END, IsHeroic() ? 10s : 15s);
        _events.RescheduleEvent(EVENT_HATCHLING_HUNGRY, IsHeroic() ? 25s : 30s);
    }

    void DamageDealt(Unit* victim, uint32& /*damage*/, DamageEffectType /*damageType*/) override
    {
        if (IsHeroic() && victim->HealthBelowPct(50) && !victim->HasAura(SPELL_GUSHING_WOUND))
            DoCast(victim, SPELL_GUSHING_WOUND_APPLY, true);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        _events.Update(diff);

        // Re-imprint if the fixated player died or left
        Unit* imprinted = ObjectAccessor::GetUnit(*me, _imprintTarget);
        if (!imprinted || !imprinted->IsAlive())
            ImprintOnNearest();

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_HATCHLING_HUNGRY:
                    DoCastSelf(SPELL_HUNGRY, true);
                    _events.ScheduleEvent(EVENT_HATCHLING_TANTRUM, 8s);
                    break;
                case EVENT_HATCHLING_TANTRUM:
                    DoCastSelf(SPELL_TANTRUM, true);
                    break;
                case EVENT_HATCHLING_SATIATED_END:
                    _satiated = false;
                    break;
                default:
                    break;
            }
        }

        DoMeleeAttackIfReady();
    }

private:
    enum HatchlingEvents
    {
        EVENT_HATCHLING_HUNGRY = 1,
        EVENT_HATCHLING_TANTRUM,
        EVENT_HATCHLING_SATIATED_END
    };

    void ImprintOnNearest()
    {
        Unit* target = nullptr;
        // Nearest player within 5yd wins (the positioned off-tank), else the
        // closest player in the arena - retail griefing behavior, kept
        if (Player* nearby = me->SelectNearestPlayer(5.0f))
            target = nearby;
        else if (Player* closest = me->SelectNearestPlayer(200.0f))
            target = closest;

        if (!target)
            return;

        _imprintTarget = target->GetGUID();
        DoCast(target, SPELL_IMPRINTED, true);
        me->GetThreatManager().ResetAllThreat();
        me->GetThreatManager().AddThreat(target, 10000000.0f);
        AttackStart(target);
    }

    EventMap _events;
    ObjectGuid _imprintTarget;
    bool _satiated = false;
};

// Plump Lava Worm (53520): stationary hatchling snack, sweeps a rotating
// Lava Spew cone.
struct npc_alysrazor_plump_lava_worm : public PassiveAI
{
    npc_alysrazor_plump_lava_worm(Creature* creature) : PassiveAI(creature) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        Talk(SAY_ADD_SPAWN);
        me->SetControlled(true, UNIT_STATE_ROOT);
        _spewTimer = 2000;
        _rotateTimer = 500;
        _rotation = frand(0.0f, 2.0f * float(M_PI));
    }

    void UpdateAI(uint32 diff) override
    {
        // ~11s per full revolution, re-oriented every 500ms; the cone spell
        // reads caster orientation
        if (_rotateTimer <= diff)
        {
            _rotateTimer = 500;
            _rotation += 2.0f * float(M_PI) / 22.0f;
            if (_rotation > 2.0f * float(M_PI))
                _rotation -= 2.0f * float(M_PI);
            me->SetFacingTo(_rotation);
        }
        else
            _rotateTimer -= diff;

        if (_spewTimer <= diff)
        {
            _spewTimer = 2400;
            if (!me->HasUnitState(UNIT_STATE_CASTING))
                DoCastAOE(SPELL_LAVA_SPEW);
        }
        else
            _spewTimer -= diff;
    }

private:
    uint32 _spewTimer = 0;
    uint32 _rotateTimer = 0;
    float _rotation = 0.0f;
};

// Incendiary Cloud (53541): flight-path hazard, burns flying players.
struct npc_alysrazor_incendiary_cloud : public PassiveAI
{
    npc_alysrazor_incendiary_cloud(Creature* creature) : PassiveAI(creature),
        _instance(creature->GetInstanceScript()) { }

    void UpdateAI(uint32 diff) override
    {
        if (_checkTimer <= diff)
        {
            _checkTimer = 500;
            std::list<Player*> players;
            Trinity::AnyPlayerInObjectRangeCheck check(me, 6.0f);
            Trinity::PlayerListSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(me, players, check);
            Cell::VisitWorldObjects(me, searcher, 6.0f);
            for (Player* player : players)
            {
                if (player->HasAura(SPELL_INCENDIARY_CLOUD_DAMAGE))
                    continue;
                DoCast(player, SPELL_INCENDIARY_CLOUD, true);
                if (Creature* alysrazor = _instance->GetCreature(DATA_ALYSRAZOR))
                    if (alysrazor->IsAIEnabled())
                        alysrazor->AI()->DoAction(ACTION_BARREL_ROLL_FAIL);
            }
        }
        else
            _checkTimer -= diff;
    }

private:
    InstanceScript* _instance;
    uint32 _checkTimer = 500;
};

// Blazing Power (53554): flight ring. Its self-aura 99462 natively pulses
// 99461 (stacking haste) onto players in range every 500ms; the ring pops
// once a flying player takes it.
struct npc_alysrazor_blazing_power : public PassiveAI
{
    npc_alysrazor_blazing_power(Creature* creature) : PassiveAI(creature) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        DoCastSelf(SPELL_BLAZING_POWER_PULSE, true);
    }

    void UpdateAI(uint32 diff) override
    {
        if (_granted)
            return;

        if (_checkTimer <= diff)
        {
            _checkTimer = 250;
            std::list<Player*> players;
            Trinity::AnyPlayerInObjectRangeCheck check(me, 5.0f);
            Trinity::PlayerListSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(me, players, check);
            Cell::VisitWorldObjects(me, searcher, 5.0f);
            for (Player* player : players)
            {
                if (!player->HasAura(SPELL_WINGS_OF_FLAME))
                    continue;
                _granted = true;
                me->DespawnOrUnsummon(500ms);
                break;
            }
        }
        else
            _checkTimer -= diff;
    }

private:
    uint32 _checkTimer = 250;
    bool _granted = false;
};

// ----- Stage 2 support NPCs ----------------------------------------------

// Fiery Vortex (53693): central tornado; damages the inner circle and
// punishes airborne players with Harsh Winds.
struct npc_alysrazor_fiery_vortex : public PassiveAI
{
    npc_alysrazor_fiery_vortex(Creature* creature) : PassiveAI(creature) { }

    void UpdateAI(uint32 diff) override
    {
        if (_tickTimer <= diff)
        {
            _tickTimer = 1000;
            std::list<Player*> players;
            Trinity::AnyPlayerInObjectRangeCheck check(me, 200.0f);
            Trinity::PlayerListSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(me, players, check);
            Cell::VisitWorldObjects(me, searcher, 200.0f);
            for (Player* player : players)
            {
                if (me->IsWithinDist2d(player, 10.0f))
                {
                    DoCast(player, SPELL_FIERY_VORTEX_DAMAGE, true);
                    if (TempSummon* summon = me->ToTempSummon())
                        if (Unit* summoner = summon->GetSummoner())
                            if (summoner->IsAIEnabled())
                                summoner->GetAI()->DoAction(ACTION_BARREL_ROLL_FAIL);
                }

                // No flying over the tornado field
                if (player->GetPositionZ() > TORNADO_Z + 15.0f && !player->HasAura(SPELL_HARSH_WINDS))
                    DoCast(player, SPELL_HARSH_WINDS, true);
            }
        }
        else
            _tickTimer -= diff;
    }

private:
    uint32 _tickTimer = 1000;
};

// Fiery Tornado (53698): orbiting hazard, lane spline started by the boss.
struct npc_alysrazor_fiery_tornado : public PassiveAI
{
    npc_alysrazor_fiery_tornado(Creature* creature) : PassiveAI(creature) { }

    void UpdateAI(uint32 diff) override
    {
        if (_tickTimer <= diff)
        {
            _tickTimer = 1000;
            std::list<Player*> players;
            Trinity::AnyPlayerInObjectRangeCheck check(me, 5.0f);
            Trinity::PlayerListSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(me, players, check);
            Cell::VisitWorldObjects(me, searcher, 5.0f);
            for (Player* player : players)
            {
                DoCast(player, SPELL_FIERY_TORNADO_DAMAGE, true);
                if (TempSummon* summon = me->ToTempSummon())
                    if (Unit* summoner = summon->GetSummoner())
                        if (summoner->IsAIEnabled())
                            summoner->GetAI()->DoAction(ACTION_BARREL_ROLL_FAIL);
            }
        }
        else
            _tickTimer -= diff;
    }

private:
    uint32 _tickTimer = 1000;
};

// ----- Stage 3 support NPC -----------------------------------------------

// Blazing Talon Clawshaper (53734): flies in during Burnout and channels
// Ignition into the boss, refueling her energy (+2/s per live channel).
struct npc_alysrazor_blazing_talon_clawshaper : public ScriptedAI
{
    npc_alysrazor_blazing_talon_clawshaper(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript()) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetCanFly(true);
        me->SetDisableGravity(true);
        Position land = me->GetPosition();
        land.m_positionZ = ARENA_GROUND_Z;
        me->GetMotionMaster()->MoveLand(POINT_CLAWSHAPER_LAND, land);
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != EFFECT_MOTION_TYPE || pointId != POINT_CLAWSHAPER_LAND)
            return;

        me->SetCanFly(false);
        me->SetDisableGravity(false);
        DoZoneInCombat();
        _channelTimer = 500;
    }

    void UpdateAI(uint32 diff) override
    {
        if (!_channelTimer)
            return;

        if (_channelTimer <= diff)
        {
            _channelTimer = 2000;
            // Keep the Ignition channel up whenever it drops (interruptible
            // by death only; spell-side immunities come from the DB mask)
            if (!me->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
                if (Creature* alysrazor = _instance->GetCreature(DATA_ALYSRAZOR))
                    DoCast(alysrazor, SPELL_IGNITION);
        }
        else
            _channelTimer -= diff;
    }

private:
    InstanceScript* _instance;
    uint32 _channelTimer = 0;
};

// ----- Heroic support NPCs -----------------------------------------------

// Herald of the Burning End (53375): heroic. Appears at the arena center,
// performs the Ritual of the Flame, casts one wipe-grade interruptible
// Cataclysm (5s), then explodes [sniff: ritual -> 100761 -> 91738 suicide].
// The raid stops it by killing or interrupting the Herald.
struct npc_alysrazor_herald_of_the_burning_end : public ScriptedAI
{
    npc_alysrazor_herald_of_the_burning_end(Creature* creature) : ScriptedAI(creature) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        me->SetReactState(REACT_PASSIVE);
        Talk(SAY_ADD_SPAWN);
        DoZoneInCombat();
        DoCastSelf(SPELL_HERALD_RITUAL, true);
        _events.ScheduleEvent(EVENT_HERALD_CATACLYSM, 1s);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_HERALD_CATACLYSM:
                    DoCastSelf(SPELL_CATACLYSM);
                    _events.ScheduleEvent(EVENT_HERALD_SUICIDE, 6s);
                    break;
                case EVENT_HERALD_SUICIDE:
                    DoCastSelf(SPELL_EXPLOSIVE_SUICIDE, true);
                    me->DespawnOrUnsummon(3s);
                    break;
                default:
                    break;
            }
        }
    }

private:
    enum HeraldEvents
    {
        EVENT_HERALD_CATACLYSM = 1,
        EVENT_HERALD_SUICIDE
    };

    EventMap _events;
};

// Meteor Caller (53487): marks the impact point, then brings down the meteor.
struct npc_alysrazor_meteor_caller : public PassiveAI
{
    npc_alysrazor_meteor_caller(Creature* creature) : PassiveAI(creature) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        DoCastSelf(SPELL_METEOR_CALL, true);
        _impactTimer = 3000;
    }

    void UpdateAI(uint32 diff) override
    {
        if (!_impactTimer)
            return;

        if (_impactTimer <= diff)
        {
            _impactTimer = 0;
            me->SummonCreature(NPC_MOLTEN_METEOR, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ() + 40.0f, 0.0f);
        }
        else
            _impactTimer -= diff;
    }

private:
    uint32 _impactTimer = 0;
};

// Molten Meteor (53784): crashes down, rolls through the arena, then rests
// as a boulder that blocks Firestorm (LoS handled in the Firestorm script).
struct npc_alysrazor_molten_meteor : public PassiveAI
{
    npc_alysrazor_molten_meteor(Creature* creature) : PassiveAI(creature) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        me->GetMotionMaster()->MoveFall(POINT_METEOR_LAND);
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != EFFECT_MOTION_TYPE)
            return;

        if (pointId == POINT_METEOR_LAND)
        {
            DoCastAOE(SPELL_METEORIC_IMPACT, true);
            // Roll away from the arena center, through and past the raid
            float angle = FlightRingCenter.GetAngle(me);
            Position dest;
            dest.Relocate(FlightRingCenter.GetPositionX() + 85.0f * std::cos(angle),
                          FlightRingCenter.GetPositionY() + 85.0f * std::sin(angle),
                          ARENA_GROUND_Z);
            me->SetWalk(true);
            me->GetMotionMaster()->MovePoint(POINT_METEOR_ROLL_END, dest);
            // Rests as a boulder, then crumbles
            me->DespawnOrUnsummon(45s);
        }
    }
};

// ----- Encounter spell scripts -------------------------------------------

// 97128 - Molten Feather (spellclick on feather creature 53089).
// The DBC chain does the real work (97128 -> 98734 energize +1 feather
// alternate power, bar aura 101410 shows it, clicking the full bar casts
// 98624). This script enforces the 3-feather cap - a capped player leaves
// the feather on the ground for someone else - and despawns the pickup.
class spell_alysrazor_molten_feather_pickup : public SpellScript
{
    SpellCastResult CheckFeatherCap()
    {
        if (Unit* caster = GetCaster())
            if (caster->GetPower(POWER_ALTERNATE_POWER) >= 3)
                return SPELL_FAILED_CASTER_AURASTATE;
        return SPELL_CAST_OK;
    }

    void HandlePickup()
    {
        if (Unit* feather = GetExplTargetUnit())
            if (Creature* featherCreature = feather->ToCreature())
                if (featherCreature->GetEntry() == NPC_MOLTEN_FEATHER)
                    featherCreature->DespawnOrUnsummon(500ms);
    }

    void Register() override
    {
        OnCheckCast.Register(&spell_alysrazor_molten_feather_pickup::CheckFeatherCap);
        AfterCast.Register(&spell_alysrazor_molten_feather_pickup::HandlePickup);
    }
};

// 98624 - Wings of Flame: launches the player (native knockback) and
// periodically applies the 98619 fly carrier. Consumes the feathers on
// apply; soft landing on expiry.
class spell_alysrazor_wings_of_flame : public AuraScript
{
    void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* target = GetTarget();
        target->RemoveAurasDueToSpell(SPELL_MOLTEN_FEATHER_AURA);
        target->SetPower(POWER_ALTERNATE_POWER, 0);
    }

    void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        // Drop the fly carrier and give Slow Fall so mid-air expiry at ring
        // height isn't lethal
        GetTarget()->RemoveAurasDueToSpell(SPELL_WINGS_FLY_CARRIER);
        GetTarget()->CastSpell(GetTarget(), 130, true);
    }

    void Register() override
    {
        AfterEffectApply.Register(&spell_alysrazor_wings_of_flame::HandleApply, EFFECT_0, SPELL_AURA_ANY, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove.Register(&spell_alysrazor_wings_of_flame::HandleRemove, EFFECT_0, SPELL_AURA_ANY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 99461 - Blazing Power (pulsed on players by the ring's 99462 aura):
// stacking haste natively; refresh Wings of Flame and award Alysra's Razor
// at 25 stacks.
class spell_alysrazor_blazing_power : public SpellScript
{
    void HandleBuff()
    {
        Unit* player = GetHitUnit();
        if (!player || !player->IsPlayer())
            return;

        if (Aura* wings = player->GetAura(SPELL_WINGS_OF_FLAME))
            wings->RefreshDuration();

        if (Aura* stacks = player->GetAura(SPELL_BLAZING_POWER))
            if (stacks->GetStackAmount() >= 25 && !player->HasAura(SPELL_ALYSRAS_RAZOR))
                player->CastSpell(player, SPELL_ALYSRAS_RAZOR, true);
    }

    void Register() override
    {
        AfterHit.Register(&spell_alysrazor_blazing_power::HandleBuff);
    }
};

// 100761 - Cataclysm (heroic Herald): completing the 5s cast is intended to
// be a wipe. The DBC effect is only SEND_EVENT, so the raid damage is dealt
// here (value tunable).
class spell_alysrazor_cataclysm : public SpellScript
{
    void HandleWipeDamage()
    {
        Unit* caster = GetCaster();
        for (auto const& ref : caster->GetMap()->GetPlayers())
            if (Player* player = ref.GetSource())
                if (player->IsAlive() && !player->IsGameMaster())
                    Unit::DealDamage(caster, player, 300000, 0, SPELL_DIRECT_DAMAGE, SPELL_SCHOOL_MASK_FIRE);
    }

    void Register() override
    {
        AfterCast.Register(&spell_alysrazor_cataclysm::HandleWipeDamage);
    }
};

// 101223 - Fieroblast (encounter initiates): uninterrupted cast stacks
// Fire It Up self-haste, mirroring the trash gauntlet's 100094 script.
class spell_alysrazor_fieroblast_encounter : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_FIRE_IT_UP });
    }

    void FireItUp()
    {
        GetCaster()->CastSpell(GetCaster(), SPELL_FIRE_IT_UP, TRIGGERED_FULL_MASK);
    }

    void Register() override
    {
        AfterCast.Register(&spell_alysrazor_fieroblast_encounter::FireItUp);
    }
};

// 98885 - Brushfire damage: any player hit voids Do a Barrel Roll!
class spell_alysrazor_brushfire_damage : public SpellScript
{
    void HandleAchievementFail()
    {
        if (!GetHitPlayer())
            return;

        if (InstanceScript* instance = GetCaster()->GetInstanceScript())
            if (Creature* alysrazor = instance->GetCreature(DATA_ALYSRAZOR))
                if (alysrazor->IsAIEnabled())
                    alysrazor->AI()->DoAction(ACTION_BARREL_ROLL_FAIL);
    }

    void Register() override
    {
        AfterHit.Register(&spell_alysrazor_brushfire_damage::HandleAchievementFail);
    }
};

// 99308 - Gushing Wound (heroic): the bleed removes itself once the victim
// is healed back above 50%.
class spell_alysrazor_gushing_wound : public AuraScript
{
    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        if (GetTarget()->HealthAbovePct(50))
            Remove();
    }

    void Register() override
    {
        OnEffectPeriodic.Register(&spell_alysrazor_gushing_wound::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
    }
};

// 99919 - Ignition (Clawshaper channel): each live channel feeds the boss
// +2 energy/s in the Burnout energy tick.
class spell_alysrazor_ignition : public AuraScript
{
    void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Creature* boss = GetTarget()->ToCreature())
            if (boss->IsAIEnabled())
                boss->AI()->SetData(DATA_IGNITION_CHANNELS, 1);
    }

    void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Creature* boss = GetTarget()->ToCreature())
            if (boss->IsAIEnabled())
                boss->AI()->SetData(DATA_IGNITION_CHANNELS, 0);
    }

    void Register() override
    {
        AfterEffectApply.Register(&spell_alysrazor_ignition::HandleApply, EFFECT_0, SPELL_AURA_ANY, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove.Register(&spell_alysrazor_ignition::HandleRemove, EFFECT_0, SPELL_AURA_ANY, AURA_EFFECT_HANDLE_REAL);
    }
};

// Firestorm raid damage (heroic): players sheltered behind a resting Molten
// Meteor boulder take no tick. Trigger creatures do not block VMAP LoS, so
// the segment test is done here in 2D.
class spell_alysrazor_firestorm_damage : public SpellScript
{
    void FilterTargets(std::list<WorldObject*>& targets)
    {
        Unit* caster = GetCaster();
        std::list<Creature*> boulders;
        GetCreatureListWithEntryInGrid(boulders, caster, NPC_MOLTEN_METEOR, 150.0f);
        boulders.remove_if([](Creature* boulder) { return boulder->isMoving(); });

        if (boulders.empty())
            return;

        targets.remove_if([&](WorldObject* target) -> bool
        {
            for (Creature* boulder : boulders)
            {
                // 2D point-segment distance: does caster->target pass within
                // the boulder's shadow?
                float cx = caster->GetPositionX(), cy = caster->GetPositionY();
                float dx = target->GetPositionX() - cx, dy = target->GetPositionY() - cy;
                float bx = boulder->GetPositionX() - cx, by = boulder->GetPositionY() - cy;
                float lenSq = dx * dx + dy * dy;
                if (lenSq < 1.0f)
                    continue;
                float t = std::clamp((bx * dx + by * dy) / lenSq, 0.0f, 1.0f);
                float px = bx - t * dx, py = by - t * dy;
                if (px * px + py * py < 36.0f) // within 6yd of the segment
                    return true;
            }
            return false;
        });
    }

    void Register() override
    {
        OnObjectAreaTargetSelect.Register(&spell_alysrazor_firestorm_damage::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
    }
};

// 100640 - Harsh Winds: no flying during the Fiery Vortex.
class spell_alysrazor_harsh_winds : public AuraScript
{
    void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        GetTarget()->RemoveAurasDueToSpell(SPELL_WINGS_OF_FLAME);
    }

    void Register() override
    {
        AfterEffectApply.Register(&spell_alysrazor_harsh_winds::HandleApply, EFFECT_0, SPELL_AURA_ANY, AURA_EFFECT_HANDLE_REAL);
    }
};

// Do a Barrel Roll! (5813): no raid member struck by Fiery Vortex, Fiery
// Tornado, Incendiary Cloud or Brushfire for the whole fight.
class achievement_alysrazor_barrel_roll : public AchievementCriteriaScript
{
public:
    achievement_alysrazor_barrel_roll() : AchievementCriteriaScript("achievement_alysrazor_barrel_roll") { }

    bool OnCheck(Player* /*source*/, Unit* target) override
    {
        if (!target || !target->IsAIEnabled())
            return false;

        return target->GetAI()->GetData(DATA_BARREL_ROLL) != 0;
    }
};
}

void AddSC_boss_alysrazor()
{
    using namespace Firelands;
    using namespace Firelands::Alysrazor;
    // Pre-boss trash gauntlet
    new npc_harbinger_of_flame();
    new npc_blazing_monstrosity();
    new npc_molten_barrage();
    new npc_egg_pile();
    new spell_alysrazor_cosmetic_egg_xplosion();
    new spell_alysrazor_turn_monstrosity();
    new spell_alysrazor_aggro_closest();
    new spell_alysrazor_fieroblast();

    // Alysrazor encounter
    RegisterFirelandsCreatureAI(boss_alysrazor);
    RegisterFirelandsCreatureAI(npc_alysrazor_majordomo_staghelm_intro);
    RegisterFirelandsCreatureAI(npc_alysrazor_blazing_talon_initiate);
    RegisterFirelandsCreatureAI(npc_alysrazor_brushfire);
    RegisterFirelandsCreatureAI(npc_alysrazor_blazing_broodmother);
    RegisterFirelandsCreatureAI(npc_alysrazor_molten_egg);
    RegisterFirelandsCreatureAI(npc_alysrazor_voracious_hatchling);
    RegisterFirelandsCreatureAI(npc_alysrazor_plump_lava_worm);
    RegisterFirelandsCreatureAI(npc_alysrazor_incendiary_cloud);
    RegisterFirelandsCreatureAI(npc_alysrazor_blazing_power);
    RegisterFirelandsCreatureAI(npc_alysrazor_fiery_vortex);
    RegisterFirelandsCreatureAI(npc_alysrazor_fiery_tornado);
    RegisterFirelandsCreatureAI(npc_alysrazor_blazing_talon_clawshaper);
    RegisterFirelandsCreatureAI(npc_alysrazor_herald_of_the_burning_end);
    RegisterFirelandsCreatureAI(npc_alysrazor_meteor_caller);
    RegisterFirelandsCreatureAI(npc_alysrazor_molten_meteor);
    RegisterSpellScript(spell_alysrazor_molten_feather_pickup);
    RegisterSpellScript(spell_alysrazor_wings_of_flame);
    RegisterSpellScript(spell_alysrazor_blazing_power);
    RegisterSpellScript(spell_alysrazor_fieroblast_encounter);
    RegisterSpellScript(spell_alysrazor_brushfire_damage);
    RegisterSpellScript(spell_alysrazor_gushing_wound);
    RegisterSpellScript(spell_alysrazor_ignition);
    RegisterSpellScript(spell_alysrazor_firestorm_damage);
    RegisterSpellScript(spell_alysrazor_cataclysm);
    RegisterSpellScript(spell_alysrazor_harsh_winds);
    new achievement_alysrazor_barrel_roll();
}
