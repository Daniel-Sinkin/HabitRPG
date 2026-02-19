# Engineer Integration Notes and Risks (Round 3)

## Implemented
- Canonical UI preset model (`calm`, `spark`, `custom`) with mutual exclusivity and custom sensory overrides.
- Persisted UI preferences with schema support and repository APIs for:
  - `preset_mode`
  - `last_non_custom_preset`
  - `motion_level`
  - `sound_level`
  - `density_level`
  - `queue_mode`
- Persisted milestone checkpoints with `candidate` to `confirmed` promotion flow.
- Reward gating policy for milestone checkpoints:
  - candidate checkpoints do not emit rewards
  - confirmed promotion emits at most one reward event (idempotent path)
- Runtime loading for artist/designer handoff assets:
  - theme JSON
  - runtime asset map JSON
  - designer copy pack markdown
- UI wiring for copy-pack high-frequency strings:
  - empty states
  - active conflict modal
  - completion toasts
  - save/retry failure text
- Migration path upgraded to `v3` with new tables:
  - `milestone_checkpoints`
  - `ui_preferences`
- Expanded automated regression coverage for Round 3:
  - preset behavior and persistence
  - checkpoint promotion idempotency
  - queue-mode persistence and filter behavior
  - migration validation to v3

## Known Limitations
1. Runtime file parsing is lightweight and format-sensitive.
- Theme/asset/copy parsing currently uses regex/manual extraction, not a strict JSON/Markdown parser.
- Malformed or unexpectedly reformatted source files can silently fall back to defaults.

2. Multi-table persistence is not wrapped in a single transaction.
- `PersistRuntimeState` writes user state, preferences, units, checkpoints, and rewards in sequence.
- A mid-save failure can leave a partially updated snapshot across tables.

3. Queue prioritization remains heuristic and static.
- Ranking uses fixed lifecycle weights plus priority score and deterministic alternation in mixed mode.
- Policy is not yet data-driven or externally configurable.

4. Performance profiling is still basic.
- Validation currently emphasizes compile/test correctness and runtime sanity checks.
- No formal frame-time or persistence throughput benchmark suite is in place.

## Blockers
- None currently.
