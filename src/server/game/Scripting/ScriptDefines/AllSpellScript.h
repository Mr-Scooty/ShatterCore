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

#ifndef SC_ALL_SPELL_SCRIPT_H
#define SC_ALL_SPELL_SCRIPT_H

#include "ScriptObject.h"
#include "SharedDefines.h"
#include <vector>

/*
 * The AllSpellScript virtuals follow their AzerothCore equivalents for
 * module source compatibility, with the following 4.3.4 adaptations:
 *
 *   The caster parameter of OnSpellPrepare/OnSpellCastCancel/OnSpellCast
 *   is a WorldObject* instead of AzerothCore's Unit* (in 4.3.4 spells can
 *   be cast by gameobjects too).
 *   OnSpellCastCancel's bySelf is always false (the 4.3.4 Spell::cancel
 *   has no such distinction).
 *
 * AzerothCore hooks bound to the WotLK aura-scaling/TargetInfo machinery
 * have no 4.3.4 equivalent and are not available: CanScalingEverything,
 * CanSelectSpecTalent, OnScaleAuraUnitAdd, OnRemoveAuraScaleTargets,
 * OnBeforeAuraRankForLevel.
 */

class Aura;
class AuraEffect;
class Creature;
class GameObject;
class Item;
class Spell;
class SpellCastTargets;
class SpellInfo;
class Unit;
class WorldObject;

// AzerothCore compatible hook enum. ShatterCore dispatches through the
// script registry for every registered script - the per-hook filtering
// of AzerothCore is accepted but not implemented (see DatabaseScript.h).
// Only the hooks that exist in ShatterCore are listed; the order follows
// AzerothCore's where the hook exists there.
enum AllSpellHook : uint16
{
    ALLSPELLHOOK_ON_CALC_MAX_DURATION,
    ALLSPELLHOOK_ON_SPELL_CHECK_CAST,
    ALLSPELLHOOK_CAN_PREPARE,
    ALLSPELLHOOK_ON_DUMMY_EFFECT_GAMEOBJECT,
    ALLSPELLHOOK_ON_DUMMY_EFFECT_CREATURE,
    ALLSPELLHOOK_ON_DUMMY_EFFECT_ITEM,
    ALLSPELLHOOK_ON_CAST_CANCEL,
    ALLSPELLHOOK_ON_CAST,
    ALLSPELLHOOK_ON_PREPARE,
    ALLSPELLHOOK_END
};

class TC_GAME_API AllSpellScript : public ScriptObject
{
    protected:

        AllSpellScript(char const* name, std::vector<uint16> enabledHooks = std::vector<uint16>());

    public:

        // Calculate max duration in applying aura
        virtual void OnCalcMaxDuration(Aura const* /*aura*/, int32& /*maxDuration*/) { }

        // Called at the top of Spell::CheckCast, res != SPELL_CAST_OK fails the cast
        virtual void OnSpellCheckCast(Spell* /*spell*/, bool /*strict*/, SpellCastResult& /*res*/) { }

        // Called in Spell::prepare, returning false aborts the cast
        [[nodiscard]] virtual bool CanPrepare(Spell* /*spell*/, SpellCastTargets const* /*targets*/, AuraEffect const* /*triggeredByAura*/) { return true; }

        /**
         * @brief This hook called after spell dummy effect
         *
         * @param caster Contains information about the WorldObject
         * @param spellID Contains information about the spell id
         * @param effIndex Contains information about the SpellEffIndex
         * @param gameObjTarget Contains information about the GameObject
         */
        virtual void OnDummyEffect(WorldObject* /*caster*/, uint32 /*spellID*/, SpellEffIndex /*effIndex*/, GameObject* /*gameObjTarget*/) { }

        /**
         * @brief This hook called after spell dummy effect
         *
         * @param caster Contains information about the WorldObject
         * @param spellID Contains information about the spell id
         * @param effIndex Contains information about the SpellEffIndex
         * @param creatureTarget Contains information about the Creature
         */
        virtual void OnDummyEffect(WorldObject* /*caster*/, uint32 /*spellID*/, SpellEffIndex /*effIndex*/, Creature* /*creatureTarget*/) { }

        /**
         * @brief This hook called after spell dummy effect
         *
         * @param caster Contains information about the WorldObject
         * @param spellID Contains information about the spell id
         * @param effIndex Contains information about the SpellEffIndex
         * @param itemTarget Contains information about the Item
         */
        virtual void OnDummyEffect(WorldObject* /*caster*/, uint32 /*spellID*/, SpellEffIndex /*effIndex*/, Item* /*itemTarget*/) { }

        // Called when a spell cast is cancelled
        virtual void OnSpellCastCancel(Spell* /*spell*/, WorldObject* /*caster*/, SpellInfo const* /*spellInfo*/, bool /*bySelf*/) { }

        // Called at the end of a successful spell cast (all casters, see also PlayerScript::OnPlayerSpellCast)
        virtual void OnSpellCast(Spell* /*spell*/, WorldObject* /*caster*/, SpellInfo const* /*spellInfo*/, bool /*skipCheck*/) { }

        // Called at the end of a successful Spell::prepare
        virtual void OnSpellPrepare(Spell* /*spell*/, WorldObject* /*caster*/, SpellInfo const* /*spellInfo*/) { }
};

// Compatibility for old scripts
using SpellSC = AllSpellScript;

#endif // SC_ALL_SPELL_SCRIPT_H
