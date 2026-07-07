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
 * Warmaster Blackhorn, sixth encounter of Dragon Soul (4.3.4).
 *
 * The raid boards the parked Skyfire beside the Wyrmrest summit and speaks to
 * the sky captain; the "launch" teleports everyone to a second Skyfire far off
 * the coast (the flight arena) where the actual fight happens on static
 * geometry with a skybox illusion of motion.
 *
 * Phase One: Blackhorn circles overhead on Goriona while three waves of two
 * Twilight Assault Drakes drop Twilight Elites on deck and bombard the ship.
 * The raid must soak Twilight Barrage and Twilight Onslaught or the Skyfire
 * (a creature health proxy shown in the encounter frame) loses integrity.
 * Twilight Sappers try to reach the engine door (never in Raid Finder).
 * Phase Two starts when the sixth drake dies: the ship's cannons drive
 * Goriona off with an artillery barrage, Blackhorn jumps down and fights;
 * Goriona strafes the deck with Twilight Flames, lands at 80% on heroic
 * (Twilight Breath + Consuming Shroud) and flees at 20% on all difficulties.
 * Heroic: Blackhorn siphons Goriona's vitality if she is still up at his 20%.
 *
 * Timers follow the 4.3.4 Deadly Boss Mods module; spell damage rides the
 * client-native SpellDifficulty forks wherever they exist.
 */

#include "Containers.h"
#include "Creature.h"
#include "CreatureAIImpl.h"
#include "InstanceScript.h"
#include "Map.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "TaskScheduler.h"
#include "TemporarySummon.h"
#include "Vehicle.h"
#include "dragon_soul.h"

namespace DragonSoul::Blackhorn
{
enum Texts
{
    // Warmaster Blackhorn (56427)
    SAY_INTRO             = 0, // "Hah! I was hoping you'd make it this far..."
    SAY_AGGRO             = 1, // "You won't get near the Master. Dragonriders, attack!"
    SAY_GORIONA           = 2, // "Goriona! Give them hell!"
    SAY_PHASE_TWO         = 3, // "Looks like I'm doing this myself. Good!"
    SAY_SHOCKWAVE         = 4, // "Mess with the bull...."
    SAY_FLAVOR            = 5, // "How's THIS?" / "COME ON!"
    SAY_SLAY              = 6, // "Down you go!" / "Get up! Oh... weakling!"
    SAY_BERSERK           = 7, // "We're flying a little too close..."
    SAY_DEATH             = 8, // "Well... done, heh...."
    EMOTE_SIPHON          = 9, // "%s siphons vitality from Goriona..."

    // Goriona (56781)
    EMOTE_ONSLAUGHT       = 0, // "%s prepares to unleash a Twilight Onslaught!"
    EMOTE_BROADSIDE       = 1, // "%s fires a Broadside at the Skyfire!"
    EMOTE_RETREAT         = 2, // "%s screeches in pain and retreats into the swirling clouds!"

    // Sky Captain Swayze (55870) / Ka'anu Reevs (55891)
    SAY_CAPTAIN_WELCOME   = 0, // "Welcome aboard the Skyfire...."
    SAY_CAPTAIN_LAUNCH    = 1, // "All ahead full...."
    SAY_CAPTAIN_ENGINES   = 2, // "Our engines are damaged! We're sitting ducks up here!"
    SAY_CAPTAIN_STATIONS  = 3, // "All hands to battle stations...."
    SAY_CAPTAIN_HARPOON   = 4, // "Concentrate everything on the armored drake!"
    SAY_CAPTAIN_SAPPER    = 5, // "An enemy sapper's breached the engine room!"
    SAY_CAPTAIN_SHIP_LOW  = 6, // "The Skyfire can't take much more of this!"
    SAY_CAPTAIN_ABANDON   = 7, // "We're going down. Abandon the ship!"
    SAY_CAPTAIN_OUTRO     = 8, // "The engines are back online...."
    SAY_CAPTAIN_SPINE     = 9, // "The plates! He's coming apart! Tear up the plates...."

    // The Skyfire (56598)
    EMOTE_DECK_FIRE       = 0, // "Structural damage to the Skyfire triggers a sudden Deck Fire!"

    // Twilight Infiltrator (56922)
    EMOTE_SAPPER_DROP     = 0  // "A drake swoops down to drop a Twilight Sapper onto the deck!"
};

enum Spells
{
    // Goriona - Phase One
    SPELL_TWILIGHT_ONSLAUGHT        = 107588, // 7s cast, missile at the marker -> 106401
    SPELL_TWILIGHT_ONSLAUGHT_DMG    = 106401, // forks 108862/109226/109227, split + ship share
    SPELL_TWILIGHT_ONSLAUGHT_SHIP   = 107589, // BP=1 combat log helper at the ship
    SPELL_ONSLAUGHT_TARGET_AURA     = 107927, // marker state aura (creature_template_addon)
    SPELL_BROADSIDE                 = 110153, // heroic: dummy at the ship
    SPELL_BROADSIDE_DMG             = 110157, // BP=20 (percent of remaining, scripted)

    // Assault Drakes
    SPELL_TWILIGHT_BARRAGE          = 107286, // missile at a deck position -> 107439
    SPELL_TWILIGHT_BARRAGE_DMG      = 107439, // forks 109203/109204/109205 (heroic: +50% shadow taken)
    SPELL_TWILIGHT_BARRAGE_SHIP     = 107501, // BP=1 combat log helper at the ship

    // Skyfire crew
    SPELL_HARPOON                   = 108038, // gun -> drake tether (periodic dummy aura)
    SPELL_RELOADING                 = 108039, // 10s reload cast
    SPELL_HEAVY_SLUG                = 108010, // cannon flavor volleys
    SPELL_ARTILLERY_BARRAGE         = 108040, // P2 transition: cannons drive Goriona off
    SPELL_ENGINE_SOUND              = 109654,
    SPELL_GAINING_SPEED             = 107514,

    // Twilight Elites
    SPELL_DEGENERATION              = 107558, // Dreadblade frontal + DoT, forks 108861/109207/109208
    SPELL_BRUTAL_STRIKE             = 107567, // Slayer strike + bleed, forks 109209/109210/109211
    SPELL_BLADE_RUSH                = 107594, // charge at a ranged player -> 107595 line damage

    // Sappers
    SPELL_SMOKE_BOMB                = 107752,
    SPELL_SHADOWCLOAK               = 110231,
    SPELL_DETONATE                  = 107518, // AoE + effect 165 (20% ship, routed via proxy)

    // Blackhorn - Phase Two
    SPELL_DEVASTATE                 = 108042, // triggers Sunder Armor 108043
    SPELL_DISRUPTING_ROAR           = 108044, // forks 109228/109229/109230, interrupt <= 10 yd
    SPELL_SHOCKWAVE                 = 110137, // aimed at a player's position -> 108046
    SPELL_SHOCKWAVE_DMG             = 108046, // cone damage + stun (no forks)
    SPELL_VENGEANCE                 = 108045, // +1% damage per 1% missing health
    SPELL_SIPHON_VITALITY           = 110312, // heroic: drains Goriona
    SPELL_BERSERK                   = 26662,

    // Goriona - Phase Two
    SPELL_TWILIGHT_FLAMES_MISSILE   = 108050, // at a random player -> 108051
    SPELL_TWILIGHT_FLAMES_DMG       = 108051, // forks 109216/109217/109218; summons patch 57268
    SPELL_TWILIGHT_FLAMES_PERIODIC  = 108053, // patch self aura, 1s ticks of 108076
    SPELL_TWILIGHT_FLAMES_TICK      = 108076, // forks 109222/109223/109224
    SPELL_TWILIGHT_BREATH           = 110212, // heroic ground: fork 110213 (25H)
    SPELL_CONSUMING_SHROUD          = 110214, // heroic ground: fork 110598 (25H), heal absorb
    SPELL_CONSUMING_SHROUD_DMG      = 110215, // mirror damage (BP scripted)

    // Ship
    SPELL_DECK_FIRE_VISUAL          = 109445,
    SPELL_DECK_FIRE_DMG             = 110095, // ~42.7k, scripted 1s ticks on players in the fire
    SPELL_ENGINE_FIRE               = 107799,
    SPELL_MASSIVE_EXPLOSION         = 108132, // ship destroyed
    SPELL_TELEPORT_TO_GUNSHIP       = 108263  // spell_target_position -> flight arena deck
};

enum Actions
{
    ACTION_START_EVENT = 1,     // captain -> controller
    ACTION_SHIP_DESTROYED,      // ship -> controller
    ACTION_SHIP_INVULNERABLE,   // controller -> ship
    ACTION_SAPPER_HIT,          // detonate script -> ship
    ACTION_ENCOUNTER_FAILED,    // anyone -> controller
    ACTION_ENCOUNTER_DONE,      // Blackhorn -> controller
    ACTION_ENABLE_HARPOON,      // controller -> harpoon guns
    ACTION_DISABLE_HARPOON,     // controller -> harpoon guns
    ACTION_RELOAD,              // harpoon aura -> gun
    ACTION_HARPOONED,           // harpoon aura -> drake
    ACTION_RELEASED,            // harpoon aura -> drake
    ACTION_ONSLAUGHT,           // controller -> Goriona
    ACTION_BROADSIDE,           // controller -> Goriona
    ACTION_PHASE_TWO,           // controller -> Goriona -> Blackhorn
    ACTION_ARTILLERY_BARRAGE,   // controller -> cannons
    ACTION_GORIONA_GONE         // Goriona -> Blackhorn
};

enum DataIds
{
    // ship proxy SetData
    SHIP_DATA_DAMAGE              = 1,
    SHIP_DATA_DAMAGE_PCT_MAX      = 2,
    SHIP_DATA_DAMAGE_PCT_REMAINING= 3,

    // controller SetData
    DATA_CAPTAIN_ENTRY            = 1,
    DATA_WAVE_INDEX               = 2
};

enum GuidIds
{
    GUID_LINK_ELITE      = 1, // controller -> elite: shared health twin
    GUID_HARPOON_GUN     = 2, // harpoon aura -> drake: reeling gun
    GUID_SHROUD_APPLY    = 3, // shroud aura -> Goriona
    GUID_SHROUD_REMOVE   = 4,
    // controller SetGUID: id = wave index (0..2), reported by drakes
    GUID_WAVE_ELITE_BASE = 10
};

enum Points
{
    POINT_DRAKE_DROP = 1,
    POINT_DRAKE_BOMBARD,
    POINT_DRAKE_REELED,
    POINT_RIDER_LAND,
    POINT_SAPPER_LAND,
    POINT_SAPPER_DOOR,
    POINT_INFILTRATOR_DETACH,
    POINT_INFILTRATOR_LEAVE,
    POINT_GORIONA_P1_HOVER,
    POINT_GORIONA_P2_DROP,
    POINT_GORIONA_P2_AIR,
    POINT_GORIONA_LAND,
    POINT_GORIONA_FLEE,
    POINT_BLACKHORN_LAND
};

// Flight arena geometry (sniffed). The deck runs along the X axis, bow facing
// -X (o ~3.11); rails at Y ~-12154 (port) and Y ~-12110 (starboard).
Position const SkyfireProxyPos      = { 13444.9f,  -12133.3f, 151.21f, 3.1147f };
Position const ArenaCaptainPos      = { 13460.63f, -12133.41f, 151.40f, 3.1147f };
Position const HarpoonGunPos[2]     =
{
    { 13430.23f, -12154.51f, 152.12f, 4.6800f }, // port
    { 13432.29f, -12110.63f, 152.12f, 1.5533f }  // starboard
};
Position const CannonPos[4]         =
{
    { 13418.71f, -12153.44f, 152.10f, 4.6800f },
    { 13421.17f, -12110.65f, 152.10f, 1.5533f },
    { 13441.60f, -12155.32f, 152.10f, 4.6800f },
    { 13443.91f, -12110.72f, 152.10f, 1.5533f }
};
Position const DeckhandPos[2]       =
{
    { 13455.0f, -12141.0f, 151.25f, 3.1000f },
    { 13455.0f, -12125.0f, 151.25f, 3.1000f }
};

Position const GorionaSpawnPos      = { 13621.2f, -12100.7f, 170.5f, 3.20f };
Position const GorionaP1HoverPos    = { 13391.8f, -12203.2f, 188.6f, 1.20f };
Position const GorionaP2DropPos     = { 13421.4f, -12130.9f, 182.4f, 0.00f };
Position const GorionaP2AirPos      = { 13408.5f, -12090.6f, 168.5f, 4.30f };
Position const GorionaLandPos       = { 13425.0f, -12128.0f, 150.9f, 5.50f };
Position const GorionaFleePos       = { 13610.0f, -12100.0f, 210.0f, 0.00f };
Position const BlackhornLandPos     = { 13432.9f, -12131.9f, 151.0f, 3.11f };

// drake index 0 = port (56855, Dreadblade rider), 1 = starboard (56587, Slayer)
uint32 const DrakeEntries[2]        = { NPC_TWILIGHT_ASSAULT_DRAKE_P, NPC_TWILIGHT_ASSAULT_DRAKE_S };
Position const DrakeSpawnPos[2]     =
{
    { 13606.5f, -12170.8f, 155.5f, 2.90f },
    { 13623.2f, -12066.9f, 157.9f, 3.60f }
};
Position const DrakeDropPos[2]      =
{
    { 13425.0f, -12145.0f, 161.0f, 1.60f },
    { 13427.0f, -12121.0f, 161.0f, 4.70f }
};
Position const DrakeBombardPos[2]   =
{
    { 13413.0f, -12190.0f, 170.0f, 1.60f },
    { 13417.0f, -12078.0f, 170.0f, 4.70f }
};
Position const DrakeReeledPos[2]    =
{
    { 13430.0f, -12163.0f, 156.0f, 1.60f },
    { 13432.0f, -12102.0f, 156.0f, 4.70f }
};
Position const RiderLandPos[2]      =
{
    { 13424.0f, -12142.0f, 151.2f, 1.60f },
    { 13426.0f, -12124.0f, 151.2f, 4.70f }
};

Position const InfiltratorSpawnPos  = { 13620.0f, -12128.0f, 172.0f, 3.14f };
Position const InfiltratorDetachPos = { 13414.0f, -12142.0f, 167.0f, 3.14f };
Position const InfiltratorLeavePos  = { 13330.0f, -12180.0f, 190.0f, 3.80f };
Position const SapperLandPos        = { 13418.9f, -12148.4f, 151.2f, 0.50f };
Position const SapperDoorPos        = { 13458.0f, -12132.0f, 151.3f, 0.00f }; // stern engine door - tune in walkthrough

// Random Barrage / Onslaught / deck fire destinations, 4+ yd inside the rails
Position const DeckPositions[] =
{
    { 13410.0f, -12144.0f, 151.2f, 0.0f },
    { 13410.0f, -12124.0f, 151.2f, 0.0f },
    { 13420.0f, -12140.0f, 151.2f, 0.0f },
    { 13420.0f, -12126.0f, 151.2f, 0.0f },
    { 13428.0f, -12146.0f, 151.2f, 0.0f },
    { 13430.0f, -12120.0f, 151.2f, 0.0f },
    { 13438.0f, -12138.0f, 151.2f, 0.0f },
    { 13438.0f, -12128.0f, 151.2f, 0.0f },
    { 13446.0f, -12144.0f, 151.2f, 0.0f },
    { 13446.0f, -12122.0f, 151.2f, 0.0f },
    { 13452.0f, -12134.0f, 151.2f, 0.0f },
    { 13414.0f, -12133.0f, 151.2f, 0.0f }
};

constexpr float DeckZ            = 151.2f;
constexpr float DeckFallZ        = 141.0f; // below the deck plane -> overboard
constexpr float ArenaRadius      = 200.0f;

// DBM 4.3.4 timers
constexpr Milliseconds CombatStartDelay   = 20500ms; // launch RP -> pull
constexpr Milliseconds WaveTimes[3]       = { 22800ms, 83800ms, 144800ms };
constexpr Milliseconds OnslaughtFirst     = 41000ms; // cast start; 7s travel = 48s impact
constexpr Milliseconds OnslaughtRepeat    = 35000ms;
constexpr Milliseconds SapperFirst        = 69000ms;
constexpr Milliseconds SapperRepeat       = 40000ms;
constexpr Milliseconds BroadsideFirst     = 57000ms; // heroic only
constexpr Milliseconds BroadsideRepeat    = 90000ms;
constexpr Milliseconds HarpoonAfterWave   = 18000ms;
constexpr Milliseconds BerserkAfterLand   = 240000ms;

struct BlackhornTuning
{
    uint8  ElitesPerDrake;
    uint8  SappersPerDrop;      // 0 = no sappers (Raid Finder)
    uint32 HarpoonHoldMs;       // how long a drake stays reeled in
    bool   GorionaLands;        // heroic: grounded at 80%
    bool   SiphonVitality;      // heroic: Blackhorn drains Goriona at 20%
    bool   Broadside;           // heroic
    bool   DeckFires;           // heroic
    bool   Berserk;             // everything but LFR
};

bool IsLFR(InstanceScript const* instance)
{
    return instance && instance->IsLFR();
}

BlackhornTuning const& GetTuning(InstanceScript const* instance, Map const* map)
{
    static BlackhornTuning const lfr = { 1, 0, 25000, false, false, false, false, false };
    static BlackhornTuning const n10 = { 1, 1, 25000, false, false, false, false, true  };
    static BlackhornTuning const n25 = { 2, 1, 25000, false, false, false, false, true  };
    static BlackhornTuning const h10 = { 1, 1, 20000, true,  true,  true,  true,  true  };
    static BlackhornTuning const h25 = { 2, 1, 20000, true,  true,  true,  true,  true  };

    if (IsLFR(instance))
        return lfr;

    switch (map->GetDifficulty())
    {
        case RAID_DIFFICULTY_25MAN_NORMAL: return n25;
        case RAID_DIFFICULTY_10MAN_HEROIC: return h10;
        case RAID_DIFFICULTY_25MAN_HEROIC: return h25;
        default:                           return n10;
    }
}

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

Position RandomDeckPosition()
{
    Position pos = Trinity::Containers::SelectRandomContainerElement(DeckPositions);
    pos.m_positionX += frand(-3.0f, 3.0f);
    pos.m_positionY += frand(-3.0f, 3.0f);
    return pos;
}

bool IsOnArena(WorldObject const* who)
{
    return who->GetExactDist2d(SkyfireProxyPos.GetPositionX(), SkyfireProxyPos.GetPositionY()) < ArenaRadius;
}

bool IsAnyPlayerAliveOnArena(Map* map)
{
    for (MapReference const& ref : map->GetPlayers())
    {
        Player* player = ref.GetSource();
        if (player && player->IsAlive() && !player->IsGameMaster() && IsOnArena(player))
            return true;
    }
    return false;
}

// 55870 Sky Captain Swayze / 55891 Ka'anu Reevs: gossip start on the parked
// staging ship; the arena copy (a controller summon) only relays mid-fight
// lines via Talk from the controller.
struct npc_ds_skyfire_captain : public ScriptedAI
{
    npc_ds_skyfire_captain(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript()) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        // arena copy: pure speaker during the fight; after the Blackhorn
        // kill it keeps gossip as the Spine of Deathwing launch point
        if (_instance->GetBossState(DATA_WARMASTER_BLACKHORN) != DONE)
            me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        if (Creature* controller = _instance->GetCreature(DATA_GUNSHIP_PURSUIT_CONTROLLER))
            controller->AI()->JustSummoned(me);
    }

    bool GossipSelect(Player* player, uint32 /*menuId*/, uint32 gossipListId) override
    {
        CloseGossipMenuFor(player);

        if (!_instance)
            return true;

        // second menu entry: leap onto Deathwing's back (Spine of Deathwing)
        if (gossipListId == 1)
        {
            StartSpineLaunch(player);
            return true;
        }

        if (_instance->GetBossState(DATA_ULTRAXION) != DONE
            || _instance->GetBossState(DATA_WARMASTER_BLACKHORN) == IN_PROGRESS
            || _instance->GetBossState(DATA_WARMASTER_BLACKHORN) == DONE)
            return true;

        // retail heroic progression gate (sends its own notification)
        if (!_instance->CheckRequiredBosses(DATA_WARMASTER_BLACKHORN, player))
            return true;

        Creature* controller = _instance->GetCreature(DATA_GUNSHIP_PURSUIT_CONTROLLER);
        if (!controller)
            return true;

        // no takebacks once the ship "launches" (both captains stand on the
        // staging deck within a few yards of each other)
        me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        for (uint32 entry : { NPC_SKY_CAPTAIN_SWAYZE, NPC_KAANU_REEVS })
            if (Creature* captain = me->FindNearestCreature(entry, 60.0f))
                captain->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);

        Talk(SAY_CAPTAIN_LAUNCH);
        controller->AI()->SetData(DATA_CAPTAIN_ENTRY, me->GetEntry());

        _scheduler.Schedule(6s, [this](TaskContext /*context*/)
        {
            for (MapReference const& ref : me->GetMap()->GetPlayers())
            {
                Player* raider = ref.GetSource();
                if (raider && raider->IsAlive() && raider->GetExactDist2d(me) < 120.0f)
                    raider->CastSpell(raider, SPELL_TELEPORT_TO_GUNSHIP, true);
            }
        });
        _scheduler.Schedule(8s, [this](TaskContext /*context*/)
        {
            if (Creature* controller = _instance->GetCreature(DATA_GUNSHIP_PURSUIT_CONTROLLER))
                controller->AI()->DoAction(ACTION_START_EVENT);
        });

        return true;
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

private:
    void StartSpineLaunch(Player* player)
    {
        if (_instance->GetBossState(DATA_WARMASTER_BLACKHORN) != DONE || _spineLaunching)
            return;

        EncounterState spineState = _instance->GetBossState(DATA_SPINE_OF_DEATHWING);
        if (spineState == IN_PROGRESS || spineState == DONE)
            return;

        // heroic progression gate (sends its own notification)
        if (!_instance->CheckRequiredBosses(DATA_SPINE_OF_DEATHWING, player))
            return;

        _spineLaunching = true;
        Talk(SAY_CAPTAIN_SPINE);

        // pull the spine grid in so the Deathwing controller exists before
        // the raid lands on his back
        me->GetMap()->LoadGrid(SpineOfDeathwingLandingPos.GetPositionX(), SpineOfDeathwingLandingPos.GetPositionY());

        _scheduler.Schedule(5s, [this](TaskContext /*context*/)
        {
            for (MapReference const& ref : me->GetMap()->GetPlayers())
            {
                Player* raider = ref.GetSource();
                if (raider && raider->IsAlive() && raider->GetExactDist2d(me) < 120.0f)
                    raider->NearTeleportTo(SpineOfDeathwingLandingPos);
            }
        });
        _scheduler.Schedule(8s, [this](TaskContext /*context*/)
        {
            _spineLaunching = false;
            if (Creature* deathwing = _instance->GetCreature(DATA_SPINE_OF_DEATHWING))
                deathwing->AI()->DoAction(ACTION_START_SPINE_ENCOUNTER);
        });
    }

    InstanceScript* _instance;
    TaskScheduler _scheduler;
    bool _spineLaunching = false;
};

// 56599 - Gunship Pursuit Controller: master state machine. Permanent spawn
// beside the staging ship (its grid loads with the raid); everything on the
// flight arena is a summon of this creature.
struct npc_gunship_pursuit_controller : public ScriptedAI
{
    enum Stage : uint8
    {
        STAGE_STAGING = 0,
        STAGE_LAUNCHING,
        STAGE_PHASE_ONE,
        STAGE_PHASE_TWO,
        STAGE_FAILING,
        STAGE_DONE
    };

    static constexpr uint32 TASK_GROUP_PHASE_ONE = 1;
    static constexpr uint32 TASK_GROUP_CHECKS    = 2;

    npc_gunship_pursuit_controller(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript()), _summons(creature) { }

    void Reset() override
    {
        me->SetReactState(REACT_PASSIVE);
        me->setActive(true);

        _scheduler.CancelAll();
        _stage = STAGE_STAGING;
        _drakesDied = 0;
        _wavesSpawned = 0;
        _captainEntry = NPC_SKY_CAPTAIN_SWAYZE;
        _arenaCaptain.Clear();
        for (auto& list : _waveElites)
            list.clear();

        // After a kill (or a restart following one) keep the arena habitable:
        // Spine access and corpse recovery need a deck to stand on.
        _scheduler.Schedule(5s, [this](TaskContext context)
        {
            if (_stage == STAGE_STAGING && _instance->GetBossState(DATA_WARMASTER_BLACKHORN) == DONE)
            {
                _stage = STAGE_DONE;
                BuildArena(false);
            }
            else
                context.Repeat(5s);
        });
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

    void JustSummoned(Creature* summon) override
    {
        _summons.Summon(summon);
    }

    void SetData(uint32 type, uint32 value) override
    {
        if (type == DATA_CAPTAIN_ENTRY)
            _captainEntry = value;
    }

    // Drakes report each dropped elite with id = wave index; the controller
    // cross-links every elite of the wave into one shared health pool.
    void SetGUID(ObjectGuid const& guid, int32 id) override
    {
        uint32 wave = uint32(id);
        if (wave >= 3)
            return;

        Creature* fresh = ObjectAccessor::GetCreature(*me, guid);
        if (!fresh)
            return;

        for (ObjectGuid other : _waveElites[wave])
        {
            if (Creature* twin = ObjectAccessor::GetCreature(*me, other))
            {
                twin->AI()->SetGUID(guid, GUID_LINK_ELITE);
                fresh->AI()->SetGUID(other, GUID_LINK_ELITE);
            }
        }
        _waveElites[wave].push_back(guid);
    }

    void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
    {
        if (_stage != STAGE_PHASE_ONE)
            return;

        if (summon->GetEntry() == NPC_TWILIGHT_ASSAULT_DRAKE_P || summon->GetEntry() == NPC_TWILIGHT_ASSAULT_DRAKE_S)
            if (++_drakesDied >= 6)
                StartPhaseTwo();
    }

    void DoAction(int32 action) override
    {
        switch (action)
        {
            case ACTION_START_EVENT:
                if (_stage == STAGE_STAGING && _instance->GetBossState(DATA_WARMASTER_BLACKHORN) != DONE)
                    StartEvent();
                break;
            case ACTION_SHIP_DESTROYED:
                if (_stage == STAGE_PHASE_ONE)
                    ShipDestroyed();
                break;
            case ACTION_ENCOUNTER_FAILED:
                if (_stage == STAGE_PHASE_ONE || _stage == STAGE_PHASE_TWO || _stage == STAGE_LAUNCHING)
                    FailEncounter();
                break;
            case ACTION_ENCOUNTER_DONE:
                if (_stage == STAGE_PHASE_TWO)
                    FinishEncounter();
                break;
            default:
                break;
        }
    }

private:
    void TalkCaptain(uint8 group)
    {
        if (Creature* captain = ObjectAccessor::GetCreature(*me, _arenaCaptain))
            captain->AI()->Talk(group);
    }

    Creature* GetSummon(uint32 entry, Position const& near) const
    {
        for (ObjectGuid guid : _summons)
            if (Creature* summon = ObjectAccessor::GetCreature(*me, guid))
                if (summon->IsAlive() && summon->GetEntry() == entry && summon->GetExactDist2d(&near) < 10.0f)
                    return summon;
        return nullptr;
    }

    void BuildArena(bool withBoss)
    {
        _summons.DespawnAll();
        _arenaCaptain.Clear();

        if (Creature* skyfire = me->SummonCreature(NPC_THE_SKYFIRE, SkyfireProxyPos, TEMPSUMMON_MANUAL_DESPAWN))
        {
            skyfire->CastSpell(skyfire, SPELL_ENGINE_SOUND, true);
            skyfire->CastSpell(skyfire, SPELL_GAINING_SPEED, true);
        }

        for (Position const& pos : HarpoonGunPos)
            me->SummonCreature(NPC_SKYFIRE_HARPOON_GUN, pos, TEMPSUMMON_MANUAL_DESPAWN);
        for (Position const& pos : CannonPos)
            me->SummonCreature(NPC_SKYFIRE_CANNON, pos, TEMPSUMMON_MANUAL_DESPAWN);
        for (Position const& pos : DeckhandPos)
            me->SummonCreature(NPC_SKYFIRE_DECKHAND, pos, TEMPSUMMON_MANUAL_DESPAWN);

        if (Creature* captain = me->SummonCreature(_captainEntry, ArenaCaptainPos, TEMPSUMMON_MANUAL_DESPAWN))
            _arenaCaptain = captain->GetGUID();

        if (withBoss)
            me->SummonCreature(NPC_GORIONA, GorionaSpawnPos, TEMPSUMMON_MANUAL_DESPAWN);
    }

    void StartEvent()
    {
        _stage = STAGE_LAUNCHING;
        _drakesDied = 0;
        _wavesSpawned = 0;
        for (auto& list : _waveElites)
            list.clear();

        // Goriona's vehicle accessory loads Blackhorn before the boss state
        // flips, so his BossAI reset cannot clobber IN_PROGRESS
        BuildArena(true);
        _instance->SetBossState(DATA_WARMASTER_BLACKHORN, IN_PROGRESS);

        _scheduler.Schedule(6500ms, [this](TaskContext /*context*/)
        {
            if (Creature* blackhorn = _instance->GetCreature(DATA_WARMASTER_BLACKHORN))
                blackhorn->AI()->Talk(SAY_INTRO);
        });
        _scheduler.Schedule(13s, [this](TaskContext /*context*/)
        {
            TalkCaptain(SAY_CAPTAIN_ENGINES);
        });
        _scheduler.Schedule(CombatStartDelay, [this](TaskContext /*context*/)
        {
            StartPhaseOne();
        });
    }

    void StartPhaseOne()
    {
        _stage = STAGE_PHASE_ONE;

        if (Creature* blackhorn = _instance->GetCreature(DATA_WARMASTER_BLACKHORN))
            blackhorn->AI()->Talk(SAY_AGGRO);

        if (Creature* skyfire = _instance->GetCreature(DATA_THE_SKYFIRE))
            _instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, skyfire, 2);

        _scheduler.Schedule(7s, TASK_GROUP_PHASE_ONE, [this](TaskContext /*context*/)
        {
            TalkCaptain(SAY_CAPTAIN_STATIONS);
        });

        // Three fixed waves of two drakes
        for (uint8 wave = 0; wave < 3; ++wave)
            _scheduler.Schedule(WaveTimes[wave], TASK_GROUP_PHASE_ONE, [this, wave](TaskContext /*context*/)
            {
                SpawnWave(wave);
            });

        // Twilight Onslaught: cast start 41s, impact at 48s, repeat 35s
        _scheduler.Schedule(OnslaughtFirst, TASK_GROUP_PHASE_ONE, [this](TaskContext context)
        {
            if (Creature* goriona = _instance->GetCreature(DATA_GORIONA))
            {
                if (context.GetRepeatCounter() == 0)
                    if (Creature* blackhorn = _instance->GetCreature(DATA_WARMASTER_BLACKHORN))
                        blackhorn->AI()->Talk(SAY_GORIONA);
                goriona->AI()->DoAction(ACTION_ONSLAUGHT);
            }
            context.Repeat(OnslaughtRepeat);
        });

        BlackhornTuning const& tuning = GetTuning(_instance, me->GetMap());

        if (tuning.SappersPerDrop)
            _scheduler.Schedule(SapperFirst, TASK_GROUP_PHASE_ONE, [this](TaskContext context)
            {
                me->SummonCreature(NPC_TWILIGHT_INFILTRATOR, InfiltratorSpawnPos, TEMPSUMMON_MANUAL_DESPAWN);
                context.Repeat(SapperRepeat);
            });

        if (tuning.Broadside)
            _scheduler.Schedule(BroadsideFirst, TASK_GROUP_PHASE_ONE, [this](TaskContext context)
            {
                if (Creature* goriona = _instance->GetCreature(DATA_GORIONA))
                    goriona->AI()->DoAction(ACTION_BROADSIDE);
                context.Repeat(BroadsideRepeat);
            });

        StartChecks();
    }

    void StartChecks()
    {
        _scheduler.Schedule(5s, TASK_GROUP_CHECKS, [this](TaskContext context)
        {
            if (!IsAnyPlayerAliveOnArena(me->GetMap()))
            {
                FailEncounter();
                return;
            }
            context.Repeat(5s);
        });

        // The Skyfire has no railings to speak of: falling off is death
        _scheduler.Schedule(1s, TASK_GROUP_CHECKS, [this](TaskContext context)
        {
            for (MapReference const& ref : me->GetMap()->GetPlayers())
            {
                Player* player = ref.GetSource();
                if (player && player->IsAlive() && !player->IsGameMaster()
                    && IsOnArena(player) && player->GetPositionZ() < DeckFallZ)
                    player->KillSelf();
            }
            context.Repeat(1s);
        });
    }

    void SpawnWave(uint8 wave)
    {
        _wavesSpawned = wave + 1;

        for (uint8 side = 0; side < 2; ++side)
            if (Creature* drake = me->SummonCreature(DrakeEntries[side], DrakeSpawnPos[side], TEMPSUMMON_MANUAL_DESPAWN))
                drake->AI()->SetData(DATA_WAVE_INDEX, wave);

        if (wave == 0)
            _scheduler.Schedule(20s, TASK_GROUP_PHASE_ONE, [this](TaskContext /*context*/)
            {
                TalkCaptain(SAY_CAPTAIN_HARPOON);
            });

        // The guns pick up the fresh drakes; the two guns fire staggered
        _scheduler.Schedule(HarpoonAfterWave, TASK_GROUP_PHASE_ONE, [this](TaskContext /*context*/)
        {
            uint8 index = 0;
            for (Position const& pos : HarpoonGunPos)
            {
                if (Creature* gun = GetSummon(NPC_SKYFIRE_HARPOON_GUN, pos))
                {
                    if (index == 0)
                        gun->AI()->DoAction(ACTION_ENABLE_HARPOON);
                    else
                        _scheduler.Schedule(6500ms, TASK_GROUP_PHASE_ONE, [this, guid = gun->GetGUID()](TaskContext /*context*/)
                        {
                            if (Creature* gun = ObjectAccessor::GetCreature(*me, guid))
                                gun->AI()->DoAction(ACTION_ENABLE_HARPOON);
                        });
                }
                ++index;
            }
        });
    }

    void StartPhaseTwo()
    {
        _stage = STAGE_PHASE_TWO;
        _scheduler.CancelGroup(TASK_GROUP_PHASE_ONE);

        if (Creature* skyfire = _instance->GetCreature(DATA_THE_SKYFIRE))
            skyfire->AI()->DoAction(ACTION_SHIP_INVULNERABLE);

        for (Position const& pos : HarpoonGunPos)
            if (Creature* gun = GetSummon(NPC_SKYFIRE_HARPOON_GUN, pos))
                gun->AI()->DoAction(ACTION_DISABLE_HARPOON);

        // The deck cannons drive Goriona off; she drops Blackhorn on deck
        for (Position const& pos : CannonPos)
            if (Creature* cannon = GetSummon(NPC_SKYFIRE_CANNON, pos))
                cannon->AI()->DoAction(ACTION_ARTILLERY_BARRAGE);

        _scheduler.Schedule(3s, [this](TaskContext /*context*/)
        {
            if (Creature* goriona = _instance->GetCreature(DATA_GORIONA))
                goriona->AI()->DoAction(ACTION_PHASE_TWO);
        });
    }

    void ShipDestroyed()
    {
        _stage = STAGE_FAILING;
        _scheduler.CancelGroup(TASK_GROUP_PHASE_ONE);
        _scheduler.CancelGroup(TASK_GROUP_CHECKS);

        TalkCaptain(SAY_CAPTAIN_ABANDON);
        if (Creature* skyfire = _instance->GetCreature(DATA_THE_SKYFIRE))
            skyfire->CastSpell(skyfire, SPELL_MASSIVE_EXPLOSION, true);

        _scheduler.Schedule(2s, [this](TaskContext /*context*/)
        {
            for (MapReference const& ref : me->GetMap()->GetPlayers())
            {
                Player* player = ref.GetSource();
                if (player && player->IsAlive() && !player->IsGameMaster() && IsOnArena(player))
                    player->KillSelf();
            }
        });
        _scheduler.Schedule(4s, [this](TaskContext /*context*/)
        {
            FailEncounter();
        });
    }

    void FailEncounter()
    {
        if (_stage == STAGE_STAGING || _stage == STAGE_DONE)
            return;
        _stage = STAGE_STAGING;

        _scheduler.CancelGroup(TASK_GROUP_PHASE_ONE);
        _scheduler.CancelGroup(TASK_GROUP_CHECKS);

        _instance->SetBossState(DATA_WARMASTER_BLACKHORN, FAIL);

        if (Creature* skyfire = _instance->GetCreature(DATA_THE_SKYFIRE))
            _instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, skyfire);
        if (Creature* blackhorn = _instance->GetCreature(DATA_WARMASTER_BLACKHORN))
            _instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, blackhorn);
        if (Creature* goriona = _instance->GetCreature(DATA_GORIONA))
            _instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, goriona);

        // Anyone still standing sails home; the dead release normally
        for (MapReference const& ref : me->GetMap()->GetPlayers())
        {
            Player* player = ref.GetSource();
            if (player && player->IsAlive() && !player->IsGameMaster() && IsOnArena(player))
                player->NearTeleportTo(SkyfireStagingPos);
        }

        _summons.DespawnAll();
        for (auto& list : _waveElites)
            list.clear();

        // the controller shares the staging deck with the captains; the arena
        // copies (same entries) are far away, so proximity finds the real pair
        _scheduler.Schedule(15s, [this](TaskContext /*context*/)
        {
            if (_stage != STAGE_STAGING)
                return;
            for (uint32 entry : { NPC_SKY_CAPTAIN_SWAYZE, NPC_KAANU_REEVS })
                if (Creature* captain = me->FindNearestCreature(entry, 100.0f))
                    captain->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        });
    }

    void FinishEncounter()
    {
        _stage = STAGE_DONE;
        _scheduler.CancelGroup(TASK_GROUP_PHASE_ONE);
        _scheduler.CancelGroup(TASK_GROUP_CHECKS);

        if (Creature* skyfire = _instance->GetCreature(DATA_THE_SKYFIRE))
            _instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, skyfire);
        if (Creature* goriona = _instance->GetCreature(DATA_GORIONA))
            goriona->DespawnOrUnsummon(5s);

        _scheduler.Schedule(4s, [this](TaskContext /*context*/)
        {
            TalkCaptain(SAY_CAPTAIN_OUTRO);
        });
    }

    InstanceScript* _instance;
    SummonList _summons;
    TaskScheduler _scheduler;
    Stage _stage = STAGE_STAGING;
    uint8 _drakesDied = 0;
    uint8 _wavesSpawned = 0;
    uint32 _captainEntry = NPC_SKY_CAPTAIN_SWAYZE;
    ObjectGuid _arenaCaptain;
    std::vector<ObjectGuid> _waveElites[3];
};

// 56598 - The Skyfire: structural integrity proxy shown in the encounter
// frame. Immune to everything; only scripted sources lower its health.
struct npc_ds_skyfire : public ScriptedAI
{
    npc_ds_skyfire(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript()) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        me->SetReactState(REACT_PASSIVE);
        ApplyLFRHealth(me, _instance, NPC_SKYFIRE_LFR_STATS);
        _invulnerable = false;
        _fireStage = 0;
        _lowWarned = false;
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage) override
    {
        // all real damage is swallowed; ship damage goes through SetData
        damage = 0;
    }

    void EnterEvadeMode(EvadeReason /*why*/) override { }

    void DoAction(int32 action) override
    {
        if (action == ACTION_SHIP_INVULNERABLE)
            _invulnerable = true;
    }

    void SetData(uint32 type, uint32 value) override
    {
        switch (type)
        {
            case SHIP_DATA_DAMAGE:
                ApplyShipDamage(value);
                break;
            case SHIP_DATA_DAMAGE_PCT_MAX:
                ApplyShipDamage(me->CountPctFromMaxHealth(value));
                break;
            case SHIP_DATA_DAMAGE_PCT_REMAINING:
                ApplyShipDamage(CalculatePct(me->GetHealth(), value));
                break;
            default:
                break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

private:
    void ApplyShipDamage(uint64 amount)
    {
        if (_invulnerable || _instance->GetBossState(DATA_WARMASTER_BLACKHORN) != IN_PROGRESS)
            return;

        if (amount >= me->GetHealth() - 1)
        {
            me->SetHealth(1);
            if (Creature* controller = _instance->GetCreature(DATA_GUNSHIP_PURSUIT_CONTROLLER))
                controller->AI()->DoAction(ACTION_SHIP_DESTROYED);
            return;
        }

        me->SetHealth(me->GetHealth() - amount);

        if (!_lowWarned && me->HealthBelowPct(30))
        {
            _lowWarned = true;
            if (Creature* captain = me->FindNearestCreature(NPC_SKY_CAPTAIN_SWAYZE, 100.0f))
                captain->AI()->Talk(SAY_CAPTAIN_SHIP_LOW);
            else if (Creature* captain = me->FindNearestCreature(NPC_KAANU_REEVS, 100.0f))
                captain->AI()->Talk(SAY_CAPTAIN_SHIP_LOW);
        }

        if (GetTuning(_instance, me->GetMap()).DeckFires)
            UpdateDeckFires();
    }

    void UpdateDeckFires()
    {
        static uint32 const thresholds[3] = { 75, 50, 25 };
        while (_fireStage < 3 && me->HealthBelowPct(thresholds[_fireStage]))
        {
            ++_fireStage;
            Talk(EMOTE_DECK_FIRE);
            DoCastSelf(SPELL_ENGINE_FIRE, true);

            for (uint8 i = 0; i < 2; ++i)
            {
                Position firePos = RandomDeckPosition();
                if (Creature* fire = me->SummonCreature(NPC_DECK_FIRE, firePos, TEMPSUMMON_TIMED_DESPAWN, 25s))
                {
                    if (Creature* controller = _instance->GetCreature(DATA_GUNSHIP_PURSUIT_CONTROLLER))
                        controller->AI()->JustSummoned(fire);

                    // a deckhand hurries over and "douses" the blaze
                    if (Creature* deckhand = me->FindNearestCreature(NPC_SKYFIRE_DECKHAND, 100.0f))
                        deckhand->GetMotionMaster()->MovePoint(0, firePos);
                }
            }
        }
    }

    InstanceScript* _instance;
    TaskScheduler _scheduler;
    bool _invulnerable = false;
    bool _lowWarned = false;
    uint8 _fireStage = 0;
};

// 56427 - Warmaster Blackhorn: rides Goriona through Phase One, fights on
// deck in Phase Two.
struct boss_warmaster_blackhorn : public BossAI
{
    boss_warmaster_blackhorn(Creature* creature) : BossAI(creature, DATA_WARMASTER_BLACKHORN) { }

    void Reset() override
    {
        _Reset();
        _scheduler.CancelAll();
        _siphoning = false;
        _gorionaGone = false;
        _landed = false;

        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
        ApplyLFRHealth(me, instance, NPC_BLACKHORN_LFR_STATS);
    }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        if (Creature* controller = instance->GetCreature(DATA_GUNSHIP_PURSUIT_CONTROLLER))
            controller->AI()->JustSummoned(me);
    }

    void DamageDealt(Unit* /*victim*/, uint32& damage, DamageEffectType /*damageType*/) override
    {
        ApplyLFRDamageReduction(instance, damage);
    }

    void DoAction(int32 action) override
    {
        if (action == ACTION_PHASE_TWO)
        {
            Talk(SAY_PHASE_TWO);
            _scheduler.Schedule(1s, [this](TaskContext /*context*/)
            {
                me->ExitVehicle();
                me->GetMotionMaster()->MoveJump(BlackhornLandPos, 20.0f, 15.0f, POINT_BLACKHORN_LAND);
            });
        }
        else if (action == ACTION_GORIONA_GONE)
        {
            _gorionaGone = true;
            if (_siphoning)
                StopSiphon();
        }
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != EFFECT_MOTION_TYPE || pointId != POINT_BLACKHORN_LAND)
            return;

        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
        me->SetReactState(REACT_AGGRESSIVE);
        me->SetHomePosition(BlackhornLandPos);
        _landed = true;
        DoZoneInCombat();
    }

    void JustEngagedWith(Unit* who) override
    {
        BossAI::JustEngagedWith(who);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 1);

        DoCastSelf(SPELL_VENGEANCE, true);
        events.ScheduleEvent(EVENT_DEVASTATE, 8500ms);
        events.ScheduleEvent(EVENT_DISRUPTING_ROAR, 10s);
        events.ScheduleEvent(EVENT_SHOCKWAVE, 13s);
        if (GetTuning(instance, me->GetMap()).Berserk)
            events.ScheduleEvent(EVENT_BERSERK, BerserkAfterLand);
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage) override
    {
        if (!_siphoning && !_gorionaGone && _landed
            && GetTuning(instance, me->GetMap()).SiphonVitality
            && me->HealthBelowPctDamaged(20, damage))
        {
            if (Creature* goriona = instance->GetCreature(DATA_GORIONA))
                if (goriona->IsAlive())
                    StartSiphon();
        }
    }

    void KilledUnit(Unit* victim) override
    {
        if (victim->GetTypeId() == TYPEID_PLAYER && roll_chance_i(50))
            Talk(SAY_SLAY);
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
        Talk(SAY_DEATH);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        if (Creature* controller = instance->GetCreature(DATA_GUNSHIP_PURSUIT_CONTROLLER))
            controller->AI()->DoAction(ACTION_ENCOUNTER_DONE);
    }

    void EnterEvadeMode(EvadeReason why) override
    {
        // dead-and-releasing raids drop off the threat list before the wipe
        // check fires; never evade while anyone is still fighting
        if (why == EVADE_REASON_NO_HOSTILES && IsAnyPlayerAliveInCombat())
            return;

        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        if (Creature* controller = instance->GetCreature(DATA_GUNSHIP_PURSUIT_CONTROLLER))
            controller->AI()->DoAction(ACTION_ENCOUNTER_FAILED);
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);

        if (!_landed || !UpdateVictim())
            return;

        events.Update(diff);
        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_DEVASTATE:
                    DoCastVictim(SPELL_DEVASTATE);
                    events.Repeat(8500ms);
                    break;
                case EVENT_DISRUPTING_ROAR:
                    if (roll_chance_i(30))
                        Talk(SAY_FLAVOR);
                    DoCastAOE(SPELL_DISRUPTING_ROAR);
                    events.Repeat(18500ms, 24s);
                    break;
                case EVENT_SHOCKWAVE:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 80.0f, true))
                    {
                        Talk(SAY_SHOCKWAVE);
                        me->SetFacingToObject(target);
                        me->AddUnitState(UNIT_STATE_CANNOT_TURN);
                        DoCast(target, SPELL_SHOCKWAVE);
                        _scheduler.Schedule(3500ms, [this](TaskContext /*context*/)
                        {
                            me->ClearUnitState(UNIT_STATE_CANNOT_TURN);
                        });
                    }
                    events.Repeat(23s);
                    break;
                case EVENT_BERSERK:
                    Talk(SAY_BERSERK);
                    DoCastSelf(SPELL_BERSERK, true);
                    break;
                default:
                    break;
            }
        }

        DoMeleeAttackIfReady();
    }

private:
    enum P2Events
    {
        EVENT_DEVASTATE = 1,
        EVENT_DISRUPTING_ROAR,
        EVENT_SHOCKWAVE,
        EVENT_BERSERK
    };

    bool IsAnyPlayerAliveInCombat() const
    {
        for (MapReference const& ref : me->GetMap()->GetPlayers())
        {
            Player* player = ref.GetSource();
            if (player && player->IsAlive() && !player->IsGameMaster() && player->IsInCombatWith(me))
                return true;
        }
        return false;
    }

    void StartSiphon()
    {
        _siphoning = true;
        Talk(EMOTE_SIPHON);

        _scheduler.Schedule(1ms, TASK_GROUP_SIPHON, [this](TaskContext context)
        {
            Creature* goriona = instance->GetCreature(DATA_GORIONA);
            if (!goriona || !goriona->IsAlive() || _gorionaGone || !me->IsAlive())
            {
                StopSiphon();
                return;
            }

            // 20% of her current health per pulse: damage her, heal him,
            // and mirror the same amount split across the raid
            uint32 drain = CalculatePct(goriona->GetHealth(), 20);
            if (drain < 1000)
                drain = 1000; // she is nearly dry; keep the pressure honest

            me->CastSpell(goriona, SPELL_SIPHON_VITALITY, true);
            Unit::DealDamage(me, goriona, drain, 0, SPELL_DIRECT_DAMAGE, SPELL_SCHOOL_MASK_SHADOW);
            me->ModifyHealth(int32(std::min<uint64>(drain, me->GetMaxHealth() - me->GetHealth())));

            std::vector<Player*> raid;
            for (MapReference const& ref : me->GetMap()->GetPlayers())
            {
                Player* player = ref.GetSource();
                if (player && player->IsAlive() && !player->IsGameMaster() && IsOnArena(player))
                    raid.push_back(player);
            }
            if (!raid.empty())
            {
                uint32 share = std::max<uint32>(drain / raid.size(), 1);
                for (Player* player : raid)
                    Unit::DealDamage(me, player, share, 0, SPELL_DIRECT_DAMAGE, SPELL_SCHOOL_MASK_SHADOW);
            }

            context.Repeat(2s);
        });
    }

    void StopSiphon()
    {
        _siphoning = false;
        _scheduler.CancelGroup(TASK_GROUP_SIPHON);
    }

    static constexpr uint32 TASK_GROUP_SIPHON = 1;

    TaskScheduler _scheduler;
    bool _siphoning = false;
    bool _gorionaGone = false;
    bool _landed = false;
};

// 56781 - Goriona
struct npc_ds_goriona : public ScriptedAI
{
    npc_ds_goriona(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript()) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetDisableGravity(true);
        ApplyLFRHealth(me, _instance, NPC_GORIONA_LFR_STATS);

        if (Creature* controller = _instance->GetCreature(DATA_GUNSHIP_PURSUIT_CONTROLLER))
            controller->AI()->JustSummoned(me);

        me->GetMotionMaster()->MovePoint(POINT_GORIONA_P1_HOVER, GorionaP1HoverPos, false);
    }

    void DamageDealt(Unit* /*victim*/, uint32& damage, DamageEffectType /*damageType*/) override
    {
        ApplyLFRDamageReduction(_instance, damage);
    }

    void DoAction(int32 action) override
    {
        switch (action)
        {
            case ACTION_ONSLAUGHT:
            {
                Talk(EMOTE_ONSLAUGHT);
                Position dest = RandomDeckPosition();
                if (Creature* marker = me->SummonCreature(NPC_ONSLAUGHT_TARGET, dest, TEMPSUMMON_TIMED_DESPAWN, 15s))
                    me->CastSpell(marker, SPELL_TWILIGHT_ONSLAUGHT, false);
                break;
            }
            case ACTION_BROADSIDE:
                Talk(EMOTE_BROADSIDE);
                if (Creature* skyfire = _instance->GetCreature(DATA_THE_SKYFIRE))
                    me->CastSpell(skyfire, SPELL_BROADSIDE, false);
                break;
            case ACTION_PHASE_TWO:
                _phaseTwo = true;
                me->InterruptNonMeleeSpells(false);
                me->GetMotionMaster()->MovePoint(POINT_GORIONA_P2_DROP, GorionaP2DropPos, false);
                break;
            default:
                break;
        }
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != POINT_MOTION_TYPE)
            return;

        switch (pointId)
        {
            case POINT_GORIONA_P2_DROP:
                // set the Warmaster down, then take up strafing position
                if (Vehicle* kit = me->GetVehicleKit())
                    if (Unit* blackhorn = kit->GetPassenger(0))
                        if (Creature* rider = blackhorn->ToCreature())
                            rider->AI()->DoAction(ACTION_PHASE_TWO);
                me->GetMotionMaster()->MovePoint(POINT_GORIONA_P2_AIR, GorionaP2AirPos, false);
                StartAirLoop();
                break;
            case POINT_GORIONA_LAND:
                Land();
                break;
            case POINT_GORIONA_FLEE:
                me->DespawnOrUnsummon();
                break;
            default:
                break;
        }
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage) override
    {
        if (!_fled && me->HealthBelowPctDamaged(20, damage))
        {
            // she survives 20% no matter the hit and flees instead
            if (damage >= me->GetHealth())
                damage = me->GetHealth() - 1;
            Retreat();
            return;
        }

        if (_phaseTwo && !_landing && !_fled
            && GetTuning(_instance, me->GetMap()).GorionaLands
            && me->HealthBelowPctDamaged(80, damage))
        {
            _landing = true;
            _scheduler.CancelGroup(TASK_GROUP_AIR);
            me->GetMotionMaster()->MovePoint(POINT_GORIONA_LAND, GorionaLandPos, false);
        }
    }

    void JustDied(Unit* /*killer*/) override
    {
        // killed outright (heroic burst): same bookkeeping as fleeing
        NotifyGone();
    }

    void EnterEvadeMode(EvadeReason why) override
    {
        if (why == EVADE_REASON_NO_HOSTILES && _instance->GetBossState(DATA_WARMASTER_BLACKHORN) == IN_PROGRESS)
            return;

        if (Creature* controller = _instance->GetCreature(DATA_GUNSHIP_PURSUIT_CONTROLLER))
            controller->AI()->DoAction(ACTION_ENCOUNTER_FAILED);
    }

    // Consuming Shroud bookkeeping: the core has no heal-absorb script hooks,
    // so a one second tracker mirrors every absorbed chunk as raid damage.
    void SetGUID(ObjectGuid const& guid, int32 id) override
    {
        if (id == GUID_SHROUD_APPLY)
        {
            if (Unit* target = ObjectAccessor::GetUnit(*me, guid))
                if (AuraEffect const* absorb = GetShroudEffect(target))
                    _shroudAmounts[guid] = uint32(absorb->GetAmount());
        }
        else if (id == GUID_SHROUD_REMOVE)
        {
            auto itr = _shroudAmounts.find(guid);
            if (itr != _shroudAmounts.end())
            {
                if (itr->second > 0)
                    MirrorShroudDamage(itr->second, guid);
                _shroudAmounts.erase(itr);
            }
        }
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);

        if (_grounded && UpdateVictim())
            DoMeleeAttackIfReady();
    }

private:
    static constexpr uint32 TASK_GROUP_AIR    = 1;
    static constexpr uint32 TASK_GROUP_GROUND = 2;

    AuraEffect const* GetShroudEffect(Unit const* target) const
    {
        if (AuraEffect const* eff = target->GetAuraEffect(SPELL_CONSUMING_SHROUD, EFFECT_0, me->GetGUID()))
            return eff;
        return target->GetAuraEffect(110598, EFFECT_0, me->GetGUID()); // 25H fork
    }

    void StartAirLoop()
    {
        _scheduler.Schedule(12s, TASK_GROUP_AIR, [this](TaskContext context)
        {
            if (Unit* target = SelectRandomDeckPlayer())
                me->CastSpell(target, SPELL_TWILIGHT_FLAMES_MISSILE, false);
            context.Repeat(8s);
        });

        // the shroud tracker runs for the whole of Phase Two
        _scheduler.Schedule(1s, [this](TaskContext context)
        {
            for (auto itr = _shroudAmounts.begin(); itr != _shroudAmounts.end();)
            {
                Unit* target = ObjectAccessor::GetUnit(*me, itr->first);
                AuraEffect const* absorb = target ? GetShroudEffect(target) : nullptr;
                if (absorb)
                {
                    uint32 current = uint32(absorb->GetAmount());
                    if (current < itr->second)
                        MirrorShroudDamage(itr->second - current, itr->first);
                    itr->second = current;
                    ++itr;
                }
                else
                    itr = _shroudAmounts.erase(itr); // removal handled by the aura script
            }
            context.Repeat(1s);
        });
    }

    Player* SelectRandomDeckPlayer() const
    {
        std::vector<Player*> candidates;
        for (MapReference const& ref : me->GetMap()->GetPlayers())
        {
            Player* player = ref.GetSource();
            if (player && player->IsAlive() && !player->IsGameMaster() && IsOnArena(player))
                candidates.push_back(player);
        }
        if (candidates.empty())
            return nullptr;
        return Trinity::Containers::SelectRandomContainerElement(candidates);
    }

    void Land()
    {
        _grounded = true;
        me->SetDisableGravity(false);
        me->SetReactState(REACT_AGGRESSIVE);
        DoZoneInCombat();
        _instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 3);

        _scheduler.Schedule(20500ms, TASK_GROUP_GROUND, [this](TaskContext context)
        {
            DoCastVictim(SPELL_TWILIGHT_BREATH);
            context.Repeat(20500ms);
        });
        _scheduler.Schedule(45s, TASK_GROUP_GROUND, [this](TaskContext context)
        {
            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 100.0f, true))
                me->CastSpell(target, SPELL_CONSUMING_SHROUD, false);
            else if (Unit* anyone = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                me->CastSpell(anyone, SPELL_CONSUMING_SHROUD, false);
            context.Repeat(30s);
        });
    }

    void Retreat()
    {
        if (_fled)
            return;
        _fled = true;

        Talk(EMOTE_RETREAT);
        _scheduler.CancelGroup(TASK_GROUP_AIR);
        _scheduler.CancelGroup(TASK_GROUP_GROUND);
        me->InterruptNonMeleeSpells(false);
        me->SetReactState(REACT_PASSIVE);
        me->AttackStop();
        me->SetDisableGravity(true);

        if (_grounded)
            _instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        _grounded = false;

        NotifyGone();
        me->GetMotionMaster()->MovePoint(POINT_GORIONA_FLEE, GorionaFleePos, false);
    }

    void NotifyGone()
    {
        if (_goneNotified)
            return;
        _goneNotified = true;

        if (_grounded)
            _instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

        if (Creature* blackhorn = _instance->GetCreature(DATA_WARMASTER_BLACKHORN))
            blackhorn->AI()->DoAction(ACTION_GORIONA_GONE);
    }

    void MirrorShroudDamage(uint32 amount, ObjectGuid except)
    {
        ApplyLFRDamageReduction(_instance, amount);
        for (MapReference const& ref : me->GetMap()->GetPlayers())
        {
            Player* player = ref.GetSource();
            if (!player || !player->IsAlive() || player->IsGameMaster() || player->GetGUID() == except || !IsOnArena(player))
                continue;
            me->CastSpell(player, SPELL_CONSUMING_SHROUD_DMG, CastSpellExtraArgs(true).AddSpellBP0(int32(amount)));
        }
    }

    InstanceScript* _instance;
    TaskScheduler _scheduler;
    std::unordered_map<ObjectGuid, uint32> _shroudAmounts;
    bool _phaseTwo = false;
    bool _landing = false;
    bool _grounded = false;
    bool _fled = false;
    bool _goneNotified = false;
};

// 56855 / 56587 - Twilight Assault Drake: strafes in, drops its riders on
// deck, then bombards the ship from off the rail until harpooned or killed.
struct npc_ds_twilight_assault_drake : public ScriptedAI
{
    npc_ds_twilight_assault_drake(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript()) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetDisableGravity(true);
        ApplyLFRHealth(me, _instance, NPC_ASSAULT_DRAKE_LFR_STATS);

        _side = me->GetEntry() == NPC_TWILIGHT_ASSAULT_DRAKE_P ? 0 : 1;
        me->GetMotionMaster()->MovePoint(POINT_DRAKE_DROP, DrakeDropPos[_side], false);
    }

    void SetData(uint32 type, uint32 value) override
    {
        if (type == DATA_WAVE_INDEX)
            _wave = uint8(value);
    }

    void DamageDealt(Unit* /*victim*/, uint32& damage, DamageEffectType /*damageType*/) override
    {
        ApplyLFRDamageReduction(_instance, damage);
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != POINT_MOTION_TYPE)
            return;

        switch (pointId)
        {
            case POINT_DRAKE_DROP:
                DropRiders();
                me->GetMotionMaster()->MovePoint(POINT_DRAKE_BOMBARD, DrakeBombardPos[_side], false);
                break;
            case POINT_DRAKE_BOMBARD:
                StartBarrage();
                break;
            case POINT_DRAKE_REELED:
                me->SetControlled(true, UNIT_STATE_ROOT);
                break;
            default:
                break;
        }
    }

    void DoAction(int32 action) override
    {
        if (action == ACTION_HARPOONED)
        {
            _harpooned = true;
            me->InterruptNonMeleeSpells(false);
            me->SetControlled(false, UNIT_STATE_ROOT);
            me->GetMotionMaster()->MovePoint(POINT_DRAKE_REELED, DrakeReeledPos[_side], false);
        }
        else if (action == ACTION_RELEASED)
        {
            _harpooned = false;
            me->SetControlled(false, UNIT_STATE_ROOT);
            me->GetMotionMaster()->MovePoint(POINT_DRAKE_BOMBARD, DrakeBombardPos[_side], false);
        }
    }

    void EnterEvadeMode(EvadeReason why) override
    {
        if (why == EVADE_REASON_NO_HOSTILES && _instance->GetBossState(DATA_WARMASTER_BLACKHORN) == IN_PROGRESS)
            return;
        ScriptedAI::EnterEvadeMode(why);
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

private:
    void DropRiders()
    {
        std::vector<Creature*> riders;

        if (Vehicle* kit = me->GetVehicleKit())
        {
            for (uint8 seat = 0; seat < 2; ++seat)
                if (Unit* passenger = kit->GetPassenger(seat))
                    if (Creature* rider = passenger->ToCreature())
                        riders.push_back(rider);
        }

        // 25 player raids field a second elite per drake; the vehicles only
        // have a single seat, so it materializes at the drop pass
        uint8 const elites = GetTuning(_instance, me->GetMap()).ElitesPerDrake;
        uint32 const extraEntry = _side == 0 ? NPC_TWILIGHT_ELITE_DREADBLADE : NPC_TWILIGHT_ELITE_SLAYER;
        while (riders.size() < elites)
            if (Creature* extra = me->SummonCreature(extraEntry, me->GetPosition(), TEMPSUMMON_MANUAL_DESPAWN))
                riders.push_back(extra);
            else
                break;

        for (Creature* rider : riders)
        {
            rider->ExitVehicle();
            Position land = RiderLandPos[_side];
            land.m_positionX += frand(-2.0f, 2.0f);
            land.m_positionY += frand(-2.0f, 2.0f);
            rider->GetMotionMaster()->MoveJump(land, 18.0f, 12.0f, POINT_RIDER_LAND);

            // controller cross-links the wave's elites into one health pool
            if (Creature* controller = _instance->GetCreature(DATA_GUNSHIP_PURSUIT_CONTROLLER))
            {
                controller->AI()->JustSummoned(rider);
                controller->AI()->SetGUID(rider->GetGUID(), _wave);
            }
        }
    }

    void StartBarrage()
    {
        _scheduler.Schedule(5s, [this](TaskContext context)
        {
            if (!_harpooned)
                me->CastSpell(RandomDeckPosition(), SPELL_TWILIGHT_BARRAGE, false);
            context.Repeat(6s);
        });
    }

    InstanceScript* _instance;
    TaskScheduler _scheduler;
    uint8 _side = 0;
    uint8 _wave = 0;
    bool _harpooned = false;
};

// 56854 Twilight Elite Dreadblade / 56848 Twilight Elite Slayer: deck melee
// with a wave-wide shared health pool (synced by percentage - the 25H pools
// are asymmetric).
struct npc_ds_twilight_elite : public ScriptedAI
{
    npc_ds_twilight_elite(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript()) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        me->SetReactState(REACT_PASSIVE);
        ApplyLFRHealth(me, _instance, NPC_TWILIGHT_ELITE_LFR_STATS);
    }

    void SetGUID(ObjectGuid const& guid, int32 id) override
    {
        if (id == GUID_LINK_ELITE)
            _twins.push_back(guid);
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type == EFFECT_MOTION_TYPE && pointId == POINT_RIDER_LAND)
            Engage();
    }

    void DamageDealt(Unit* /*victim*/, uint32& damage, DamageEffectType /*damageType*/) override
    {
        ApplyLFRDamageReduction(_instance, damage);
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage) override
    {
        if (_syncing)
            return;

        uint64 newHealth = damage >= me->GetHealth() ? 0 : me->GetHealth() - damage;
        float pct = 100.0f * float(newHealth) / float(me->GetMaxHealth());

        _syncing = true;
        for (ObjectGuid guid : _twins)
        {
            Creature* twin = ObjectAccessor::GetCreature(*me, guid);
            if (!twin || !twin->IsAlive())
                continue;

            if (!newHealth)
                twin->KillSelf();
            else
                twin->SetHealth(std::max<uint64>(uint64(twin->GetMaxHealth() * pct / 100.0f), 1));
        }
        _syncing = false;
    }

    void EnterEvadeMode(EvadeReason why) override
    {
        if (why == EVADE_REASON_NO_HOSTILES && _instance->GetBossState(DATA_WARMASTER_BLACKHORN) == IN_PROGRESS)
            return;
        ScriptedAI::EnterEvadeMode(why);
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);

        if (!_engaged)
        {
            // belt: even without a clean jump-landing inform, join the fight
            if (!_landFailsafe)
            {
                _landFailsafe = true;
                _scheduler.Schedule(4s, [this](TaskContext /*context*/)
                {
                    if (!_engaged)
                        Engage();
                });
            }
            return;
        }

        if (!UpdateVictim())
            return;

        _events.Update(diff);
        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_SPECIAL:
                    DoCastVictim(me->GetEntry() == NPC_TWILIGHT_ELITE_DREADBLADE ? SPELL_DEGENERATION : SPELL_BRUTAL_STRIKE);
                    _events.Repeat(me->GetEntry() == NPC_TWILIGHT_ELITE_DREADBLADE ? 8500ms : 10s);
                    break;
                case EVENT_BLADE_RUSH:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 80.0f, true))
                        me->CastSpell(target, SPELL_BLADE_RUSH, false);
                    _events.Repeat(me->GetMap()->IsHeroic() ? Milliseconds(15500ms) : randtime(20s, 25s));
                    break;
                default:
                    break;
            }
        }

        DoMeleeAttackIfReady();
    }

private:
    enum EliteEvents
    {
        EVENT_SPECIAL = 1,
        EVENT_BLADE_RUSH
    };

    void Engage()
    {
        if (_engaged)
            return;
        _engaged = true;

        me->SetReactState(REACT_AGGRESSIVE);
        DoZoneInCombat();
        _events.ScheduleEvent(EVENT_SPECIAL, randtime(5s, 9s));
        _events.ScheduleEvent(EVENT_BLADE_RUSH, randtime(18s, 25s));
    }

    InstanceScript* _instance;
    TaskScheduler _scheduler;
    EventMap _events;
    std::vector<ObjectGuid> _twins;
    bool _engaged = false;
    bool _landFailsafe = false;
    bool _syncing = false;
};

// 56681 - Skyfire Harpoon Gun: spears a drake, reels it into melee range,
// reloads, repeats.
struct npc_ds_skyfire_harpoon_gun : public ScriptedAI
{
    npc_ds_skyfire_harpoon_gun(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript()) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void DoAction(int32 action) override
    {
        switch (action)
        {
            case ACTION_ENABLE_HARPOON:
                if (!_active)
                {
                    _active = true;
                    _scheduler.Schedule(500ms, [this](TaskContext /*context*/) { TryHarpoon(); });
                }
                break;
            case ACTION_DISABLE_HARPOON:
                _active = false;
                _scheduler.CancelAll();
                me->InterruptNonMeleeSpells(false);
                break;
            case ACTION_RELOAD:
                if (_active)
                {
                    DoCastSelf(SPELL_RELOADING);
                    _scheduler.Schedule(11s, [this](TaskContext /*context*/) { TryHarpoon(); });
                }
                break;
            default:
                break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

private:
    void TryHarpoon()
    {
        if (!_active)
            return;

        Creature* target = PickDrake();
        if (!target)
        {
            _scheduler.Schedule(5s, [this](TaskContext /*context*/) { TryHarpoon(); });
            return;
        }

        me->CastSpell(target, SPELL_HARPOON, true);
    }

    Creature* PickDrake() const
    {
        // prefer the drake bombing this gun's side of the ship
        Creature* best = nullptr;
        float bestScore = std::numeric_limits<float>::max();
        bool const port = me->GetPositionY() < SkyfireProxyPos.GetPositionY();

        for (uint32 entry : { NPC_TWILIGHT_ASSAULT_DRAKE_P, NPC_TWILIGHT_ASSAULT_DRAKE_S })
        {
            std::list<Creature*> drakes;
            me->GetCreatureListWithEntryInGrid(drakes, entry, 250.0f);
            for (Creature* drake : drakes)
            {
                if (!drake->IsAlive() || drake->HasAura(SPELL_HARPOON))
                    continue;
                bool const drakePort = drake->GetPositionY() < SkyfireProxyPos.GetPositionY();
                float score = me->GetExactDist2d(drake) + (drakePort == port ? 0.0f : 500.0f);
                if (score < bestScore)
                {
                    bestScore = score;
                    best = drake;
                }
            }
        }
        return best;
    }

    InstanceScript* _instance;
    TaskScheduler _scheduler;
    bool _active = false;
};

// 57260 - Skyfire Cannon: flavor volleys during Phase One; drives Goriona
// off with the Artillery Barrage at the phase transition.
struct npc_ds_skyfire_cannon : public ScriptedAI
{
    npc_ds_skyfire_cannon(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript()) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        me->SetReactState(REACT_PASSIVE);

        _scheduler.Schedule(randtime(6s, 14s), [this](TaskContext context)
        {
            if (_instance->GetBossState(DATA_WARMASTER_BLACKHORN) == IN_PROGRESS)
                for (uint32 entry : { NPC_TWILIGHT_ASSAULT_DRAKE_P, NPC_TWILIGHT_ASSAULT_DRAKE_S })
                    if (Creature* drake = me->FindNearestCreature(entry, 250.0f))
                    {
                        me->CastSpell(drake, SPELL_HEAVY_SLUG, true);
                        break;
                    }
            context.Repeat(randtime(8s, 16s));
        });
    }

    void DoAction(int32 action) override
    {
        if (action == ACTION_ARTILLERY_BARRAGE)
            if (Creature* goriona = _instance->GetCreature(DATA_GORIONA))
                me->CastSpell(goriona, SPELL_ARTILLERY_BARRAGE, true);
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

private:
    InstanceScript* _instance;
    TaskScheduler _scheduler;
};

// 56922 - Twilight Infiltrator: darts over the deck and drops a sapper
struct npc_ds_twilight_infiltrator : public ScriptedAI
{
    npc_ds_twilight_infiltrator(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript()) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetDisableGravity(true);
        if (Creature* controller = _instance->GetCreature(DATA_GUNSHIP_PURSUIT_CONTROLLER))
            controller->AI()->JustSummoned(me);
        me->GetMotionMaster()->MovePoint(POINT_INFILTRATOR_DETACH, InfiltratorDetachPos, false);
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != POINT_MOTION_TYPE)
            return;

        if (pointId == POINT_INFILTRATOR_DETACH)
        {
            Talk(EMOTE_SAPPER_DROP);
            for (uint8 i = 0; i < GetTuning(_instance, me->GetMap()).SappersPerDrop; ++i)
                if (Creature* sapper = me->SummonCreature(NPC_TWILIGHT_SAPPER, me->GetPosition(), TEMPSUMMON_MANUAL_DESPAWN))
                    if (Creature* controller = _instance->GetCreature(DATA_GUNSHIP_PURSUIT_CONTROLLER))
                        controller->AI()->JustSummoned(sapper);

            me->GetMotionMaster()->MovePoint(POINT_INFILTRATOR_LEAVE, InfiltratorLeavePos, false);
        }
        else if (pointId == POINT_INFILTRATOR_LEAVE)
            me->DespawnOrUnsummon();
    }

private:
    InstanceScript* _instance;
};

// 56923 - Twilight Sapper: smoke-bombs onto the deck and makes for the
// engine door; Detonate costs the ship 20% of its total integrity.
struct npc_ds_twilight_sapper : public ScriptedAI
{
    npc_ds_twilight_sapper(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript()) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetSpeedRate(MOVE_RUN, 0.8f);
        me->GetMotionMaster()->MoveJump(SapperLandPos, 16.0f, 10.0f, POINT_SAPPER_LAND);
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type == EFFECT_MOTION_TYPE && pointId == POINT_SAPPER_LAND)
        {
            DoCastSelf(SPELL_SMOKE_BOMB, true);
            DoCastSelf(SPELL_SHADOWCLOAK, true);
            _scheduler.Schedule(2s, [this](TaskContext /*context*/)
            {
                _running = true;
                me->GetMotionMaster()->MovePoint(POINT_SAPPER_DOOR, SapperDoorPos, true);

                // crowd control merely delays the little goblin
                _scheduler.Schedule(1s, [this](TaskContext context)
                {
                    if (!_detonating && me->IsAlive()
                        && !me->HasUnitState(UNIT_STATE_LOST_CONTROL | UNIT_STATE_ROOT | UNIT_STATE_JUMPING)
                        && !me->isMoving())
                        me->GetMotionMaster()->MovePoint(POINT_SAPPER_DOOR, SapperDoorPos, true);
                    context.Repeat(1s);
                });
            });
        }
        else if (type == POINT_MOTION_TYPE && pointId == POINT_SAPPER_DOOR && _running && !_detonating)
        {
            if (me->GetExactDist2d(&SapperDoorPos) > 3.0f)
                return; // interrupted mid-path; the failsafe reissues

            _detonating = true;
            me->RemoveAurasDueToSpell(SPELL_SHADOWCLOAK);
            DoCastAOE(SPELL_DETONATE); // the spell script feeds the ship its 20%
            me->DespawnOrUnsummon(4s);
        }
    }

    void EnterEvadeMode(EvadeReason why) override
    {
        if (why == EVADE_REASON_NO_HOSTILES && _instance->GetBossState(DATA_WARMASTER_BLACKHORN) == IN_PROGRESS)
            return;
        ScriptedAI::EnterEvadeMode(why);
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

private:
    InstanceScript* _instance;
    TaskScheduler _scheduler;
    bool _running = false;
    bool _detonating = false;
};

// 57268 - Twilight Flames: lingering void-fire patch beneath Goriona's
// strafing runs (summoned natively by 108051 EFFECT_1)
struct npc_ds_twilight_flames : public ScriptedAI
{
    npc_ds_twilight_flames(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript()) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        me->SetReactState(REACT_PASSIVE);
        DoCastSelf(SPELL_TWILIGHT_FLAMES_PERIODIC, true);
        me->DespawnOrUnsummon(30s);

        if (Creature* controller = _instance->GetCreature(DATA_GUNSHIP_PURSUIT_CONTROLLER))
            controller->AI()->JustSummoned(me);
    }

    void DamageDealt(Unit* /*victim*/, uint32& damage, DamageEffectType /*damageType*/) override
    {
        ApplyLFRDamageReduction(_instance, damage);
    }

private:
    InstanceScript* _instance;
};

// 57920 - Deck Fire (heroic): scripted burn, doused after a while
struct npc_ds_deck_fire : public ScriptedAI
{
    npc_ds_deck_fire(Creature* creature) : ScriptedAI(creature) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        me->SetReactState(REACT_PASSIVE);
        DoCastSelf(SPELL_DECK_FIRE_VISUAL, true);

        _scheduler.Schedule(1s, [this](TaskContext context)
        {
            for (MapReference const& ref : me->GetMap()->GetPlayers())
            {
                Player* player = ref.GetSource();
                if (player && player->IsAlive() && !player->IsGameMaster() && me->GetExactDist2d(player) < 4.0f)
                    me->CastSpell(player, SPELL_DECK_FIRE_DMG, true);
            }
            context.Repeat(1s);
        });
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

private:
    TaskScheduler _scheduler;
};

// 107439, 109203, 109204, 109205 - Twilight Barrage (damage): split among the
// players who soak it; the ship eats the full hit - and the Deck Defender
// eligibility - only when nobody does.
class spell_blackhorn_twilight_barrage : public SpellScript
{
    void CountTargets(std::list<WorldObject*>& targets)
    {
        _targets = uint32(targets.size());
    }

    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        if (_targets > 1)
            SetHitDamage(GetHitDamage() / int32(_targets));
    }

    void HandleAfterCast()
    {
        if (_targets)
            return;

        Unit* caster = GetCaster();
        InstanceScript* instance = caster ? caster->GetInstanceScript() : nullptr;
        if (!instance)
            return;

        uint32 shipDamage = uint32(GetSpellInfo()->Effects[EFFECT_0].CalcValue(caster));
        ApplyLFRDamageReduction(instance, shipDamage);

        if (Creature* skyfire = instance->GetCreature(DATA_THE_SKYFIRE))
        {
            skyfire->AI()->SetData(SHIP_DATA_DAMAGE, shipDamage);
            caster->CastSpell(skyfire, SPELL_TWILIGHT_BARRAGE_SHIP, true);
        }

        instance->SetData(DATA_BLACKHORN_ACHIEVEMENT_FAILED, 1);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect.Register(&spell_blackhorn_twilight_barrage::CountTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
        OnEffectHitTarget.Register(&spell_blackhorn_twilight_barrage::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        AfterCast.Register(&spell_blackhorn_twilight_barrage::HandleAfterCast);
    }

    uint32 _targets = 0;
};

// 106401, 108862, 109226, 109227 - Twilight Onslaught (damage): split among
// the soakers plus the ship, which always takes one share (the full hit when
// nobody soaks).
class spell_goriona_twilight_onslaught : public SpellScript
{
    void CountTargets(std::list<WorldObject*>& targets)
    {
        _targets = uint32(targets.size());
    }

    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        SetHitDamage(GetHitDamage() / int32(_targets + 1));
    }

    void HandleAfterCast()
    {
        Unit* caster = GetCaster();
        InstanceScript* instance = caster ? caster->GetInstanceScript() : nullptr;
        if (!instance)
            return;

        uint32 base = uint32(GetSpellInfo()->Effects[EFFECT_0].CalcValue(caster));
        uint32 shipDamage = _targets ? base / (_targets + 1) : base;
        ApplyLFRDamageReduction(instance, shipDamage);

        if (Creature* skyfire = instance->GetCreature(DATA_THE_SKYFIRE))
        {
            skyfire->AI()->SetData(SHIP_DATA_DAMAGE, shipDamage);
            caster->CastSpell(skyfire, SPELL_TWILIGHT_ONSLAUGHT_SHIP, true);
        }
    }

    void Register() override
    {
        OnObjectAreaTargetSelect.Register(&spell_goriona_twilight_onslaught::CountTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
        OnEffectHitTarget.Register(&spell_goriona_twilight_onslaught::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        AfterCast.Register(&spell_goriona_twilight_onslaught::HandleAfterCast);
    }

    uint32 _targets = 0;
};

// 108038 - Harpoon: reels the drake in for a fixed window, then the gun
// reloads
class spell_blackhorn_harpoon : public AuraScript
{
    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        int32 duration = 25000;
        if (Unit* target = GetTarget())
        {
            duration = int32(GetTuning(target->GetInstanceScript(), target->GetMap()).HarpoonHoldMs);
            if (Creature* drake = target->ToCreature())
                drake->AI()->DoAction(ACTION_HARPOONED);
        }
        GetAura()->SetMaxDuration(duration);
        GetAura()->SetDuration(duration);
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Creature* drake = GetTarget()->ToCreature())
            if (drake->IsAlive())
                drake->AI()->DoAction(ACTION_RELEASED);

        if (Unit* caster = GetCaster())
            if (Creature* gun = caster->ToCreature())
                if (gun->IsAlive())
                    gun->AI()->DoAction(ACTION_RELOAD);
    }

    void Register() override
    {
        AfterEffectApply.Register(&spell_blackhorn_harpoon::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove.Register(&spell_blackhorn_harpoon::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 107518 - Detonate: the sapper reached the engine door
class spell_blackhorn_detonate : public SpellScript
{
    void HandleShipDamage(SpellEffIndex effIndex)
    {
        // effect 165 (20% max health) aims at the ship proxy natively; the
        // proxy swallows real damage, so the hit is routed by hand instead
        PreventHitDefaultEffect(effIndex);
    }

    void HandleAfterCast()
    {
        Unit* caster = GetCaster();
        InstanceScript* instance = caster ? caster->GetInstanceScript() : nullptr;
        if (!instance)
            return;

        Creature* skyfire = instance->GetCreature(DATA_THE_SKYFIRE);
        if (!skyfire)
            return;

        skyfire->AI()->SetData(SHIP_DATA_DAMAGE_PCT_MAX, 20);

        Creature* captain = skyfire->FindNearestCreature(NPC_SKY_CAPTAIN_SWAYZE, 100.0f);
        if (!captain)
            captain = skyfire->FindNearestCreature(NPC_KAANU_REEVS, 100.0f);
        if (captain)
            captain->AI()->Talk(SAY_CAPTAIN_SAPPER);
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_blackhorn_detonate::HandleShipDamage, EFFECT_1, SPELL_EFFECT_DAMAGE_FROM_MAX_HEALTH_PCT);
        AfterCast.Register(&spell_blackhorn_detonate::HandleAfterCast);
    }
};

// 108044, 109228, 109229, 109230 - Disrupting Roar: the interrupt only
// clips casters within ten yards
class spell_blackhorn_disrupting_roar : public SpellScript
{
    void FilterInterrupt(std::list<WorldObject*>& targets)
    {
        Unit* caster = GetCaster();
        targets.remove_if([caster](WorldObject* target)
        {
            return caster->GetExactDist(target) > 10.0f;
        });
    }

    void Register() override
    {
        OnObjectAreaTargetSelect.Register(&spell_blackhorn_disrupting_roar::FilterInterrupt, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
    }
};

// 110137 - Shockwave: dummy aimed at a player's position; the cone resolves
// where the boss was facing when the cast finished
class spell_blackhorn_shockwave : public SpellScript
{
    void HandleAfterCast()
    {
        // eff0 of the damage spell wants an enemy target; the cone itself
        // resolves along the caster's (locked) facing
        Unit* caster = GetCaster();
        Unit* target = GetExplTargetUnit();
        if (!target)
            target = caster->GetVictim();
        if (target)
            caster->CastSpell(target, SPELL_SHOCKWAVE_DMG, true);
    }

    void Register() override
    {
        AfterCast.Register(&spell_blackhorn_shockwave::HandleAfterCast);
    }
};

// 108045 - Vengeance: +1% damage done per 1% of health missing
class spell_blackhorn_vengeance : public AuraScript
{
    void OnPeriodic(AuraEffect const* /*aurEff*/)
    {
        Unit* owner = GetUnitOwner();
        if (AuraEffect* damageDone = GetAura()->GetEffect(EFFECT_0))
        {
            int32 missing = 100 - int32(owner->GetHealthPct());
            if (damageDone->GetAmount() != missing)
                damageDone->ChangeAmount(missing);
        }
    }

    void Register() override
    {
        OnEffectPeriodic.Register(&spell_blackhorn_vengeance::OnPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// 110214, 110598 - Consuming Shroud: heal absorb whose absorbed healing is
// mirrored as raid damage (tracked by Goriona's AI - the core has no
// heal-absorb script hooks)
class spell_goriona_consuming_shroud : public AuraScript
{
    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        NotifyGoriona(GUID_SHROUD_APPLY);
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        NotifyGoriona(GUID_SHROUD_REMOVE);
    }

    void NotifyGoriona(int32 what)
    {
        if (Unit* caster = GetCaster())
            if (Creature* goriona = caster->ToCreature())
                if (goriona->IsAIEnabled())
                    goriona->AI()->SetGUID(GetTarget()->GetGUID(), what);
    }

    void Register() override
    {
        AfterEffectApply.Register(&spell_goriona_consuming_shroud::OnApply, EFFECT_0, SPELL_AURA_SCHOOL_HEAL_ABSORB, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove.Register(&spell_goriona_consuming_shroud::OnRemove, EFFECT_0, SPELL_AURA_SCHOOL_HEAL_ABSORB, AURA_EFFECT_HANDLE_REAL);
    }
};

// 110153 - Broadside: strips a fifth of the ship's remaining integrity
class spell_goriona_broadside : public SpellScript
{
    void HandleAfterCast()
    {
        Unit* caster = GetCaster();
        InstanceScript* instance = caster ? caster->GetInstanceScript() : nullptr;
        if (!instance)
            return;

        if (Creature* skyfire = instance->GetCreature(DATA_THE_SKYFIRE))
        {
            skyfire->AI()->SetData(SHIP_DATA_DAMAGE_PCT_REMAINING, 20);
            caster->CastSpell(skyfire, SPELL_BROADSIDE_DMG, CastSpellExtraArgs(true).AddSpellBP0(1));
        }
    }

    void Register() override
    {
        AfterCast.Register(&spell_goriona_broadside::HandleAfterCast);
    }
};
}

void AddSC_boss_warmaster_blackhorn()
{
    using namespace DragonSoul;
    using namespace DragonSoul::Blackhorn;

    RegisterDragonSoulCreatureAI(npc_ds_skyfire_captain);
    RegisterDragonSoulCreatureAI(npc_gunship_pursuit_controller);
    RegisterDragonSoulCreatureAI(npc_ds_skyfire);
    RegisterDragonSoulCreatureAI(boss_warmaster_blackhorn);
    RegisterDragonSoulCreatureAI(npc_ds_goriona);
    RegisterDragonSoulCreatureAI(npc_ds_twilight_assault_drake);
    RegisterDragonSoulCreatureAI(npc_ds_twilight_elite);
    RegisterDragonSoulCreatureAI(npc_ds_skyfire_harpoon_gun);
    RegisterDragonSoulCreatureAI(npc_ds_skyfire_cannon);
    RegisterDragonSoulCreatureAI(npc_ds_twilight_infiltrator);
    RegisterDragonSoulCreatureAI(npc_ds_twilight_sapper);
    RegisterDragonSoulCreatureAI(npc_ds_twilight_flames);
    RegisterDragonSoulCreatureAI(npc_ds_deck_fire);

    RegisterSpellScript(spell_blackhorn_twilight_barrage);
    RegisterSpellScript(spell_goriona_twilight_onslaught);
    RegisterSpellScript(spell_blackhorn_harpoon);
    RegisterSpellScript(spell_blackhorn_detonate);
    RegisterSpellScript(spell_blackhorn_disrupting_roar);
    RegisterSpellScript(spell_blackhorn_shockwave);
    RegisterSpellScript(spell_blackhorn_vengeance);
    RegisterSpellScript(spell_goriona_consuming_shroud);
    RegisterSpellScript(spell_goriona_broadside);
}
