/*
 * This file is part of the ShatterCore Project. See AUTHORS file for Copyright information
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

#include "firelands.h"
#include "InstanceScript.h"
#include "Map.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "PassiveAI.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "TemporarySummon.h"

namespace Firelands::Bethtilac
{

enum Spells
{
    // Beth'tilac
    SPELL_EMBER_FLARE               = 98934,
    SPELL_METEOR_BURN               = 99076,
    SPELL_CONSUME                   = 99304,
    SPELL_SMOLDERING_DEVASTATION    = 99052,
    SPELL_VENOM_RAIN                = 99333,
    SPELL_ZERO_POWER_REGEN          = 72242,
    SPELL_FRENZY                    = 99497,

    // Cinderweb Spinner
    SPELL_BURNING_ACID_SPINNER      = 98471,
    SPELL_FIERY_WEB_SPIN            = 97202,

    // Cinderweb Drone
    SPELL_BOILING_SPLATTER          = 99463,
    SPELL_BURNING_ACID_DRONE        = 98471,
    SPELL_CONSUME_DRONE             = 99352,
    SPELL_LEECH_VENOM               = 99506,

    // Cinderweb Spiderling
    SPELL_SEEPING_VENOM             = 97079,
};

enum Events
{
    // Beth'tilac
    EVENT_DRAIN_ENERGY = 1,
    EVENT_EMBER_FLARE,
    EVENT_METEOR_BURN,
    EVENT_CHECK_WEB_PLAYERS,
    EVENT_SMOLDERING_DEVASTATION,
    EVENT_RESTORE_ENERGY,
    EVENT_SPAWN_SPINNERS,
    EVENT_SPAWN_SPIDERLINGS,
    EVENT_SPAWN_DRONE,

    // Cinderweb Spinner
    EVENT_BURNING_ACID,
    EVENT_FIERY_WEB_SPIN,

    // Cinderweb Drone
    EVENT_BOILING_SPLATTER,
    EVENT_DRONE_BURNING_ACID,
    EVENT_DRAIN_DRONE_ENERGY,

    // Cinderweb Spiderling
    EVENT_FIND_TARGET,
    EVENT_CHECK_DISTANCE,
};

enum Actions
{
    ACTION_SPIDERLING_CONSUMED = 1,
    ACTION_DRONE_LEECH_ENERGY,
};

enum Points
{
    POINT_WEB_LEVEL = 1,
    POINT_GROUND_LEVEL,
};

// Z-coordinate constants (from sniff data)
float const WEB_LEVEL_Z = 88.42f;       // Beth'tilac web platform height
float const GROUND_LEVEL_Z = 45.0f;     // Ground floor height
float const WEB_THRESHOLD_Z = 66.0f;    // Detection midpoint for abilities

// Spawn positions from sniff data
// Beth'tilac position: X: 63.70139 Y: 387.3229 Z: 88.42146
Position const BethilacHomePos = { 63.70139f, 387.3229f, 88.42146f, 0.0f };

// Cinderweb Spinner spawn positions (suspended at mid-web level)
Position const SpinnerSpawnPositions[] =
{
    { -425.47f, 285.56f, 77.86f, 0.0f },
    { -414.50f, 266.49f, 70.99f, 0.0f },
    { -426.36f, 285.80f, 78.18f, 0.0f },
    { -428.36f, 285.57f, 79.22f, 0.0f },
};

// Cinderweb Spiderling spawn positions (ground level caves)
Position const SpiderlingSpawnPositions[] =
{
    { -284.49f, 162.31f, 47.96f, 0.0f },
    { -264.09f, 256.98f, 45.21f, 0.0f },
    { -292.31f, 230.11f, 59.87f, 0.0f },
    { -335.94f, 204.62f, 62.24f, 0.0f },
    { -282.64f, 198.72f, 47.58f, 0.0f },
    { -276.11f, 201.54f, 47.60f, 0.0f },
    { -260.22f, 252.29f, 45.04f, 0.0f },
    { -258.71f, 249.64f, 44.93f, 0.0f },
};

// Cinderweb Drone spawn positions (ground level caves)
Position const DroneSpawnPositions[] =
{
    { -307.52f, 44.66f, 45.07f, 0.0f },
    { -280.65f, 41.62f, 45.31f, 0.0f },
    { -262.91f, -36.65f, 44.76f, 0.0f },
    { -200.97f, 76.29f, 44.81f, 0.0f },
};

// Beth'tilac AI
struct boss_bethtilac : public BossAI
{
    boss_bethtilac(Creature* creature) : BossAI(creature, DATA_BETHTILAC),
        _smolderingCount(0), _fireEnergy(100)
    {
        me->setActive(true);
    }

    void InitializeAI() override
    {
        BossAI::InitializeAI();
        me->SetPowerType(POWER_ENERGY);
        me->SetMaxPower(POWER_ENERGY, 100);
        me->SetPower(POWER_ENERGY, 100);
    }

    void Reset() override
    {
        _Reset();
        _smolderingCount = 0;
        _fireEnergy = 100;
        me->SetPower(POWER_ENERGY, 100);
    }

    void JustEngagedWith(Unit* who) override
    {
        BossAI::JustEngagedWith(who);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

        _smolderingCount = 0;
        _fireEnergy = 100;
        me->SetPower(POWER_ENERGY, 100);

        // Prevent energy regeneration
        DoCastSelf(SPELL_ZERO_POWER_REGEN, true);

        // Schedule abilities
        events.ScheduleEvent(EVENT_DRAIN_ENERGY, 900ms);
        events.ScheduleEvent(EVENT_EMBER_FLARE, 5s);
        events.ScheduleEvent(EVENT_METEOR_BURN, 8s);
        events.ScheduleEvent(EVENT_CHECK_WEB_PLAYERS, 1s);

        // Schedule add spawns
        events.ScheduleEvent(EVENT_SPAWN_SPINNERS, 100ms);
        events.ScheduleEvent(EVENT_SPAWN_SPIDERLINGS, 30s);
        events.ScheduleEvent(EVENT_SPAWN_DRONE, 40s);
    }

    void JustDied(Unit* killer) override
    {
        _JustDied();
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
    }

    void EnterEvadeMode(EvadeReason why) override
    {
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        summons.DespawnAll();
        _DespawnAtEvade();
    }

    void DoAction(int32 action) override
    {
        switch (action)
        {
            case ACTION_SPIDERLING_CONSUMED:
                // Heal 10% max HP
                me->ModifyHealth(int32(me->GetMaxHealth() * 0.10f));
                break;
            case ACTION_DRONE_LEECH_ENERGY:
                // Drone reached web level and is leeching energy
                // Accelerate energy drain
                if (_fireEnergy > 20)
                    _fireEnergy -= 20;
                else
                    _fireEnergy = 0;
                me->SetPower(POWER_ENERGY, _fireEnergy);
                break;
            default:
                break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_DRAIN_ENERGY:
                    DrainFireEnergy();
                    events.ScheduleEvent(EVENT_DRAIN_ENERGY, 900ms);
                    break;
                case EVENT_EMBER_FLARE:
                    CastEmberFlareOnWebPlayers();
                    events.ScheduleEvent(EVENT_EMBER_FLARE, 5s, 6s);
                    break;
                case EVENT_METEOR_BURN:
                    CastMeteorBurn();
                    events.ScheduleEvent(EVENT_METEOR_BURN, 10s, 15s);
                    break;
                case EVENT_CHECK_WEB_PLAYERS:
                    CheckWebLevelPlayers();
                    events.ScheduleEvent(EVENT_CHECK_WEB_PLAYERS, 2s);
                    break;
                case EVENT_SMOLDERING_DEVASTATION:
                    DoCastAOE(SPELL_SMOLDERING_DEVASTATION);
                    _smolderingCount++;
                    if (_smolderingCount >= 3)
                        EnterPhaseTwo();
                    else
                        events.ScheduleEvent(EVENT_RESTORE_ENERGY, 9s);
                    break;
                case EVENT_RESTORE_ENERGY:
                    _fireEnergy = 100;
                    me->SetPower(POWER_ENERGY, 100);
                    events.ScheduleEvent(EVENT_DRAIN_ENERGY, 900ms);
                    break;
                case EVENT_SPAWN_SPINNERS:
                    SpawnSpinners(2);
                    break;
                case EVENT_SPAWN_SPIDERLINGS:
                    SpawnSpiderlings(8);
                    events.ScheduleEvent(EVENT_SPAWN_SPIDERLINGS, 30s);
                    break;
                case EVENT_SPAWN_DRONE:
                    SpawnDrone();
                    events.ScheduleEvent(EVENT_SPAWN_DRONE, 40s, 60s);
                    break;
                default:
                    break;
            }
        }

        DoMeleeAttackIfReady();
    }

private:
    uint8 _smolderingCount;
    uint8 _fireEnergy;

    void DrainFireEnergy()
    {
        if (_fireEnergy > 0)
        {
            _fireEnergy--;
            me->SetPower(POWER_ENERGY, _fireEnergy);

            if (_fireEnergy == 0)
            {
                events.CancelEvent(EVENT_DRAIN_ENERGY);
                events.ScheduleEvent(EVENT_SMOLDERING_DEVASTATION, 100ms);
            }
        }
    }

    void CastEmberFlareOnWebPlayers()
    {
        Map::PlayerList const& players = me->GetMap()->GetPlayers();
        for (auto const& pair : players)
        {
            Player* player = pair.GetSource();
            if (player && player->IsAlive() && player->GetPositionZ() > WEB_THRESHOLD_Z)
                me->CastSpell(player, SPELL_EMBER_FLARE, true);
        }
    }

    void CastMeteorBurn()
    {
        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
        {
            Position pos = target->GetPosition();
            pos.m_positionZ = WEB_LEVEL_Z;
            me->CastSpell(pos, SPELL_METEOR_BURN, true);
        }
    }

    void CheckWebLevelPlayers()
    {
        bool hasWebPlayers = false;
        Map::PlayerList const& players = me->GetMap()->GetPlayers();
        for (auto const& pair : players)
        {
            Player* player = pair.GetSource();
            if (player && player->IsAlive() && player->GetPositionZ() > WEB_THRESHOLD_Z)
            {
                hasWebPlayers = true;
                break;
            }
        }

        if (!hasWebPlayers && !me->HasAura(SPELL_VENOM_RAIN))
            DoCastAOE(SPELL_VENOM_RAIN);
        else if (hasWebPlayers && me->HasAura(SPELL_VENOM_RAIN))
            me->RemoveAurasDueToSpell(SPELL_VENOM_RAIN);
    }

    void SpawnSpinners(uint8 count)
    {
        for (uint8 i = 0; i < count && i < std::size(SpinnerSpawnPositions); ++i)
        {
            if (TempSummon* spinner = me->SummonCreature(NPC_CINDERWEB_SPINNER,
                SpinnerSpawnPositions[i], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000))
            {
                spinner->AI()->DoZoneInCombat();
            }
        }
    }

    void SpawnSpiderlings(uint8 count)
    {
        for (uint8 i = 0; i < count; ++i)
        {
            uint8 posIndex = i % std::size(SpiderlingSpawnPositions);
            if (TempSummon* spiderling = me->SummonCreature(NPC_CINDERWEB_SPIDERLING,
                SpiderlingSpawnPositions[posIndex], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000))
            {
                spiderling->AI()->DoZoneInCombat();
            }
        }
    }

    void SpawnDrone()
    {
        static uint8 droneIndex = 0;
        uint8 posIndex = droneIndex % std::size(DroneSpawnPositions);
        droneIndex++;

        if (TempSummon* drone = me->SummonCreature(NPC_CINDERWEB_DRONE,
            DroneSpawnPositions[posIndex], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000))
        {
            drone->AI()->DoZoneInCombat();
        }
    }

    void EnterPhaseTwo()
    {
        // Cancel Phase 1 events
        events.CancelEvent(EVENT_EMBER_FLARE);
        events.CancelEvent(EVENT_METEOR_BURN);
        events.CancelEvent(EVENT_SPAWN_SPIDERLINGS);
        events.CancelEvent(EVENT_SPAWN_DRONE);
        events.CancelEvent(EVENT_SPAWN_SPINNERS);
        events.CancelEvent(EVENT_CHECK_WEB_PLAYERS);

        // Remove Venom Rain
        me->RemoveAurasDueToSpell(SPELL_VENOM_RAIN);

        // Despawn remaining adds
        summons.DespawnAll();

        // Phase 2 mechanics to be implemented
        // For now, just continue combat at ground level
    }
};

// Cinderweb Spinner AI
struct npc_cinderweb_spinner : public ScriptedAI
{
    npc_cinderweb_spinner(Creature* creature) : ScriptedAI(creature),
        _isSuspended(true)
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetDisableGravity(true);
    }

    void Reset() override
    {
        _events.Reset();
        _isSuspended = true;
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        _events.ScheduleEvent(EVENT_BURNING_ACID, 5s);
        if (_isSuspended)
            _events.ScheduleEvent(EVENT_FIERY_WEB_SPIN, 10s);
    }

    void DamageTaken(Unit* attacker, uint32& /*damage*/) override
    {
        if (_isSuspended && attacker && attacker->IsPlayer())
            DescendToGround();
    }

    void SpellHit(WorldObject* /*caster*/, SpellInfo const* spell) override
    {
        // Check if spell is a taunt
        if (_isSuspended && spell && spell->HasEffect(SPELL_EFFECT_ATTACK_ME))
            DescendToGround();
    }

    void JustDied(Unit* /*killer*/) override
    {
        // Spawn web filament for player ascent
        if (TempSummon* filament = me->SummonCreature(NPC_WEB_FILAMENT,
            me->GetPosition(), TEMPSUMMON_TIMED_DESPAWN, 120000))
        {
            filament->SetReactState(REACT_PASSIVE);
            filament->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
        }
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
                case EVENT_BURNING_ACID:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f, true))
                        DoCast(target, SPELL_BURNING_ACID_SPINNER);
                    _events.ScheduleEvent(EVENT_BURNING_ACID, 10s);
                    break;
                case EVENT_FIERY_WEB_SPIN:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f, true))
                        DoCast(target, SPELL_FIERY_WEB_SPIN);
                    _events.ScheduleEvent(EVENT_FIERY_WEB_SPIN, 25s);
                    break;
                default:
                    break;
            }
        }

        DoMeleeAttackIfReady();
    }

private:
    EventMap _events;
    bool _isSuspended;

    void DescendToGround()
    {
        _isSuspended = false;
        me->SetDisableGravity(false);
        me->SetReactState(REACT_AGGRESSIVE);
        _events.CancelEvent(EVENT_FIERY_WEB_SPIN);

        Position pos = me->GetPosition();
        pos.m_positionZ = GROUND_LEVEL_Z;
        me->GetMotionMaster()->MovePoint(POINT_GROUND_LEVEL, pos);
    }
};

// Cinderweb Drone AI
struct npc_cinderweb_drone : public ScriptedAI
{
    npc_cinderweb_drone(Creature* creature) : ScriptedAI(creature),
        _droneEnergy(85), _isClimbing(false), _instance(creature->GetInstanceScript())
    {
        me->SetPowerType(POWER_ENERGY);
        me->SetMaxPower(POWER_ENERGY, 85);
        me->SetPower(POWER_ENERGY, 85);
    }

    void Reset() override
    {
        _events.Reset();
        _droneEnergy = 85;
        _isClimbing = false;
        me->SetPower(POWER_ENERGY, 85);
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        _events.ScheduleEvent(EVENT_BOILING_SPLATTER, 6s);
        _events.ScheduleEvent(EVENT_DRONE_BURNING_ACID, 8s);
        _events.ScheduleEvent(EVENT_DRAIN_DRONE_ENERGY, 1s);
    }

    void DoAction(int32 action) override
    {
        if (action == ACTION_SPIDERLING_CONSUMED)
        {
            // Heal 20% max HP and gain buff
            me->ModifyHealth(int32(me->GetMaxHealth() * 0.20f));
            DoCastSelf(SPELL_CONSUME_DRONE, true);
        }
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type == POINT_MOTION_TYPE && id == POINT_WEB_LEVEL)
        {
            // Reached web level, start leeching Beth'tilac
            DoCastSelf(SPELL_LEECH_VENOM);

            // Notify Beth'tilac
            if (_instance)
            {
                if (Creature* bethtilac = _instance->GetCreature(DATA_BETHTILAC))
                    bethtilac->AI()->DoAction(ACTION_DRONE_LEECH_ENERGY);
            }
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (_isClimbing)
            return;

        if (!UpdateVictim())
            return;

        _events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_DRAIN_DRONE_ENERGY:
                    if (_droneEnergy > 0)
                    {
                        _droneEnergy--;
                        me->SetPower(POWER_ENERGY, _droneEnergy);

                        if (_droneEnergy == 0)
                            ClimbToWebLevel();
                        else
                            _events.ScheduleEvent(EVENT_DRAIN_DRONE_ENERGY, 1s);
                    }
                    break;
                case EVENT_BOILING_SPLATTER:
                    DoCastVictim(SPELL_BOILING_SPLATTER);
                    _events.ScheduleEvent(EVENT_BOILING_SPLATTER, 10s);
                    break;
                case EVENT_DRONE_BURNING_ACID:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f, true))
                        DoCast(target, SPELL_BURNING_ACID_DRONE);
                    _events.ScheduleEvent(EVENT_DRONE_BURNING_ACID, 12s);
                    break;
                default:
                    break;
            }
        }

        DoMeleeAttackIfReady();
    }

private:
    EventMap _events;
    uint8 _droneEnergy;
    bool _isClimbing;
    InstanceScript* _instance;

    void ClimbToWebLevel()
    {
        _isClimbing = true;
        me->SetReactState(REACT_PASSIVE);
        me->SetDisableGravity(true);
        me->GetMotionMaster()->Clear();

        // Move to web level
        Position webPos = me->GetPosition();
        webPos.m_positionZ = WEB_LEVEL_Z;
        me->GetMotionMaster()->MovePoint(POINT_WEB_LEVEL, webPos);
    }
};

// Cinderweb Spiderling AI
struct npc_cinderweb_spiderling : public ScriptedAI
{
    npc_cinderweb_spiderling(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript())
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void Reset() override
    {
        _events.Reset();
        _targetGUID.Clear();
    }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        DoCastSelf(SPELL_SEEPING_VENOM, true);
        _events.ScheduleEvent(EVENT_FIND_TARGET, 500ms);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_FIND_TARGET:
                {
                    Creature* target = FindNearestDroneOrBoss();
                    if (target)
                    {
                        _targetGUID = target->GetGUID();
                        me->GetMotionMaster()->MoveFollow(target, 0.0f, 0.0f);
                        _events.ScheduleEvent(EVENT_CHECK_DISTANCE, 500ms);
                    }
                    else
                        _events.ScheduleEvent(EVENT_FIND_TARGET, 1s);
                    break;
                }
                case EVENT_CHECK_DISTANCE:
                {
                    Creature* target = ObjectAccessor::GetCreature(*me, _targetGUID);
                    if (target && target->IsAlive() && me->GetDistance2d(target) < 2.0f)
                    {
                        // Consumed by target
                        target->AI()->DoAction(ACTION_SPIDERLING_CONSUMED);
                        me->DespawnOrUnsummon(100ms);
                    }
                    else
                    {
                        // Re-evaluate target in case it died or moved
                        Creature* newTarget = FindNearestDroneOrBoss();
                        if (newTarget && (!target || newTarget->GetGUID() != _targetGUID))
                        {
                            _targetGUID = newTarget->GetGUID();
                            me->GetMotionMaster()->MoveFollow(newTarget, 0.0f, 0.0f);
                        }
                        _events.ScheduleEvent(EVENT_CHECK_DISTANCE, 500ms);
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }

private:
    EventMap _events;
    ObjectGuid _targetGUID;
    InstanceScript* _instance;

    Creature* FindNearestDroneOrBoss()
    {
        // Priority: Nearest Drone > Beth'tilac
        if (Creature* drone = me->FindNearestCreature(NPC_CINDERWEB_DRONE, 100.0f, true))
            return drone;

        if (_instance)
            if (Creature* boss = _instance->GetCreature(DATA_BETHTILAC))
                if (boss->IsAlive())
                    return boss;

        return nullptr;
    }
};

// Web Filament AI
struct npc_web_filament : public NullCreatureAI
{
    npc_web_filament(Creature* creature) : NullCreatureAI(creature) { }

    void OnSpellClick(Unit* clicker, bool& /*result*/) override
    {
        if (Player* player = clicker->ToPlayer())
        {
            // Teleport player to web level
            Position pos = player->GetPosition();
            pos.m_positionZ = WEB_LEVEL_Z;
            player->NearTeleportTo(pos.GetPositionX(), pos.GetPositionY(),
                                  pos.GetPositionZ(), pos.GetOrientation());

            // Despawn filament (single use)
            me->DespawnOrUnsummon(100ms);
        }
    }
};

} // namespace Firelands::Bethtilac

void AddSC_boss_bethtilac()
{
    using namespace Firelands;
    using namespace Firelands::Bethtilac;
    RegisterFirelandsCreatureAI(boss_bethtilac);
    RegisterFirelandsCreatureAI(npc_cinderweb_spinner);
    RegisterFirelandsCreatureAI(npc_cinderweb_drone);
    RegisterFirelandsCreatureAI(npc_cinderweb_spiderling);
    RegisterFirelandsCreatureAI(npc_web_filament);
}
