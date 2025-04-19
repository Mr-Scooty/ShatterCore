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

#include "gilneas.h"
#include "ScriptMgr.h"
#include "CombatAI.h"
#include "MotionMaster.h"
#include "MoveSplineInit.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "PassiveAI.h"
#include "PhasingHandler.h"
#include "ScriptedCreature.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "TemporarySummon.h"
#include "Vehicle.h"
#include "GameObjectAI.h"
#include "GameObject.h"

namespace Gilneas::Chapter2
{
enum GilneasInvasionCamera
{
    CINEMATIC_FORSAKEN_INVASION = 168
};

struct go_gilneas_invasion_camera : public GameObjectAI
{
    go_gilneas_invasion_camera(GameObject* go) : GameObjectAI(go) { }

    bool OnReportUse(Player* player) override
    {
        player->SendCinematicStart(CINEMATIC_FORSAKEN_INVASION);
        return true;
    }
};

enum HorridAbomination
{
    // Horrid Abomination
    SPELL_KEG_PLACED                    = 68555,
    SPELL_ABOMINATION_KILL_ME           = 68558,
    SPELL_RANDOM_CIRCUMFERENCE_POISON   = 42266,
    SPELL_RANDOM_CIRCUMFERENCE_BONE     = 42267,
    SPELL_RANDOM_CIRCUMFERENCE_BONE_2   = 42274,
    SPELL_HORRID_ABOMINATION_EXPLOSION  = 68560,
    SPELL_RESTITCHING                   = 68864,

    QUEST_HORRID_ABOMINATION_CREDIT     = 36233,

    SAY_KEG_PLACED                      = 0,
    EVENT_ABOMINATION_KILL_ME           = 1,

    // Prince Liam Greymane
    SPELL_SHOOT                         = 68559
};

struct npc_gilneas_horrid_abomination : public ScriptedAI
{
    npc_gilneas_horrid_abomination(Creature* creature) : ScriptedAI(creature)
    {
        Initialize();
    }

    void Initialize()
    {
        _playerGUID = ObjectGuid::Empty;
        _allowEvents = false;
    }

    void Reset() override
    {
        Initialize();
        me->GetMotionMaster()->MoveRandom(6.0f);
    }

    void SpellHit(WorldObject* caster, SpellInfo const* spell) override
    {
        switch (spell->Id)
        {
            case SPELL_KEG_PLACED:
                Talk(SAY_KEG_PLACED);
                me->AttackStop();
                me->SetReactState(REACT_PASSIVE);
                me->GetMotionMaster()->InitDefault();
                me->StopMoving();
                _playerGUID = caster->GetGUID();
                _allowEvents = true;
                _events.ScheduleEvent(EVENT_ABOMINATION_KILL_ME, Seconds(2));
                break;
            case SPELL_SHOOT:
                if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID))
                    player->KilledMonsterCredit(QUEST_HORRID_ABOMINATION_CREDIT);

                me->RemoveAurasDueToSpell(SPELL_KEG_PLACED);

                for (uint8 i = 0; i < 11; i++)
                    DoCastSelf(SPELL_RANDOM_CIRCUMFERENCE_POISON, true);

                for (uint8 i = 0; i < 6; i++)
                    DoCastSelf(SPELL_RANDOM_CIRCUMFERENCE_BONE, true);

                for (uint8 i = 0; i < 4; i++)
                    DoCastSelf(SPELL_RANDOM_CIRCUMFERENCE_BONE_2, true);

                DoCastSelf(SPELL_HORRID_ABOMINATION_EXPLOSION, true);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->DespawnOrUnsummon(Seconds(5));
                break;
            default:
                break;
        }
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage) override
    {
        if (damage >= me->GetHealth())
        {
            damage = 0;
            if (!me->HasUnitState(UNIT_STATE_CASTING) && !me->HasAura(SPELL_RESTITCHING))
                DoCastSelf(SPELL_RESTITCHING);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim() && !_allowEvents)
            return;

        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_ABOMINATION_KILL_ME:
                    DoCastAOE(SPELL_ABOMINATION_KILL_ME, true);
                    break;
                default:
                    break;
            }
        }
        DoMeleeAttackIfReady();
    }

private:
    EventMap _events;
    ObjectGuid _playerGUID;
    bool _allowEvents;
};

enum SaveTheChildren
{
    SPELL_GILNEAS_QUEST_SAVE_JAMES      = 68596,
    SPELL_GILNEAS_QUEST_SAVE_CYNTHIA    = 68597,
    SPELL_GILNEAS_QUEST_SAVE_ASHLEY     = 68598,

    NPC_CYNTHIA                         = 36287,
    NPC_ASHLEY                          = 36288,
    NPC_JAMES                           = 36289,

    SAY_CHILD_RESCUED                   = 0,
    EVENT_TALK_RESCUED                  = 1,
    EVENT_RUN_TO_BASEMENT               = 2,
    EVENT_CRY                           = 3,

    POINT_BASEMENT_1                    = 1,
    POINT_BASEMENT_2                    = 2,
};

Position const JamesEscapePos = { -1913.021f, 2558.333f, 1.511007f };

Position const AshleyEscapePos[] =
{
    { -1923.283f, 2552.308f, 12.73581f }, // Ashley Point 1
    { -1920.023f, 2558.055f, 7.076692f }  // Ashley Point 2
};

Position const CynthiaEscapePos[] =
{
    { -1969.23f,  2517.465f, 2.580818f }, // Cynthia Point 1
    { -1947.472f, 2515.521f, 2.318746f }, // Cynthia Point 2
    { -1926.536f, 2519.312f, 2.246772f }  // Cynthia Point 3
};

class spell_gilneas_quest_save_the_children : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_GILNEAS_QUEST_SAVE_JAMES });
    }

    void HandleDummy(SpellEffIndex effIndex)
    {
        if (Unit* caster = GetCaster())
        {
            if (Player* player = caster->ToPlayer())
            {
                Unit* target = GetHitUnit();
                player->Talk(GetSpellInfo()->Effects[effIndex].BasePoints, CHAT_MSG_SAY, 0.0f, target);
                player->KilledMonsterCredit(target->GetEntry());
            }
        }
    }

    void Register() override
    {
        if (m_scriptSpellId == SPELL_GILNEAS_QUEST_SAVE_JAMES)
            OnEffectHitTarget.Register(&spell_gilneas_quest_save_the_children::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        else
            OnEffectHitTarget.Register(&spell_gilneas_quest_save_the_children::HandleDummy, EFFECT_1, SPELL_EFFECT_DUMMY);
    }
};

struct npc_gilneas_save_the_children : public ScriptedAI
{
    npc_gilneas_save_the_children(Creature* creature) : ScriptedAI(creature)
    {
        Initialize();
    }

    void Initialize()
    {
        _playerGUID = ObjectGuid::Empty;
    }

    void Reset() override
    {
        Initialize();
        if (me->GetEntry() == NPC_CYNTHIA)
            _events.ScheduleEvent(EVENT_CRY, Seconds(1));
    }

    void SpellHit(WorldObject* caster, SpellInfo const* spell) override
    {
        switch (spell->Id)
        {
            case SPELL_GILNEAS_QUEST_SAVE_JAMES:
            case SPELL_GILNEAS_QUEST_SAVE_CYNTHIA:
            case SPELL_GILNEAS_QUEST_SAVE_ASHLEY:
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                _playerGUID = caster->GetGUID();
                _events.ScheduleEvent(EVENT_TALK_RESCUED, Seconds(2) + Milliseconds(500));
                _events.CancelEvent(EVENT_CRY);
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
            case POINT_BASEMENT_1:
                if (me->GetEntry() == NPC_ASHLEY)
                    me->GetMotionMaster()->MovePoint(0, AshleyEscapePos[1], true);
                else
                    me->GetMotionMaster()->MovePoint(POINT_BASEMENT_2, CynthiaEscapePos[1], true);
                break;
            case POINT_BASEMENT_2:
                me->GetMotionMaster()->MovePoint(0, CynthiaEscapePos[2], true);
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
                case EVENT_TALK_RESCUED:
                    Talk(SAY_CHILD_RESCUED, ObjectAccessor::GetPlayer(*me, _playerGUID));
                    _events.ScheduleEvent(EVENT_RUN_TO_BASEMENT, me->GetEntry() == NPC_JAMES ? Seconds(3) + Milliseconds(600) : Seconds(2) + Milliseconds(300));
                    break;
                case EVENT_RUN_TO_BASEMENT:
                    switch (me->GetEntry())
                    {
                        case NPC_JAMES:
                            me->GetMotionMaster()->MovePoint(0, JamesEscapePos, true);
                            me->DespawnOrUnsummon(Seconds(5) + Milliseconds(200));
                            break;
                        case NPC_ASHLEY:
                            me->GetMotionMaster()->MovePoint(POINT_BASEMENT_1, AshleyEscapePos[0], true);
                            me->DespawnOrUnsummon(Seconds(3) + Milliseconds(800));
                            break;
                        case NPC_CYNTHIA:
                            me->GetMotionMaster()->MovePoint(POINT_BASEMENT_1, CynthiaEscapePos[0], true);
                            me->DespawnOrUnsummon(Seconds(8) + Milliseconds(500));
                            break;
                        default:
                            break;
                    }
                    break;
                case EVENT_CRY:
                    me->HandleEmoteCommand(EMOTE_ONESHOT_CRY);
                    _events.Repeat(Seconds(1), Seconds(1) + Milliseconds(500));
                    break;
                default:
                    break;
            }
        }
    }

private:
    EventMap _events;
    ObjectGuid _playerGUID;
};

enum ForsakenCatapult
{
    NPC_FORSAKEN_MACHINIST  = 36292,

    SPELL_FIERY_BOULDER     = 68591,
    SPELL_LAUNCH_INTERNAL   = 96114,
    SPELL_LAUNCH_INTERNAL_2 = 96185,
    SPELL_LAUNCH            = 66251,

    EVENT_FIERY_BOULDER     = 1,
    EVENT_CHECK_AREA        = 2,

    SEAT_0                  = 0,

    SAY_WARN_OUT_OF_AREA    = 0,

    AREA_ID_DUSKMIST_SHORE  = 5720
};

struct npc_gilneas_forsaken_catapult : public VehicleAI
{
    npc_gilneas_forsaken_catapult(Creature* creature) : VehicleAI(creature)
    {
        Initialize();
    }

    void Initialize()
    {
        _preparedDespawn = false;
    }

    void Reset() override
    {
        Initialize();
        _events.ScheduleEvent(EVENT_FIERY_BOULDER, Milliseconds(1), Seconds(7));
    }

    void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
    {
        if (!passenger)
            return;

        if (passenger->GetEntry() == NPC_FORSAKEN_MACHINIST)
        {
            if (apply)
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                passenger->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                if (Creature* creature = passenger->ToCreature())
                    creature->SetReactState(REACT_PASSIVE);
            }
            else
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->SetFaction(FACTION_FRIENDLY);
                _events.CancelEvent(EVENT_FIERY_BOULDER);
            }
        }
        else if (passenger->GetTypeId() == TYPEID_PLAYER && !apply)
            me->DespawnOrUnsummon(Seconds(9));
        else if (passenger->GetTypeId() == TYPEID_PLAYER && apply)
            _events.ScheduleEvent(EVENT_CHECK_AREA, Milliseconds(1));
    }

    void SpellHit(WorldObject* /*caster*/, SpellInfo const* spell) override
    {
        switch (spell->Id)
        {
            case SPELL_LAUNCH_INTERNAL:
                DoCastSelf(SPELL_LAUNCH_INTERNAL_2, true);
                break;
            default:
                break;
        }
    }

    void SetTargetDestination(Position pos)
    {
        _targetPos = pos;
    }

    void SpellHitTarget(WorldObject* target, SpellInfo const* spell) override
    {
        Unit* unitTarget = target->ToUnit();
        if (!unitTarget)
            return;

        switch (spell->Id)
        {
            case SPELL_LAUNCH:
                if (unitTarget->GetVehicleCreatureBase())
                {
                    Position pos = unitTarget->GetPosition();
                    pos.m_positionZ += 6.0f;
                    unitTarget->ExitVehicle(&pos);
                    unitTarget->GetMotionMaster()->MoveJump(_targetPos, 58.62504f, 12.75955f);
                }
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
                case EVENT_FIERY_BOULDER:
                    DoCastAOE(SPELL_FIERY_BOULDER);
                    _events.Repeat(Seconds(7), Seconds(8));
                    break;
                case EVENT_CHECK_AREA:
                    if (me->GetAreaId() != AREA_ID_DUSKMIST_SHORE)
                    {
                        if (!_preparedDespawn)
                        {
                            if (Vehicle* vehicle = me->GetVehicleKit())
                                if (Unit* passenger = vehicle->GetPassenger(SEAT_0))
                                    Talk(SAY_WARN_OUT_OF_AREA, passenger);

                            _preparedDespawn = true;
                        }
                        else
                            me->DespawnOrUnsummon();

                        _events.Repeat(Seconds(10));
                    }
                    else
                    {
                        if (_preparedDespawn)
                        {
                            _preparedDespawn = false;
                            _events.Repeat(Seconds(2));
                        }
                        else
                            _events.Repeat(Seconds(2));
                    }
                    break;
                default:
                    break;
            }
        }

    }
private:
    EventMap _events;
    Position _targetPos;
    bool _preparedDespawn;
};

class spell_gilneas_launch : public SpellScript
{
    void TransferDestination(SpellEffIndex /*effIndex*/)
    {
        if (Unit* caster = GetCaster())
            if (Creature* creature = caster->ToCreature())
                if (creature->IsAIEnabled())
                    CAST_AI(npc_gilneas_forsaken_catapult, creature->AI())->SetTargetDestination(GetExplTargetDest()->GetPosition());
    }

    void Register()
    {
        OnEffectLaunch.Register(&spell_gilneas_launch::TransferDestination, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

class FireBoulderInFrontCheck
{
    public:
        FireBoulderInFrontCheck(Unit* _caster) : caster(_caster) { }

        bool operator()(WorldObject* object)
        {
            if (Unit* target = object->ToUnit())
                return (!caster->isInFront(target, float(M_PI * 0.3f)));

            return false;
        }
    private:
        Unit* caster;

};

class spell_gilneas_fiery_boulder : public SpellScript
{
    void FilterTargets(std::list<WorldObject*>& targets)
    {
        if (targets.empty())
            return;

        targets.remove_if(FireBoulderInFrontCheck(GetCaster()));

        if (targets.empty())
            return;

        Trinity::Containers::RandomResize(targets, 1);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect.Register(&spell_gilneas_fiery_boulder::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
    }
};

enum LeaderOfThePack
{
    NPC_ATTACK_MASTIFF = 36405
};

Position const AttackMastiffSummonPositions[] =
{
    { -1944.483f, 2656.656f, 1.051441f,  1.691914f  },
    { -1956.602f, 2649.942f, 1.374257f,  1.441419f  },
    { -1973.627f, 2654.836f, -0.6995407f, 1.098437f },
    { -1983.201f, 2662.242f, -1.66652f,  0.8627869f },
    { -1994.557f, 2672.134f, -2.303949f, 0.5766099f },
    { -1949.314f, 2642.024f, 1.299083f,  1.580745f  },
    { -1972.606f, 2639.383f, 1.211673f,  1.217789f  },
    { -1997.009f, 2650.811f, -1.030188f, 0.8184887f },
    { -2006.259f, 2663.115f, -2.00431f,  0.5941383f },
    { -1945.504f, 2653.386f, 1.177739f,  1.675516f  }
};

class spell_gilneas_call_attack_mastiff : public SpellScript
{
    void HandleHit(SpellEffIndex /*effIndex*/)
    {
        if (Unit* caster = GetCaster())
        {
            for (uint8 i = 0; i < 10; i++)
                if (Creature* mastiff = caster->SummonCreature(NPC_ATTACK_MASTIFF, AttackMastiffSummonPositions[i], TEMPSUMMON_TIMED_DESPAWN, 60000))
                    mastiff->AI()->AttackStart(GetHitUnit());
        }
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_gilneas_call_attack_mastiff::HandleHit, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

class spell_gilneas_forcecast_cataclysm_1 : public SpellScript
{
    void HandleForcecast(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);
        if (Unit* caster = GetCaster())
            caster->CastSpell(caster, GetSpellInfo()->Effects[effIndex].TriggerSpell, true);
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_gilneas_forcecast_cataclysm_1::HandleForcecast, EFFECT_0, SPELL_EFFECT_FORCE_CAST);
    }
};

class SummonerTargetSelector
{
public:
    SummonerTargetSelector(Unit* caster) : _caster(caster) { }

    bool operator() (WorldObject* target)
    {
        if (target->GetTypeId() != TYPEID_UNIT)
            return true;

        if (TempSummon* summon = target->ToUnit()->ToTempSummon())
            if (summon->GetSummoner() == _caster)
                return false;

        return true;
    }

private:
    Unit* _caster;
};

class spell_gilneas_worgen_intro_completion : public SpellScript
{
    void FilterTargets(std::list<WorldObject*>& targets)
    {
        if (targets.empty())
            return;

        targets.remove_if(SummonerTargetSelector(GetCaster()));
    }

    void Register() override
    {
        OnObjectAreaTargetSelect.Register(&spell_gilneas_worgen_intro_completion::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENTRY);
    }
};

enum GaspingForBreath
{
    NPC_DROWNING_WATCHMAN        = 36440,
    NPC_DROWNING_WATCHMAN_CREDIT = 36450,  
    SPELL_RESCUE_DROWNING_WATCHMAN = 68735,
    SPELL_SAVE_DROWNING_MILITIA_EFFECT = 68737,
    SPELL_DROWNING_MILITIA_DUMMY     = 68739,
    SPELL_DROWNING_VEHICLE_EXIT_DUMMY= 68741
};

class spell_gilneas_rescue_drowning_watchman : public SpellScript
{
    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        if (Player* player = GetCaster()->ToPlayer())
        {
            if (Unit* watchman = GetHitUnit())
            {
                if (watchman->GetEntry() == NPC_DROWNING_WATCHMAN)
                    watchman->EnterVehicle(player, 0);
            }
        }
    }

    void Register() override
    {
        // Bind HandleDummy to effect 1 of spell (SPELL_EFFECT_DUMMY)
        OnEffectHitTarget.Register(&spell_gilneas_rescue_drowning_watchman::HandleDummy, EFFECT_1, SPELL_EFFECT_DUMMY);
    }
};


// 68737 Save Drowning Militia Effect
class spell_gilneas_save_drowning_milita_effect : public SpellScript
{
    void HandleScript(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);

        if (Player* player = GetHitPlayer())
            player->KilledMonsterCredit(NPC_DROWNING_WATCHMAN_CREDIT);
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_gilneas_save_drowning_milita_effect::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

// 68737 Drowning Vehicle Exit Dummy
class spell_gilneas_drowning_vehicle_exit_dummy : public SpellScript
{
    void HandleScript(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);

        if (Player* player = GetHitPlayer())
            player->RemoveAurasDueToSpell(SPELL_RESCUE_DROWNING_WATCHMAN);
    }

    void HandleHit(SpellEffIndex /*effIndex*/)
    {
        if (Unit* unit = GetHitUnit())
            unit->ExitVehicle();
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_gilneas_drowning_vehicle_exit_dummy::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        OnEffectHitTarget.Register(&spell_gilneas_drowning_vehicle_exit_dummy::HandleHit, EFFECT_1, SPELL_EFFECT_DUMMY);
    }
};

class at_gasping_for_breath : public AreaTriggerScript
{
public:
    at_gasping_for_breath() : AreaTriggerScript("at_gasping_for_breath") { }

    bool OnTrigger(Player* player, AreaTriggerEntry const* /*areaTrigger*/) override
    {
        if (Vehicle* vehicle = player->GetVehicleKit())
            if (Unit* passenger = vehicle->GetPassenger(SEAT_0))
            {
                player->CastSpell(passenger, SPELL_DROWNING_MILITIA_DUMMY);
                player->CastSpell(passenger, SPELL_DROWNING_VEHICLE_EXIT_DUMMY);
                player->CastSpell(nullptr, SPELL_SAVE_DROWNING_MILITIA_EFFECT);
            }

        return true;
    }
};

enum MountainHorse
{
    SPELL_ROUND_UP_HORSE  = 68903,
    SPELL_ROPE_CHANNEL    = 68940,
    SPELL_ROPE_IN_HORSE   = 68908,  
    SPELL_MOUNTAIN_HORSE_CREDIT = 68917,
    NPC_MOUNTAIN_HORSE    = 36540,
    NPC_MOUNTAIN_HORSE_FOLLOWER = 36555,
    NPC_MOUNTAIN_HORSE_RESCUED = 36560,  
    QUEST_THE_HUNGRY_ETTIN = 14416,       
    NPC_LORNA_CROWLEY     = 36457,
    NPC_LORNA_HORSE_TRIGGER = 800100, 
    REQUIRED_HORSE_COUNT   = 5
};

const float LORNA_CREDIT_RADIUS = 10.0f;

struct npc_mountain_horse : public ScriptedAI
{
    npc_mountain_horse(Creature* creature) : ScriptedAI(creature) 
    { 
        if (!me->GetVehicleKit())
            me->CreateVehicleKit(36540, 36540); 
        _checkTimer = 500;
        _currentRider = ObjectGuid::Empty;
        _followerGUID = ObjectGuid::Empty;
    }

    void Reset() override 
    {
        if (!me->GetVehicleKit())
            me->CreateVehicleKit(36540, 36540); 
        _checkTimer = 500;
        _currentRider = ObjectGuid::Empty;
        _followerGUID = ObjectGuid::Empty;
    }

    void OnSpellClick(Unit* clicker, bool& result) override
    {
        if (!result)
            return;

        if (Player* player = clicker->ToPlayer())
        {
            if (me->GetVehicleKit())
            {
                me->SetFaction(player->GetFaction());
                
                player->CastSpell(player, SPELL_MOUNTAIN_HORSE_CREDIT, true);
                
                _currentRider = player->GetGUID();
                
                me->SetRespawnDelay(60);
                me->SaveRespawnTime();
            }
        }
    }

    void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
    {
        if (!passenger)
            return;

        if (passenger->GetTypeId() == TYPEID_PLAYER)
        {
            if (Player* player = passenger->ToPlayer())
            {
                if (apply)
                {
                    if (!player->HasAura(SPELL_MOUNTAIN_HORSE_CREDIT))
                        player->CastSpell(player, SPELL_MOUNTAIN_HORSE_CREDIT, true);
                    
                    _currentRider = player->GetGUID();
                    
                    me->SetFaction(player->GetFaction());
                    
                    if (player->GetPetGUID().IsEmpty() && !me->IsPet())
                    {
                        me->SetOwnerGUID(player->GetGUID());
                        
                        if (!me->GetVehicleKit())
                            me->CreateVehicleKit(36540, 36540);
                    }

                    // Spawn an invisible horse follower
                    // Check if player already has horse followers
                    std::list<Creature*> followerList;
                    player->GetCreatureListWithEntryInGrid(followerList, NPC_MOUNTAIN_HORSE_FOLLOWER, 100.0f);
                    
                    // Only spawn a new follower if the player doesn't already have one
                    if (followerList.empty())
                    {
                        // Create an invisible follower at the horse's position
                        if (Creature* follower = player->SummonCreature(NPC_MOUNTAIN_HORSE_FOLLOWER, 
                                                                       me->GetPositionX(), 
                                                                       me->GetPositionY(), 
                                                                       me->GetPositionZ(), 
                                                                       me->GetOrientation(), 
                                                                       TEMPSUMMON_TIMED_DESPAWN, 1200000)) 
                        {
                            // Make it completely invisible (stealth display ID)
                            follower->SetDisplayId(11686);  // Completely invisible bunny model
                            follower->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                            
                            // Set AI flags in correct order
                            if (follower->AI())
                            {
                                // Set the flag to prevent rope display FIRST
                                follower->AI()->SetData(1, 1);
                                // Then set the GUID
                                follower->AI()->SetGUID(player->GetGUID());
                            }
                                
                            // Store the follower's GUID for later cleanup
                            _followerGUID = follower->GetGUID();
                        }
                    }
                }
                else if (!apply)
                {
                    if (me->FindNearestCreature(NPC_LORNA_CROWLEY, LORNA_CREDIT_RADIUS))
                    {
                        if (player->GetQuestStatus(QUEST_THE_HUNGRY_ETTIN) == QUEST_STATUS_INCOMPLETE)
                        {
                            player->KilledMonsterCredit(NPC_MOUNTAIN_HORSE_RESCUED);
                            
                            uint16 questSlot = player->FindQuestSlot(QUEST_THE_HUNGRY_ETTIN);
                            if (questSlot != MAX_QUEST_LOG_SIZE)
                            {
                                uint16 kills = player->GetQuestSlotCounter(questSlot, 0);
                                if (kills >= REQUIRED_HORSE_COUNT)
                                    player->CompleteQuest(QUEST_THE_HUNGRY_ETTIN);
                            }
                        }
                        
                        me->SetRespawnDelay(60); 
                        me->SaveRespawnTime();
                        me->DespawnOrUnsummon(500); 
                    }
                    
                    if (!_followerGUID.IsEmpty())
                    {
                        if (Creature* follower = ObjectAccessor::GetCreature(*me, _followerGUID))
                            follower->DespawnOrUnsummon();
                        _followerGUID = ObjectGuid::Empty;
                    }
                    
                    _currentRider = ObjectGuid::Empty;
                    
                    player->RemoveAurasDueToSpell(SPELL_MOUNTAIN_HORSE_CREDIT);
                }
            }
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (_checkTimer <= diff)
        {
            if (!me->GetVehicleKit())
                me->CreateVehicleKit(36540, 36540);
            
            Player* rider = nullptr;
            if (!_currentRider.IsEmpty())
                rider = ObjectAccessor::GetPlayer(*me, _currentRider);
                
            if (rider)
            {
                if (!rider->HasAura(SPELL_MOUNTAIN_HORSE_CREDIT))
                    rider->CastSpell(rider, SPELL_MOUNTAIN_HORSE_CREDIT, true);
                
                bool nearLorna = false;

                if (me->FindNearestCreature(NPC_LORNA_CROWLEY, LORNA_CREDIT_RADIUS))
                    nearLorna = true;
                
                if (!nearLorna)
                {
                    float lornaX = -2059.699951f;
                    float lornaY = 2254.169922f;
                    float lornaZ = 22.573099f;
                    
                    float distToLorna = me->GetDistance(lornaX, lornaY, lornaZ);
                    if (distToLorna < 10.0f)
                        nearLorna = true;
                }
                
                if (nearLorna)
                {
                    if (rider->GetQuestStatus(QUEST_THE_HUNGRY_ETTIN) == QUEST_STATUS_INCOMPLETE)
                    {
                        rider->KilledMonsterCredit(NPC_MOUNTAIN_HORSE_RESCUED);
                        
                        uint16 questSlot = rider->FindQuestSlot(QUEST_THE_HUNGRY_ETTIN);
                        if (questSlot != MAX_QUEST_LOG_SIZE)
                        {
                            uint16 kills = rider->GetQuestSlotCounter(questSlot, 0);
                            if (kills >= REQUIRED_HORSE_COUNT)
                                rider->CompleteQuest(QUEST_THE_HUNGRY_ETTIN);
                        }
                    }
                    
                    if (Vehicle* vehicle = me->GetVehicleKit())
                        if (vehicle->GetPassenger(0))
                            vehicle->RemovePassenger(vehicle->GetPassenger(0));
                    
                    rider->ExitVehicle();
                    
                    rider->RemoveAurasByType(SPELL_AURA_CONTROL_VEHICLE);
                    
                    if (rider->IsMounted())
                    {
                        rider->Dismount();
                        rider->RemoveAurasByType(SPELL_AURA_MOUNTED);
                    }
                    
                    rider->RemoveAurasDueToSpell(SPELL_MOUNTAIN_HORSE_CREDIT);
                    
                    _currentRider = ObjectGuid::Empty;
                    
                    me->SetRespawnDelay(60);
                    me->SaveRespawnTime();
                    me->DespawnOrUnsummon(500); 
                    return;
                }
            }
            
            if (Vehicle* vehicle = me->GetVehicleKit())
            {
                if (Unit* passenger = vehicle->GetPassenger(0))
                {
                    if (Player* player = passenger->ToPlayer())
                    {
                        _currentRider = player->GetGUID();
                        
                        if (!player->HasAura(SPELL_MOUNTAIN_HORSE_CREDIT))
                            player->CastSpell(player, SPELL_MOUNTAIN_HORSE_CREDIT, true);
                    }
                }
            }
            
            _checkTimer = 500;
        }
        else
        {
            _checkTimer -= diff;
        }
    }

    void SpellHit(WorldObject* caster, SpellInfo const* spell) override
    {
        if (spell->Id == SPELL_ROUND_UP_HORSE)
        {
            if (Player* player = caster->ToPlayer())
            {
                std::list<Creature*> followerList;
                player->GetCreatureListWithEntryInGrid(followerList, NPC_MOUNTAIN_HORSE_FOLLOWER, 100.0f);
                
                if (followerList.empty())
                {
                    if (Creature* follower = player->SummonCreature(NPC_MOUNTAIN_HORSE_FOLLOWER, 
                                                                   me->GetPositionX(), 
                                                                   me->GetPositionY(), 
                                                                   me->GetPositionZ(), 
                                                                   me->GetOrientation(), 
                                                                   TEMPSUMMON_TIMED_DESPAWN, 1200000)) 
                    {
                        // Make it completely invisible (stealth display ID)
                        follower->SetDisplayId(11686);  // Completely invisible bunny model
                        follower->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                        
                        if (follower->AI())
                        {
                            // Set the flag to prevent rope display FIRST
                            follower->AI()->SetData(1, 1);
                            // Then set the GUID
                            follower->AI()->SetGUID(player->GetGUID());
                        }
                    }
                }
                
                me->SetRespawnDelay(60); 
                me->SaveRespawnTime();
                me->RemoveFromWorld();
            }
        }
    }

    void JustSummoned(Creature* summon) override
    {
        if (summon->GetEntry() == NPC_MOUNTAIN_HORSE)
        {
            if (!summon->GetVehicleKit())
                summon->CreateVehicleKit(36540, 36540);
        }
    }

private:
    uint32 _checkTimer;
    ObjectGuid _currentRider;  
    ObjectGuid _followerGUID; 
};

struct npc_mountain_horse_follower : public ScriptedAI
{
    npc_mountain_horse_follower(Creature* creature) : ScriptedAI(creature)
    {
        Initialize();
    }

    void Initialize()
    {
        _playerGUID.Clear();
        _followUpdateTimer = 0;
        _pathfindingTimer = 0;
        _lastZ = 0.0f;
        _stuckCount = 0;
        _questCreditCheckTimer = 2000; 
        _dismountCheckTimer = 1000;    
        _noRopeDisplay = false;  
    }

    void Reset() override
    {
        Initialize();
    }

    void SetData(uint32 id, uint32 value) override
    {
        if (id == 1) // 1 = flag for no rope display
        {
            _noRopeDisplay = value != 0; 
            
            // Immediately remove any existing rope auras
            if (_noRopeDisplay)
            {
                me->RemoveAurasDueToSpell(SPELL_ROPE_CHANNEL);
                
                // Also ensure any attempt to reapply the aura is interrupted
                if (me->HasUnitState(UNIT_STATE_CASTING))
                    me->InterruptNonMeleeSpells(false);
            }
        }
    }

    uint32 GetData(uint32 id) const override
    {
        if (id == 1) 
            return _noRopeDisplay ? 1 : 0;
        
        return 0;
    }

    void IsSummonedBy(Unit* summoner) override
    {
        if (Player* player = summoner->ToPlayer())
        {
            _playerGUID = player->GetGUID();
            
            // Skip rope visualization completely if the flag is set
            if (!_noRopeDisplay)
                DoCast(player, SPELL_ROPE_CHANNEL, true);
            
            _lastZ = me->GetPositionZ();

            SetupFollow(player);
            
            _followUpdateTimer = 1000; 
            _pathfindingTimer = 500;  
        }
    }
    
    void SetGUID(ObjectGuid const& guid, int32 /*id*/) override
    {
        _playerGUID = guid;
        
        if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID))
        {
            // Skip rope visualization completely if the flag is set
            if (!_noRopeDisplay)
                DoCast(player, SPELL_ROPE_CHANNEL, true);
            
            _lastZ = me->GetPositionZ();

            SetupFollow(player);
            
            _followUpdateTimer = 1000; 
            _pathfindingTimer = 500;   
        }
    }

    void SetupFollow(Unit* target)
    {
        if (!target)
            return;

        me->GetMotionMaster()->Clear();
        me->GetMotionMaster()->MoveFollow(target, 3.0f, DEFAULT_FOLLOW_ANGLE, MOTION_SLOT_ACTIVE, 0, true);
        
        if (target->IsMounted() || target->GetVehicle())
        {
            if (Unit* vehicle = target->GetVehicleBase())
                me->SetSpeedRate(MOVE_RUN, vehicle->GetSpeedRate(MOVE_RUN) * 1.1f);
        }
        else
        {
            me->SetSpeedRate(MOVE_RUN, target->GetSpeedRate(MOVE_RUN) * 1.1f);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID))
        {
            Unit* followTarget = player;
            if (player->IsMounted() || player->GetVehicle())
            {
                if (Unit* vehicle = player->GetVehicleBase())
                {
                    followTarget = vehicle;
                    me->SetSpeedRate(MOVE_RUN, vehicle->GetSpeedRate(MOVE_RUN) * 1.1f);
                }
            }

            float maxDistance = 25.0f;

            if (me->GetDistance(followTarget) > maxDistance)
            {
                float x = followTarget->GetPositionX() - 3.0f * cos(followTarget->GetOrientation());
                float y = followTarget->GetPositionY() - 3.0f * sin(followTarget->GetOrientation());
                float z = followTarget->GetPositionZ();
                
                me->NearTeleportTo(x, y, z, me->GetOrientation());
                
                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MoveFollow(followTarget, 3.0f, DEFAULT_FOLLOW_ANGLE, MOTION_SLOT_ACTIVE, 0, true);
            }

            if (_pathfindingTimer <= diff)
            {
                float currentZ = me->GetPositionZ();
                if (fabs(currentZ - _lastZ) < 0.2f && me->GetDistance(followTarget) > 10.0f)
                {
                    _stuckCount++;
                    if (_stuckCount >= 5) 
                    {
                        float x = followTarget->GetPositionX() - 3.0f * cos(followTarget->GetOrientation());
                        float y = followTarget->GetPositionY() - 3.0f * sin(followTarget->GetOrientation());
                        float z = followTarget->GetPositionZ();
                        
                        me->NearTeleportTo(x, y, z, me->GetOrientation());
                        
                        _stuckCount = 0;
                    }
                }
                else
                {
                    _stuckCount = 0;
                }
                
                _lastZ = currentZ;
                
                _pathfindingTimer = 500; 
            }
            else
            {
                _pathfindingTimer -= diff;
            }
            
            if (_followUpdateTimer <= diff)
            {
                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MoveFollow(followTarget, 3.0f, DEFAULT_FOLLOW_ANGLE, MOTION_SLOT_ACTIVE, 0, true);
                
                // For normal followers with rope visuals, make sure the rope stays connected
                if (!_noRopeDisplay && !me->HasAura(SPELL_ROPE_CHANNEL))
                {
                    DoCast(player, SPELL_ROPE_CHANNEL, true);
                }
                else if (_noRopeDisplay && me->HasAura(SPELL_ROPE_CHANNEL))
                {
                    // If we should not display the rope but it's showing, remove it
                    me->RemoveAurasDueToSpell(SPELL_ROPE_CHANNEL);
                }
                
                _followUpdateTimer = 1000;
            }
            else
            {
                _followUpdateTimer -= diff;
            }

            // Check for dismount near Lorna Crowley's position
            if (_dismountCheckTimer <= diff)
            {
                float lornaX = -2059.699951f;
                float lornaY = 2254.169922f;
                float lornaZ = 22.573099f;
            
                float distToLorna = player->GetDistance(lornaX, lornaY, lornaZ);
                
                if (distToLorna < 8.0f)
                {
                    if (player->GetVehicle() || player->GetVehicleBase())
                    {
                        if (Unit* vehicleBase = player->GetVehicleBase())
                        {
                            if (vehicleBase->GetEntry() == NPC_MOUNTAIN_HORSE)
                            {
                                vehicleBase->ToCreature()->SetRespawnDelay(60);
                                vehicleBase->ToCreature()->SaveRespawnTime();
                                vehicleBase->ToCreature()->RemoveFromWorld();
                            }
                        }
                        
                        player->ExitVehicle();
                    }
                    
                    if (player->IsMounted())
                    {
                        player->Dismount();
                        player->RemoveAurasByType(SPELL_AURA_MOUNTED);
                    }
                    
                    player->RemoveAurasByType(SPELL_AURA_CONTROL_VEHICLE);
                    
                    if (player->GetQuestStatus(QUEST_THE_HUNGRY_ETTIN) == QUEST_STATUS_INCOMPLETE)
                    {
                        player->KilledMonsterCredit(NPC_MOUNTAIN_HORSE_FOLLOWER);
                        
                        uint16 questSlot = player->FindQuestSlot(QUEST_THE_HUNGRY_ETTIN);
                        if (questSlot != MAX_QUEST_LOG_SIZE)
                        {
                            uint16 kills = player->GetQuestSlotCounter(questSlot, 0);
                            if (kills >= REQUIRED_HORSE_COUNT)
                                player->CompleteQuest(QUEST_THE_HUNGRY_ETTIN);
                        }
                    }
                }
                
                _dismountCheckTimer = 1000; 
            }
            else
            {
                _dismountCheckTimer -= diff;
            }

            if (_questCreditCheckTimer <= diff)
            {
                if (player->GetQuestStatus(QUEST_THE_HUNGRY_ETTIN) == QUEST_STATUS_INCOMPLETE)
                {
                    if (player->FindNearestCreature(NPC_LORNA_CROWLEY, LORNA_CREDIT_RADIUS))
                    {
                        player->KilledMonsterCredit(NPC_MOUNTAIN_HORSE_RESCUED);
                        
                        uint16 questSlot = player->FindQuestSlot(QUEST_THE_HUNGRY_ETTIN);
                        if (questSlot != MAX_QUEST_LOG_SIZE)
                        {
                            uint16 kills = player->GetQuestSlotCounter(questSlot, 0);
                            if (kills >= REQUIRED_HORSE_COUNT)
                                player->CompleteQuest(QUEST_THE_HUNGRY_ETTIN);
                        }
                            
                        me->RemoveAurasDueToSpell(SPELL_ROPE_CHANNEL);
                        me->DespawnOrUnsummon(1000);
                    }
                }
                
                _questCreditCheckTimer = 2000;
            }
            else
            {
                _questCreditCheckTimer -= diff;
            }
        }
        else
        {
            me->DespawnOrUnsummon();
        }
    }

    void SpellHit(WorldObject* /*caster*/, SpellInfo const* spell) override
    {
        // Prevent rope channel spell from being applied if _noRopeDisplay is set
        if (spell->Id == SPELL_ROPE_CHANNEL && _noRopeDisplay)
        {
            me->RemoveAurasDueToSpell(SPELL_ROPE_CHANNEL);
        }
    }

private:
    ObjectGuid _playerGUID;
    uint32 _followUpdateTimer;
    uint32 _pathfindingTimer;
    uint32 _questCreditCheckTimer;
    uint32 _dismountCheckTimer;
    float _lastZ;
    uint8 _stuckCount;
    bool _noRopeDisplay;
};

class spell_round_up_horse : public SpellScript
{
    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        if (Unit* caster = GetCaster())
        {
            if (Player* player = caster->ToPlayer())
            {
                if (Unit* target = GetHitUnit())
                {
                    if (target->GetEntry() == NPC_MOUNTAIN_HORSE)
                    {
                        std::list<Creature*> followerList;
                        player->GetCreatureListWithEntryInGrid(followerList, NPC_MOUNTAIN_HORSE_FOLLOWER, 100.0f);
                        
                        if (!followerList.empty())
                        {
                            if (Creature* horse = target->ToCreature())
                            {
                                horse->SetRespawnDelay(60);
                                horse->SaveRespawnTime();
                                
                                horse->RemoveFromWorld();
                            }
                        }
                    }
                }
            }
        }
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_round_up_horse::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

struct npc_lorna_crowley_gilneas : public ScriptedAI
{
    npc_lorna_crowley_gilneas(Creature* creature) : ScriptedAI(creature)
    {
        _checkTimer = 500;
    }

    void Reset() override
    {
        _checkTimer = 500;
    }

    void UpdateAI(uint32 diff) override
    {
        if (_checkTimer <= diff)
        {
            std::list<Player*> playerList;
            me->GetPlayerListInGrid(playerList, 15.0f);
            
            for (Player* player : playerList)
            {
                if (player->GetQuestStatus(QUEST_THE_HUNGRY_ETTIN) == QUEST_STATUS_INCOMPLETE)
                {
                    std::list<Creature*> followerList;
                    player->GetCreatureListWithEntryInGrid(followerList, NPC_MOUNTAIN_HORSE_FOLLOWER, 50.0f);
                    
                    if (!followerList.empty())
                    {
                        for (uint8 i = 0; i < followerList.size(); i++)
                            player->KilledMonsterCredit(NPC_MOUNTAIN_HORSE_RESCUED);
                        
                        uint16 questSlot = player->FindQuestSlot(QUEST_THE_HUNGRY_ETTIN);
                        if (questSlot != MAX_QUEST_LOG_SIZE)
                        {
                            uint16 updatedKills = player->GetQuestSlotCounter(questSlot, 0);
                            if (updatedKills >= REQUIRED_HORSE_COUNT)
                                player->CompleteQuest(QUEST_THE_HUNGRY_ETTIN);
                        }
                            
                        for (Creature* follower : followerList)
                        {
                            follower->RemoveAurasDueToSpell(SPELL_ROPE_CHANNEL);
                            follower->DespawnOrUnsummon(1000);
                        }
                    }
                    
                    if (player->HasAura(SPELL_MOUNTAIN_HORSE_CREDIT))
                    {
                        bool isInRecognizedVehicle = false;
                        
                        if (player->GetVehicle() && player->GetVehicleBase())
                        {
                            Unit* vehicleBase = player->GetVehicleBase();
                            if (vehicleBase->GetEntry() == NPC_MOUNTAIN_HORSE)
                            {
                                isInRecognizedVehicle = true;
                                player->ExitVehicle();
                                
                                vehicleBase->ToCreature()->SetRespawnDelay(60);
                                vehicleBase->ToCreature()->SaveRespawnTime();
                                vehicleBase->ToCreature()->DespawnOrUnsummon(500); 
                            }
                        }
                        
                        if (!isInRecognizedVehicle)
                        {
                            std::list<Creature*> nearbyHorses;
                            player->GetCreatureListWithEntryInGrid(nearbyHorses, NPC_MOUNTAIN_HORSE, 5.0f);
                            
                            for (Creature* horse : nearbyHorses)
                            {
                                if (horse->GetFaction() == player->GetFaction() || 
                                    horse->GetCharmerGUID() == player->GetGUID() || 
                                    horse->GetOwnerGUID() == player->GetGUID())
                                {
                                    player->KilledMonsterCredit(NPC_MOUNTAIN_HORSE_RESCUED);
                                    
                                    uint16 questSlot = player->FindQuestSlot(QUEST_THE_HUNGRY_ETTIN);
                                    if (questSlot != MAX_QUEST_LOG_SIZE)
                                    {
                                        uint16 kills = player->GetQuestSlotCounter(questSlot, 0);
                                        if (kills >= REQUIRED_HORSE_COUNT)
                                            player->CompleteQuest(QUEST_THE_HUNGRY_ETTIN);
                                    }
                                    
                                    if (player->IsMounted())
                                    {
                                        player->Dismount();
                                        player->RemoveAurasByType(SPELL_AURA_MOUNTED);
                                    }
                                    
                                    player->RemoveAurasByType(SPELL_AURA_CONTROL_VEHICLE);
                                    
                                    horse->SetRespawnDelay(60);
                                    horse->SaveRespawnTime();
                                    horse->DespawnOrUnsummon(500); 
                                    break;
                                }
                            }
                        }
                        
                        player->RemoveAurasDueToSpell(SPELL_MOUNTAIN_HORSE_CREDIT);
                    }
                }
            }
            
            _checkTimer = 500; 
        }
        else
        {
            _checkTimer -= diff;
        }
    }

private:
    uint32 _checkTimer;
};

struct npc_lorna_horse_trigger : public ScriptedAI
{
    npc_lorna_horse_trigger(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetDisplayId(11686); 
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
        me->AddUnitState(UNIT_STATE_ROOT);
        _checkTimer = 100; 
    }

    void Reset() override
    {
        _checkTimer = 100; 
    }

    void UpdateAI(uint32 diff) override
    {
        if (_checkTimer <= diff)
        {
            std::list<Player*> playerList;
            me->GetPlayerListInGrid(playerList, 20.0f); 
            
            for (Player* player : playerList)
            {
                bool checkedVehicle = false;
                
                if (Unit* vehicle = player->GetVehicleBase())
                {
                    checkedVehicle = true;
                    if (vehicle->GetEntry() == NPC_MOUNTAIN_HORSE)
                    {
                        if (player->GetQuestStatus(QUEST_THE_HUNGRY_ETTIN) == QUEST_STATUS_INCOMPLETE)
                        {
                            player->KilledMonsterCredit(NPC_MOUNTAIN_HORSE_RESCUED);
                            
                            if (Vehicle* veh = vehicle->GetVehicleKit())
                                if (veh->GetPassenger(0))
                                    veh->RemovePassenger(veh->GetPassenger(0));
                            
                            player->ExitVehicle();
                            
                            player->RemoveAurasByType(SPELL_AURA_CONTROL_VEHICLE);
                             
                            if (player->IsMounted())
                            {
                                player->Dismount();
                                player->RemoveAurasByType(SPELL_AURA_MOUNTED);
                            }
                            
                            player->RemoveAurasDueToSpell(SPELL_MOUNTAIN_HORSE_CREDIT);
                            
                            //if (Map* map = me->GetMap())
                            if (me->GetMap())
                            {
                                Creature* horseVehicle = vehicle->ToCreature();
                                if (horseVehicle)
                                {
                                    // Set respawn time and despawn the horse
                                    horseVehicle->SetRespawnDelay(60);
                                    horseVehicle->SaveRespawnTime();
                                    horseVehicle->DespawnOrUnsummon(100); 
                                }
                            }
                            
                            // Check if player has completed the quest
                            uint16 questSlot = player->FindQuestSlot(QUEST_THE_HUNGRY_ETTIN);
                            if (questSlot != MAX_QUEST_LOG_SIZE)
                            {
                                uint16 kills = player->GetQuestSlotCounter(questSlot, 0);
                                if (kills >= REQUIRED_HORSE_COUNT)
                                    player->CompleteQuest(QUEST_THE_HUNGRY_ETTIN);
                            }
                        }
                    }
                }
                
                // Check for credit aura even if not in a recognized vehicle
                if (!checkedVehicle && player->HasAura(SPELL_MOUNTAIN_HORSE_CREDIT))
                {
                    
                    // Check if player has the quest
                    if (player->GetQuestStatus(QUEST_THE_HUNGRY_ETTIN) == QUEST_STATUS_INCOMPLETE)
                    {
                        // Give quest credit
                        player->KilledMonsterCredit(NPC_MOUNTAIN_HORSE_RESCUED);
                        
                        // Check for nearby horses that might be the player's
                        std::list<Creature*> nearbyHorses;
                        player->GetCreatureListWithEntryInGrid(nearbyHorses, NPC_MOUNTAIN_HORSE, 10.0f);
                        
                        for (Creature* horse : nearbyHorses)
                        {
                            // Set respawn time and despawn the horse
                            horse->SetRespawnDelay(60);
                            horse->SaveRespawnTime();
                            horse->DespawnOrUnsummon(100);
                        }
                        
                        // Remove mount auras
                        if (player->IsMounted())
                        {
                            player->Dismount();
                            player->RemoveAurasByType(SPELL_AURA_MOUNTED);
                        }
                        
                        // Remove the credit aura
                        player->RemoveAurasDueToSpell(SPELL_MOUNTAIN_HORSE_CREDIT);
                        
                        // Check if player has completed the quest
                        uint16 questSlot = player->FindQuestSlot(QUEST_THE_HUNGRY_ETTIN);
                        if (questSlot != MAX_QUEST_LOG_SIZE)
                        {
                            uint16 kills = player->GetQuestSlotCounter(questSlot, 0);
                            if (kills >= REQUIRED_HORSE_COUNT)
                                player->CompleteQuest(QUEST_THE_HUNGRY_ETTIN);
                        }
                    }
                }
            }
            
            _checkTimer = 100; 
        }
        else
        {
            _checkTimer -= diff;
        }
    }

private:
    uint32 _checkTimer;
};

}

void AddSC_gilneas_chapter_2()
{
    using namespace Gilneas::Chapter2;
    RegisterGameObjectAI(go_gilneas_invasion_camera);
    RegisterCreatureAI(npc_gilneas_horrid_abomination);
    RegisterCreatureAI(npc_gilneas_save_the_children);
    RegisterCreatureAI(npc_gilneas_forsaken_catapult);
    RegisterCreatureAI(npc_lorna_crowley_gilneas);
    RegisterCreatureAI(npc_lorna_horse_trigger); 
    RegisterSpellScript(spell_gilneas_quest_save_the_children);
    RegisterSpellScript(spell_gilneas_launch);
    RegisterSpellScript(spell_gilneas_fiery_boulder);
    RegisterSpellScript(spell_gilneas_call_attack_mastiff);
    RegisterSpellScript(spell_gilneas_forcecast_cataclysm_1);
    RegisterSpellScript(spell_gilneas_worgen_intro_completion);
    RegisterSpellScript(spell_gilneas_save_drowning_milita_effect);
    RegisterSpellScript(spell_gilneas_drowning_vehicle_exit_dummy);
    RegisterSpellScript(spell_gilneas_rescue_drowning_watchman);
    new at_gasping_for_breath();
    RegisterCreatureAI(npc_mountain_horse);
    RegisterCreatureAI(npc_mountain_horse_follower);
    RegisterSpellScript(spell_round_up_horse);
}