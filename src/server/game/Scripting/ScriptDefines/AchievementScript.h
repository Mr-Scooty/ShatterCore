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

#ifndef SC_ACHIEVEMENT_SCRIPT_H
#define SC_ACHIEVEMENT_SCRIPT_H

#include "ScriptObject.h"
#include <chrono>
#include <vector>

/*
 * ShatterCore adaptation notes (4.3.4):
 *
 * AzerothCore's AchievementMgr is a plain class, the 4.3.4 one is templated
 * (AchievementMgr<Player> for player achievements, AchievementMgr<Guild> for
 * the cataclysm guild achievements). The AzerothCore hooks taking an
 * AchievementMgr* are therefore bound to the player specialization
 * AchievementMgr<Player>* here; guild achievement processing does NOT fire
 * these hooks.
 *
 * AzerothCore's SystemTimePoint typedef is expanded to its underlying
 * std::chrono::system_clock::time_point.
 */

class AchievementGlobalMgr;
class Player;
struct AchievementCriteriaEntry;
struct AchievementEntry;
struct CriteriaProgress;

template<class T>
class AchievementMgr;

typedef std::vector<AchievementCriteriaEntry const*> AchievementCriteriaEntryList;

// AzerothCore compatible hook enum. ShatterCore dispatches through the
// script registry for every registered script - the per-hook filtering
// of AzerothCore is accepted but not implemented (see DatabaseScript.h).
enum AchievementHook : uint16
{
    ACHIEVEMENTHOOK_SET_REALM_COMPLETED,
    ACHIEVEMENTHOOK_IS_COMPLETED_CRITERIA,
    ACHIEVEMENTHOOK_IS_REALM_COMPLETED,
    ACHIEVEMENTHOOK_ON_BEFORE_CHECK_CRITERIA,
    ACHIEVEMENTHOOK_CAN_CHECK_CRITERIA,
    ACHIEVEMENTHOOK_END
};

class TC_GAME_API AchievementScript : public ScriptObject
{
    protected:

        AchievementScript(char const* name, std::vector<uint16> enabledHooks = std::vector<uint16>());

    public:

        // Called when a realm first achievement is considered completed for the realm
        virtual void SetRealmCompleted(AchievementEntry const* /*achievement*/) { }

        // Called when checking if an achievement criteria has been completed, returning false marks it uncompleted
        [[nodiscard]] virtual bool IsCompletedCriteria(AchievementMgr<Player>* /*mgr*/, AchievementCriteriaEntry const* /*achievementCriteria*/, AchievementEntry const* /*achievement*/, CriteriaProgress const* /*progress*/) { return true; }

        // Called when checking if a realm first achievement is already completed, returning false marks it uncompleted
        [[nodiscard]] virtual bool IsRealmCompleted(AchievementGlobalMgr const* /*globalmgr*/, AchievementEntry const* /*achievement*/, std::chrono::system_clock::time_point /*completionTime*/) { return true; }

        // Called before the criteria list of an update round is processed
        virtual void OnBeforeCheckCriteria(AchievementMgr<Player>* /*mgr*/, AchievementCriteriaEntryList const* /*achievementCriteriaList*/) { }

        // Called for every criteria of an update round, returning false skips the criteria
        [[nodiscard]] virtual bool CanCheckCriteria(AchievementMgr<Player>* /*mgr*/, AchievementCriteriaEntry const* /*achievementCriteria*/) { return true; }
};

#endif // SC_ACHIEVEMENT_SCRIPT_H
