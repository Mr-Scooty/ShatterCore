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

#include "ModuleMgr.h"

namespace
{
    std::string _modulesList;
}

void Trinity::Module::SetEnableModulesList(std::string_view modulesList)
{
    _modulesList = modulesList;
}

std::vector<std::string> Trinity::Module::GetEnableModulesList()
{
    std::vector<std::string> list;

    std::string_view remaining(_modulesList);
    while (!remaining.empty())
    {
        size_t const pos = remaining.find(',');
        std::string_view const token = remaining.substr(0, pos);
        if (!token.empty())
            list.emplace_back(token);

        if (pos == std::string_view::npos)
            break;

        remaining.remove_prefix(pos + 1);
    }

    return list;
}

bool Trinity::Module::IsModuleEnabled(std::string_view moduleName)
{
    for (std::string const& name : GetEnableModulesList())
        if (name == moduleName)
            return true;

    return false;
}
