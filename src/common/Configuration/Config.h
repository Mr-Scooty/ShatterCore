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

#ifndef CONFIG_H
#define CONFIG_H

#include "Define.h"
#include <string>
#include <type_traits>
#include <string_view>
#include <vector>

class TC_COMMON_API ConfigMgr
{
    ConfigMgr() = default;
    ConfigMgr(ConfigMgr const&) = delete;
    ConfigMgr& operator=(ConfigMgr const&) = delete;
    ~ConfigMgr() = default;

public:
    /// Method used only for loading main configuration files (bnetserver.conf and worldserver.conf)
    bool LoadInitial(std::string const& file, std::vector<std::string> args, std::string& error);

    static ConfigMgr* instance();

    bool Reload(std::string& error);

    /// Stores the comma separated list of module configuration file names
    /// (baked in at compile time through the CONFIG_FILE_LIST define).
    void SetModuleConfigFileList(std::string_view configFileList);

    /// Loads all module configuration files from the "modules" directory
    /// next to the main configuration file and merges their settings
    /// into the main configuration (later files override earlier keys).
    /// Looks for "<name>.conf" first and falls back to "<name>.conf.dist".
    bool LoadModulesConfigs(bool isReload = false);

    std::string GetStringDefault(std::string const& name, const std::string& def) const;
    bool GetBoolDefault(std::string const& name, bool def) const;
    int GetIntDefault(std::string const& name, int def) const;
    float GetFloatDefault(std::string const& name, float def) const;

    /// AzerothCore module compatibility: typed option getter dispatching to the
    /// Get*Default accessors above (modules ported from AC use sConfigMgr->GetOption<T>).
    template<class T>
    T GetOption(std::string const& name, T const& def, bool /*showLogs*/ = true) const
    {
        if constexpr (std::is_same_v<T, bool>)
            return GetBoolDefault(name, def);
        else if constexpr (std::is_same_v<T, std::string>)
            return GetStringDefault(name, def);
        else if constexpr (std::is_floating_point_v<T>)
            return static_cast<T>(GetFloatDefault(name, static_cast<float>(def)));
        else
            return static_cast<T>(GetIntDefault(name, static_cast<int>(def)));
    }

    std::string const& GetFilename();
    std::vector<std::string> const& GetArguments() const;
    std::vector<std::string> GetKeysByString(std::string const& name);

private:
    template<class T>
    T GetValueDefault(std::string const& name, T def) const;
};

#define sConfigMgr ConfigMgr::instance()

#endif
