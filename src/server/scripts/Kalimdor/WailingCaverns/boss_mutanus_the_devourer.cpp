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

#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "MotionMaster.h"
#include "ScriptedCreature.h"
#include "wailing_caverns.h"

namespace WailingCaverns
{
enum MutanusSpells
{
    SPELL_THUNDERCRACK        = 8150,
    SPELL_TERRIFY             = 7399,
    SPELL_NARALEXS_NIGHTMARE  = 7967
};

enum MutanusEvents
{
    EVENT_THUNDERCRACK = 1,
    EVENT_TERRIFY,
    EVENT_NARALEXS_NIGHTMARE
};

enum MutanusPoints
{
    POINT_EMERGE = 1
};

// Summon point lies in the nightmare pool below the navmesh, so he is walked out on a forced
// spline to the edge of Naralex's platform (on the disciple's escort track) before engaging.
Position const MutanusEmergePos = { 119.0f, 243.0f, -96.0f, 3.7f };

class boss_mutanus_the_devourer : public CreatureScript
{
public:
    boss_mutanus_the_devourer() : CreatureScript("boss_mutanus_the_devourer") { }

    struct boss_mutanus_the_devourerAI : public ScriptedAI
    {
        boss_mutanus_the_devourerAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        void IsSummonedBy(Unit* /*summoner*/) override
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetWalk(false);
            me->GetMotionMaster()->MovePoint(POINT_EMERGE, MutanusEmergePos, false);
        }

        void MovementInform(uint32 type, uint32 pointId) override
        {
            if (type != POINT_MOTION_TYPE || pointId != POINT_EMERGE)
                return;

            me->SetHomePosition(MutanusEmergePos);
            me->SetReactState(REACT_AGGRESSIVE);
            DoZoneInCombat();
        }

        void Reset() override
        {
            events.Reset();
        }

        void JustEngagedWith(Unit* /*who*/) override
        {
            events.ScheduleEvent(EVENT_THUNDERCRACK, Seconds(9), Seconds(14));
            events.ScheduleEvent(EVENT_TERRIFY, Seconds(15), Seconds(20));
            events.ScheduleEvent(EVENT_NARALEXS_NIGHTMARE, Seconds(25), Seconds(30));
        }

        void JustDied(Unit* /*killer*/) override
        {
            instance->SetData(DATA_MUTANUS_THE_DEVOURER, DONE);
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
                    case EVENT_THUNDERCRACK:
                        DoCastSelf(SPELL_THUNDERCRACK);
                        events.Repeat(Seconds(12), Seconds(18));
                        break;
                    case EVENT_TERRIFY:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 30.0f, true))
                            DoCast(target, SPELL_TERRIFY);
                        events.Repeat(Seconds(18), Seconds(25));
                        break;
                    case EVENT_NARALEXS_NIGHTMARE:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 30.0f, true))
                            DoCast(target, SPELL_NARALEXS_NIGHTMARE);
                        events.Repeat(Seconds(25), Seconds(35));
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
        EventMap events;
        InstanceScript* instance;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetWailingCavernsAI<boss_mutanus_the_devourerAI>(creature);
    }
};
}

void AddSC_boss_mutanus_the_devourer()
{
    using namespace WailingCaverns;
    new boss_mutanus_the_devourer();
}
