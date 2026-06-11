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

#ifndef SC_UNIT_SCRIPT_H
#define SC_UNIT_SCRIPT_H

#include "ScriptObject.h"
#include <vector>

/*
 * The UnitScript virtuals follow their AzerothCore equivalents for module
 * source compatibility, with the following 4.3.4 adaptations:
 *
 *   ModifyPeriodicDamageAurasTick / ModifySpellDamageTaken /
 *   ModifyHealReceived carry AzerothCore's SpellInfo const* parameter.
 *   ModifyMeleeDamage has no SpellInfo parameter (same as AzerothCore).
 *   OnAuraRemove receives ShatterCore's AuraRemoveFlags instead of
 *   AzerothCore's AuraRemoveMode (the 4.3.4 core tracks remove reasons
 *   as a flag mask).
 *   OnUnitEnterEvadeMode's evadeReason is a CreatureAI::EvadeReason value.
 *   OnUnitEnterCombat is dispatched from Creature::AtEngage (creatures
 *   only, like AzerothCore's Creature engage path).
 *
 * AzerothCore hooks without a clean 4.3.4 equivalent are not available:
 * DealDamage, OnBeforeRollMeleeOutcomeAgainst, IfNormalReaction,
 * CanSetPhaseMask, IsCustomBuildValuesUpdate,
 * ShouldTrackValuesUpdatePosByIndex, OnPatchValuesUpdate.
 */

class Aura;
class AuraApplication;
class SpellInfo;
class Unit;

enum class AuraRemoveFlags : uint32;

// AzerothCore compatible hook enum. ShatterCore dispatches through the
// script registry for every registered script - the per-hook filtering
// of AzerothCore is accepted but not implemented (see DatabaseScript.h).
// Only the hooks that exist in ShatterCore are listed; the order follows
// AzerothCore's where the hook exists there.
enum UnitHook : uint16
{
    UNITHOOK_ON_HEAL,
    UNITHOOK_ON_DAMAGE,
    UNITHOOK_MODIFY_PERIODIC_DAMAGE_AURAS_TICK,
    UNITHOOK_MODIFY_MELEE_DAMAGE,
    UNITHOOK_MODIFY_SPELL_DAMAGE_TAKEN,
    UNITHOOK_MODIFY_HEAL_RECEIVED,
    UNITHOOK_ON_AURA_APPLY,
    UNITHOOK_ON_AURA_REMOVE,
    UNITHOOK_ON_UNIT_UPDATE,
    UNITHOOK_ON_DISPLAYID_CHANGE,
    UNITHOOK_ON_UNIT_ENTER_EVADE_MODE,
    UNITHOOK_ON_UNIT_ENTER_COMBAT,
    UNITHOOK_ON_UNIT_DEATH,
    UNITHOOK_ON_UNIT_SET_SHAPESHIFT_FORM,
    UNITHOOK_END
};

class TC_GAME_API UnitScript : public ScriptObject
{
    protected:

        UnitScript(char const* name, bool addToScripts = true, std::vector<uint16> enabledHooks = std::vector<uint16>());

    public:

        // Called when a unit deals healing to another unit
        virtual void OnHeal(Unit* /*healer*/, Unit* /*reciever*/, uint32& /*gain*/) { }

        // Called when a unit deals damage to another unit
        virtual void OnDamage(Unit* /*attacker*/, Unit* /*victim*/, uint32& /*damage*/) { }

        // Called when DoT's Tick Damage is being Dealt
        // Attacker can be nullptr if he is despawned while the aura still exists on target
        virtual void ModifyPeriodicDamageAurasTick(Unit* /*target*/, Unit* /*attacker*/, uint32& /*damage*/, SpellInfo const* /*spellInfo*/) { }

        // Called when Melee Damage is being Dealt
        virtual void ModifyMeleeDamage(Unit* /*target*/, Unit* /*attacker*/, uint32& /*damage*/) { }

        // Called when Spell Damage is being Dealt
        virtual void ModifySpellDamageTaken(Unit* /*target*/, Unit* /*attacker*/, int32& /*damage*/, SpellInfo const* /*spellInfo*/) { }

        // Called when Heal is Received
        virtual void ModifyHealReceived(Unit* /*target*/, Unit* /*healer*/, uint32& /*heal*/, SpellInfo const* /*spellInfo*/) { }

        // Called when an aura is applied to a unit
        virtual void OnAuraApply(Unit* /*unit*/, Aura* /*aura*/) { }

        // Called when an aura application is removed from a unit
        virtual void OnAuraRemove(Unit* /*unit*/, AuraApplication* /*aurApp*/, AuraRemoveFlags /*mode*/) { }

        /**
         * @brief This hook runs in Unit::Update
         *
         * @param unit Contains information about the Unit
         * @param diff Contains information about the diff time
         */
        virtual void OnUnitUpdate(Unit* /*unit*/, uint32 /*diff*/) { }

        // Called when the display id of a unit changes
        virtual void OnDisplayIdChange(Unit* /*unit*/, uint32 /*displayId*/) { }

        // Called when a creature AI enters evade mode, evadeReason is a CreatureAI::EvadeReason value
        virtual void OnUnitEnterEvadeMode(Unit* /*unit*/, uint8 /*evadeReason*/) { }

        // Called when a creature engages a target (enters combat)
        virtual void OnUnitEnterCombat(Unit* /*unit*/, Unit* /*victim*/) { }

        // Called when a unit dies
        virtual void OnUnitDeath(Unit* /*unit*/, Unit* /*killer*/) { }

        // Called when the shapeshift form of a unit changes
        virtual void OnUnitSetShapeshiftForm(Unit* /*unit*/, uint8 /*form*/) { }
};

#endif // SC_UNIT_SCRIPT_H
