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

#ifndef _MODULE_MGR_H_
#define _MODULE_MGR_H_

#include "Define.h"
#include <string>
#include <string_view>
#include <vector>

namespace Trinity::Module
{
    /// Stores the comma separated list of enabled modules
    /// (baked in at compile time through the TC_MODULES_LIST define).
    TC_GAME_API void SetEnableModulesList(std::string_view modulesList);

    /// Returns the names of all enabled modules.
    TC_GAME_API std::vector<std::string> GetEnableModulesList();

    /// Returns whether the module with the given name is enabled.
    TC_GAME_API bool IsModuleEnabled(std::string_view moduleName);
}

#endif // _MODULE_MGR_H_
