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

#ifndef SC_BATTLEFIELD_SCRIPT_H
#define SC_BATTLEFIELD_SCRIPT_H

#include "ScriptObject.h"
#include <vector>

class Battlefield;
class Map;
class Player;

// BattlefieldScript is database bound: the script is assigned to a specific
// battlefield (Wintergrasp, Tol Barad) through its `ScriptName` in the world
// database and acts as the factory for the Battlefield implementation.
class TC_GAME_API BattlefieldScript : public ScriptObject
{
    protected:

        BattlefieldScript(char const* name);

    public:

        virtual Battlefield* GetBattlefield(Map* map) const = 0;
};

/*
 * AzerothCore's battlefield event hook type is also named BattlefieldScript
 * (AzerothCore has no battlefield factory type, its Wintergrasp is created
 * directly). ShatterCore keeps the TrinityCore database bound factory under
 * that name, so the AzerothCore event hooks live on AllBattlefieldScript,
 * following the All* naming of the other non database bound broadcast types.
 *
 * AzerothCore's OnBattlefieldPlayerKill is not available: its call site sits
 * inside AzerothCore's Wintergrasp implementation (BattlefieldWG::HandleKill),
 * which is a content script in ShatterCore, not core code. Use the PlayerScript
 * OnPlayerPVPKill hook instead.
 */

// AzerothCore compatible hook enum. ShatterCore dispatches through the
// script registry for every registered script - the per-hook filtering
// of AzerothCore is accepted but not implemented (see DatabaseScript.h).
// Only the hooks that exist in ShatterCore are listed; the order follows
// AzerothCore's where the hook exists there.
enum BattlefieldHook : uint16
{
    BATTLEFIELDHOOK_ON_PLAYER_ENTER_ZONE,
    BATTLEFIELDHOOK_ON_PLAYER_LEAVE_ZONE,
    BATTLEFIELDHOOK_ON_PLAYER_JOIN_WAR,
    BATTLEFIELDHOOK_ON_PLAYER_LEAVE_WAR,
    BATTLEFIELDHOOK_BEFORE_INVITE_PLAYER_TO_WAR,
    BATTLEFIELDHOOK_ON_WAR_END,
    BATTLEFIELDHOOK_END
};

class TC_GAME_API AllBattlefieldScript : public ScriptObject
{
    protected:

        AllBattlefieldScript(char const* name, std::vector<uint16> enabledHooks = std::vector<uint16>());

    public:

        // Called at the top of Battlefield::HandlePlayerEnterZone, before the
        // player is added to the zone player list
        virtual void OnBattlefieldPlayerEnterZone(Battlefield* /*bf*/, Player* /*player*/) { }

        // Called at the end of Battlefield::HandlePlayerLeaveZone, after all cleanup
        virtual void OnBattlefieldPlayerLeaveZone(Battlefield* /*bf*/, Player* /*player*/) { }

        // Called after a player has been added to the active war (accepted the invitation)
        virtual void OnBattlefieldPlayerJoinWar(Battlefield* /*bf*/, Player* /*player*/) { }

        // Called after a player has been removed from the active war
        virtual void OnBattlefieldPlayerLeaveWar(Battlefield* /*bf*/, Player* /*player*/) { }

        // Called in Battlefield::InvitePlayerToWar after the kick entry of the
        // player is erased and before the player is added to the invited list
        virtual void OnBattlefieldBeforeInvitePlayerToWar(Battlefield* /*bf*/, Player* /*player*/) { }

        // Called at the end of Battlefield::EndBattle, after the zone script
        // OnBattleEnd has run (endByTimer is false when the attackers won)
        virtual void OnBattlefieldWarEnd(Battlefield* /*bf*/, bool /*endByTimer*/) { }
};

#endif // SC_BATTLEFIELD_SCRIPT_H
