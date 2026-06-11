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

#include "UnitScript.h"
#include "ScriptMgr.h"
#include "ScriptRegistry.h"

UnitScript::UnitScript(char const* name, bool addToScripts, std::vector<uint16> /*enabledHooks*/)
    : ScriptObject(name)
{
    if (addToScripts)
        ScriptRegistry<UnitScript>::Instance()->AddScript(this);
}

void ScriptMgr::OnHeal(Unit* healer, Unit* reciever, uint32& gain)
{
    FOREACH_SCRIPT(UnitScript)->OnHeal(healer, reciever, gain);
}

void ScriptMgr::OnDamage(Unit* attacker, Unit* victim, uint32& damage)
{
    FOREACH_SCRIPT(UnitScript)->OnDamage(attacker, victim, damage);
}

void ScriptMgr::ModifyPeriodicDamageAurasTick(Unit* target, Unit* attacker, uint32& damage, SpellInfo const* spellInfo)
{
    FOREACH_SCRIPT(UnitScript)->ModifyPeriodicDamageAurasTick(target, attacker, damage, spellInfo);
}

void ScriptMgr::ModifyMeleeDamage(Unit* target, Unit* attacker, uint32& damage)
{
    FOREACH_SCRIPT(UnitScript)->ModifyMeleeDamage(target, attacker, damage);
}

void ScriptMgr::ModifySpellDamageTaken(Unit* target, Unit* attacker, int32& damage, SpellInfo const* spellInfo)
{
    FOREACH_SCRIPT(UnitScript)->ModifySpellDamageTaken(target, attacker, damage, spellInfo);
}

void ScriptMgr::ModifyHealReceived(Unit* target, Unit* healer, uint32& heal, SpellInfo const* spellInfo)
{
    FOREACH_SCRIPT(UnitScript)->ModifyHealReceived(target, healer, heal, spellInfo);
}

void ScriptMgr::OnAuraApply(Unit* unit, Aura* aura)
{
    FOREACH_SCRIPT(UnitScript)->OnAuraApply(unit, aura);
}

void ScriptMgr::OnAuraRemove(Unit* unit, AuraApplication* aurApp, AuraRemoveFlags mode)
{
    FOREACH_SCRIPT(UnitScript)->OnAuraRemove(unit, aurApp, mode);
}

void ScriptMgr::OnUnitUpdate(Unit* unit, uint32 diff)
{
    FOREACH_SCRIPT(UnitScript)->OnUnitUpdate(unit, diff);
}

void ScriptMgr::OnDisplayIdChange(Unit* unit, uint32 displayId)
{
    FOREACH_SCRIPT(UnitScript)->OnDisplayIdChange(unit, displayId);
}

void ScriptMgr::OnUnitEnterEvadeMode(Unit* unit, uint8 evadeReason)
{
    FOREACH_SCRIPT(UnitScript)->OnUnitEnterEvadeMode(unit, evadeReason);
}

void ScriptMgr::OnUnitEnterCombat(Unit* unit, Unit* victim)
{
    FOREACH_SCRIPT(UnitScript)->OnUnitEnterCombat(unit, victim);
}

void ScriptMgr::OnUnitDeath(Unit* unit, Unit* killer)
{
    FOREACH_SCRIPT(UnitScript)->OnUnitDeath(unit, killer);
}

void ScriptMgr::OnUnitSetShapeshiftForm(Unit* unit, uint8 form)
{
    FOREACH_SCRIPT(UnitScript)->OnUnitSetShapeshiftForm(unit, form);
}

template class TC_GAME_API ScriptRegistry<UnitScript>;
