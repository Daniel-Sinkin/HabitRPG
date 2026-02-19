# HabitRPG

Round 2 engineering implementation for a neurodivergent-friendly life + learning RPG desktop app.

## Stack
- C++23
- CMake presets
- SQLite persistence (schema migrations)
- Dear ImGui dockspace shell on SDL3 + OpenGL3

## Project Layout
- `include/habitrpg/app`, `src/app`: app state and runtime
- `include/habitrpg/ui`, `src/ui`: dockspace shell, view-model contracts, style/reward mapping
- `include/habitrpg/domain`, `src/domain`: entities, commands/events, reward engine, queue and interaction flow services
- `include/habitrpg/data`, `src/data`: repository interfaces, SQLite repo, schema migrations (v1 -> v2)
- `tests`: smoke, roundtrip, queue composition/ranking, lifecycle transitions, migration upgrade tests

## Build and Test
1. Configure core (tests only):
```bash
cmake --preset core
```
2. Build core:
```bash
cmake --build --preset core
```
3. Run tests:
```bash
ctest --preset core
```

## Round 2 Highlights
- Deterministic Today queue ranking/filtering for `mixed`, `life_only`, `learning_only`.
- Mixed queue composition alternates tracks when both tracks have pending units.
- UI flows for create/start/complete life actions.
- UI flows for create/start/checkpoint/complete C++ learning sessions.
- Single-active-unit runtime behavior: starting one unit pauses other active units across tracks.
- Explicit lifecycle states: `ready`, `active`, `partial`, `missed`, `paused`, `completed`, `checkpoint_candidate`.
- SQLite schema migration v2 for lifecycle/priority/checkpoint persistence.

## UI Runtime Build
`habitrpg_app` is built only when all Dear ImGui source files are available under `third_party/imgui`.

Populate vendored ImGui files:
```bash
./scripts/fetch_imgui.sh
```

Then configure/build full dev preset:
```bash
cmake --preset dev
cmake --build --preset dev
```

Run app:
```bash
./build/dev/habitrpg_app
```

## Engineering Notes
- Contracts: `docs/ENGINEER_CONTRACTS.md`
- Limitations/Risks: `docs/ENGINEER_LIMITATIONS.md`
