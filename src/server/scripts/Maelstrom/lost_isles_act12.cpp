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
 * The Lost Isles, Acts 1-2: shipwreck shore through Oomlot Village.
 * Quests 14239/14474 (arrival) through 24924.
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

enum LostIslesAct12Quests
{
    QUEST_DONT_GO_INTO_THE_LIGHT    = 14239,
    QUEST_MINER_TROUBLES            = 14021,
    QUEST_BACK_TO_AGGRA             = 14303,
    QUEST_TO_THE_CLIFFS             = 14240,
    QUEST_PRECIOUS_CARGO            = 14242,
    QUEST_WARCHIEFS_REVENGE         = 14243,
    QUEST_UP_UP_AND_AWAY            = 14244,
    QUEST_ITS_A_TOWN_IN_A_BOX       = 14245,
    QUEST_A_GOBLIN_IN_SHARKS_CLOTHING = 24817
};

enum LostIslesAct12Creatures
{
    NPC_DOC_ZAPNOZZLE               = 36608,
    NPC_GOBLIN_SURVIVOR             = 34748,
    NPC_BOMB_THROWING_MONKEY        = 34699,
    NPC_MONKEY_BUSINESS_CREDIT      = 35760,
    NPC_FRIGHTENED_MINER            = 35813,
    NPC_ORE_CART                    = 35814,
    NPC_MINER_TROUBLES_CREDIT       = 35816,
    NPC_PYGMY_WITCHDOCTOR           = 35838,
    NPC_WEED_WHACKER_BUNNY          = 35903,
    NPC_BASTIA                      = 36585,
    NPC_GYROCHOPPA                  = 36143,
    NPC_THRALL_CLIFF                = 36145,
    NPC_CYCLONE_OF_THE_ELEMENTS     = 36178,
    NPC_SLING_ROCKET                = 36505,
    NPC_SLING_ROCKET_GALLYWIX       = 36514,
    NPC_WILD_CLUCKER                = 38111,
    NPC_CLUSTER_CLUCK_CREDIT        = 38117,
    NPC_MECHASHARK                  = 38318,
    NPC_NAGA_HATCHLING_1            = 38412,
    NPC_NAGA_HATCHLING_2            = 44580,
    NPC_POOL_PONY_CREDIT            = 38413
};

enum LostIslesAct12Spells
{
    // Don't Go Into the Light!
    SPELL_NEAR_DEATH                = 69010, // native stun + invisibility 7 + lying-down visual
    SPELL_SUMMON_DOC_ZAPNOZZLE      = 69018, // personal-spawn summon (SummonProperties 3052, flag 0x10)

    // Monkey Business
    SPELL_NITRO_POTASSIUM_BANANAS   = 67917,
    SPELL_EXPLODING_BANANAS         = 67919,

    // Miner Troubles
    SPELL_SUMMON_ORE_CART           = 68064,
    SPELL_ORE_CART_TRANSFORM        = 68065,
    SPELL_ORE_CART_CHAIN            = 68122,
    SPELL_MINER_CLEANUP             = 68060,

    // Capturing the Unknown
    SPELL_KTC_SNAPFLASH             = 68280,
    SPELL_SNAPFLASH_CHANNEL         = 68281,
    SPELL_SNAPFLASH_EFFECT          = 68296,

    // Weed Whacker
    SPELL_WEED_WHACKER_DUMMY        = 68211,
    SPELL_WEED_WHACKER_VEHICLE      = 68212,
    SPELL_SUMMON_WHACKER_BUNNY      = 68216,
    SPELL_WHACKER_BUNNY_RIDE        = 68217,
    SPELL_WHACKER_BUNNY_BEAM        = 68214,
    SPELL_WHACKER_BUNNY_DESPAWN     = 68215,

    // To the Cliffs / Precious Cargo / Warchief's Revenge
    SPELL_SUMMON_BASTIA             = 68973,
    SPELL_SUMMON_GYROCHOPPA         = 68386,
    SPELL_SUMMON_CYCLONE            = 68408,
    SPELL_CYCLONE_END               = 68439,
    SPELL_CYCLONE_ABANDON           = 68438,

    // Up, Up & Away!
    SPELL_SUMMON_SLING_ROCKET       = 68804,
    SPELL_GALLYWIX_ROCKET_COSMETIC  = 68819,
    SPELL_ROCKET_LANDING            = 68813, // force-casts 66127 (kill credit 50046 + explosion)

    // Cluster Cluck
    SPELL_REMOTE_FIREWORKS          = 71170,
    SPELL_CLUCKER_FIREWORKS         = 74177,

    // A Goblin in Shark's Clothing
    SPELL_SUMMON_MECHASHARK         = 71648,

    // Irresistible Pool Pony
    SPELL_SUMMON_HATCHLING_1        = 71919,
    SPELL_SUMMON_HATCHLING_2        = 71918,
    SPELL_SUMMON_HATCHLING_3        = 83115,
    SPELL_SUMMON_HATCHLING_4        = 83116
};

enum LostIslesAct12Misc
{
    ZONE_LOST_ISLES                 = 4720
};

// -----------------------------------------------------------------------------
// Don't Go Into the Light! (14239)
// -----------------------------------------------------------------------------

// 69018 - Don't Go Into The Light!: Summon Doc Zapnozzle.
// The spell is fired from spell_area twice on a fresh landing (the 69010
// aura-apply hook and the area-update loop both autocast it) and again on
// every login while the player is still Near Death - keep a single private
// Doc per player.
class spell_lost_isles_summon_doc_zapnozzle : public SpellScriptLoader
{
public:
    spell_lost_isles_summon_doc_zapnozzle() : SpellScriptLoader("spell_lost_isles_summon_doc_zapnozzle") { }

    class spell_lost_isles_summon_doc_zapnozzle_SpellScript : public SpellScript
    {
    public:
        void HandleSummon(SpellEffIndex effIndex)
        {
            Unit* caster = GetCaster();
            if (!caster)
                return;

            std::list<Creature*> docs;
            caster->GetCreatureListWithEntryInGrid(docs, NPC_DOC_ZAPNOZZLE, 250.0f);
            for (Creature* doc : docs)
                if (doc->GetPrivateObjectOwner() == caster->GetGUID())
                {
                    PreventHitDefaultEffect(effIndex);
                    return;
                }
        }

        void Register() override
        {
            OnEffectHit.Register(&spell_lost_isles_summon_doc_zapnozzle_SpellScript::HandleSummon, EFFECT_0, SPELL_EFFECT_SUMMON);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_lost_isles_summon_doc_zapnozzle_SpellScript();
    }
};

// 69013 - Don't Go Into The Light!: Quest Complete (quest 14239 reward spell).
// Retail ends the resuscitation here: the script effect strips Near Death,
// which unroots the player and stands them back up.
class spell_lost_isles_dont_go_into_the_light : public SpellScriptLoader
{
public:
    spell_lost_isles_dont_go_into_the_light() : SpellScriptLoader("spell_lost_isles_dont_go_into_the_light") { }

    class spell_lost_isles_dont_go_into_the_light_SpellScript : public SpellScript
    {
    public:
        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            if (Unit* target = GetHitUnit())
                target->RemoveAurasDueToSpell(SPELL_NEAR_DEATH);
        }

        void Register() override
        {
            OnEffectHitTarget.Register(&spell_lost_isles_dont_go_into_the_light_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_lost_isles_dont_go_into_the_light_SpellScript();
    }
};

// -----------------------------------------------------------------------------
// Frightened Miner escort (14021)
// -----------------------------------------------------------------------------

// Escort route extracted from the sniff (P3, miner Low 351277). The Pygmy
// Witchdoctor and the Smart Mining Monkeys along the way are static spawns;
// the quest credit fires when the witchdoctor dies (SAI on 35838).
Position const MinerSpawnPos = { 492.4184f, 2976.3213f, 8.040207f, 5.5267f };

struct MinerWaypoint
{
    Position Pos;
    uint32 PauseMs;     // pause after reaching the point
    int8 TextGroup;     // creature_text group to say on arrival (-1 = none)
};

MinerWaypoint const MinerRoute[] =
{
    { { 506.80f, 2976.93f, 7.14f },  0,     -1 },
    { { 516.90f, 2973.13f, 8.61f },  0,     -1 },
    { { 528.69f, 2961.12f, 6.86f },  0,     -1 },
    { { 538.82f, 2951.87f, 5.02f },  0,     -1 },
    { { 548.12f, 2943.13f, 2.30f },  3000,   1 }, // mine entrance: cave paintings line
    { { 572.20f, 2911.65f, -7.62f }, 0,     -1 },
    { { 557.69f, 2935.65f, 1.09f },  0,     -1 },
    { { 565.62f, 2936.04f, 0.30f },  0,     -1 },
    { { 572.24f, 2944.43f, -0.29f }, 0,     -1 },
    { { 579.50f, 2956.21f, -1.52f }, 0,     -1 },
    { { 587.78f, 2964.38f, -2.26f }, 18000,  3 }, // mining stop 1: accountant story
    { { 589.45f, 2939.09f, -7.32f }, 0,     -1 },
    { { 581.43f, 2923.87f, -6.39f }, 0,     -1 },
    { { 575.18f, 2905.77f, -7.21f }, 15000,  5 }, // mining stop 2: hit the jackpot
    { { 574.46f, 2890.24f, -7.65f }, 0,     -1 },
    { { 592.10f, 2876.20f, -6.64f }, 0,     -1 },
    { { 603.36f, 2865.64f, -6.54f }, 0,     -1 },
    { { 607.68f, 2857.41f, -7.00f }, 12000,  6 }, // mining stop 3: let's move on
    { { 620.28f, 2886.39f, -4.44f }, 0,     -1 },
    { { 628.28f, 2911.83f, -1.83f }, 0,     -1 },
    { { 635.80f, 2919.50f, -0.95f }, 0,     -1 },
    { { 647.74f, 2927.78f, -0.02f }, 0,     -1 },
    { { 658.50f, 2936.43f, 0.29f },  0,     -1 },
    { { 666.17f, 2947.39f, 0.12f },  10000, -1 }, // final chamber: witchdoctor fight
    { { 654.54f, 2974.84f, 1.53f },  0,     -1 },
    { { 660.71f, 2962.35f, 0.96f },  0,     -1 }  // final stop: credit + farewell (sniff 09:07:11)
};

// After the credit the miner runs back out along the tunnel and despawns
// mid-run (sniff 09:07:17-09:07:26).
Position const MinerRunOffRoute[] =
{
    { 654.05f, 2934.90f, 0.10f },
    { 632.76f, 2920.28f, -1.30f },
    { 622.06f, 2898.40f, -3.63f },
    { 613.32f, 2877.02f, -6.60f },
    { 603.03f, 2870.68f, -6.60f }
};

enum MinerEvents
{
    EVENT_MINER_START_WALK          = 1,
    EVENT_MINER_NEXT_POINT          = 2,
    EVENT_MINER_SECOND_EMOTE        = 3,
    EVENT_MINER_RUN_OFF             = 4
};

uint32 const MINER_RUNOFF_POINT_BASE = 100;

class npc_frightened_miner : public CreatureScript
{
public:
    npc_frightened_miner() : CreatureScript("npc_frightened_miner") { }

    struct npc_frightened_minerAI : public ScriptedAI
    {
        npc_frightened_minerAI(Creature* creature) : ScriptedAI(creature), _pointIndex(0), _watchdog(5000) { }

        void IsSummonedBy(Unit* summoner) override
        {
            // Summoned either by the player (quest accept) or by Foreman
            // Dampwick's gossip re-summon; the escort belongs to the nearest
            // player on the quest either way.
            Player* owner = summoner ? summoner->ToPlayer() : nullptr;
            if (!owner)
            {
                std::list<Player*> players;
                me->GetPlayerListInGrid(players, 30.0f);
                for (Player* candidate : players)
                {
                    if (candidate->GetQuestStatus(QUEST_MINER_TROUBLES) == QUEST_STATUS_INCOMPLETE)
                    {
                        owner = candidate;
                        break;
                    }
                }
            }

            if (owner)
                _ownerGUID = owner->GetGUID();

            if (sSpellMgr->GetSpellInfo(SPELL_SUMMON_ORE_CART))
                me->CastSpell(me, SPELL_SUMMON_ORE_CART, true);

            Talk(0);
            _events.ScheduleEvent(EVENT_MINER_START_WALK, 10s);
        }

        void MovementInform(uint32 type, uint32 pointId) override
        {
            if (type != POINT_MOTION_TYPE)
                return;

            // Run-off legs after the credit.
            if (pointId >= MINER_RUNOFF_POINT_BASE)
            {
                uint32 runoffIndex = pointId - MINER_RUNOFF_POINT_BASE + 1;
                if (runoffIndex >= std::size(MinerRunOffRoute))
                {
                    me->DespawnOrUnsummon(200ms);
                    return;
                }
                me->GetMotionMaster()->MovePoint(MINER_RUNOFF_POINT_BASE + runoffIndex, MinerRunOffRoute[runoffIndex]);
                return;
            }

            if (pointId != _pointIndex)
                return;

            MinerWaypoint const& point = MinerRoute[_pointIndex];
            if (point.TextGroup >= 0)
                Talk(uint8(point.TextGroup));

            ++_pointIndex;
            if (_pointIndex >= std::size(MinerRoute))
            {
                // Final stop reached (retail: credit fires here, NOT on the
                // witchdoctor's death - the witchdoctor may still be fighting).
                me->HandleEmoteCommand(EMOTE_ONESHOT_TALK_NO_SHEATHE);
                if (Player* owner = ObjectAccessor::GetPlayer(*me, _ownerGUID))
                    owner->KilledMonsterCredit(NPC_MINER_TROUBLES_CREDIT);
                Talk(4);
                _events.ScheduleEvent(EVENT_MINER_SECOND_EMOTE, 1500ms);
                _events.ScheduleEvent(EVENT_MINER_RUN_OFF, 6s);
                return;
            }

            _events.ScheduleEvent(EVENT_MINER_NEXT_POINT, Milliseconds(500 + point.PauseMs));
        }

        void UpdateAI(uint32 diff) override
        {
            _events.Update(diff);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_MINER_START_WALK:
                    case EVENT_MINER_NEXT_POINT:
                        if (_pointIndex < std::size(MinerRoute))
                            me->GetMotionMaster()->MovePoint(_pointIndex, MinerRoute[_pointIndex].Pos);
                        break;
                    case EVENT_MINER_SECOND_EMOTE:
                        me->HandleEmoteCommand(EMOTE_ONESHOT_TALK_NO_SHEATHE);
                        break;
                    case EVENT_MINER_RUN_OFF:
                        _runningOff = true;
                        me->CombatStop(true);
                        me->SetWalk(false);
                        me->GetMotionMaster()->MovePoint(MINER_RUNOFF_POINT_BASE, MinerRunOffRoute[0]);
                        // Failsafe if a run-off leg never completes.
                        me->DespawnOrUnsummon(25s);
                        break;
                    default:
                        break;
                }
            }

            // Combat interrupts point movement; nudge the walk back on track.
            if (_watchdog <= diff)
            {
                _watchdog = 5000;
                if (!_runningOff && !me->IsInCombat() && !me->isMoving() && _pointIndex < std::size(MinerRoute) && _events.Empty())
                    me->GetMotionMaster()->MovePoint(_pointIndex, MinerRoute[_pointIndex].Pos);
            }
            else
                _watchdog -= diff;

            if (UpdateVictim())
                DoMeleeAttackIfReady();
        }

    private:
        EventMap _events;
        ObjectGuid _ownerGUID;
        uint32 _pointIndex;
        uint32 _watchdog;
        bool _runningOff = false;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_frightened_minerAI(creature);
    }
};

// 35814 - Miner Troubles Ore Cart: transforms, tethers to the miner (68122)
// and trundles along behind him (retail: independent follower, no vehicle).
class npc_lost_isles_ore_cart : public CreatureScript
{
public:
    npc_lost_isles_ore_cart() : CreatureScript("npc_lost_isles_ore_cart") { }

    struct npc_lost_isles_ore_cartAI : public ScriptedAI
    {
        npc_lost_isles_ore_cartAI(Creature* creature) : ScriptedAI(creature), _checkTimer(2000) { }

        void IsSummonedBy(Unit* summoner) override
        {
            if (!summoner)
                return;

            _minerGUID = summoner->GetGUID();
            me->SetReactState(REACT_PASSIVE);
            me->CastSpell(me, SPELL_ORE_CART_TRANSFORM, true);
            me->CastSpell(summoner, SPELL_ORE_CART_CHAIN, true);
            me->GetMotionMaster()->MoveFollow(summoner, 2.0f, float(M_PI));
        }

        void UpdateAI(uint32 diff) override
        {
            if (_checkTimer <= diff)
            {
                _checkTimer = 2000;
                Unit* miner = ObjectAccessor::GetUnit(*me, _minerGUID);
                if (!miner || !miner->IsAlive())
                    me->DespawnOrUnsummon(1200ms); // retail: cart destroyed ~1.2s after the miner
            }
            else
                _checkTimer -= diff;
        }

    private:
        ObjectGuid _minerGUID;
        uint32 _checkTimer;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_lost_isles_ore_cartAI(creature);
    }
};

// -----------------------------------------------------------------------------
// Weed Whacker (14236) - the player becomes the vehicle (SET_VEHICLE_ID 493).
// -----------------------------------------------------------------------------

// 68211 - Weed Whacker: chain into the mount + weapon auras.
class spell_lost_isles_weed_whacker : public SpellScriptLoader
{
public:
    spell_lost_isles_weed_whacker() : SpellScriptLoader("spell_lost_isles_weed_whacker") { }

    class spell_lost_isles_weed_whacker_SpellScript : public SpellScript
    {
    public:
        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            Unit* caster = GetCaster();
            if (!caster)
                return;

            // Vehicle aura first: the bunny boards the player, so the player
            // must already own the vehicle kit when the bunny initializes.
            if (sSpellMgr->GetSpellInfo(SPELL_WEED_WHACKER_VEHICLE))
                caster->CastSpell(caster, SPELL_WEED_WHACKER_VEHICLE, true);
            if (sSpellMgr->GetSpellInfo(SPELL_SUMMON_WHACKER_BUNNY))
                caster->CastSpell(caster, SPELL_SUMMON_WHACKER_BUNNY, true);
        }

        void Register() override
        {
            OnEffectHitTarget.Register(&spell_lost_isles_weed_whacker_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_lost_isles_weed_whacker_SpellScript();
    }
};

// 68212 - Weed Whacker vehicle aura: tear down the bunny when it drops.
class spell_lost_isles_weed_whacker_aura : public SpellScriptLoader
{
public:
    spell_lost_isles_weed_whacker_aura() : SpellScriptLoader("spell_lost_isles_weed_whacker_aura") { }

    class spell_lost_isles_weed_whacker_aura_AuraScript : public AuraScript
    {
    public:
        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            Unit* target = GetTarget();
            if (!target)
                return;

            if (sSpellMgr->GetSpellInfo(SPELL_WHACKER_BUNNY_DESPAWN))
                target->CastSpell(target, SPELL_WHACKER_BUNNY_DESPAWN, true);

            std::list<Creature*> bunnies;
            target->GetCreatureListWithEntryInGrid(bunnies, NPC_WEED_WHACKER_BUNNY, 30.0f);
            for (Creature* bunny : bunnies)
                if (bunny->GetOwnerGUID() == target->GetGUID() || (bunny->ToTempSummon() && bunny->ToTempSummon()->GetSummonerGUID() == target->GetGUID()))
                    bunny->DespawnOrUnsummon();
        }

        void Register() override
        {
            AfterEffectRemove.Register(&spell_lost_isles_weed_whacker_aura_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_SET_VEHICLE_ID, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_lost_isles_weed_whacker_aura_AuraScript();
    }
};

// 35903 - Weed Whacker Channel Bunny: rides the player and beams the whacker.
class npc_weed_whacker_bunny : public CreatureScript
{
public:
    npc_weed_whacker_bunny() : CreatureScript("npc_weed_whacker_bunny") { }

    struct npc_weed_whacker_bunnyAI : public ScriptedAI
    {
        npc_weed_whacker_bunnyAI(Creature* creature) : ScriptedAI(creature), _checkTimer(2000) { }

        void IsSummonedBy(Unit* summoner) override
        {
            if (!summoner)
                return;

            _ownerGUID = summoner->GetGUID();
            me->SetReactState(REACT_PASSIVE);

            // 68217's implicit master-target cannot resolve for a plain summon;
            // board the player-vehicle (68212's SET_VEHICLE_ID) directly.
            if (summoner->GetVehicleKit())
                me->EnterVehicle(summoner, 0);
            if (sSpellMgr->GetSpellInfo(SPELL_WHACKER_BUNNY_BEAM))
                me->CastSpell(summoner, SPELL_WHACKER_BUNNY_BEAM, true);
        }

        void UpdateAI(uint32 diff) override
        {
            if (_checkTimer <= diff)
            {
                _checkTimer = 2000;
                Unit* owner = ObjectAccessor::GetUnit(*me, _ownerGUID);
                if (!owner || !owner->HasAura(SPELL_WEED_WHACKER_VEHICLE))
                    me->DespawnOrUnsummon();
            }
            else
                _checkTimer -= diff;
        }

    private:
        ObjectGuid _ownerGUID;
        uint32 _checkTimer;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_weed_whacker_bunnyAI(creature);
    }
};

// -----------------------------------------------------------------------------
// Monkey Business (14019)
// -----------------------------------------------------------------------------

// 67917 - Nitro-Potassium Bananas: feed a Bomb-Throwing Monkey.
class spell_lost_isles_exploding_bananas : public SpellScriptLoader
{
public:
    spell_lost_isles_exploding_bananas() : SpellScriptLoader("spell_lost_isles_exploding_bananas") { }

    class spell_lost_isles_exploding_bananas_SpellScript : public SpellScript
    {
    public:
        SpellCastResult CheckTarget()
        {
            Unit* target = GetExplTargetUnit();
            if (!target || target->GetEntry() != NPC_BOMB_THROWING_MONKEY || !target->IsAlive()
                || target->HasAura(SPELL_EXPLODING_BANANAS))
                return SPELL_FAILED_BAD_TARGETS;
            return SPELL_CAST_OK;
        }

        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            Creature* monkey = GetHitCreature();
            Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
            if (!monkey || !player)
                return;

            player->KilledMonsterCredit(NPC_MONKEY_BUSINESS_CREDIT);

            if (monkey->HasAura(SPELL_EXPLODING_BANANAS))
                return;

            monkey->SetFacingToObject(player);
            monkey->CastSpell(monkey, SPELL_EXPLODING_BANANAS, false);
            // Retail (P3 sniff): the monkey munches the banana and blows up ~4.5s later,
            // dying for real and leaving a corpse.
            monkey->m_Events.AddEventAtOffset([monkey]()
            {
                monkey->KillSelf();
            }, 4500ms);
        }

        void Register() override
        {
            OnCheckCast.Register(&spell_lost_isles_exploding_bananas_SpellScript::CheckTarget);
            OnEffectHitTarget.Register(&spell_lost_isles_exploding_bananas_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_lost_isles_exploding_bananas_SpellScript();
    }
};

// -----------------------------------------------------------------------------
// Capturing the Unknown (14031)
// -----------------------------------------------------------------------------

// 68280 - KTC Snapflash: start the capture channel on the vignette bunny.
class spell_lost_isles_ktc_snapflash : public SpellScriptLoader
{
public:
    spell_lost_isles_ktc_snapflash() : SpellScriptLoader("spell_lost_isles_ktc_snapflash") { }

    class spell_lost_isles_ktc_snapflash_SpellScript : public SpellScript
    {
    public:
        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            Unit* caster = GetCaster();
            Creature* target = GetHitCreature();
            if (!caster || !target)
                return;

            if (sSpellMgr->GetSpellInfo(SPELL_SNAPFLASH_CHANNEL))
                caster->CastSpell(target, SPELL_SNAPFLASH_CHANNEL, false);
        }

        void Register() override
        {
            OnEffectHitTarget.Register(&spell_lost_isles_ktc_snapflash_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_lost_isles_ktc_snapflash_SpellScript();
    }
};

// 68296 - Snapflash effect: credit the photographed vignette.
class spell_lost_isles_snapflash_effect : public SpellScriptLoader
{
public:
    spell_lost_isles_snapflash_effect() : SpellScriptLoader("spell_lost_isles_snapflash_effect") { }

    class spell_lost_isles_snapflash_effect_SpellScript : public SpellScript
    {
    public:
        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
            Creature* target = GetHitCreature();
            if (!player)
                if (Unit* caster = GetCaster())
                    if (Unit* owner = caster->GetCharmerOrOwner())
                        player = owner->ToPlayer();

            if (player && target)
                player->KilledMonsterCredit(target->GetEntry());
        }

        void Register() override
        {
            OnEffectHitTarget.Register(&spell_lost_isles_snapflash_effect_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_lost_isles_snapflash_effect_SpellScript();
    }
};

// -----------------------------------------------------------------------------
// Vehicles: Bastia (14240 / 25100), Gyrochoppa (14242), Cyclone (14243),
// Sling Rocket (14244)
// -----------------------------------------------------------------------------

class npc_lost_isles_bastia : public CreatureScript
{
public:
    npc_lost_isles_bastia() : CreatureScript("npc_lost_isles_bastia") { }

    struct npc_lost_isles_bastiaAI : public VehicleAI
    {
        npc_lost_isles_bastiaAI(Creature* creature) : VehicleAI(creature) { }

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
        return new npc_lost_isles_bastiaAI(creature);
    }
};

// Flight spline from the sniff (P3, mover Low 352417): the ride ends at the
// overlook by Thrall's boat where the player is ejected.
Position const GyrochoppaPath[] =
{
    { 846.74f, 3335.54f, 10.14f },
    { 755.89f, 3374.22f, 14.39f },
    { 780.33f, 3449.16f, 14.39f },
    { 1071.62f, 3589.42f, 26.22f },
    { 1098.78f, 3729.42f, 93.03f },
    { 971.51f, 3802.34f, 14.36f }
};

enum GyrochoppaEvents
{
    EVENT_GYROCHOPPA_TAKEOFF        = 1
};

class npc_lost_isles_gyrochoppa : public CreatureScript
{
public:
    npc_lost_isles_gyrochoppa() : CreatureScript("npc_lost_isles_gyrochoppa") { }

    struct npc_lost_isles_gyrochoppaAI : public VehicleAI
    {
        npc_lost_isles_gyrochoppaAI(Creature* creature) : VehicleAI(creature) { }

        void PassengerBoarded(Unit* passenger, int8 seatId, bool apply) override
        {
            if (seatId != 0 || !passenger->IsPlayer())
                return;

            if (apply)
            {
                _passengerGUID = passenger->GetGUID();
                _events.ScheduleEvent(EVENT_GYROCHOPPA_TAKEOFF, 2s);
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

            if (pointId != std::size(GyrochoppaPath) - 1)
                return;

            if (Player* player = ObjectAccessor::GetPlayer(*me, _passengerGUID))
            {
                player->KilledMonsterCredit(NPC_THRALL_CLIFF);
                player->ExitVehicle();
            }
            me->DespawnOrUnsummon(3000);
        }

        void UpdateAI(uint32 diff) override
        {
            VehicleAI::UpdateAI(diff);
            _events.Update(diff);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                if (eventId == EVENT_GYROCHOPPA_TAKEOFF)
                {
                    me->SetCanFly(true);
                    me->SetDisableGravity(true);
                    me->GetMotionMaster()->MoveSmoothPath(uint32(std::size(GyrochoppaPath) - 1), GyrochoppaPath, std::size(GyrochoppaPath), false, true);
                }
            }
        }

    private:
        EventMap _events;
        ObjectGuid _passengerGUID;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_lost_isles_gyrochoppaAI(creature);
    }
};

class npc_lost_isles_cyclone : public CreatureScript
{
public:
    npc_lost_isles_cyclone() : CreatureScript("npc_lost_isles_cyclone") { }

    struct npc_lost_isles_cycloneAI : public VehicleAI
    {
        npc_lost_isles_cycloneAI(Creature* creature) : VehicleAI(creature) { }

        void IsSummonedBy(Unit* /*summoner*/) override
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetDisableGravity(true);
            me->SetControlled(true, UNIT_STATE_ROOT);
        }

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
        return new npc_lost_isles_cycloneAI(creature);
    }
};

// Launch spline from the sniff (P3, mover Low 352655).
Position const SlingRocketPath[] =
{
    { 878.42f, 2740.18f, 130.86f },
    { 882.46f, 2726.06f, 146.09f },
    { 890.91f, 2697.69f, 164.53f },
    { 916.37f, 2591.61f, 207.50f },
    { 928.46f, 2538.97f, 196.31f },
    { 932.94f, 2506.70f, 167.42f },
    { 941.32f, 2463.62f, 111.22f },
    { 945.63f, 2440.61f, 69.72f },
    { 945.26f, 2396.82f, 4.59f }
};

enum SlingRocketEvents
{
    EVENT_ROCKET_LAUNCH             = 1
};

class npc_lost_isles_sling_rocket : public CreatureScript
{
public:
    npc_lost_isles_sling_rocket() : CreatureScript("npc_lost_isles_sling_rocket") { }

    struct npc_lost_isles_sling_rocketAI : public VehicleAI
    {
        npc_lost_isles_sling_rocketAI(Creature* creature) : VehicleAI(creature) { }

        void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
        {
            if (!passenger->IsPlayer())
                return;

            if (apply)
            {
                _passengerGUID = passenger->GetGUID();
                _events.ScheduleEvent(EVENT_ROCKET_LAUNCH, 1500ms);
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

            if (pointId != std::size(SlingRocketPath) - 1)
                return;

            // 68813 force-casts 66127: kill credit 50046 + the crash explosion.
            if (Player* player = ObjectAccessor::GetPlayer(*me, _passengerGUID))
            {
                if (sSpellMgr->GetSpellInfo(SPELL_ROCKET_LANDING))
                    me->CastSpell(player, SPELL_ROCKET_LANDING, true);
                else
                    player->KilledMonsterCredit(50046);
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
                if (eventId == EVENT_ROCKET_LAUNCH)
                {
                    me->SetCanFly(true);
                    me->SetDisableGravity(true);
                    me->GetMotionMaster()->MoveSmoothPath(uint32(std::size(SlingRocketPath) - 1), SlingRocketPath, std::size(SlingRocketPath), false, true);
                }
            }
        }

    private:
        EventMap _events;
        ObjectGuid _passengerGUID;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_lost_isles_sling_rocketAI(creature);
    }
};

// 196439 - Rocket Sling: launches the player toward the town site (14244).
class go_rocket_sling : public GameObjectScript
{
public:
    go_rocket_sling() : GameObjectScript("go_rocket_sling") { }

    struct go_rocket_slingAI : public GameObjectAI
    {
        go_rocket_slingAI(GameObject* go) : GameObjectAI(go) { }

        bool GossipHello(Player* player) override
        {
            if (player->GetQuestStatus(QUEST_UP_UP_AND_AWAY) != QUEST_STATUS_INCOMPLETE)
                return true;

            if (player->GetVehicle())
                return true;

            if (sSpellMgr->GetSpellInfo(SPELL_SUMMON_SLING_ROCKET))
            {
                player->CastSpell(player, SPELL_SUMMON_SLING_ROCKET, true);
                if (sSpellMgr->GetSpellInfo(SPELL_GALLYWIX_ROCKET_COSMETIC))
                    player->CastSpell(player, SPELL_GALLYWIX_ROCKET_COSMETIC, true);
            }
            else if (Creature* rocket = player->SummonCreature(NPC_SLING_ROCKET, { 878.42f, 2740.18f, 130.78f, 4.7f }, TEMPSUMMON_MANUAL_DESPAWN))
                player->EnterVehicle(rocket, 0);

            return true;
        }
    };

    GameObjectAI* GetAI(GameObject* go) const override
    {
        return new go_rocket_slingAI(go);
    }
};

// -----------------------------------------------------------------------------
// Cluster Cluck (24671)
// -----------------------------------------------------------------------------

// 71170 - Remote Fireworks: catch a Wild Clucker.
class spell_lost_isles_remote_fireworks : public SpellScriptLoader
{
public:
    spell_lost_isles_remote_fireworks() : SpellScriptLoader("spell_lost_isles_remote_fireworks") { }

    class spell_lost_isles_remote_fireworks_SpellScript : public SpellScript
    {
    public:
        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
            Creature* clucker = GetHitCreature();
            if (!player || !clucker || clucker->GetEntry() != NPC_WILD_CLUCKER)
                return;

            player->KilledMonsterCredit(NPC_CLUSTER_CLUCK_CREDIT);

            if (sSpellMgr->GetSpellInfo(SPELL_CLUCKER_FIREWORKS))
                clucker->CastSpell(clucker, SPELL_CLUCKER_FIREWORKS, true);
            clucker->DespawnOrUnsummon(4000, 60s);
        }

        void Register() override
        {
            OnEffectHitTarget.Register(&spell_lost_isles_remote_fireworks_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_lost_isles_remote_fireworks_SpellScript();
    }
};

// -----------------------------------------------------------------------------
// A Goblin in Shark's Clothing (24817)
// -----------------------------------------------------------------------------

Position const MechasharkSpawnPos = { 905.50f, 2432.40f, -9.53f, 4.5f };

// 71648 - Summon Mechashark X-Steam: the nearby-entry dest cannot resolve
// (nothing with the summoned entry exists); pin the destination to the water
// beside the controller dock.
class spell_lost_isles_summon_mechashark : public SpellScriptLoader
{
public:
    spell_lost_isles_summon_mechashark() : SpellScriptLoader("spell_lost_isles_summon_mechashark") { }

    class spell_lost_isles_summon_mechashark_SpellScript : public SpellScript
    {
    public:
        void SetDest(SpellDestination& dest)
        {
            dest.Relocate(MechasharkSpawnPos);
        }

        void Register() override
        {
            OnDestinationTargetSelect.Register(&spell_lost_isles_summon_mechashark_SpellScript::SetDest, EFFECT_0, TARGET_DEST_NEARBY_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_lost_isles_summon_mechashark_SpellScript();
    }
};

// -----------------------------------------------------------------------------
// Irresistible Pool Pony (24864)
// -----------------------------------------------------------------------------

// 71919/71918/83115/83116 - rescue a Naga Hatchling (summons the follower
// natively; the clicked static hatchling is credited and removed here).
class spell_lost_isles_pool_pony_click : public SpellScriptLoader
{
public:
    spell_lost_isles_pool_pony_click() : SpellScriptLoader("spell_lost_isles_pool_pony_click") { }

    class spell_lost_isles_pool_pony_click_SpellScript : public SpellScript
    {
    public:
        void HandleAfterCast()
        {
            Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
            if (!player)
                return;

            player->KilledMonsterCredit(NPC_POOL_PONY_CREDIT);

            if (Unit* target = GetExplTargetUnit())
                if (Creature* hatchling = target->ToCreature())
                    if (hatchling->GetEntry() == NPC_NAGA_HATCHLING_1 || hatchling->GetEntry() == NPC_NAGA_HATCHLING_2)
                        hatchling->DespawnOrUnsummon(0, 30s);
        }

        void Register() override
        {
            AfterCast.Register(&spell_lost_isles_pool_pony_click_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_lost_isles_pool_pony_click_SpellScript();
    }
};

// -----------------------------------------------------------------------------
// Shared player script: arrival auto-quest, accept-casts, phase nudges, cleanup.
// -----------------------------------------------------------------------------

class player_script_lost_isles : public PlayerScript
{
public:
    player_script_lost_isles() : PlayerScript("player_script_lost_isles") { }

    void OnUpdateZone(Player* player, uint32 newZone, uint32 /*newArea*/) override
    {
        // Retail auto-accepts "Don't Go Into the Light!" on washing ashore.
        if (newZone != ZONE_LOST_ISLES)
            return;

        if (player->GetQuestStatus(QUEST_DONT_GO_INTO_THE_LIGHT) != QUEST_STATUS_NONE)
            return;

        Quest const* quest = sObjectMgr->GetQuestTemplate(QUEST_DONT_GO_INTO_THE_LIGHT);
        if (quest && player->CanAddQuest(quest, false))
            player->AddQuestAndCheckCompletion(quest, nullptr);
    }

    void OnQuestStatusChange(Player* player, uint32 questId) override
    {
        QuestStatus status = player->GetQuestStatus(questId);

        switch (questId)
        {
            case QUEST_MINER_TROUBLES:
                if (status == QUEST_STATUS_INCOMPLETE)
                {
                    if (!FindOwnedCreature(player, NPC_FRIGHTENED_MINER))
                        // Retail summon prop 2261 carries the personal-spawn flag:
                        // every rescuer gets their own private escort.
                        player->SummonCreature(NPC_FRIGHTENED_MINER, MinerSpawnPos, TEMPSUMMON_MANUAL_DESPAWN, 0, 0, player->GetGUID());
                }
                else if (status == QUEST_STATUS_NONE || status == QUEST_STATUS_FAILED || status == QUEST_STATUS_REWARDED)
                {
                    CleanupOwnedCreatures(player, { NPC_FRIGHTENED_MINER, NPC_ORE_CART });
                    if (status == QUEST_STATUS_REWARDED && sSpellMgr->GetSpellInfo(SPELL_MINER_CLEANUP))
                        player->CastSpell(player, SPELL_MINER_CLEANUP, true);
                }
                break;
            case QUEST_BACK_TO_AGGRA:
                if (status == QUEST_STATUS_REWARDED)
                    PhasingHandler::OnConditionChange(player); // 170 -> 171
                break;
            case QUEST_TO_THE_CLIFFS:
                if (status == QUEST_STATUS_INCOMPLETE)
                    CastAcceptSpell(player, SPELL_SUMMON_BASTIA);
                else if (status == QUEST_STATUS_NONE || status == QUEST_STATUS_FAILED || status == QUEST_STATUS_REWARDED)
                {
                    CleanupOwnedCreatures(player, { NPC_BASTIA });
                    if (status == QUEST_STATUS_REWARDED)
                        PhasingHandler::OnConditionChange(player); // 171 -> 172
                }
                break;
            case QUEST_PRECIOUS_CARGO:
                if (status == QUEST_STATUS_INCOMPLETE)
                    CastAcceptSpell(player, SPELL_SUMMON_GYROCHOPPA);
                else if (status == QUEST_STATUS_NONE || status == QUEST_STATUS_FAILED || status == QUEST_STATUS_REWARDED)
                {
                    CleanupOwnedCreatures(player, { NPC_GYROCHOPPA });
                    if (status == QUEST_STATUS_REWARDED)
                        PhasingHandler::OnConditionChange(player); // 172 -> 179
                }
                break;
            case QUEST_WARCHIEFS_REVENGE:
                if (status == QUEST_STATUS_INCOMPLETE)
                    CastAcceptSpell(player, SPELL_SUMMON_CYCLONE);
                else if (status == QUEST_STATUS_COMPLETE)
                {
                    if (player->GetVehicleBase() && player->GetVehicleBase()->GetEntry() == NPC_CYCLONE_OF_THE_ELEMENTS)
                        player->ExitVehicle();
                    if (sSpellMgr->GetSpellInfo(SPELL_CYCLONE_END))
                        player->CastSpell(player, SPELL_CYCLONE_END, true);
                }
                else if (status == QUEST_STATUS_NONE || status == QUEST_STATUS_FAILED || status == QUEST_STATUS_REWARDED)
                {
                    if (status == QUEST_STATUS_NONE && sSpellMgr->GetSpellInfo(SPELL_CYCLONE_ABANDON))
                        player->CastSpell(player, SPELL_CYCLONE_ABANDON, true);
                    CleanupOwnedCreatures(player, { NPC_CYCLONE_OF_THE_ELEMENTS });
                }
                break;
            case QUEST_UP_UP_AND_AWAY:
                if (status == QUEST_STATUS_NONE || status == QUEST_STATUS_FAILED || status == QUEST_STATUS_REWARDED)
                    CleanupOwnedCreatures(player, { NPC_SLING_ROCKET, NPC_SLING_ROCKET_GALLYWIX });
                break;
            case QUEST_ITS_A_TOWN_IN_A_BOX:
                if (status == QUEST_STATUS_COMPLETE)
                    PhasingHandler::OnConditionChange(player); // 179 -> 180 (town deployed)
                break;
            case QUEST_A_GOBLIN_IN_SHARKS_CLOTHING:
                if (status == QUEST_STATUS_NONE || status == QUEST_STATUS_FAILED || status == QUEST_STATUS_REWARDED)
                    CleanupOwnedCreatures(player, { NPC_MECHASHARK });
                break;
            default:
                break;
        }
    }

private:
    static void CastAcceptSpell(Player* player, uint32 spellId)
    {
        if (!sSpellMgr->GetSpellInfo(spellId))
            return;
        if (player->GetVehicle())
            return;
        player->CastSpell(player, spellId, true);
    }

    static Creature* FindOwnedCreature(Player* player, uint32 entry)
    {
        std::list<Creature*> creatures;
        player->GetCreatureListWithEntryInGrid(creatures, entry, 250.0f);
        for (Creature* creature : creatures)
        {
            if (creature->GetOwnerGUID() == player->GetGUID())
                return creature;
            if (TempSummon* summon = creature->ToTempSummon())
                if (summon->GetSummonerGUID() == player->GetGUID())
                    return creature;
        }
        return nullptr;
    }

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

void AddSC_lost_isles_act12()
{
    new spell_lost_isles_summon_doc_zapnozzle();
    new spell_lost_isles_dont_go_into_the_light();
    new npc_frightened_miner();
    new npc_lost_isles_ore_cart();
    new spell_lost_isles_weed_whacker();
    new spell_lost_isles_weed_whacker_aura();
    new npc_weed_whacker_bunny();
    new spell_lost_isles_exploding_bananas();
    new spell_lost_isles_ktc_snapflash();
    new spell_lost_isles_snapflash_effect();
    new npc_lost_isles_bastia();
    new npc_lost_isles_gyrochoppa();
    new npc_lost_isles_cyclone();
    new npc_lost_isles_sling_rocket();
    new go_rocket_sling();
    new spell_lost_isles_remote_fireworks();
    new spell_lost_isles_summon_mechashark();
    new spell_lost_isles_pool_pony_click();
    new player_script_lost_isles();
}
