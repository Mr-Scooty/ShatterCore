/*
 * Temporary test module used to verify the module system during development.
 */

#include "Log.h"
#include "ScriptMgr.h"

class mod_test_worldscript : public WorldScript
{
public:
    mod_test_worldscript() : WorldScript("mod_test_worldscript") { }

    void OnStartup() override
    {
        TC_LOG_INFO("server.loading", "[mod-test] OnStartup fired - module system is alive.");
    }
};

void Addmod_testScripts()
{
    new mod_test_worldscript();
}
