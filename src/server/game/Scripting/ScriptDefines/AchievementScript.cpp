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

#include "AchievementScript.h"
#include "ScriptMgr.h"
#include "ScriptRegistry.h"

AchievementScript::AchievementScript(char const* name, std::vector<uint16> /*enabledHooks*/)
    : ScriptObject(name)
{
    ScriptRegistry<AchievementScript>::Instance()->AddScript(this);
}

void ScriptMgr::SetRealmCompleted(AchievementEntry const* achievement)
{
    FOREACH_SCRIPT(AchievementScript)->SetRealmCompleted(achievement);
}

bool ScriptMgr::IsCompletedCriteria(AchievementMgr<Player>* mgr, AchievementCriteriaEntry const* achievementCriteria, AchievementEntry const* achievement, CriteriaProgress const* progress)
{
    FOR_SCRIPTS_RET(AchievementScript, itr, end, true)
        if (!itr->second->IsCompletedCriteria(mgr, achievementCriteria, achievement, progress))
            return false;

    return true;
}

bool ScriptMgr::IsRealmCompleted(AchievementGlobalMgr const* globalmgr, AchievementEntry const* achievement, std::chrono::system_clock::time_point completionTime)
{
    FOR_SCRIPTS_RET(AchievementScript, itr, end, true)
        if (!itr->second->IsRealmCompleted(globalmgr, achievement, completionTime))
            return false;

    return true;
}

void ScriptMgr::OnBeforeCheckCriteria(AchievementMgr<Player>* mgr, AchievementCriteriaEntryList const* achievementCriteriaList)
{
    FOREACH_SCRIPT(AchievementScript)->OnBeforeCheckCriteria(mgr, achievementCriteriaList);
}

bool ScriptMgr::CanCheckCriteria(AchievementMgr<Player>* mgr, AchievementCriteriaEntry const* achievementCriteria)
{
    FOR_SCRIPTS_RET(AchievementScript, itr, end, true)
        if (!itr->second->CanCheckCriteria(mgr, achievementCriteria))
            return false;

    return true;
}

template class TC_GAME_API ScriptRegistry<AchievementScript>;
