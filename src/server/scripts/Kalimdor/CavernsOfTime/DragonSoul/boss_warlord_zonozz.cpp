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

#include "dragon_soul.h"
#include "Containers.h"
#include "InstanceScript.h"
#include "Map.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "TaskScheduler.h"
#include "TemporarySummon.h"

namespace DragonSoul::Zonozz
{
enum Texts
{
    // Each Shath'Yar yell is followed by its translation, whispered to the raid
    SAY_INTRO         = 0,  // "Vwyq agth sshoq'meg N'Zoth vra zz shfk qwor ga'halahs agthu. Uulg'ma, ag qam."
    SAY_AGGRO         = 2,  // "Zzof Shuul'wah. Thoq fssh N'Zoth!"
    SAY_VOID          = 4,  // "Gul'kafh an'qov N'Zoth." - Void of the Unmaking
    SAY_PHASE         = 6,  // "N'Zoth ga zyqtahg iilth." - Black Blood of Go'rath
    SAY_SHADOWS_FIRST = 8,  // three paired variants: yells 8/10/12, whispers 9/11/13
    SAY_SLAY_FIRST    = 14, // three paired variants: yells 14/16/18, whispers 15/17/19
    SAY_DEATH         = 20  // "Uovssh thyzz... qwaz..."
};

enum Spells
{
    // Warlord Zon'ozz
    SPELL_SUMMON_VOID_OF_THE_UNMAKING = 103571, // instant, summons 55334 in front of the caster
    SPELL_FOCUSED_ANGER               = 104543, // forks: 109409 / 109410 / 109411 (stacking +10% damage/attack speed)
    SPELL_PSYCHIC_DRAIN               = 104322, // forks: 104606 / 104607 / 104608 (cone health leech, heals 10x)
    SPELL_DISRUPTING_SHADOWS          = 103434, // forks: 104599 / 104600 / 104601 (20s magic DoT)
    SPELL_DISRUPTING_SHADOWS_KNOCK    = 103948, // forks: 108342 / 108343 / 108344 (fired on dispel; AoE on heroic)
    SPELL_TANTRUM                     = 103953, // detonation package: strips Focused Anger, 6x shadow burst
    SPELL_DARKNESS                    = 109413, // Phase Two marker visual
    SPELL_BLACK_BLOOD_OF_GORATH       = 104378, // fork: 110322 (30s raid-wide 1s pulse, all modes)
    SPELL_BLACK_BLOOD_ERUPTION        = 104377, // fork: 110306 (heroic: stacks with each living Eye of Go'rath)
    SPELL_BERSERK                     = 26662,

    // Void of the Unmaking
    SPELL_VOID_SPAWN_VISUAL           = 105336, // birth animation
    SPELL_VOID_VISUAL                 = 103627, // forks: 110305 / 110304 / 110303 - finite duration, re-applied
    SPELL_VOID_DIFFUSION              = 103527, // forks: 104605 / 108345 / 108346 - split damage on player
                                                // collision; natively stacks 106836 (+20% damage, +20% scale) on the ball
    SPELL_VOID_DIFFUSION_DEBUFF       = 104031, // boss debuff: +5% damage taken per stack (stack = bounce count)

    // Tentacles of Go'rath (heroic)
    SPELL_SHADOW_GAZE                 = 104347, // forks: 104602 / 104603 / 104604 - Eye ranged attack
    SPELL_WILD_FLAIL                  = 109199, // fork: 110308 - Flail PBAoE damage + knockback
    SPELL_OOZE_SPIT                   = 109396  // Claw/Flail ranged attack when no melee target
};

enum Events
{
    EVENT_SUMMON_VOID = 1,
    EVENT_FOCUSED_ANGER,
    EVENT_PSYCHIC_DRAIN,
    EVENT_DISRUPTING_SHADOWS,
    EVENT_BLACK_BLOOD_END,
    EVENT_BERSERK
};

enum Actions
{
    ACTION_VOID_DETONATED = 1
};

enum Phases
{
    PHASE_COMBAT = 1,
    PHASE_BLACK_BLOOD
};

enum Points
{
    POINT_TRAVEL = 1
};

namespace
{
// The sphere crosses the room at a constant ~6.5 yd/s (sniffed splines),
// driven in short straight segments with manual collision checks
constexpr float SphereSpeed          = 6.5f;
constexpr float SphereSegmentLength  = 12.0f;
constexpr float SphereHoverHeight    = 2.0f;
constexpr float PlayerHitRadius      = 5.0f;
constexpr float BossHitRadius        = 7.0f;
constexpr uint32 SphereSpawnGraceMs  = 3000; // don't pop on the melee stack while spawning
constexpr uint32 SphereBounceGraceMs = 1500; // no machine-gun rebounces off the soaking group
constexpr uint32 CollisionPeriodMs   = 250;
constexpr uint32 PingPongBounces     = 10;   // Ping Pong Champion (achievement 6128)
constexpr uint8 VoidDiffusionMaxStacks = 80;

// The room is a rough disc; the sphere reflects off its edge without damage
Position const RoomCenter = { -1765.0f, -1915.0f, -226.0f };
constexpr float RoomRadius = 60.0f;

// Tentacle spawn rings (25 heroic sniff). 10 heroic uses the first entries.
Position const EyePositions[] =
{
    { -1792.20f, -1988.63f, -221.37f }, { -1801.84f, -1851.69f, -221.44f },
    { -1702.57f, -1884.71f, -221.51f }, { -1694.25f, -1943.25f, -221.13f },
    { -1834.55f, -1952.28f, -221.38f }, { -1734.35f, -1983.18f, -221.45f },
    { -1839.37f, -1895.09f, -221.38f }, { -1745.46f, -1847.31f, -221.44f }
};

Position const FlailPositions[] =
{
    { -1731.46f, -1875.72f, -225.21f }, { -1765.91f, -1973.53f, -225.12f },
    { -1802.75f, -1957.85f, -225.29f }, { -1804.93f, -1889.42f, -225.39f }
};

Position const ClawPositions[] =
{
    { -1725.00f, -1914.87f, -226.26f }, { -1798.68f, -1926.58f, -226.18f }
};

bool IsHeroicZonozz(Map const* map)
{
    return map->GetDifficulty() == RAID_DIFFICULTY_10MAN_HEROIC
        || map->GetDifficulty() == RAID_DIFFICULTY_25MAN_HEROIC;
}

bool IsLFR(InstanceScript const* instance)
{
    return instance && instance->IsLFR();
}

// Raid Finder tuning: reduced health (from the LFR stats templates) and
// reduced outgoing damage, applied on top of the 25 player normal profile
void ApplyLFRHealth(Creature* creature, InstanceScript const* instance, uint32 statsEntry)
{
    if (!IsLFR(instance))
        return;

    CreatureTemplate const* lfrStats = sObjectMgr->GetCreatureTemplate(statsEntry);
    if (!lfrStats)
        return;

    if (CreatureBaseStats const* baseStats = sObjectMgr->GetCreatureBaseStats(creature->getLevel(), lfrStats->unit_class))
    {
        creature->SetMaxHealth(baseStats->GenerateHealth(lfrStats));
        creature->SetFullHealth();
    }
}

void ApplyLFRDamageReduction(InstanceScript const* instance, uint32& damage)
{
    if (IsLFR(instance))
        damage = damage * LFR_DAMAGE_PCT / 100;
}
}

struct boss_warlord_zonozz : public BossAI
{
    boss_warlord_zonozz(Creature* creature) : BossAI(creature, DATA_WARLORD_ZONOZZ) { }

    void Reset() override
    {
        _Reset();
        _scheduler.CancelAll();
        me->RemoveAurasDueToSpell(SPELL_VOID_DIFFUSION_DEBUFF);
        me->RemoveAurasDueToSpell(sSpellMgr->GetSpellIdForDifficulty(SPELL_FOCUSED_ANGER, me));
        me->SetReactState(REACT_AGGRESSIVE);
        ApplyLFRHealth(me, instance, NPC_ZONOZZ_LFR_STATS);
    }

    void DamageDealt(Unit* /*victim*/, uint32& damage, DamageEffectType /*damageType*/) override
    {
        // Covers melee, Psychic Drain (the 10x leech heal shrinks with it),
        // Disrupting Shadows ticks and the script-applied Black Blood pulses
        ApplyLFRDamageReduction(instance, damage);
    }

    void MoveInLineOfSight(Unit* who) override
    {
        if (!_introDone && who->GetTypeId() == TYPEID_PLAYER && !who->ToPlayer()->IsGameMaster() && me->IsWithinDistInMap(who, 70.0f))
        {
            _introDone = true;
            TalkPairToRaid(SAY_INTRO);
        }

        BossAI::MoveInLineOfSight(who);
    }

    void JustEngagedWith(Unit* who) override
    {
        BossAI::JustEngagedWith(who);
        TalkPairToRaid(SAY_AGGRO);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

        events.SetPhase(PHASE_COMBAT);
        events.ScheduleEvent(EVENT_SUMMON_VOID, 5500ms, 0, PHASE_COMBAT);
        events.ScheduleEvent(EVENT_FOCUSED_ANGER, 10500ms, 0, PHASE_COMBAT);
        events.ScheduleEvent(EVENT_PSYCHIC_DRAIN, 13s, 0, PHASE_COMBAT);
        events.ScheduleEvent(EVENT_DISRUPTING_SHADOWS, 25s, 0, PHASE_COMBAT);
        if (!IsLFR(instance))
            events.ScheduleEvent(EVENT_BERSERK, 6min);
    }

    void JustSummoned(Creature* summon) override
    {
        BossAI::JustSummoned(summon);
        if (summon->GetEntry() == NPC_VOID_OF_THE_UNMAKING)
            _sphereGuid = summon->GetGUID();
    }

    void KilledUnit(Unit* victim) override
    {
        if (victim->GetTypeId() == TYPEID_PLAYER && roll_chance_i(50))
            TalkPairToRaid(SAY_SLAY_FIRST + urand(0, 2) * 2);
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
        TalkPairToRaid(SAY_DEATH);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        CleanupEncounter();
    }

    void EnterEvadeMode(EvadeReason /*why*/) override
    {
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        CleanupEncounter();
        _EnterEvadeMode();
        summons.DespawnAll();
        me->GetMotionMaster()->MoveTargetedHome();
        _DespawnAtEvade();
    }

    void DoAction(int32 action) override
    {
        if (action != ACTION_VOID_DETONATED || !events.IsInPhase(PHASE_COMBAT))
            return;

        // ---- Phase Two: the void sphere detonated on the boss ----
        events.SetPhase(PHASE_BLACK_BLOOD);
        events.CancelEvent(EVENT_SUMMON_VOID);
        events.CancelEvent(EVENT_FOCUSED_ANGER);
        events.CancelEvent(EVENT_PSYCHIC_DRAIN);
        events.CancelEvent(EVENT_DISRUPTING_SHADOWS);

        me->InterruptNonMeleeSpells(true);
        me->AttackStop();
        me->SetReactState(REACT_PASSIVE);

        TalkPairToRaid(SAY_PHASE);
        DoCastSelf(SPELL_TANTRUM, true);   // strips Focused Anger, pulses the shadow burst
        DoCastSelf(SPELL_DARKNESS, true);

        StartBlackBlood();
        if (IsHeroicZonozz(me->GetMap()))
            SummonTentacles();

        events.ScheduleEvent(EVENT_BLACK_BLOOD_END, 30s, 0, PHASE_BLACK_BLOOD);
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);

        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_SUMMON_VOID:
                {
                    // Never two spheres at once - shouldn't happen, but a lost
                    // sphere must not soft lock the encounter either
                    if (Creature* sphere = ObjectAccessor::GetCreature(*me, _sphereGuid))
                    {
                        if (sphere->IsAlive())
                        {
                            events.Repeat(10s);
                            break;
                        }
                    }

                    // Launch toward the raid: face a random ranged player
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 60.0f, true))
                        me->SetFacingToObject(target);

                    TalkPairToRaid(SAY_VOID);
                    DoCastSelf(SPELL_SUMMON_VOID_OF_THE_UNMAKING);
                    events.Repeat(90300ms);
                    break;
                }
                case EVENT_FOCUSED_ANGER:
                    DoCastSelf(SPELL_FOCUSED_ANGER, true);
                    events.Repeat(6s);
                    break;
                case EVENT_PSYCHIC_DRAIN:
                    // Cone on the tank's direction - hold the boss faced away
                    DoCastVictim(SPELL_PSYCHIC_DRAIN);
                    events.Repeat(20s, 25s);
                    break;
                case EVENT_DISRUPTING_SHADOWS:
                    TalkPairToRaid(SAY_SHADOWS_FIRST + urand(0, 2) * 2);
                    DoCastAOE(SPELL_DISRUPTING_SHADOWS, true);
                    events.Repeat(25s, 30s);
                    break;
                case EVENT_BLACK_BLOOD_END:
                    EndBlackBlood();
                    break;
                case EVENT_BERSERK:
                    DoCastSelf(SPELL_BERSERK, true);
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
    // Yells in Shath'Yar, then whispers the translation to every raid member
    void TalkPairToRaid(uint32 yellGroup)
    {
        Talk(yellGroup);
        _scheduler.Schedule(3s, [this, yellGroup](TaskContext /*context*/)
        {
            for (MapReference const& ref : me->GetMap()->GetPlayers())
                if (Player* player = ref.GetSource())
                    Talk(yellGroup + 1, player);
        });
    }

    // The 30 second flood: everyone is pulsed with Black Blood of Go'rath
    // regardless of position. On heroic, the Eyes of Go'rath add a stacking
    // eruption that weakens as the raid kills them.
    void StartBlackBlood()
    {
        uint32 const bloodSpellId = sSpellMgr->GetSpellIdForDifficulty(SPELL_BLACK_BLOOD_OF_GORATH, me);
        uint32 const eruptionSpellId = sSpellMgr->GetSpellIdForDifficulty(SPELL_BLACK_BLOOD_ERUPTION, me);
        bool const heroic = IsHeroicZonozz(me->GetMap());

        _scheduler.Schedule(1ms, [this, bloodSpellId, eruptionSpellId, heroic](TaskContext context)
        {
            uint8 eyes = 0;
            if (heroic)
                for (ObjectGuid guid : summons)
                    if (Creature* summon = ObjectAccessor::GetCreature(*me, guid))
                        if (summon->GetEntry() == NPC_EYE_OF_GORATH && summon->IsAlive())
                            ++eyes;

            for (MapReference const& ref : me->GetMap()->GetPlayers())
            {
                Player* player = ref.GetSource();
                if (!player || !player->IsAlive() || player->IsGameMaster())
                    continue;

                // The aura's own 30s duration ends the phase for latecomers too
                if (!player->HasAura(bloodSpellId))
                    me->AddAura(bloodSpellId, player);

                if (!heroic)
                    continue;

                if (!eyes)
                    player->RemoveAurasDueToSpell(eruptionSpellId);
                else if (Aura* eruption = player->GetAura(eruptionSpellId))
                    eruption->SetStackAmount(eyes);
                else if (Aura* eruption = me->AddAura(eruptionSpellId, player))
                    eruption->SetStackAmount(eyes);
            }

            context.Repeat(2s);
        });
    }

    void EndBlackBlood()
    {
        _scheduler.CancelAll();
        RemoveBlackBloodFromPlayers();
        DespawnTentacles();

        me->SetReactState(REACT_AGGRESSIVE);
        if (Unit* victim = me->GetVictim())
            AttackStart(victim);

        events.SetPhase(PHASE_COMBAT);
        // Retail cadence (DBM): the next sphere comes noticeably sooner on heroic
        events.ScheduleEvent(EVENT_SUMMON_VOID, IsHeroicZonozz(me->GetMap()) ? 15300ms : 24300ms, 0, PHASE_COMBAT);
        events.ScheduleEvent(EVENT_FOCUSED_ANGER, 6s, 0, PHASE_COMBAT);
        events.ScheduleEvent(EVENT_PSYCHIC_DRAIN, 13s, 0, PHASE_COMBAT);
        events.ScheduleEvent(EVENT_DISRUPTING_SHADOWS, 6s, 0, PHASE_COMBAT);
    }

    void SummonTentacles()
    {
        bool const is25 = me->GetMap()->Is25ManRaid();

        auto summonRing = [this](Position const* positions, uint8 count)
        {
            for (uint8 i = 0; i < count; ++i)
            {
                Position pos = positions[i];
                pos.SetOrientation(pos.GetAngle(RoomCenter));
                uint32 entry = positions == EyePositions ? NPC_EYE_OF_GORATH
                    : positions == FlailPositions ? NPC_FLAIL_OF_GORATH : NPC_CLAW_OF_GORATH;
                me->SummonCreature(entry, pos, TEMPSUMMON_MANUAL_DESPAWN);
            }
        };

        summonRing(EyePositions, is25 ? 8 : 5);
        summonRing(FlailPositions, is25 ? 4 : 1);
        summonRing(ClawPositions, is25 ? 2 : 1);
    }

    void DespawnTentacles()
    {
        // Survivors sink back into the blood - they do not persist into Phase One
        for (ObjectGuid guid : summons)
            if (Creature* summon = ObjectAccessor::GetCreature(*me, guid))
                switch (summon->GetEntry())
                {
                    case NPC_EYE_OF_GORATH:
                    case NPC_FLAIL_OF_GORATH:
                    case NPC_CLAW_OF_GORATH:
                        summon->DespawnOrUnsummon(2s);
                        break;
                    default:
                        break;
                }
    }

    void RemoveBlackBloodFromPlayers()
    {
        instance->DoRemoveAurasDueToSpellOnPlayers(sSpellMgr->GetSpellIdForDifficulty(SPELL_BLACK_BLOOD_OF_GORATH, me));
        instance->DoRemoveAurasDueToSpellOnPlayers(sSpellMgr->GetSpellIdForDifficulty(SPELL_BLACK_BLOOD_ERUPTION, me));
    }

    void CleanupEncounter()
    {
        _scheduler.CancelAll();
        RemoveBlackBloodFromPlayers();
    }

    TaskScheduler _scheduler;
    ObjectGuid _sphereGuid;
    bool _introDone = false;
};

struct npc_void_of_the_unmaking : public ScriptedAI
{
    npc_void_of_the_unmaking(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript())
    {
        SetCombatMovement(false);
        me->SetReactState(REACT_PASSIVE);
    }

    void IsSummonedBy(Unit* summoner) override
    {
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        me->SetDisableGravity(true);

        DoCastSelf(SPELL_VOID_SPAWN_VISUAL, true);
        DoCastSelf(SPELL_VOID_VISUAL, true);
        // The travel visual has a finite duration - keep it alive
        _scheduler.Schedule(30s, [this](TaskContext context)
        {
            DoCastSelf(SPELL_VOID_VISUAL, true);
            context.Repeat(30s);
        });

        // Travels in the direction the boss faced at the summon
        _angle = summoner->GetOrientation();
        LaunchSegment();
    }

    void DamageDealt(Unit* /*victim*/, uint32& damage, DamageEffectType /*damageType*/) override
    {
        ApplyLFRDamageReduction(_instance, damage);
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != POINT_MOTION_TYPE || pointId != POINT_TRAVEL || _detonated)
            return;

        LaunchSegment(); // the sphere never idles
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);

        if (_detonated)
            return;

        if (_graceTimer > diff)
            _graceTimer -= diff;
        else
            _graceTimer = 0;

        if (_checkTimer > diff)
        {
            _checkTimer -= diff;
            return;
        }
        _checkTimer = CollisionPeriodMs;

        CheckCollisions();
    }

private:
    void LaunchSegment()
    {
        float x = me->GetPositionX() + std::cos(_angle) * SphereSegmentLength;
        float y = me->GetPositionY() + std::sin(_angle) * SphereSegmentLength;
        ReflectOffWall(x, y);

        // The room floor slopes - the sphere hovers a fixed height above it
        float z = me->GetMapHeight(x, y, me->GetPositionZ());
        if (z > INVALID_HEIGHT)
            z += SphereHoverHeight;
        else
            z = me->GetPositionZ();

        me->GetMotionMaster()->MovePoint(POINT_TRAVEL, x, y, z, false, SphereSpeed);
    }

    // Reaching the room edge reflects the sphere without damage or a stack
    void ReflectOffWall(float& x, float& y)
    {
        if (RoomCenter.GetExactDist2d(x, y) <= RoomRadius)
            return;

        float normalX = me->GetPositionX() - RoomCenter.GetPositionX();
        float normalY = me->GetPositionY() - RoomCenter.GetPositionY();
        float const length = std::sqrt(normalX * normalX + normalY * normalY);
        if (length > 0.01f)
        {
            normalX /= length;
            normalY /= length;
            float const dirX = std::cos(_angle);
            float const dirY = std::sin(_angle);
            float const dot = dirX * normalX + dirY * normalY;
            _angle = Position::NormalizeOrientation(std::atan2(dirY - 2.0f * dot * normalY, dirX - 2.0f * dot * normalX));
        }
        else
            _angle = Position::NormalizeOrientation(_angle + float(M_PI));

        x = me->GetPositionX() + std::cos(_angle) * SphereSegmentLength;
        y = me->GetPositionY() + std::sin(_angle) * SphereSegmentLength;

        // Corner case (spawned outside the disc): head back to the center
        if (RoomCenter.GetExactDist2d(x, y) > RoomRadius + SphereSegmentLength)
        {
            _angle = me->GetAngle(&RoomCenter);
            x = me->GetPositionX() + std::cos(_angle) * SphereSegmentLength;
            y = me->GetPositionY() + std::sin(_angle) * SphereSegmentLength;
        }
    }

    void CheckCollisions()
    {
        // Boss first: a detonation beats a simultaneous player bounce
        if (Creature* zonozz = _instance->GetCreature(DATA_WARLORD_ZONOZZ))
        {
            if (zonozz->IsAlive() && zonozz->IsInCombat() && me->GetExactDist2d(zonozz) < BossHitRadius)
            {
                Detonate(zonozz);
                return;
            }
        }

        if (_graceTimer)
            return;

        std::list<Player*> players;
        me->GetPlayerListInGrid(players, PlayerHitRadius);
        players.remove_if([](Player* player) { return !player->IsAlive() || player->IsGameMaster(); });
        if (players.empty())
            return;

        Bounce();
    }

    void Bounce()
    {
        // Split shadow damage on everyone packed around the impact; the spell
        // also stacks the sphere's own +20% damage / +20% scale aura (106836)
        DoCastAOE(SPELL_VOID_DIFFUSION, true);
        ++_bounces;

        // Straight reflection back the way it came
        _angle = Position::NormalizeOrientation(_angle + float(M_PI));
        _graceTimer = SphereBounceGraceMs;
        me->StopMoving();
        LaunchSegment();
    }

    void Detonate(Creature* zonozz)
    {
        _detonated = true;

        // The boss takes +5% damage per bounce the raid fed the sphere
        if (_bounces)
            if (Aura* debuff = zonozz->AddAura(SPELL_VOID_DIFFUSION_DEBUFF, zonozz))
                debuff->SetStackAmount(uint8(std::min<uint32>(_bounces, VoidDiffusionMaxStacks)));

        if (_bounces >= PingPongBounces)
            _instance->SetData(DATA_ZONOZZ_PING_PONG, 1);

        if (zonozz->IsAIEnabled())
            zonozz->AI()->DoAction(ACTION_VOID_DETONATED);

        me->StopMoving();
        me->DespawnOrUnsummon(2s); // brief linger for the detonation visual
    }

    InstanceScript* _instance;
    TaskScheduler _scheduler;
    float _angle = 0.0f;
    uint32 _bounces = 0;
    uint32 _graceTimer = SphereSpawnGraceMs;
    uint32 _checkTimer = CollisionPeriodMs;
    bool _detonated = false;
};

struct npc_zonozz_eye_of_gorath : public ScriptedAI
{
    npc_zonozz_eye_of_gorath(Creature* creature) : ScriptedAI(creature)
    {
        SetCombatMovement(false);
        me->SetReactState(REACT_PASSIVE);
    }

    void JustAppeared() override
    {
        me->SetControlled(true, UNIT_STATE_ROOT);
        DoZoneInCombat();

        _scheduler.Schedule(2s, 3s, [this](TaskContext context)
        {
            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 150.0f, true))
                DoCast(target, SPELL_SHADOW_GAZE);
            context.Repeat(2s, 3s);
        });
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

private:
    TaskScheduler _scheduler;
};

struct npc_zonozz_claw_of_gorath : public ScriptedAI
{
    npc_zonozz_claw_of_gorath(Creature* creature) : ScriptedAI(creature)
    {
        SetCombatMovement(false);
    }

    void JustAppeared() override
    {
        me->SetControlled(true, UNIT_STATE_ROOT);
        DoZoneInCombat();

        // Turret behavior: no melee target in reach means ranged spit
        _scheduler.Schedule(5s, [this](TaskContext context)
        {
            Unit* victim = me->GetVictim();
            if (!victim || !me->IsWithinMeleeRange(victim))
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 150.0f, true))
                    DoCast(target, SPELL_OOZE_SPIT);
            context.Repeat(5s);
        });
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
};

struct npc_zonozz_flail_of_gorath : public ScriptedAI
{
    npc_zonozz_flail_of_gorath(Creature* creature) : ScriptedAI(creature)
    {
        SetCombatMovement(false);
    }

    void JustAppeared() override
    {
        me->SetControlled(true, UNIT_STATE_ROOT);
        DoZoneInCombat();

        _scheduler.Schedule(8s, 12s, [this](TaskContext context)
        {
            DoCastAOE(SPELL_WILD_FLAIL);
            context.Repeat(8s, 12s);
        });

        _scheduler.Schedule(5s, [this](TaskContext context)
        {
            Unit* victim = me->GetVictim();
            if (!victim || !me->IsWithinMeleeRange(victim))
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 150.0f, true))
                    DoCast(target, SPELL_OOZE_SPIT);
            context.Repeat(5s);
        });
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
};

// 103434, 104599, 104600, 104601 - Disrupting Shadows
class spell_zonozz_disrupting_shadows : public SpellScript
{
    void FilterTargets(std::list<WorldObject*>& targets)
    {
        Unit* caster = GetCaster();

        // Retail avoided the active tank
        if (Unit* victim = caster->GetVictim())
            targets.remove(victim);

        uint32 count;
        if (IsLFR(caster->GetInstanceScript()))
            count = 2;
        else if (caster->GetMap()->Is25ManRaid())
            count = IsHeroicZonozz(caster->GetMap()) ? 7 : 5;
        else
            count = IsHeroicZonozz(caster->GetMap()) ? 3 : 2;

        if (targets.size() > count)
            Trinity::Containers::RandomResize(targets, count);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect.Register(&spell_zonozz_disrupting_shadows::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
    }
};

// 103434, 104599, 104600, 104601 - Disrupting Shadows (aura)
class spell_zonozz_disrupting_shadows_aura : public AuraScript
{
    void HandleDispel(DispelInfo* /*dispelInfo*/)
    {
        // Dispelling detonates the shadows: damage and a knockback that can
        // throw the target into (or out of) the void sphere's path. The
        // player self-cast resolves the heroic AoE forks by map difficulty.
        if (Unit* target = GetUnitOwner())
            target->CastSpell(target, SPELL_DISRUPTING_SHADOWS_KNOCK, true);
    }

    void Register() override
    {
        AfterDispel.Register(&spell_zonozz_disrupting_shadows_aura::HandleDispel);
    }
};

// 103948, 108342, 108343, 108344 - Disrupting Shadows (dispel detonation)
class spell_zonozz_disrupting_shadows_knockback : public SpellScript
{
    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        // Cast by the dispelled player - the boss damage hook can't cover it
        uint32 damage = GetHitDamage();
        ApplyLFRDamageReduction(GetCaster()->GetInstanceScript(), damage);
        SetHitDamage(damage);
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_zonozz_disrupting_shadows_knockback::HandleDamage, EFFECT_FIRST_FOUND, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};
}

void AddSC_boss_warlord_zonozz()
{
    using namespace DragonSoul;
    using namespace DragonSoul::Zonozz;

    RegisterDragonSoulCreatureAI(boss_warlord_zonozz);
    RegisterDragonSoulCreatureAI(npc_void_of_the_unmaking);
    RegisterDragonSoulCreatureAI(npc_zonozz_eye_of_gorath);
    RegisterDragonSoulCreatureAI(npc_zonozz_claw_of_gorath);
    RegisterDragonSoulCreatureAI(npc_zonozz_flail_of_gorath);

    RegisterSpellScript(spell_zonozz_disrupting_shadows);
    RegisterSpellScript(spell_zonozz_disrupting_shadows_aura);
    RegisterSpellScript(spell_zonozz_disrupting_shadows_knockback);
}
