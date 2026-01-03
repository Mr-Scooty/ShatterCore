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
#include "CombatAI.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "ScriptedCreature.h"

namespace DunMorogh
{
enum FrozenMountaineer
{
    DATA_SET_ICE_BROKEN      = 1,
    EVENT_RUN_AWAY           = 1,
    SAY_MONSTEREMOTE         = 0,
    SPELL_SUMMON_FROZEN_TOMB = 77906,
    SPELL_FREEZE_ANIM        = 77910
};

enum Sanitron500
{
    NPC_CLEAN_CANNON_X2                 = 46208,
    PATH_SANITRON_500                   = 4618500, // Matches `sql/updates/world/4.3.4/2026_01_02_01_world.sql`
    QUEST_DECONTAMINATION              = 27635,
    SPELL_SANITRON_PULSE                = 86294,
    SPELL_DECONTAMINATION_STAGE_1       = 86075,
    SPELL_DECONTAMINATION_STAGE_2       = 86098,
    SPELL_DECONTAMINATION_COMPLETE      = 86086,
    SPELL_IRRADIATED                    = 80653,
    SPELL_CLEAN_CANNON_VISUAL           = 86080,
    EVENT_SANITRON_START_PATH           = 1,
    EVENT_SANITRON_SAY_COMPLETE         = 2,
    EVENT_SANITRON_SAY_WARNING          = 3,
    EVENT_SANITRON_FINISH               = 4,
    POINT_STAGE_1                       = 0,
    POINT_STAGE_2                       = 1,
    POINT_STAGE_3                       = 2,
    POINT_STAGE_4                       = 3,
    SAY_SANITRON_COMMENCE               = 0,
    SAY_SANITRON_COMPLETE               = 1,
    SAY_SANITRON_WARNING                = 2
};

Position const SanitronExitPos = { -5174.817f, 702.6089f, 291.72363f, 4.680743f };

/*######
# npc_frozen_mountaineer
######*/

class npc_frozen_mountaineer : public CreatureScript
{
public:
    npc_frozen_mountaineer() : CreatureScript("npc_frozen_mountaineer") { }

    struct npc_frozen_mountaineerAI : public ScriptedAI
    {
        npc_frozen_mountaineerAI(Creature* creature) : ScriptedAI(creature), _dataOneSet(false) { }

        void Reset() override
        {
            _events.Reset();
            DoCastSelf(SPELL_SUMMON_FROZEN_TOMB, true);
            DoCastSelf(SPELL_FREEZE_ANIM, true);
        }

        void SetData(uint32 /*type*/, uint32 data) override
        {
            if (data == DATA_SET_ICE_BROKEN && !_dataOneSet)
            {
                me->RemoveAllAuras();
                Talk(SAY_MONSTEREMOTE);
                _dataOneSet = true;
                _events.ScheduleEvent(EVENT_RUN_AWAY, Seconds(3));
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (!_dataOneSet)
                return;

            _events.Update(diff);

            if (_events.ExecuteEvent() == EVENT_RUN_AWAY)
            {
                me->GetMotionMaster()->MovePoint(0, me->GetPositionX() + (std::cos(me->GetOrientation()) * 15.0f), me->GetPositionY() + (std::sin(me->GetOrientation()) * 15.0f), me->GetPositionZ());
                me->DespawnOrUnsummon(Seconds(2));
            }
        }
    private:
        EventMap _events;
        bool _dataOneSet;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_frozen_mountaineerAI(creature);
    }
};

/*######
# npc_sanitron_500
######*/

class npc_sanitron_500 : public CreatureScript
{
public:
    npc_sanitron_500() : CreatureScript("npc_sanitron_500") { }

    struct npc_sanitron_500AI : public VehicleAI
    {
        npc_sanitron_500AI(Creature* creature) : VehicleAI(creature), _inProgress(false) { }

        void Reset() override
        {
            _events.Reset();
            _passengerGuid.Clear();
            _inProgress = false;
        }

        void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
        {
            if (!passenger)
                return;

            if (!apply)
            {
                if (passenger->GetGUID() == _passengerGuid)
                    Reset();
                return;
            }

            if (_inProgress || passenger->GetTypeId() != TYPEID_PLAYER)
                return;

            Player* player = passenger->ToPlayer();
            if (player->GetQuestStatus(QUEST_DECONTAMINATION) != QUEST_STATUS_INCOMPLETE)
            {
                player->ExitVehicle();
                return;
            }

            _inProgress = true;
            _passengerGuid = passenger->GetGUID();

            Talk(SAY_SANITRON_COMMENCE, player);

            _events.ScheduleEvent(EVENT_SANITRON_START_PATH, Milliseconds(1700));
        }

        void MovementInform(uint32 type, uint32 pointId) override
        {
            if (type != WAYPOINT_MOTION_TYPE || !_inProgress)
                return;

            switch (pointId)
            {
                case POINT_STAGE_1:
                    HandleStage1();
                    break;
                case POINT_STAGE_2:
                    HandleStage2();
                    break;
                case POINT_STAGE_3:
                    HandleStage3();
                    break;
                case POINT_STAGE_4:
                    HandleStage4();
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
                    case EVENT_SANITRON_START_PATH:
                        me->LoadPath(PATH_SANITRON_500);
                        me->GetMotionMaster()->MovePath(PATH_SANITRON_500, false);
                        break;
                    case EVENT_SANITRON_SAY_COMPLETE:
                        if (Player* player = GetPassenger())
                            Talk(SAY_SANITRON_COMPLETE, player);
                        break;
                    case EVENT_SANITRON_SAY_WARNING:
                        if (Player* player = GetPassenger())
                            Talk(SAY_SANITRON_WARNING, player);
                        break;
                    case EVENT_SANITRON_FINISH:
                        if (Player* player = GetPassenger())
                        {
                            player->ExitVehicle();
                            player->NearTeleportTo(SanitronExitPos.GetPositionX(), SanitronExitPos.GetPositionY(), SanitronExitPos.GetPositionZ(), SanitronExitPos.GetOrientation());
                        }

                        me->KillSelf();
                        // Despawn the dead Sanitron and force a quick respawn at the original spawn position.
                        me->DespawnOrUnsummon(2000, Seconds(5));
                        Reset();
                        break;
                    default:
                        break;
                }
            }
        }

    private:
        void HandleStage1()
        {
            if (Player* player = GetPassenger())
            {
                DoCastSelf(SPELL_SANITRON_PULSE, true);
                DoCast(player, SPELL_DECONTAMINATION_STAGE_1, true);
            }
        }

        void HandleStage2()
        {
            if (Player* player = GetPassenger())
            {
                DoCastSelf(SPELL_SANITRON_PULSE, true);
                DoCast(player, SPELL_DECONTAMINATION_STAGE_2, true);

                std::list<Creature*> cannons;
                GetCreatureListWithEntryInGrid(cannons, me, NPC_CLEAN_CANNON_X2, 60.0f);
                for (Creature* cannon : cannons)
                    cannon->CastSpell(cannon, SPELL_CLEAN_CANNON_VISUAL, true);
            }
        }

        void HandleStage3()
        {
            if (Player* player = GetPassenger())
            {
                DoCast(player, SPELL_DECONTAMINATION_COMPLETE, true);
                player->RemoveAurasDueToSpell(SPELL_IRRADIATED);
                _events.ScheduleEvent(EVENT_SANITRON_SAY_COMPLETE, Milliseconds(2500));
            }
        }

        void HandleStage4()
        {
            _events.ScheduleEvent(EVENT_SANITRON_SAY_WARNING, Milliseconds(1300));
            _events.ScheduleEvent(EVENT_SANITRON_FINISH, Milliseconds(2800));
        }

        Player* GetPassenger() const
        {
            if (_passengerGuid.IsEmpty())
                return nullptr;

            return ObjectAccessor::GetPlayer(*me, _passengerGuid);
        }

        EventMap _events;
        ObjectGuid _passengerGuid;
        bool _inProgress;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_sanitron_500AI(creature);
    }
};
}

void AddSC_dun_morogh()
{
    using namespace DunMorogh;
    new npc_frozen_mountaineer();
    new npc_sanitron_500();
}
