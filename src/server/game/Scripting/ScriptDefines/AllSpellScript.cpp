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

#include "AllSpellScript.h"
#include "ScriptMgr.h"
#include "ScriptRegistry.h"

AllSpellScript::AllSpellScript(char const* name, std::vector<uint16> /*enabledHooks*/)
    : ScriptObject(name)
{
    ScriptRegistry<AllSpellScript>::Instance()->AddScript(this);
}

void ScriptMgr::OnCalcMaxDuration(Aura const* aura, int32& maxDuration)
{
    FOREACH_SCRIPT(AllSpellScript)->OnCalcMaxDuration(aura, maxDuration);
}

void ScriptMgr::OnSpellCheckCast(Spell* spell, bool strict, SpellCastResult& res)
{
    FOREACH_SCRIPT(AllSpellScript)->OnSpellCheckCast(spell, strict, res);
}

bool ScriptMgr::CanPrepare(Spell* spell, SpellCastTargets const* targets, AuraEffect const* triggeredByAura)
{
    FOR_SCRIPTS_RET(AllSpellScript, itr, end, true)
        if (!itr->second->CanPrepare(spell, targets, triggeredByAura))
            return false;

    return true;
}

void ScriptMgr::OnDummyEffect(WorldObject* caster, uint32 spellID, SpellEffIndex effIndex, GameObject* gameObjTarget)
{
    FOREACH_SCRIPT(AllSpellScript)->OnDummyEffect(caster, spellID, effIndex, gameObjTarget);
}

void ScriptMgr::OnDummyEffect(WorldObject* caster, uint32 spellID, SpellEffIndex effIndex, Creature* creatureTarget)
{
    FOREACH_SCRIPT(AllSpellScript)->OnDummyEffect(caster, spellID, effIndex, creatureTarget);
}

void ScriptMgr::OnDummyEffect(WorldObject* caster, uint32 spellID, SpellEffIndex effIndex, Item* itemTarget)
{
    FOREACH_SCRIPT(AllSpellScript)->OnDummyEffect(caster, spellID, effIndex, itemTarget);
}

void ScriptMgr::OnSpellCastCancel(Spell* spell, WorldObject* caster, SpellInfo const* spellInfo, bool bySelf)
{
    FOREACH_SCRIPT(AllSpellScript)->OnSpellCastCancel(spell, caster, spellInfo, bySelf);
}

void ScriptMgr::OnSpellCast(Spell* spell, WorldObject* caster, SpellInfo const* spellInfo, bool skipCheck)
{
    FOREACH_SCRIPT(AllSpellScript)->OnSpellCast(spell, caster, spellInfo, skipCheck);
}

void ScriptMgr::OnSpellPrepare(Spell* spell, WorldObject* caster, SpellInfo const* spellInfo)
{
    FOREACH_SCRIPT(AllSpellScript)->OnSpellPrepare(spell, caster, spellInfo);
}

template class TC_GAME_API ScriptRegistry<AllSpellScript>;
