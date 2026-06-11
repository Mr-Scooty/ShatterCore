/*
 * This file is part of the mod-skeleton module for ShatterCore.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 */

#include "Chat.h"
#include "Config.h"
#include "Log.h"
#include "Player.h"
#include "ScriptMgr.h"

class skeleton_worldscript : public WorldScript
{
public:
    skeleton_worldscript() : WorldScript("skeleton_worldscript") { }

    void OnAfterConfigLoad(bool reload) override
    {
        // Module configuration values live in conf/mod_skeleton.conf.dist
        // and are merged into the global configuration at startup.
        if (sConfigMgr->GetBoolDefault("Skeleton.Enable", false))
            TC_LOG_INFO("server.loading", "[mod-skeleton] Enabled (reload: %s).", reload ? "yes" : "no");
    }

    void OnStartup() override
    {
        TC_LOG_INFO("server.loading", "[mod-skeleton] World started.");
    }
};

class skeleton_playerscript : public PlayerScript
{
public:
    skeleton_playerscript() : PlayerScript("skeleton_playerscript") { }

    void OnPlayerLogin(Player* player) override
    {
        if (sConfigMgr->GetBoolDefault("Skeleton.Enable", false))
            ChatHandler(player->GetSession()).SendSysMessage("This server is running mod-skeleton.");
    }
};

// The loader entry point: "Add" + module name with underscores + "Scripts"
void Addmod_skeletonScripts()
{
    new skeleton_worldscript();
    new skeleton_playerscript();
}
