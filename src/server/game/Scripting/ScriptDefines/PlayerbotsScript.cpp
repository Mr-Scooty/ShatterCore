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

#include "PlayerbotsScript.h"
#include "ScriptMgr.h"
#include "ScriptRegistry.h"

PlayerbotScript::PlayerbotScript(char const* name, std::vector<uint16> /*enabledHooks*/)
    : ScriptObject(name)
{
    ScriptRegistry<PlayerbotScript>::Instance()->AddScript(this);
}

bool ScriptMgr::OnPlayerbotCheckLFGQueue(GuidList const& guidsList)
{
    FOR_SCRIPTS_RET(PlayerbotScript, itr, end, true)
        if (!itr->second->OnPlayerbotCheckLFGQueue(guidsList))
            return false;

    return true;
}

void ScriptMgr::OnPlayerbotCheckKillTask(Player* player, Unit* victim)
{
    FOREACH_SCRIPT(PlayerbotScript)->OnPlayerbotCheckKillTask(player, victim);
}

void ScriptMgr::OnPlayerbotCheckPetitionAccount(Player* player, bool& found)
{
    FOREACH_SCRIPT(PlayerbotScript)->OnPlayerbotCheckPetitionAccount(player, found);
}

bool ScriptMgr::OnPlayerbotCheckUpdatesToSend(Player* player)
{
    FOR_SCRIPTS_RET(PlayerbotScript, itr, end, true)
        if (!itr->second->OnPlayerbotCheckUpdatesToSend(player))
            return false;

    return true;
}

void ScriptMgr::OnPlayerbotPacketSent(Player* player, WorldPacket const* packet)
{
    FOREACH_SCRIPT(PlayerbotScript)->OnPlayerbotPacketSent(player, packet);
}

void ScriptMgr::OnPlayerbotUpdate(uint32 diff)
{
    FOREACH_SCRIPT(PlayerbotScript)->OnPlayerbotUpdate(diff);
}

void ScriptMgr::OnPlayerbotUpdateSessions(Player* player)
{
    FOREACH_SCRIPT(PlayerbotScript)->OnPlayerbotUpdateSessions(player);
}

void ScriptMgr::OnPlayerbotLogout(Player* player)
{
    FOREACH_SCRIPT(PlayerbotScript)->OnPlayerbotLogout(player);
}

void ScriptMgr::OnPlayerbotLogoutBots()
{
    FOREACH_SCRIPT(PlayerbotScript)->OnPlayerbotLogoutBots();
}

template class TC_GAME_API ScriptRegistry<PlayerbotScript>;
