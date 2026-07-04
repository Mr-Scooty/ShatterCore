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
#include "ObjectAccessor.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "TemporarySummon.h"
#include "Vehicle.h"

namespace Gilneas::Chapter3
{
/*######
## Quest 14465 - To Greymane Manor
######*/

enum ToGreymaneManor
{
    EVENT_START_DRIVE = 1
};

// Reconciled WPP path 860 / sniff spline table (summon-point node dropped,
// the drive starts from the coach's current position). Speed 13.0 yd/s.
Position const SwiftMountainHorsePath[] =
{
    { -1866.48f, 2290.80f, 42.31f }, // joins road (WPP node 1)
    { -1859.98f, 2297.05f, 42.56f },
    { -1857.55f, 2299.51f, 42.29f },
    { -1850.72f, 2305.36f, 41.59f },
    { -1843.22f, 2311.86f, 40.09f },
    { -1836.22f, 2317.61f, 39.09f },
    { -1832.42f, 2321.03f, 38.02f },
    { -1824.22f, 2327.41f, 37.23f },
    { -1816.97f, 2332.91f, 36.73f },
    { -1804.72f, 2342.16f, 36.23f },
    { -1796.72f, 2348.41f, 36.48f },
    { -1789.52f, 2353.78f, 36.94f }, // turn N off road
    { -1787.49f, 2362.50f, 38.99f },
    { -1786.99f, 2364.50f, 39.24f },
    { -1784.99f, 2373.25f, 40.99f },
    { -1783.01f, 2382.44f, 43.14f },
    { -1782.17f, 2392.43f, 46.38f },
    { -1781.17f, 2401.43f, 48.88f },
    { -1780.17f, 2411.18f, 51.63f },
    { -1779.42f, 2421.18f, 54.88f },
    { -1779.20f, 2422.81f, 55.29f }, // bend NE
    { -1774.96f, 2430.75f, 57.79f },
    { -1770.72f, 2438.70f, 60.11f },
    { -1766.49f, 2446.64f, 62.44f },
    { -1762.25f, 2454.59f, 65.55f },
    { -1758.49f, 2461.65f, 67.87f }, // turn E
    { -1757.00f, 2462.54f, 68.52f },
    { -1754.00f, 2462.54f, 69.27f },
    { -1752.25f, 2462.04f, 70.02f },
    { -1743.00f, 2463.54f, 73.02f },
    { -1733.50f, 2465.04f, 76.27f },
    { -1724.50f, 2466.54f, 79.02f },
    { -1717.50f, 2467.79f, 81.27f },
    { -1717.25f, 2466.04f, 81.02f },
    { -1711.24f, 2466.49f, 82.77f }, // turn NE up final ramp
    { -1706.29f, 2474.20f, 86.93f },
    { -1701.29f, 2481.45f, 90.18f },
    { -1696.04f, 2488.95f, 93.43f },
    { -1694.04f, 2492.20f, 94.68f },
    { -1691.42f, 2495.97f, 95.83f },
    { -1684.65f, 2503.78f, 97.61f },
    { -1677.90f, 2511.53f, 98.11f },
    { -1669.87f, 2520.10f, 97.90f }  // END - manor gate, eject here
};

// Summoned by 69255 on quest accept; ride-back 69254 boards the summoner and
// carries the transient phase 184 aura - no manual boarding or phase work here.
struct npc_swift_mountain_horse : public VehicleAI
{
    npc_swift_mountain_horse(Creature* creature) : VehicleAI(creature) { }

    void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
    {
        if (!passenger || !passenger->IsPlayer())
            return;

        if (apply)
            _events.ScheduleEvent(EVENT_START_DRIVE, Seconds(1), Seconds(2));
        else
        {
            _events.Reset();
            me->DespawnOrUnsummon(Seconds(3));
        }
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != EFFECT_MOTION_TYPE && type != POINT_MOTION_TYPE)
            return;

        if (pointId != std::size(SwiftMountainHorsePath) - 1)
            return;

        if (Vehicle* vehicle = me->GetVehicleKit())
            vehicle->RemoveAllPassengers();

        me->DespawnOrUnsummon(Seconds(2));
    }

    void UpdateAI(uint32 diff) override
    {
        VehicleAI::UpdateAI(diff);
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_START_DRIVE:
                    me->GetMotionMaster()->MoveSmoothPath(uint32(std::size(SwiftMountainHorsePath) - 1), SwiftMountainHorsePath, std::size(SwiftMountainHorsePath), false, false, 13.0f);
                    break;
                default:
                    break;
            }
        }
    }

private:
    EventMap _events;
};

/*######
## Quest 14466 - The King's Observatory
######*/

enum KingsObservatory
{
    SPELL_PHASE_QUEST_ZONE_SPECIFIC_03 = 69484 // phase 186
};

// 68953 - The King's Observatory (reward 68954 force-casts this on the player)
class spell_gilneas_king_observatory : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PHASE_QUEST_ZONE_SPECIFIC_03 });
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        if (Player* player = GetHitPlayer())
            player->CastSpell(player, SPELL_PHASE_QUEST_ZONE_SPECIFIC_03, true);
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_gilneas_king_observatory::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

/*######
## Quest 14467 - Alas, Gilneas!
######*/

enum AlasGilneas
{
    CINEMATIC_TELESCOPE = 167
};

// 69257 - Alas, Gilneas! (effects PlayMusic + ActivateObject are native)
class spell_gilneas_alas_gilneas : public SpellScript
{
    void HandleCinematic()
    {
        Player* player = GetHitPlayer();
        if (!player)
            if (Unit* caster = GetCaster())
                player = caster->ToPlayer();

        if (player)
            player->SendCinematicStart(CINEMATIC_TELESCOPE);
    }

    void Register() override
    {
        AfterHit.Register(&spell_gilneas_alas_gilneas::HandleCinematic);
    }
};

/*######
## Quest 24616 - Losing Your Tail
######*/

enum LosingYourTail
{
    QUEST_LOSING_YOUR_TAIL      = 24616,

    NPC_DARK_SCOUT              = 37953,

    SPELL_FREEZING_TRAP_EFFECT  = 70794, // stun + forcecast 95845 + 70795 (summons 37953)
    SPELL_TALISMAN_BREAK_FREE   = 72752,

    // 37953 creature_text groups (break-free line pre-exists as group 0)
    SAY_BREAK_FREE              = 0,
    SAY_TAUNT                   = 1,
    SAY_TALISMAN_HINT           = 2,

    EVENT_TAUNT                 = 1,
    EVENT_TALISMAN_HINT         = 2
};

Creature* GetOwnedDarkScout(Player* player)
{
    std::list<Creature*> scouts;
    player->GetCreatureListWithEntryInGrid(scouts, NPC_DARK_SCOUT, 100.0f);
    for (Creature* scout : scouts)
        if (TempSummon* summon = scout->ToTempSummon())
            if (summon->GetSummonerGUID() == player->GetGUID())
                return scout;

    return nullptr;
}

// 70797 - Belysra's Talisman (the trail ambush itself is fired by trigger 35374, not this spell)
class spell_gilneas_belysras_talisman : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_FREEZING_TRAP_EFFECT, SPELL_TALISMAN_BREAK_FREE });
    }

    SpellCastResult CheckQuest()
    {
        Unit* caster = GetCaster();
        Player* player = caster ? caster->ToPlayer() : nullptr;
        if (!player || player->GetQuestStatus(QUEST_LOSING_YOUR_TAIL) != QUEST_STATUS_INCOMPLETE)
            return SPELL_FAILED_DONT_REPORT;

        return SPELL_CAST_OK;
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Player* player = caster ? caster->ToPlayer() : nullptr;
        if (!player)
            return;

        if (player->HasAura(SPELL_FREEZING_TRAP_EFFECT))
        {
            player->RemoveAurasDueToSpell(SPELL_FREEZING_TRAP_EFFECT);
            player->CastSpell(player, SPELL_TALISMAN_BREAK_FREE, true);
            if (Creature* scout = GetOwnedDarkScout(player))
                scout->AI()->Talk(SAY_BREAK_FREE);
        }
        else if (!GetOwnedDarkScout(player))
            player->CastSpell(player, SPELL_FREEZING_TRAP_EFFECT, true); // fallback when no ambush ran
    }

    // The implicit NEARBY_ENTRY search has no conditions data - pin the target so
    // the cast cannot fail with no scout around (the handler only uses the caster).
    void SetTarget(WorldObject*& target)
    {
        Unit* caster = GetCaster();
        Player* player = caster ? caster->ToPlayer() : nullptr;
        if (!player)
            return;

        if (Creature* scout = GetOwnedDarkScout(player))
            target = scout;
        else
            target = player;
    }

    void Register() override
    {
        OnCheckCast.Register(&spell_gilneas_belysras_talisman::CheckQuest);
        OnObjectTargetSelect.Register(&spell_gilneas_belysras_talisman::SetTarget, EFFECT_1, TARGET_UNIT_NEARBY_ENTRY);
        OnEffectHitTarget.Register(&spell_gilneas_belysras_talisman::HandleDummy, EFFECT_1, SPELL_EFFECT_DUMMY);
    }
};

struct npc_gilneas_dark_scout : public ScriptedAI
{
    npc_gilneas_dark_scout(Creature* creature) : ScriptedAI(creature) { }

    void IsSummonedBy(Unit* summoner) override
    {
        if (!summoner->IsPlayer())
            return;

        _summonerGUID = summoner->GetGUID();
        AttackStart(summoner);
        _events.ScheduleEvent(EVENT_TAUNT, Seconds(3) + Milliseconds(600));
        _events.ScheduleEvent(EVENT_TALISMAN_HINT, Seconds(6) + Milliseconds(400));
    }

    void JustDied(Unit* /*killer*/) override
    {
        me->DespawnOrUnsummon(Seconds(30));
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_TAUNT:
                    Talk(SAY_TAUNT);
                    break;
                case EVENT_TALISMAN_HINT:
                    if (Player* player = ObjectAccessor::GetPlayer(*me, _summonerGUID))
                        Talk(SAY_TALISMAN_HINT, player);
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
    ObjectGuid _summonerGUID;
};

/*######
## Quest 24646 - Take Back What's Ours (Horn of Tal'doren)
######*/

enum HornOfTaldoren
{
    QUEST_TAKE_BACK_WHATS_OURS = 24646,

    NPC_TALDOREN_TRACKER    = 38027,
    NPC_VETERAN_DARK_RANGER = 38022
};

// Sniffed 38027 create positions at the Blackwald cabin front
Position const TaldorenTrackerSummonPos[] =
{
    { -2166.77f, 1608.44f, -43.27f, 6.0f },
    { -2150.44f, 1610.97f, -43.44f, 6.0f },
    { -2134.15f, 1613.50f, -43.60f, 6.0f }
};

// 71061 - Horn of Tal'doren (single effect SEND_EVENT 23338; AfterCast fires on the caster regardless)
class spell_gilneas_horn_of_taldoren : public SpellScript
{
    SpellCastResult CheckRequirements()
    {
        Unit* caster = GetCaster();
        Player* player = caster ? caster->ToPlayer() : nullptr;
        if (!player || player->GetQuestStatus(QUEST_TAKE_BACK_WHATS_OURS) != QUEST_STATUS_INCOMPLETE)
            return SPELL_FAILED_DONT_REPORT;

        if (player->GetDistance(TaldorenTrackerSummonPos[1]) > 100.0f)
            return SPELL_FAILED_NOT_HERE;

        return SPELL_CAST_OK;
    }

    void HandleSummonTrackers()
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        for (Position const& pos : TaldorenTrackerSummonPos)
            if (TempSummon* tracker = caster->SummonCreature(NPC_TALDOREN_TRACKER, pos, TEMPSUMMON_TIMED_DESPAWN, Seconds(300)))
                if (Creature* ranger = tracker->FindNearestCreature(NPC_VETERAN_DARK_RANGER, 40.0f))
                    if (tracker->IsAIEnabled())
                        tracker->AI()->AttackStart(ranger);
    }

    void Register() override
    {
        OnCheckCast.Register(&spell_gilneas_horn_of_taldoren::CheckRequirements);
        AfterCast.Register(&spell_gilneas_horn_of_taldoren::HandleSummonTrackers);
    }
};

}

void AddSC_gilneas_chapter_3()
{
    using namespace Gilneas::Chapter3;
    RegisterCreatureAI(npc_swift_mountain_horse);
    RegisterCreatureAI(npc_gilneas_dark_scout);
    RegisterSpellScript(spell_gilneas_king_observatory);
    RegisterSpellScript(spell_gilneas_alas_gilneas);
    RegisterSpellScript(spell_gilneas_belysras_talisman);
    RegisterSpellScript(spell_gilneas_horn_of_taldoren);
}
