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
 * Abyssal Depths (zone 5145) scripted events.
 *
 * Quest 25987/25988 "Put It On": personal ~40 s possession scene. On accept the
 * player casts the retail summon chain (crew doubles + 41814 faceless + 41840
 * possessed player-double); the double's AI directs the timeline, completes the
 * COMPLETION_EVENT quest and tears the scene down. The summon spells use
 * TARGET_DEST_NEARBY_ENTRY anchored on the questgiver via conditions (see
 * abyssal_cpp.sql); scene actors are private objects, the player gets the
 * personal scene phase 233 for the duration.
 *
 * Quest 26143 "All that Rises": swaps the 26154 Possessed Torrent (42325) for
 * the released Vengeful Torrent (48620) on accept and cleans the ride up on
 * turn-in/abandon (RewardSpell 79052 has no effects in 4.3.4 - set it to 0).
 *
 * Quest 26193/26194 "Defending the Rift": RewardSpell 93268/93302 completion
 * events summon the personal Captain Taylor 50259 / Legionnaire Nazgrim 50261
 * escort that dives into the Abyssal Breach rift (Throne of the Tides lead-in).
 *
 * L'ghorek 42197: "Attuned Runestone of Binding" (57172) re-grant gossip
 * (no create-item spell exists, so SAI cannot do this) and the "L'ghorek
 * Dies!" farewell on accepting 26181/26182.
 */

#include "ScriptMgr.h"
#include "vashjir.h"
#include "CreatureTextMgr.h"
#include "EventMap.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "PassiveAI.h"
#include "PhasingHandler.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "TemporarySummon.h"
#include "Vehicle.h"
#include <span>

namespace Vashjir::AbyssalDepths
{

enum AbyssalQuests
{
    QUEST_PUT_IT_ON_A               = 25987,
    QUEST_PUT_IT_ON_H               = 25988,
    QUEST_TWILIGHT_EXTERMINATION    = 26154,
    QUEST_ALL_THAT_RISES            = 26143,
    QUEST_BACK_TO_DARKBREAK_COVE    = 26181,
    QUEST_BACK_TO_TENEBROUS_CAVERN  = 26182
};

enum AbyssalCreatures
{
    // Put It On scene actors (all summoned by the scene player)
    NPC_HEXASCRUB_DOUBLE            = 41837,
    NPC_JORLAN_DOUBLE               = 41884,
    NPC_FOXY_DOUBLE                 = 41889,
    NPC_RALLINGS_DOUBLE             = 47090,
    NPC_DARKBREAK_GUARD_DOUBLE      = 47094,
    NPC_SIZZLEGRIN_DOUBLE           = 41852,
    NPC_TOLDREK_DOUBLE              = 41886,
    NPC_GERTRUDE_DOUBLE             = 41887,
    NPC_TALEY_DOUBLE                = 47107,
    NPC_NERIUS_DOUBLE               = 47108,
    NPC_CAVERN_GRUNT_DOUBLE         = 47111,
    NPC_MERCILESS_CONTROLLER        = 41814, // "Merciless One in Control of You" - head rider
    NPC_MERCILESS_DOUBLE            = 41840, // "Merciless One" - possessed player double (Vehicle 1388, head seat)

    // torrents
    NPC_POSSESSED_TORRENT           = 42325, // 26154 native possession chain 79045 -> 78952 -> 78955
    NPC_VENGEFUL_TORRENT            = 48620, // 26143 released torrent (VehicleId 1342, SQL)

    // L'ghorek
    NPC_LGHOREK                     = 42197,

    // Defending the Rift aftermath escorts (post-quest entries, faction 2295/1595)
    NPC_TAYLOR_RIFT_ESCORT          = 50259,
    NPC_NAZGRIM_RIFT_ESCORT         = 50261
};

enum AbyssalSpells
{
    // Put It On player-cast summon chain (dest anchored on the questgiver via conditions)
    SPELL_SUMMON_HEXASCRUB_DOUBLE   = 78021,
    SPELL_SUMMON_JORLAN_DOUBLE      = 78083,
    SPELL_SUMMON_FOXY_DOUBLE        = 78085,
    SPELL_SUMMON_RALLINGS_DOUBLE    = 87752,
    SPELL_SUMMON_GUARD_DOUBLE       = 87760,
    SPELL_SUMMON_SIZZLEGRIN_DOUBLE  = 78022,
    SPELL_SUMMON_TOLDREK_DOUBLE     = 78084,
    SPELL_SUMMON_GERTRUDE_DOUBLE    = 78086,
    SPELL_SUMMON_TALEY_DOUBLE       = 87787,
    SPELL_SUMMON_NERIUS_DOUBLE      = 87789,
    SPELL_SUMMON_GRUNT_DOUBLE       = 87792,
    SPELL_SUMMON_MERCILESS_CONTROL  = 77988, // 41814
    SPELL_SUMMON_MERCILESS_DOUBLE   = 78008, // 41840 - cast last, its AI directs the scene

    // Put It On scene dressing (retail 78015/78048/78606/78601 are 5.5.3-only - dropped)
    SPELL_CAMERA_CHANNEL            = 87746, // player channels the cinematic camera at the double
    SPELL_REVERSE_CAST_PUT_IT_ON    = 94397, // script effect at summoner, BP 78004
    SPELL_MERCILESS_ONE_SCREEN      = 78004, // screen effect + periodic dummy, self-targeted effects
    SPELL_COWER_ANIM_KIT            = 78087,
    SPELL_STRANGULATE_STATE         = 78037,
    SPELL_RIDE_VEHICLE_HARDCODED    = 46598,
    SPELL_CLONE_ME                  = 45204, // spell_gen_clone -> offhand chain 45206/45205
    SPELL_COPY_WEAPON               = 41055, // spell_gen_clone_weapon -> 41054

    // Defending the Rift
    SPELL_COMPLETION_EVENT_A        = 93268, // RewardSpell 26193
    SPELL_COMPLETION_EVENT_H        = 93302, // RewardSpell 26194
    SPELL_RE_BREATHER               = 76040
};

enum AbyssalTexts
{
    // 41840 possessed double (shared A/H)
    TEXT_DOUBLE_I_SEE_YOU           = 0,
    TEXT_DOUBLE_DIE                 = 1,
    TEXT_DOUBLE_SIMPLE_MIND         = 2,
    // 41837/41852 engineer double
    TEXT_ENGINEER_ITS_NOT_DEAD      = 0,
    TEXT_ENGINEER_GOING_TO_DIE      = 1,
    TEXT_ENGINEER_YOUR_FAULT        = 2,
    TEXT_ENGINEER_MOMMY             = 3,
    // 41884/41886 soldier double
    TEXT_SOLDIER_WHAT_THE           = 0,
    TEXT_SOLDIER_FACE_IT            = 1,
    TEXT_SOLDIER_KNOCK_IT_OFF       = 2,
    // 41889/41887 third crew double
    TEXT_THIRD_MARBLES              = 0,
    TEXT_THIRD_NUTMEGS              = 1,
    TEXT_THIRD_FORE_AND_AFT         = 2,
    // 42197 L'ghorek
    TEXT_LGHOREK_DIES               = 0,
    // 48620 Vengeful Torrent
    TEXT_TORRENT_RELEASED           = 0, // whisper "%s's bindings have been released! ..."
    TEXT_TORRENT_FREED              = 1, // yell "I am freed! Let us slay Hallazeal ..."
    // 50259/50261 rift escort
    TEXT_ESCORT_FOLLOW_ME           = 0
};

enum AbyssalMisc
{
    PHASE_PUT_IT_ON                 = 233,   // personal scene phase (SQL batch-C allocation)
    ITEM_ATTUNED_RUNESTONE          = 57172,
    GOSSIP_MENU_LGHOREK             = 11607,
    GOSSIP_OPTION_RUNESTONE         = 0,
    POINT_RIFT_DIVE                 = 1
};

// Aftermath Taylor/Nazgrim spawn (sniffed 44490 create position, facing the rift)
Position const RiftEscortSpawnPos = { -5841.18f, 5390.71f, -1213.89f, 5.84f };

// Sniffed 50259 dive spline into the Abyssal Breach (19.3 s on retail)
Position const RiftDivePath[] =
{
    { -5836.06f, 5388.30f, -1209.31f },
    { -5827.14f, 5378.77f, -1207.90f },
    { -5811.93f, 5367.41f, -1219.74f },
    { -5768.05f, 5354.44f, -1260.82f },
    { -5706.03f, 5340.79f, -1322.68f }
};

float constexpr RIFT_DIVE_VELOCITY = 10.0f; // ~193 yd / 19.3 s

uint32 const AllianceSceneSummonSpells[] =
{
    SPELL_SUMMON_HEXASCRUB_DOUBLE, SPELL_SUMMON_JORLAN_DOUBLE, SPELL_SUMMON_FOXY_DOUBLE,
    SPELL_SUMMON_RALLINGS_DOUBLE, SPELL_SUMMON_GUARD_DOUBLE,
    SPELL_SUMMON_MERCILESS_CONTROL, SPELL_SUMMON_MERCILESS_DOUBLE
};

uint32 const HordeSceneSummonSpells[] =
{
    SPELL_SUMMON_SIZZLEGRIN_DOUBLE, SPELL_SUMMON_TOLDREK_DOUBLE, SPELL_SUMMON_GERTRUDE_DOUBLE,
    SPELL_SUMMON_TALEY_DOUBLE, SPELL_SUMMON_NERIUS_DOUBLE, SPELL_SUMMON_GRUNT_DOUBLE,
    SPELL_SUMMON_MERCILESS_CONTROL, SPELL_SUMMON_MERCILESS_DOUBLE
};

uint32 const AllianceCrewDoubles[] =
{
    NPC_HEXASCRUB_DOUBLE, NPC_JORLAN_DOUBLE, NPC_FOXY_DOUBLE,
    NPC_RALLINGS_DOUBLE, NPC_DARKBREAK_GUARD_DOUBLE
};

uint32 const HordeCrewDoubles[] =
{
    NPC_SIZZLEGRIN_DOUBLE, NPC_TOLDREK_DOUBLE, NPC_GERTRUDE_DOUBLE,
    NPC_TALEY_DOUBLE, NPC_NERIUS_DOUBLE, NPC_CAVERN_GRUNT_DOUBLE
};

// all scene actors are per-player summons - resolve them through their summoner
Creature* FindPlayerSummon(WorldObject* source, uint32 entry, ObjectGuid summonerGUID, float range = 60.0f)
{
    std::list<Creature*> summons;
    source->GetCreatureListWithEntryInGrid(summons, entry, range);
    for (Creature* creature : summons)
        if (TempSummon* summon = creature->ToTempSummon())
            if (summon->GetSummonerGUID() == summonerGUID)
                return creature;
    return nullptr;
}

/*######
## npc_abyssal_merciless_double - 41840 "Merciless One" (scene director)
## Summoned last by the scene player; wears the player's looks (clone chain),
## carries the faceless one on its head (Vehicle 1388, seat attachment 11) and
## runs the sniffed ~40 s dialogue timeline.
######*/

enum PutItOnEvents
{
    EVENT_SCENE_ATTACH = 1,
    EVENT_SCENE_PANIC_1,
    EVENT_SCENE_PANIC_2,
    EVENT_SCENE_I_SEE_YOU,
    EVENT_SCENE_NUTMEGS,
    EVENT_SCENE_DIE,
    EVENT_SCENE_SIMPLE_MIND,
    EVENT_SCENE_FORE_AND_AFT,
    EVENT_SCENE_KNOCK_IT_OFF,
    EVENT_SCENE_YOUR_FAULT,
    EVENT_SCENE_SIMPLE_MIND_2,
    EVENT_SCENE_MOMMY,
    EVENT_SCENE_FINISH,
    EVENT_SCENE_FAILSAFE
};

struct npc_abyssal_merciless_double : public PassiveAI
{
    npc_abyssal_merciless_double(Creature* creature) : PassiveAI(creature), _horde(false), _done(false) { }

    void IsSummonedBy(Unit* summoner) override
    {
        Player* player = summoner->ToPlayer();
        if (!player)
        {
            me->DespawnOrUnsummon();
            return;
        }

        _playerGUID = player->GetGUID();
        _horde = player->GetTeam() == HORDE;

        // retail adds a personal phase on accept (PHASE_SHIFT_CHANGE) - fork phase 233
        PhasingHandler::AddPhase(player, PHASE_PUT_IT_ON, true);

        PrepareActor(me, player, false);
        me->SetFacingToObject(player);
        for (uint32 entry : CrewEntries())
            if (Creature* crew = FindPlayerSummon(me, entry, _playerGUID))
                PrepareActor(crew, player, true);
        if (Creature* faceless = FindPlayerSummon(me, NPC_MERCILESS_CONTROLLER, _playerGUID))
            PrepareActor(faceless, player, false);

        // the double is the player: model + weapon copies (generic clone scripts)
        player->CastSpell(me, SPELL_CLONE_ME, true);
        player->CastSpell(me, SPELL_COPY_WEAPON, true);
        DoCastSelf(SPELL_STRANGULATE_STATE, true);
        player->CastSpell(me, SPELL_CAMERA_CHANNEL, true);

        // sniff offsets from the 25987 accept (14:29:31 -> first yells +2 s -> complete +40 s);
        // the NUTMEGS/FORE_AND_AFT/MOMMY slots are unsniffed extra lines (AB1/AB3 text recovery)
        _events.ScheduleEvent(EVENT_SCENE_ATTACH, 500ms);
        _events.ScheduleEvent(EVENT_SCENE_PANIC_1, 2s);
        _events.ScheduleEvent(EVENT_SCENE_PANIC_2, 5s);
        _events.ScheduleEvent(EVENT_SCENE_I_SEE_YOU, 6s);
        _events.ScheduleEvent(EVENT_SCENE_NUTMEGS, 9s);
        _events.ScheduleEvent(EVENT_SCENE_DIE, 14s);
        _events.ScheduleEvent(EVENT_SCENE_SIMPLE_MIND, 16s);
        _events.ScheduleEvent(EVENT_SCENE_FORE_AND_AFT, 20s);
        _events.ScheduleEvent(EVENT_SCENE_KNOCK_IT_OFF, 27s);
        _events.ScheduleEvent(EVENT_SCENE_YOUR_FAULT, 30s);
        _events.ScheduleEvent(EVENT_SCENE_SIMPLE_MIND_2, 32s);
        _events.ScheduleEvent(EVENT_SCENE_MOMMY, 34s);
        _events.ScheduleEvent(EVENT_SCENE_FINISH, 40s);
        _events.ScheduleEvent(EVENT_SCENE_FAILSAFE, 50s);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_SCENE_ATTACH:
                    if (Creature* faceless = FindPlayerSummon(me, NPC_MERCILESS_CONTROLLER, _playerGUID))
                    {
                        // script effect at the summoner: the player casts 78004 on himself (horror screen)
                        faceless->CastSpell(nullptr, SPELL_REVERSE_CAST_PUT_IT_ON, true);
                        if (me->IsVehicle())
                            faceless->CastSpell(me, SPELL_RIDE_VEHICLE_HARDCODED, true);
                    }
                    break;
                case EVENT_SCENE_PANIC_1:
                    CrewTalk(EngineerEntry(), TEXT_ENGINEER_ITS_NOT_DEAD);
                    CrewTalk(SoldierEntry(), TEXT_SOLDIER_WHAT_THE);
                    CrewTalk(ThirdEntry(), TEXT_THIRD_MARBLES);
                    break;
                case EVENT_SCENE_PANIC_2:
                    CrewTalk(EngineerEntry(), TEXT_ENGINEER_GOING_TO_DIE);
                    break;
                case EVENT_SCENE_I_SEE_YOU:
                    TalkToPlayer(TEXT_DOUBLE_I_SEE_YOU);
                    break;
                case EVENT_SCENE_NUTMEGS:
                    CrewTalk(ThirdEntry(), TEXT_THIRD_NUTMEGS);
                    break;
                case EVENT_SCENE_DIE:
                    TalkToPlayer(TEXT_DOUBLE_DIE);
                    break;
                case EVENT_SCENE_SIMPLE_MIND:
                    TalkToPlayer(TEXT_DOUBLE_SIMPLE_MIND);
                    CrewTalk(SoldierEntry(), TEXT_SOLDIER_FACE_IT);
                    break;
                case EVENT_SCENE_FORE_AND_AFT:
                    CrewTalk(ThirdEntry(), TEXT_THIRD_FORE_AND_AFT);
                    break;
                case EVENT_SCENE_KNOCK_IT_OFF:
                    CrewTalk(SoldierEntry(), TEXT_SOLDIER_KNOCK_IT_OFF);
                    break;
                case EVENT_SCENE_YOUR_FAULT:
                    CrewTalk(EngineerEntry(), TEXT_ENGINEER_YOUR_FAULT);
                    break;
                case EVENT_SCENE_SIMPLE_MIND_2:
                    TalkToPlayer(TEXT_DOUBLE_SIMPLE_MIND);
                    break;
                case EVENT_SCENE_MOMMY:
                    CrewTalk(EngineerEntry(), TEXT_ENGINEER_MOMMY);
                    break;
                case EVENT_SCENE_FINISH:
                    if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID))
                    {
                        uint32 questId = _horde ? QUEST_PUT_IT_ON_H : QUEST_PUT_IT_ON_A;
                        if (player->GetQuestStatus(questId) == QUEST_STATUS_INCOMPLETE)
                            player->AreaExploredOrEventHappens(questId);
                    }
                    Cleanup();
                    break;
                case EVENT_SCENE_FAILSAFE:
                    Cleanup();
                    break;
                default:
                    break;
            }
        }
    }

private:
    std::span<uint32 const> CrewEntries() const
    {
        return _horde ? std::span<uint32 const>(HordeCrewDoubles) : std::span<uint32 const>(AllianceCrewDoubles);
    }

    uint32 EngineerEntry() const { return _horde ? NPC_SIZZLEGRIN_DOUBLE : NPC_HEXASCRUB_DOUBLE; }
    uint32 SoldierEntry() const { return _horde ? NPC_TOLDREK_DOUBLE : NPC_JORLAN_DOUBLE; }
    uint32 ThirdEntry() const { return _horde ? NPC_GERTRUDE_DOUBLE : NPC_FOXY_DOUBLE; }

    static void PrepareActor(Creature* actor, Player* player, bool crewMember)
    {
        // scene actors are visible to the scene player only
        actor->SetPrivateObjectOwner(player->GetGUID());
        actor->UpdateObjectVisibility();
        actor->DespawnOrUnsummon(1min); // backstop - the director cleans up earlier
        if (crewMember)
        {
            actor->CastSpell(actor, SPELL_COWER_ANIM_KIT, true);
            actor->GetMotionMaster()->MoveRandom(8.0f); // panicked scrambling
        }
    }

    void CrewTalk(uint32 entry, uint8 group)
    {
        if (Creature* crew = FindPlayerSummon(me, entry, _playerGUID))
            sCreatureTextMgr->SendChat(crew, group, ObjectAccessor::GetPlayer(*me, _playerGUID));
    }

    void TalkToPlayer(uint8 group)
    {
        Talk(group, ObjectAccessor::GetPlayer(*me, _playerGUID));
    }

    void Cleanup()
    {
        if (_done)
            return;
        _done = true;

        if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID))
        {
            player->InterruptNonMeleeSpells(true, SPELL_CAMERA_CHANNEL);
            player->RemoveAurasDueToSpell(SPELL_CAMERA_CHANNEL);
            player->RemoveAurasDueToSpell(SPELL_MERCILESS_ONE_SCREEN);
            PhasingHandler::RemovePhase(player, PHASE_PUT_IT_ON, true);
        }

        // 50630 Eject All Passengers has no core handler on this fork - eject manually
        if (Creature* faceless = FindPlayerSummon(me, NPC_MERCILESS_CONTROLLER, _playerGUID))
        {
            faceless->ExitVehicle();
            faceless->DespawnOrUnsummon(1s);
        }
        for (uint32 entry : CrewEntries())
            if (Creature* crew = FindPlayerSummon(me, entry, _playerGUID))
                crew->DespawnOrUnsummon();

        me->DespawnOrUnsummon(2s);
    }

    EventMap _events;
    ObjectGuid _playerGUID;
    bool _horde;
    bool _done;
};

/*######
## npc_abyssal_lghorek - 42197 L'ghorek
## Gossip 11607/0: re-grant the Attuned Runestone of Binding (no create-item
## spell exists for 57172). QuestAccept 26181/26182: farewell boss whisper.
######*/

struct npc_abyssal_lghorek : public PassiveAI
{
    npc_abyssal_lghorek(Creature* creature) : PassiveAI(creature) { }

    bool GossipSelect(Player* player, uint32 menuId, uint32 gossipListId) override
    {
        if (menuId != GOSSIP_MENU_LGHOREK || gossipListId != GOSSIP_OPTION_RUNESTONE)
            return false;

        CloseGossipMenuFor(player);
        if ((player->GetQuestStatus(QUEST_TWILIGHT_EXTERMINATION) == QUEST_STATUS_INCOMPLETE
            || player->GetQuestStatus(QUEST_ALL_THAT_RISES) == QUEST_STATUS_INCOMPLETE)
            && !player->HasItemCount(ITEM_ATTUNED_RUNESTONE))
            player->AddItem(ITEM_ATTUNED_RUNESTONE, 1);
        return true;
    }

    void QuestAccept(Player* player, Quest const* quest) override
    {
        if (quest->GetQuestId() == QUEST_BACK_TO_DARKBREAK_COVE || quest->GetQuestId() == QUEST_BACK_TO_TENEBROUS_CAVERN)
            Talk(TEXT_LGHOREK_DIES, player);
    }
};

/*######
## npc_abyssal_rift_escort - 50259 Captain Taylor / 50261 Legionnaire Nazgrim
## Personal aftermath escort of Defending the Rift: re-breather, "Follow me!",
## then the sniffed 19.3 s dive spline into the rift.
######*/

enum RiftEscortEvents
{
    EVENT_ESCORT_REBREATHER = 1,
    EVENT_ESCORT_FOLLOW_ME,
    EVENT_ESCORT_DIVE
};

struct npc_abyssal_rift_escort : public PassiveAI
{
    npc_abyssal_rift_escort(Creature* creature) : PassiveAI(creature) { }

    void IsSummonedBy(Unit* summoner) override
    {
        _playerGUID = summoner->GetGUID();
        me->SetDisableGravity(true);
        _events.ScheduleEvent(EVENT_ESCORT_REBREATHER, 1s);
        _events.ScheduleEvent(EVENT_ESCORT_FOLLOW_ME, 2s + 500ms);
        _events.ScheduleEvent(EVENT_ESCORT_DIVE, 4s);
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type == EFFECT_MOTION_TYPE && id == POINT_RIFT_DIVE)
            me->DespawnOrUnsummon(2s);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_ESCORT_REBREATHER:
                    DoCastSelf(SPELL_RE_BREATHER, true);
                    break;
                case EVENT_ESCORT_FOLLOW_ME:
                    Talk(TEXT_ESCORT_FOLLOW_ME, ObjectAccessor::GetPlayer(*me, _playerGUID));
                    break;
                case EVENT_ESCORT_DIVE:
                    me->GetMotionMaster()->MoveSmoothPath(POINT_RIFT_DIVE, RiftDivePath, std::size(RiftDivePath), false, true, RIFT_DIVE_VELOCITY);
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

/*######
## player_abyssal_depths - Put It On scene start + All that Rises torrent handling
######*/

class player_abyssal_depths : public PlayerScript
{
public:
    player_abyssal_depths() : PlayerScript("player_abyssal_depths") { }

    void OnPlayerQuestStatusChange(Player* player, uint32 questId) override
    {
        switch (questId)
        {
            case QUEST_PUT_IT_ON_A:
            case QUEST_PUT_IT_ON_H:
                if (player->GetQuestStatus(questId) == QUEST_STATUS_INCOMPLETE)
                    StartPutItOnScene(player, questId);
                break;
            case QUEST_TWILIGHT_EXTERMINATION:
                // abandoning the possession quest revokes the Possessed Torrent
                if (player->GetQuestStatus(questId) == QUEST_STATUS_NONE)
                    CleanupTorrent(player);
                break;
            case QUEST_ALL_THAT_RISES:
                switch (player->GetQuestStatus(questId))
                {
                    case QUEST_STATUS_INCOMPLETE:
                        ReleaseTorrent(player);
                        break;
                    case QUEST_STATUS_NONE:
                    case QUEST_STATUS_REWARDED:
                        // RewardSpell 79052 has no effects in 4.3.4 (SQL sets it to 0) - eject here
                        CleanupTorrent(player);
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }

    void OnPlayerLogin(Player* player) override
    {
        // logged out mid-scene: the personal phase and the scene summons are gone -
        // restart the scene (self-heals into a plain completion if the summons fail)
        for (uint32 questId : { uint32(QUEST_PUT_IT_ON_A), uint32(QUEST_PUT_IT_ON_H) })
            if (player->GetQuestStatus(questId) == QUEST_STATUS_INCOMPLETE)
                StartPutItOnScene(player, questId);
    }

private:
    static void StartPutItOnScene(Player* player, uint32 questId)
    {
        if (FindPlayerSummon(player, NPC_MERCILESS_DOUBLE, player->GetGUID()))
            return; // scene already running

        // retail: the player casts the whole summon set on accept; the dests are
        // anchored on the questgiver (41666/41669) via conditions. The double is
        // summoned last - its AI phases the player and directs the scene.
        if (player->GetTeam() == ALLIANCE)
            for (uint32 spellId : AllianceSceneSummonSpells)
                player->CastSpell(nullptr, spellId, true);
        else
            for (uint32 spellId : HordeSceneSummonSpells)
                player->CastSpell(nullptr, spellId, true);

        // graceful degradation: if the chain could not deploy (no questgiver anchor
        // in range), do not leave a COMPLETION_EVENT quest stuck - just complete it
        player->m_Events.AddEventAtOffset([player, questId]()
        {
            if (player->GetQuestStatus(questId) != QUEST_STATUS_INCOMPLETE)
                return;
            if (!FindPlayerSummon(player, NPC_MERCILESS_DOUBLE, player->GetGUID()))
            {
                player->AreaExploredOrEventHappens(questId);
                PhasingHandler::RemovePhase(player, PHASE_PUT_IT_ON, true);
            }
        }, 5s);
    }

    static void ReleaseTorrent(Player* player)
    {
        Position pos = player->GetPosition();
        Unit* vehicle = player->GetVehicleBase();
        if (vehicle && vehicle->GetEntry() == NPC_VENGEFUL_TORRENT)
            return; // already released
        if (vehicle && vehicle->GetEntry() == NPC_POSSESSED_TORRENT)
        {
            pos = vehicle->GetPosition();
            player->ExitVehicle();
            if (Creature* possessed = vehicle->ToCreature())
                possessed->DespawnOrUnsummon(1s);
        }

        Creature* torrent = player->SummonCreature(NPC_VENGEFUL_TORRENT, pos, TEMPSUMMON_TIMED_DESPAWN, 30min);
        if (!torrent)
            return;

        ObjectGuid torrentGUID = torrent->GetGUID();
        player->m_Events.AddEventAtOffset([player, torrentGUID]()
        {
            if (Creature* torrent = ObjectAccessor::GetCreature(*player, torrentGUID))
                player->EnterVehicle(torrent, 0);
        }, 500ms);

        ObjectGuid playerGUID = player->GetGUID();
        torrent->m_Events.AddEventAtOffset([torrent, playerGUID]()
        {
            if (Player* player = ObjectAccessor::GetPlayer(*torrent, playerGUID))
                sCreatureTextMgr->SendChat(torrent, TEXT_TORRENT_RELEASED, player);
        }, 2s);
        torrent->m_Events.AddEventAtOffset([torrent, playerGUID]()
        {
            if (Player* player = ObjectAccessor::GetPlayer(*torrent, playerGUID))
                sCreatureTextMgr->SendChat(torrent, TEXT_TORRENT_FREED, player);
        }, 5s);
    }

    static void CleanupTorrent(Player* player)
    {
        Unit* vehicle = player->GetVehicleBase();
        if (!vehicle || (vehicle->GetEntry() != NPC_POSSESSED_TORRENT && vehicle->GetEntry() != NPC_VENGEFUL_TORRENT))
            return;

        player->ExitVehicle();
        if (Creature* torrent = vehicle->ToCreature())
            torrent->DespawnOrUnsummon(1s);
    }
};

/*######
## spell_abyssal_put_it_on_reverse_cast - 94397 (script effect at summoner, BP 78004)
######*/

class spell_abyssal_put_it_on_reverse_cast : public SpellScript
{
    void HandleScriptEffect(SpellEffIndex /*effIndex*/)
    {
        // hit unit = the scene player; 78004's effects are self-targeted (horror screen)
        if (Unit* target = GetHitUnit())
            target->CastSpell(target, uint32(GetEffectValue()), true);
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_abyssal_put_it_on_reverse_cast::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

/*######
## spell_abyssal_defending_the_rift_completion - 93268 / 93302 (quest reward events)
######*/

class spell_abyssal_defending_the_rift_completion : public SpellScript
{
    void HandleScriptEffect(SpellEffIndex /*effIndex*/)
    {
        Player* player = GetHitPlayer();
        if (!player)
            return;

        uint32 entry = GetSpellInfo()->Id == SPELL_COMPLETION_EVENT_H ? NPC_NAZGRIM_RIFT_ESCORT : NPC_TAYLOR_RIFT_ESCORT;
        player->SummonCreature(entry, RiftEscortSpawnPos, TEMPSUMMON_TIMED_DESPAWN, 45 * IN_MILLISECONDS, 0, player->GetGUID());
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_abyssal_defending_the_rift_completion::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

} // namespace Vashjir::AbyssalDepths

void AddSC_vashjir_abyssal_depths()
{
    using namespace Vashjir::AbyssalDepths;
    new player_abyssal_depths();
    RegisterCreatureAI(npc_abyssal_merciless_double);
    RegisterCreatureAI(npc_abyssal_lghorek);
    RegisterCreatureAI(npc_abyssal_rift_escort);
    RegisterSpellScript(spell_abyssal_put_it_on_reverse_cast);
    RegisterSpellScript(spell_abyssal_defending_the_rift_completion);
}
