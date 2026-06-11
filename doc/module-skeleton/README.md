# mod-skeleton

A template module for ShatterCore. Copy it into `modules/` (or use
`modules/create_module.sh`) and rename the `skeleton` tokens.

## Layout

```
mod-skeleton/
├── src/                      C++ sources (auto-discovered; a directory under
│                             modules/ is treated as a module iff src/ exists)
│   └── mod_skeleton.cpp      Must define void Addmod_skeletonScripts()
├── conf/
│   └── mod_skeleton.conf.dist  Installed to <conf>/modules/; MUST begin with
│                               the [worldserver] section header
├── data/sql/                 SQL updates, applied automatically at startup
│   ├── db-world/             e.g. 2026_06_12_00_mod_skeleton.sql
│   ├── db-auth/
│   ├── db-characters/
│   └── db-hotfixes/
└── mod-skeleton.cmake        Optional; included during CMake configure
```

## Rules

- The loader function is `Add<module name with underscores>Scripts()` —
  `mod-skeleton` → `Addmod_skeletonScripts()`. Exactly one source file must
  define it.
- SQL files apply in filename order and are tracked in the `updates` table
  with `state='MODULE'`; disabling the module never deletes its rows.
- Config values merge into the global configuration; prefix keys with your
  module name (`Skeleton.Enable`) to avoid collisions.
