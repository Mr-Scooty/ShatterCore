/*
 * This file is part of the ShatterCore and TrinityCore Projects. See AUTHORS file for Copyright information
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
 * The Lost Isles, Acts 3-4: the volcano, the Gallywix Labor Mine and the
 * Bilgewater Buccaneers finale. Quests 24925 through 25265 plus the escape
 * to Azshara/Orgrimmar (25266/25267).
 */

#include "ScriptMgr.h"
#include "CombatAI.h"
#include "GameObject.h"
#include "GameObjectAI.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "PhasingHandler.h"
#include "Player.h"
#include "QuestDef.h"
#include "ScriptedCreature.h"
#include "SpellAuraEffects.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "TemporarySummon.h"
#include "Vehicle.h"
#include "VehicleDefines.h"
#include <array>

using namespace std::chrono_literals;

enum LostIslesAct34Quests
{
    QUEST_ZOMBIES_VS_ROCKET_BOOTS   = 24942,
    QUEST_VOLCANOTH                 = 24958,
    QUEST_OLD_FRIENDS               = 25023,
    QUEST_REPEL_THE_PARATROOPERS    = 25024,
    QUEST_PRIDE_OF_KEZAN            = 25066,
    QUEST_LETS_RIDE                 = 25100,
    QUEST_MORALE_BOOST              = 25122,
    QUEST_WILD_MINE_CART_RIDE       = 25184,
    QUEST_GOODBYE_SWEET_OIL         = 25207,
    QUEST_THE_SLAVE_PITS            = 25213,
    QUEST_ESCAPE_VELOCITY           = 25214,
    QUEST_FINAL_CONFRONTATION       = 25251,
    QUEST_WARCHIEFS_EMISSARY        = 25266,
    QUEST_MESSAGE_FOR_GARROSH       = 25267
};

enum LostIslesAct34Creatures
{
    NPC_ROCKET_BOOTS                = 38802,
    NPC_BOOT_STOMP_CREDIT           = 38807,
    NPC_GOBLIN_ZOMBIE_1             = 38753,
    NPC_GOBLIN_ZOMBIE_2             = 38813,
    NPC_GOBLIN_ZOMBIE_3             = 38815,
    NPC_GOBLIN_ZOMBIE_4             = 38816,
    NPC_VOLCANOTH                   = 38855,
    NPC_SASSY_LOST_PEAK             = 38928,
    NPC_VOLCANOTH_CREDIT            = 38868,
    NPC_ERUPTION_BUNNY              = 38985,
    NPC_FLYING_BOMBER               = 38918,
    NPC_ALLIANCE_PARATROOPER        = 39042,
    NPC_PRIDE_OF_KEZAN              = 39074,
    NPC_STEALTH_FIGHTER             = 39039,
    NPC_BASTIA_RIDE                 = 39152,
    NPC_ACE_CAPTIVE                 = 38441,
    NPC_IZZY_CAPTIVE                = 38647,
    NPC_GOBBER_CAPTIVE              = 38746,
    NPC_GOBLIN_SURVIVOR_MINE        = 38409,
    NPC_KEZAN_CITIZEN_MINE          = 38745,
    NPC_SOULSTONE_CREDIT            = 39276,
    NPC_ASSISTANT_GREELY_MINE       = 39199,
    NPC_MINE_CART_RIDE              = 39329,
    NPC_MINE_CART_GIVER             = 39341,
    NPC_MINE_CART_CREDIT            = 39335,
    NPC_OIL_RIG_CREDIT              = 39393,
    NPC_OIL_EXPLOSION_BUNNY         = 39383,
    NPC_FOOTBOMB_UNIFORM_DISGUISE   = 47956,
    NPC_CAPTURED_GOBLIN             = 39456,
    NPC_ULTIMATE_FOOTBOMB_UNIFORM   = 39598,
    NPC_TRADE_PRINCE_GALLYWIX       = 39582,
    NPC_THRALL_FINALE               = 39594,
    NPC_BATTLEWORG                  = 39611
};

enum LostIslesAct34Spells
{
    // Rocket boots
    SPELL_ROCKET_BOOTS_SUMMON       = 72891,
    SPELL_ROCKET_BOOTS_ENGAGE       = 72897,
    SPELL_BOOT_STOMP                = 72886,

    // Volcanoth
    SPELL_VOLCANOTH_BREATH          = 73016,
    SPELL_VOLCANOTH_CONE            = 73097,
    SPELL_VOLCANOTH_AURA            = 72996,
    SPELL_ERUPTION_FX_1             = 73193,
    SPELL_ERUPTION_FX_2             = 74070,
    SPELL_ERUPTION_FX_3             = 74076,

    // Old Friends
    SPELL_BOMBER_MARKER             = 73149,

    // Paratroopers
    SPELL_PARACHUTE                 = 73363,
    SPELL_PARATROOPER_SHOOT         = 6660,

    // Morale Boost
    SPELL_ACE_FREED                 = 73602,
    SPELL_IZZY_FREED                = 73613,
    SPELL_GOBBER_FREED              = 73614,

    // Morale Boost
    SPELL_KAJA_COLA_DRINK           = 73599,
    SPELL_TOSS_EMPTY_CAN            = 70486,

    // Throw It On the Ground!
    SPELL_SOULSTONE_VISUAL          = 73703,

    // Good-bye, Sweet Oil
    SPELL_OIL_RIG_DETONATE          = 73888,

    // Escape Velocity
    SPELL_ESCAPE_VELOCITY_LAUNCH    = 73948,

    // Final Confrontation - Gallywix
    SPELL_GALLYWIX_PUNCH            = 74006,
    SPELL_GALLYWIX_CHANNEL          = 74000,
    SPELL_GALLYWIX_SPELL_3          = 74005,
    SPELL_GALLYWIX_SPELL_4          = 74004,
    SPELL_GALLYWIX_DEBUFF           = 74003,
    SPELL_GALLYWIX_PULL             = 81000,
    SPELL_UNIFORM_RIDE              = 73989,

    // Final Confrontation - Thrall
    SPELL_THRALL_ATTACK_1           = 74019,
    SPELL_THRALL_ATTACK_2           = 74020,
    SPELL_THRALL_ATTACK_3           = 74021,

    // Zone exit
    SPELL_SET_SAIL                  = 74924
};

enum LostIslesAct34Misc
{
    ZONE_LOST_ISLES_34              = 4720,
    ACTION_GALLYWIX_ENGAGE          = 1,

    GO_LAND_MINE                    = 202472
};

Position const AzsharaLandingPos = { 1468.8f, -5012.29f, 11.7693f, 3.23862f };
Position const OrgrimmarArrivalPos = { 1431.3741f, -4386.0894f, 25.570818f, 4.874169f };

// -----------------------------------------------------------------------------
// Zombies vs. Super Booster Rocket Boots (24942) / Rocket Boot Boost (24952)
// -----------------------------------------------------------------------------

// 72891 - Rocket Boots engaged: summon and board the boots.
class spell_lost_isles_rocket_boots : public SpellScriptLoader
{
public:
    spell_lost_isles_rocket_boots() : SpellScriptLoader("spell_lost_isles_rocket_boots") { }

    class spell_lost_isles_rocket_boots_SpellScript : public SpellScript
    {
    public:
        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            Player* player = GetHitPlayer();
            if (!player)
                if (Unit* caster = GetCaster())
                    player = caster->ToPlayer();
            if (!player || player->GetVehicle())
                return;

            if (Creature* boots = player->SummonCreature(NPC_ROCKET_BOOTS, player->GetPosition(), TEMPSUMMON_MANUAL_DESPAWN))
                player->EnterVehicle(boots, 0);
        }

        void Register() override
        {
            OnEffectHitTarget.Register(&spell_lost_isles_rocket_boots_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_lost_isles_rocket_boots_SpellScript();
    }
};

class npc_super_booster_rocket_boots : public CreatureScript
{
public:
    npc_super_booster_rocket_boots() : CreatureScript("npc_super_booster_rocket_boots") { }

    struct npc_super_booster_rocket_bootsAI : public VehicleAI
    {
        npc_super_booster_rocket_bootsAI(Creature* creature) : VehicleAI(creature) { }

        void PassengerBoarded(Unit* passenger, int8 seatId, bool apply) override
        {
            if (seatId != 0 || !passenger->IsPlayer())
                return;

            if (apply)
            {
                if (sSpellMgr->GetSpellInfo(SPELL_ROCKET_BOOTS_ENGAGE))
                    me->CastSpell(me, SPELL_ROCKET_BOOTS_ENGAGE, true);
            }
            else
            {
                // Stop the stomp periodic that got re-parented onto the rider.
                passenger->RemoveAurasDueToSpell(72885);
                if (me->IsSummon())
                    me->DespawnOrUnsummon(1000);
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_super_booster_rocket_bootsAI(creature);
    }
};

// 72886 - boot stomp damage: squash zombies and credit the driver.
class spell_lost_isles_boot_stomp : public SpellScriptLoader
{
public:
    spell_lost_isles_boot_stomp() : SpellScriptLoader("spell_lost_isles_boot_stomp") { }

    class spell_lost_isles_boot_stomp_SpellScript : public SpellScript
    {
    public:
        static bool IsZombie(uint32 entry)
        {
            switch (entry)
            {
                case NPC_GOBLIN_ZOMBIE_1:
                case NPC_GOBLIN_ZOMBIE_2:
                case NPC_GOBLIN_ZOMBIE_3:
                case NPC_GOBLIN_ZOMBIE_4:
                    return true;
                default:
                    return false;
            }
        }

        void HandleDamage(SpellEffIndex /*effIndex*/)
        {
            Creature* zombie = GetHitCreature();
            Unit* caster = GetCaster();
            if (!zombie || !caster || !IsZombie(zombie->GetEntry()) || !zombie->IsAlive())
                return;

            // The stomp periodic (72885) ticks on the PLAYER (re-parented by
            // the 72897 force-cast), so the caster usually is the driver.
            Player* driver = caster->ToPlayer();
            if (!driver && caster->GetCharmer())
                driver = caster->GetCharmer()->ToPlayer();
            if (!driver)
                if (Vehicle* kit = caster->GetVehicleKit())
                    if (Unit* seat0 = kit->GetPassenger(0))
                        driver = seat0->ToPlayer();

            if (driver && driver->GetQuestStatus(QUEST_ZOMBIES_VS_ROCKET_BOOTS) == QUEST_STATUS_INCOMPLETE)
                driver->KilledMonsterCredit(NPC_BOOT_STOMP_CREDIT);
        }

        void Register() override
        {
            OnEffectHitTarget.Register(&spell_lost_isles_boot_stomp_SpellScript::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_lost_isles_boot_stomp_SpellScript();
    }
};

// -----------------------------------------------------------------------------
// Volcanoth! (24958)
// -----------------------------------------------------------------------------

enum VolcanothEvents
{
    EVENT_VOLCANOTH_BREATH          = 1,
    EVENT_VOLCANOTH_CONE            = 2,

    EVENT_ERUPTION_FX_1             = 1,
    EVENT_ERUPTION_FX_2             = 2,
    EVENT_ERUPTION_FX_3             = 3
};

class boss_volcanoth : public CreatureScript
{
public:
    boss_volcanoth() : CreatureScript("boss_volcanoth") { }

    struct boss_volcanothAI : public ScriptedAI
    {
        boss_volcanothAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset() override
        {
            _events.Reset();
            // Sniff: the cosmetic flame breath (73016) is an OUT-of-combat
            // ambience on a ~52s cadence; 72996 ("First Aid") never belongs
            // to the boss - it is Doc Zapnozzle's camp ambience.
            _events.ScheduleEvent(EVENT_VOLCANOTH_BREATH, 10s, 30s);
        }

        void JustEngagedWith(Unit* /*who*/) override
        {
            _events.CancelEvent(EVENT_VOLCANOTH_BREATH);
            Talk(0); // "Stay out of the way of Volcanoth's breath!" (~2s after engage in sniff)
            _events.ScheduleEvent(EVENT_VOLCANOTH_CONE, 4s); // sniff: first breath ~4s in
        }

        void JustDied(Unit* /*killer*/) override
        {
            // The turtle god's death floods the vale: credit + phase advance
            // for everyone participating, then the eruption effects.
            std::list<Player*> players;
            me->GetPlayerListInGrid(players, 150.0f);
            for (Player* player : players)
            {
                if (player->GetQuestStatus(QUEST_VOLCANOTH) == QUEST_STATUS_INCOMPLETE)
                {
                    player->KilledMonsterCredit(NPC_VOLCANOTH_CREDIT);
                    PhasingHandler::OnConditionChange(player); // 182 -> 183
                }
            }

            me->SummonCreature(NPC_ERUPTION_BUNNY, me->GetPosition(), TEMPSUMMON_TIMED_DESPAWN, 30s);

            // Sassy calls the player over for the escape flight (sniffed with the credit).
            if (Creature* sassy = me->FindNearestCreature(NPC_SASSY_LOST_PEAK, 150.0f))
                sassy->AI()->Talk(0);
        }

        void UpdateAI(uint32 diff) override
        {
            _events.Update(diff);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_VOLCANOTH_BREATH: // OOC cosmetic flame breath (sniff ~52s)
                        if (!me->IsInCombat())
                            DoCastSelf(SPELL_VOLCANOTH_BREATH);
                        _events.ScheduleEvent(EVENT_VOLCANOTH_BREATH, 45s, 60s);
                        break;
                    case EVENT_VOLCANOTH_CONE:
                        if (sSpellMgr->GetSpellInfo(SPELL_VOLCANOTH_CONE))
                            DoCastVictim(SPELL_VOLCANOTH_CONE);
                        _events.ScheduleEvent(EVENT_VOLCANOTH_CONE, 9700ms); // sniff: fixed 9.7s cadence
                        break;
                    default:
                        break;
                }
            }

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }

    private:
        EventMap _events;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_volcanothAI(creature);
    }
};

class npc_volcanoth_eruption_bunny : public CreatureScript
{
public:
    npc_volcanoth_eruption_bunny() : CreatureScript("npc_volcanoth_eruption_bunny") { }

    struct npc_volcanoth_eruption_bunnyAI : public ScriptedAI
    {
        npc_volcanoth_eruption_bunnyAI(Creature* creature) : ScriptedAI(creature) { }

        void IsSummonedBy(Unit* /*summoner*/) override
        {
            me->SetReactState(REACT_PASSIVE);
            _events.ScheduleEvent(EVENT_ERUPTION_FX_1, 1s);
            _events.ScheduleEvent(EVENT_ERUPTION_FX_2, 5s);
            _events.ScheduleEvent(EVENT_ERUPTION_FX_3, 10s);
        }

        void UpdateAI(uint32 diff) override
        {
            _events.Update(diff);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                uint32 spellId = 0;
                switch (eventId)
                {
                    case EVENT_ERUPTION_FX_1: spellId = SPELL_ERUPTION_FX_1; break;
                    case EVENT_ERUPTION_FX_2: spellId = SPELL_ERUPTION_FX_2; break;
                    case EVENT_ERUPTION_FX_3: spellId = SPELL_ERUPTION_FX_3; break;
                    default: break;
                }
                if (spellId && sSpellMgr->GetSpellInfo(spellId))
                    me->CastSpell(me, spellId, true);
            }
        }

    private:
        EventMap _events;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_volcanoth_eruption_bunnyAI(creature);
    }
};

// -----------------------------------------------------------------------------
// Old Friends (25023) - the Flying Bomber hop to Thrall's camp.
// -----------------------------------------------------------------------------

// Flight spline from the sniff (P4, mover Low 356556): a 107-second scenic
// tour from the volcano down to Thrall's camp.
Position const FlyingBomberPath[] =
{
    { 1151.07f, 1115.43f, 129.64f },
    { 1099.58f, 1166.04f, 160.91f },
    { 985.60f, 1262.28f, 123.75f },
    { 1028.52f, 1418.13f, 106.69f },
    { 922.44f, 1584.99f, 168.41f },
    { 737.91f, 1636.48f, 142.39f },
    { 763.01f, 1888.22f, 119.61f },
    { 894.96f, 2161.13f, 93.30f },
    { 938.34f, 2458.16f, 23.83f },
    { 771.26f, 2526.76f, 11.08f },
    { 807.43f, 2367.57f, 30.66f },
    { 1235.78f, 2192.42f, 93.26f },
    { 1584.89f, 2684.93f, 95.60f }
};

enum FlyingBomberEvents
{
    EVENT_BOMBER_TAKEOFF            = 1
};

class npc_flying_bomber : public CreatureScript
{
public:
    npc_flying_bomber() : CreatureScript("npc_flying_bomber") { }

    struct npc_flying_bomberAI : public VehicleAI
    {
        npc_flying_bomberAI(Creature* creature) : VehicleAI(creature) { }

        void PassengerBoarded(Unit* passenger, int8 seatId, bool apply) override
        {
            if (!passenger->IsPlayer())
                return;

            if (apply)
            {
                _passengerGUID = passenger->GetGUID();
                if (sSpellMgr->GetSpellInfo(SPELL_BOMBER_MARKER))
                    me->CastSpell(passenger, SPELL_BOMBER_MARKER, true);
                _events.ScheduleEvent(EVENT_BOMBER_TAKEOFF, 2s);
            }
            else
            {
                _events.Reset();
                // The static bomber respawns; player summons despawn.
                if (me->IsSummon())
                    me->DespawnOrUnsummon(2000);
                else
                    me->GetMotionMaster()->MoveTargetedHome();
            }
        }

        void MovementInform(uint32 type, uint32 pointId) override
        {
            if (type != EFFECT_MOTION_TYPE && type != POINT_MOTION_TYPE)
                return;

            if (pointId != std::size(FlyingBomberPath) - 1)
                return;

            if (Player* player = ObjectAccessor::GetPlayer(*me, _passengerGUID))
                player->ExitVehicle();
        }

        void UpdateAI(uint32 diff) override
        {
            VehicleAI::UpdateAI(diff);
            _events.Update(diff);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                if (eventId == EVENT_BOMBER_TAKEOFF)
                {
                    me->SetCanFly(true);
                    me->SetDisableGravity(true);
                    me->GetMotionMaster()->MoveSmoothPath(uint32(std::size(FlyingBomberPath) - 1), FlyingBomberPath, std::size(FlyingBomberPath), false, true);
                }
            }
        }

    private:
        EventMap _events;
        ObjectGuid _passengerGUID;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_flying_bomberAI(creature);
    }
};

// -----------------------------------------------------------------------------
// Repel the Paratroopers (25024)
// -----------------------------------------------------------------------------

class npc_alliance_paratrooper : public CreatureScript
{
public:
    npc_alliance_paratrooper() : CreatureScript("npc_alliance_paratrooper") { }

    struct npc_alliance_paratrooperAI : public ScriptedAI
    {
        npc_alliance_paratrooperAI(Creature* creature) : ScriptedAI(creature) { }

        void IsSummonedBy(Unit* /*summoner*/) override
        {
            me->SetDisableGravity(true);
            if (sSpellMgr->GetSpellInfo(SPELL_PARACHUTE))
                me->CastSpell(me, SPELL_PARACHUTE, true);

            float groundZ = me->GetMap()->GetHeight(me->GetPhaseShift(), me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
            me->GetMotionMaster()->MovePoint(1, me->GetPositionX(), me->GetPositionY(), groundZ, false);
        }

        void MovementInform(uint32 type, uint32 pointId) override
        {
            if (type != POINT_MOTION_TYPE || pointId != 1)
                return;

            me->SetDisableGravity(false);
            me->RemoveAurasDueToSpell(SPELL_PARACHUTE);
            me->SetReactState(REACT_AGGRESSIVE);
            if (Player* target = me->SelectNearestPlayer(40.0f))
                AttackStart(target);
        }

        void JustEngagedWith(Unit* /*who*/) override
        {
            _shootTimer = urand(1500, 2500);
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            // Sniff: Shoot (6660) is their bread-and-butter ranged auto (~2s).
            if (_shootTimer <= diff)
            {
                _shootTimer = urand(1800, 2600);
                if (!me->IsWithinMeleeRange(me->GetVictim()))
                    DoCastVictim(SPELL_PARATROOPER_SHOOT);
            }
            else
                _shootTimer -= diff;

            DoMeleeAttackIfReady();
        }

    private:
        uint32 _shootTimer = 2000;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_alliance_paratrooperAI(creature);
    }
};

// -----------------------------------------------------------------------------
// The Pride of Kezan (25066)
// -----------------------------------------------------------------------------

enum PrideOfKezanEvents
{
    EVENT_SUMMON_FIGHTERS           = 1
};

class npc_pride_of_kezan : public CreatureScript
{
public:
    npc_pride_of_kezan() : CreatureScript("npc_pride_of_kezan") { }

    struct npc_pride_of_kezanAI : public VehicleAI
    {
        npc_pride_of_kezanAI(Creature* creature) : VehicleAI(creature) { }

        void PassengerBoarded(Unit* passenger, int8 seatId, bool apply) override
        {
            if (seatId != 0 || !passenger->IsPlayer())
                return;

            if (apply)
            {
                me->SetCanFly(true);
                me->SetDisableGravity(true);
                _events.ScheduleEvent(EVENT_SUMMON_FIGHTERS, 4s);
            }
            else
            {
                _events.Reset();
                DespawnFighters();
                if (me->IsSummon())
                    me->DespawnOrUnsummon(2000);
            }
        }

        void JustSummoned(Creature* summon) override
        {
            if (summon->GetEntry() != NPC_STEALTH_FIGHTER)
                return;

            _fighterGUIDs.push_back(summon->GetGUID());
            summon->SetCanFly(true);
            summon->SetDisableGravity(true);
            summon->SetReactState(REACT_PASSIVE);
            summon->GetMotionMaster()->MoveRandom(40.0f);
        }

        void UpdateAI(uint32 diff) override
        {
            VehicleAI::UpdateAI(diff);
            _events.Update(diff);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                if (eventId == EVENT_SUMMON_FIGHTERS)
                {
                    // Fighter wings circle at the sniffed patrol anchors.
                    static Position const FighterAnchors[] =
                    {
                        { 1855.72f, 3015.21f, 90.22f },
                        { 1944.16f, 2952.64f, 57.82f },
                        { 2005.04f, 2940.56f, 96.14f },
                        { 2024.57f, 2933.08f, 104.03f },
                        { 1701.00f, 3122.55f, 58.17f },
                        { 1981.27f, 2978.07f, 69.21f },
                        { 2020.07f, 3014.28f, 71.49f }
                    };
                    for (Position const& anchor : FighterAnchors)
                    {
                        me->SummonCreature(NPC_STEALTH_FIGHTER, anchor, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5min);
                        Position offset = anchor;
                        offset.m_positionX += frand(-25.0f, 25.0f);
                        offset.m_positionY += frand(-25.0f, 25.0f);
                        me->SummonCreature(NPC_STEALTH_FIGHTER, offset, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5min);
                    }
                }
            }
        }

    private:
        void DespawnFighters()
        {
            for (ObjectGuid const& guid : _fighterGUIDs)
                if (Creature* fighter = ObjectAccessor::GetCreature(*me, guid))
                    fighter->DespawnOrUnsummon();
            _fighterGUIDs.clear();
        }

        EventMap _events;
        std::vector<ObjectGuid> _fighterGUIDs;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_pride_of_kezanAI(creature);
    }
};

// 74958 / 73477 - gunship guns: credit downed stealth fighters.
class spell_lost_isles_gunship_gun : public SpellScriptLoader
{
public:
    spell_lost_isles_gunship_gun() : SpellScriptLoader("spell_lost_isles_gunship_gun") { }

    class spell_lost_isles_gunship_gun_SpellScript : public SpellScript
    {
    public:
        void HandleDamage(SpellEffIndex /*effIndex*/)
        {
            Creature* fighter = GetHitCreature();
            Unit* caster = GetCaster();
            if (!fighter || !caster || fighter->GetEntry() != NPC_STEALTH_FIGHTER)
                return;

            Player* driver = caster->GetCharmer() ? caster->GetCharmer()->ToPlayer() : nullptr;
            if (!driver)
                if (Vehicle* kit = caster->GetVehicleKit())
                    if (Unit* seat0 = kit->GetPassenger(0))
                        driver = seat0->ToPlayer();

            if (driver && driver->GetQuestStatus(QUEST_PRIDE_OF_KEZAN) == QUEST_STATUS_INCOMPLETE)
                driver->KilledMonsterCredit(NPC_STEALTH_FIGHTER);

            fighter->KillSelf();
            fighter->DespawnOrUnsummon(4000);
        }

        void Register() override
        {
            OnEffectHitTarget.Register(&spell_lost_isles_gunship_gun_SpellScript::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_lost_isles_gunship_gun_SpellScript();
    }
};

// -----------------------------------------------------------------------------
// Mine Disposal, the Goblin Way (25058)
// -----------------------------------------------------------------------------

// 73425 - grenade toss: defuse the nearest land mine.
class spell_lost_isles_grenade : public SpellScriptLoader
{
public:
    spell_lost_isles_grenade() : SpellScriptLoader("spell_lost_isles_grenade") { }

    class spell_lost_isles_grenade_SpellScript : public SpellScript
    {
    public:
        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            Unit* caster = GetCaster();
            Player* player = caster ? caster->ToPlayer() : nullptr;
            if (!player)
                return;

            GameObject* mine = nullptr;
            if (WorldLocation const* dest = GetExplTargetDest())
            {
                mine = player->FindNearestGameObject(GO_LAND_MINE, 40.0f);
                if (mine && mine->GetExactDist2d(dest->GetPositionX(), dest->GetPositionY()) > 12.0f)
                    mine = nullptr;
            }
            else
                mine = player->FindNearestGameObject(GO_LAND_MINE, 12.0f);

            if (!mine)
                return;

            player->KillCreditGO(GO_LAND_MINE, mine->GetGUID());
            mine->SetLootState(GO_JUST_DEACTIVATED);
            mine->SetRespawnTime(60);
        }

        void Register() override
        {
            // OnEffectHit (not HitTarget): the dummy effect targets enemies at
            // the destination and the minefield is usually empty of units.
            OnEffectHit.Register(&spell_lost_isles_grenade_SpellScript::HandleDummy, EFFECT_1, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_lost_isles_grenade_SpellScript();
    }
};

// 72518 - Oomlot Shaman captive drain: the DBC targets TARGET_UNIT_SUMMONER,
// but the shamans are static spawns; redirect to the SAI-provided target.
class spell_lost_isles_captive_drain : public SpellScriptLoader
{
public:
    spell_lost_isles_captive_drain() : SpellScriptLoader("spell_lost_isles_captive_drain") { }

    class spell_lost_isles_captive_drain_SpellScript : public SpellScript
    {
    public:
        void SetTarget(WorldObject*& target)
        {
            if (WorldObject* explTarget = GetExplTargetWorldObject())
                target = explTarget;
        }

        void Register() override
        {
            OnObjectTargetSelect.Register(&spell_lost_isles_captive_drain_SpellScript::SetTarget, EFFECT_0, TARGET_UNIT_SUMMONER);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_lost_isles_captive_drain_SpellScript();
    }
};

// -----------------------------------------------------------------------------
// Morale Boost (25122)
// -----------------------------------------------------------------------------

// 73583 - Kaja'Cola Zero-One: hand a can to a demoralized prisoner.
class spell_lost_isles_kaja_cola : public SpellScriptLoader
{
public:
    spell_lost_isles_kaja_cola() : SpellScriptLoader("spell_lost_isles_kaja_cola") { }

    class spell_lost_isles_kaja_cola_SpellScript : public SpellScript
    {
    public:
        static bool IsValidTarget(uint32 entry)
        {
            switch (entry)
            {
                case NPC_ACE_CAPTIVE:
                case NPC_IZZY_CAPTIVE:
                case NPC_GOBBER_CAPTIVE:
                case NPC_GOBLIN_SURVIVOR_MINE:
                case NPC_KEZAN_CITIZEN_MINE:
                    return true;
                default:
                    return false;
            }
        }

        SpellCastResult CheckTarget()
        {
            Unit* target = GetExplTargetUnit();
            if (!target || !IsValidTarget(target->GetEntry()) || !target->IsAlive())
                return SPELL_FAILED_BAD_TARGETS;
            return SPELL_CAST_OK;
        }

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
            Creature* target = GetHitCreature();
            if (!player || !target)
                return;

            uint32 freedSpell = 0;
            switch (target->GetEntry())
            {
                case NPC_ACE_CAPTIVE:    freedSpell = SPELL_ACE_FREED;    break;
                case NPC_IZZY_CAPTIVE:   freedSpell = SPELL_IZZY_FREED;   break;
                case NPC_GOBBER_CAPTIVE: freedSpell = SPELL_GOBBER_FREED; break;
                default:                                                  break;
            }

            // Both prisoner entries count toward the 38409 survivor objective
            // (sniff: 3 of the 6 rescues hit Kezan Citizens crediting 38409).
            player->KilledMonsterCredit(freedSpell ? target->GetEntry() : NPC_GOBLIN_SURVIVOR_MINE);

            if (freedSpell && sSpellMgr->GetSpellInfo(freedSpell))
            {
                player->CastSpell(player, freedSpell, true);
                // The freed version takes over - remove the captive so Ace/
                // Izzy/Gobber don't stand next to their own doubles.
                target->DespawnOrUnsummon(1500ms, 120s);
            }
            else
            {
                // Sniff: the prisoner chugs the cola, tosses the empty can
                // and yells an "idea" line before wandering off.
                target->CastSpell(target, SPELL_KAJA_COLA_DRINK, true);
                target->CastSpell(target, SPELL_TOSS_EMPTY_CAN, true);
                if (target->IsAIEnabled())
                    target->AI()->Talk(0, player);
                target->HandleEmoteCommand(EMOTE_ONESHOT_CHEER);
                target->DespawnOrUnsummon(5000, 60s);
            }
        }

        void Register() override
        {
            OnCheckCast.Register(&spell_lost_isles_kaja_cola_SpellScript::CheckTarget);
            OnEffectHitTarget.Register(&spell_lost_isles_kaja_cola_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_lost_isles_kaja_cola_SpellScript();
    }
};

// -----------------------------------------------------------------------------
// Throw It On the Ground! (25123)
// -----------------------------------------------------------------------------

// 73702 - Blastshadow's Soulstone: smash it.
class spell_lost_isles_soulstone : public SpellScriptLoader
{
public:
    spell_lost_isles_soulstone() : SpellScriptLoader("spell_lost_isles_soulstone") { }

    class spell_lost_isles_soulstone_SpellScript : public SpellScript
    {
    public:
        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
            if (!player)
                return;

            player->KilledMonsterCredit(NPC_SOULSTONE_CREDIT);
            // 73703 is NOT a visual - it summons another lootable soulstone
            // chest; retail only Blastshadow casts it (on death). Greely
            // celebrates the smash instead (sniffed with the credit).
            if (Creature* greely = player->FindNearestCreature(NPC_ASSISTANT_GREELY_MINE, 100.0f))
                greely->AI()->Talk(2, player);
        }

        void Register() override
        {
            OnEffectHitTarget.Register(&spell_lost_isles_soulstone_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_lost_isles_soulstone_SpellScript();
    }
};

// -----------------------------------------------------------------------------
// Wild Mine Cart Ride (25184)
// -----------------------------------------------------------------------------

// Tunnel spline from the sniff (P4, mover Low 358412): from the parked cart
// down the rails to Greely at the tunnel exit.
Position const MineCartPath[] =
{
    { 2070.93f, 1845.76f, 138.33f },
    { 2079.47f, 1848.77f, 138.21f },
    { 2089.52f, 1851.94f, 134.83f },
    { 2109.83f, 1857.30f, 128.36f },
    { 2131.06f, 1863.16f, 127.58f },
    { 2150.16f, 1869.64f, 127.89f },
    { 2166.74f, 1875.38f, 126.67f },
    { 2177.63f, 1878.95f, 107.83f },
    { 2192.89f, 1883.49f, 91.45f },
    { 2207.23f, 1887.69f, 77.04f },
    { 2223.78f, 1892.50f, 64.67f },
    { 2243.41f, 1898.26f, 57.02f },
    { 2262.62f, 1903.34f, 47.47f },
    { 2281.97f, 1908.10f, 37.30f },
    { 2302.46f, 1914.88f, 33.34f },
    { 2323.30f, 1921.85f, 29.84f },
    { 2343.20f, 1929.02f, 26.27f },
    { 2364.05f, 1932.62f, 23.59f },
    { 2368.44f, 1934.03f, 21.19f }
};

enum MineCartEvents
{
    EVENT_CART_DEPART               = 1
};

class npc_lost_isles_mine_cart : public CreatureScript
{
public:
    npc_lost_isles_mine_cart() : CreatureScript("npc_lost_isles_mine_cart") { }

    struct npc_lost_isles_mine_cartAI : public VehicleAI
    {
        npc_lost_isles_mine_cartAI(Creature* creature) : VehicleAI(creature) { }

        void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
        {
            if (!passenger->IsPlayer())
                return;

            if (apply)
            {
                _passengerGUID = passenger->GetGUID();
                _events.ScheduleEvent(EVENT_CART_DEPART, 2s);
            }
            else
            {
                _events.Reset();
                if (me->IsSummon())
                    me->DespawnOrUnsummon(2000);
            }
        }

        void MovementInform(uint32 type, uint32 pointId) override
        {
            if (type != EFFECT_MOTION_TYPE && type != POINT_MOTION_TYPE)
                return;

            if (pointId != std::size(MineCartPath) - 1)
                return;

            if (Player* player = ObjectAccessor::GetPlayer(*me, _passengerGUID))
            {
                if (player->GetQuestStatus(QUEST_WILD_MINE_CART_RIDE) == QUEST_STATUS_INCOMPLETE)
                {
                    player->KilledMonsterCredit(NPC_MINE_CART_CREDIT);
                    PhasingHandler::OnConditionChange(player); // 183 -> 184
                }
                player->ExitVehicle();
            }
            me->DespawnOrUnsummon(2000);
        }

        void UpdateAI(uint32 diff) override
        {
            VehicleAI::UpdateAI(diff);
            _events.Update(diff);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                if (eventId == EVENT_CART_DEPART)
                    me->GetMotionMaster()->MoveSmoothPath(uint32(std::size(MineCartPath) - 1), MineCartPath, std::size(MineCartPath), false, false);
            }
        }

    private:
        EventMap _events;
        ObjectGuid _passengerGUID;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_lost_isles_mine_cartAI(creature);
    }
};

// -----------------------------------------------------------------------------
// Good-bye, Sweet Oil (25207)
// -----------------------------------------------------------------------------

class go_platform_control_panel : public GameObjectScript
{
public:
    go_platform_control_panel() : GameObjectScript("go_platform_control_panel") { }

    struct go_platform_control_panelAI : public GameObjectAI
    {
        go_platform_control_panelAI(GameObject* go) : GameObjectAI(go) { }

        bool GossipHello(Player* player) override
        {
            // Only intercept the detonation use; quest give/turn-in at this
            // panel must fall through to the default gossip handling.
            if (player->GetQuestStatus(QUEST_GOODBYE_SWEET_OIL) != QUEST_STATUS_INCOMPLETE)
                return false;

            if (player->GetReqKillOrCastCurrentCount(QUEST_GOODBYE_SWEET_OIL, NPC_OIL_RIG_CREDIT) > 0)
                return false;

            if (sSpellMgr->GetSpellInfo(SPELL_OIL_RIG_DETONATE))
                player->CastSpell(player, SPELL_OIL_RIG_DETONATE, true); // native credit 39393 + bunny
            else
            {
                player->KilledMonsterCredit(NPC_OIL_RIG_CREDIT);
                player->SummonCreature(NPC_OIL_EXPLOSION_BUNNY, me->GetPosition(), TEMPSUMMON_TIMED_DESPAWN, 30s);
            }

            return true;
        }
    };

    GameObjectAI* GetAI(GameObject* go) const override
    {
        return new go_platform_control_panelAI(go);
    }
};

// -----------------------------------------------------------------------------
// Escape Velocity (25214)
// -----------------------------------------------------------------------------

// 73947 - launch a Captured Goblin skyward.
class spell_lost_isles_escape_velocity : public SpellScriptLoader
{
public:
    spell_lost_isles_escape_velocity() : SpellScriptLoader("spell_lost_isles_escape_velocity") { }

    class spell_lost_isles_escape_velocity_SpellScript : public SpellScript
    {
    public:
        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            Unit* caster = GetCaster();
            Player* player = caster ? caster->ToPlayer() : nullptr;
            if (!player && caster)
                if (Unit* owner = caster->GetCharmerOrOwner())
                    player = owner->ToPlayer();

            Creature* captive = GetHitCreature();
            if (!captive && caster && caster->GetEntry() == NPC_CAPTURED_GOBLIN)
                captive = caster->ToCreature();

            if (!player || !captive || captive->GetEntry() != NPC_CAPTURED_GOBLIN)
                return;

            player->KilledMonsterCredit(NPC_CAPTURED_GOBLIN);

            // 73948 only carries the rocket-flame aura - the ascent is
            // server-side movement (sniff: ~4.5s of rumbling, then the cage
            // shoots ~200yd straight up and vanishes at the apex).
            captive->CastSpell(captive, SPELL_ESCAPE_VELOCITY_LAUNCH, true);
            if (captive->IsAIEnabled())
                captive->AI()->Talk(0, player);
            captive->m_Events.AddEventAtOffset([captive]()
            {
                captive->SetDisableGravity(true);
                captive->GetMotionMaster()->MoveCharge(captive->GetPositionX(), captive->GetPositionY(), captive->GetPositionZ() + 200.0f, 40.0f, 2);
            }, 4500ms);
            captive->DespawnOrUnsummon(10s, 30s);
        }

        void Register() override
        {
            OnEffectHitTarget.Register(&spell_lost_isles_escape_velocity_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_lost_isles_escape_velocity_SpellScript();
    }
};

// -----------------------------------------------------------------------------
// Final Confrontation (25251)
// -----------------------------------------------------------------------------

enum GallywixData
{
    EVENT_GALLYWIX_PUNCH            = 1,
    EVENT_GALLYWIX_CHANNEL          = 2,
    EVENT_GALLYWIX_SPELL_3          = 3,
    EVENT_GALLYWIX_SPELL_4          = 4,
    EVENT_GALLYWIX_DEBUFF           = 5,
    EVENT_GALLYWIX_PULL             = 6,
    EVENT_GALLYWIX_OUTRO            = 7,
    EVENT_GALLYWIX_RESET            = 8,

    // creature_text groups 0..12, imported from the sniff in encounter order:
    // 0 raise, 1 sucker, 2 traitor, 3 burn-you, 4 so-money, 5 dispose,
    // 6 fired-up, 7 eat-it, 8 unload, 9 uncle, 10 beaten, 11 your-goblin,
    // 12 for-the-horde.
    SAY_GALLYWIX_SURRENDER          = 9,
    SAY_GALLYWIX_BEATEN             = 10,
    SAY_GALLYWIX_YOUR_GOBLIN        = 11,
    SAY_GALLYWIX_FOR_THE_HORDE      = 12,

    // Thrall (39594) text groups: 0 grunt-emote, 1 remain-trade-prince,
    // 2 send-representative, 3 new-home-azshara.
    SAY_THRALL_GRUNT                = 0,
    SAY_THRALL_REMAIN               = 1,
    SAY_THRALL_REPRESENTATIVE       = 2,
    SAY_THRALL_AZSHARA              = 3,

    ACTION_THRALL_OUTRO             = 2
};

struct GallywixYell
{
    uint8 HealthPct;
    uint8 TextGroup;
};

// Health-triggered fight yells, from the sniffed encounter timeline.
GallywixYell const GallywixYells[] =
{
    { 90, 0 }, // I like you. Here's a raise!
    { 82, 1 }, // I need to move these toxic assets onto a sucker... like you!
    { 78, 2 }, // I SEE THE TRAITOR IS HERE TO RESCUE YOU, WARCHIEF...
    { 72, 3 }, // You burned down my headquarters...
    { 62, 4 }, // I'm so money!
    { 52, 5 }, // Excuse me while I dispose of these toxic assets all over you!
    { 45, 6 }, // I'm all fired up over finally gettin' rid of you!
    { 30, 5 }, // (dispose, repeat)
    { 20, 3 }, // (burn-you, repeat)
    { 10, 7 }, // Eat it!
    { 6,  8 }  // Here, I need to unload some toxic assets!
};

class boss_trade_prince_gallywix : public CreatureScript
{
public:
    boss_trade_prince_gallywix() : CreatureScript("boss_trade_prince_gallywix") { }

    struct boss_trade_prince_gallywixAI : public ScriptedAI
    {
        boss_trade_prince_gallywixAI(Creature* creature) : ScriptedAI(creature), _surrendered(false), _yellIndex(0), _outroStep(0) { }

        void Reset() override
        {
            _events.Reset();
            _surrendered = false;
            _yellIndex = 0;
            _outroStep = 0;
            me->SetReactState(REACT_AGGRESSIVE);
            me->SetFullHealth();
        }

        void JustEngagedWith(Unit* /*who*/) override
        {
            _events.ScheduleEvent(EVENT_GALLYWIX_PUNCH, 5s, 8s);
            _events.ScheduleEvent(EVENT_GALLYWIX_CHANNEL, 40s, 50s);
            _events.ScheduleEvent(EVENT_GALLYWIX_SPELL_3, 18s, 25s);
            _events.ScheduleEvent(EVENT_GALLYWIX_SPELL_4, 22s, 35s);
            _events.ScheduleEvent(EVENT_GALLYWIX_DEBUFF, 25s, 40s);
            _events.ScheduleEvent(EVENT_GALLYWIX_PULL, 15s, 20s);
        }

        void DamageTaken(Unit* /*attacker*/, uint32& damage) override
        {
            if (_surrendered)
            {
                damage = 0;
                return;
            }

            // Health-scheduled fight yells (from the sniffed timeline).
            while (_yellIndex < std::size(GallywixYells) &&
                   me->HealthBelowPctDamaged(GallywixYells[_yellIndex].HealthPct, damage))
            {
                Talk(GallywixYells[_yellIndex].TextGroup);
                ++_yellIndex;
            }

            // Gallywix surrenders at ~5% rather than dying.
            if (damage >= me->GetHealth() || me->HealthBelowPctDamaged(5, damage))
            {
                if (damage >= me->GetHealth())
                    damage = me->GetHealth() - 1;

                Surrender();
            }
        }

        void UpdateAI(uint32 diff) override
        {
            _events.Update(diff);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_GALLYWIX_PUNCH:
                        if (!_surrendered && sSpellMgr->GetSpellInfo(SPELL_GALLYWIX_PUNCH))
                            DoCastVictim(SPELL_GALLYWIX_PUNCH);
                        _events.ScheduleEvent(EVENT_GALLYWIX_PUNCH, 15s, 27s);
                        break;
                    case EVENT_GALLYWIX_CHANNEL:
                        if (!_surrendered && sSpellMgr->GetSpellInfo(SPELL_GALLYWIX_CHANNEL))
                            DoCastVictim(SPELL_GALLYWIX_CHANNEL);
                        _events.ScheduleEvent(EVENT_GALLYWIX_CHANNEL, 31s, 53s);
                        break;
                    case EVENT_GALLYWIX_SPELL_3:
                        if (!_surrendered && sSpellMgr->GetSpellInfo(SPELL_GALLYWIX_SPELL_3))
                            DoCastVictim(SPELL_GALLYWIX_SPELL_3);
                        _events.ScheduleEvent(EVENT_GALLYWIX_SPELL_3, 15s, 27s);
                        break;
                    case EVENT_GALLYWIX_SPELL_4:
                        if (!_surrendered && sSpellMgr->GetSpellInfo(SPELL_GALLYWIX_SPELL_4))
                            DoCastVictim(SPELL_GALLYWIX_SPELL_4);
                        _events.ScheduleEvent(EVENT_GALLYWIX_SPELL_4, 30s, 40s);
                        break;
                    case EVENT_GALLYWIX_DEBUFF:
                        if (!_surrendered && sSpellMgr->GetSpellInfo(SPELL_GALLYWIX_DEBUFF))
                            DoCastVictim(SPELL_GALLYWIX_DEBUFF);
                        _events.ScheduleEvent(EVENT_GALLYWIX_DEBUFF, 37s, 60s);
                        break;
                    case EVENT_GALLYWIX_PULL:
                        if (!_surrendered && sSpellMgr->GetSpellInfo(SPELL_GALLYWIX_PULL))
                            DoCastVictim(SPELL_GALLYWIX_PULL);
                        _events.ScheduleEvent(EVENT_GALLYWIX_PULL, 15s, 33s);
                        break;
                    case EVENT_GALLYWIX_OUTRO:
                        HandleOutroStep();
                        break;
                    case EVENT_GALLYWIX_RESET:
                        EnterEvadeMode(EVADE_REASON_OTHER);
                        break;
                    default:
                        break;
                }
            }

            if (!_surrendered && UpdateVictim())
                DoMeleeAttackIfReady();
        }

    private:
        void Surrender()
        {
            _surrendered = true;
            _events.Reset();
            me->SetReactState(REACT_PASSIVE);
            me->AttackStop();
            me->CombatStop(true);
            me->InterruptNonMeleeSpells(true);
            me->SetFullHealth();

            // Credit every nearby driver on the quest.
            std::list<Player*> players;
            me->GetPlayerListInGrid(players, 120.0f);
            for (Player* player : players)
                if (player->GetQuestStatus(QUEST_FINAL_CONFRONTATION) == QUEST_STATUS_INCOMPLETE)
                    player->KilledMonsterCredit(NPC_TRADE_PRINCE_GALLYWIX);

            _outroStep = 0;
            _events.ScheduleEvent(EVENT_GALLYWIX_OUTRO, 5s);
        }

        void HandleOutroStep()
        {
            Creature* thrall = me->FindNearestCreature(NPC_THRALL_FINALE, 100.0f, true);

            switch (_outroStep)
            {
                case 0: Talk(SAY_GALLYWIX_SURRENDER);      _events.ScheduleEvent(EVENT_GALLYWIX_OUTRO, 6500ms); break;
                case 1: Talk(SAY_GALLYWIX_BEATEN);         _events.ScheduleEvent(EVENT_GALLYWIX_OUTRO, 9700ms); break;
                case 2: Talk(SAY_GALLYWIX_YOUR_GOBLIN);    _events.ScheduleEvent(EVENT_GALLYWIX_OUTRO, 3200ms); break;
                case 3:
                    if (thrall && thrall->AI())
                        thrall->AI()->Talk(SAY_THRALL_GRUNT, me);
                    _events.ScheduleEvent(EVENT_GALLYWIX_OUTRO, 3600ms);
                    break;
                case 4:
                    if (thrall && thrall->AI())
                        thrall->AI()->Talk(SAY_THRALL_REMAIN, me);
                    _events.ScheduleEvent(EVENT_GALLYWIX_OUTRO, 6500ms);
                    break;
                case 5:
                    if (thrall && thrall->AI())
                        thrall->AI()->Talk(SAY_THRALL_REPRESENTATIVE, me);
                    _events.ScheduleEvent(EVENT_GALLYWIX_OUTRO, 9700ms);
                    break;
                case 6:
                    if (thrall && thrall->AI())
                        thrall->AI()->Talk(SAY_THRALL_AZSHARA, me);
                    _events.ScheduleEvent(EVENT_GALLYWIX_OUTRO, 9700ms);
                    break;
                case 7:
                    Talk(SAY_GALLYWIX_FOR_THE_HORDE);
                    _events.ScheduleEvent(EVENT_GALLYWIX_RESET, 60s);
                    break;
                default:
                    break;
            }
            ++_outroStep;
        }

        EventMap _events;
        bool _surrendered;
        uint8 _yellIndex;
        uint8 _outroStep;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_trade_prince_gallywixAI(creature);
    }
};

// 39594 - Thrall at the finale: batters Gallywix alongside the player.
class npc_thrall_gallywix_fight : public CreatureScript
{
public:
    npc_thrall_gallywix_fight() : CreatureScript("npc_thrall_gallywix_fight") { }

    struct npc_thrall_gallywix_fightAI : public ScriptedAI
    {
        npc_thrall_gallywix_fightAI(Creature* creature) : ScriptedAI(creature), _attackTimer(6000) { }

        void UpdateAI(uint32 diff) override
        {
            if (_attackTimer <= diff)
            {
                _attackTimer = urand(6000, 10000);
                if (Creature* gallywix = me->FindNearestCreature(NPC_TRADE_PRINCE_GALLYWIX, 60.0f, true))
                {
                    if (gallywix->IsInCombat())
                    {
                        uint32 spells[] = { SPELL_THRALL_ATTACK_1, SPELL_THRALL_ATTACK_2, SPELL_THRALL_ATTACK_3 };
                        uint32 spellId = spells[urand(0, 2)];
                        if (sSpellMgr->GetSpellInfo(spellId))
                            me->CastSpell(gallywix, spellId, false);
                    }
                }
            }
            else
                _attackTimer -= diff;
        }

    private:
        uint32 _attackTimer;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_thrall_gallywix_fightAI(creature);
    }
};

class npc_ultimate_footbomb_uniform : public CreatureScript
{
public:
    npc_ultimate_footbomb_uniform() : CreatureScript("npc_ultimate_footbomb_uniform") { }

    struct npc_ultimate_footbomb_uniformAI : public VehicleAI
    {
        npc_ultimate_footbomb_uniformAI(Creature* creature) : VehicleAI(creature) { }

        void PassengerBoarded(Unit* passenger, int8 seatId, bool apply) override
        {
            if (seatId != 0 || !passenger->IsPlayer())
                return;

            if (!apply && me->IsSummon())
                me->DespawnOrUnsummon(2000);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ultimate_footbomb_uniformAI(creature);
    }
};

// -----------------------------------------------------------------------------
// Shared player script: cleanup, exit teleports, login guards.
// -----------------------------------------------------------------------------

// Run spline from the sniff (P6, worg Low 359859): the worg runs the whole
// way from the Durotar coast, loops past Orgrimmar's gate and ends east of it,
// where the player is ported into Grommash Hold.
Position const BattleworgPath[] =
{
    { 1460.60f, -5013.10f, 11.65f },
    { 1435.06f, -4967.07f, 11.80f },
    { 1427.18f, -4889.81f, 11.18f },
    { 1411.41f, -4832.22f, 17.69f },
    { 1384.61f, -4767.72f, 26.08f },
    { 1373.59f, -4716.95f, 28.36f },
    { 1356.10f, -4648.85f, 25.93f },
    { 1323.39f, -4599.55f, 23.91f },
    { 1307.44f, -4533.37f, 22.29f },
    { 1318.64f, -4466.78f, 24.49f },
    { 1313.35f, -4395.26f, 25.58f },
    { 1366.66f, -4374.42f, 26.07f },
    { 1422.61f, -4365.81f, 25.57f },
    { 1431.14f, -4383.75f, 25.57f },
    { 1457.02f, -4420.73f, 25.57f },
    { 1514.19f, -4412.82f, 22.02f },
    { 1573.52f, -4395.46f, 15.97f },
    { 1604.30f, -4377.46f, 20.97f }
};

enum BattleworgEvents
{
    EVENT_WORG_RUN                  = 1
};

class npc_lost_isles_battleworg : public CreatureScript
{
public:
    npc_lost_isles_battleworg() : CreatureScript("npc_lost_isles_battleworg") { }

    struct npc_lost_isles_battleworgAI : public VehicleAI
    {
        npc_lost_isles_battleworgAI(Creature* creature) : VehicleAI(creature) { }

        void PassengerBoarded(Unit* passenger, int8 seatId, bool apply) override
        {
            if (seatId != 0 || !passenger->IsPlayer())
                return;

            if (apply)
            {
                _passengerGUID = passenger->GetGUID();
                _events.ScheduleEvent(EVENT_WORG_RUN, 2s);
            }
            else
            {
                _events.Reset();
                if (me->IsSummon())
                    me->DespawnOrUnsummon(2000);
            }
        }

        void MovementInform(uint32 type, uint32 pointId) override
        {
            if (type != EFFECT_MOTION_TYPE && type != POINT_MOTION_TYPE)
                return;

            if (pointId != std::size(BattleworgPath) - 1)
                return;

            if (Player* player = ObjectAccessor::GetPlayer(*me, _passengerGUID))
            {
                player->ExitVehicle();
                player->TeleportTo(1, OrgrimmarArrivalPos.GetPositionX(), OrgrimmarArrivalPos.GetPositionY(), OrgrimmarArrivalPos.GetPositionZ(), OrgrimmarArrivalPos.GetOrientation());
            }
            me->DespawnOrUnsummon(2000);
        }

        void UpdateAI(uint32 diff) override
        {
            VehicleAI::UpdateAI(diff);
            _events.Update(diff);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                if (eventId == EVENT_WORG_RUN)
                    me->GetMotionMaster()->MoveSmoothPath(uint32(std::size(BattleworgPath) - 1), BattleworgPath, std::size(BattleworgPath), false, false);
            }
        }

    private:
        EventMap _events;
        ObjectGuid _passengerGUID;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_lost_isles_battleworgAI(creature);
    }
};

class player_script_lost_isles_act34 : public PlayerScript
{
public:
    player_script_lost_isles_act34() : PlayerScript("player_script_lost_isles_act34") { }

    void OnLogin(Player* player, bool /*firstLogin*/) override
    {
        // Logged out mid-departure: finish the trip to Azshara.
        if (player->GetZoneId() == ZONE_LOST_ISLES_34 && player->GetQuestStatus(QUEST_WARCHIEFS_EMISSARY) == QUEST_STATUS_REWARDED)
            player->TeleportTo(1, AzsharaLandingPos.GetPositionX(), AzsharaLandingPos.GetPositionY(), AzsharaLandingPos.GetPositionZ(), AzsharaLandingPos.GetOrientation());
    }

    void OnQuestStatusChange(Player* player, uint32 questId) override
    {
        QuestStatus status = player->GetQuestStatus(questId);

        switch (questId)
        {
            case QUEST_ZOMBIES_VS_ROCKET_BOOTS:
                if (status == QUEST_STATUS_NONE || status == QUEST_STATUS_FAILED || status == QUEST_STATUS_REWARDED)
                    CleanupOwnedCreatures(player, { NPC_ROCKET_BOOTS });
                break;
            case QUEST_OLD_FRIENDS:
                if (status == QUEST_STATUS_NONE || status == QUEST_STATUS_FAILED || status == QUEST_STATUS_REWARDED)
                    CleanupOwnedCreatures(player, { NPC_FLYING_BOMBER });
                break;
            case QUEST_PRIDE_OF_KEZAN:
                if (status == QUEST_STATUS_NONE || status == QUEST_STATUS_FAILED || status == QUEST_STATUS_REWARDED)
                    CleanupOwnedCreatures(player, { NPC_PRIDE_OF_KEZAN });
                break;
            case QUEST_LETS_RIDE:
                if (status == QUEST_STATUS_NONE || status == QUEST_STATUS_FAILED || status == QUEST_STATUS_REWARDED)
                    CleanupOwnedCreatures(player, { NPC_BASTIA_RIDE });
                break;
            case QUEST_WILD_MINE_CART_RIDE:
                if (status == QUEST_STATUS_NONE || status == QUEST_STATUS_FAILED || status == QUEST_STATUS_REWARDED)
                    CleanupOwnedCreatures(player, { NPC_MINE_CART_RIDE });
                break;
            case QUEST_THE_SLAVE_PITS:
                if (status == QUEST_STATUS_NONE || status == QUEST_STATUS_FAILED || status == QUEST_STATUS_REWARDED)
                    CleanupOwnedCreatures(player, { NPC_FOOTBOMB_UNIFORM_DISGUISE });
                break;
            case QUEST_FINAL_CONFRONTATION:
                if (status == QUEST_STATUS_NONE || status == QUEST_STATUS_FAILED || status == QUEST_STATUS_REWARDED)
                {
                    player->RemoveAurasDueToSpell(SPELL_UNIFORM_RIDE);
                    CleanupOwnedCreatures(player, { NPC_ULTIMATE_FOOTBOMB_UNIFORM });
                }
                break;
            case QUEST_MESSAGE_FOR_GARROSH:
                // Boarding is native (SourceSpellID 74031 summons + rides the worg).
                if (status == QUEST_STATUS_NONE || status == QUEST_STATUS_FAILED || status == QUEST_STATUS_REWARDED)
                    CleanupOwnedCreatures(player, { NPC_BATTLEWORG });
                break;
            default:
                break;
        }
    }

private:
    static void CleanupOwnedCreatures(Player* player, std::initializer_list<uint32> entries)
    {
        std::list<Creature*> creatures;
        for (uint32 entry : entries)
            player->GetCreatureListWithEntryInGrid(creatures, entry, 250.0f);

        for (Creature* creature : creatures)
        {
            bool ownedByPlayer = creature->GetOwnerGUID() == player->GetGUID();
            if (TempSummon* summon = creature->ToTempSummon())
                if (summon->GetSummonerGUID() == player->GetGUID())
                    ownedByPlayer = true;

            if (!ownedByPlayer)
                continue;

            if (player->GetVehicleBase() == creature)
                player->ExitVehicle();

            creature->DespawnOrUnsummon(500);
        }
    }
};

void AddSC_lost_isles_act34()
{
    new spell_lost_isles_rocket_boots();
    new npc_super_booster_rocket_boots();
    new spell_lost_isles_boot_stomp();
    new boss_volcanoth();
    new npc_volcanoth_eruption_bunny();
    new npc_flying_bomber();
    new npc_alliance_paratrooper();
    new npc_pride_of_kezan();
    new spell_lost_isles_gunship_gun();
    new spell_lost_isles_grenade();
    new spell_lost_isles_captive_drain();
    new spell_lost_isles_kaja_cola();
    new spell_lost_isles_soulstone();
    new npc_lost_isles_mine_cart();
    new go_platform_control_panel();
    new spell_lost_isles_escape_velocity();
    new boss_trade_prince_gallywix();
    new npc_thrall_gallywix_fight();
    new npc_ultimate_footbomb_uniform();
    new npc_lost_isles_battleworg();
    new player_script_lost_isles_act34();
}
