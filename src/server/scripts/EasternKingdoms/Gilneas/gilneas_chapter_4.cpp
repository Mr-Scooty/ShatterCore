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
 * Gilneas, Chapter 4: The Battle for Gilneas City (24904), The Hunt For
 * Sylvanas (24902), Slowing the Inevitable (24920), Knee-Deep (24678),
 * Patriarch's Blessing funeral (24679), They Have Allies (24681) and the
 * Endgame gunship voyage (26706) with the 14434 map handoff.
 * All positions/timings extracted from the retail 4.4.0 sniff.
 */

#include "ScriptMgr.h"
#include "CombatAI.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "TemporarySummon.h"
#include "Vehicle.h"
#include <array>

using namespace std::chrono_literals;

namespace Gilneas::Chapter4
{
enum Chapter4Quests
{
    QUEST_TO_GREYMANE_MANOR         = 14465,
    QUEST_LOSING_YOUR_TAIL          = 24616,
    QUEST_BATTLE_FOR_GILNEAS_CITY   = 24904,
    QUEST_THE_HUNT_FOR_SYLVANAS     = 24902,
    QUEST_KNEE_DEEP                 = 24678,
    QUEST_PATRIARCHS_BLESSING       = 24679,
    QUEST_ENDGAME                   = 26706,
    QUEST_RUTTHERAN_VILLAGE    = 14434
};

/*######
## Quest 24904 - The Battle for Gilneas City
######*/

enum BattleForGilneasCity
{
    NPC_PRINCE_LIAM_BATTLE          = 38218,
    NPC_LORNA_BATTLE                = 38426,
    NPC_DARIUS_BATTLE               = 38415,
    NPC_GOREROT                     = 38331,
    NPC_VILE_ABOMINATION            = 38420,
    NPC_GENN_COURT                  = 38470,
    NPC_SYLVANAS_COURT              = 38469,
    NPC_LIAM_DYING                  = 38474,
    NPC_SOULTETHERED_BANSHEE        = 38473,
    NPC_FORSAKEN_CROSSBOWMAN        = 38210,
    NPC_FORSAKEN_INFANTRY           = 38192,
    NPC_BFGC_KILL_CREDIT            = 38854,

    SPELL_GREYMANE_TRANSFORM_FX     = 86141,
    SPELL_BANSHEE_QUEENS_WAIL       = 72113,
    SPELL_AIMED_SHOT_SCENE          = 72114,
    SPELL_LIAM_SLAIN_DUMMY          = 72361,
    SPELL_LIAM_SYLVANAS_DUMMY       = 72115,

    // Prince Liam 38218 creature_text
    SAY_LIAM_SPEECH_0               = 0,
    SAY_LIAM_SPEECH_1               = 1,
    SAY_LIAM_SPEECH_2               = 2,
    SAY_LIAM_SPEECH_3               = 3,
    SAY_LIAM_SPEECH_4               = 4,
    SAY_LIAM_SPEECH_5               = 5,
    SAY_LIAM_SPEECH_6               = 6,
    SAY_LIAM_ATTACK                 = 7,
    SAY_LIAM_PUSH_THEM_BACK         = 8,
    SAY_LIAM_ABOMINATIONS           = 9,
    SAY_LIAM_SORE_EYES              = 10,
    SAY_LIAM_PRESS_ON               = 11,
    SAY_LIAM_PREVAIL                = 12,
    SAY_LIAM_TIME_IS_UP             = 13,

    SAY_LORNA_VILLAGERS             = 0,
    SAY_DARIUS_CATAPULTS            = 0,
    SAY_DARIUS_JOIN_KING            = 1,
    SAY_GOREROT_CRUSH               = 0,
    SAY_GENN_BLOCK_RETREAT          = 0,
    SAY_GENN_SYLVANAS               = 1,
    SAY_GENN_LIAM_NO                = 2,
    SAY_SYLVANAS_ENOUGH             = 0,
    SAY_SYLVANAS_STUBBORN_LEADER    = 1,
    SAY_SYLVANAS_SUCH_A_WASTE       = 2,
    SAY_DYING_LIAM_FATHER           = 0,
    SAY_DYING_LIAM_WE_DID_IT        = 1,
    SAY_DYING_LIAM_TOOK_BACK        = 2,

    ACTION_RESTART_BATTLE           = 1,

    POINT_LIAM_GATES                = 1,
    POINT_LIAM_BOULEVARD            = 2,
    POINT_LIAM_BARRICADE            = 3,
    POINT_LIAM_COURT                = 4
};

enum BattleEvents
{
    EVENT_BATTLE_SPEECH_0           = 1,
    EVENT_BATTLE_SPEECH_1,
    EVENT_BATTLE_SPEECH_2,
    EVENT_BATTLE_SPEECH_3,
    EVENT_BATTLE_SPEECH_4,
    EVENT_BATTLE_SPEECH_5,
    EVENT_BATTLE_SPEECH_6,
    EVENT_BATTLE_BARK_PUSH,
    EVENT_BATTLE_MARCH_B,
    EVENT_BATTLE_ABOM_LIAM,
    EVENT_BATTLE_ABOM_LORNA,
    EVENT_BATTLE_ABOM_LIAM_2,
    EVENT_BATTLE_ABOM_WAVE,
    EVENT_BATTLE_PRESS_ON,
    EVENT_BATTLE_BARK_ATTACK,
    EVENT_BATTLE_GOREROT_YELL,
    EVENT_BATTLE_GOREROT_LEAP,
    EVENT_BATTLE_DARIUS_BARK,
    EVENT_BATTLE_MARCH_C,
    EVENT_BATTLE_DARIUS_JOIN,
    EVENT_BATTLE_MARCH_D,
    EVENT_BATTLE_BARK_PREVAIL,
    EVENT_BATTLE_BARK_TIME_UP,
    EVENT_BATTLE_GENN_RUN,
    EVENT_BATTLE_GENN_JUMP,
    EVENT_BATTLE_GENN_YELL,
    EVENT_BATTLE_COURT_FIGHT,
    EVENT_BATTLE_ENOUGH_CHECK,
    EVENT_BATTLE_ENOUGH_FALLBACK,
    EVENT_BATTLE_FINALE_STEP,
    EVENT_BATTLE_REARM
};

// Liam's march, run 2 splines (sniff lines 18402388-19344708), combat jitter removed.
// Segment A: gates -> first square (march starts with "FOR GILNEAS!!!", 15:51:53)
Position const LiamPathGates[] =
{
    { -1416.94f, 1288.90f, 36.47f },   // 15:51:53.376  L18402388
    { -1422.60f, 1314.04f, 36.47f },   // 15:51:55.045  L18405009
    { -1430.66f, 1342.41f, 35.37f },   // 15:51:58.296  L18409276
    { -1435.96f, 1363.55f, 35.56f },   // 15:52:01.516  L18414288
    { -1438.50f, 1380.21f, 35.56f },   // 15:52:04.756  L18420257
    { -1443.68f, 1401.14f, 35.56f },   // 15:52:06.367  L18424186
    { -1465.55f, 1398.72f, 35.81f },   // corridor waypoint of long spline L18434148
    { -1478.80f, 1402.22f, 35.56f },   // corridor waypoint of long spline L18434148
    { -1503.92f, 1415.79f, 35.56f },   // 15:52:11.228  L18434148
    { -1514.17f, 1419.01f, 35.40f },   // 15:52:22.543  L18460513
};
// Segment B: square -> overlook -> down the stairs -> boulevard west
Position const LiamPathBoulevard[] =
{
    { -1541.94f, 1423.83f, 35.56f },   // 15:52:56.533  L18566765
    { -1551.21f, 1413.55f, 35.56f },   // 15:52:58.202  L18568752
    { -1551.90f, 1399.52f, 36.47f },   // 15:52:59.823  L18576430
    { -1553.29f, 1377.83f, 35.59f },   // 15:53:01.452  L18580011
    { -1557.17f, 1358.55f, 35.56f },   // 15:53:03.001  L18584024
    { -1559.54f, 1335.53f, 35.56f },   // 15:53:06.215  L18590230
    { -1568.49f, 1320.71f, 35.56f },   // 15:53:32.205  L18652180  <- stage 3 fires here
    { -1596.12f, 1314.50f, 22.92f },   // 15:53:41.865  L18674983 (stairs down)
    { -1625.02f, 1309.07f, 20.43f },   // 15:54:02.863  L18718808
    { -1637.74f, 1307.63f, 19.66f },   // 15:54:04.503  L18724101
    { -1677.19f, 1307.97f, 19.90f },   // 15:54:06.090  L18728197  <- stage 4 "Press on!"
};
// Segment C: boulevard -> Darius' barricade
Position const LiamPathBarricade[] =
{
    { -1689.63f, 1304.15f, 19.78f },   // 15:56:25.301  L19087042
    { -1713.93f, 1303.93f, 20.30f },   // 15:56:26.996  L19089992
    { -1748.51f, 1327.23f, 19.83f },   // 15:56:31.838  L19097280
    { -1757.68f, 1344.13f, 19.84f },   // 15:56:33.450  L19098273
    { -1761.56f, 1366.13f, 19.75f },   // 15:56:35.068  L19099030
    { -1731.20f, 1390.25f, 20.54f },   // 15:56:38.327  L19100486
    { -1719.91f, 1398.67f, 21.67f },   // 15:56:41.552  L19102220
};
// Segment D: barricade -> Greymane Court
Position const LiamPathCourt[] =
{
    { -1730.76f, 1391.18f, 20.72f },   // 15:57:55.963  L19182575
    { -1752.71f, 1385.24f, 19.79f },   // 15:57:57.565  L19185383
    { -1779.39f, 1379.17f, 19.76f },   // 15:57:59.274  L19188048
    { -1788.88f, 1376.65f, 19.87f },   // 15:58:02.516  L19193039
    { -1800.98f, 1393.22f, 19.77f },   // 15:58:04.086  L19195459
    { -1802.33f, 1408.17f, 19.86f },   // 15:58:05.757  L19198159
    { -1804.16f, 1423.31f, 19.68f },   // 15:58:07.357  L19202329
    { -1806.62f, 1446.45f, 19.00f },   // 15:58:08.999  L19205983
    { -1807.79f, 1470.31f, 19.15f },   // 15:58:20.336  L19240962
    { -1808.11f, 1487.56f, 19.40f },   // 15:58:21.973  L19244218
    { -1804.74f, 1511.05f, 19.78f },   // 15:58:23.504  L19248041
    { -1800.13f, 1539.63f, 20.08f },   // 15:58:26.765  L19254761
    { -1795.73f, 1569.68f, 20.49f },   // 15:58:30.007  L19262253
    { -1780.15f, 1606.23f, 20.48f },   // 15:58:42.986  L19293735
    { -1755.51f, 1637.60f, 20.46f },   // 15:58:52.658  L19324692
    { -1748.92f, 1663.42f, 22.30f },   // 15:58:54.540  L19331427
    { -1757.84f, 1676.58f, 22.30f },   // 15:58:59.201  L19340097
    { -1762.71f, 1680.46f, 22.30f },   // 15:59:00.830  L19344708  (final; holds through finale)
};

// Far-west Vile Abomination statics activated at stage 3 ("lure them into the open").
Position const AbominationWavePos[] =
{
    { -1691.31f, 1323.97f, 18.21f },
    { -1686.37f, 1325.34f, 17.36f },
    { -1689.80f, 1328.62f, 16.19f },
    { -1737.03f, 1299.07f, 20.37f },
    { -1729.96f, 1332.53f, 20.35f }
};
Position const AbominationWaveDest = { -1637.74f, 1307.63f, 19.66f }; // boulevard node they shamble toward

Position const GorerotPerchPos     = { -1674.29f, 1446.18f, 52.37f, 3.892f }; // DB guid 256911
Position const GorerotGroundPos    = { -1707.95f, 1415.44f, 21.67f };         // leap landing, mt 2290

// Greymane Court finale (scene center = Sylvanas' spawn, DB guid 257032)
Position const GennRunInPath[] =
{
    { -1741.66f, 1662.78f, 22.24f },   // L19330800
    { -1709.61f, 1638.80f, 20.80f },   // L19338124
    { -1699.16f, 1630.61f, 20.57f },   // jump-off of 72107
};
Position const GennFightAnchorPos  = { -1680.57f, 1613.74f, 20.49f };
Position const GennKnockbackPos    = { -1685.19f, 1617.30f, 20.49f };
Position const GennGriefPos        = { -1681.09f, 1614.42f, 20.49f };
Position const DyingLiamRunPos     = { -1660.79f, 1620.83f, 20.49f };
Position const DyingLiamInterposePath[] =
{
    { -1684.20f, 1617.44f, 20.49f },
    { -1680.27f, 1613.84f, 20.49f },
};
Position const SylvanasFleePath[] =
{
    { -1636.61f, 1620.66f, 21.48f },   // L19534654
    { -1631.11f, 1645.55f, 21.65f },   // L19536718, DestroyObject there
};

// Finale sub-beat delays (ms), offsets vs "Enough!" = 0 / 2.2 / 3.2 / 6.8 / 12.8 /
// 12.9 / 13.2 / 16.0 / 16.5 / 19.8 / 22.9 / +reset.
uint32 const BattleFinaleStepDelays[] = { 2200, 1000, 3600, 6000, 100, 300, 2800, 500, 3300, 3100, 17100 };

struct npc_battle_for_gilneas_controller : public ScriptedAI
{
    npc_battle_for_gilneas_controller(Creature* creature) : ScriptedAI(creature)
    {
        Initialize();
    }

    void Initialize()
    {
        _running = false;
        _enoughStarted = false;
        _finaleStep = 0;
        _armTimer = 2000;
        _gennGUID.Clear();
        _sylvanasGUID.Clear();
        _dyingLiamGUID.Clear();
        _lornaGUID.Clear();
        _dariusGUID.Clear();
        _gorerotGUID.Clear();
    }

    void JustAppeared() override
    {
        ScriptedAI::JustAppeared();
        Initialize();
        _events.Reset();
    }

    void SetData(uint32 id, uint32 value) override
    {
        // Lorna 38611's retry gossip (menu 12693) relays SET_DATA 1 1 via SAI.
        if (id == 1 && value == 1)
            TryStart();
    }

    void DoAction(int32 action) override
    {
        if (action == ACTION_RESTART_BATTLE)
            TryStart();
    }

    void UpdateAI(uint32 diff) override
    {
        if (_armTimer <= diff)
        {
            _armTimer = 2000;
            if (!_running)
            {
                std::list<Player*> players;
                me->GetPlayerListInGrid(players, 30.0f);
                for (Player* player : players)
                {
                    if (player->IsAlive() && player->GetQuestStatus(QUEST_BATTLE_FOR_GILNEAS_CITY) == QUEST_STATUS_INCOMPLETE)
                    {
                        TryStart();
                        break;
                    }
                }
            }
        }
        else
            _armTimer -= diff;

        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_BATTLE_SPEECH_0:
                    Talk(SAY_LIAM_SPEECH_0);
                    break;
                case EVENT_BATTLE_SPEECH_1:
                    Talk(SAY_LIAM_SPEECH_1);
                    break;
                case EVENT_BATTLE_SPEECH_2:
                    Talk(SAY_LIAM_SPEECH_2);
                    break;
                case EVENT_BATTLE_SPEECH_3:
                    Talk(SAY_LIAM_SPEECH_3);
                    break;
                case EVENT_BATTLE_SPEECH_4:
                    Talk(SAY_LIAM_SPEECH_4);
                    me->HandleEmoteCommand(EMOTE_ONESHOT_POINT);
                    break;
                case EVENT_BATTLE_SPEECH_5:
                    Talk(SAY_LIAM_SPEECH_5);
                    break;
                case EVENT_BATTLE_SPEECH_6:
                    Talk(SAY_LIAM_SPEECH_6);
                    me->GetMotionMaster()->MoveSmoothPath(POINT_LIAM_GATES, LiamPathGates, std::size(LiamPathGates), false, false, 8.0f);
                    break;
                case EVENT_BATTLE_BARK_PUSH:
                    Talk(SAY_LIAM_PUSH_THEM_BACK);
                    break;
                case EVENT_BATTLE_MARCH_B:
                    me->GetMotionMaster()->MoveSmoothPath(POINT_LIAM_BOULEVARD, LiamPathBoulevard, std::size(LiamPathBoulevard), false, false, 8.0f);
                    break;
                case EVENT_BATTLE_ABOM_LIAM:
                    Talk(SAY_LIAM_ABOMINATIONS);
                    break;
                case EVENT_BATTLE_ABOM_LORNA:
                    if (Creature* lorna = GetActor(NPC_LORNA_BATTLE, _lornaGUID))
                        lorna->AI()->Talk(SAY_LORNA_VILLAGERS);
                    break;
                case EVENT_BATTLE_ABOM_LIAM_2:
                    Talk(SAY_LIAM_SORE_EYES);
                    break;
                case EVENT_BATTLE_ABOM_WAVE:
                    ActivateAbominationWave();
                    break;
                case EVENT_BATTLE_PRESS_ON:
                    Talk(SAY_LIAM_PRESS_ON);
                    break;
                case EVENT_BATTLE_BARK_ATTACK:
                    Talk(SAY_LIAM_ATTACK);
                    break;
                case EVENT_BATTLE_GOREROT_YELL:
                    if (Creature* gorerot = GetActor(NPC_GOREROT, _gorerotGUID))
                        gorerot->AI()->Talk(SAY_GOREROT_CRUSH);
                    break;
                case EVENT_BATTLE_GOREROT_LEAP:
                    if (Creature* gorerot = GetActor(NPC_GOREROT, _gorerotGUID))
                    {
                        gorerot->GetMotionMaster()->MoveJump(GorerotGroundPos, 20.0f, 15.0f);
                        gorerot->SetHomePosition(GorerotGroundPos.GetPositionX(), GorerotGroundPos.GetPositionY(), GorerotGroundPos.GetPositionZ(), gorerot->GetOrientation());
                        gorerot->SetImmuneToPC(false);
                        gorerot->SetReactState(REACT_AGGRESSIVE);
                        if (Player* target = gorerot->SelectNearestPlayer(50.0f))
                            gorerot->AI()->AttackStart(target);
                    }
                    break;
                case EVENT_BATTLE_DARIUS_BARK:
                    if (Creature* gorerot = GetActor(NPC_GOREROT, _gorerotGUID))
                    {
                        if (gorerot->IsAlive())
                        {
                            if (Creature* darius = GetActor(NPC_DARIUS_BATTLE, _dariusGUID))
                                darius->AI()->Talk(SAY_DARIUS_CATAPULTS);
                            _events.Repeat(27s);
                        }
                    }
                    break;
                case EVENT_BATTLE_MARCH_C:
                    me->GetMotionMaster()->MoveSmoothPath(POINT_LIAM_BARRICADE, LiamPathBarricade, std::size(LiamPathBarricade), false, false, 8.0f);
                    break;
                case EVENT_BATTLE_DARIUS_JOIN:
                    _events.CancelEvent(EVENT_BATTLE_DARIUS_BARK);
                    if (Creature* darius = GetActor(NPC_DARIUS_BATTLE, _dariusGUID))
                        darius->AI()->Talk(SAY_DARIUS_JOIN_KING);
                    break;
                case EVENT_BATTLE_MARCH_D:
                    me->GetMotionMaster()->MoveSmoothPath(POINT_LIAM_COURT, LiamPathCourt, std::size(LiamPathCourt), false, false, 8.0f);
                    break;
                case EVENT_BATTLE_BARK_PREVAIL:
                    Talk(SAY_LIAM_PREVAIL);
                    break;
                case EVENT_BATTLE_BARK_TIME_UP:
                    Talk(SAY_LIAM_TIME_IS_UP);
                    break;
                case EVENT_BATTLE_GENN_RUN:
                    if (Creature* genn = GetActor(NPC_GENN_COURT, _gennGUID))
                    {
                        genn->AI()->Talk(SAY_GENN_BLOCK_RETREAT);
                        genn->GetMotionMaster()->MoveSmoothPath(0, GennRunInPath, std::size(GennRunInPath), false, false, 8.0f);
                    }
                    GetActor(NPC_SYLVANAS_COURT, _sylvanasGUID);
                    GetActor(NPC_LIAM_DYING, _dyingLiamGUID);
                    break;
                case EVENT_BATTLE_GENN_JUMP:
                    if (Creature* genn = GetActor(NPC_GENN_COURT, _gennGUID))
                    {
                        if (sSpellMgr->GetSpellInfo(SPELL_GREYMANE_TRANSFORM_FX))
                            genn->CastSpell(genn, SPELL_GREYMANE_TRANSFORM_FX, true);
                        genn->GetMotionMaster()->MoveJumpWithGravity(GennFightAnchorPos, 20.0f, 19.291f);
                    }
                    break;
                case EVENT_BATTLE_GENN_YELL:
                    if (Creature* genn = GetActor(NPC_GENN_COURT, _gennGUID))
                        genn->AI()->Talk(SAY_GENN_SYLVANAS);
                    break;
                case EVENT_BATTLE_COURT_FIGHT:
                    StartCourtFight();
                    break;
                case EVENT_BATTLE_ENOUGH_CHECK:
                    if (!_enoughStarted)
                    {
                        Creature* sylvanas = GetActor(NPC_SYLVANAS_COURT, _sylvanasGUID);
                        if (sylvanas && sylvanas->IsAlive() && sylvanas->HealthBelowPct(40))
                            StartEnoughScene();
                        else
                            _events.Repeat(2s);
                    }
                    break;
                case EVENT_BATTLE_ENOUGH_FALLBACK:
                    if (!_enoughStarted)
                        StartEnoughScene();
                    break;
                case EVENT_BATTLE_FINALE_STEP:
                    HandleFinaleStep();
                    break;
                case EVENT_BATTLE_REARM:
                    _running = false;
                    break;
                default:
                    break;
            }
        }

        if (UpdateVictim())
            DoMeleeAttackIfReady();
    }

private:
    void TryStart()
    {
        if (_running)
            return;
        StartEvent();
    }

    void StartEvent()
    {
        _running = true;
        _enoughStarted = false;
        _finaleStep = 0;
        _gennGUID.Clear();
        _sylvanasGUID.Clear();
        _dyingLiamGUID.Clear();
        _lornaGUID.Clear();
        _dariusGUID.Clear();
        _gorerotGUID.Clear();

        me->setActive(true);
        me->AttackStop();
        me->CombatStop(true);
        me->SetReactState(REACT_PASSIVE);
        me->SetImmuneToNPC(true);
        _events.Reset();

        // Gorerot waits on his perch until the scripted leap
        if (Creature* gorerot = GetActor(NPC_GOREROT, _gorerotGUID))
        {
            gorerot->SetReactState(REACT_PASSIVE);
            gorerot->SetImmuneToPC(true);
        }

        // Stage timeline, offsets from t0 = event start (sniff run 2, 15:50:46).
        _events.ScheduleEvent(EVENT_BATTLE_SPEECH_0, 10800ms);
        _events.ScheduleEvent(EVENT_BATTLE_SPEECH_1, 18900ms);
        _events.ScheduleEvent(EVENT_BATTLE_SPEECH_2, 30300ms);
        _events.ScheduleEvent(EVENT_BATTLE_SPEECH_3, 41600ms);
        _events.ScheduleEvent(EVENT_BATTLE_SPEECH_4, 52900ms);
        _events.ScheduleEvent(EVENT_BATTLE_SPEECH_5, 64300ms);
        _events.ScheduleEvent(EVENT_BATTLE_SPEECH_6, 67500ms);
        _events.ScheduleEvent(EVENT_BATTLE_BARK_PUSH, 86900ms);
        _events.ScheduleEvent(EVENT_BATTLE_MARCH_B, 130500ms);
        _events.ScheduleEvent(EVENT_BATTLE_BARK_PUSH, 167900ms);
        _events.ScheduleEvent(EVENT_BATTLE_ABOM_LIAM, 176000ms);
        _events.ScheduleEvent(EVENT_BATTLE_ABOM_LORNA, 185800ms);
        _events.ScheduleEvent(EVENT_BATTLE_ABOM_LIAM_2, 197100ms);
        _events.ScheduleEvent(EVENT_BATTLE_ABOM_WAVE, 197700ms);
        _events.ScheduleEvent(EVENT_BATTLE_PRESS_ON, 214900ms);
        _events.ScheduleEvent(EVENT_BATTLE_BARK_ATTACK, 269800ms);
        _events.ScheduleEvent(EVENT_BATTLE_GOREROT_YELL, 286400ms);
        _events.ScheduleEvent(EVENT_BATTLE_GOREROT_LEAP, 289100ms);
        _events.ScheduleEvent(EVENT_BATTLE_DARIUS_BARK, 301000ms);
        _events.ScheduleEvent(EVENT_BATTLE_MARCH_C, 339300ms);
        _events.ScheduleEvent(EVENT_BATTLE_DARIUS_JOIN, 424900ms);
        _events.ScheduleEvent(EVENT_BATTLE_MARCH_D, 429900ms);
        _events.ScheduleEvent(EVENT_BATTLE_BARK_PREVAIL, 444700ms);
        _events.ScheduleEvent(EVENT_BATTLE_BARK_TIME_UP, 460000ms);
        _events.ScheduleEvent(EVENT_BATTLE_BARK_ATTACK, 480100ms);
        _events.ScheduleEvent(EVENT_BATTLE_GENN_RUN, 488400ms);
        _events.ScheduleEvent(EVENT_BATTLE_GENN_JUMP, 496400ms);
        _events.ScheduleEvent(EVENT_BATTLE_GENN_YELL, 496600ms);
        _events.ScheduleEvent(EVENT_BATTLE_COURT_FIGHT, 498800ms);
        _events.ScheduleEvent(EVENT_BATTLE_ENOUGH_FALLBACK, 581600ms);
    }

    Creature* GetActor(uint32 entry, ObjectGuid& cacheGUID, float radius = 250.0f)
    {
        if (!cacheGUID.IsEmpty())
            if (Creature* creature = ObjectAccessor::GetCreature(*me, cacheGUID))
                return creature;

        if (Creature* creature = me->FindNearestCreature(entry, radius))
        {
            cacheGUID = creature->GetGUID();
            return creature;
        }
        return nullptr;
    }

    void ActivateAbominationWave()
    {
        std::list<Creature*> abominations;
        me->GetCreatureListWithEntryInGrid(abominations, NPC_VILE_ABOMINATION, 300.0f);
        for (Creature* abomination : abominations)
        {
            if (!abomination->IsAlive() || abomination->IsInCombat())
                continue;

            for (Position const& wavePos : AbominationWavePos)
            {
                if (abomination->GetExactDist2d(wavePos.GetPositionX(), wavePos.GetPositionY()) < 8.0f)
                {
                    abomination->GetMotionMaster()->MovePoint(0, AbominationWaveDest, true);
                    break;
                }
            }
        }
    }

    void StartCourtFight()
    {
        Creature* genn = GetActor(NPC_GENN_COURT, _gennGUID);
        Creature* sylvanas = GetActor(NPC_SYLVANAS_COURT, _sylvanasGUID);
        if (genn && sylvanas && genn->IsAlive() && sylvanas->IsAlive())
        {
            genn->AI()->AttackStart(sylvanas);
            sylvanas->AI()->AttackStart(genn);

            std::list<Creature*> banshees;
            sylvanas->GetCreatureListWithEntryInGrid(banshees, NPC_SOULTETHERED_BANSHEE, 40.0f);
            for (Creature* banshee : banshees)
                if (banshee->IsAlive() && !banshee->IsInCombat())
                    banshee->AI()->AttackStart(genn);
        }
        _events.ScheduleEvent(EVENT_BATTLE_ENOUGH_CHECK, 2s);
    }

    void StartEnoughScene()
    {
        _enoughStarted = true;
        _events.CancelEvent(EVENT_BATTLE_ENOUGH_CHECK);
        _events.CancelEvent(EVENT_BATTLE_ENOUGH_FALLBACK);
        _finaleStep = 0;
        _events.ScheduleEvent(EVENT_BATTLE_FINALE_STEP, 1ms);
    }

    void HandleFinaleStep()
    {
        Creature* genn = GetActor(NPC_GENN_COURT, _gennGUID);
        Creature* sylvanas = GetActor(NPC_SYLVANAS_COURT, _sylvanasGUID);
        Creature* dyingLiam = GetActor(NPC_LIAM_DYING, _dyingLiamGUID);

        switch (_finaleStep)
        {
            case 0: // "Enough!" - the scream ends the fight
                if (sylvanas)
                {
                    sylvanas->AI()->Talk(SAY_SYLVANAS_ENOUGH);
                    sylvanas->AttackStop();
                    sylvanas->CombatStop(true);
                    sylvanas->SetReactState(REACT_PASSIVE);
                    if (sSpellMgr->GetSpellInfo(SPELL_BANSHEE_QUEENS_WAIL))
                        sylvanas->CastSpell(sylvanas, SPELL_BANSHEE_QUEENS_WAIL, true);
                    ClearCourtHostiles(sylvanas);
                }
                if (genn)
                {
                    genn->AttackStop();
                    genn->CombatStop(true);
                    genn->GetMotionMaster()->MoveJump(GennKnockbackPos, 20.0f, 12.0f);
                }
                break;
            case 1: // dying Liam breaks from the crowd
                if (dyingLiam)
                {
                    dyingLiam->SetWalk(false);
                    dyingLiam->GetMotionMaster()->MovePoint(0, DyingLiamRunPos, true);
                }
                break;
            case 2: // Sylvanas draws her bow at Genn
                if (sylvanas)
                {
                    sylvanas->AI()->Talk(SAY_SYLVANAS_STUBBORN_LEADER);
                    if (genn && sSpellMgr->GetSpellInfo(SPELL_AIMED_SHOT_SCENE))
                        sylvanas->CastSpell(genn, SPELL_AIMED_SHOT_SCENE, false);
                }
                break;
            case 3: // "FATHER!!!" - Liam interposes
                if (dyingLiam)
                {
                    dyingLiam->AI()->Talk(SAY_DYING_LIAM_FATHER);
                    dyingLiam->GetMotionMaster()->MoveSmoothPath(0, DyingLiamInterposePath, std::size(DyingLiamInterposePath), false, false, 8.0f);
                }
                break;
            case 4: // the arrow: scene dummies + quest credit to every eligible player
                if (sylvanas)
                {
                    sylvanas->InterruptNonMeleeSpells(true);
                    if (dyingLiam)
                    {
                        if (sSpellMgr->GetSpellInfo(SPELL_LIAM_SLAIN_DUMMY))
                            sylvanas->CastSpell(dyingLiam, SPELL_LIAM_SLAIN_DUMMY, true);
                        if (sSpellMgr->GetSpellInfo(SPELL_LIAM_SYLVANAS_DUMMY))
                            dyingLiam->CastSpell(dyingLiam, SPELL_LIAM_SYLVANAS_DUMMY, true);
                    }
                }
                GrantBattleCredit(sylvanas);
                break;
            case 5:
                if (sylvanas)
                    sylvanas->AI()->Talk(SAY_SYLVANAS_SUCH_A_WASTE);
                if (genn)
                    genn->AI()->Talk(SAY_GENN_LIAM_NO);
                break;
            case 6:
                if (dyingLiam)
                    dyingLiam->SetStandState(UNIT_STAND_STATE_DEAD);
                if (genn)
                {
                    genn->SetWalk(true);
                    genn->GetMotionMaster()->MovePoint(0, GennGriefPos, true);
                }
                break;
            case 7: // Sylvanas flees
                if (sylvanas)
                {
                    sylvanas->SetWalk(false);
                    sylvanas->GetMotionMaster()->MoveSmoothPath(0, SylvanasFleePath, std::size(SylvanasFleePath), false, false, 8.0f);
                }
                break;
            case 8:
                if (dyingLiam)
                    dyingLiam->AI()->Talk(SAY_DYING_LIAM_WE_DID_IT);
                break;
            case 9:
                if (dyingLiam)
                    dyingLiam->AI()->Talk(SAY_DYING_LIAM_TOOK_BACK);
                break;
            case 10: // static Sylvanas leaves; the spawn system brings her back
                if (sylvanas)
                    sylvanas->DespawnOrUnsummon(0ms, Seconds(60));
                break;
            case 11:
                ResetActors();
                return;
            default:
                return;
        }

        _events.ScheduleEvent(EVENT_BATTLE_FINALE_STEP, Milliseconds(BattleFinaleStepDelays[_finaleStep]));
        ++_finaleStep;
    }

    void ClearCourtHostiles(Creature* center)
    {
        // Retail 72384 "Despawn Forsaken" clears leftover hostiles at scene start.
        std::list<Creature*> hostiles;
        center->GetCreatureListWithEntryInGrid(hostiles, NPC_FORSAKEN_CROSSBOWMAN, 50.0f);
        center->GetCreatureListWithEntryInGrid(hostiles, NPC_FORSAKEN_INFANTRY, 50.0f);
        center->GetCreatureListWithEntryInGrid(hostiles, NPC_SOULTETHERED_BANSHEE, 50.0f);
        for (Creature* hostile : hostiles)
            if (hostile->IsAlive())
                hostile->DespawnOrUnsummon(0ms, Seconds(120));
    }

    void GrantBattleCredit(Creature* sceneCenter)
    {
        WorldObject* center = sceneCenter ? static_cast<WorldObject*>(sceneCenter) : static_cast<WorldObject*>(me);
        float radius = sceneCenter ? 100.0f : 200.0f;

        std::list<Player*> players;
        center->GetPlayerListInGrid(players, radius);
        for (Player* player : players)
            if (player->GetQuestStatus(QUEST_BATTLE_FOR_GILNEAS_CITY) == QUEST_STATUS_INCOMPLETE)
                player->KilledMonsterCredit(NPC_BFGC_KILL_CREDIT);
    }

    void ResetActors()
    {
        if (Creature* genn = GetActor(NPC_GENN_COURT, _gennGUID))
        {
            genn->RemoveAurasDueToSpell(SPELL_GREYMANE_TRANSFORM_FX);
            genn->SetWalk(false);
            genn->GetMotionMaster()->Clear();
            genn->NearTeleportTo(genn->GetHomePosition());
        }
        if (Creature* dyingLiam = GetActor(NPC_LIAM_DYING, _dyingLiamGUID))
        {
            dyingLiam->SetStandState(UNIT_STAND_STATE_STAND);
            dyingLiam->GetMotionMaster()->Clear();
            dyingLiam->NearTeleportTo(dyingLiam->GetHomePosition());
        }
        if (Creature* gorerot = GetActor(NPC_GOREROT, _gorerotGUID))
        {
            if (gorerot->IsAlive())
            {
                gorerot->AttackStop();
                gorerot->CombatStop(true);
                gorerot->SetHomePosition(GorerotPerchPos);
                gorerot->NearTeleportTo(GorerotPerchPos);
                gorerot->SetFullHealth();
                gorerot->SetReactState(REACT_PASSIVE);
                gorerot->SetImmuneToPC(true);
            }
        }

        me->GetMotionMaster()->Clear();
        me->NearTeleportTo(me->GetHomePosition());
        me->SetReactState(REACT_AGGRESSIVE);
        me->SetImmuneToNPC(false);
        me->setActive(false);

        _events.Reset();
        _events.ScheduleEvent(EVENT_BATTLE_REARM, 60s); // cooldown, then re-armable
    }

    EventMap _events;
    bool _running;
    bool _enoughStarted;
    uint8 _finaleStep;
    uint32 _armTimer;
    ObjectGuid _gennGUID;
    ObjectGuid _sylvanasGUID;
    ObjectGuid _dyingLiamGUID;
    ObjectGuid _lornaGUID;
    ObjectGuid _dariusGUID;
    ObjectGuid _gorerotGUID;
};

/*######
## Quest 24902 - The Hunt For Sylvanas
######*/

enum HuntForSylvanas
{
    NPC_TOBIAS_HUNT                 = 38507,
    NPC_WARHOWL                     = 38533,
    NPC_SYLVANAS_HUNT               = 38530,
    NPC_CRENSHAW                    = 38537,
    NPC_FORSAKEN_GENERAL            = 38617,

    SAY_TOBIAS_LETS_GO              = 0,
    SAY_TOBIAS_FOLLOW               = 1, // raid boss whisper
    SAY_TOBIAS_NOT_BE_SEEN          = 2,
    SAY_TOBIAS_HURRY                = 3,
    SAY_TOBIAS_HIDE                 = 4,
    SAY_GENERAL_READY               = 0,
    SAY_WARHOWL_LOSING_CONTROL      = 0,
    SAY_WARHOWL_CONFIDENT           = 1,
    SAY_WARHOWL_REPORT              = 2,
    SAY_SYLVANAS_SETBACK            = 0,
    SAY_SYLVANAS_WATCH_TONE         = 1,
    SAY_SYLVANAS_GO_WITH_HONOR      = 2,
    SAY_SYLVANAS_PLAGUE             = 3,
    SAY_CRENSHAW_MY_LADY            = 0,
    SAY_CRENSHAW_AS_YOU_WISH        = 1,

    POINT_TOBIAS_CANAL              = 100,
    TOBIAS_HOLD_GENERAL_INDEX       = 8
};

enum HuntEvents
{
    EVENT_TOBIAS_TALK_LETS_GO       = 1,
    EVENT_TOBIAS_TALK_FOLLOW,
    EVENT_TOBIAS_START_ESCORT,
    EVENT_TOBIAS_TALK_NOT_BE_SEEN,
    EVENT_TOBIAS_GENERAL_BEAT,
    EVENT_TOBIAS_TALK_HURRY,
    EVENT_TOBIAS_RESUME_ESCORT,
    EVENT_SCENE_FACING,
    EVENT_SCENE_WARHOWL_0,
    EVENT_SCENE_SYLVANAS_0,
    EVENT_SCENE_WARHOWL_1,
    EVENT_SCENE_SYLVANAS_1,
    EVENT_SCENE_WARHOWL_2,
    EVENT_SCENE_WARHOWL_EXIT,
    EVENT_SCENE_SYLVANAS_2,
    EVENT_SCENE_CRENSHAW_STEP,
    EVENT_SCENE_CRENSHAW_0,
    EVENT_SCENE_PLAGUE_REVEAL,
    EVENT_SCENE_SYLVANAS_3,
    EVENT_SCENE_CRENSHAW_1,
    EVENT_SCENE_CRENSHAW_EXIT,
    EVENT_TOBIAS_FINISH
};

Position const TobiasSummonPos = { -1656.6016f, 1602.8499f, 23.5201f, 4.19905f };

// Tobias escort path - SMSG_ON_MONSTER_MOVE Entry 38507 Low 4224847 dest points
Position const TobiasPath[] =
{
    { -1648.38f, 1619.91f, 20.49f },   // 19555948  16:01:24.522 leave courtyard
    { -1627.25f, 1612.77f, 21.70f },   // 19556172  16:01:25.263
    { -1589.10f, 1606.64f, 21.60f },   // 19558266  16:01:35.031
    { -1578.49f, 1628.04f, 20.61f },   // 19561227  16:01:43.955 north jog into market street
    { -1557.69f, 1622.51f, 20.73f },   // 19561512  16:01:45.159
    { -1541.38f, 1615.75f, 20.49f },   // 19562348  16:01:47.924
    { -1523.84f, 1608.76f, 20.49f },   // 19562605  16:01:49.192
    { -1497.43f, 1579.61f, 20.49f },   // 19562687  16:01:49.603
    { -1490.69f, 1576.04f, 20.49f },   // 19570444  16:01:54.458 -> HOLD 1 (Forsaken General)
    { -1525.43f, 1582.96f, 26.67f },   // 19577403  16:02:14.699 up the stairs
    { -1546.72f, 1569.55f, 29.28f },   // 19578785  16:02:17.878
    { -1553.04f, 1567.18f, 29.28f },   // 19579844  16:02:21.107
    { -1561.21f, 1566.29f, 29.28f },   // 19580171  16:02:21.896
    { -1564.86f, 1562.23f, 29.28f },   // 19580624  16:02:22.787
    { -1566.11f, 1552.46f, 29.28f },   // 19581011  16:02:23.602 (passes the trio anchors)
    { -1571.51f, 1546.02f, 29.28f },   // 19581872  16:02:24.407
    { -1586.55f, 1530.87f, 29.28f },   // 19582216  16:02:25.201
    { -1591.10f, 1528.72f, 29.28f },   // 19583183  16:02:28.470
    { -1603.38f, 1532.59f, 29.30f },   // 19583370  16:02:29.265
    { -1609.75f, 1535.88f, 29.30f },   // 19583544  16:02:30.079 -> jump-off point
};
Position const TobiasCanalHidePos = { -1617.5781f, 1531.6406f, 26.2338f, 1.5708f }; // 72204 jump, gravity 25.899

// Cathedral scene (all timings relative to Tobias' canal jump; trio create +0.11s,
// first Warhowl line +21.6s = scene t0 16:02:55.277)
Position const WarhowlSummonPos  = { -1565.2986f, 1556.5348f, 29.2847f, 5.5676f };
Position const SylvanasSummonPos = { -1566.7952f, 1555.3004f, 29.2847f, 0.8203f };
Position const CrenshawSummonPos = { -1566.7952f, 1555.3004f, 29.2847f, 0.8203f };

Position const SylvanasScenePos  = { -1603.87f, 1522.16f, 29.30f, 0.8029f };
Position const CrenshawStepPos   = { -1596.32f, 1521.71f, 29.28f };

Position const WarhowlEntry[]  = { { -1565.94f, 1549.27f, 29.28f }, { -1576.63f, 1538.81f, 29.28f }, { -1590.64f, 1524.89f, 29.28f }, { -1594.79f, 1526.28f, 29.28f } };
Position const CrenshawEntry[] = { { -1572.23f, 1544.61f, 29.28f }, { -1580.39f, 1536.36f, 29.28f }, { -1591.29f, 1521.84f, 29.28f }, { -1592.88f, 1521.67f, 29.28f } };
Position const SylvanasEntry[] = { { -1569.81f, 1547.95f, 29.28f }, { -1585.86f, 1535.68f, 29.28f }, { -1594.16f, 1529.59f, 29.28f }, { -1603.87f, 1522.16f, 29.30f } };

Position const WarhowlExit[]  = { { -1585.78f, 1531.20f, 29.28f }, { -1574.19f, 1542.31f, 29.28f }, { -1564.65f, 1552.24f, 29.28f }, { -1566.94f, 1559.93f, 29.28f } };
Position const SylvanasExit[] = { { -1596.27f, 1528.71f, 29.28f }, { -1589.17f, 1532.20f, 29.28f }, { -1575.83f, 1542.59f, 29.28f }, { -1565.80f, 1555.97f, 29.28f } };
Position const CrenshawExit[] = { { -1589.43f, 1526.78f, 29.28f }, { -1580.59f, 1536.14f, 29.28f }, { -1572.10f, 1544.97f, 29.28f } };

// 72470 - Summon Tobias Mistmantle: pin the summon destination to the retail
// courtyard spot (the DBC dest offset would scatter him up to 50 yd away).
class spell_gilneas_summon_tobias : public SpellScript
{
    void SetDest(SpellDestination& dest)
    {
        dest.Relocate(TobiasSummonPos);
    }

    void Register() override
    {
        OnDestinationTargetSelect.Register(&spell_gilneas_summon_tobias::SetDest, EFFECT_0, TARGET_DEST_DEST_BACK);
    }
};

struct npc_gilneas_tobias_hunt : public ScriptedAI
{
    npc_gilneas_tobias_hunt(Creature* creature) : ScriptedAI(creature)
    {
        Initialize();
    }

    void Initialize()
    {
        _pointIndex = 0;
        _escortStarted = false;
        _holding = false;
        _sceneStarted = false;
        _watchdog = 500;
    }

    void IsSummonedBy(Unit* summoner) override
    {
        Initialize();
        me->SetReactState(REACT_PASSIVE);

        // The escort is owner-paced; 72470's native 180s duration is too short for
        // slow players. The script owns every despawn path itself.
        if (TempSummon* summon = me->ToTempSummon())
            summon->SetTempSummonType(TEMPSUMMON_MANUAL_DESPAWN);

        Player* owner = summoner ? summoner->ToPlayer() : nullptr;
        if (!owner)
        {
            std::list<Player*> players;
            me->GetPlayerListInGrid(players, 30.0f);
            for (Player* candidate : players)
            {
                if (candidate->GetQuestStatus(QUEST_THE_HUNT_FOR_SYLVANAS) == QUEST_STATUS_INCOMPLETE)
                {
                    owner = candidate;
                    break;
                }
            }
        }
        if (owner)
            _ownerGUID = owner->GetGUID();

        _events.ScheduleEvent(EVENT_TOBIAS_TALK_LETS_GO, 100ms);
        _events.ScheduleEvent(EVENT_TOBIAS_TALK_FOLLOW, 3200ms);
        _events.ScheduleEvent(EVENT_TOBIAS_START_ESCORT, 4700ms);
        _events.ScheduleEvent(EVENT_TOBIAS_TALK_NOT_BE_SEEN, 12100ms);
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type == EFFECT_MOTION_TYPE && pointId == POINT_TOBIAS_CANAL)
        {
            me->SetFacingTo(TobiasCanalHidePos.GetOrientation());
            return;
        }

        if (type != POINT_MOTION_TYPE || !_escortStarted || _sceneStarted)
            return;

        if (pointId != _pointIndex)
            return;

        if (_pointIndex == TOBIAS_HOLD_GENERAL_INDEX)
        {
            // HOLD 1: the Forsaken General beat, then resume up the stairs.
            _holding = true;
            _events.ScheduleEvent(EVENT_TOBIAS_GENERAL_BEAT, 2300ms);
            _events.ScheduleEvent(EVENT_TOBIAS_TALK_HURRY, 12900ms);
            _events.ScheduleEvent(EVENT_TOBIAS_RESUME_ESCORT, 17s);
        }
        else if (_pointIndex == std::size(TobiasPath) - 1)
            StartCathedralScene();

        ++_pointIndex;
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_TOBIAS_TALK_LETS_GO:
                    Talk(SAY_TOBIAS_LETS_GO);
                    break;
                case EVENT_TOBIAS_TALK_FOLLOW:
                    Talk(SAY_TOBIAS_FOLLOW, ObjectAccessor::GetPlayer(*me, _ownerGUID));
                    break;
                case EVENT_TOBIAS_START_ESCORT:
                    _escortStarted = true;
                    break;
                case EVENT_TOBIAS_TALK_NOT_BE_SEEN:
                    Talk(SAY_TOBIAS_NOT_BE_SEEN);
                    break;
                case EVENT_TOBIAS_GENERAL_BEAT:
                    if (Creature* general = me->FindNearestCreature(NPC_FORSAKEN_GENERAL, 40.0f))
                    {
                        general->AI()->Talk(SAY_GENERAL_READY);
                        general->HandleEmoteCommand(EMOTE_ONESHOT_TALK);
                    }
                    break;
                case EVENT_TOBIAS_TALK_HURRY:
                    Talk(SAY_TOBIAS_HURRY);
                    break;
                case EVENT_TOBIAS_RESUME_ESCORT:
                    _holding = false;
                    break;
                case EVENT_SCENE_FACING:
                    if (Creature* sylvanas = GetSceneActor(_sylvanasGUID))
                    {
                        sylvanas->SetFacingTo(SylvanasScenePos.GetOrientation());
                        if (Creature* warhowl = GetSceneActor(_warhowlGUID))
                            warhowl->SetFacingToObject(sylvanas);
                        if (Creature* crenshaw = GetSceneActor(_crenshawGUID))
                            crenshaw->SetFacingToObject(sylvanas);
                    }
                    break;
                case EVENT_SCENE_WARHOWL_0:
                    if (Creature* warhowl = GetSceneActor(_warhowlGUID))
                        warhowl->AI()->Talk(SAY_WARHOWL_LOSING_CONTROL);
                    break;
                case EVENT_SCENE_SYLVANAS_0:
                    if (Creature* sylvanas = GetSceneActor(_sylvanasGUID))
                        sylvanas->AI()->Talk(SAY_SYLVANAS_SETBACK);
                    break;
                case EVENT_SCENE_WARHOWL_1:
                    if (Creature* warhowl = GetSceneActor(_warhowlGUID))
                        warhowl->AI()->Talk(SAY_WARHOWL_CONFIDENT);
                    break;
                case EVENT_SCENE_SYLVANAS_1:
                    if (Creature* sylvanas = GetSceneActor(_sylvanasGUID))
                        sylvanas->AI()->Talk(SAY_SYLVANAS_WATCH_TONE);
                    break;
                case EVENT_SCENE_WARHOWL_2:
                    if (Creature* warhowl = GetSceneActor(_warhowlGUID))
                        warhowl->AI()->Talk(SAY_WARHOWL_REPORT);
                    break;
                case EVENT_SCENE_WARHOWL_EXIT:
                    if (Creature* warhowl = GetSceneActor(_warhowlGUID))
                        warhowl->GetMotionMaster()->MoveSmoothPath(0, WarhowlExit, std::size(WarhowlExit), true, false);
                    break;
                case EVENT_SCENE_SYLVANAS_2:
                    if (Creature* sylvanas = GetSceneActor(_sylvanasGUID))
                        sylvanas->AI()->Talk(SAY_SYLVANAS_GO_WITH_HONOR);
                    break;
                case EVENT_SCENE_CRENSHAW_STEP:
                    if (Creature* crenshaw = GetSceneActor(_crenshawGUID))
                        crenshaw->GetMotionMaster()->MovePoint(0, CrenshawStepPos, true);
                    break;
                case EVENT_SCENE_CRENSHAW_0:
                    if (Creature* crenshaw = GetSceneActor(_crenshawGUID))
                        crenshaw->AI()->Talk(SAY_CRENSHAW_MY_LADY);
                    break;
                case EVENT_SCENE_PLAGUE_REVEAL:
                    // The plague reveal completes the quest (SpecialFlags=2 event shape).
                    if (Player* owner = ObjectAccessor::GetPlayer(*me, _ownerGUID))
                        owner->AreaExploredOrEventHappens(QUEST_THE_HUNT_FOR_SYLVANAS);
                    if (Creature* sylvanas = GetSceneActor(_sylvanasGUID))
                        sylvanas->GetMotionMaster()->MoveSmoothPath(0, SylvanasExit, std::size(SylvanasExit), true, false);
                    break;
                case EVENT_SCENE_SYLVANAS_3:
                    if (Creature* sylvanas = GetSceneActor(_sylvanasGUID))
                        sylvanas->AI()->Talk(SAY_SYLVANAS_PLAGUE);
                    break;
                case EVENT_SCENE_CRENSHAW_1:
                    if (Creature* crenshaw = GetSceneActor(_crenshawGUID))
                        crenshaw->AI()->Talk(SAY_CRENSHAW_AS_YOU_WISH);
                    break;
                case EVENT_SCENE_CRENSHAW_EXIT:
                    if (Creature* crenshaw = GetSceneActor(_crenshawGUID))
                        crenshaw->GetMotionMaster()->MoveSmoothPath(0, CrenshawExit, std::size(CrenshawExit), true, false);
                    break;
                case EVENT_TOBIAS_FINISH:
                    me->DespawnOrUnsummon();
                    return;
                default:
                    break;
            }
        }

        if (_watchdog <= diff)
        {
            _watchdog = 500;

            Player* owner = ObjectAccessor::GetPlayer(*me, _ownerGUID);
            if (!owner)
            {
                me->DespawnOrUnsummon();
                return;
            }

            // Owner-proximity gated escort: only advance while the owner keeps up.
            if (_escortStarted && !_holding && !_sceneStarted && !me->isMoving() &&
                _pointIndex < std::size(TobiasPath) && me->IsWithinDist(owner, 50.0f))
                me->GetMotionMaster()->MovePoint(_pointIndex, TobiasPath[_pointIndex], true);
        }
        else
            _watchdog -= diff;
    }

private:
    Creature* GetSceneActor(ObjectGuid const& guid)
    {
        return ObjectAccessor::GetCreature(*me, guid);
    }

    TempSummon* SummonSceneActor(Player* owner, uint32 entry, Position const& pos, Milliseconds duration)
    {
        TempSummon* summon = owner->SummonCreature(entry, pos, TEMPSUMMON_TIMED_DESPAWN, uint32(duration.count()), 0, owner->GetGUID());
        if (summon)
        {
            summon->SetReactState(REACT_PASSIVE);
            summon->SetImmuneToAll(true);
            summon->SetWalk(true);
        }
        return summon;
    }

    void StartCathedralScene()
    {
        _sceneStarted = true;

        me->GetMotionMaster()->MoveJumpWithGravity(TobiasCanalHidePos, 25.0f, 25.899f, POINT_TOBIAS_CANAL);
        Talk(SAY_TOBIAS_HIDE);

        Player* owner = ObjectAccessor::GetPlayer(*me, _ownerGUID);
        if (!owner)
        {
            me->DespawnOrUnsummon(5s);
            return;
        }

        // Owner-personal scene cast; despawn times match the retail destroys.
        if (TempSummon* warhowl = SummonSceneActor(owner, NPC_WARHOWL, WarhowlSummonPos, 92000ms))
        {
            _warhowlGUID = warhowl->GetGUID();
            warhowl->GetMotionMaster()->MoveSmoothPath(0, WarhowlEntry, std::size(WarhowlEntry), true, false);
        }
        if (TempSummon* sylvanas = SummonSceneActor(owner, NPC_SYLVANAS_HUNT, SylvanasSummonPos, 117000ms))
        {
            _sylvanasGUID = sylvanas->GetGUID();
            sylvanas->GetMotionMaster()->MoveSmoothPath(0, SylvanasEntry, std::size(SylvanasEntry), true, false);
        }
        if (TempSummon* crenshaw = SummonSceneActor(owner, NPC_CRENSHAW, CrenshawSummonPos, 120000ms))
        {
            _crenshawGUID = crenshaw->GetGUID();
            crenshaw->GetMotionMaster()->MoveSmoothPath(0, CrenshawEntry, std::size(CrenshawEntry), true, false);
        }

        // Dialogue offsets (sniff): trio create +21.6s = first line, then
        // 0/12.1/19.0/32.4/47.4/50.5/55.1/56.6/58.3/71.6/71.7/82.6/84.0.
        _events.ScheduleEvent(EVENT_SCENE_FACING, 24500ms);
        _events.ScheduleEvent(EVENT_SCENE_WARHOWL_0, 21600ms);
        _events.ScheduleEvent(EVENT_SCENE_SYLVANAS_0, 33700ms);
        _events.ScheduleEvent(EVENT_SCENE_WARHOWL_1, 40600ms);
        _events.ScheduleEvent(EVENT_SCENE_SYLVANAS_1, 54000ms);
        _events.ScheduleEvent(EVENT_SCENE_WARHOWL_2, 69000ms);
        _events.ScheduleEvent(EVENT_SCENE_WARHOWL_EXIT, 72100ms);
        _events.ScheduleEvent(EVENT_SCENE_SYLVANAS_2, 76700ms);
        _events.ScheduleEvent(EVENT_SCENE_CRENSHAW_STEP, 78200ms);
        _events.ScheduleEvent(EVENT_SCENE_CRENSHAW_0, 79900ms);
        _events.ScheduleEvent(EVENT_SCENE_PLAGUE_REVEAL, 93200ms);
        _events.ScheduleEvent(EVENT_SCENE_SYLVANAS_3, 93300ms);
        _events.ScheduleEvent(EVENT_SCENE_CRENSHAW_1, 104200ms);
        _events.ScheduleEvent(EVENT_SCENE_CRENSHAW_EXIT, 105600ms);
        _events.ScheduleEvent(EVENT_TOBIAS_FINISH, 107s);
    }

    EventMap _events;
    ObjectGuid _ownerGUID;
    ObjectGuid _warhowlGUID;
    ObjectGuid _sylvanasGUID;
    ObjectGuid _crenshawGUID;
    uint32 _pointIndex;
    uint32 _watchdog;
    bool _escortStarted;
    bool _holding;
    bool _sceneStarted;
};

/*######
## Quest 24920 - Slowing the Inevitable (Captured Riding Bat)
######*/

// 38540 - summoned per-click by 72472 from the parked bat 38615; the bomb
// (72246) sits on the vehicle bar natively.
struct npc_gilneas_captured_riding_bat : public VehicleAI
{
    npc_gilneas_captured_riding_bat(Creature* creature) : VehicleAI(creature) { }

    void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
    {
        if (!passenger->IsPlayer())
            return;

        if (!apply)
            me->DespawnOrUnsummon(2s);
    }
};

/*######
## Quest 24678 - Knee-Deep (Half-Burnt Torch)
######*/

// 70631 - Swing Torch: turn-in-only quest on retail; the swing is pure flavor
// (its area dummy only brushed tunnel critters in the sniff). Lenient gate.
class spell_gilneas_half_burnt_torch : public SpellScript
{
    SpellCastResult CheckQuest()
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!player)
            return SPELL_CAST_OK;

        QuestStatus status = player->GetQuestStatus(QUEST_KNEE_DEEP);
        if (status == QUEST_STATUS_INCOMPLETE || status == QUEST_STATUS_COMPLETE)
            return SPELL_CAST_OK;

        return SPELL_FAILED_DONT_REPORT;
    }

    void Register() override
    {
        OnCheckCast.Register(&spell_gilneas_half_burnt_torch::CheckQuest);
    }
};

/*######
## Quest 24679 - Patriarch's Blessing + Aderic's Repose funeral
######*/

enum Funeral
{
    NPC_FUNERAL_GENN                = 50893,
    NPC_FUNERAL_LORNA               = 50881,
    NPC_FUNERAL_DARIUS              = 50902,
    NPC_FUNERAL_CAMERA              = 51083,

    SPELL_SUMMON_FUNERAL_CAMERA     = 94244, // BP 94245 = ride + transient phase 187

    SAY_FUNERAL_GENN_BLESS          = 0,
    SAY_FUNERAL_GENN_HEROES         = 1,
    SAY_FUNERAL_GENN_RETURN         = 2,
    SAY_FUNERAL_LORNA_TRUE_MAN      = 0,
    SAY_FUNERAL_DARIUS_COURAGE      = 0,

    POINT_FUNERAL_RIDE              = 1
};

enum FuneralEvents
{
    EVENT_FUNERAL_BEAT              = 1,
    EVENT_CAMERA_FLY,
    EVENT_CAMERA_EJECT
};

// Camera spawn/summon dest (94244 TargA 46 needs a script-supplied dest) - sniff line 20506704
Position const FuneralCameraSpawn = { -1644.0538f, 1904.0173f, 30.961607f, 3.054326f };

// Single retail camera spline - SMSG_ON_MONSTER_MOVE line 20509063 (16:11:00.263)
// Flags: Flying|CatmullRom|UncompressedPath, MoveTime 46368 ms, dist 28.98 yd, speed 0.625 yd/s
Position const FuneralCameraPath[] =
{
    { -1644.0538f, 1904.0173f, 30.961607f }, // start        t=0.0s
    { -1634.3403f, 1906.0747f, 31.936602f }, // node 1       t=16.0s
    { -1633.3108f, 1902.1945f, 33.546100f }, // node 2       t=22.9s
    { -1637.3646f, 1900.1580f, 33.097385f }, // node 3       t=30.2s
    { -1641.7709f, 1898.9166f, 33.097385f }, // node 4       t=37.5s
    { -1646.7135f, 1899.0348f, 30.598402f }, // node 5 (end) t=46.4s
};

// Retail teleports the rider out (SMSG_MOVE_TELEPORT line 20528644) = spell_target_position 94260 eff1
Position const FuneralExitPos = { -1724.18f, 1872.09f, 17.7994f, 3.228859f };

// 72853 - Place Blessed Offerings: GO spawn + kill credit 38147 are native in
// the item spell; the funeral camera chain (retail serverside) is bridged here.
class spell_gilneas_blessed_offerings : public SpellScript
{
    SpellCastResult CheckQuest()
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!player || player->GetQuestStatus(QUEST_PATRIARCHS_BLESSING) == QUEST_STATUS_INCOMPLETE)
            return SPELL_CAST_OK;

        return SPELL_FAILED_DONT_REPORT;
    }

    void HandleAfterCast()
    {
        if (Unit* caster = GetCaster())
            if (sSpellMgr->GetSpellInfo(SPELL_SUMMON_FUNERAL_CAMERA))
                caster->CastSpell(FuneralCameraSpawn, SPELL_SUMMON_FUNERAL_CAMERA, true);
    }

    void Register() override
    {
        OnCheckCast.Register(&spell_gilneas_blessed_offerings::CheckQuest);
        AfterCast.Register(&spell_gilneas_blessed_offerings::HandleAfterCast);
    }
};

// 94244 - Summon Funeral Camera: TARGET_DEST_NEARBY_ENTRY has no conditions
// anchor; pin the destination to the retail camera spawn.
class spell_gilneas_funeral_camera_summon : public SpellScript
{
    void SetDest(SpellDestination& dest)
    {
        dest.Relocate(FuneralCameraSpawn);
    }

    void Register() override
    {
        OnDestinationTargetSelect.Register(&spell_gilneas_funeral_camera_summon::SetDest, EFFECT_0, TARGET_DEST_NEARBY_ENTRY);
    }
};

// 51083 - Gilneas Funeral Camera: the 94245 ride aura carries the transient
// phase 187; ejecting drops it natively, no phase calls anywhere.
struct npc_gilneas_funeral_camera : public VehicleAI
{
    npc_gilneas_funeral_camera(Creature* creature) : VehicleAI(creature) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        me->SetCanFly(true);
        me->SetDisableGravity(true);
    }

    void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
    {
        if (!passenger->IsPlayer())
            return;

        if (apply)
        {
            _passengerGUID = passenger->GetGUID();
            _events.ScheduleEvent(EVENT_CAMERA_FLY, 1300ms);
        }
        else
            me->DespawnOrUnsummon(3s);
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != EFFECT_MOTION_TYPE && type != POINT_MOTION_TYPE)
            return;

        if (pointId == POINT_FUNERAL_RIDE)
            _events.ScheduleEvent(EVENT_CAMERA_EJECT, 2s);
    }

    void UpdateAI(uint32 diff) override
    {
        VehicleAI::UpdateAI(diff);
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_CAMERA_FLY:
                    me->GetMotionMaster()->MoveSmoothPath(POINT_FUNERAL_RIDE, FuneralCameraPath, std::size(FuneralCameraPath), false, true, 0.625f);
                    break;
                case EVENT_CAMERA_EJECT:
                    if (Player* player = ObjectAccessor::GetPlayer(*me, _passengerGUID))
                    {
                        player->ExitVehicle(); // 94245 drops -> phase 187 removed natively
                        player->NearTeleportTo(FuneralExitPos);
                    }
                    me->DespawnOrUnsummon(2s);
                    break;
                default:
                    break;
            }
        }
    }

private:
    EventMap _events;
    ObjectGuid _passengerGUID;
};

// 50893 - King Genn Greymane (funeral static): perpetual ambient scene loop.
// Beats 0 / 9.8 / 17.9 / 28.0 / 34.1 s, ~11 s pause -> 45 s loop; the one-shot
// emotes and sounds ride the creature_text rows.
struct npc_gilneas_funeral_controller : public ScriptedAI
{
    npc_gilneas_funeral_controller(Creature* creature) : ScriptedAI(creature), _step(0) { }

    void Reset() override
    {
        _step = 0;
        _events.Reset();
        _events.ScheduleEvent(EVENT_FUNERAL_BEAT, 5s);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            if (eventId != EVENT_FUNERAL_BEAT)
                continue;

            switch (_step)
            {
                case 0:
                    Talk(SAY_FUNERAL_GENN_BLESS);
                    _events.Repeat(9800ms);
                    break;
                case 1:
                    Talk(SAY_FUNERAL_GENN_HEROES);
                    _events.Repeat(8100ms);
                    break;
                case 2:
                    if (Creature* lorna = me->FindNearestCreature(NPC_FUNERAL_LORNA, 20.0f))
                        lorna->AI()->Talk(SAY_FUNERAL_LORNA_TRUE_MAN);
                    _events.Repeat(10100ms);
                    break;
                case 3:
                    if (Creature* darius = me->FindNearestCreature(NPC_FUNERAL_DARIUS, 20.0f))
                        darius->AI()->Talk(SAY_FUNERAL_DARIUS_COURAGE);
                    _events.Repeat(6100ms);
                    break;
                case 4:
                    Talk(SAY_FUNERAL_GENN_RETURN);
                    _events.Repeat(10900ms);
                    break;
                default:
                    break;
            }
            _step = (_step + 1) % 5;
        }
    }

private:
    EventMap _events;
    uint8 _step;
};

/*######
## Quest 24681 - They Have Allies, But So Do We (Glaive Thrower)
######*/

// 37927 - static harbor spawns with spellclick 68503; abandoned throwers cycle
// through the spawn system.
struct npc_gilneas_glaive_thrower : public VehicleAI
{
    npc_gilneas_glaive_thrower(Creature* creature) : VehicleAI(creature) { }

    void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
    {
        if (!passenger->IsPlayer())
            return;

        if (!apply)
        {
            if (me->IsSummon())
                me->DespawnOrUnsummon(2s);
            else
                me->DespawnOrUnsummon(Milliseconds(2000), Seconds(30));
        }
    }
};

/*######
## Quest 26706 - Endgame: hippogryph flight, gunship boarding, wyvern escape
######*/

enum Endgame
{
    NPC_ENDGAME_LORNA               = 43566,
    NPC_KORM_BONEGRIND              = 43567,
    NPC_GUNSHIP_GRUNT               = 42141,
    NPC_WORGEN_WARRIOR_SHIP         = 43651,
    NPC_GILNEAN_SHARPSHOOTER        = 43703,
    NPC_ENDGAME_CREDIT              = 43729,

    SPELL_SUMMON_WYVERN_RIDE        = 81779, // native: credit 43729 + summon 43713 + ride-back 95869
    SPELL_GUNSHIP_EXPLOSION_PLAYER  = 81927,
    SPELL_GUNSHIP_EXPLOSION_WYVERN  = 81928,

    // Lorna Crowley 43566 creature_text (groups 0-6 existing, 7 = new "Attack!")
    SAY_LORNA_RAFTERS               = 0,
    SAY_LORNA_RAPPEL                = 1,
    SAY_LORNA_HANDS_UP              = 2,
    SAY_LORNA_DOWNSTAIRS            = 3,
    SAY_LORNA_FURNACE               = 4,
    SAY_LORNA_BIG_ORC               = 5,
    SAY_LORNA_WYVERNS               = 6,
    SAY_LORNA_ATTACK                = 7,

    POINT_HIPPOGRYPH_DECK           = 1,
    POINT_HIPPOGRYPH_DEPART         = 2,
    POINT_WYVERN_LOOP               = 1,
    POINT_WYVERN_ESCAPE             = 2
};

enum EndgameEvents
{
    EVENT_HIPPOGRYPH_TAKEOFF        = 1,
    EVENT_HIPPOGRYPH_DESPAWN,

    EVENT_ENDGAME_ATTACK,
    EVENT_ENDGAME_RAFTERS,
    EVENT_ENDGAME_RAPPEL,
    EVENT_ENDGAME_DESCEND,
    EVENT_ENDGAME_HANDS_UP,
    EVENT_ENDGAME_DOWNSTAIRS,
    EVENT_ENDGAME_FURNACE,
    EVENT_ENDGAME_BIG_ORC,
    EVENT_ENDGAME_KORM_POLL,
    EVENT_ENDGAME_WYVERN_YELL,
    EVENT_ENDGAME_LAUNCH,
    EVENT_ENDGAME_RESET,

    EVENT_WYVERN_TAKEOFF
};

// Hippogryph flight (sniff 20828876, Flying|CatmullRom, MoveTime 52050 ms, speed 24.0)
// start { -1293.90f, 2129.48f, 6.68f } = Keel Harbor summon spot
Position const HippogryphPath[] =
{
    { -1303.32f, 2140.34f, 14.03f },   // +0.7s
    { -1267.07f, 2178.13f, 31.51f },   // +3.0s
    { -1227.10f, 2252.05f, 47.81f },   // +6.5s
    { -1150.97f, 2561.88f, 112.31f },  // +20.1s  (long climb over the bay)
    { -1465.18f, 3256.92f, 189.79f },  // +52.0s  (hover point above gunship deck)
};
Position const HippogryphEjectPos = { -1465.15f, 3256.89f, 191.68f, 2.3074f }; // drops ~13u onto deck
Position const HippogryphDepartPath[] =
{
    { -1525.01f, 3322.89f, 105.88f },  // +5.1s
    { -1566.10f, 3316.15f, 105.88f },  // +6.8s
};

// Wyvern loop RE-BASED to the frozen static ship (sniff 20954641, speed 16.0)
Position const WyvernLoopPath[] =
{
    { -1508.77f, 3168.55f, 113.94f },
    { -1528.32f, 3156.63f, 122.52f },
    { -1544.67f, 3169.84f, 123.02f },
    { -1586.64f, 3222.29f, 141.24f },
};
// Escape into Keel Harbor: bridge to the last two retail escape nodes, then the landing drop
Position const WyvernEscapePath[] =
{
    { -1315.28f, 2030.00f, 63.73f },   // retail escape node (+20.2s)
    { -1336.09f, 2097.85f, 14.60f },   // Keel Harbor overhead (+23.8s)
    { -1336.09f, 2097.85f, 5.63f },    // landing drop (sniff 20987834)
};
Position const WyvernEjectPos = { -1336.10f, 2097.88f, 13.87f, 1.8684f }; // sniff 20987994

// Rope-bottom marks on the mid ("rafters") level for the cosmetic squad descent
Position const SquadMidLevelPos[] =
{
    { -1454.60f, 3268.34f, 128.70f },
    { -1459.38f, 3265.42f, 128.49f },
    { -1483.42f, 3250.66f, 128.58f },
    { -1485.68f, 3251.26f, 128.67f },
};

struct npc_gilneas_endgame_hippogryph : public VehicleAI
{
    npc_gilneas_endgame_hippogryph(Creature* creature) : VehicleAI(creature), _arrived(false) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        me->SetCanFly(true);
        me->SetDisableGravity(true);
    }

    void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
    {
        if (!passenger->IsPlayer())
            return;

        if (apply)
        {
            _passengerGUID = passenger->GetGUID();
            _events.ScheduleEvent(EVENT_HIPPOGRYPH_TAKEOFF, 2s);
        }
        else if (!_arrived)
        {
            _events.Reset();
            me->DespawnOrUnsummon(2s);
        }
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != EFFECT_MOTION_TYPE && type != POINT_MOTION_TYPE)
            return;

        switch (pointId)
        {
            case POINT_HIPPOGRYPH_DECK:
                _arrived = true;
                if (Player* player = ObjectAccessor::GetPlayer(*me, _passengerGUID))
                {
                    player->ExitVehicle();
                    player->NearTeleportTo(HippogryphEjectPos); // ExitVehicle discards exit positions in this fork
                }
                me->GetMotionMaster()->MoveSmoothPath(POINT_HIPPOGRYPH_DEPART, HippogryphDepartPath, std::size(HippogryphDepartPath), false, true, 24.0f);
                _events.ScheduleEvent(EVENT_HIPPOGRYPH_DESPAWN, 10s);
                break;
            case POINT_HIPPOGRYPH_DEPART:
                me->DespawnOrUnsummon(1s);
                break;
            default:
                break;
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
                case EVENT_HIPPOGRYPH_TAKEOFF:
                    me->GetMotionMaster()->MoveSmoothPath(POINT_HIPPOGRYPH_DECK, HippogryphPath, std::size(HippogryphPath), false, true, 24.0f);
                    break;
                case EVENT_HIPPOGRYPH_DESPAWN:
                    me->DespawnOrUnsummon();
                    break;
                default:
                    break;
            }
        }
    }

private:
    EventMap _events;
    ObjectGuid _passengerGUID;
    bool _arrived;
};

// 43566 - Lorna Crowley (gunship deck static): staged boarding action.
// Timeline offsets from arming (= deck arrival), sniff 16:17:26 window.
struct npc_gilneas_endgame_controller : public ScriptedAI
{
    npc_gilneas_endgame_controller(Creature* creature) : ScriptedAI(creature)
    {
        Initialize();
    }

    void Initialize()
    {
        _running = false;
        _armTimer = 2000;
        _kormGUID.Clear();
    }

    void JustAppeared() override
    {
        ScriptedAI::JustAppeared();
        Initialize();
        _events.Reset();
    }

    void UpdateAI(uint32 diff) override
    {
        if (_armTimer <= diff)
        {
            _armTimer = 2000;
            if (!_running)
            {
                std::list<Player*> players;
                me->GetPlayerListInGrid(players, 40.0f);
                for (Player* player : players)
                {
                    if (player->IsAlive() && player->GetQuestStatus(QUEST_ENDGAME) == QUEST_STATUS_INCOMPLETE)
                    {
                        Arm();
                        break;
                    }
                }
            }
        }
        else
            _armTimer -= diff;

        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_ENDGAME_ATTACK:
                    Talk(SAY_LORNA_ATTACK);
                    EngageDeckWave(178.0f, 190.0f);
                    break;
                case EVENT_ENDGAME_RAFTERS:
                    Talk(SAY_LORNA_RAFTERS);
                    break;
                case EVENT_ENDGAME_RAPPEL:
                    Talk(SAY_LORNA_RAPPEL);
                    break;
                case EVENT_ENDGAME_DESCEND:
                    SquadDescend();
                    break;
                case EVENT_ENDGAME_HANDS_UP:
                    Talk(SAY_LORNA_HANDS_UP);
                    break;
                case EVENT_ENDGAME_DOWNSTAIRS:
                    Talk(SAY_LORNA_DOWNSTAIRS);
                    break;
                case EVENT_ENDGAME_FURNACE:
                    Talk(SAY_LORNA_FURNACE);
                    ActivateKorm();
                    _events.ScheduleEvent(EVENT_ENDGAME_KORM_POLL, 2s);
                    break;
                case EVENT_ENDGAME_BIG_ORC:
                    Talk(SAY_LORNA_BIG_ORC);
                    break;
                case EVENT_ENDGAME_KORM_POLL:
                    if (IsKormDead())
                    {
                        _abandonedPolls = 0;
                        _events.ScheduleEvent(EVENT_ENDGAME_WYVERN_YELL, 750ms);
                        _events.ScheduleEvent(EVENT_ENDGAME_LAUNCH, 3200ms);
                        _events.ScheduleEvent(EVENT_ENDGAME_RESET, 123s);
                    }
                    else
                    {
                        // Stuck-run watchdog: nobody left to kill Korm -> reset
                        if (HasEligiblePlayerNearby())
                            _abandonedPolls = 0;
                        else if (++_abandonedPolls >= 90) // ~3 minutes empty
                        {
                            _abandonedPolls = 0;
                            ResetEvent();
                            break;
                        }
                        _events.Repeat(2s);
                    }
                    break;
                case EVENT_ENDGAME_WYVERN_YELL:
                    Talk(SAY_LORNA_WYVERNS);
                    break;
                case EVENT_ENDGAME_LAUNCH:
                    LaunchWyverns();
                    break;
                case EVENT_ENDGAME_RESET:
                    ResetEvent();
                    break;
                default:
                    break;
            }
        }

        if (UpdateVictim())
            DoMeleeAttackIfReady();
    }

private:
    void Arm()
    {
        _running = true;
        _kormGUID.Clear();
        _events.Reset();

        _events.ScheduleEvent(EVENT_ENDGAME_ATTACK, 2100ms);
        _events.ScheduleEvent(EVENT_ENDGAME_RAFTERS, 63700ms);
        _events.ScheduleEvent(EVENT_ENDGAME_RAPPEL, 79900ms);
        _events.ScheduleEvent(EVENT_ENDGAME_DESCEND, 94000ms);
        _events.ScheduleEvent(EVENT_ENDGAME_HANDS_UP, 115600ms);
        _events.ScheduleEvent(EVENT_ENDGAME_DOWNSTAIRS, 126900ms);
        _events.ScheduleEvent(EVENT_ENDGAME_FURNACE, 172300ms);
        _events.ScheduleEvent(EVENT_ENDGAME_BIG_ORC, 183600ms);
    }

    void GetSquad(std::list<Creature*>& squad, float radius)
    {
        me->GetCreatureListWithEntryInGrid(squad, NPC_WORGEN_WARRIOR_SHIP, radius);
        me->GetCreatureListWithEntryInGrid(squad, NPC_GILNEAN_SHARPSHOOTER, radius);
    }

    Creature* FindNearestGruntInBand(WorldObject* from, float minZ, float maxZ)
    {
        Creature* nearest = nullptr;
        std::list<Creature*> grunts;
        me->GetCreatureListWithEntryInGrid(grunts, NPC_GUNSHIP_GRUNT, 120.0f);
        for (Creature* grunt : grunts)
        {
            if (!grunt->IsAlive() || grunt->GetPositionZ() < minZ || grunt->GetPositionZ() > maxZ)
                continue;
            if (!nearest || from->GetExactDist(grunt) < from->GetExactDist(nearest))
                nearest = grunt;
        }
        return nearest;
    }

    void EngageDeckWave(float minZ, float maxZ)
    {
        std::list<Creature*> squad;
        GetSquad(squad, 50.0f);
        for (Creature* member : squad)
        {
            if (!member->IsAlive() || member->IsInCombat() || member->GetPositionZ() < minZ)
                continue;
            if (Creature* grunt = FindNearestGruntInBand(member, minZ, maxZ))
                member->AI()->AttackStart(grunt);
        }

        if (Creature* grunt = FindNearestGruntInBand(me, minZ, maxZ))
            AttackStart(grunt);
    }

    void SquadDescend()
    {
        // Cosmetic rappel: drop up to four warriors to the rope-bottom marks.
        std::list<Creature*> warriors;
        me->GetCreatureListWithEntryInGrid(warriors, NPC_WORGEN_WARRIOR_SHIP, 60.0f);
        uint8 slot = 0;
        for (Creature* warrior : warriors)
        {
            if (slot >= std::size(SquadMidLevelPos))
                break;
            if (!warrior->IsAlive() || warrior->GetPositionZ() < 170.0f)
                continue;

            warrior->AttackStop();
            warrior->CombatStop(true);
            warrior->NearTeleportTo(SquadMidLevelPos[slot]);
            if (Creature* grunt = FindNearestGruntInBand(warrior, 120.0f, 140.0f))
                warrior->AI()->AttackStart(grunt);
            ++slot;
        }
    }

    void ActivateKorm()
    {
        if (Creature* korm = me->FindNearestCreature(NPC_KORM_BONEGRIND, 250.0f))
        {
            _kormGUID = korm->GetGUID();
            // Template ships him IMMUNE_TO_PC|IMMUNE_TO_NPC so he cannot be pulled
            // before the furnace beat; flags self-restore on his respawn.
            korm->SetImmuneToPC(false);
            korm->SetImmuneToNPC(false);
            korm->SetReactState(REACT_AGGRESSIVE);
            if (Player* target = korm->SelectNearestPlayer(60.0f))
                korm->AI()->AttackStart(target);
        }
    }

    bool HasEligiblePlayerNearby()
    {
        std::list<Player*> players;
        me->GetPlayerListInGrid(players, 250.0f);
        for (Player* player : players)
            if (player->IsAlive() && player->GetQuestStatus(QUEST_ENDGAME) == QUEST_STATUS_INCOMPLETE)
                return true;

        return false;
    }

    bool IsKormDead()
    {
        if (_kormGUID.IsEmpty())
        {
            if (Creature* korm = me->FindNearestCreature(NPC_KORM_BONEGRIND, 250.0f))
            {
                _kormGUID = korm->GetGUID();
                return !korm->IsAlive();
            }
            return true; // already killed and despawned
        }

        Creature* korm = ObjectAccessor::GetCreature(*me, _kormGUID);
        return !korm || !korm->IsAlive();
    }

    void LaunchWyverns()
    {
        // 81779 natively grants credit 43729, summons the wyvern and auto-rides.
        std::list<Player*> players;
        me->GetPlayerListInGrid(players, 150.0f);
        for (Player* player : players)
        {
            if (!player->IsAlive() || player->GetQuestStatus(QUEST_ENDGAME) != QUEST_STATUS_INCOMPLETE)
                continue;

            if (!player->GetVehicle() && sSpellMgr->GetSpellInfo(SPELL_SUMMON_WYVERN_RIDE))
                player->CastSpell(player, SPELL_SUMMON_WYVERN_RIDE, true);
            else
                player->KilledMonsterCredit(NPC_ENDGAME_CREDIT);
        }
    }

    void ResetEvent()
    {
        std::list<Creature*> squad;
        GetSquad(squad, 250.0f);
        for (Creature* member : squad)
        {
            if (!member->IsAlive())
                continue;
            member->AttackStop();
            member->CombatStop(true);
            member->NearTeleportTo(member->GetHomePosition());
        }

        _events.Reset();
        _kormGUID.Clear();
        _running = false; // grunts/Korm come back through the spawn system
    }

    EventMap _events;
    bool _running;
    uint32 _armTimer;
    uint32 _abandonedPolls = 0;
    ObjectGuid _kormGUID;
};

struct npc_gilneas_endgame_wyvern : public VehicleAI
{
    npc_gilneas_endgame_wyvern(Creature* creature) : VehicleAI(creature), _arrived(false) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        me->SetCanFly(true);
        me->SetDisableGravity(true);
    }

    void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
    {
        if (!passenger->IsPlayer())
            return;

        if (apply)
        {
            _passengerGUID = passenger->GetGUID();
            _events.ScheduleEvent(EVENT_WYVERN_TAKEOFF, 2s);
        }
        else if (!_arrived)
        {
            _events.Reset();
            me->DespawnOrUnsummon(2s);
        }
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != EFFECT_MOTION_TYPE && type != POINT_MOTION_TYPE)
            return;

        switch (pointId)
        {
            case POINT_WYVERN_LOOP:
                // Escape start: the gunship blows behind the wyvern.
                if (sSpellMgr->GetSpellInfo(SPELL_GUNSHIP_EXPLOSION_WYVERN))
                    me->CastSpell(me, SPELL_GUNSHIP_EXPLOSION_WYVERN, true);
                if (Player* player = ObjectAccessor::GetPlayer(*me, _passengerGUID))
                    if (sSpellMgr->GetSpellInfo(SPELL_GUNSHIP_EXPLOSION_PLAYER))
                        player->CastSpell(player, SPELL_GUNSHIP_EXPLOSION_PLAYER, true);
                me->GetMotionMaster()->MoveSmoothPath(POINT_WYVERN_ESCAPE, WyvernEscapePath, std::size(WyvernEscapePath), false, true, 24.0f);
                break;
            case POINT_WYVERN_ESCAPE:
                _arrived = true;
                if (Player* player = ObjectAccessor::GetPlayer(*me, _passengerGUID))
                {
                    player->ExitVehicle();
                    player->NearTeleportTo(WyvernEjectPos); // ExitVehicle discards exit positions in this fork
                }
                me->DespawnOrUnsummon(2s);
                break;
            default:
                break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        VehicleAI::UpdateAI(diff);
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            if (eventId == EVENT_WYVERN_TAKEOFF)
                me->GetMotionMaster()->MoveSmoothPath(POINT_WYVERN_LOOP, WyvernLoopPath, std::size(WyvernLoopPath), false, true, 16.0f);
        }
    }

private:
    EventMap _events;
    ObjectGuid _passengerGUID;
    bool _arrived;
};

/*######
## Shared player script: summon cleanup + login catch-ups
######*/

enum GilneasTail
{
    NPC_SWIFT_MOUNTAIN_HORSE        = 36741,
    NPC_DARK_SCOUT                  = 37953,
    NPC_HIPPOGRYPH                  = 43751,
    NPC_WYVERN                      = 43713,

    MAP_GILNEAS                     = 654,
    MAP_KALIMDOR                    = 1
};

Position const RuttheranArrivalPos  = { 8343.86f, 1165.28f, 4.40f, 4.834f };  // spell_target_position 78107
Position const KeelHarborDockPos    = { -1293.90f, 2129.48f, 6.68f, 6.2565f };
Position const GunshipDeckCenter    = { -1465.76f, 3251.48f, 93.84f };        // frozen ship frame origin

class player_script_gilneas_tail : public PlayerScript
{
public:
    player_script_gilneas_tail() : PlayerScript("player_script_gilneas_tail") { }

    void OnPlayerLogin(Player* player) override
    {
        if (player->GetMapId() != MAP_GILNEAS)
            return;

        // Logged out after the 14434 handoff: finish the trip to Rut'theran.
        if (player->GetQuestStatus(QUEST_RUTTHERAN_VILLAGE) == QUEST_STATUS_REWARDED)
        {
            player->TeleportTo(MAP_KALIMDOR, RuttheranArrivalPos.GetPositionX(), RuttheranArrivalPos.GetPositionY(), RuttheranArrivalPos.GetPositionZ(), RuttheranArrivalPos.GetOrientation());
            return;
        }

        // Logged out on the gunship deck mid-Endgame without a vehicle: back to Keel Harbor.
        if (player->GetQuestStatus(QUEST_ENDGAME) == QUEST_STATUS_INCOMPLETE && !player->GetVehicle() &&
            player->GetExactDist2d(GunshipDeckCenter.GetPositionX(), GunshipDeckCenter.GetPositionY()) < 200.0f &&
            player->GetPositionZ() > 90.0f)
            player->TeleportTo(MAP_GILNEAS, KeelHarborDockPos.GetPositionX(), KeelHarborDockPos.GetPositionY(), KeelHarborDockPos.GetPositionZ(), KeelHarborDockPos.GetOrientation());
    }

    void OnPlayerQuestStatusChange(Player* player, uint32 questId) override
    {
        QuestStatus status = player->GetQuestStatus(questId);
        if (status != QUEST_STATUS_NONE && status != QUEST_STATUS_FAILED && status != QUEST_STATUS_REWARDED)
            return;

        switch (questId)
        {
            case QUEST_TO_GREYMANE_MANOR:
                CleanupOwnedCreatures(player, { NPC_SWIFT_MOUNTAIN_HORSE });
                break;
            case QUEST_LOSING_YOUR_TAIL:
                CleanupOwnedCreatures(player, { NPC_DARK_SCOUT });
                break;
            case QUEST_THE_HUNT_FOR_SYLVANAS:
                CleanupOwnedCreatures(player, { NPC_TOBIAS_HUNT, NPC_WARHOWL, NPC_SYLVANAS_HUNT, NPC_CRENSHAW });
                break;
            case QUEST_PATRIARCHS_BLESSING:
                CleanupOwnedCreatures(player, { NPC_FUNERAL_CAMERA });
                break;
            case QUEST_ENDGAME:
                CleanupOwnedCreatures(player, { NPC_HIPPOGRYPH, NPC_WYVERN });
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

} // namespace Gilneas::Chapter4

void AddSC_gilneas_chapter_4()
{
    using namespace Gilneas::Chapter4;
    RegisterCreatureAI(npc_battle_for_gilneas_controller);
    RegisterCreatureAI(npc_gilneas_tobias_hunt);
    RegisterCreatureAI(npc_gilneas_captured_riding_bat);
    RegisterCreatureAI(npc_gilneas_funeral_camera);
    RegisterCreatureAI(npc_gilneas_funeral_controller);
    RegisterCreatureAI(npc_gilneas_glaive_thrower);
    RegisterCreatureAI(npc_gilneas_endgame_hippogryph);
    RegisterCreatureAI(npc_gilneas_endgame_controller);
    RegisterCreatureAI(npc_gilneas_endgame_wyvern);
    RegisterSpellScript(spell_gilneas_summon_tobias);
    RegisterSpellScript(spell_gilneas_half_burnt_torch);
    RegisterSpellScript(spell_gilneas_blessed_offerings);
    RegisterSpellScript(spell_gilneas_funeral_camera_summon);
    new player_script_gilneas_tail();
}
