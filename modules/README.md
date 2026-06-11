# ShatterCore Modules

Modules are self-contained addons living in this directory — their own C++
scripts, configuration files and SQL migrations — compiled into (or alongside)
the worldserver without touching the core source. The system mirrors
[AzerothCore's module system](https://www.azerothcore.org/wiki/create-a-module),
so AzerothCore module layouts and most module code port over directly (the
C++ API is 4.3.4, so game-API calls may need adaptation).

## Quick start

```bash
./create_module.sh mod-my-feature      # scaffold from doc/module-skeleton
cmake <build-dir> -DMODULES=static     # reconfigure; the module is auto-discovered
```

A directory under `modules/` is treated as a module **iff it contains a
`src/` directory**.

## Build configuration

| CMake option | Effect |
|---|---|
| `-DMODULES=static` (default) | All modules compiled into the worldserver |
| `-DMODULES=dynamic` | Each module becomes `libscripts_mod_<name>.so` in `bin/scripts/`, loaded (and hot-reloadable) at runtime via the script hot-swap system |
| `-DMODULES=disabled` | No modules built |
| `-DMODULE_MOD_MY_FEATURE=static\|dynamic\|disabled` | Per-module override |

Dynamic modules require dynamic linking (forced automatically) and are picked
up by the ScriptReloadMgr: with `HotSwap.Enabled = 1` in worldserver.conf,
replacing the `.so` in `bin/scripts/` hot-reloads the module without a
restart.

## Module layout

```
modules/mod-my-feature/
├── src/                          C++ sources, auto-collected
├── conf/*.conf.dist              installed to <conf dir>/modules/
├── data/sql/db-world/*.sql       applied automatically, tracked as
├── data/sql/db-auth/             state='MODULE' in the updates table
├── data/sql/db-characters/
├── data/sql/db-hotfixes/
└── mod-my-feature.cmake          optional CMake hook
```

## Rules and conventions

- **Loader function**: exactly one source file must define
  `void Add<name_with_underscores>Scripts()` using the full module name —
  e.g. `mod-my-feature` → `Addmod_my_featureScripts()`.
- **Config section header**: module `.conf`/`.conf.dist` files MUST begin
  with `[worldserver]` — without it every key is silently outside a section
  and the file is rejected with an error at startup.
- **Config precedence**: the server loads `<conf>/modules/<name>.conf` if it
  exists, else `<name>.conf.dist`. Later module files override earlier keys.
  `reload config` in the console re-reads module configs too.
- **SQL updates**: files apply in filename order after all core updates
  (guaranteeing the `updates.state` enum migration runs first). Rows are
  tagged `state='MODULE'` and are never warned about or cleaned when a
  module is disabled.
- **Hooks**: the full AzerothCore-style hook catalog is available — see
  `src/server/game/Scripting/ScriptDefines/`. Subclass a script type
  (`PlayerScript`, `WorldScript`, `AllCreatureScript`, `GlobalScript`,
  `ModuleScript` for custom in-module hooks, …) and instantiate it inside
  your loader function. The optional AzerothCore `enabledHooks` constructor
  argument is accepted for source compatibility but ignored.

## Porting AzerothCore modules — known differences

- Game version is 4.3.4: money is 64-bit, no ammo/feral-AP/arena-points
  hooks, talents are tree-based (no ChrSpecialization), phasing uses
  PhaseShift (phase-mask hooks unavailable). Each ScriptDefines header
  documents hooks that could not be ported and why.
- `CreatureScript` gossip virtuals are not restored (gossip flows through
  CreatureAI in modern TrinityCore) — use `AllCreatureScript`'s
  `CanCreatureGossip*` hooks instead.
- Namespace is `Trinity::`, logging is `TC_LOG_*` printf-style, config reads
  are `sConfigMgr->GetBoolDefault/GetIntDefault/...` (no `GetOption<T>`).
