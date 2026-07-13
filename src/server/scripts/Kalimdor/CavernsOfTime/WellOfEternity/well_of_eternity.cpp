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
#include "GameObject.h"
#include "GameObjectAI.h"
#include "InstanceScript.h"
#include "Map.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "PassiveAI.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "TemporarySummon.h"
#include "Vehicle.h"

namespace WellOfEternity
{
/*######
## npc_woe_illidan_gauntlet - the Courtyard of Lights escort (55500)
######*/

namespace IllidanGauntlet
{
enum Texts
{
    SAY_INTRO_1             = 0,  // Over here, in the shadows.
    SAY_INTRO_2             = 1,  // I think we stand a better chance fighting alongside one another.
    SAY_CLOAK               = 2,  // We now hide in shadows, hidden from our enemies.
    SAY_ESCORT_START        = 3,  // Come with me if you'd like to live long enough to see me save this world!
    SAY_GUARDIAN_WARNING    = 4,  // I've seen a single Guardian Demon slaughter a hundred elves. Tread lightly.
    SAY_WALL_OF_SHADOW      = 5,  // I will hold them back so we can get past. Be ready.
    SAY_MAGIC_FADING        = 6,  // My magic is fading. I'm going through!
    SAY_ATTACK              = 7,  // Attack. I don't like to be kept waiting.
    SAY_DEATH_TO_LEGION     = 8,  // Death to the Legion!
    SAY_DESTROY_FOCUS       = 9,  // Destroy the portal energy focus!
    SAY_LEAVING             = 10, // We're leaving. Stay close.
    SAY_ENDLESS             = 11, // They come endlessly from the palace.
    SAY_FIRST_KILL          = 12, // I'll let you have the first kill. Don't make me regret that.
    SAY_SMASH_CRYSTAL       = 13, // Smash the crystal. We need to move.
    SAY_STENCH              = 14, // The stench of sulfur and brimstone...
    SAY_CUT_DOWN            = 15, // Cut this one down from the shadows.
    SAY_FINAL_PORTAL        = 16, // Let us shut down this final portal and finish this.
    SAY_DESTROY_CRYSTAL_3   = 17, // Destroy the crystal so we can move on.
    SAY_DEMONS_LEAVING      = 18, // The demons should all be leaving. We will be at the palace in no time.
    SAY_NO_LONGER_POURING   = 19, // The demons are no longer pouring from the palace. We can move ahead.
    SAY_TOO_EASY            = 20, // Too easy.
    SAY_ANOTHER_DEMON       = 21, // Another demon, ready to be slaughtered.
    SAY_NOTHING_STOPS       = 22, // Nothing will stop me. Not even you, demon.
    SAY_DRAIN_COUNTER       = 23, // Your magic is pathetic. Let me show you mine.
    SAY_RETURN_SHADOWS      = 24, // Return to the shadows!
    SAY_STRENGTH_RETURNS    = 25, // My strength returns.
    SAY_HUNTER_PREY         = 26, // The hunter became the prey.
    SAY_FAREWELL            = 27, // You did well, but for now I must continue alone. Good hunting.
    SAY_WAITING             = 28  // Waiting to attack...
};

enum Spells
{
    SPELL_WALL_OF_SHADOW            = 104400,
    SPELL_DISTRACT_DEMON_MISSILE    = 110121,
    SPELL_DRAIN_ESSENCE_FEEDBACK    = 104906,
    SPELL_ABSORB_FEL_ENERGY         = 105543,
    SPELL_SHADOW_BOLT               = 105546,
    SPELL_REGENERATION              = 105547,
    SPELL_RETURN_TO_THE_SHADOWS     = 105635
};

enum Actions
{
    // 1-9 reserved for shared header actions
    ACTION_ADVANCE_STAGE            = 10 // a Portal Energy Focus was destroyed
};

enum Events
{
    EVENT_INTRO_2 = 1,
    EVENT_CLOAK_PULSE,
    EVENT_LEG_FLAVOR,
    EVENT_HOLD_ATTACK,
    EVENT_HOLD_FIRST_KILL,
    EVENT_HOLD_DESTROY_FOCUS,
    EVENT_DRAIN_FEEDBACK,
    EVENT_DRAIN_ABSORB,
    EVENT_DRAIN_SHADOW_BOLT,
    EVENT_DRAIN_REGENERATE,
    EVENT_DRAIN_RECLOAK,
    EVENT_OUTRO_HUNTER_PREY,
    EVENT_OUTRO_FAREWELL,
    EVENT_OUTRO_DEPART
};

enum Points
{
    POINT_ESCORT_BASE   = 100
};

uint8 constexpr MaxEscortStages = 4;

struct EscortLeg
{
    uint8 NodeCount;
    Position Nodes[6];
};

// Leg N walks Illidan from stage-N spawn/hold toward the next hold point.
EscortLeg const EscortLegs[MaxEscortStages] =
{
    // ledge -> portal 1 (west)
    { 5, { { 3190.932f, -4894.249f, 194.356f }, { 3216.731f, -4935.000f, 190.000f }, { 3245.000f, -4960.000f, 184.000f }, { 3270.000f, -4975.000f, 181.300f }, { 3294.200f, -4981.970f, 181.160f } } },
    // portal 1 -> portal 2 (center-east)
    { 4, { { 3327.893f, -4878.755f, 181.079f }, { 3370.000f, -4870.000f, 181.100f }, { 3420.000f, -4878.000f, 181.100f }, { 3444.980f, -4886.340f, 181.160f } } },
    // portal 2 -> portal 3 (upper terrace)
    { 3, { { 3447.643f, -4800.971f, 189.731f }, { 3465.000f, -4820.000f, 192.000f }, { 3471.120f, -4839.830f, 194.215f } } },
    // portal 3 -> Peroth'arn assist spot at the courtyard edge
    { 3, { { 3452.155f, -4860.000f, 187.000f }, { 3400.000f, -4900.000f, 181.300f }, { 3359.602f, -4939.480f, 181.162f } } }
};

struct npc_woe_illidan_gauntlet : public ScriptedAI
{
    npc_woe_illidan_gauntlet(Creature* creature) : ScriptedAI(creature), _instance(nullptr),
        _stage(0), _pathNode(0), _escorting(false), _introDone(false) { }

    void InitializeAI() override
    {
        _instance = me->GetInstanceScript();
    }

    void JustAppeared() override
    {
        me->SetImmuneToPC(true);
        me->SetImmuneToNPC(true);
        me->CastSpell(me, SPELL_SHADOWCLOAK_ILLIDAN, true);

        if (!_instance)
            return;

        _stage = std::min<uint8>(_instance->GetData(DATA_PORTALS_SHUT_DOWN), MaxEscortStages - 1);
        if (_stage == 0 && !_introDone)
        {
            _introDone = true;
            Talk(SAY_INTRO_1);
            _events.ScheduleEvent(EVENT_INTRO_2, 4s);
        }
        _events.ScheduleEvent(EVENT_CLOAK_PULSE, 2s);
    }

    bool GossipHello(Player* player) override
    {
        // Escort not started yet: offer the shadowcloak. Afterwards: flavor only.
        if (!_escorting && _stage == 0)
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I am ready to be hidden by your shadowcloak.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
        SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());
        return true;
    }

    bool GossipSelect(Player* player, uint32 /*menuId*/, uint32 /*gossipListId*/) override
    {
        CloseGossipMenuFor(player);
        if (_escorting || _stage != 0)
            return true;

        StartEscort();
        return true;
    }

    void DoAction(int32 action) override
    {
        switch (action)
        {
            case ACTION_ADVANCE_STAGE:
            {
                uint8 portals = uint8(std::min<uint32>(_instance ? _instance->GetData(DATA_PORTALS_SHUT_DOWN) : 0, MaxEscortStages - 1));
                if (portals <= _stage && _escorting)
                    break;
                _stage = portals;
                if (_stage >= 3)
                {
                    Talk(SAY_NO_LONGER_POURING);
                    _events.ScheduleEvent(EVENT_LEG_FLAVOR, 8s);
                }
                else
                    Talk(SAY_LEAVING);
                StartLeg();
                break;
            }
            case ACTION_ILLIDAN_DRAIN_ESSENCE:
                _events.ScheduleEvent(EVENT_DRAIN_FEEDBACK, 1s);
                _events.ScheduleEvent(EVENT_DRAIN_ABSORB, 5s);
                _events.ScheduleEvent(EVENT_DRAIN_SHADOW_BOLT, 10s + 500ms);
                _events.ScheduleEvent(EVENT_DRAIN_REGENERATE, 15s);
                _events.ScheduleEvent(EVENT_DRAIN_RECLOAK, 16s);
                break;
            case ACTION_ILLIDAN_HIDE_ENDED:
                Talk(SAY_STRENGTH_RETURNS);
                break;
            case ACTION_ILLIDAN_PEROTHARN_DEAD:
                _events.ScheduleEvent(EVENT_OUTRO_HUNTER_PREY, 4s);
                _events.ScheduleEvent(EVENT_OUTRO_FAREWELL, 2min);
                break;
            default:
                break;
        }
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != POINT_MOTION_TYPE || pointId < POINT_ESCORT_BASE)
            return;

        EscortLeg const& leg = EscortLegs[_stage];
        uint8 node = uint8(pointId - POINT_ESCORT_BASE);

        // Stage-0 flavor at the Guardian Demon choke point
        if (_stage == 0 && node == 1)
        {
            Talk(SAY_GUARDIAN_WARNING);
            _events.ScheduleEvent(EVENT_LEG_FLAVOR, 4s);
        }

        if (node + 1 < leg.NodeCount)
        {
            _pathNode = node + 1;
            me->GetMotionMaster()->MovePoint(POINT_ESCORT_BASE + _pathNode, leg.Nodes[_pathNode]);
        }
        else
            ArriveAtHold();
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_INTRO_2:
                    Talk(SAY_INTRO_2);
                    break;
                case EVENT_CLOAK_PULSE:
                    CloakNearbyPlayers();
                    _events.Repeat(2s);
                    break;
                case EVENT_LEG_FLAVOR:
                    switch (_stage)
                    {
                        case 0:
                            Talk(SAY_WALL_OF_SHADOW);
                            DoCastSelf(SPELL_WALL_OF_SHADOW);
                            for (uint8 i = 0; i < 6; ++i)
                            {
                                Position dest = me->GetPosition();
                                me->MovePosition(dest, 15.0f + i * 3.0f, frand(-0.6f, 0.6f));
                                me->CastSpell(dest, SPELL_DISTRACT_DEMON_MISSILE, true);
                            }
                            Talk(SAY_MAGIC_FADING);
                            break;
                        case 1:
                            Talk(SAY_ENDLESS);
                            break;
                        case 2:
                            Talk(SAY_STENCH);
                            break;
                        case 3:
                            Talk(SAY_DEMONS_LEAVING);
                            break;
                        default:
                            break;
                    }
                    break;
                case EVENT_HOLD_ATTACK:
                    Talk(SAY_ATTACK);
                    break;
                case EVENT_HOLD_FIRST_KILL:
                    Talk(SAY_FIRST_KILL);
                    break;
                case EVENT_HOLD_DESTROY_FOCUS:
                    Talk(_stage == 2 ? SAY_DESTROY_CRYSTAL_3 : (_stage == 1 ? SAY_SMASH_CRYSTAL : SAY_DESTROY_FOCUS));
                    break;
                case EVENT_DRAIN_FEEDBACK:
                    DoCastSelf(SPELL_DRAIN_ESSENCE_FEEDBACK, true);
                    if (_drainFeedbackCount++ < 3)
                        _events.Repeat(1s);
                    else
                        _drainFeedbackCount = 0;
                    break;
                case EVENT_DRAIN_ABSORB:
                    Talk(SAY_DRAIN_COUNTER);
                    if (Creature* perotharn = _instance->GetCreature(BOSS_PEROTHARN))
                        DoCast(perotharn, SPELL_ABSORB_FEL_ENERGY);
                    break;
                case EVENT_DRAIN_SHADOW_BOLT:
                    if (Creature* perotharn = _instance->GetCreature(BOSS_PEROTHARN))
                        DoCast(perotharn, SPELL_SHADOW_BOLT, true);
                    if (_shadowBoltCount++ < 4)
                        _events.Repeat(1s);
                    else
                        _shadowBoltCount = 0;
                    break;
                case EVENT_DRAIN_REGENERATE:
                    DoCastSelf(SPELL_REGENERATION, true);
                    break;
                case EVENT_DRAIN_RECLOAK:
                    Talk(SAY_RETURN_SHADOWS);
                    DoCastSelf(SPELL_RETURN_TO_THE_SHADOWS, true);
                    CloakNearbyPlayers(true);
                    break;
                case EVENT_OUTRO_HUNTER_PREY:
                    Talk(SAY_HUNTER_PREY);
                    break;
                case EVENT_OUTRO_FAREWELL:
                    Talk(SAY_FAREWELL);
                    _events.ScheduleEvent(EVENT_OUTRO_DEPART, 6s);
                    break;
                case EVENT_OUTRO_DEPART:
                    me->GetMotionMaster()->MovePoint(0, 3392.934f, -4981.066f, 196.782f);
                    me->DespawnOrUnsummon(12s);
                    break;
                default:
                    break;
            }
        }
    }

private:
    void StartEscort()
    {
        _escorting = true;
        Talk(SAY_CLOAK);
        CloakNearbyPlayers(true);
        Talk(SAY_ESCORT_START);
        StartLeg();
    }

    void StartLeg()
    {
        _escorting = true;
        _pathNode = 0;
        me->GetMotionMaster()->Clear();
        me->GetMotionMaster()->MovePoint(POINT_ESCORT_BASE, EscortLegs[_stage].Nodes[0]);
    }

    void ArriveAtHold()
    {
        me->SetFacingTo(IllidanHoldOrientation(_stage));
        if (_stage < 3)
        {
            _events.ScheduleEvent(EVENT_HOLD_ATTACK, 3s);
            _events.ScheduleEvent(EVENT_HOLD_FIRST_KILL, 10s);
            _events.ScheduleEvent(EVENT_HOLD_DESTROY_FOCUS, 18s);
            if (_stage == 2)
                Talk(SAY_FINAL_PORTAL);
        }
        // Stage 3: waiting at the courtyard edge for Peroth'arn; nothing to do.
    }

    static float IllidanHoldOrientation(uint8 stage)
    {
        switch (stage)
        {
            case 0: return 0.8727f;
            case 1: return 4.0143f;
            case 2: return 2.0071f;
            default: return 3.0935f;
        }
    }

    void CloakNearbyPlayers(bool includeCombat = false)
    {
        Map::PlayerList const& players = me->GetMap()->GetPlayers();
        for (auto const& ref : players)
        {
            Player* player = ref.GetSource();
            if (!player || !player->IsAlive() || player->IsGameMaster())
                continue;
            if (!includeCombat && player->IsInCombat())
                continue;
            if (!me->IsWithinDistInMap(player, 50.0f))
                continue;
            if (!player->HasAura(SPELL_SHADOW_WALK))
            {
                player->CastSpell(player, SPELL_SHADOW_WALK, true);
                player->CastSpell(player, SPELL_SHADOW_AMBUSHER_STEALTH, true);
                player->CastSpell(player, SPELL_SHADOW_AMBUSHER, true);
            }
        }
    }

    InstanceScript* _instance;
    EventMap _events;
    uint8 _stage;
    uint8 _pathNode;
    uint8 _drainFeedbackCount = 0;
    uint8 _shadowBoltCount = 0;
    bool _escorting;
    bool _introDone;
};

} // namespace IllidanGauntlet

/*######
## npc_woe_legion_demon_door_guard - the opening pull (55503)
######*/

enum WOEQuestCredits
{
    NPC_CREDIT_PORTAL_1         = 58239, // In Unending Numbers, one per crystal
    NPC_CREDIT_LEGION_DEMON     = 57856  // Documenting the Timeways
};

void GiveCreditToGroup(Unit* killer, uint32 creditEntry)
{
    if (!killer)
        return;
    if (Player* player = killer->GetCharmerOrOwnerPlayerOrPlayerItself())
        player->KilledMonsterCredit(creditEntry); // propagates to the group
}

struct npc_woe_legion_demon_door_guard : public ScriptedAI
{
    npc_woe_legion_demon_door_guard(Creature* creature) : ScriptedAI(creature) { }

    void JustDied(Unit* killer) override
    {
        GiveCreditToGroup(killer, NPC_CREDIT_LEGION_DEMON);
        if (InstanceScript* instance = me->GetInstanceScript())
            // Only the courtyard-door guard (western spawn) opens the gauntlet.
            if (me->GetHomePosition().GetPositionX() < 3300.0f)
                instance->SetData(DATA_LEGION_DEMON_FIRST_PULL, 1);
    }
};

/*######
## npc_woe_legion_demon_marching - endless palace reinforcements (54500)
######*/

namespace MarchingDemon
{
Position const MarchPath[] =
{
    { 3444.490f, -5069.680f, 213.880f },
    { 3425.490f, -5037.930f, 197.380f },
    { 3405.740f, -5006.430f, 197.130f },
    { 3388.240f, -4978.430f, 197.380f },
    { 3375.490f, -4957.680f, 181.630f },
    { 3337.686f, -4900.463f, 181.077f }
};

Position const PatrolLegEast[] =
{
    { 3340.490f, -4883.618f, 181.080f },
    { 3410.395f, -4839.722f, 181.077f }
};

Position const PatrolLegWest[] =
{
    { 3326.771f, -4884.219f, 181.080f },
    { 3283.181f, -4816.979f, 181.412f }
};

enum Points
{
    POINT_MARCH_END     = 1,
    POINT_PATROL_OUT    = 2,
    POINT_PATROL_BACK   = 3
};
}

struct npc_woe_legion_demon_marching : public ScriptedAI
{
    npc_woe_legion_demon_marching(Creature* creature) : ScriptedAI(creature), _east(false) { }

    void JustDied(Unit* killer) override
    {
        GiveCreditToGroup(killer, NPC_CREDIT_LEGION_DEMON);
    }

    void JustAppeared() override
    {
        using namespace MarchingDemon;
        _east = roll_chance_i(50);
        me->GetMotionMaster()->MoveSmoothPath(POINT_MARCH_END, MarchPath, std::extent<decltype(MarchPath)>::value, true);
        _scheduler.Schedule(5s, [this](TaskContext ctx)
        {
            if (InstanceScript* instance = me->GetInstanceScript())
                if (instance->GetData(DATA_PORTALS_SHUT_DOWN) >= 3 || instance->GetBossState(BOSS_PEROTHARN) == DONE)
                {
                    me->DespawnOrUnsummon(1s);
                    return;
                }
            ctx.Repeat(5s);
        });
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        using namespace MarchingDemon;
        if (type != POINT_MOTION_TYPE && type != EFFECT_MOTION_TYPE)
            return;

        switch (pointId)
        {
            case POINT_MARCH_END:
            case POINT_PATROL_BACK:
                me->GetMotionMaster()->MovePoint(POINT_PATROL_OUT, _east ? PatrolLegEast[1] : PatrolLegWest[1]);
                break;
            case POINT_PATROL_OUT:
                me->GetMotionMaster()->MovePoint(POINT_PATROL_BACK, MarchPath[5]);
                break;
            default:
                break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
        if (!UpdateVictim())
            return;
        DoMeleeAttackIfReady();
    }

private:
    TaskScheduler _scheduler;
    bool _east;
};

/*######
## go_woe_portal_energy_focus - the three gauntlet crystals
######*/

enum PortalFocusMisc
{
    SPELL_FEL_CRYSTAL_DESTRUCTION   = 105079,
    SPELL_PORTAL_SHUTTING_DOWN      = 102457
};

struct go_woe_portal_energy_focus : public GameObjectAI
{
    go_woe_portal_energy_focus(GameObject* gameObject) : GameObjectAI(gameObject), _instance(nullptr), _used(false) { }

    void InitializeAI() override
    {
        _instance = me->GetInstanceScript();
    }

    bool GossipHello(Player* /*player*/) override
    {
        if (!_instance || _used)
            return false;
        if (!_instance->GetData(DATA_LEGION_DEMON_FIRST_PULL) || _instance->GetData(DATA_PORTALS_SHUT_DOWN) >= 3)
            return false;

        _used = true;
        me->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
        me->SendCustomAnim(0);

        // Shatter visual from the crystal stalker + shut down the feeding portal
        if (Creature* stalker = me->FindNearestCreature(NPC_FEL_CRYSTAL_STALKER, 15.0f))
            stalker->CastSpell(stalker, SPELL_FEL_CRYSTAL_DESTRUCTION, true);
        if (Creature* portal = me->FindNearestCreature(NPC_LEGION_PORTAL, 60.0f))
            portal->CastSpell(portal, SPELL_PORTAL_SHUTTING_DOWN, true);

        _instance->SetData(DATA_PORTALS_SHUT_DOWN, 1);
        if (Creature* illidan = _instance->GetCreature(DATA_GAUNTLET_ILLIDAN))
            if (illidan->IsAIEnabled())
                illidan->AI()->DoAction(IllidanGauntlet::ACTION_ADVANCE_STAGE);

        // In Unending Numbers: one credit dummy per disabled portal
        uint32 credit = NPC_CREDIT_PORTAL_1 + std::min<uint32>(_instance->GetData(DATA_PORTALS_SHUT_DOWN), 3) - 1;
        Map::PlayerList const& players = me->GetMap()->GetPlayers();
        for (auto const& ref : players)
            if (Player* mapPlayer = ref.GetSource())
                mapPlayer->KilledMonsterCredit(credit);

        return true;
    }

private:
    InstanceScript* _instance;
    bool _used;
};

/*######
## npc_woe_bronze_drake - flight from Azshara's terrace to the Shores of the Well (57107)
######*/

namespace BronzeDrake
{
enum Texts
{
    SAY_HEROES = 0 // Heroes! We have been sent by Nozdormu! Quickly, on our backs...
};

enum Points
{
    POINT_LANDING   = 1,
    POINT_DIVE      = 2
};

Position const FlightPath[] =
{
    { 3427.594f, -5425.372f, 208.055f },
    { 3274.889f, -5499.763f, 80.990f  },
    { 3114.960f, -5564.842f, 20.377f  }
};

Position const DivePath[] =
{
    { 3232.311f, -5517.089f, -33.177f }
};
}

struct npc_woe_bronze_drake : public ScriptedAI
{
    npc_woe_bronze_drake(Creature* creature) : ScriptedAI(creature), _flying(false) { }

    void JustAppeared() override
    {
        me->SetDisableGravity(true);
    }

    void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
    {
        using namespace BronzeDrake;
        if (!apply || !passenger->IsPlayer() || _flying)
            return;

        _flying = true;
        Talk(SAY_HEROES);
        _scheduler.Schedule(2s, [this](TaskContext)
        {
            using namespace BronzeDrake;
            me->GetMotionMaster()->MoveSmoothPath(POINT_LANDING, FlightPath, std::extent<decltype(FlightPath)>::value, false, true);
        });
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        using namespace BronzeDrake;
        if (type != EFFECT_MOTION_TYPE && type != POINT_MOTION_TYPE)
            return;

        if (pointId == POINT_LANDING)
        {
            if (Vehicle* vehicle = me->GetVehicleKit())
                vehicle->RemoveAllPassengers();
            _scheduler.Schedule(1s, [this](TaskContext)
            {
                using namespace BronzeDrake;
                me->GetMotionMaster()->MoveSmoothPath(POINT_DIVE, DivePath, std::extent<decltype(DivePath)>::value, false, true);
            });
        }
        else if (pointId == POINT_DIVE)
            me->DespawnOrUnsummon();
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

private:
    TaskScheduler _scheduler;
    bool _flying;
};

/*######
## npc_woe_legion_army_doomguard - ambient army streaming out of the Well (55700)
######*/

struct npc_woe_legion_army_doomguard : public PassiveAI
{
    npc_woe_legion_army_doomguard(Creature* creature) : PassiveAI(creature) { }

    void JustAppeared() override
    {
        me->SetDisableGravity(true);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_NOT_SELECTABLE);

        float angle = frand(0.0f, 2.0f * float(M_PI));
        Position const& origin = me->GetHomePosition();
        Position path[3];
        for (uint8 i = 0; i < 3; ++i)
        {
            float radius = 130.0f + 100.0f * i;
            path[i] = { origin.GetPositionX() + radius * std::cos(angle),
                        origin.GetPositionY() + radius * std::sin(angle),
                        i == 0 ? 59.3f : (i == 1 ? 100.5f : 126.4f), 0.0f };
        }
        me->GetMotionMaster()->MoveSmoothPath(1, path, 3, false, true);
        me->DespawnOrUnsummon(45s);
    }
};

/*######
## go_woe_time_transit_device - checkpoint teleporter network
######*/

namespace TransitDevice
{
enum TransitSpells
{
    SPELL_TELEPORT_COURTYARD_ENTRANCE   = 107934,
    SPELL_TELEPORT_AZSHARAS_PALACE      = 107690,
    SPELL_TELEPORT_AZSHARAS_OVERLOOK    = 107979,
    SPELL_TELEPORT_WELL_OF_ETERNITY     = 107691
};

enum TransitGossip
{
    GOSSIP_MENU_ID_TRANSIT = 13326
};

struct TransitDestination
{
    uint32 GossipIndex;
    uint32 SpellId;
    Position Pos; // used to hide the option when already there
};

TransitDestination const Destinations[] =
{
    { 0, SPELL_TELEPORT_COURTYARD_ENTRANCE, { 3221.692f, -5003.707f, 194.093f } },
    { 1, SPELL_TELEPORT_AZSHARAS_PALACE,    { 3495.437f, -5009.148f, 197.617f } },
    { 2, SPELL_TELEPORT_AZSHARAS_OVERLOOK,  { 3492.210f, -5200.014f, 229.949f } },
    { 3, SPELL_TELEPORT_WELL_OF_ETERNITY,   { 3056.847f, -5564.731f, 18.125f  } }
};

bool IsDestinationUnlocked(InstanceScript const* instance, uint32 gossipIndex)
{
    switch (gossipIndex)
    {
        case 0:
            return true;
        case 1:
        case 2:
            return instance->GetBossState(BOSS_PEROTHARN) == DONE;
        case 3:
            return instance->GetBossState(BOSS_QUEEN_AZSHARA) == DONE;
        default:
            return false;
    }
}
}

struct go_woe_time_transit_device : public GameObjectAI
{
    go_woe_time_transit_device(GameObject* gameObject) : GameObjectAI(gameObject), _instance(nullptr) { }

    void InitializeAI() override
    {
        _instance = me->GetInstanceScript();
    }

    bool GossipHello(Player* player) override
    {
        using namespace TransitDevice;
        if (!_instance)
            return false;

        for (TransitDestination const& dest : Destinations)
        {
            if (player->GetExactDist2d(dest.Pos.GetPositionX(), dest.Pos.GetPositionY()) < 50.0f)
                continue;
            if (!IsDestinationUnlocked(_instance, dest.GossipIndex))
                continue;
            AddGossipItemFor(player, GOSSIP_MENU_ID_TRANSIT, dest.GossipIndex, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + dest.GossipIndex);
        }

        SendGossipMenuFor(player, player->GetGossipTextId(GOSSIP_MENU_ID_TRANSIT, me), me->GetGUID());
        return true;
    }

    bool GossipSelect(Player* player, uint32 gossipMenuId, uint32 gossipListId) override
    {
        using namespace TransitDevice;
        if (!_instance || gossipMenuId != GOSSIP_MENU_ID_TRANSIT)
            return true;

        uint32 gossipAction = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
        ClearGossipMenuFor(player);

        if (player->IsInCombat() || gossipAction < GOSSIP_ACTION_INFO_DEF)
            return true;

        uint32 index = gossipAction - GOSSIP_ACTION_INFO_DEF;
        for (TransitDestination const& dest : Destinations)
            if (dest.GossipIndex == index && IsDestinationUnlocked(_instance, index))
                player->CastSpell(player, dest.SpellId, true);

        return true;
    }

private:
    InstanceScript* _instance;
};
}

void AddSC_well_of_eternity()
{
    using namespace WellOfEternity;
    using IllidanGauntlet::npc_woe_illidan_gauntlet;
    RegisterWellOfEternityCreatureAI(npc_woe_illidan_gauntlet);
    RegisterWellOfEternityCreatureAI(npc_woe_legion_demon_door_guard);
    RegisterWellOfEternityCreatureAI(npc_woe_legion_demon_marching);
    RegisterWellOfEternityCreatureAI(npc_woe_bronze_drake);
    RegisterWellOfEternityCreatureAI(npc_woe_legion_army_doomguard);
    RegisterGameObjectAI(go_woe_portal_energy_focus);
    RegisterGameObjectAI(go_woe_time_transit_device);
}
