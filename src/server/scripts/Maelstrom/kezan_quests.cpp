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
 * Kezan goblin starting zone: quests after "Rolling with my Homies".
 * Report for Tryouts, The Replacements, Necessary Roughness, Fourth and Goal,
 * Give Sassy the News, The New You, Life of the Party, Pirate Party Crashers,
 * The Uninvited Guest, A Bazillion Macaroons?!, the bank heist quartet,
 * 447 and Life Savings (yacht escape).
 */

#include "ScriptMgr.h"
#include "Chat.h"
#include "CombatAI.h"
#include "GameObject.h"
#include "GameObjectAI.h"
#include "GameTime.h"
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

enum KezanQuests
{
    QUEST_NECESSARY_ROUGHNESS       = 24502,
    QUEST_FOURTH_AND_GOAL           = 24503,
    QUEST_FOURTH_AND_GOAL_ALT       = 28414,
    QUEST_LIFE_OF_THE_PARTY_M       = 14113,
    QUEST_LIFE_OF_THE_PARTY_F       = 14153,
    QUEST_GREAT_BANK_HEIST          = 14122,
    QUEST_WALTZ_RIGHT_IN            = 14123,
    QUEST_447                       = 14125,
    QUEST_LIFE_SAVINGS              = 14126,

    QUEST_CLASS_FIRST               = 14007,
    QUEST_CLASS_LAST                = 14013
};

enum KezanCreatures
{
    NPC_BILGEWATER_BUCCANEER_PROP   = 48526,
    NPC_BILGEWATER_BUCCANEER_THROW  = 37179,
    NPC_BILGEWATER_BUCCANEER_KICK   = 37213,
    NPC_STEAMWHEEDLE_SHARK          = 37114,
    NPC_FOURTH_AND_GOAL_TARGET      = 37203,
    NPC_KAJARO_BREATH_TARGET        = 42196, // ELM trigger on Mount Kajaro's summit
    NPC_DEATHWING                   = 48572,
    NPC_KEZAN_CITIZEN_1             = 35063,
    NPC_KEZAN_CITIZEN_2             = 35075,
    NPC_PARTYGOER_1                 = 35175,
    NPC_PARTYGOER_2                 = 35185,
    NPC_PARTYGOER_3                 = 35186,
    NPC_PARTYGOER_4                 = 35201,
    NPC_FBOK_VAULT                  = 35486,
    NPC_MOOK_DISGUISE               = 48925,
    NPC_GASBOT                      = 37598,
    NPC_OVERLOADED_GENERATOR        = 37561,
    NPC_STOVE_LEAK                  = 37590,
    NPC_SMOLDERING_BED              = 37594,
    NPC_SPELL_PRACTICE_CREDIT       = 44175
};

enum KezanSpells
{
    // Necessary Roughness / Fourth and Goal
    SPELL_SUMMON_KICK_BUCCANEER     = 70075, // summons 37213 at the caster
    SPELL_FOOTBOMB_IMPACT           = 69993,
    SPELL_KICK_FOOTBOMB_IMPACT      = 70052,
    SPELL_SEE_SPAWNED_BUCCANEER     = 90161, // hidden type-12 see-invis for the parked prop (48526)
    SPELL_PERMANENT_FEIGN_DEATH     = 29266,
    SPELL_DEATHWING_FIRE_BREATH     = 66858,
    SPELL_DEATHWING_FLYOVER_COSMETIC = 69988,
    SPELL_PANICKED_CITIZEN_INVIS    = 90636, // type-14 invis carried by the panic-double citizen spawns
    SPELL_SEE_PANICKED_CITIZENS     = 83042, // Quest Invisibility Detection 6 (type 14; permanent, script-removed)

    // Life of the Party
    SPELL_HAPPY_PARTYGOER           = 66916,
    SPELL_SUMMON_DISCO_BALL         = 66930,

    // The Great Bank Heist
    SPELL_SUMMON_VAULT_VEHICLE      = 67488, // summons 35486, native ride chain via BP = 67476
    SPELL_VAULT_CRACKED             = 67492, // create-item 46858 aimed at the passenger
    SPELL_VAULT_POWER_CORRECT       = 67493, // +10 mana self-energize
    SPELL_VAULT_POWER_INCORRECT     = 67494, // -10 mana self-energize (scripted; core drops negative energize)
    SPELL_VAULT_TELEGRAPH_DRILL     = 67495, // per-prompt state auras on the vault (P2 sniff)
    SPELL_VAULT_TELEGRAPH_EXPLOSIVES = 67496,
    SPELL_VAULT_TELEGRAPH_LISTEN    = 67497,
    SPELL_VAULT_TELEGRAPH_LOCKPICK  = 67498,
    SPELL_VAULT_TELEGRAPH_G_RAY     = 67499,
    SPELL_VAULT_TIMER               = 67502, // 5s input window; the tools require it on their caster
    SPELL_TOOL_AMAZING_G_RAY        = 67526,
    SPELL_TOOL_BLASTCRACKERS        = 67508,
    SPELL_TOOL_EAR_O_SCOPE          = 67524,
    SPELL_TOOL_INFINIFOLD_LOCKPICK  = 67525,
    SPELL_TOOL_KAJAMITE_DRILL       = 67522,

    // Waltz Right In
    SPELL_MOOK_DISGUISE             = 67435,

    // 447 (P2 sniff: every step is a player-cast master chain - the goober/panel
    // force-casts a short master aura whose script despawns the previous prop
    // and summons a fresh one at TARGET_DEST_NEARBY_ENTRY, anchored on the goober)
    SPELL_GENERATOR_MASTER          = 70223, // via goober force-cast 70197
    SPELL_GENERATOR_DESPAWN         = 70220,
    SPELL_GENERATOR_SUMMON          = 70198, // 37561, carries visual 70226
    SPELL_STOVE_MASTER              = 70239, // via goober force-cast 70238
    SPELL_STOVE_DESPAWN             = 70240,
    SPELL_STOVE_SUMMON              = 70241, // 37590, carries visual 70236
    SPELL_BED_MASTER                = 70247, // lands with the cigar missile 70251 (goober force-cast 70245)
    SPELL_BED_DESPAWN               = 70248,
    SPELL_BED_SUMMON                = 70249, // 37594, carries visual 70250
    SPELL_SUMMON_GASBOT             = 70252, // summon 37598 at TARGET_DEST_NEARBY_ENTRY (the panel)
    SPELL_GASBOT_MASTER             = 70254, // 500ms dummy on the arsonist: despawn stale bot, summon fresh one
    SPELL_GASBOT_DESPAWN            = 70255, // script effect on area Gasbots
    SPELL_GASBOT_GAS_STREAM         = 70256, // permanent dummy on the bot and its four gas targets (spray visual)
    SPELL_GASBOT_EXPLOSION          = 70259, // self dummy, detonation visual
    SPELL_GASBOT_SCRIPT_TO_CHAR     = 70260, // script effect at TARGET_UNIT_MASTER: the HQ-ablaze credit

    // Life Savings
    SPELL_YACHT_MORTAR_LAUNCH       = 92633  // jump to spell_target_position (the yacht deck)
};

enum KezanItems
{
    ITEM_PERSONAL_RICHES            = 46858
};

enum KezanTexts
{
    // npc_bilgewater_buccaneer (both entries)
    SAY_BUCCANEER_INSTRUCTIONS      = 0,

    // npc_kezan_deathwing
    SAY_DEATHWING_FLYOVER           = 0,

    // npc_first_bank_vault
    SAY_VAULT_INTRO_1               = 0,
    SAY_VAULT_INTRO_2               = 1,
    SAY_VAULT_INTRO_3               = 2,
    SAY_VAULT_INTRO_4               = 3,
    SAY_VAULT_PROMPT_G_RAY          = 4,
    SAY_VAULT_PROMPT_BLASTCRACKERS  = 5,
    SAY_VAULT_PROMPT_EAR_O_SCOPE    = 6,
    SAY_VAULT_PROMPT_LOCKPICK       = 7,
    SAY_VAULT_PROMPT_DRILL          = 8,
    SAY_VAULT_CORRECT               = 9,
    SAY_VAULT_INCORRECT             = 10,
    SAY_VAULT_SUCCESS               = 11
};

enum KezanActions
{
    ACTION_DEATHWING_FLYOVER        = 1,
    ACTION_PARTYGOER_SERVED         = 2,
    ACTION_DEATHWING_LEG_1          = 4,
    ACTION_DEATHWING_SYNC_LEG_1     = 5,
    ACTION_DEATHWING_YELL           = 6,
    ACTION_DEATHWING_LEG_2          = 7,
    ACTION_DEATHWING_SYNC_LEG_2     = 8,
    ACTION_DEATHWING_CIRCUIT        = 9,
    ACTION_DEATHWING_COSMETIC       = 10,
    ACTION_DEATHWING_WHISPER        = 11,
    ACTION_DEATHWING_SYNC_EXIT      = 12,
    ACTION_DEATHWING_EXIT           = 13,
    ACTION_DEATHWING_PANIC_START    = 14,
    ACTION_DEATHWING_SCREAM         = 15,
    ACTION_DEATHWING_CAMERA_SHAKE   = 16,
    ACTION_DEATHWING_PANIC_END      = 17,
    ACTION_DEATHWING_EJECT_VIEWER   = 18,

    DATA_PARTYGOER_WANT             = 1,
    DATA_DEATHWING_VIEWER           = 2,
    DATA_ACTIVE_DEATHWING           = 3
};

enum KezanMisc
{
    MOVIE_ESCAPE_FROM_KEZAN         = 22,
    ZONE_KEZAN                      = 4737,

    // Deathwing flyover roars: object sounds sourced at the viewer, one per
    // movement beat (P2 sniff: 23227 leg 1, 23228 leg 2 ~100ms before the
    // yell, 23229 circuit, 23230 with 69988 at circuit end).
    SOUND_DEATHWING_FLYBY_1         = 23227,
    SOUND_DEATHWING_FLYBY_2         = 23228,
    SOUND_DEATHWING_FLYBY_3         = 23229,
    SOUND_DEATHWING_FLYBY_4         = 23230,

    // Crowd bed (P2 sniff): cheer at the kick, cheer at the summon, then
    // Event_EbonHold_CrowdScream1-4 every ~3s while Deathwing is overhead.
    SOUND_CROWD_CHEER_KICK          = 17467,
    SOUND_CROWD_CHEER_SUMMON        = 8574,

    // The Great Bank Heist vault feedback dings (P2 sniff, object sound
    // sourced at the vault and targeted at the passenger).
    SOUND_VAULT_CORRECT             = 11595,
    SOUND_VAULT_INCORRECT_1         = 847,
    SOUND_VAULT_INCORRECT_2         = 16381,

    // 90615 'Fourth and Goal: Character Earthquake' uses SpellVisual 20130,
    // whose impact kit (19579) contains sound 1485 and
    // SpellEffectCameraShakes row 6. Row 6 expands to CameraShakes 4/5/6: a
    // four-second positional/rotational quake. The 4.3.4 client does not play
    // an impact kit sent as a standalone visual kit, so send its two payloads
    // through their native packets instead.
    SOUND_CHARACTER_EARTHQUAKE      = 1485,
    CAMERA_SHAKE_CHARACTER_EARTHQUAKE = 6
};

// Retail scream order for the nine crowd-scream beats (P2 sniff, t+3s..t+27s)
uint32 const DeathwingPanicScreams[] = { 14558, 14557, 14556, 14558, 14556, 14558, 14559, 14559, 14557 };

float constexpr FOOTBOMB_BUCCANEER_ORIENTATION = 3.12414f;
float constexpr DEATHWING_FLYOVER_SPEED = 49.0f;

Position const KezanLostIslesShore = { 534.835f, 3272.92f, 0.171872f, 5.14795f };

// -----------------------------------------------------------------------------
// Necessary Roughness (24502) / Fourth and Goal (24503 / 28414)
// -----------------------------------------------------------------------------

Position const SharkPositions[8] =
{
    { -8288.62f, 1479.97f, 43.97f, 0.0f },
    { -8273.75f, 1484.46f, 43.02f, 0.0f },
    { -8288.08f, 1487.72f, 43.93f, 0.0f },
    { -8281.04f, 1477.49f, 43.39f, 0.0f },
    { -8281.33f, 1490.41f, 43.56f, 0.0f },
    { -8295.10f, 1484.91f, 44.41f, 0.0f },
    { -8294.66f, 1474.68f, 44.38f, 0.0f },
    { -8294.61f, 1493.67f, 44.71f, 0.0f }
};

// The sharks walk from their spawn cluster toward the goal line (near the player's
// shredder). Destinations lifted from the P2 sniff (one lane per spawn point above).
Position const SharkGoalPositions[8] =
{
    { -8260.88f, 1483.13f, 42.11f, 0.0f },
    { -8260.94f, 1484.35f, 42.13f, 0.0f },
    { -8260.90f, 1485.19f, 42.11f, 0.0f },
    { -8260.70f, 1482.08f, 42.10f, 0.0f },
    { -8260.75f, 1486.25f, 42.13f, 0.0f },
    { -8260.94f, 1484.41f, 42.11f, 0.0f },
    { -8260.71f, 1482.11f, 42.10f, 0.0f },
    { -8260.72f, 1486.35f, 42.12f, 0.0f }
};

enum BuccaneerEvents
{
    EVENT_BOARD_OWNER               = 1,
    EVENT_SUMMON_SHARKS             = 2
};

class npc_bilgewater_buccaneer : public CreatureScript
{
public:
    npc_bilgewater_buccaneer() : CreatureScript("npc_bilgewater_buccaneer") { }

    struct npc_bilgewater_buccaneerAI : public VehicleAI
    {
        npc_bilgewater_buccaneerAI(Creature* creature) : VehicleAI(creature) { }

        void IsSummonedBy(Unit* summoner) override
        {
            if (!summoner)
                return;

            _ownerGUID = summoner->GetGUID();
            AlignToParkedProp();

            // The throw-boat (37179) boards its summoner natively through the
            // summon spell's ride-back (70016, which also grants credit 48271).
            // The kick-boat's summon (70075) has no ride-back, board by hand.
            if (me->GetEntry() == NPC_BILGEWATER_BUCCANEER_KICK)
                _events.ScheduleEvent(EVENT_BOARD_OWNER, 1s);
        }

        void PassengerBoarded(Unit* passenger, int8 seatId, bool apply) override
        {
            if (seatId != 0 || !passenger->IsPlayer())
                return;

            if (apply)
            {
                // The ride boat overlaps the parked prop (48526) exactly and, as the
                // driver's charmer, is always visible to them. Drop the see-invisibility
                // so the prop stops rendering for the driver -> only one boat shows.
                passenger->RemoveAurasDueToSpell(SPELL_SEE_SPAWNED_BUCCANEER);
                Talk(SAY_BUCCANEER_INSTRUCTIONS, passenger);
                if (me->GetEntry() == NPC_BILGEWATER_BUCCANEER_THROW)
                    _events.ScheduleEvent(EVENT_SUMMON_SHARKS, 5s);
            }
            else
            {
                // While either footbomb quest is still in the log (in progress OR
                // complete-but-not-turned-in), restore the see-invisibility so the
                // parked prop reappears and the player can board again.
                if (HasActiveFootbombQuest(passenger->ToPlayer()))
                    passenger->CastSpell(passenger, SPELL_SEE_SPAWNED_BUCCANEER, true);

                _events.Reset();
                DespawnSharks();
                if (me->IsSummon())
                    me->DespawnOrUnsummon(2000);
            }
        }

        void JustSummoned(Creature* summon) override
        {
            if (summon->GetEntry() == NPC_STEAMWHEEDLE_SHARK)
            {
                _sharkGUIDs.push_back(summon->GetGUID());
                summon->SetReactState(REACT_PASSIVE);
            }
        }

        void UpdateAI(uint32 diff) override
        {
            VehicleAI::UpdateAI(diff);
            _events.Update(diff);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_BOARD_OWNER:
                        if (Player* owner = ObjectAccessor::GetPlayer(*me, _ownerGUID))
                            if (!owner->GetVehicle() && me->GetVehicleKit())
                            {
                                AlignToParkedProp();
                                owner->SetFacingTo(me->GetOrientation());
                                owner->EnterVehicle(me, 0);
                            }
                        break;
                    case EVENT_SUMMON_SHARKS:
                        // Spawn the 8 sharks and set each walking toward the goal line
                        // (toward the player's shredder) so they can be footbombed en route.
                        for (uint8 i = 0; i < 8; ++i)
                            if (Creature* shark = me->SummonCreature(NPC_STEAMWHEEDLE_SHARK, SharkPositions[i], TEMPSUMMON_TIMED_DESPAWN, 5min))
                            {
                                shark->SetWalk(true);
                                shark->GetMotionMaster()->MovePoint(0, SharkGoalPositions[i]);
                            }
                        break;
                    default:
                        break;
                }
            }
        }

    private:
        void AlignToParkedProp()
        {
            float orientation = FOOTBOMB_BUCCANEER_ORIENTATION;
            if (Creature* prop = me->FindNearestCreature(NPC_BILGEWATER_BUCCANEER_PROP, 15.0f, true))
                orientation = prop->GetOrientation();

            me->SetFacingTo(orientation);
        }

        static bool HasActiveFootbombQuest(Player* player)
        {
            if (!player)
                return false;

            for (uint32 questId : { QUEST_NECESSARY_ROUGHNESS, QUEST_FOURTH_AND_GOAL, QUEST_FOURTH_AND_GOAL_ALT })
            {
                QuestStatus status = player->GetQuestStatus(questId);
                if (status == QUEST_STATUS_INCOMPLETE || status == QUEST_STATUS_COMPLETE)
                    return true;
            }
            return false;
        }

        void DespawnSharks()
        {
            for (ObjectGuid const& guid : _sharkGUIDs)
                if (Creature* shark = ObjectAccessor::GetCreature(*me, guid))
                    shark->DespawnOrUnsummon();
            _sharkGUIDs.clear();
        }

        EventMap _events;
        ObjectGuid _ownerGUID;
        std::vector<ObjectGuid> _sharkGUIDs;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_bilgewater_buccaneerAI(creature);
    }
};

// 69993 - Throw Footbomb (impact): feign the shark, credit the driver.
class spell_kezan_footbomb_impact : public SpellScriptLoader
{
public:
    spell_kezan_footbomb_impact() : SpellScriptLoader("spell_kezan_footbomb_impact") { }

    class spell_kezan_footbomb_impact_SpellScript : public SpellScript
    {
    public:
        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            Creature* shark = GetHitCreature();
            if (!shark || shark->GetEntry() != NPC_STEAMWHEEDLE_SHARK)
                return;

            if (shark->HasAura(SPELL_PERMANENT_FEIGN_DEATH))
                return;

            shark->CastSpell(shark, SPELL_PERMANENT_FEIGN_DEATH, true);
            shark->DespawnOrUnsummon(15000);

            Unit* caster = GetCaster();
            if (!caster)
                return;

            Player* driver = caster->GetCharmer() ? caster->GetCharmer()->ToPlayer() : nullptr;
            if (!driver)
                if (Vehicle* kit = caster->GetVehicleKit())
                    if (Unit* seat0 = kit->GetPassenger(0))
                        driver = seat0->ToPlayer();

            if (driver)
                driver->KilledMonsterCredit(NPC_STEAMWHEEDLE_SHARK);
        }

        void Register() override
        {
            OnEffectHitTarget.Register(&spell_kezan_footbomb_impact_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_kezan_footbomb_impact_SpellScript();
    }
};

// 70052 - Kick Footbomb (impact): goal detection between the smokestacks.
class spell_kezan_footbomb_kick_impact : public SpellScriptLoader
{
public:
    spell_kezan_footbomb_kick_impact() : SpellScriptLoader("spell_kezan_footbomb_kick_impact") { }

    class spell_kezan_footbomb_kick_impact_SpellScript : public SpellScript
    {
    public:
        void HandleAfterCast()
        {
            Unit* caster = GetCaster();
            if (!caster)
                return;

            WorldLocation const* dest = GetExplTargetDest();
            if (!dest)
                return;

            Player* driver = caster->GetCharmer() ? caster->GetCharmer()->ToPlayer() : nullptr;
            if (!driver)
                if (Vehicle* kit = caster->GetVehicleKit())
                    if (Unit* seat0 = kit->GetPassenger(0))
                        driver = seat0->ToPlayer();

            if (!driver)
                return;

            if (driver->GetQuestStatus(QUEST_FOURTH_AND_GOAL) != QUEST_STATUS_INCOMPLETE &&
                driver->GetQuestStatus(QUEST_FOURTH_AND_GOAL_ALT) != QUEST_STATUS_INCOMPLETE)
                return;

            Creature* target = caster->FindNearestCreature(NPC_FOURTH_AND_GOAL_TARGET, 200.0f);
            if (!target || target->GetExactDist2d(dest->GetPositionX(), dest->GetPositionY()) > 15.0f)
                return;

            driver->KilledMonsterCredit(NPC_FOURTH_AND_GOAL_TARGET);
            driver->PlayDistanceSound(SOUND_CROWD_CHEER_KICK, driver);
            if (CreatureAI* ai = target->AI())
            {
                ai->SetGUID(driver->GetGUID(), DATA_DEATHWING_VIEWER);
                ai->DoAction(ACTION_DEATHWING_FLYOVER);
            }
        }

        void Register() override
        {
            AfterCast.Register(&spell_kezan_footbomb_kick_impact_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_kezan_footbomb_kick_impact_SpellScript();
    }
};

// 37203 - Fourth And Goal Target: summons the Deathwing flyover after the goal.
class npc_fourth_and_goal_target : public CreatureScript
{
public:
    npc_fourth_and_goal_target() : CreatureScript("npc_fourth_and_goal_target") { }

    struct npc_fourth_and_goal_targetAI : public ScriptedAI
    {
        npc_fourth_and_goal_targetAI(Creature* creature) : ScriptedAI(creature), _nextFlyoverTime(0) { }

        void SetGUID(ObjectGuid const& guid, int32 id) override
        {
            if (id == DATA_DEATHWING_VIEWER)
                _viewerGUID = guid;
            else if (id == DATA_ACTIVE_DEATHWING)
                _deathwingGUID = guid;
        }

        void DoAction(int32 action) override
        {
            if (action != ACTION_DEATHWING_FLYOVER)
                return;

            time_t now = GameTime::GetGameTime();
            if (_nextFlyoverTime > now)
                return;

            // A flyover that stopped updating would also stop its timed despawn.
            // Remove that orphan before allowing a later retrigger.
            if (Creature* deathwing = ObjectAccessor::GetCreature(*me, _deathwingGUID))
                deathwing->DespawnOrUnsummon();
            _deathwingGUID.Clear();

            _nextFlyoverTime = now + 60;
            if (Creature* deathwing = me->SummonCreature(NPC_DEATHWING, { -8178.59f, 1482.14f, 84.0f, 3.106686f }, TEMPSUMMON_TIMED_DESPAWN, 45s))
            {
                _deathwingGUID = deathwing->GetGUID();
                if (CreatureAI* ai = deathwing->AI())
                    ai->SetGUID(_viewerGUID, DATA_DEATHWING_VIEWER);

                if (Player* viewer = ObjectAccessor::GetPlayer(*me, _viewerGUID))
                {
                    viewer->PlayDistanceSound(SOUND_CROWD_CHEER_SUMMON, viewer);
                    ScheduleDeathwingAction(viewer, _deathwingGUID, ACTION_DEATHWING_PANIC_START, 300ms);
                    ScheduleDeathwingAction(viewer, _deathwingGUID, ACTION_DEATHWING_LEG_1, 1500ms);
                    ScheduleDeathwingAction(viewer, _deathwingGUID, ACTION_DEATHWING_SYNC_LEG_1, 4600ms);
                    ScheduleDeathwingAction(viewer, _deathwingGUID, ACTION_DEATHWING_LEG_2, 4700ms);
                    ScheduleDeathwingAction(viewer, _deathwingGUID, ACTION_DEATHWING_YELL, 4800ms);
                    ScheduleDeathwingAction(viewer, _deathwingGUID, ACTION_DEATHWING_SYNC_LEG_2, 10350ms);
                    ScheduleDeathwingAction(viewer, _deathwingGUID, ACTION_DEATHWING_CIRCUIT, 10400ms);
                    ScheduleDeathwingAction(viewer, _deathwingGUID, ACTION_DEATHWING_EJECT_VIEWER, 16800ms);
                    ScheduleDeathwingAction(viewer, _deathwingGUID, ACTION_DEATHWING_COSMETIC, 26500ms);
                    ScheduleDeathwingAction(viewer, _deathwingGUID, ACTION_DEATHWING_WHISPER, 26800ms);
                    ScheduleDeathwingAction(viewer, _deathwingGUID, ACTION_DEATHWING_CAMERA_SHAKE, 28500ms);
                    ScheduleDeathwingAction(viewer, _deathwingGUID, ACTION_DEATHWING_SYNC_EXIT, 28900ms);
                    ScheduleDeathwingAction(viewer, _deathwingGUID, ACTION_DEATHWING_PANIC_END, 28900ms);
                    ScheduleDeathwingAction(viewer, _deathwingGUID, ACTION_DEATHWING_EXIT, 29s);
                    for (uint32 scream = 0; scream < std::size(DeathwingPanicScreams); ++scream)
                        ScheduleDeathwingAction(viewer, _deathwingGUID, ACTION_DEATHWING_SCREAM, Milliseconds(3000 + scream * 3000));
                    ScheduleDeathwingCleanup(viewer, me->GetGUID(), _deathwingGUID, 42s);
                }
            }
        }

    private:
        static void ScheduleDeathwingAction(Player* viewer, ObjectGuid deathwingGUID, int32 action, Milliseconds delay)
        {
            ObjectGuid viewerGUID = viewer->GetGUID();
            viewer->m_Events.AddEventAtOffset([viewerGUID, deathwingGUID, action]()
            {
                if (Player* player = ObjectAccessor::FindPlayer(viewerGUID))
                    if (Creature* deathwing = ObjectAccessor::GetCreature(*player, deathwingGUID))
                        if (CreatureAI* ai = deathwing->AI())
                            ai->DoAction(action);
            }, delay);
        }

        static void ScheduleDeathwingCleanup(Player* viewer, ObjectGuid targetGUID, ObjectGuid deathwingGUID, Milliseconds delay)
        {
            ObjectGuid viewerGUID = viewer->GetGUID();
            viewer->m_Events.AddEventAtOffset([viewerGUID, targetGUID, deathwingGUID]()
            {
                Player* player = ObjectAccessor::FindPlayer(viewerGUID);
                if (!player)
                    return;

                // Failsafe for the PANIC_END beat (relog edge cases).
                player->RemoveAurasDueToSpell(SPELL_SEE_PANICKED_CITIZENS);

                if (Creature* deathwing = ObjectAccessor::GetCreature(*player, deathwingGUID))
                    deathwing->DespawnOrUnsummon();

                if (Creature* target = ObjectAccessor::GetCreature(*player, targetGUID))
                    if (CreatureAI* ai = target->AI())
                        ai->SetGUID(ObjectGuid::Empty, DATA_ACTIVE_DEATHWING);
            }, delay);
        }

        time_t _nextFlyoverTime;
        ObjectGuid _viewerGUID;
        ObjectGuid _deathwingGUID;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_fourth_and_goal_targetAI(creature);
    }
};

// 48572 - Deathwing: cinematic flyover across the stadium (path from sniff).
Position const DeathwingLeg1[1] =
{
    { -8307.88f, 1483.6702f, 137.1013f }
};

Position const DeathwingLeg2[2] =
{
    { -8357.739f, 1482.9305f, 150.9125f },
    { -8544.15f, 1481.06f, 276.202f }
};

Position const DeathwingCircuit[6] =
{
    { -8562.12f, 1483.93f, 283.8f },
    { -8644.45f, 1499.07f, 312.9f },
    { -8707.98f, 1559.36f, 312.9f },
    { -8689.69f, 1657.08f, 312.9f },
    { -8565.21f, 1675.97f, 285.0f },
    { -8311.69f, 1501.69f, 93.48f }
};

Position const DeathwingExit[4] =
{
    { -8262.02f, 1468.5f, 93.79f },
    { -8140.4f, 1405.29f, 89.01f },
    { -7904.96f, 1380.62f, 89.01f },
    { -7736.61f, 1402.56f, 89.01f }
};

class npc_kezan_deathwing : public CreatureScript
{
public:
    npc_kezan_deathwing() : CreatureScript("npc_kezan_deathwing") { }

    struct npc_kezan_deathwingAI : public ScriptedAI
    {
        npc_kezan_deathwingAI(Creature* creature) : ScriptedAI(creature) { }

        void SetGUID(ObjectGuid const& guid, int32 id) override
        {
            if (id == DATA_DEATHWING_VIEWER)
                _viewerGUID = guid;
        }

        void IsSummonedBy(Unit* /*summoner*/) override
        {
            // The volcano circuit leaves the viewer's grid update bubble (Map::Update
            // only visits cells within Visibility.Distance.Continents of a player),
            // which froze the spline, the leg schedule and even the 45s despawn timer
            // mid-flight until someone flew out to him. Active objects get their
            // cells visited every tick. Client visibility across the ~470yd circuit
            // comes from creature_template_addon.visibilityDistanceType = Infinite.
            me->setActive(true);
            me->setDeathState(ALIVE);
            me->SetFullHealth();
            me->SetStandState(UNIT_STAND_STATE_STAND);
            me->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);
            me->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
            me->SetAnimationTier(AnimationTier::Fly);
            me->SetCanFly(true);
            me->SetDisableGravity(true);
            me->SetSpeedRate(MOVE_FLIGHT, 7.0f);
            me->SetReactState(REACT_PASSIVE);
        }

        void DoAction(int32 action) override
        {
            switch (action)
            {
                case ACTION_DEATHWING_LEG_1:
                    PlayFlybySound(SOUND_DEATHWING_FLYBY_1);
                    MovePath(DeathwingLeg1, std::size(DeathwingLeg1));
                    break;
                case ACTION_DEATHWING_SYNC_LEG_1:
                    SynchronizePosition(DeathwingLeg1[std::size(DeathwingLeg1) - 1], 3.1268f);
                    break;
                case ACTION_DEATHWING_YELL:
                    Talk(SAY_DEATHWING_FLYOVER);
                    break;
                case ACTION_DEATHWING_LEG_2:
                    PlayFlybySound(SOUND_DEATHWING_FLYBY_2);
                    MovePath(DeathwingLeg2, std::size(DeathwingLeg2));
                    break;
                case ACTION_DEATHWING_SYNC_LEG_2:
                    SynchronizePosition(DeathwingLeg2[std::size(DeathwingLeg2) - 1], 2.9832f);
                    break;
                case ACTION_DEATHWING_CIRCUIT:
                    PlayFlybySound(SOUND_DEATHWING_FLYBY_3);
                    if (sSpellMgr->GetSpellInfo(SPELL_DEATHWING_FIRE_BREATH))
                        me->CastSpell(GetFireBreathTarget(), SPELL_DEATHWING_FIRE_BREATH, true);
                    MovePath(DeathwingCircuit, std::size(DeathwingCircuit));
                    break;
                case ACTION_DEATHWING_COSMETIC:
                    PlayFlybySound(SOUND_DEATHWING_FLYBY_4);
                    if (sSpellMgr->GetSpellInfo(SPELL_DEATHWING_FLYOVER_COSMETIC))
                        me->CastSpell(me, SPELL_DEATHWING_FLYOVER_COSMETIC, true);
                    break;
                case ACTION_DEATHWING_PANIC_START:
                    StartStadiumPanic();
                    break;
                case ACTION_DEATHWING_SCREAM:
                    if (_screamIndex < std::size(DeathwingPanicScreams))
                        PlayFlybySound(DeathwingPanicScreams[_screamIndex++]);
                    break;
                case ACTION_DEATHWING_WHISPER:
                    SendMountKajaroWhisper();
                    break;
                case ACTION_DEATHWING_EJECT_VIEWER:
                    // Retail dismounts the kicker mid-flyover (P2 sniff, t+16.8s);
                    // they watch the finale - including the camera shake - on foot.
                    if (Player* viewer = ObjectAccessor::GetPlayer(*me, _viewerGUID))
                        if (viewer->GetVehicle())
                            viewer->ExitVehicle();
                    break;
                case ACTION_DEATHWING_CAMERA_SHAKE:
                    if (Player* viewer = ObjectAccessor::GetPlayer(*me, _viewerGUID))
                    {
                        // SMSG_CAMERA_SHAKE takes a SpellEffectCameraShakes.dbc
                        // row followed by an unused uint32 on the 4.3.4 client.
                        WorldPacket cameraShake(SMSG_CAMERA_SHAKE, 2 * sizeof(uint32));
                        cameraShake << uint32(CAMERA_SHAKE_CHARACTER_EARTHQUAKE);
                        cameraShake << uint32(0);
                        viewer->SendDirectMessage(&cameraShake);
                        viewer->PlayDirectSound(SOUND_CHARACTER_EARTHQUAKE, viewer);
                    }
                    break;
                case ACTION_DEATHWING_PANIC_END:
                    if (Player* viewer = ObjectAccessor::GetPlayer(*me, _viewerGUID))
                        viewer->RemoveAurasDueToSpell(SPELL_SEE_PANICKED_CITIZENS);
                    break;
                case ACTION_DEATHWING_SYNC_EXIT:
                    SynchronizePosition(DeathwingCircuit[std::size(DeathwingCircuit) - 1], 5.6810f);
                    break;
                case ACTION_DEATHWING_EXIT:
                    MovePath(DeathwingExit, std::size(DeathwingExit));
                    break;
                default:
                    break;
            }
        }

    private:
        void MovePath(Position const* path, size_t pathSize)
        {
            // Spline packets broadcast within the mover's own visibility range
            // (SendMessageToSet -> GetVisibilityRange), which the Infinite
            // visibilityDistanceType extends to the whole circuit - no manual
            // packet forwarding needed.
            me->GetMotionMaster()->MoveSmoothPath(0, path, pathSize, false, true, DEATHWING_FLYOVER_SPEED);
        }

        void PlayFlybySound(uint32 soundId)
        {
            // Retail plays the flyby roars as object sounds sourced and
            // targeted at the viewer (P2 sniff PLAY_OBJECT_SOUND per beat).
            if (Player* viewer = ObjectAccessor::GetPlayer(*me, _viewerGUID))
                viewer->PlayDistanceSound(soundId, viewer);
        }

        Unit* GetFireBreathTarget()
        {
            // Retail aims the speed-70 missile at the Mount Kajaro summit
            // trigger so the flame streaks from Deathwing to the volcano.
            if (Creature* target = me->FindNearestCreature(NPC_KAJARO_BREATH_TARGET, 400.0f))
                return target;
            return me;
        }

        uint8 _screamIndex = 0;

        void SynchronizePosition(Position const& position, float orientation)
        {
            Position synchronizedPosition = position;
            synchronizedPosition.SetOrientation(orientation);
            me->NearTeleportTo(synchronizedPosition);
        }

        void SendMountKajaroWhisper()
        {
            Player* player = ObjectAccessor::GetPlayer(*me, _viewerGUID);
            if (!player)
                return;

            WorldPacket data;
            ChatHandler::BuildChatPacket(data, CHAT_MSG_RAID_BOSS_WHISPER, LANG_UNIVERSAL, player, player, "What did that dragon do to Mount Kajaro?!!!");
            player->SendDirectMessage(&data);
        }

        void StartStadiumPanic()
        {
            // Retail runs a population swap (P2 sniff): 32 dedicated runner
            // citizens spawn right after the summon, visible only to the
            // kicking player, and scatter-run for ~28.6s while the bleacher
            // crowd keeps cheering. Our DB carries them as permanent spawns
            // under type-14 invis (90636); grant the viewer the matching
            // detection and set every double running. Movement and restore
            // are self-scheduled per citizen so they need nothing from this
            // AI afterwards.
            // The detection must be aura-applied: CastSpell on a player who is
            // driving the kick boat gets rejected by the cast system before the
            // aura lands (verified in the 2026-07-10 walkthrough).
            if (Player* viewer = ObjectAccessor::GetPlayer(*me, _viewerGUID))
                viewer->AddAura(SPELL_SEE_PANICKED_CITIZENS, viewer);

            for (uint32 entry : { NPC_KEZAN_CITIZEN_1, NPC_KEZAN_CITIZEN_2 })
            {
                std::list<Creature*> citizens;
                me->GetCreatureListWithEntryInGrid(citizens, entry, 250.0f);

                for (Creature* citizen : citizens)
                    if (citizen->IsAlive() && citizen->HasAura(SPELL_PANICKED_CITIZEN_INVIS))
                        SchedulePanicScatter(citizen);
            }
        }

        static void SchedulePanicScatter(Creature* citizen)
        {
            // Retail cadence: a fresh 2-22yd single-point run spline at
            // 8.0 yd/s every ~1.6s, milling around the spawn point rather
            // than fleeing (median net displacement ~12yd over ~195yd run).
            for (uint32 hop = 0; hop < 18; ++hop)
                citizen->m_Events.AddEventAtOffset([citizen]()
                {
                    if (!citizen->IsAlive())
                        return;

                    Position dest = citizen->GetRandomPoint(citizen->GetHomePosition(), 12.0f);
                    citizen->GetMotionMaster()->MovePoint(0, dest, false, 8.0f);
                }, Milliseconds(hop * 1600));

            citizen->m_Events.AddEventAtOffset([citizen]()
            {
                if (citizen->IsAlive())
                    citizen->GetMotionMaster()->MoveTargetedHome();
            }, Milliseconds(18 * 1600));
        }

        ObjectGuid _viewerGUID;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_kezan_deathwingAI(creature);
    }
};

// -----------------------------------------------------------------------------
// Life of the Party (14113 / 14153)
// -----------------------------------------------------------------------------

enum PartygoerWants
{
    WANT_BUBBLY                     = 0,
    WANT_BUCKET                     = 1,
    WANT_DANCE                      = 2,
    WANT_FIREWORKS                  = 3,
    WANT_HORS_DOEUVRES              = 4,
    WANT_MAX
};

// action spell cast by the player (from the Awesome Party Ensemble bar)
uint32 const PartygoerActionSpells[WANT_MAX] = { 66909, 66910, 66911, 66912, 66913 };
// self-cast "wanted" bubble visual
uint32 const PartygoerWantVisuals[WANT_MAX] = { 75042, 75044, 75046, 75048, 75050 };
// creature_text groups (want bark / served response) - layout from TDB 35186 texts
uint8 const PartygoerWantTexts[WANT_MAX] = { 2, 1, 0, 3, 4 };
uint8 const PartygoerServedTexts[WANT_MAX] = { 5, 6, 7, 8, 9 };

enum PartygoerEvents
{
    EVENT_PARTYGOER_WANT_BARK       = 1,
    EVENT_PARTYGOER_RESET           = 2
};

class npc_kezan_partygoer : public CreatureScript
{
public:
    npc_kezan_partygoer() : CreatureScript("npc_kezan_partygoer") { }

    struct npc_kezan_partygoerAI : public ScriptedAI
    {
        npc_kezan_partygoerAI(Creature* creature) : ScriptedAI(creature), _want(WANT_BUBBLY), _served(false) { }

        void Reset() override
        {
            _served = false;
            RollNewWant();
            _events.Reset();
            _events.ScheduleEvent(EVENT_PARTYGOER_WANT_BARK, 5s, 60s);
        }

        uint32 GetData(uint32 type) const override
        {
            if (type == DATA_PARTYGOER_WANT)
                return _served ? WANT_MAX : _want;
            return 0;
        }

        void DoAction(int32 action) override
        {
            if (action != ACTION_PARTYGOER_SERVED || _served)
                return;

            _served = true;
            me->RemoveAurasDueToSpell(PartygoerWantVisuals[_want]);

            if (sSpellMgr->GetSpellInfo(SPELL_HAPPY_PARTYGOER))
                me->CastSpell(me, SPELL_HAPPY_PARTYGOER, true);

            if (_want == WANT_DANCE && sSpellMgr->GetSpellInfo(SPELL_SUMMON_DISCO_BALL))
                me->CastSpell(me, SPELL_SUMMON_DISCO_BALL, true);

            Talk(PartygoerServedTexts[_want]);

            _events.CancelEvent(EVENT_PARTYGOER_WANT_BARK);
            _events.ScheduleEvent(EVENT_PARTYGOER_RESET, 30s);
        }

        void UpdateAI(uint32 diff) override
        {
            _events.Update(diff);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_PARTYGOER_WANT_BARK:
                        if (!_served)
                            Talk(PartygoerWantTexts[_want]);
                        _events.ScheduleEvent(EVENT_PARTYGOER_WANT_BARK, 30s, 90s);
                        break;
                    case EVENT_PARTYGOER_RESET:
                        _served = false;
                        me->RemoveAurasDueToSpell(SPELL_HAPPY_PARTYGOER);
                        RollNewWant();
                        _events.ScheduleEvent(EVENT_PARTYGOER_WANT_BARK, 5s, 30s);
                        break;
                    default:
                        break;
                }
            }
        }

    private:
        void RollNewWant()
        {
            me->RemoveAurasDueToSpell(PartygoerWantVisuals[_want]);
            _want = urand(0, WANT_MAX - 1);
            if (sSpellMgr->GetSpellInfo(PartygoerWantVisuals[_want]))
                me->CastSpell(me, PartygoerWantVisuals[_want], true);
        }

        EventMap _events;
        uint32 _want;
        bool _served;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_kezan_partygoerAI(creature);
    }
};

// 66909-66913 - party actions: only the wanted action serves a partygoer.
class spell_kezan_party_action : public SpellScriptLoader
{
public:
    spell_kezan_party_action() : SpellScriptLoader("spell_kezan_party_action") { }

    class spell_kezan_party_action_SpellScript : public SpellScript
    {
    public:
        static bool IsPartygoer(Unit const* unit)
        {
            switch (unit->GetEntry())
            {
                case NPC_PARTYGOER_1:
                case NPC_PARTYGOER_2:
                case NPC_PARTYGOER_3:
                case NPC_PARTYGOER_4:
                    return true;
                default:
                    return false;
            }
        }

        uint32 GetWantIndex() const
        {
            for (uint8 i = 0; i < WANT_MAX; ++i)
                if (PartygoerActionSpells[i] == GetSpellInfo()->Id)
                    return i;
            return WANT_MAX;
        }

        SpellCastResult CheckTarget()
        {
            Unit* target = GetExplTargetUnit();
            if (!target || !IsPartygoer(target) || !target->IsAlive())
                return SPELL_FAILED_BAD_TARGETS;

            Creature* partygoer = target->ToCreature();
            if (!partygoer || !partygoer->AI())
                return SPELL_FAILED_BAD_TARGETS;

            if (partygoer->AI()->GetData(DATA_PARTYGOER_WANT) != GetWantIndex())
                return SPELL_FAILED_BAD_TARGETS;

            return SPELL_CAST_OK;
        }

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            Player* player = GetCaster()->ToPlayer();
            Creature* partygoer = GetHitCreature();
            if (!player || !partygoer || !IsPartygoer(partygoer))
                return;

            if (!partygoer->AI() || partygoer->AI()->GetData(DATA_PARTYGOER_WANT) != GetWantIndex())
                return;

            partygoer->AI()->DoAction(ACTION_PARTYGOER_SERVED);
            player->KilledMonsterCredit(NPC_PARTYGOER_1);
        }

        void Register() override
        {
            OnCheckCast.Register(&spell_kezan_party_action_SpellScript::CheckTarget);
            OnEffectHitTarget.Register(&spell_kezan_party_action_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_kezan_party_action_SpellScript();
    }
};

// -----------------------------------------------------------------------------
// The Great Bank Heist (14122)
// -----------------------------------------------------------------------------

enum VaultEvents
{
    EVENT_VAULT_INTRO_1             = 1,
    EVENT_VAULT_INTRO_2             = 2,
    EVENT_VAULT_INTRO_3             = 3,
    EVENT_VAULT_INTRO_4             = 4,
    EVENT_VAULT_NEXT_PROMPT         = 5,
    EVENT_VAULT_TIMEOUT             = 6,
    EVENT_VAULT_DECAY               = 7,
    EVENT_VAULT_EJECT               = 8
};

enum VaultTools
{
    TOOL_G_RAY                      = 0,
    TOOL_BLASTCRACKERS              = 1,
    TOOL_EAR_O_SCOPE                = 2,
    TOOL_INFINIFOLD_LOCKPICK        = 3,
    TOOL_KAJAMITE_DRILL             = 4,
    TOOL_MAX
};

uint32 const VaultToolSpells[TOOL_MAX] = { SPELL_TOOL_AMAZING_G_RAY, SPELL_TOOL_BLASTCRACKERS, SPELL_TOOL_EAR_O_SCOPE, SPELL_TOOL_INFINIFOLD_LOCKPICK, SPELL_TOOL_KAJAMITE_DRILL };
uint32 const VaultTelegraphSpells[TOOL_MAX] = { SPELL_VAULT_TELEGRAPH_G_RAY, SPELL_VAULT_TELEGRAPH_EXPLOSIVES, SPELL_VAULT_TELEGRAPH_LISTEN, SPELL_VAULT_TELEGRAPH_LOCKPICK, SPELL_VAULT_TELEGRAPH_DRILL };
uint8 const VaultToolTexts[TOOL_MAX] = { SAY_VAULT_PROMPT_G_RAY, SAY_VAULT_PROMPT_BLASTCRACKERS, SAY_VAULT_PROMPT_EAR_O_SCOPE, SAY_VAULT_PROMPT_LOCKPICK, SAY_VAULT_PROMPT_DRILL };

// Retail flow (P2 sniff): the seated player pet-casts a tool, the cast is
// executed by the player at the vault (TARGET_UNIT_VEHICLE, gated on the 5s
// 67502 window aura), and the vault answers with 67493 (+10 mana) or 67494
// (-10 mana). The mana bar is the vault-breaking progress bar; it decays 5
// power every 5s tick. The vault is cracked by a correct answer landing while
// the bar is already full (the only full-overflow energize in the sniff).
class npc_first_bank_vault : public CreatureScript
{
public:
    npc_first_bank_vault() : CreatureScript("npc_first_bank_vault") { }

    struct npc_first_bank_vaultAI : public VehicleAI
    {
        npc_first_bank_vaultAI(Creature* creature) : VehicleAI(creature), _currentTool(TOOL_MAX), _awaitingInput(false)
        {
            me->SetPowerType(POWER_MANA);
            me->SetMaxPower(POWER_MANA, 100);
            me->SetPower(POWER_MANA, 0);
        }

        void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
        {
            Player* player = passenger->ToPlayer();
            if (!player)
                return;

            if (apply)
            {
                _playerGUID = player->GetGUID();
                _currentTool = TOOL_MAX;
                _awaitingInput = false;

                me->SetPower(POWER_MANA, 0);

                // Retail grants the quest credit the moment the ride aura lands.
                player->KilledMonsterCredit(NPC_FBOK_VAULT);

                // Retail beats: intro whispers at +1.5s/+8s/+16s/+24s, first
                // prompt at ~27s, decay tick every ~5s from boarding.
                _events.Reset();
                _events.ScheduleEvent(EVENT_VAULT_INTRO_1, 1500ms);
                _events.ScheduleEvent(EVENT_VAULT_INTRO_2, 8s);
                _events.ScheduleEvent(EVENT_VAULT_INTRO_3, 16s);
                _events.ScheduleEvent(EVENT_VAULT_INTRO_4, 24s);
                _events.ScheduleEvent(EVENT_VAULT_NEXT_PROMPT, 27s);
                _events.ScheduleEvent(EVENT_VAULT_DECAY, 5s);
            }
            else
            {
                _events.Reset();
                _playerGUID.Clear();
                _awaitingInput = false;
                if (me->IsSummon())
                    me->DespawnOrUnsummon(2000);
            }
        }

        void DoAction(int32 action) override
        {
            // action = tool index used by the passenger (from spell_kezan_vault_tool)
            if (action < 0 || action >= TOOL_MAX || !_awaitingInput)
                return;

            _awaitingInput = false;
            _events.CancelEvent(EVENT_VAULT_TIMEOUT);

            Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID);
            if (!player)
                return;

            if (_currentTool < TOOL_MAX)
                me->RemoveAurasDueToSpell(VaultTelegraphSpells[_currentTool]);

            if (uint32(action) == _currentTool)
            {
                bool const wasFull = me->GetPower(POWER_MANA) >= me->GetMaxPower(POWER_MANA);
                me->PlayDistanceSound(SOUND_VAULT_CORRECT, player);

                if (wasFull)
                {
                    // Native create-item 46858 aimed at the passenger, with a
                    // fallback in case the cast system drops the seated player
                    // as a hit target.
                    me->CastSpell(me, SPELL_VAULT_CRACKED, true);
                    if (!player->HasItemCount(ITEM_PERSONAL_RICHES))
                        player->AddItem(ITEM_PERSONAL_RICHES, 1);

                    me->CastSpell(me, SPELL_VAULT_POWER_CORRECT, true);
                    Talk(SAY_VAULT_SUCCESS, player);
                    _events.ScheduleEvent(EVENT_VAULT_EJECT, 200ms);
                }
                else
                {
                    me->CastSpell(me, SPELL_VAULT_POWER_CORRECT, true);
                    Talk(SAY_VAULT_CORRECT, player);
                    _events.ScheduleEvent(EVENT_VAULT_NEXT_PROMPT, 3s);
                }
            }
            else
            {
                me->PlayDistanceSound(SOUND_VAULT_INCORRECT_1, player);
                me->PlayDistanceSound(SOUND_VAULT_INCORRECT_2, player);
                me->CastSpell(me, SPELL_VAULT_POWER_INCORRECT, true);
                Talk(SAY_VAULT_INCORRECT, player);
                _events.ScheduleEvent(EVENT_VAULT_NEXT_PROMPT, 3s);
            }
        }

        void UpdateAI(uint32 diff) override
        {
            VehicleAI::UpdateAI(diff);
            _events.Update(diff);

            Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_VAULT_DECAY:
                        // Retail: the progress bar bleeds 5 power every ~5s.
                        if (int32 power = me->GetPower(POWER_MANA))
                            me->SetPower(POWER_MANA, std::max<int32>(power - 5, 0));
                        _events.ScheduleEvent(EVENT_VAULT_DECAY, 5s);
                        continue;
                    default:
                        break;
                }

                if (!player)
                    continue;

                switch (eventId)
                {
                    case EVENT_VAULT_INTRO_1:
                        Talk(SAY_VAULT_INTRO_1, player);
                        break;
                    case EVENT_VAULT_INTRO_2:
                        Talk(SAY_VAULT_INTRO_2, player);
                        break;
                    case EVENT_VAULT_INTRO_3:
                        Talk(SAY_VAULT_INTRO_3, player);
                        break;
                    case EVENT_VAULT_INTRO_4:
                        Talk(SAY_VAULT_INTRO_4, player);
                        break;
                    case EVENT_VAULT_NEXT_PROMPT:
                    {
                        _currentTool = urand(0, TOOL_MAX - 1);
                        me->CastSpell(me, VaultTelegraphSpells[_currentTool], true);

                        // 67502 hits the vault (client greys the bar buttons
                        // without it) and the seat-0 passenger (the tools
                        // require it on their caster). The cast system drops
                        // hits on vehicle-driving players, so back-fill the
                        // passenger half with AddAura.
                        me->CastSpell(me, SPELL_VAULT_TIMER, true);
                        if (!me->HasAura(SPELL_VAULT_TIMER))
                            me->AddAura(SPELL_VAULT_TIMER, me);
                        if (!player->HasAura(SPELL_VAULT_TIMER))
                            me->AddAura(SPELL_VAULT_TIMER, player);

                        _awaitingInput = true;
                        Talk(VaultToolTexts[_currentTool], player);
                        _events.ScheduleEvent(EVENT_VAULT_TIMEOUT, 5s);
                        break;
                    }
                    case EVENT_VAULT_TIMEOUT:
                        // No retail penalty for hesitating - the decay tick is
                        // the cost. Just move on to the next prompt.
                        _awaitingInput = false;
                        if (_currentTool < TOOL_MAX)
                            me->RemoveAurasDueToSpell(VaultTelegraphSpells[_currentTool]);
                        _events.ScheduleEvent(EVENT_VAULT_NEXT_PROMPT, 1s);
                        break;
                    case EVENT_VAULT_EJECT:
                        player->ExitVehicle();
                        break;
                    default:
                        break;
                }
            }
        }

    private:
        EventMap _events;
        ObjectGuid _playerGUID;
        uint32 _currentTool;
        bool _awaitingInput;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_first_bank_vaultAI(creature);
    }
};

// 67555 - The Great Bank Heist: Vault Interact (cast by the vault door GO).
class spell_kezan_vault_interact : public SpellScriptLoader
{
public:
    spell_kezan_vault_interact() : SpellScriptLoader("spell_kezan_vault_interact") { }

    class spell_kezan_vault_interact_SpellScript : public SpellScript
    {
    public:
        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            Player* player = GetHitPlayer();
            if (!player)
                return;

            if (player->GetQuestStatus(QUEST_GREAT_BANK_HEIST) != QUEST_STATUS_INCOMPLETE)
                return;

            if (player->GetVehicle())
                return;

            // Summons a personal vault console (35486) and boards it (67476).
            player->CastSpell(player, SPELL_SUMMON_VAULT_VEHICLE, true);
        }

        void Register() override
        {
            OnEffectHitTarget.Register(&spell_kezan_vault_interact_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_kezan_vault_interact_SpellScript();
    }
};

// 67526/67508/67524/67525/67522 - vault cracking tools, cast by the seated
// player at the vault (TARGET_UNIT_VEHICLE).
class spell_kezan_vault_tool : public SpellScriptLoader
{
public:
    spell_kezan_vault_tool() : SpellScriptLoader("spell_kezan_vault_tool") { }

    class spell_kezan_vault_tool_SpellScript : public SpellScript
    {
    public:
        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            Creature* vault = GetHitCreature();
            if (!vault || vault->GetEntry() != NPC_FBOK_VAULT || !vault->AI())
                return;

            for (uint8 i = 0; i < TOOL_MAX; ++i)
            {
                if (VaultToolSpells[i] == GetSpellInfo()->Id)
                {
                    vault->AI()->DoAction(i);
                    break;
                }
            }
        }

        void HandleAfterCast()
        {
            // Retail strips the input-window aura from the player the moment
            // a tool is used (cast-interrupt on 67502).
            if (Unit* caster = GetCaster())
                caster->RemoveAurasDueToSpell(SPELL_VAULT_TIMER);
        }

        void Register() override
        {
            OnEffectHitTarget.Register(&spell_kezan_vault_tool_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            AfterCast.Register(&spell_kezan_vault_tool_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_kezan_vault_tool_SpellScript();
    }
};

// 67494 - The Great Bank Heist: Power Incorrect. The core drops negative
// energizes for mana, apply the -10 by hand.
class spell_kezan_vault_power_incorrect : public SpellScriptLoader
{
public:
    spell_kezan_vault_power_incorrect() : SpellScriptLoader("spell_kezan_vault_power_incorrect") { }

    class spell_kezan_vault_power_incorrect_SpellScript : public SpellScript
    {
    public:
        void HandleEnergize(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);
            if (Unit* target = GetHitUnit())
                target->ModifyPower(POWER_MANA, GetEffectValue());
        }

        void Register() override
        {
            OnEffectHitTarget.Register(&spell_kezan_vault_power_incorrect_SpellScript::HandleEnergize, EFFECT_0, SPELL_EFFECT_ENERGIZE);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_kezan_vault_power_incorrect_SpellScript();
    }
};

// -----------------------------------------------------------------------------
// Waltz Right In (14123)
// -----------------------------------------------------------------------------

// 67435 - Mook Disguise: forces the Villa Mook faction friendly and makes the
// player a vehicle (1362); we mount the disguise prop on top.
class spell_kezan_mook_disguise : public SpellScriptLoader
{
public:
    spell_kezan_mook_disguise() : SpellScriptLoader("spell_kezan_mook_disguise") { }

    class spell_kezan_mook_disguise_AuraScript : public AuraScript
    {
    public:
        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            Player* player = GetTarget()->ToPlayer();
            if (!player)
                return;

            ObjectGuid playerGUID = player->GetGUID();
            player->m_Events.AddEventAtOffset([playerGUID]()
            {
                Player* owner = ObjectAccessor::FindPlayer(playerGUID);
                if (!owner || !owner->HasAura(SPELL_MOOK_DISGUISE) || !owner->GetVehicleKit())
                    return;

                if (owner->GetVehicleKit()->GetPassenger(0))
                    return;

                if (Creature* disguise = owner->SummonCreature(NPC_MOOK_DISGUISE, owner->GetPosition(), TEMPSUMMON_MANUAL_DESPAWN))
                {
                    disguise->SetReactState(REACT_PASSIVE);
                    disguise->EnterVehicle(owner, 0);
                }
            }, 1s);
        }

        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            Player* player = GetTarget()->ToPlayer();
            if (!player)
                return;

            if (Vehicle* kit = player->GetVehicleKit())
                if (Unit* passenger = kit->GetPassenger(0))
                    if (passenger->GetEntry() == NPC_MOOK_DISGUISE)
                    {
                        passenger->ExitVehicle();
                        if (Creature* disguise = passenger->ToCreature())
                            disguise->DespawnOrUnsummon();
                    }
        }

        void Register() override
        {
            AfterEffectApply.Register(&spell_kezan_mook_disguise_AuraScript::OnApply, EFFECT_0, SPELL_AURA_FORCE_REACTION, AURA_EFFECT_HANDLE_REAL);
            AfterEffectRemove.Register(&spell_kezan_mook_disguise_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_FORCE_REACTION, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_kezan_mook_disguise_AuraScript();
    }
};

// -----------------------------------------------------------------------------
// Liberate the Kaja'mite (14124)
// -----------------------------------------------------------------------------

enum KablooeyData
{
    GO_KAJAMITE_DEPOSIT             = 195488, // bombable goober, era phase 383
    GO_KAJAMITE_CHUNK               = 195492, // consumable chest with the quest item

    CHUNKS_PER_DEPOSIT              = 3,
    CHUNK_DESPAWN_SECS              = 120,
};

// 67682 - Kablooey!: EFFECT_0 (ACTIVATE_OBJECT, action Open) hits the deposit
// goobers within 5yd of the blast. Retail (Goblin_P2 sniff): the deposit plays
// custom anim 0, despawns, and three Kaja'mite Chunk chests scatter 2-4yd
// around its base with random facing.
class spell_kezan_kablooey_bombs : public SpellScriptLoader
{
public:
    spell_kezan_kablooey_bombs() : SpellScriptLoader("spell_kezan_kablooey_bombs") { }

    class spell_kezan_kablooey_bombs_SpellScript : public SpellScript
    {
    public:
        void HandleActivateObject(SpellEffIndex effIndex)
        {
            // The goober's own Use() path (kill credit, IN_USE state machine)
            // does nothing useful here; the explosion is scripted in full.
            PreventHitDefaultEffect(effIndex);

            GameObject* deposit = GetHitGObj();
            if (!deposit || deposit->GetEntry() != GO_KAJAMITE_DEPOSIT || !deposit->isSpawned())
                return;

            Unit* caster = GetCaster();
            if (!caster)
                return;

            deposit->SendCustomAnim(0);
            deposit->DespawnOrUnsummon(); // respawns after gameobject.spawntimesecs

            for (uint8 i = 0; i < CHUNKS_PER_DEPOSIT; ++i)
            {
                float angle = frand(0.0f, 2.0f * float(M_PI));
                float dist = frand(1.5f, 3.5f);
                float x = deposit->GetPositionX() + dist * std::cos(angle);
                float y = deposit->GetPositionY() + dist * std::sin(angle);
                float z = deposit->GetPositionZ();
                deposit->UpdateGroundPositionZ(x, y, z);

                float facing = frand(0.0f, 2.0f * float(M_PI));
                caster->SummonGameObject(GO_KAJAMITE_CHUNK, Position(x, y, z, facing),
                    QuaternionData::fromEulerAnglesZYX(facing, 0.0f, 0.0f), CHUNK_DESPAWN_SECS);
            }
        }

        void Register() override
        {
            OnEffectHitTarget.Register(&spell_kezan_kablooey_bombs_SpellScript::HandleActivateObject, EFFECT_0, SPELL_EFFECT_ACTIVATE_OBJECT);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_kezan_kablooey_bombs_SpellScript();
    }
};

// -----------------------------------------------------------------------------
// 447 (14125)
// -----------------------------------------------------------------------------

enum GasbotData
{
    EVENT_GASBOT_GAS_STREAM         = 1,
    EVENT_GASBOT_DETONATE           = 2,
    EVENT_GASBOT_DESPAWN            = 3,

    POINT_GASBOT_KTC                = 1
};

// P2 sniff: the panel chain (70253 -> 70254 -> 70255 + 70252, all player-cast)
// summons the bot in front of the panel; it walks (2.5 yd/s) straight up the
// KTC Headquarters front steps with the gas streams on, detonates at the top
// (t+16.8s) and is destroyed 1.6s later.
Position const GasbotSpawnPos = { -8424.346f, 1328.0365f, 102.042694f, 1.570796f };

Position const GasbotPath[] =
{
    { -8424.281f, 1328.6875f, 102.22952f  },
    { -8424.281f, 1331.6875f, 102.47952f  },
    { -8424.281f, 1334.6875f, 102.47952f  },
    { -8424.217f, 1341.3385f, 102.41634f  },
    { -8424.228f, 1344.4723f, 103.27698f  },
    { -8424.147f, 1346.4358f, 104.670074f },
    { -8424.052f, 1350.2894f, 104.81177f  },
    { -8424.134f, 1354.3507f, 104.69782f  },
    { -8424.043f, 1358.6611f, 104.947815f },
    { -8423.944f, 1364.3264f, 104.69782f  }
};

class npc_gasbot : public CreatureScript
{
public:
    npc_gasbot() : CreatureScript("npc_gasbot") { }

    struct npc_gasbotAI : public ScriptedAI
    {
        npc_gasbotAI(Creature* creature) : ScriptedAI(creature), _detonated(false) { }

        void IsSummonedBy(Unit* /*summoner*/) override
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetWalk(true);
            _events.ScheduleEvent(EVENT_GASBOT_GAS_STREAM, 1500ms);
        }

        void MovementInform(uint32 type, uint32 pointId) override
        {
            if (type != EFFECT_MOTION_TYPE || pointId != POINT_GASBOT_KTC)
                return;

            _events.CancelEvent(EVENT_GASBOT_DETONATE);
            Detonate();
        }

        void UpdateAI(uint32 diff) override
        {
            _events.Update(diff);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_GASBOT_GAS_STREAM:
                        // TARGET_UNIT_CASTER_AND_PASSENGERS: the bot plus its four gas targets.
                        DoCastSelf(SPELL_GASBOT_GAS_STREAM);
                        me->GetMotionMaster()->MoveSmoothPath(POINT_GASBOT_KTC, GasbotPath, std::size(GasbotPath), true);
                        _events.ScheduleEvent(EVENT_GASBOT_DETONATE, 17s); // failsafe in case the spline is rejected
                        break;
                    case EVENT_GASBOT_DETONATE:
                        Detonate();
                        break;
                    case EVENT_GASBOT_DESPAWN:
                        me->DespawnOrUnsummon();
                        break;
                    default:
                        break;
                }
            }
        }

    private:
        void Detonate()
        {
            if (_detonated)
                return;
            _detonated = true;

            me->CastSpell(me, SPELL_GASBOT_EXPLOSION, false);
            // TARGET_UNIT_MASTER resolves to the summoner; the SpellScript grants the credit.
            me->CastSpell(me->GetCharmerOrOwner(), SPELL_GASBOT_SCRIPT_TO_CHAR, true);
            _events.ScheduleEvent(EVENT_GASBOT_DESPAWN, 1600ms);
        }

        EventMap _events;
        bool _detonated;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_gasbotAI(creature);
    }
};

// 70223, 70239, 70247 - the three arson-house master auras (forced onto the
// player by the house goobers): despawn the previous prop, summon a fresh one.
class spell_kezan_arson_master : public SpellScriptLoader
{
public:
    spell_kezan_arson_master() : SpellScriptLoader("spell_kezan_arson_master") { }

    class spell_kezan_arson_master_SpellScript : public SpellScript
    {
    public:
        void HandleApply(SpellEffIndex /*effIndex*/)
        {
            Unit* target = GetHitUnit();
            if (!target)
                return;

            uint32 despawnSpell = 0;
            uint32 summonSpell = 0;
            switch (GetSpellInfo()->Id)
            {
                case SPELL_GENERATOR_MASTER:
                    despawnSpell = SPELL_GENERATOR_DESPAWN;
                    summonSpell = SPELL_GENERATOR_SUMMON;
                    break;
                case SPELL_STOVE_MASTER:
                    despawnSpell = SPELL_STOVE_DESPAWN;
                    summonSpell = SPELL_STOVE_SUMMON;
                    break;
                case SPELL_BED_MASTER:
                    despawnSpell = SPELL_BED_DESPAWN;
                    summonSpell = SPELL_BED_SUMMON;
                    break;
                default:
                    return;
            }

            target->CastSpell(target, despawnSpell, true);
            target->CastSpell(target, summonSpell, true);

            // The goober credit (KillCreditGO) has already been granted at this
            // point; if this was the last objective, light up the HQ (phase 385).
            if (Player* player = target->ToPlayer())
                if (player->GetQuestStatus(QUEST_447) == QUEST_STATUS_COMPLETE)
                    PhasingHandler::OnConditionChange(player);
        }

        void Register() override
        {
            OnEffectHitTarget.Register(&spell_kezan_arson_master_SpellScript::HandleApply, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_kezan_arson_master_SpellScript();
    }
};

// 70220, 70240, 70248, 70255 - despawn the previous arson prop / Gasbot (area entry target)
class spell_kezan_arson_despawn : public SpellScriptLoader
{
public:
    spell_kezan_arson_despawn() : SpellScriptLoader("spell_kezan_arson_despawn") { }

    class spell_kezan_arson_despawn_SpellScript : public SpellScript
    {
    public:
        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            if (Creature* prop = GetHitCreature())
                prop->DespawnOrUnsummon();
        }

        void Register() override
        {
            OnEffectHitTarget.Register(&spell_kezan_arson_despawn_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_kezan_arson_despawn_SpellScript();
    }
};

// 70253 - 447: AICast Gasbot Master (panel goober, playerCast)
class spell_kezan_gasbot_panel : public SpellScriptLoader
{
public:
    spell_kezan_gasbot_panel() : SpellScriptLoader("spell_kezan_gasbot_panel") { }

    class spell_kezan_gasbot_panel_SpellScript : public SpellScript
    {
    public:
        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            Player* player = GetHitPlayer();
            if (!player)
                return;

            // The event is only worth running while the fourth credit is still owed.
            if (player->GetQuestStatus(QUEST_447) != QUEST_STATUS_INCOMPLETE)
                return;
            if (player->GetReqKillOrCastCurrentCount(QUEST_447, NPC_GASBOT) > 0)
                return;

            player->CastSpell(player, SPELL_GASBOT_MASTER, true);
        }

        void Register() override
        {
            OnEffectHitTarget.Register(&spell_kezan_gasbot_panel_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_kezan_gasbot_panel_SpellScript();
    }
};

// 70254 - 447: Gasbot Master
class spell_kezan_gasbot_master : public SpellScriptLoader
{
public:
    spell_kezan_gasbot_master() : SpellScriptLoader("spell_kezan_gasbot_master") { }

    class spell_kezan_gasbot_master_SpellScript : public SpellScript
    {
    public:
        void HandleApply(SpellEffIndex /*effIndex*/)
        {
            Unit* target = GetHitUnit();
            if (!target)
                return;

            // Retail order: clear any lingering bot, then summon a fresh one.
            target->CastSpell(target, SPELL_GASBOT_DESPAWN, true);
            target->CastSpell(target, SPELL_SUMMON_GASBOT, true);
        }

        void Register() override
        {
            OnEffectHitTarget.Register(&spell_kezan_gasbot_master_SpellScript::HandleApply, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_kezan_gasbot_master_SpellScript();
    }
};

// 70252 - 447: Summon Gasbot
class spell_kezan_summon_gasbot : public SpellScriptLoader
{
public:
    spell_kezan_summon_gasbot() : SpellScriptLoader("spell_kezan_summon_gasbot") { }

    class spell_kezan_summon_gasbot_SpellScript : public SpellScript
    {
    public:
        void SetDest(SpellDestination& dest)
        {
            // The nearby-entry anchor (the panel) is ~3 yd off the sniffed spawn point.
            dest.Relocate(GasbotSpawnPos);
        }

        void Register() override
        {
            OnDestinationTargetSelect.Register(&spell_kezan_summon_gasbot_SpellScript::SetDest, EFFECT_0, TARGET_DEST_NEARBY_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_kezan_summon_gasbot_SpellScript();
    }
};

// 70260 - 447: Gasbot Scriptcast to Character
class spell_kezan_gasbot_credit : public SpellScriptLoader
{
public:
    spell_kezan_gasbot_credit() : SpellScriptLoader("spell_kezan_gasbot_credit") { }

    class spell_kezan_gasbot_credit_SpellScript : public SpellScript
    {
    public:
        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            Player* player = GetHitPlayer();
            if (!player)
                return;

            player->KilledMonsterCredit(NPC_GASBOT);
            PhasingHandler::OnConditionChange(player); // 447 Fires phase in (385) the moment the quest completes
        }

        void Register() override
        {
            OnEffectHitTarget.Register(&spell_kezan_gasbot_credit_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_kezan_gasbot_credit_SpellScript();
    }
};

// -----------------------------------------------------------------------------
// Life Savings (14126)
// -----------------------------------------------------------------------------

// 92629 - Last Chance Yacht Boarding Mortar - Cover: launch onto the yacht deck.
class spell_kezan_yacht_mortar : public SpellScriptLoader
{
public:
    spell_kezan_yacht_mortar() : SpellScriptLoader("spell_kezan_yacht_mortar") { }

    class spell_kezan_yacht_mortar_SpellScript : public SpellScript
    {
    public:
        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            Unit* target = GetHitUnit();
            if (!target)
                return;

            // The player may still be sitting in the Hot Rod; the launch moves
            // the passenger, not the car.
            Player* player = target->ToPlayer();
            if (!player)
                if (Vehicle* kit = target->GetVehicleKit())
                    if (Unit* seat0 = kit->GetPassenger(0))
                        player = seat0->ToPlayer();

            if (!player)
                return;

            if (player->GetQuestStatus(QUEST_LIFE_SAVINGS) == QUEST_STATUS_NONE)
                return;

            player->ExitVehicle();
            player->CastSpell(player, SPELL_YACHT_MORTAR_LAUNCH, true);
        }

        void Register() override
        {
            OnEffectHitTarget.Register(&spell_kezan_yacht_mortar_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_kezan_yacht_mortar_SpellScript();
    }
};

// -----------------------------------------------------------------------------
// Shared player script: class quest credits, boat lifecycle, zone exit.
// -----------------------------------------------------------------------------

class player_script_kezan_events : public PlayerScript
{
public:
    player_script_kezan_events() : PlayerScript("player_script_kezan_events") { }

    void OnLogin(Player* player, bool /*firstLogin*/) override
    {
        // Catch-up: logged out during the escape movie.
        if (player->GetZoneId() == ZONE_KEZAN && player->GetQuestStatus(QUEST_LIFE_SAVINGS) == QUEST_STATUS_REWARDED)
            player->TeleportTo(648, KezanLostIslesShore.GetPositionX(), KezanLostIslesShore.GetPositionY(), KezanLostIslesShore.GetPositionZ(), KezanLostIslesShore.GetOrientation());
    }

    void OnSpellCast(Player* player, Spell* spell, bool /*skipCheck*/) override
    {
        // Kezan class quests: practice the new ability (quest_template.RequiredSpell).
        if (player->GetZoneId() != ZONE_KEZAN)
            return;

        uint32 spellId = spell->GetSpellInfo()->Id;
        for (uint32 questId = QUEST_CLASS_FIRST; questId <= QUEST_CLASS_LAST; ++questId)
        {
            if (player->GetQuestStatus(questId) != QUEST_STATUS_INCOMPLETE)
                continue;

            Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
            if (!quest || quest->GetRequiredSpell() != spellId)
                continue;

            player->KilledMonsterCredit(NPC_SPELL_PRACTICE_CREDIT);
            break;
        }
    }

    void OnQuestStatusChange(Player* player, uint32 questId) override
    {
        QuestStatus status = player->GetQuestStatus(questId);

        switch (questId)
        {
            case QUEST_FOURTH_AND_GOAL:
            case QUEST_FOURTH_AND_GOAL_ALT:
                // Boarding is driven by the visible parked prop's 70075 spellclick.
                // Accepting Fourth and Goal must not create a second kick boat.
                if (status == QUEST_STATUS_NONE || status == QUEST_STATUS_FAILED || status == QUEST_STATUS_REWARDED)
                {
                    // Last-resort cleanup of the panic detection if the viewer
                    // relogged past both the PANIC_END beat and the 42s cleanup.
                    player->RemoveAurasDueToSpell(SPELL_SEE_PANICKED_CITIZENS);
                    CleanupOwnedCreatures(player, { NPC_BILGEWATER_BUCCANEER_KICK });
                }
                break;
            case QUEST_NECESSARY_ROUGHNESS:
                // Do not drop the see-invisibility here: Fourth and Goal auto-accepts on
                // reward and still needs the prop visible. spell_area removes 90161 when
                // the player leaves the field with no footbomb quest left.
                if (status == QUEST_STATUS_NONE || status == QUEST_STATUS_FAILED || status == QUEST_STATUS_REWARDED)
                    CleanupOwnedCreatures(player, { NPC_BILGEWATER_BUCCANEER_THROW });
                break;
            case QUEST_GREAT_BANK_HEIST:
                if (status == QUEST_STATUS_NONE || status == QUEST_STATUS_FAILED)
                    CleanupOwnedCreatures(player, { NPC_FBOK_VAULT });
                break;
            case QUEST_WALTZ_RIGHT_IN:
                if (status == QUEST_STATUS_NONE || status == QUEST_STATUS_FAILED || status == QUEST_STATUS_REWARDED)
                    player->RemoveAurasDueToSpell(SPELL_MOOK_DISGUISE);
                break;
            case QUEST_447:
                if (status == QUEST_STATUS_NONE || status == QUEST_STATUS_FAILED || status == QUEST_STATUS_REWARDED)
                    CleanupOwnedCreatures(player, { NPC_GASBOT, NPC_OVERLOADED_GENERATOR, NPC_STOVE_LEAK, NPC_SMOLDERING_BED });
                break;
            case QUEST_LIFE_SAVINGS:
                if (status == QUEST_STATUS_REWARDED)
                {
                    // RewardSpell 91847 plays the escape movie (22) natively;
                    // move the castaway to the Lost Isles once it has played.
                    ObjectGuid playerGUID = player->GetGUID();
                    player->m_Events.AddEventAtOffset([playerGUID]()
                    {
                        Player* castaway = ObjectAccessor::FindPlayer(playerGUID);
                        if (!castaway || castaway->GetZoneId() != ZONE_KEZAN)
                            return;

                        castaway->ExitVehicle();
                        castaway->TeleportTo(648, KezanLostIslesShore.GetPositionX(), KezanLostIslesShore.GetPositionY(), KezanLostIslesShore.GetPositionZ(), KezanLostIslesShore.GetOrientation());
                    }, 9s);
                }
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

void AddSC_kezan_quests()
{
    new npc_bilgewater_buccaneer();
    new spell_kezan_footbomb_impact();
    new spell_kezan_footbomb_kick_impact();
    new npc_fourth_and_goal_target();
    new npc_kezan_deathwing();
    new npc_kezan_partygoer();
    new spell_kezan_party_action();
    new npc_first_bank_vault();
    new spell_kezan_vault_interact();
    new spell_kezan_vault_tool();
    new spell_kezan_vault_power_incorrect();
    new spell_kezan_mook_disguise();
    new spell_kezan_kablooey_bombs();
    new npc_gasbot();
    new spell_kezan_arson_master();
    new spell_kezan_arson_despawn();
    new spell_kezan_gasbot_panel();
    new spell_kezan_gasbot_master();
    new spell_kezan_summon_gasbot();
    new spell_kezan_gasbot_credit();
    new spell_kezan_yacht_mortar();
    new player_script_kezan_events();
}
