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

#include "BattlefieldScript.h"
#include "ScriptMgr.h"
#include "ScriptRegistry.h"

BattlefieldScript::BattlefieldScript(char const* name)
    : ScriptObject(name)
{
    ScriptRegistry<BattlefieldScript>::Instance()->AddScript(this);
}

AllBattlefieldScript::AllBattlefieldScript(char const* name, std::vector<uint16> /*enabledHooks*/)
    : ScriptObject(name)
{
    ScriptRegistry<AllBattlefieldScript>::Instance()->AddScript(this);
}

Battlefield* ScriptMgr::CreateBattlefield(uint32 scriptId, Map* map)
{
    GET_SCRIPT_RET(BattlefieldScript, scriptId, tmpscript, nullptr);
    return tmpscript->GetBattlefield(map);
}

void ScriptMgr::OnBattlefieldPlayerEnterZone(Battlefield* bf, Player* player)
{
    ASSERT(bf);
    ASSERT(player);
    FOREACH_SCRIPT(AllBattlefieldScript)->OnBattlefieldPlayerEnterZone(bf, player);
}

void ScriptMgr::OnBattlefieldPlayerLeaveZone(Battlefield* bf, Player* player)
{
    ASSERT(bf);
    ASSERT(player);
    FOREACH_SCRIPT(AllBattlefieldScript)->OnBattlefieldPlayerLeaveZone(bf, player);
}

void ScriptMgr::OnBattlefieldPlayerJoinWar(Battlefield* bf, Player* player)
{
    ASSERT(bf);
    ASSERT(player);
    FOREACH_SCRIPT(AllBattlefieldScript)->OnBattlefieldPlayerJoinWar(bf, player);
}

void ScriptMgr::OnBattlefieldPlayerLeaveWar(Battlefield* bf, Player* player)
{
    ASSERT(bf);
    ASSERT(player);
    FOREACH_SCRIPT(AllBattlefieldScript)->OnBattlefieldPlayerLeaveWar(bf, player);
}

void ScriptMgr::OnBattlefieldBeforeInvitePlayerToWar(Battlefield* bf, Player* player)
{
    ASSERT(bf);
    ASSERT(player);
    FOREACH_SCRIPT(AllBattlefieldScript)->OnBattlefieldBeforeInvitePlayerToWar(bf, player);
}

void ScriptMgr::OnBattlefieldWarEnd(Battlefield* bf, bool endByTimer)
{
    ASSERT(bf);
    FOREACH_SCRIPT(AllBattlefieldScript)->OnBattlefieldWarEnd(bf, endByTimer);
}

template class TC_GAME_API ScriptRegistry<BattlefieldScript>;
template class TC_GAME_API ScriptRegistry<AllBattlefieldScript>;
