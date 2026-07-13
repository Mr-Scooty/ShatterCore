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
 * Well of Eternity: Mannoroth & Captain Varo'then (map 939, finale encounter)
 *
 * Timeline sources: 11.2.5 retail sniff (original 4.3.4 spell IDs), 4.3.4 DBM
 * (Mannoroth.lua - Fel Firestorm 15 s first / 29 s cd / 2 casts before the
 * suppression window, "Tyrande needs help" 82 s), 4.3.4 Spell.dbc decodes.
 *
 * Encounter layout:
 * - Mannoroth (54969, vehicle 584) owns the encounter (BossAI). Illidan tank-
 *   locks him for the whole fight; players handle Varo'then, the Dreadlord
 *   Debilitators and later the portal waves.
 * - Captain Varo'then (55419) is a satellite boss with his own encounter
 *   frame. His death (or Fel Drain sacrifice) drops Varo'then's Magical Blade;
 *   throwing it at Mannoroth starts the Magistrike Arc burn phase.
 * - Tyrande deletes the Doomguard Devastator stream with Lunar Shot unless
 *   Debilitators flay her; players free her using the Blessing of Elune proc
 *   (103918 -> 103919 vs lesser demons).
 * - Stage 3 at 20%: Nether Tear portal waves (Felhound/Felguard), resumed Fel
 *   Firestorms, Gift of Sargeras on the party, Inferno infernals, Tyrande's
 *   Hand of Elune finale and collapse.
 * - Fel Drain (104961, absent from the modern sniff, rebuilt from DBC +
 *   "That's Not Canon!"): heavy player damage on Mannoroth while Varo'then
 *   lives sacrifices Varo'then and heals Mannoroth to full, arming the
 *   achievement via DATA_FEL_DRAIN_TRIGGERED.
 */

#include "well_of_eternity.h"
#include "Containers.h"
#include "GameObject.h"
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
#include "TaskScheduler.h"
#include "TemporarySummon.h"
#include "Vehicle.h"

#include <vector>

namespace WellOfEternity::MannorothVarothen
{
enum Spells
{
    // Mannoroth
    SPELL_FELBLADE                      = 103966, // self proc-aura enabler, doublet scripted below
    SPELL_FELBURN                       = 103972, // the Felblade payload, only ever hits Illidan
    SPELL_FEL_FIRESTORM                 = 103888, // 12 s channel, periodic 300 ms -> 103889
    SPELL_FEL_FIRESTORM_MISSILE         = 103889, // dest missile, summons 55502 Fel Flames
    SPELL_NETHER_PORTAL                 = 104625, // cosmetic, missiles at the GP bunnies
    SPELL_FEL_DRAIN                     = 104961, // INSTAKILL Varo'then + heal caster to full
    SPELL_EMBEDDED_BLADE_MANNOROTH      = 104820, // proc aura marker (arc cycle is scripted)
    SPELL_EMBEDDED_BLADE_RETRIGGER      = 109542,
    SPELL_MAGISTRIKE_ARC                = 105524, // force-cast driver at a random player
    SPELL_MAGISTRIKE_ARC_DAMAGE         = 105523, // victim -> Mannoroth, 1,000,000
    SPELL_MAGISTRIKE_ARC_BLADE          = 104822, // Embedded Blade pulse visual
    SPELL_FEL_FIRE_NOVA                 = 105093, // wound pulse while the blade is embedded
    SPELL_NETHER_TEAR                   = 105041, // stage 3 portal opener (at GP bunny 54020)
    SPELL_SUMMON_FELHOUND               = 105053, // periodic 3 s -> 105054 (dest targeting needs
    SPELL_SUMMON_FELGUARD               = 105057, // conditions; waves are scheduled instead)
    SPELL_INFERNO_SELF                  = 105141, // "Inferno!" self state
    SPELL_INFERNO_MISSILE               = 105145, // dest missile (105146 trigger absent from DBC)
    SPELL_MANNOROTH_FINALE_ANIMKIT      = 105422,

    // Captain Varo'then
    SPELL_MAGISTRIKE                    = 103669, // bolt at a random player
    SPELL_SUMMON_MAGICAL_BLADE          = 104815, // dest missile -> 104816 summon (scripted directly)

    // Varo'then's Magical Blade / Embedded Blade
    SPELL_MAGICAL_BLADE_GROUND_VISUAL   = 104819,
    SPELL_MAGICAL_BLADE_PICKUP          = 104818, // spellclick: missile -> 104817 at Mannoroth
    SPELL_EMBEDDED_BLADE_VISUAL         = 104823, // blade NPC self aura in seat 0

    // Illidan
    SPELL_DEMONIC_SIGHT                 = 104746, // 90% dodge
    SPELL_TAUNT                         = 104461,
    SPELL_DEMON_RUSH                    = 104205,
    SPELL_DARKLANCE                     = 104394,
    SPELL_AURA_OF_IMMOLATION            = 104379, // self, periodic 3 s -> 104387
    SPELL_AURA_OF_IMMOLATION_DAMAGE     = 104387,
    SPELL_WATERS_OF_ETERNITY            = 103952, // self douse, periodic 500 ms -> 103954
    SPELL_WATERS_OF_ETERNITY_ZONE       = 103954, // -90% fire taken near Illidan
    SPELL_GIFT_OF_SARGERAS_ILLIDAN      = 104998,
    SPELL_GIFT_OF_SARGERAS_PLAYERS      = 105009,

    // Tyrande
    SPELL_BLESSING_OF_ELUNE             = 103917, // self, periodic 1 s -> 103918
    SPELL_BLESSING_OF_ELUNE_PLAYERS     = 103918, // proc aura -> 103919 vs lesser demons
    SPELL_ELUNES_WRATH                  = 103919,
    SPELL_LUNAR_SHOT_ESCORT             = 104214,
    SPELL_LUNAR_SHOT                    = 104313,
    SPELL_LUNAR_SHOT_AOE                = 104688,
    SPELL_HAND_OF_ELUNE                 = 105072, // 6 s cast, periodic 3 s -> 105073
    SPELL_WRATH_OF_ELUNE                = 105073,
    SPELL_WRATH_OF_ELUNE_FINAL          = 105075,
    SPELL_HAND_OF_ELUNE_FAREWELL        = 109546, // 30 min goodbye buff

    // Dreadlord Debilitator
    SPELL_DEBILITATOR_COSMETIC          = 104672,
    SPELL_DEBILITATING_FLAY             = 104678, // infinite channel on Tyrande

    // Highguard mirror wave
    SPELL_DISPLACEMENT                  = 103763, // shadowbat mirror trick
    SPELL_SHADOWBAT_COSMETIC            = 103756,

    // Portal adds
    SPELL_PORTAL_PULL_VISUAL            = 105339, // fly-in visual on spawn
    SPELL_FEL_ENTRANCE                  = 104468, // landing cosmetic

    // Fel Flames ground fire
    SPELL_FEL_FLAMES_PERIODIC           = 103892  // 1 s pulse of 103891
};

enum Texts
{
    // Mannoroth (54969)
    SAY_MANNOROTH_INTRO         = 0, // Varo'then, see that I am not disrupted by this rabble!
    SAY_MANNOROTH_AGGRO         = 1, // Come Stormrage, ...
    EMOTE_FEL_FIRESTORM         = 2, // %s begins to cast [Fel Firestorm]!
    SAY_MANNOROTH_NETHER_PORTAL = 3, // [Demonic] Amanare maev ...
    SAY_MANNOROTH_BLADE         = 4, // Rrraaaghhh!!
    SAY_MANNOROTH_STAGE_THREE   = 5, // Lord Sargeras, I will not fail you! ...
    EMOTE_FELGUARD_PORTAL       = 6, // Felguard pour forth from the demon portal!
    SAY_MANNOROTH_BURNING_EYES  = 7, // Yes...yes! I can feel his burning eyes upon me ...
    EMOTE_INFERNALS             = 8, // Infernals rain from the sky!
    SAY_MANNOROTH_DEATH_THROES  = 9, // No...no! This victory will not be ripped from my grasp! ...

    // Captain Varo'then (55419)
    SAY_VAROTHEN_HIGHGUARD      = 0, // Highguard, to arms! For your queen! For Azshara!
    SAY_VAROTHEN_AGGRO          = 1, // For you, Azshara.
    SAY_VAROTHEN_DEATH          = 2, // Light of lights...I have failed you. ...

    // Illidan (55532)
    SAY_ILLIDAN_INTRO_1         = 0,  // Can you close the portal, brother?
    SAY_ILLIDAN_INTRO_2         = 1,  // Very well, we shall break it for you.
    SAY_ILLIDAN_INTRO_3         = 2,  // Let them come.
    SAY_ILLIDAN_ROAD            = 3,  // Weak, pitiful creatures. ...
    SAY_ILLIDAN_VIAL_FUN        = 4,  // Oh this will be fun...
    SAY_ILLIDAN_VIAL_IDEA       = 5,  // Wait, I have an idea.
    SAY_ILLIDAN_VIAL_PEOPLE     = 6,  // What our people could not.
    EMOTE_ILLIDAN_WATERS        = 7,  // %s splashes the Waters of Eternity over himself!
    SAY_ILLIDAN_VIAL_POWER      = 8,  // Yes...YES. I can feel the raw power ...
    SAY_ILLIDAN_MIRROR_HINT     = 9,  // They are not where they appear to be! ...
    SAY_ILLIDAN_HANDLE_VAROTHEN = 10, // Handle Varo'then. Mannoroth is mine.
    SAY_ILLIDAN_SWORD_PIERCED   = 11, // The sword has pierced his infernal armor! ...
    EMOTE_ILLIDAN_PORTAL_OPENS  = 12, // A massive demonic portal opens nearby!
    SAY_ILLIDAN_STILL_CONNECTED = 13, // He is still connected to the Well somehow! ...
    SAY_ILLIDAN_ARTIFACT        = 14, // The artifact!
    SAY_ILLIDAN_EPILOGUE_1      = 15, // Brother. A timely arrival...
    SAY_ILLIDAN_EPILOGUE_2      = 16, // Aye. It's been twisted and turned by too many spells. ...
    SAY_ILLIDAN_EPILOGUE_3      = 17, // If you've a way out of here, we should probably use it! ...

    // Tyrande (55524)
    SAY_TYRANDE_INTRO_1         = 0,  // He knows what we attempt. ...
    SAY_TYRANDE_INTRO_2         = 1,  // Mother moon, guide us through this darkness.
    SAY_TYRANDE_VIAL            = 2,  // Illidan, what is in that vial? What are you doing?
    SAY_TYRANDE_MIRROR          = 3,  // I cannot strike them!  What is this demon magic?
    SAY_TYRANDE_HANDLE_DEMONS   = 4,  // I will handle the demons. Elune, guide my arrows!
    SAY_TYRANDE_FLAYED          = 5,  // Light of Elune, save me!
    EMOTE_TYRANDE_OVERWHELMED   = 6,  // Tyrande is overwhelmed! Use the Blessing of Elune ...
    EMOTE_TYRANDE_HOLDS_OWN     = 7,  // Tyrande can hold her own once again!
    SAY_TYRANDE_HOLD_THEM       = 8,  // I will hold them back for now!
    SAY_TYRANDE_OUT_OF_ARROWS   = 9,  // Illidan, I am out of arrows! ...
    EMOTE_TYRANDE_IMBUED        = 10, // Tyrande is imbued with the shining light ...
    SAY_TYRANDE_TOO_MANY        = 11, // There are too many of them!
    EMOTE_TYRANDE_COLLAPSES     = 12, // Tyrande collapses!  The Light of Elune winks out!
    SAY_TYRANDE_VICTORY         = 13, // Malfurion, he has done it! The portal is collapsing!
    SAY_TYRANDE_EPILOGUE_1      = 14, // Malfurion...
    SAY_TYRANDE_EPILOGUE_2      = 15, // By the very edge...
    SAY_TYRANDE_EPILOGUE_3      = 16, // I do not know who you are, but I thank you. ...

    // Malfurion (55570)
    SAY_MALFURION_INTRO_1       = 0,  // It is being maintained by the will of a powerful demon...Mannoroth.
    SAY_MALFURION_INTRO_2       = 1,  // I cannot break his will alone...
    EMOTE_MALFURION_SOUL        = 2,  // The Dragon Soul's link to the portal has been broken! ...
    SAY_MALFURION_EPILOGUE_1    = 3,  // Hush, Tyrande. Where is Illidan?
    SAY_MALFURION_EPILOGUE_2    = 4,  // Illidan! The well is out of control!
    SAY_MALFURION_EPILOGUE_3    = 5,  // Not if we're caught up in it! ...
    SAY_MALFURION_EPILOGUE_4    = 6,  // This way!

    // Varo'then's Magical Blade (55837)
    EMOTE_SWORD_FALLS           = 0,  // Varo'then's magical sword falls to the ground!

    // Chromie (57913)
    SAY_CHROMIE_ARRIVAL         = 0,  // Did I miss anything? Oh WOW!
    SAY_CHROMIE_LOOT            = 1,  // We've gathered up some items from this time period. ...

    // Nozdormu finale (56102) - groups 0-3 belong to Dragon Soul Madness, ours is 4
    SAY_NOZDORMU_FINALE         = 4   // The Dragon Soul is safe once again. ...
};

// File-local cross-AI actions (shared header actions occupy 1-5)
enum Actions
{
    ACTION_ENGAGE               = 10, // Mannoroth -> allies: pull
    ACTION_RESTAGE              = 11, // Mannoroth -> allies: wipe/evade restage
    ACTION_FAST_FORWARD_INTRO   = 12, // Mannoroth -> Illidan: players pulled before RP finished
    ACTION_START_BLESSING       = 13, // Illidan -> Tyrande: begin Blessing of Elune pulses
    ACTION_VAROTHEN_DIED        = 14, // Varo'then -> Mannoroth: drop the sword
    ACTION_BLADE_EMBEDDED       = 15, // ground sword -> Mannoroth: begin the arc phase
    ACTION_ARM_ENCOUNTER        = 16, // Illidan -> Varo'then: RP done, proximity pull live
    ACTION_STAGE_THREE          = 17, // Mannoroth -> Illidan/Tyrande
    ACTION_MANNOROTH_DEAD       = 18  // Mannoroth -> allies: finale RP
};

enum Data
{
    // boss_mannoroth AI storage, survives ally respawns (volatile across map reload)
    DATA_INTRO_STATE            = 1
};

enum IntroState
{
    INTRO_NOT_STARTED   = 0,
    INTRO_RUNNING       = 1,
    INTRO_DONE          = 2
};

enum Events
{
    // Mannoroth
    EVENT_FELBLADE = 1,
    EVENT_FEL_FIRESTORM,
    EVENT_NETHER_PORTAL,
    EVENT_SUMMON_DEBILITATORS,

    // Captain Varo'then
    EVENT_MAGISTRIKE
};

enum MovePoints
{
    POINT_ROAD_BASE     = 10, // + road waypoint index
    POINT_STAGING       = 20,
    POINT_DEMON_LANDING = 30,
    POINT_FLAY          = 31
};

enum TaskGroups
{
    // Illidan
    GROUP_INTRO         = 1,
    GROUP_WATERS        = 2,
    GROUP_TANK_LOCK     = 3,

    // Tyrande
    GROUP_BLESSING      = 1,
    GROUP_LANE          = 2,
    GROUP_WRATH         = 3,
    GROUP_FINALE        = 4,

    // Mannoroth
    GROUP_ENCOUNTER     = 1
};

uint8 constexpr MaxRoadWaypoints        = 4;
uint8 constexpr MaxDevastators          = 8;  // live cap for the Tyrande stream // tune
uint8 constexpr MaxWaveAddsPerEntry     = 12; // stage 3 soft cap per entry      // tune
float constexpr IntroTriggerRange       = 40.f;
float constexpr VarothenPullRange       = 15.f;

// Shore road from the drake landing towards the arena - approximated along the
// coast (Z values snapped to nearby sniffed trash spawns). // walkthrough: smooth
Position const RoadWaypoints[MaxRoadWaypoints] =
{
    { 3200.72f, -5598.49f, 15.41f }, // shore road, first felhound packs
    { 3239.16f, -5638.94f, 14.21f },
    { 3260.00f, -5660.00f, 14.30f }, // vial RP point
    { 3300.00f, -5700.00f, 14.50f }  // arena approach
};

// Combat staging posts (approximate, tuned around the sniffed lane geometry)
Position const IllidanStagingPos    = { 3313.00f, -5702.00f, 15.40f, 5.90f };
Position const TyrandeStagingPos    = { 3311.50f, -5680.50f, 14.60f, 0.95f };
Position const MalfurionStagingPos  = { 3290.50f, -5672.50f, 15.80f, 5.50f };

// Doomguard Devastators pour out of the well interior towards Tyrande (sniff §5)
Position const DevastatorSpawnPos   = { 3364.00f, -5551.50f, 24.30f, 4.20f };

// Stage 3 wave portal (sniffed Felhound/Felguard spawn cluster)
Position const PortalWavePos        = { 3339.50f, -5698.00f, 13.00f, 4.60f };

// Dreadlord Debilitators spawn on the two GP bunny markers (sniff §5)
Position const DebilitatorSpawnPos[2] =
{
    { 3295.438f, -5687.229f, 14.189f, 5.7421f },
    { 3324.479f, -5694.274f, 13.962f, 3.1765f }
};

// Inferno impact points (sniffed infernal landings)
Position const InfernoImpactPos[6] =
{
    { 3340.351f, -5752.084f, 15.161f, 2.6526f },
    { 3331.848f, -5744.104f, 15.393f, 2.6526f },
    { 3368.153f, -5740.505f, 14.918f, 2.6526f },
    { 3356.303f, -5715.451f, 15.044f, 2.6526f },
    { 3368.832f, -5737.917f, 14.801f, 2.6526f },
    { 3369.183f, -5734.729f, 14.790f, 2.6526f }
};

Position const ChromieSpawnPos      = { 3355.967f, -5743.554f, 15.242f, 3.2114f };
Position const NozdormuSpawnPos     = { 3447.446f, -5418.628f, 99.298f, 4.2411f };

uint32 const WaveDemonEntries[]     = { NPC_DOOMGUARD_DEVASTATOR, NPC_FELHOUND_WAVE, NPC_FELGUARD_WAVE, NPC_INFERNAL };

bool IsLesserDemon(uint32 entry)
{
    switch (entry)
    {
        case NPC_DOOMGUARD_DEVASTATOR:
        case NPC_DREADLORD_DEBILITATOR:
        case NPC_FELHOUND_WAVE:
        case NPC_FELGUARD_WAVE:
        case NPC_INFERNAL:
        case NPC_VORACIOUS_FELHOUND:
        case NPC_SHADOWBAT_MIRROR:
            return true;
        default:
            return false;
    }
}

Player* SelectRandomPlayer(Creature* source, float range)
{
    std::vector<Player*> candidates;
    for (MapReference const& ref : source->GetMap()->GetPlayers())
    {
        Player* player = ref.GetSource();
        if (player && player->IsAlive() && !player->IsGameMaster() && source->IsWithinDist(player, range))
            candidates.push_back(player);
    }

    if (candidates.empty())
        return nullptr;

    return Trinity::Containers::SelectRandomContainerElement(candidates);
}

bool AnyPlayerAlive(Creature* source, float range)
{
    for (MapReference const& ref : source->GetMap()->GetPlayers())
    {
        Player* player = ref.GetSource();
        if (player && player->IsAlive() && !player->IsGameMaster() && source->IsWithinDist(player, range, false))
            return true;
    }

    return false;
}

// Instance ObjectData accessor with a grid fallback so the encounter degrades
// gracefully if a singleton actor is momentarily unavailable.
Creature* GetActor(WorldObject* source, InstanceScript* instance, uint32 dataId, uint32 entry)
{
    if (Creature* creature = instance->GetCreature(dataId))
        return creature;

    if (Creature* creature = source->FindNearestCreature(entry, 500.f))
        return creature;

    return source->FindNearestCreature(entry, 500.f, false); // dead fallback
}

void KillWaveAdds(Creature* killer, std::initializer_list<uint32> entries)
{
    for (uint32 entry : entries)
    {
        std::list<Creature*> adds;
        killer->GetCreatureListWithEntryInGrid(adds, entry, 250.f);
        for (Creature* add : adds)
            if (add->IsAlive())
                Unit::Kill(killer, add);
    }
}

struct boss_mannoroth : public BossAI
{
    boss_mannoroth(Creature* creature) : BossAI(creature, BOSS_MANNOROTH_AND_VAROTHEN),
        _introState(INTRO_NOT_STARTED), _stageThree(false), _bladeEmbedded(false), _swordDropped(false),
        _felDrainDone(false), _defeated(false), _evading(false), _firestormCount(0),
        _devastatorsAlive(0), _debilitatorsAlive(0), _felhoundsAlive(0), _felguardsAlive(0), _playerDamage(0) { }

    void JustAppeared() override
    {
        // The shipped template carries assorted immunity flags; the script owns pull gating.
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_SELECTABLE);
        _Reset();
    }

    void Reset() override
    {
        _Reset();
        scheduler.CancelAll();
        _stageThree = false;
        _bladeEmbedded = false;
        _swordDropped = false;
        _felDrainDone = false;
        _defeated = false;
        _firestormCount = 0;
        _devastatorsAlive = 0;
        _debilitatorsAlive = 0;
        _felhoundsAlive = 0;
        _felguardsAlive = 0;
        _playerDamage = 0;
        me->SetReactState(REACT_PASSIVE); // Illidan is hand-fed as the only tank
        instance->SetData(DATA_FEL_DRAIN_TRIGGERED, 0);
    }

    uint32 GetData(uint32 type) const override
    {
        if (type == DATA_INTRO_STATE)
            return _introState;

        return 0;
    }

    void SetData(uint32 type, uint32 value) override
    {
        if (type == DATA_INTRO_STATE)
            _introState = uint8(value);
    }

    void JustEngagedWith(Unit* who) override
    {
        BossAI::JustEngagedWith(who);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 1);
        Talk(SAY_MANNOROTH_AGGRO);

        // Players skipped or outran the shore RP - snap the trio to their posts.
        if (_introState != INTRO_DONE)
        {
            _introState = INTRO_DONE;
            if (Creature* illidan = GetIllidan())
                if (illidan->IsAIEnabled())
                    illidan->AI()->DoAction(ACTION_FAST_FORWARD_INTRO);
        }

        if (Creature* varothen = GetVarothen())
            if (varothen->IsAlive() && !varothen->IsInCombat() && varothen->IsAIEnabled())
                varothen->AI()->DoZoneInCombat();

        // Illidan's tank lock: massive threat seed, watched below.
        if (Creature* illidan = GetIllidan())
        {
            AddThreat(illidan, 50000000.f);
            AttackStart(illidan);
            if (illidan->IsAIEnabled())
                illidan->AI()->DoAction(ACTION_ENGAGE);
        }

        if (Creature* tyrande = GetTyrande())
            if (tyrande->IsAIEnabled())
                tyrande->AI()->DoAction(ACTION_ENGAGE);

        events.ScheduleEvent(EVENT_FELBLADE, 5s);
        events.ScheduleEvent(EVENT_FEL_FIRESTORM, 15s);       // DBM: first at 15 s
        events.ScheduleEvent(EVENT_NETHER_PORTAL, 60s);       // sniff: +60 s demonic yell

        // Doomguard Devastator stream at Tyrande for the whole of stages 1-2
        scheduler.Schedule(3s, GROUP_ENCOUNTER, [this](TaskContext task)
        {
            if (!_stageThree && _devastatorsAlive < MaxDevastators)
                SummonPortalDemon(NPC_DOOMGUARD_DEVASTATOR, DevastatorSpawnPos);
            task.Repeat(2s + 500ms, 4s);
        });

        // Tank-lock watchdog: Mannoroth must never turn onto players.
        scheduler.Schedule(2s, GROUP_ENCOUNTER, [this](TaskContext task)
        {
            if (Creature* illidan = GetIllidan())
            {
                if (illidan->IsAlive())
                {
                    if (me->GetVictim() != illidan)
                    {
                        AddThreat(illidan, 50000000.f);
                        AttackStart(illidan);
                    }
                    if (illidan->GetVictim() != me && illidan->IsAIEnabled())
                        illidan->AI()->AttackStart(me);
                }
            }
            task.Repeat(2s);
        });

        // Wipe watchdog: Illidan tanks forever, so the encounter must notice a
        // dead party by itself.
        scheduler.Schedule(3s, GROUP_ENCOUNTER, [this](TaskContext task)
        {
            if (!AnyPlayerAlive(me, 300.f))
            {
                EnterEvadeMode(EVADE_REASON_NO_HOSTILES);
                return;
            }
            task.Repeat(3s);
        });
    }

    void DamageTaken(Unit* attacker, uint32& damage) override
    {
        if (_defeated)
        {
            if (damage >= me->GetHealth())
                damage = me->GetHealth() - 1; // never die twice while the fatal-blow block runs
            return;
        }

        // Fel Drain: too much player damage while Varo'then still lives.
        if (!_felDrainDone && attacker && attacker->IsControlledByPlayer())
        {
            if (Creature* varothen = GetVarothen())
            {
                if (varothen->IsAlive())
                {
                    _playerDamage += damage;
                    if (_playerDamage > CalculatePct(me->GetMaxHealth(), 10)) // tune - threshold for the sacrifice
                        TriggerFelDrain(varothen);
                }
            }
        }

        if (!_stageThree && me->HealthBelowPctDamaged(20, damage))
            StartStageThree();

        // The killing blow: fire everything that needs a living caster before death lands.
        if (damage >= me->GetHealth())
        {
            _defeated = true;
            DoCastAOE(SPELL_MANNOROTH_ACHIEVEMENT_SPELL, true); // LFG credit + achievements 6118/6121/6070
            DoCastSelf(SPELL_MANNOROTH_FINALE_ANIMKIT, true);   // death-throes cosmetic
        }
    }

    void DoAction(int32 action) override
    {
        switch (action)
        {
            case ACTION_VAROTHEN_DIED:
                DropMagicalBlade();
                break;
            case ACTION_BLADE_EMBEDDED:
                EmbedBlade();
                break;
            default:
                break;
        }
    }

    void JustSummoned(Creature* summon) override
    {
        BossAI::JustSummoned(summon);
        switch (summon->GetEntry())
        {
            case NPC_DOOMGUARD_DEVASTATOR:
                ++_devastatorsAlive;
                break;
            case NPC_DREADLORD_DEBILITATOR:
                ++_debilitatorsAlive;
                break;
            case NPC_FELHOUND_WAVE:
                ++_felhoundsAlive;
                break;
            case NPC_FELGUARD_WAVE:
                ++_felguardsAlive;
                break;
            default:
                break;
        }
    }

    void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
    {
        switch (summon->GetEntry())
        {
            case NPC_DOOMGUARD_DEVASTATOR:
                if (_devastatorsAlive)
                    --_devastatorsAlive;
                break;
            case NPC_FELHOUND_WAVE:
                if (_felhoundsAlive)
                    --_felhoundsAlive;
                break;
            case NPC_FELGUARD_WAVE:
                if (_felguardsAlive)
                    --_felguardsAlive;
                break;
            case NPC_DREADLORD_DEBILITATOR:
                if (_debilitatorsAlive)
                    --_debilitatorsAlive;
                // Both flayers down -> Tyrande frees herself (her aura check
                // notices), and the cycle re-arms until stage 3.
                if (!_debilitatorsAlive && !_stageThree && me->IsInCombat())
                    events.ScheduleEvent(EVENT_NETHER_PORTAL, 50s);
                break;
            default:
                break;
        }
    }

    void EnterEvadeMode(EvadeReason why) override
    {
        if (_evading)
            return;

        _evading = true;
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        instance->SetData(DATA_FEL_DRAIN_TRIGGERED, 0);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_GIFT_OF_SARGERAS_PLAYERS);
        me->RemoveAurasDueToSpell(SPELL_EMBEDDED_BLADE_MANNOROTH);
        me->RemoveAurasDueToSpell(SPELL_EMBEDDED_BLADE_RETRIGGER);
        summons.DespawnAll(); // devastators, debilitators, waves, infernals, fel flames, sword, embedded blade
        RestageAllies();
        ScriptedAI::EnterEvadeMode(why); // full heal + home + Reset()
        _evading = false;
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied(); // boss state DONE (instance spawns the Minor Cache spawn group), summons cleaned up
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_GIFT_OF_SARGERAS_PLAYERS);

        // Keep the corpse lootable: no despawn, no portal-pull removal - the
        // collapsing-portal exit cosmetic is deliberately skipped.

        if (Creature* illidan = GetIllidan())
            if (illidan->IsAIEnabled())
                illidan->AI()->DoAction(ACTION_MANNOROTH_DEAD);

        // Tyrande owns the timed finale (a corpse cannot run a scheduler).
        if (Creature* tyrande = GetTyrande())
            if (tyrande->IsAIEnabled())
                tyrande->AI()->DoAction(ACTION_MANNOROTH_DEAD);
    }

    void UpdateAI(uint32 diff) override
    {
        scheduler.Update(diff);

        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_FELBLADE:
                    DoCastSelf(SPELL_FELBLADE);
                    // Sniffed Felburn doublet ~2 s apart, only ever at Illidan
                    scheduler.Schedule(2s, GROUP_ENCOUNTER, [this](TaskContext /*task*/)
                    {
                        if (Creature* illidan = GetIllidan())
                            if (illidan->IsAlive())
                                me->CastSpell(illidan, SPELL_FELBURN, true);
                    });
                    scheduler.Schedule(4s, GROUP_ENCOUNTER, [this](TaskContext /*task*/)
                    {
                        if (Creature* illidan = GetIllidan())
                            if (illidan->IsAlive())
                                me->CastSpell(illidan, SPELL_FELBURN, true);
                    });
                    events.Repeat(17s, 18s + 500ms);
                    break;
                case EVENT_FEL_FIRESTORM:
                    Talk(EMOTE_FEL_FIRESTORM);
                    DoCastSelf(SPELL_FEL_FIRESTORM); // 12 s channel; 300 ms ticks summon 55502
                    ++_firestormCount;
                    // DBM: 29 s cd, only two firestorms before the phase change,
                    // then a ~30 s cycle again once the stage 3 portal is open.
                    if (_firestormCount < 2 || _stageThree)
                        events.Repeat(29s);
                    break;
                case EVENT_NETHER_PORTAL:
                    if (_stageThree)
                        break;
                    Talk(SAY_MANNOROTH_NETHER_PORTAL);
                    CastNetherPortal(SPELL_NETHER_PORTAL);
                    events.ScheduleEvent(EVENT_SUMMON_DEBILITATORS, 10s);
                    break;
                case EVENT_SUMMON_DEBILITATORS:
                    if (_stageThree)
                        break;
                    for (Position const& pos : DebilitatorSpawnPos)
                        me->SummonCreature(NPC_DREADLORD_DEBILITATOR, pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10s);
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
    Creature* GetVarothen() const { return GetActor(me, instance, DATA_CAPTAIN_VAROTHEN, NPC_CAPTAIN_VAROTHEN); }
    Creature* GetIllidan() const { return GetActor(me, instance, DATA_ILLIDAN_FINALE, NPC_ILLIDAN_FINALE); }
    Creature* GetTyrande() const { return GetActor(me, instance, DATA_TYRANDE, NPC_TYRANDE); }
    Creature* GetMalfurion() const { return GetActor(me, instance, DATA_MALFURION, NPC_MALFURION); }

    void CastNetherPortal(uint32 spellId)
    {
        // 104625/105041 both point at the huge flying GP bunny over the well.
        if (Creature* bunny = me->FindNearestCreature(NPC_GP_BUNNY_JMF_FLYING_HUGE, 300.f))
            me->CastSpell(bunny, spellId, true);
        else
            me->CastSpell(me, spellId, true);
    }

    void SummonPortalDemon(uint32 entry, Position const& pos)
    {
        me->SummonCreature(entry, pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5s);
    }

    void TriggerFelDrain(Creature* varothen)
    {
        _felDrainDone = true;
        instance->SetData(DATA_FEL_DRAIN_TRIGGERED, 1); // arms That's Not Canon! (cleared on reset)
        me->CastSpell(varothen, SPELL_FEL_DRAIN, true); // INSTAKILL + full self heal

        // The spell needs a conditions row for its NEARBY_ENTRY target - make
        // the sacrifice stick even if the cast whiffs.
        ObjectGuid varothenGuid = varothen->GetGUID();
        scheduler.Schedule(1s, GROUP_ENCOUNTER, [this, varothenGuid](TaskContext /*task*/)
        {
            if (Creature* varothen = ObjectAccessor::GetCreature(*me, varothenGuid))
                if (varothen->IsAlive())
                    Unit::Kill(me, varothen);
            if (me->IsAlive() && !me->IsFullHealth())
                me->SetFullHealth();
        });
    }

    void DropMagicalBlade()
    {
        if (_swordDropped || instance->GetBossState(BOSS_MANNOROTH_AND_VAROTHEN) != IN_PROGRESS)
            return;

        _swordDropped = true;
        Position dropPos = IllidanStagingPos; // safe fallback
        if (Creature* varothen = GetVarothen())
            dropPos = varothen->GetPosition();

        // 104815/104816 do this natively but from a corpse the cast fails - summon directly.
        me->SummonCreature(NPC_VAROTHENS_MAGICAL_BLADE, dropPos, TEMPSUMMON_MANUAL_DESPAWN);
    }

    void EmbedBlade()
    {
        if (_bladeEmbedded || !me->IsAlive() || !me->IsInCombat())
            return;

        _bladeEmbedded = true;
        Talk(SAY_MANNOROTH_BLADE);
        DoCastSelf(SPELL_EMBEDDED_BLADE_MANNOROTH, true); // marker aura; arc cycle scripted below

        if (Creature* blade = me->SummonCreature(NPC_EMBEDDED_BLADE, me->GetPosition(), TEMPSUMMON_MANUAL_DESPAWN))
            blade->EnterVehicle(me, 0); // seat 0 = the wound

        scheduler.Schedule(5s, GROUP_ENCOUNTER, [this](TaskContext /*task*/)
        {
            if (Creature* illidan = GetIllidan())
                if (illidan->IsAIEnabled())
                    illidan->AI()->Talk(SAY_ILLIDAN_SWORD_PIERCED);
        });

        // Magistrike Arc: random player every 3.5 s; the victim strikes back
        // for 1,000,000 and the blade pulses in lockstep (spell script below).
        scheduler.Schedule(3s + 500ms, GROUP_ENCOUNTER, [this](TaskContext task)
        {
            if (Player* target = SelectRandomPlayer(me, 100.f))
                me->CastSpell(target, SPELL_MAGISTRIKE_ARC, true);
            task.Repeat(3s + 500ms);
        });

        // Fel Fire Nova pulses from the wound - Illidan's Waters of Eternity
        // zone is the -90% fire haven.
        scheduler.Schedule(5s, GROUP_ENCOUNTER, [this](TaskContext task)
        {
            me->CastSpell(me, SPELL_FEL_FIRE_NOVA, true);
            task.Repeat(5s);
        });
    }

    void StartStageThree()
    {
        if (_stageThree)
            return;

        _stageThree = true;
        Talk(SAY_MANNOROTH_STAGE_THREE);
        events.CancelEvent(EVENT_NETHER_PORTAL);
        events.CancelEvent(EVENT_SUMMON_DEBILITATORS);

        scheduler.Schedule(6s, GROUP_ENCOUNTER, [this](TaskContext /*task*/)
        {
            CastNetherPortal(SPELL_NETHER_TEAR);
        });

        scheduler.Schedule(12s, GROUP_ENCOUNTER, [this](TaskContext /*task*/)
        {
            if (Creature* illidan = GetIllidan())
                if (illidan->IsAIEnabled())
                    illidan->AI()->Talk(EMOTE_ILLIDAN_PORTAL_OPENS);
            // 105053/105057 are the era periodic-summon self auras; their
            // summon triggers use unconditioned dest-entry targeting, so the
            // waves are scheduled here instead.
        });

        scheduler.Schedule(14s, GROUP_ENCOUNTER, [this](TaskContext /*task*/)
        {
            Talk(EMOTE_FELGUARD_PORTAL);
        });

        // Felhound every 3 s, Felguard every 4 s (105053/105057 periods)
        scheduler.Schedule(15s, GROUP_ENCOUNTER, [this](TaskContext task)
        {
            if (_felhoundsAlive < MaxWaveAddsPerEntry)
                SummonPortalDemon(NPC_FELHOUND_WAVE, PortalWavePos);
            task.Repeat(3s);
        });
        scheduler.Schedule(17s, GROUP_ENCOUNTER, [this](TaskContext task)
        {
            if (_felguardsAlive < MaxWaveAddsPerEntry)
                SummonPortalDemon(NPC_FELGUARD_WAVE, PortalWavePos);
            task.Repeat(4s);
        });

        // Fel Firestorm resumes (sniff: third cast at stage 3 +21 s)
        events.CancelEvent(EVENT_FEL_FIRESTORM);
        events.ScheduleEvent(EVENT_FEL_FIRESTORM, 21s);

        if (Creature* illidan = GetIllidan())
            if (illidan->IsAIEnabled())
                illidan->AI()->DoAction(ACTION_STAGE_THREE);

        if (Creature* tyrande = GetTyrande())
            if (tyrande->IsAIEnabled())
                tyrande->AI()->DoAction(ACTION_STAGE_THREE);

        scheduler.Schedule(37s, GROUP_ENCOUNTER, [this](TaskContext /*task*/)
        {
            Talk(SAY_MANNOROTH_BURNING_EYES);
        });

        // Inferno! at +51 s: six missiles over ~3 s, an infernal on every impact
        scheduler.Schedule(51s, GROUP_ENCOUNTER, [this](TaskContext /*task*/)
        {
            me->CastSpell(me, SPELL_INFERNO_SELF, true);
            Talk(EMOTE_INFERNALS);
            for (uint8 i = 0; i < 6; ++i)
            {
                Position const& impact = InfernoImpactPos[i];
                scheduler.Schedule(Milliseconds(500 * i), GROUP_ENCOUNTER, [this, impact](TaskContext /*task*/)
                {
                    // 105145's own trigger (105146) is absent from the 4.3.4 DBC -
                    // the missile is fired for the visual, the summon is scripted.
                    me->CastSpell(impact, SPELL_INFERNO_MISSILE, true);
                });
                scheduler.Schedule(Milliseconds(2000 + 500 * i), GROUP_ENCOUNTER, [this, impact](TaskContext /*task*/)
                {
                    me->SummonCreature(NPC_INFERNAL, impact, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5s);
                });
            }
        });
    }

    void RestageAllies()
    {
        if (Creature* varothen = GetVarothen())
        {
            if (!varothen->IsAlive())
                varothen->Respawn(true);
            else if (varothen->IsInCombat() && varothen->IsAIEnabled())
                varothen->AI()->EnterEvadeMode(EVADE_REASON_OTHER);
        }

        for (uint32 dataId : { uint32(DATA_ILLIDAN_FINALE), uint32(DATA_TYRANDE), uint32(DATA_MALFURION) })
        {
            uint32 entry = dataId == DATA_ILLIDAN_FINALE ? NPC_ILLIDAN_FINALE : (dataId == DATA_TYRANDE ? NPC_TYRANDE : NPC_MALFURION);
            if (Creature* ally = GetActor(me, instance, dataId, entry))
            {
                if (!ally->IsAlive())
                    ally->Respawn(true);
                if (ally->IsAIEnabled())
                    ally->AI()->DoAction(ACTION_RESTAGE);
            }
        }
    }

    uint8 _introState;
    bool _stageThree;
    bool _bladeEmbedded;
    bool _swordDropped;
    bool _felDrainDone;
    bool _defeated;
    bool _evading;
    uint8 _firestormCount;
    uint8 _devastatorsAlive;
    uint8 _debilitatorsAlive;
    uint8 _felhoundsAlive;
    uint8 _felguardsAlive;
    uint64 _playerDamage;
};

struct boss_captain_varothen : public ScriptedAI
{
    boss_captain_varothen(Creature* creature) : ScriptedAI(creature), _armed(false), _evading(false)
    {
        _instance = creature->GetInstanceScript();
    }

    void JustAppeared() override
    {
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_NOT_SELECTABLE);
        SyncArmedState();
    }

    void Reset() override
    {
        _events.Reset();
        _scheduler.CancelAll();
        SyncArmedState();
    }

    void DoAction(int32 action) override
    {
        // The Highguard yell already ran during the shore RP - arming is silent.
        if (action == ACTION_ARM_ENCOUNTER)
            _armed = true;
    }

    void MoveInLineOfSight(Unit* who) override
    {
        if (_armed && !me->IsInCombat() && who->IsPlayer() && !who->ToPlayer()->IsGameMaster()
            && me->IsWithinDistInMap(who, VarothenPullRange) && me->IsValidAttackTarget(who))
        {
            AttackStart(who);
            return;
        }

        ScriptedAI::MoveInLineOfSight(who);
    }

    void JustEngagedWith(Unit* who) override
    {
        ScriptedAI::JustEngagedWith(who);
        _armed = true;
        Talk(SAY_VAROTHEN_AGGRO);
        _instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 2);

        // Chain the main boss (and thereby the allies) into the fight.
        if (Creature* mannoroth = GetActor(me, _instance, BOSS_MANNOROTH_AND_VAROTHEN, NPC_MANNOROTH))
            if (mannoroth->IsAlive() && !mannoroth->IsInCombat() && mannoroth->IsAIEnabled())
                mannoroth->AI()->DoZoneInCombat();

        _events.ScheduleEvent(EVENT_MAGISTRIKE, 3s);
    }

    void EnterEvadeMode(EvadeReason why) override
    {
        if (_evading)
            return;

        _evading = true;
        _instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

        // A wipe on either boss resets the whole encounter.
        if (Creature* mannoroth = GetActor(me, _instance, BOSS_MANNOROTH_AND_VAROTHEN, NPC_MANNOROTH))
            if (mannoroth->IsAlive() && mannoroth->IsInCombat() && mannoroth->IsAIEnabled())
                mannoroth->AI()->EnterEvadeMode(why);

        ScriptedAI::EnterEvadeMode(why);
        _evading = false;
    }

    void JustDied(Unit* /*killer*/) override
    {
        Talk(SAY_VAROTHEN_DEATH);
        _instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

        // Documenting the Timeways tracks a credit dummy (57858), not his real
        // entry; a Fel Drain kill (Mannoroth as killer) must credit the group too.
        for (auto const& ref : me->GetMap()->GetPlayers())
            if (Player* player = ref.GetSource())
                player->KilledMonsterCredit(57858);

        // Sword drop chain also runs when Fel Drain consumed him.
        if (Creature* mannoroth = GetActor(me, _instance, BOSS_MANNOROTH_AND_VAROTHEN, NPC_MANNOROTH))
            if (mannoroth->IsAIEnabled())
                mannoroth->AI()->DoAction(ACTION_VAROTHEN_DIED);
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
                case EVENT_MAGISTRIKE:
                    if (Player* target = SelectRandomPlayer(me, 60.f))
                        DoCast(target, SPELL_MAGISTRIKE);
                    _events.Repeat(9s + 500ms, 10s + 500ms);
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
    void SyncArmedState()
    {
        // Armed state lives on Mannoroth's AI so it survives Varo'then's own
        // respawn after a Fel Drain wipe. Delayed - creature load order at map
        // load is not guaranteed.
        _scheduler.Schedule(2s, [this](TaskContext /*task*/)
        {
            if (Creature* mannoroth = GetActor(me, _instance, BOSS_MANNOROTH_AND_VAROTHEN, NPC_MANNOROTH))
                if (mannoroth->IsAIEnabled() && mannoroth->AI()->GetData(DATA_INTRO_STATE) == INTRO_DONE)
                    _armed = true;
        });
    }

    InstanceScript* _instance;
    EventMap _events;
    TaskScheduler _scheduler;
    bool _armed;
    bool _evading;
};

struct npc_woe_illidan_finale : public ScriptedAI
{
    npc_woe_illidan_finale(Creature* creature) : ScriptedAI(creature), _introTriggered(false), _watersActive(false), _currentLeg(0)
    {
        _instance = creature->GetInstanceScript();
    }

    void JustAppeared() override
    {
        DoCastSelf(SPELL_DEMONIC_SIGHT, true); // 90% dodge - he holds Mannoroth all fight
        me->SetReactState(REACT_PASSIVE);

        // Rejoin a completed intro after a mid-run respawn or grid reload.
        _scheduler.Schedule(2s, [this](TaskContext /*task*/)
        {
            if (!_introTriggered && GetIntroState() == INTRO_DONE)
            {
                _introTriggered = true;
                SnapToStagingPost();
            }
        });
    }

    void Reset() override
    {
        if (!me->HasAura(SPELL_DEMONIC_SIGHT))
            DoCastSelf(SPELL_DEMONIC_SIGHT, true);
    }

    void MoveInLineOfSight(Unit* who) override
    {
        if (_introTriggered || !who->IsPlayer() || who->ToPlayer()->IsGameMaster())
            return;

        if (!me->IsWithinDistInMap(who, IntroTriggerRange))
            return;

        if (_instance->GetBossState(BOSS_QUEEN_AZSHARA) != DONE || _instance->GetBossState(BOSS_MANNOROTH_AND_VAROTHEN) == DONE)
            return;

        if (GetIntroState() != INTRO_NOT_STARTED)
        {
            _introTriggered = true; // someone else already drove it
            return;
        }

        StartIntro();
    }

    void DoAction(int32 action) override
    {
        switch (action)
        {
            case ACTION_ENGAGE:
                EngageMannoroth();
                break;
            case ACTION_FAST_FORWARD_INTRO:
                FastForwardIntro();
                break;
            case ACTION_STAGE_THREE:
                _scheduler.Schedule(16s, [this](TaskContext /*task*/)
                {
                    Talk(SAY_ILLIDAN_STILL_CONNECTED);
                    // Gift of Sargeras - era mechanic absent from the modern sniff
                    DoCastSelf(SPELL_GIFT_OF_SARGERAS_ILLIDAN, true);
                    DoCastAOE(SPELL_GIFT_OF_SARGERAS_PLAYERS, true);
                });
                break;
            case ACTION_MANNOROTH_DEAD:
                _scheduler.CancelGroup(GROUP_TANK_LOCK);
                _scheduler.CancelGroup(GROUP_WATERS);
                me->RemoveAurasDueToSpell(SPELL_GIFT_OF_SARGERAS_ILLIDAN);
                me->AttackStop();
                me->SetReactState(REACT_PASSIVE);
                break;
            case ACTION_RESTAGE:
                RestageSelf();
                break;
            default:
                break;
        }
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != POINT_MOTION_TYPE)
            return;

        if (pointId >= POINT_ROAD_BASE && pointId < POINT_ROAD_BASE + MaxRoadWaypoints)
        {
            uint8 leg = uint8(pointId - POINT_ROAD_BASE);
            switch (leg)
            {
                case 0:
                    ResolveLeg(1);
                    break;
                case 1:
                    Talk(SAY_ILLIDAN_ROAD);
                    ResolveLeg(2);
                    break;
                case 2:
                    StartVialScene();
                    break;
                case 3:
                    TakeStagingPositions();
                    break;
                default:
                    break;
            }
        }
        else if (pointId == POINT_STAGING)
        {
            me->SetHomePosition(IllidanStagingPos);
            me->SetFacingTo(IllidanStagingPos.GetOrientation());
            Talk(SAY_ILLIDAN_HANDLE_VAROTHEN);
            ArmEncounter();
        }
    }

    void JustDied(Unit* /*killer*/) override
    {
        // Ally death during the encounter is a raid wipe.
        if (_instance->GetBossState(BOSS_MANNOROTH_AND_VAROTHEN) == IN_PROGRESS)
            if (Creature* mannoroth = GetMannoroth())
                if (mannoroth->IsAlive() && mannoroth->IsAIEnabled())
                    mannoroth->AI()->EnterEvadeMode(EVADE_REASON_OTHER);
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }

private:
    Creature* GetMannoroth() const { return GetActor(me, _instance, BOSS_MANNOROTH_AND_VAROTHEN, NPC_MANNOROTH); }
    Creature* GetTyrande() const { return GetActor(me, _instance, DATA_TYRANDE, NPC_TYRANDE); }
    Creature* GetMalfurion() const { return GetActor(me, _instance, DATA_MALFURION, NPC_MALFURION); }
    Creature* GetVarothen() const { return GetActor(me, _instance, DATA_CAPTAIN_VAROTHEN, NPC_CAPTAIN_VAROTHEN); }

    uint8 GetIntroState() const
    {
        if (Creature* mannoroth = GetMannoroth())
            if (mannoroth->IsAIEnabled())
                return uint8(mannoroth->AI()->GetData(DATA_INTRO_STATE));
        return INTRO_NOT_STARTED;
    }

    void SetIntroState(uint8 state)
    {
        if (Creature* mannoroth = GetMannoroth())
            if (mannoroth->IsAIEnabled())
                mannoroth->AI()->SetData(DATA_INTRO_STATE, state);
    }

    // ---------------------------------------------------------------- intro

    void StartIntro()
    {
        _introTriggered = true;
        SetIntroState(INTRO_RUNNING);

        // Portal-court dialogue (timestamps from the sniff, relative seconds)
        _scheduler.Schedule(1s, GROUP_INTRO, [this](TaskContext /*task*/) { Talk(SAY_ILLIDAN_INTRO_1); });
        _scheduler.Schedule(7s, GROUP_INTRO, [this](TaskContext /*task*/) { TalkAlly(GetMalfurion(), SAY_MALFURION_INTRO_1); });
        _scheduler.Schedule(13s, GROUP_INTRO, [this](TaskContext /*task*/) { TalkAlly(GetMalfurion(), SAY_MALFURION_INTRO_2); });
        _scheduler.Schedule(17s, GROUP_INTRO, [this](TaskContext /*task*/) { Talk(SAY_ILLIDAN_INTRO_2); });
        _scheduler.Schedule(23s, GROUP_INTRO, [this](TaskContext /*task*/) { TalkAlly(GetTyrande(), SAY_TYRANDE_INTRO_1); });
        _scheduler.Schedule(29s, GROUP_INTRO, [this](TaskContext /*task*/) { Talk(SAY_ILLIDAN_INTRO_3); });
        _scheduler.Schedule(34s, GROUP_INTRO, [this](TaskContext /*task*/) { TalkAlly(GetTyrande(), SAY_TYRANDE_INTRO_2); });
        _scheduler.Schedule(39s, GROUP_INTRO, [this](TaskContext /*task*/)
        {
            if (Creature* tyrande = GetTyrande())
                if (tyrande->IsAIEnabled())
                    tyrande->AI()->DoAction(ACTION_START_BLESSING);
        });
        _scheduler.Schedule(41s, GROUP_INTRO, [this](TaskContext /*task*/) { BeginRoadLeg(0); });
    }

    void TalkAlly(Creature* ally, uint8 group)
    {
        if (ally && ally->IsAIEnabled())
            ally->AI()->Talk(group);
    }

    void BeginRoadLeg(uint8 leg)
    {
        _currentLeg = leg;
        me->SetReactState(REACT_AGGRESSIVE);
        me->GetMotionMaster()->MovePoint(POINT_ROAD_BASE + leg, RoadWaypoints[leg]);

        if (Creature* tyrande = GetTyrande())
            tyrande->GetMotionMaster()->MoveFollow(me, 4.f, 2.4f);
        if (Creature* malfurion = GetMalfurion())
            malfurion->GetMotionMaster()->MoveFollow(me, 4.f, 3.9f);
    }

    // Fight the felhound packs (and the Doomguard Annihilators squatting on
    // the road) blocking a leg; a hard timeout keeps the RP moving even if a
    // pack refuses to die.
    void ResolveLeg(uint8 nextLeg)
    {
        _scheduler.Schedule(2s, GROUP_INTRO, [this, nextLeg](TaskContext task)
        {
            Creature* demon = FindRoadDemon();
            if (demon && task.GetRepeatCounter() < 15)
            {
                FightRoadDemon(demon);
                task.Repeat(3s);
                return;
            }

            me->AttackStop();
            me->RemoveAurasDueToSpell(SPELL_AURA_OF_IMMOLATION);
            BeginRoadLeg(nextLeg);
        });
    }

    Creature* FindRoadDemon() const
    {
        if (Creature* hound = me->FindNearestCreature(NPC_VORACIOUS_FELHOUND, 35.f))
            return hound;
        return me->FindNearestCreature(NPC_DOOMGUARD_ANNIHILATOR, 35.f);
    }

    void FightRoadDemon(Creature* demon)
    {
        if (!me->GetVictim())
        {
            AttackStart(demon);
            me->CastSpell(demon, SPELL_DEMON_RUSH, true);
        }

        if (!me->HasAura(SPELL_AURA_OF_IMMOLATION))
            DoCastSelf(SPELL_AURA_OF_IMMOLATION, true); // pulses 104387 every 3 s

        me->CastSpell(demon, SPELL_DARKLANCE, true);

        if (Creature* tyrande = GetTyrande())
            tyrande->CastSpell(demon, SPELL_LUNAR_SHOT_ESCORT, true);

        // Anti-stall: Illidan executes wounded demons so the escort cannot wedge.
        if (demon->HealthBelowPct(25))
            Unit::Kill(me, demon);
    }

    void StartVialScene()
    {
        me->AttackStop();
        me->SetReactState(REACT_PASSIVE);

        _scheduler.Schedule(1s, GROUP_INTRO, [this](TaskContext /*task*/) { Talk(SAY_ILLIDAN_VIAL_FUN); });
        _scheduler.Schedule(2s, GROUP_INTRO, [this](TaskContext /*task*/) { TalkAlly(GetTyrande(), SAY_TYRANDE_VIAL); });
        _scheduler.Schedule(5s, GROUP_INTRO, [this](TaskContext /*task*/) { Talk(SAY_ILLIDAN_VIAL_IDEA); });
        _scheduler.Schedule(15s, GROUP_INTRO, [this](TaskContext /*task*/)
        {
            Talk(SAY_ILLIDAN_VIAL_PEOPLE);
            Talk(EMOTE_ILLIDAN_WATERS);
            StartWaters();
        });
        _scheduler.Schedule(25s, GROUP_INTRO, [this](TaskContext /*task*/) { Talk(SAY_ILLIDAN_VIAL_POWER); });

        // Mannoroth notices - Varo'then raises the highguard.
        _scheduler.Schedule(38s, GROUP_INTRO, [this](TaskContext /*task*/)
        {
            if (Creature* mannoroth = GetMannoroth())
                if (mannoroth->IsAIEnabled())
                    mannoroth->AI()->Talk(SAY_MANNOROTH_INTRO);
        });
        _scheduler.Schedule(44s, GROUP_INTRO, [this](TaskContext /*task*/)
        {
            TalkAlly(GetVarothen(), SAY_VAROTHEN_HIGHGUARD);
            StartHighguardWave();
        });
        _scheduler.Schedule(65s, GROUP_INTRO, [this](TaskContext /*task*/) { TalkAlly(GetTyrande(), SAY_TYRANDE_MIRROR); });
        _scheduler.Schedule(69s, GROUP_INTRO, [this](TaskContext /*task*/) { Talk(SAY_ILLIDAN_MIRROR_HINT); });

        // Advance once the two Highguard Elites are down (2 min hard timeout).
        _scheduler.Schedule(50s, GROUP_INTRO, [this](TaskContext task)
        {
            bool highguardAlive = me->FindNearestCreature(NPC_HIGHGUARD_ELITE, 150.f) != nullptr;
            if (highguardAlive && task.GetRepeatCounter() < 24)
            {
                task.Repeat(5s);
                return;
            }
            BeginRoadLeg(3);
        });
    }

    void StartWaters()
    {
        DoCastSelf(SPELL_WATERS_OF_ETERNITY, true); // native 500 ms pulse of 103954
        if (_watersActive)
            return;

        _watersActive = true;
        // Belt and braces: keep the -90% fire zone alive around Illidan for
        // the whole encounter even if the native periodic drops.
        _scheduler.Schedule(500ms, GROUP_WATERS, [this](TaskContext task)
        {
            if (!me->HasAura(SPELL_WATERS_OF_ETERNITY))
                DoCastSelf(SPELL_WATERS_OF_ETERNITY, true);
            DoCastAOE(SPELL_WATERS_OF_ETERNITY_ZONE, true);
            task.Repeat(500ms);
        });
    }

    void StartHighguardWave()
    {
        // The two Highguard Elites + Shadowbat vehicles are DB spawns beside
        // Varo'then; kick them at the party with the Displacement mirror trick.
        std::list<Creature*> wave;
        me->GetCreatureListWithEntryInGrid(wave, NPC_HIGHGUARD_ELITE, 250.f);

        std::list<Creature*> bats;
        me->GetCreatureListWithEntryInGrid(bats, NPC_SHADOWBAT_VEHICLE, 250.f);
        wave.splice(wave.end(), bats);

        for (Creature* guard : wave)
        {
            if (!guard->IsAlive())
                continue;

            guard->SetImmuneToPC(false);

            if (guard->GetEntry() == NPC_SHADOWBAT_VEHICLE)
            {
                guard->CastSpell(guard, SPELL_DISPLACEMENT, true);
                // Mirror images: two decoy bats per vehicle
                for (uint8 i = 0; i < 2; ++i)
                {
                    Position mirrorPos = guard->GetNearPosition(frand(4.f, 7.f), frand(0.f, float(2 * M_PI)));
                    if (Creature* mirror = guard->SummonCreature(NPC_SHADOWBAT_MIRROR, mirrorPos, TEMPSUMMON_TIMED_DESPAWN, 2min))
                    {
                        mirror->CastSpell(mirror, SPELL_SHADOWBAT_COSMETIC, true);
                        if (mirror->IsAIEnabled())
                            mirror->AI()->DoZoneInCombat();
                    }
                }
            }

            if (guard->IsAIEnabled())
            {
                guard->AI()->DoZoneInCombat();
                if (Player* target = SelectRandomPlayer(guard, 100.f))
                    guard->AI()->AttackStart(target);
            }
        }
    }

    void TakeStagingPositions()
    {
        if (Creature* tyrande = GetTyrande())
        {
            tyrande->GetMotionMaster()->Clear();
            tyrande->GetMotionMaster()->MovePoint(POINT_STAGING, TyrandeStagingPos);
            tyrande->SetHomePosition(TyrandeStagingPos);
        }
        if (Creature* malfurion = GetMalfurion())
        {
            malfurion->GetMotionMaster()->Clear();
            malfurion->GetMotionMaster()->MovePoint(POINT_STAGING, MalfurionStagingPos);
            malfurion->SetHomePosition(MalfurionStagingPos);
        }
        me->GetMotionMaster()->MovePoint(POINT_STAGING, IllidanStagingPos);
    }

    void ArmEncounter()
    {
        SetIntroState(INTRO_DONE);
        if (Creature* varothen = GetVarothen())
            if (varothen->IsAIEnabled())
                varothen->AI()->DoAction(ACTION_ARM_ENCOUNTER);
    }

    void FastForwardIntro()
    {
        _introTriggered = true;
        _scheduler.CancelGroup(GROUP_INTRO);
        me->AttackStop();
        SnapToStagingPost();

        if (Creature* tyrande = GetTyrande())
        {
            tyrande->NearTeleportTo(TyrandeStagingPos);
            tyrande->SetHomePosition(TyrandeStagingPos);
            if (tyrande->IsAIEnabled())
                tyrande->AI()->DoAction(ACTION_START_BLESSING);
        }
        if (Creature* malfurion = GetMalfurion())
        {
            malfurion->GetMotionMaster()->Clear();
            malfurion->NearTeleportTo(MalfurionStagingPos);
            malfurion->SetHomePosition(MalfurionStagingPos);
        }
        if (Creature* varothen = GetVarothen())
            if (varothen->IsAIEnabled())
                varothen->AI()->DoAction(ACTION_ARM_ENCOUNTER);
    }

    void SnapToStagingPost()
    {
        me->GetMotionMaster()->Clear();
        me->NearTeleportTo(IllidanStagingPos);
        me->SetHomePosition(IllidanStagingPos);
        me->SetReactState(REACT_PASSIVE);
        StartWaters();
    }

    // --------------------------------------------------------------- combat

    void EngageMannoroth()
    {
        Creature* mannoroth = GetMannoroth();
        if (!mannoroth)
            return;

        me->SetReactState(REACT_PASSIVE);
        AttackStart(mannoroth);
        StartWaters();

        ObjectGuid mannorothGuid = mannoroth->GetGUID();

        _scheduler.Schedule(6s, GROUP_TANK_LOCK, [this, mannorothGuid](TaskContext task)
        {
            if (Creature* target = ObjectAccessor::GetCreature(*me, mannorothGuid))
                if (target->IsAlive() && target->IsInCombat())
                    me->CastSpell(target, SPELL_TAUNT, true);
            task.Repeat(6s);
        });
        _scheduler.Schedule(4s, GROUP_TANK_LOCK, [this, mannorothGuid](TaskContext task)
        {
            if (Creature* target = ObjectAccessor::GetCreature(*me, mannorothGuid))
                if (target->IsAlive() && target->IsInCombat())
                    me->CastSpell(target, SPELL_DEMON_RUSH, true);
            task.Repeat(3s + 500ms, 5s);
        });
        _scheduler.Schedule(15s, GROUP_TANK_LOCK, [this, mannorothGuid](TaskContext task)
        {
            if (Creature* target = ObjectAccessor::GetCreature(*me, mannorothGuid))
                if (target->IsAlive() && target->IsInCombat())
                    me->CastSpell(target, SPELL_DARKLANCE, true);
            task.Repeat(13s, 19s);
        });
    }

    void RestageSelf()
    {
        _scheduler.CancelGroup(GROUP_TANK_LOCK);
        me->InterruptNonMeleeSpells(false);
        me->AttackStop();
        me->CombatStop(true);
        me->RemoveAurasDueToSpell(SPELL_GIFT_OF_SARGERAS_ILLIDAN);
        _introTriggered = true;
        SnapToStagingPost();
    }

    InstanceScript* _instance;
    TaskScheduler _scheduler;
    bool _introTriggered;
    bool _watersActive;
    uint8 _currentLeg;
};

struct npc_woe_tyrande : public ScriptedAI
{
    npc_woe_tyrande(Creature* creature) : ScriptedAI(creature), _blessingActive(false), _flayed(false), _collapsed(false)
    {
        _instance = creature->GetInstanceScript();
    }

    void JustAppeared() override
    {
        me->SetReactState(REACT_PASSIVE);

        _scheduler.Schedule(2s + 500ms, [this](TaskContext /*task*/)
        {
            // Mid-run respawn/reload after the RP already happened
            if (Creature* mannoroth = GetMannoroth())
                if (mannoroth->IsAIEnabled() && mannoroth->AI()->GetData(DATA_INTRO_STATE) == INTRO_DONE)
                    DoAction(ACTION_START_BLESSING);
        });
    }

    void DoAction(int32 action) override
    {
        switch (action)
        {
            case ACTION_START_BLESSING:
                StartBlessing();
                break;
            case ACTION_ENGAGE:
                StartLane();
                break;
            case ACTION_STAGE_THREE:
                StartStageThree();
                break;
            case ACTION_MANNOROTH_DEAD:
                StartFinale();
                break;
            case ACTION_RESTAGE:
                Restage();
                break;
            default:
                break;
        }
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (_instance->GetBossState(BOSS_MANNOROTH_AND_VAROTHEN) == IN_PROGRESS)
            if (Creature* mannoroth = GetMannoroth())
                if (mannoroth->IsAlive() && mannoroth->IsAIEnabled())
                    mannoroth->AI()->EnterEvadeMode(EVADE_REASON_OTHER);
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

private:
    Creature* GetMannoroth() const { return GetActor(me, _instance, BOSS_MANNOROTH_AND_VAROTHEN, NPC_MANNOROTH); }
    Creature* GetIllidan() const { return GetActor(me, _instance, DATA_ILLIDAN_FINALE, NPC_ILLIDAN_FINALE); }
    Creature* GetMalfurion() const { return GetActor(me, _instance, DATA_MALFURION, NPC_MALFURION); }

    void StartBlessing()
    {
        DoCastSelf(SPELL_BLESSING_OF_ELUNE, true); // native 1 s pulse of 103918

        if (_blessingActive)
            return;

        _blessingActive = true;
        // Scripted 1 s pulse alongside the native one - the party must never
        // drop the Elune's Wrath proc for the whole area.
        _scheduler.Schedule(1s, GROUP_BLESSING, [this](TaskContext task)
        {
            if (!me->HasAura(SPELL_BLESSING_OF_ELUNE))
                DoCastSelf(SPELL_BLESSING_OF_ELUNE, true);
            DoCastAOE(SPELL_BLESSING_OF_ELUNE_PLAYERS, true);
            task.Repeat(1s);
        });
    }

    void StartLane()
    {
        _collapsed = false;
        _flayed = false;
        StartBlessing();

        _scheduler.Schedule(13s, GROUP_LANE, [this](TaskContext /*task*/)
        {
            Talk(SAY_TYRANDE_HANDLE_DEMONS);
        });

        // Lunar Shot: she deletes the devastator stream (retail intent - the
        // shot one-kills wave demons).
        _scheduler.Schedule(2s, GROUP_LANE, [this](TaskContext task)
        {
            if (!_flayed && !_collapsed)
            {
                if (Creature* target = SelectLaneTarget())
                {
                    me->CastSpell(target, SPELL_LUNAR_SHOT, true);
                    ObjectGuid targetGuid = target->GetGUID();
                    _scheduler.Schedule(700ms, GROUP_LANE, [this, targetGuid](TaskContext /*task*/)
                    {
                        if (Creature* victim = ObjectAccessor::GetCreature(*me, targetGuid))
                            if (victim->IsAlive())
                                Unit::Kill(me, victim);
                    });
                }
            }
            task.Repeat(1s + 400ms);
        });

        // Debilitating Flay watcher - the flay channel on her is the state.
        _scheduler.Schedule(1s, GROUP_LANE, [this](TaskContext task)
        {
            bool flayedNow = me->HasAura(SPELL_DEBILITATING_FLAY);
            if (flayedNow && !_flayed)
            {
                _flayed = true;
                Talk(SAY_TYRANDE_FLAYED);
                Talk(EMOTE_TYRANDE_OVERWHELMED);
            }
            else if (!flayedNow && _flayed)
            {
                _flayed = false;
                Talk(EMOTE_TYRANDE_HOLDS_OWN);
                Talk(SAY_TYRANDE_HOLD_THEM);
                DoCastAOE(SPELL_LUNAR_SHOT_AOE, true);
                KillWaveAdds(me, { NPC_DOOMGUARD_DEVASTATOR }); // her lane clear
            }
            task.Repeat(1s);
        });
    }

    Creature* SelectLaneTarget() const
    {
        Creature* best = nullptr;
        for (uint32 entry : { NPC_DOOMGUARD_DEVASTATOR, NPC_FELHOUND_WAVE, NPC_FELGUARD_WAVE })
            if (Creature* candidate = me->FindNearestCreature(entry, 80.f))
                if (!best || me->GetExactDist2d(candidate) < me->GetExactDist2d(best))
                    best = candidate;
        return best;
    }

    void StartStageThree()
    {
        // She keeps shooting until the quiver runs dry (sniff: +26 s), then
        // Elune takes over.
        _scheduler.Schedule(26s, GROUP_WRATH, [this](TaskContext /*task*/)
        {
            _scheduler.CancelGroup(GROUP_LANE);
            Talk(SAY_TYRANDE_OUT_OF_ARROWS);
            DoCastSelf(SPELL_HAND_OF_ELUNE); // 6 s cast, periodic 3 s -> 105073
        });

        _scheduler.Schedule(33s, GROUP_WRATH, [this](TaskContext /*task*/)
        {
            Talk(EMOTE_TYRANDE_IMBUED);
        });

        // Wrath of Elune obliterates the live portal waves every 3 s.
        _scheduler.Schedule(34s, GROUP_WRATH, [this](TaskContext task)
        {
            DoCastAOE(SPELL_WRATH_OF_ELUNE, true);
            KillWaveAdds(me, { NPC_DOOMGUARD_DEVASTATOR, NPC_FELHOUND_WAVE, NPC_FELGUARD_WAVE, NPC_INFERNAL });
            if (task.GetRepeatCounter() < 7)
                task.Repeat(3s);
        });

        // +57 s: the final wrath takes the infernal rain with it, then she collapses.
        _scheduler.Schedule(57s, GROUP_WRATH, [this](TaskContext /*task*/)
        {
            _scheduler.CancelGroup(GROUP_WRATH);
            DoCastAOE(SPELL_WRATH_OF_ELUNE_FINAL, true);
            KillWaveAdds(me, { NPC_DOOMGUARD_DEVASTATOR, NPC_FELHOUND_WAVE, NPC_FELGUARD_WAVE, NPC_INFERNAL });
            Talk(SAY_TYRANDE_TOO_MANY);

            _scheduler.Schedule(2s, GROUP_FINALE, [this](TaskContext /*task*/)
            {
                Talk(EMOTE_TYRANDE_COLLAPSES);
                me->RemoveAurasDueToSpell(SPELL_HAND_OF_ELUNE);
                me->SetStandState(UNIT_STAND_STATE_KNEEL);
                _collapsed = true; // she stops assisting - the burn race is on
            });
        });
    }

    void StartFinale()
    {
        _scheduler.CancelGroup(GROUP_LANE);
        _scheduler.CancelGroup(GROUP_WRATH);
        _scheduler.CancelGroup(GROUP_BLESSING);
        _blessingActive = false;

        Talk(SAY_TYRANDE_VICTORY);

        // Timings shifted from the sniffed finale (t0 = Mannoroth's death).
        _scheduler.Schedule(12s, GROUP_FINALE, [this](TaskContext /*task*/)
        {
            if (Creature* mannoroth = GetMannoroth())
                if (mannoroth->IsAIEnabled())
                    mannoroth->AI()->Talk(SAY_MANNOROTH_DEATH_THROES);
        });

        _scheduler.Schedule(20s, GROUP_FINALE, [this](TaskContext /*task*/)
        {
            if (Creature* chromie = me->SummonCreature(NPC_CHROMIE, ChromieSpawnPos, TEMPSUMMON_MANUAL_DESPAWN))
                if (chromie->IsAIEnabled())
                    chromie->AI()->Talk(SAY_CHROMIE_ARRIVAL);
        });

        _scheduler.Schedule(34s, GROUP_FINALE, [this](TaskContext /*task*/)
        {
            if (Creature* malfurion = GetMalfurion())
                if (malfurion->IsAIEnabled())
                    malfurion->AI()->Talk(EMOTE_MALFURION_SOUL);
        });

        _scheduler.Schedule(36s, GROUP_FINALE, [this](TaskContext /*task*/)
        {
            if (Creature* chromie = me->FindNearestCreature(NPC_CHROMIE, 100.f))
                if (chromie->IsAIEnabled())
                    chromie->AI()->Talk(SAY_CHROMIE_LOOT);
        });

        _scheduler.Schedule(38s, GROUP_FINALE, [this](TaskContext /*task*/)
        {
            if (Creature* illidan = GetIllidan())
                if (illidan->IsAIEnabled())
                    illidan->AI()->Talk(SAY_ILLIDAN_ARTIFACT);
        });

        _scheduler.Schedule(52s, GROUP_FINALE, [this](TaskContext /*task*/)
        {
            if (Creature* nozdormu = me->SummonCreature(NPC_NOZDORMU_FINALE, NozdormuSpawnPos, TEMPSUMMON_MANUAL_DESPAWN))
                if (nozdormu->IsAIEnabled())
                    nozdormu->AI()->Talk(SAY_NOZDORMU_FINALE); // group 4 - 0-3 belong to DS Madness
        });

        // Stormrage epilogue with the sniffed conversational gaps
        _scheduler.Schedule(65s, GROUP_FINALE, [this](TaskContext /*task*/)
        {
            me->SetStandState(UNIT_STAND_STATE_STAND);
            _collapsed = false;
            Talk(SAY_TYRANDE_EPILOGUE_1);
        });
        _scheduler.Schedule(67s, GROUP_FINALE, [this](TaskContext /*task*/) { TalkMalfurion(SAY_MALFURION_EPILOGUE_1); });
        _scheduler.Schedule(72s, GROUP_FINALE, [this](TaskContext /*task*/) { Talk(SAY_TYRANDE_EPILOGUE_2); });
        _scheduler.Schedule(81s, GROUP_FINALE, [this](TaskContext /*task*/) { TalkIllidan(SAY_ILLIDAN_EPILOGUE_1); });
        _scheduler.Schedule(86s, GROUP_FINALE, [this](TaskContext /*task*/) { TalkMalfurion(SAY_MALFURION_EPILOGUE_2); });
        _scheduler.Schedule(93s, GROUP_FINALE, [this](TaskContext /*task*/) { TalkIllidan(SAY_ILLIDAN_EPILOGUE_2); });
        _scheduler.Schedule(123s, GROUP_FINALE, [this](TaskContext /*task*/) { TalkMalfurion(SAY_MALFURION_EPILOGUE_3); });
        _scheduler.Schedule(134s, GROUP_FINALE, [this](TaskContext /*task*/) { TalkIllidan(SAY_ILLIDAN_EPILOGUE_3); });
        _scheduler.Schedule(145s, GROUP_FINALE, [this](TaskContext /*task*/) { TalkMalfurion(SAY_MALFURION_EPILOGUE_4); });
        _scheduler.Schedule(151s, GROUP_FINALE, [this](TaskContext /*task*/) { Talk(SAY_TYRANDE_EPILOGUE_3); });
        _scheduler.Schedule(167s, GROUP_FINALE, [this](TaskContext /*task*/)
        {
            DoCastAOE(SPELL_HAND_OF_ELUNE_FAREWELL, true); // 30 min goodbye
        });
    }

    void TalkIllidan(uint8 group)
    {
        if (Creature* illidan = GetIllidan())
            if (illidan->IsAIEnabled())
                illidan->AI()->Talk(group);
    }

    void TalkMalfurion(uint8 group)
    {
        if (Creature* malfurion = GetMalfurion())
            if (malfurion->IsAIEnabled())
                malfurion->AI()->Talk(group);
    }

    void Restage()
    {
        _scheduler.CancelGroup(GROUP_LANE);
        _scheduler.CancelGroup(GROUP_WRATH);
        _scheduler.CancelGroup(GROUP_FINALE);
        me->InterruptNonMeleeSpells(false);
        me->CombatStop(true);
        me->RemoveAurasDueToSpell(SPELL_HAND_OF_ELUNE);
        me->RemoveAurasDueToSpell(SPELL_DEBILITATING_FLAY);
        me->SetStandState(UNIT_STAND_STATE_STAND);
        _collapsed = false;
        _flayed = false;
        me->GetMotionMaster()->Clear();
        me->NearTeleportTo(TyrandeStagingPos);
        me->SetHomePosition(TyrandeStagingPos);
        StartBlessing();
    }

    InstanceScript* _instance;
    TaskScheduler _scheduler;
    bool _blessingActive;
    bool _flayed;
    bool _collapsed;
};

struct npc_woe_malfurion : public PassiveAI
{
    npc_woe_malfurion(Creature* creature) : PassiveAI(creature)
    {
        _instance = creature->GetInstanceScript();
    }

    void DoAction(int32 action) override
    {
        if (action == ACTION_RESTAGE)
        {
            me->GetMotionMaster()->Clear();
            me->NearTeleportTo(MalfurionStagingPos);
            me->SetHomePosition(MalfurionStagingPos);
        }
    }

private:
    InstanceScript* _instance;
};

struct npc_varothens_magical_blade : public PassiveAI
{
    npc_varothens_magical_blade(Creature* creature) : PassiveAI(creature), _clicked(false)
    {
        _instance = creature->GetInstanceScript();
    }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        DoCastSelf(SPELL_MAGICAL_BLADE_GROUND_VISUAL, true);
        Talk(EMOTE_SWORD_FALLS);
        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK); // template row also carries it
    }

    void OnSpellClick(Unit* clicker, bool& result) override
    {
        if (!result || _clicked)
            return;

        Creature* mannoroth = GetActor(me, _instance, BOSS_MANNOROTH_AND_VAROTHEN, NPC_MANNOROTH);
        if (!mannoroth || !mannoroth->IsAlive() || !mannoroth->IsInCombat())
            return;

        _clicked = true;
        me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);

        // 104818 hurls the blade (missile -> 104817); the embed itself is
        // driven directly so a whiffed spell cannot strand the encounter.
        clicker->CastSpell(mannoroth, SPELL_MAGICAL_BLADE_PICKUP, true);

        if (mannoroth->IsAIEnabled())
            mannoroth->AI()->DoAction(ACTION_BLADE_EMBEDDED);

        me->RemoveAurasDueToSpell(SPELL_MAGICAL_BLADE_GROUND_VISUAL);
        me->DespawnOrUnsummon(1s);
    }

private:
    InstanceScript* _instance;
    bool _clicked;
};

struct npc_woe_embedded_blade : public PassiveAI
{
    npc_woe_embedded_blade(Creature* creature) : PassiveAI(creature) { }

    void IsSummonedBy(Unit* summoner) override
    {
        DoCastSelf(SPELL_EMBEDDED_BLADE_VISUAL, true);
        if (Creature* mannoroth = summoner->ToCreature())
            if (mannoroth->GetEntry() == NPC_MANNOROTH)
                me->EnterVehicle(mannoroth, 0); // seat 0 = the wound
    }
};

struct npc_dreadlord_debilitator : public ScriptedAI
{
    npc_dreadlord_debilitator(Creature* creature) : ScriptedAI(creature)
    {
        _instance = creature->GetInstanceScript();
    }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        me->SetReactState(REACT_PASSIVE);
        // Taunt-proof but killable - players must burn them down.
        me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
        me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, true);
        DoCastSelf(SPELL_DEBILITATOR_COSMETIC, true);

        if (Creature* tyrande = GetActor(me, _instance, DATA_TYRANDE, NPC_TYRANDE))
        {
            Position flayPos = tyrande->GetNearPosition(12.f, tyrande->GetAngle(me));
            me->GetMotionMaster()->MovePoint(POINT_FLAY, flayPos);
        }

        // Channel watchdog: re-establish the flay if pushback or line of
        // sight hiccups break it (it only ends when the debilitator dies).
        _scheduler.Schedule(8s, [this](TaskContext task)
        {
            if (me->IsAlive() && !me->HasUnitState(UNIT_STATE_CASTING))
                ChannelFlay();
            task.Repeat(3s);
        });
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type == POINT_MOTION_TYPE && pointId == POINT_FLAY)
            ChannelFlay();
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
        // no melee - it only flays
    }

private:
    void ChannelFlay()
    {
        Creature* tyrande = GetActor(me, _instance, DATA_TYRANDE, NPC_TYRANDE);
        if (!tyrande || !tyrande->IsAlive())
            return;

        me->SetFacingToObject(tyrande);
        me->CastSpell(tyrande, SPELL_DEBILITATING_FLAY); // infinite channel
        DoZoneInCombat();
    }

    InstanceScript* _instance;
    TaskScheduler _scheduler;
};

// 55739 Doomguard Devastator / 56001 Felhound / 56002 Felguard / 56036 Infernal
// One AI - behavior keyed off the entry.
struct npc_woe_portal_demon : public ScriptedAI
{
    npc_woe_portal_demon(Creature* creature) : ScriptedAI(creature)
    {
        _instance = creature->GetInstanceScript();
    }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        me->SetReactState(REACT_PASSIVE);
        DoCastSelf(SPELL_PORTAL_PULL_VISUAL, true); // fly-in visual

        switch (me->GetEntry())
        {
            case NPC_DOOMGUARD_DEVASTATOR:
            {
                // Leap over the well rim down into Tyrande's lane.
                me->SetCanFly(true);
                me->SetDisableGravity(true);
                Position landing = TyrandeStagingPos;
                landing.m_positionX += frand(-5.f, 5.f);
                landing.m_positionY += frand(-5.f, 5.f);
                me->GetMotionMaster()->MovePoint(POINT_DEMON_LANDING, landing, false);
                break;
            }
            case NPC_FELHOUND_WAVE:
            case NPC_FELGUARD_WAVE:
                DoCastSelf(SPELL_FEL_ENTRANCE, true);
                _scheduler.Schedule(1s, [this](TaskContext /*task*/) { Engage(); });
                break;
            case NPC_INFERNAL:
                DoCastSelf(SPELL_FEL_ENTRANCE, true);
                _scheduler.Schedule(2s, [this](TaskContext /*task*/) { Engage(); });
                break;
            default:
                break;
        }
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != POINT_MOTION_TYPE || pointId != POINT_DEMON_LANDING)
            return;

        me->SetCanFly(false);
        me->SetDisableGravity(false);
        me->RemoveAurasDueToSpell(SPELL_PORTAL_PULL_VISUAL);
        DoCastSelf(SPELL_FEL_ENTRANCE, true);
        Engage();
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }

private:
    void Engage()
    {
        me->RemoveAurasDueToSpell(SPELL_PORTAL_PULL_VISUAL);

        Unit* target = nullptr;
        switch (me->GetEntry())
        {
            case NPC_DOOMGUARD_DEVASTATOR:
            case NPC_FELHOUND_WAVE:
                target = GetActor(me, _instance, DATA_TYRANDE, NPC_TYRANDE);
                break;
            case NPC_FELGUARD_WAVE:
                target = GetActor(me, _instance, DATA_ILLIDAN_FINALE, NPC_ILLIDAN_FINALE);
                break;
            default:
                break;
        }

        if (target && target->IsAlive())
        {
            // Hard threat lock onto the ally lane target
            me->GetThreatManager().AddThreat(target, 50000000.f);
            AttackStart(target);
        }
        else
        {
            // Infernals (and orphaned adds) fight the players
            me->SetReactState(REACT_AGGRESSIVE);
            DoZoneInCombat();
            if (Player* player = SelectRandomPlayer(me, 100.f))
                AttackStart(player);
        }
    }

    InstanceScript* _instance;
    TaskScheduler _scheduler;
};

// 55502 - Fel Flames: Fel Firestorm ground fire (also reused by Peroth'arn's kit)
struct npc_woe_fel_flames : public NullCreatureAI
{
    npc_woe_fel_flames(Creature* creature) : NullCreatureAI(creature) { }

    void JustAppeared() override
    {
        DoCastSelf(SPELL_FEL_FLAMES_PERIODIC, true); // 1 s pulse of 103891 ground damage
        me->DespawnOrUnsummon(25s);
    }
};

// 103918 - Blessing of Elune: attacks against lesser demons proc 103919 Elune's Wrath
class spell_woe_blessing_of_elune : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_ELUNES_WRATH });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        Unit* target = eventInfo.GetProcTarget();
        return target && IsLesserDemon(target->GetEntry());
    }

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();
        GetTarget()->CastSpell(eventInfo.GetProcTarget(), SPELL_ELUNES_WRATH, true);
    }

    void Register() override
    {
        DoCheckProc.Register(&spell_woe_blessing_of_elune::CheckProc);
        OnEffectProc.Register(&spell_woe_blessing_of_elune::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

// 105524 - Magistrike Arc: the random victim arcs 105523 (1,000,000) back into
// Mannoroth and the Embedded Blade pulses 104822 in lockstep. Both force-casts
// are resolved here deterministically (their NEARBY_ENTRY targeting is left
// unconditioned on purpose - the defaults are prevented).
class spell_mannoroth_magistrike_arc : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGISTRIKE_ARC_DAMAGE, SPELL_MAGISTRIKE_ARC_BLADE });
    }

    void HandleVictimArc(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);
        Unit* caster = GetCaster();
        Unit* victim = GetHitUnit();
        if (caster && victim)
            victim->CastSpell(caster, SPELL_MAGISTRIKE_ARC_DAMAGE, true);
    }

    void HandleBladePulse(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);
    }

    void PulseBlade()
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (Vehicle* vehicle = caster->GetVehicleKit())
            if (Unit* blade = vehicle->GetPassenger(0))
                blade->CastSpell(caster, SPELL_MAGISTRIKE_ARC_BLADE, true);
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_mannoroth_magistrike_arc::HandleVictimArc, EFFECT_0, SPELL_EFFECT_FORCE_CAST);
        OnEffectHitTarget.Register(&spell_mannoroth_magistrike_arc::HandleBladePulse, EFFECT_1, SPELL_EFFECT_FORCE_CAST);
        AfterCast.Register(&spell_mannoroth_magistrike_arc::PulseBlade);
    }
};
}

void AddSC_boss_mannoroth_and_varothen()
{
    using namespace WellOfEternity;
    using namespace WellOfEternity::MannorothVarothen;
    RegisterWellOfEternityCreatureAI(boss_mannoroth);
    RegisterWellOfEternityCreatureAI(boss_captain_varothen);
    RegisterWellOfEternityCreatureAI(npc_woe_illidan_finale);
    RegisterWellOfEternityCreatureAI(npc_woe_tyrande);
    RegisterWellOfEternityCreatureAI(npc_woe_malfurion);
    RegisterWellOfEternityCreatureAI(npc_varothens_magical_blade);
    RegisterWellOfEternityCreatureAI(npc_woe_embedded_blade);
    RegisterWellOfEternityCreatureAI(npc_dreadlord_debilitator);
    RegisterWellOfEternityCreatureAI(npc_woe_portal_demon);
    RegisterWellOfEternityCreatureAI(npc_woe_fel_flames);
    RegisterSpellScript(spell_woe_blessing_of_elune);
    RegisterSpellScript(spell_mannoroth_magistrike_arc);
}
