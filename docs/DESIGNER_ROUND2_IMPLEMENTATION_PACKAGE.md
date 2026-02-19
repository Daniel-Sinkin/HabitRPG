# HabitRPG Designer Round 2 Implementation Package

Date: February 19, 2026  
Owner: Designer Agent  
Source instruction: `agents/DESIGNER_AGENT.md` (Round 2 Tasks)

## 1) Deliverables Completed

1. Finalized implementation contract deltas and locked state taxonomy.
2. Published definitive persisted field requirements for queue/session/milestone state.
3. Produced implementation QA spec with pass/fail checks mapped to UI controls and expected repository writes.
4. Added explicit acceptance tests for `mixed` / `life_only` / `learning_only` queue behavior.
5. Defined learning UX quality rails for stuck interventions and artifact-light vs artifact-rich completion variants.
6. Validated Calm/Spark behavior against active implementation and reported mismatches.

## 2) Contract Deltas (v1 -> v2 Proposal)

### 2.1 Locked State Taxonomy

`track_type`
- `life`
- `learning`

`queue_mode`
- `mixed`
- `life_only`
- `learning_only`

`action_unit_state`
- `ready`
- `active`
- `paused`
- `partial`
- `completed`
- `missed`

`learning_goal_state`
- `new`
- `in_progress`
- `stuck`
- `milestone_candidate`
- `milestone_ready`
- `completed`

`learning_session_state`
- `idle`
- `active`
- `completed`
- `abandoned`

`milestone_checkpoint_state`
- `candidate`
- `confirmed`
- `rejected`

### 2.2 Candidate Milestone Behavior (Ambiguity Removed)

Definition:
- A `candidate` checkpoint is user-submitted milestone evidence that is intentionally unconfirmed.

Hard rules:
1. `candidate` does not grant XP or badge rewards.
2. `candidate` does not advance milestone progression counters.
3. `candidate` must remain visible in a pending-review list.
4. Only `confirmed` checkpoints can grant milestone reward events.
5. Promotion path must be one action: `candidate -> confirmed`.
6. Rejection path must preserve evidence history: `candidate -> rejected`.

Triggers:
1. Auto-candidate when confidence <= 2 of 5.
2. Auto-candidate when user selects "Mark as candidate" in checkpoint dialog.
3. Manual promote/reject from milestone review panel.

### 2.3 Component Contract Deltas (Implementation-Ready)

| Component Name | Trigger | Expected State Update | Edge Cases |
| --- | --- | --- | --- |
| `QueueModeSegmentedControl` | User picks mixed/life/learning | `queue_mode` persisted; queue re-ranked/re-filtered | Empty mode shows contextual empty state + add CTA |
| `DoNextCardPrimaryButton` | Start/Continue/Complete action | `action_unit_state` transition based on CTA | Conflict if another session active |
| `RecoveryBannerAction` | Select recovery option | Applies non-punitive reschedule/split/drop updates | Retry path required if write fails |
| `LearningSessionCompleteButton` | Confirm completion variant | `learning_session_state=completed`; artifact metadata persisted | `artifact_kind=none` allowed |
| `MilestoneCheckpointSubmitButton` | Submit checkpoint | `milestone_checkpoint_state=candidate` or `confirmed` | Confirmed only if evidence minimum met or forced confirm |
| `MilestoneCandidatePromoteButton` | Promote pending candidate | `candidate -> confirmed`; reward event emitted once | Idempotency: no duplicate reward events |
| `PresetModeToggle` | Select Calm or Spark | `preset_mode` and dependent controls updated | Calm/Spark mutual exclusivity required |
| `SensoryControlOverride` | Manual motion/sound/density change | `preset_mode=custom`; override values persisted | Preserve last non-custom preset for one-click restore |

## 3) Persisted Field Requirements (Definitive)

### 3.1 Queue State Persistence (`queue_items`)

Required fields:

1. `id` TEXT PK
2. `track_type` TEXT CHECK (`life`,`learning`)
3. `source_type` TEXT (`action_unit`,`learning_session`,`system_recovery`)
4. `source_id` TEXT
5. `queue_mode_origin` TEXT CHECK (`mixed`,`life_only`,`learning_only`)
6. `state` TEXT CHECK (`ready`,`active`,`paused`,`partial`,`completed`,`missed`)
7. `energy_tag` TEXT CHECK (`low`,`normal`,`high`)
8. `estimated_minutes` INTEGER
9. `priority_score` INTEGER
10. `is_two_min_variant` INTEGER (0/1)
11. `position_hint` INTEGER
12. `due_at` TEXT nullable
13. `started_at` TEXT nullable
14. `paused_at` TEXT nullable
15. `completed_at` TEXT nullable
16. `missed_at` TEXT nullable
17. `created_at` TEXT NOT NULL
18. `updated_at` TEXT NOT NULL

Indexes:
1. `(state, priority_score DESC, track_type)`
2. `(queue_mode_origin, state)`
3. `(due_at)`

### 3.2 Learning Session Persistence (`learning_sessions`)

Existing fields retained and extended:

1. `id` TEXT PK
2. `goal_id` TEXT FK
3. `state` TEXT CHECK (`idle`,`active`,`completed`,`abandoned`)
4. `mode` TEXT CHECK (`read`,`build`,`review`)
5. `duration_minutes` INTEGER
6. `artifact_kind` TEXT CHECK (`none`,`note`,`code_snippet`,`solved_exercise`)
7. `artifact_ref` TEXT nullable
8. `retention_key_concept` TEXT nullable
9. `retention_next_use` TEXT nullable
10. `confidence_level` INTEGER CHECK (1..5) nullable
11. `friction_tag` TEXT nullable
12. `started_at` TEXT nullable
13. `completed_at` TEXT nullable
14. `abandoned_at` TEXT nullable
15. `created_at` TEXT NOT NULL
16. `updated_at` TEXT NOT NULL

### 3.3 Milestone Persistence (`milestone_checkpoints`)

Required table:

1. `id` TEXT PK
2. `goal_id` TEXT FK
3. `milestone_key` TEXT
4. `state` TEXT CHECK (`candidate`,`confirmed`,`rejected`)
5. `evidence_kind` TEXT CHECK (`note`,`snippet`,`exercise`,`reference`)
6. `evidence_ref` TEXT nullable
7. `confidence_level` INTEGER CHECK (1..5)
8. `candidate_reason` TEXT nullable
9. `reward_event_id` TEXT nullable UNIQUE
10. `submitted_at` TEXT NOT NULL
11. `reviewed_at` TEXT nullable
12. `confirmed_at` TEXT nullable
13. `rejected_at` TEXT nullable
14. `created_at` TEXT NOT NULL
15. `updated_at` TEXT NOT NULL

Rule:
- `reward_event_id` must be null unless `state='confirmed'`.

### 3.4 Sensory Preference Persistence (`ui_preferences`)

Required fields:

1. `id` INTEGER PK CHECK (`id=1`)
2. `preset_mode` TEXT CHECK (`calm`,`spark`,`custom`)
3. `motion_level` INTEGER CHECK (0..2)
4. `sound_level` INTEGER CHECK (0..2)
5. `density_level` INTEGER CHECK (0..2)
6. `prompt_concurrency_limit` INTEGER
7. `nudge_cooldown_seconds` INTEGER
8. `updated_at` TEXT NOT NULL

## 4) QA Specification for Active Build

### 4.1 Baseline Build Verification

Command run:

```bash
./build/dev/habitrpg_tests
```

Observed result:
- `[PASS] startup_smoke`
- `[PASS] habit_action_reward_roundtrip`
- `[PASS] learning_session_reward_roundtrip`
- `All tests passed`

Interpretation:
- Core bootstrap and reward/persistence roundtrip paths pass.
- Round 2 interaction-layer acceptance is not covered by automated tests yet.

### 4.2 Flow QA: Pass/Fail Checks Mapped to UI + Repository Writes

Status legend:
- `PASS`: implemented and behaving per requirement.
- `FAIL`: control exists but required behavior not implemented.
- `MISSING`: control/behavior absent.

| QA ID | Flow | UI Control | Expected UI Update | Expected Repository Write | Active Build |
| --- | --- | --- | --- | --- | --- |
| `QA-LIFE-ADD-001` | Add habit | `+ Habit` quick action | Habit appears in list + queue refresh | `habits` insert + queue item insert | MISSING |
| `QA-LIFE-START-001` | Start action | `Start` on do-next card | Unit enters `active`; bottom strip starts timer | `action_units.state=active`, `started_at` set | FAIL |
| `QA-LIFE-COMPLETE-001` | Complete action | `Complete` in center or strip | Confirmation state + next queue item promoted | `action_units.state=completed` + `reward_events` insert + `user_state` XP update | FAIL |
| `QA-LIFE-RECOVER-001` | Recover missed day | Recovery banner CTA | Non-punitive plan applied; queue rebuilt | Queue/missed-state updates persisted | MISSING |
| `QA-LEARN-ADD-001` | Add learning goal | `+ Learning Goal` quick action | Goal card appears; first session queued | `learning_goals` insert + queue item insert | MISSING |
| `QA-LEARN-START-001` | Start learning session | `Start Session` | Session active in strip | `learning_sessions.state=active`, `started_at` set | MISSING |
| `QA-LEARN-COMPLETE-001` | Complete learning session | Completion panel confirm | Feedback + milestone progress update | `learning_sessions.state=completed`; artifact fields; `reward_events` insert | FAIL |
| `QA-LEARN-CHECK-001` | Milestone checkpoint | `Checkpoint` | Candidate or confirmed state visible | `milestone_checkpoints` insert/update | MISSING |
| `QA-PRESET-001` | Calm mode | `Calm Mode` checkbox | Applies full calm bundle values | `ui_preferences` update | FAIL |
| `QA-PRESET-002` | Spark mode | `Spark Mode` checkbox | Applies spark bundle with bounds | `ui_preferences` update | FAIL |
| `QA-PRESET-003` | Preset exclusivity | Calm then Spark toggle | Exactly one preset active | `preset_mode` single value | FAIL |
| `QA-PRESET-004` | Sensory persistence | Restart app after preset change | Prior preset restored | Persisted `ui_preferences` read/write | FAIL |

### 4.3 Queue Mode Acceptance Tests (Required Explicit Coverage)

| QA ID | Mode | Steps | Expected Result | Active Build |
| --- | --- | --- | --- | --- |
| `QA-QUEUE-MIXED-001` | `mixed` | Select Mixed track filter | Do-next shows both life and learning items when both exist | FAIL (static list, no real filter binding) |
| `QA-QUEUE-LIFE-001` | `life_only` | Select Life filter | Do-next contains only life items | FAIL |
| `QA-QUEUE-LEARN-001` | `learning_only` | Select Learning filter | Do-next contains only learning items | FAIL |
| `QA-QUEUE-PERSIST-001` | Any mode | Change mode, restart app | Same queue mode restored | FAIL (no UI preference persistence) |
| `QA-QUEUE-EMPTY-001` | Filtered empty | Select mode with no items | Targeted empty-state copy + add CTA | FAIL |

### 4.4 Active Wiring Evidence

Queue/filter controls exist:
- `src/ui/dockspace_shell.cpp:110`
- `src/ui/dockspace_shell.cpp:113`
- `src/ui/dockspace_shell.cpp:114`
- `src/ui/dockspace_shell.cpp:115`

Queue rendering is static text (not filter/data-backed):
- `src/ui/dockspace_shell.cpp:127`
- `src/ui/dockspace_shell.cpp:128`
- `src/ui/dockspace_shell.cpp:129`
- `src/ui/dockspace_shell.cpp:130`

No persistence path for UI state:
- `src/app/application.cpp:111`
- `src/app/application.cpp:113`
- `src/app/application.cpp:160`

Current persisted schema lacks queue/session-state/milestone tables:
- `src/data/migrations.cpp:58`
- `src/data/migrations.cpp:79`
- `src/data/migrations.cpp:91`
- `src/data/migrations.cpp:103`

## 5) Learning UX Quality Rails

### 5.1 Stuck Intervention Patterns (C++ Goals)

Trigger conditions:
1. Three planned sessions missed in 7 days.
2. Confidence <= 2 for two sessions in a row.
3. Same milestone unchanged for 10+ days.

Intervention menu (ranked):
1. `Reduce Scope`:
- Split milestone into one 10-minute starter and one deeper follow-up.
2. `Switch Mode`:
- Move next session from `build` to `review` or `read`.
3. `Use Worked Example`:
- Prompt to complete a solved example before original task.
4. `Take Recovery Session`:
- One low-load reflection session with artifact optional.

Required behavior:
1. Interventions must be optional, not forced.
2. Any intervention selection writes a plan-adjustment event (telemetry/local log).
3. First intervention CTA is one-click from stuck banner.

### 5.2 Session Completion Variants

`artifact_light` variant:
- Trigger when session duration <= 15 min or energy set to low.
- Completion fields required:
1. `artifact_kind=none|note`
2. optional one-line key concept
- Goal: keep closure friction low.

`artifact_rich` variant:
- Trigger when session duration > 15 min or milestone-ready context.
- Completion fields required:
1. `artifact_kind` in `note|code_snippet|solved_exercise`
2. `artifact_ref` required for snippet/exercise
3. confidence score required
- Goal: strengthen retention and evidence quality.

Fallback:
- User can downgrade artifact-rich to artifact-light once per session without error state.

## 6) Calm/Spark Validation Against Active UI Wiring

### 6.1 Expected vs Observed

| Area | Expected | Observed in Active Build | Status |
| --- | --- | --- | --- |
| Preset model | Single `preset_mode` (`calm`/`spark`/`custom`) | Two independent booleans (`calm_mode`, `spark_mode`) | Mismatch |
| Mutual exclusivity | Calm and Spark cannot both be active | Both checkboxes can be true | Mismatch |
| Calm default bundle | Motion Off, Sound Off, Density Spacious | Defaults are motion low, sound off, density comfortable | Mismatch |
| Spark bundle + bounds | Motion low + nudge bounds/cooldowns | No bounded stimulation logic present | Mismatch |
| Persistence | Preset and controls survive restart | Only `user_state` is saved; UI state not persisted | Mismatch |
| Behavioral impact | Presets change queue/feedback behavior | Toggles currently do not alter queue/feedback logic | Mismatch |

Evidence locations:
- `include/habitrpg/ui/contracts.hpp:24`
- `include/habitrpg/ui/contracts.hpp:28`
- `include/habitrpg/ui/contracts.hpp:29`
- `include/habitrpg/ui/contracts.hpp:31`
- `include/habitrpg/ui/contracts.hpp:33`
- `src/ui/dockspace_shell.cpp:183`
- `src/ui/dockspace_shell.cpp:184`
- `src/ui/dockspace_shell.cpp:190`
- `src/ui/dockspace_shell.cpp:192`
- `src/app/application.cpp:160`

### 6.2 Required Fix Order

1. Replace boolean preset toggles with one `preset_mode`.
2. Add preset application function that sets linked motion/sound/density defaults.
3. Add `ui_preferences` persistence load/save path.
4. Apply preset behavior to queue prompts and feedback intensity.
5. Add automated regression checks for preset exclusivity and persistence.

## 7) Architect Decision Flags

1. If candidate checkpoints should ever grant partial XP, Architect decision required.
2. If mixed mode must enforce 1+ item per track in top 3 under all conditions, Architect decision required.
3. If stuck intervention should auto-trigger vs only recommend, Architect decision required.

## 8) Round 3 Validation Addendum (2026-02-19, v3)

### 8.1 Validation Method

1. Re-ran automated verification:

```bash
./build/dev/habitrpg_tests
```

Result:
- `[PASS] startup_smoke`
- `[PASS] habit_action_reward_roundtrip`
- `[PASS] learning_session_reward_roundtrip`
- `[PASS] today_queue_ranking`
- `[PASS] mixed_queue_composition`
- `[PASS] single_active_conflict_resolution`
- `[PASS] learning_checkpoint_lifecycle`
- `[PASS] schema_migration_v1_to_v2`
- `All tests passed`

2. Reviewed active UI wiring and persistence paths in:
- `/Users/danielsinkin/GitHub_private/HabitRPG/src/ui/dockspace_shell.cpp`
- `/Users/danielsinkin/GitHub_private/HabitRPG/src/app/application.cpp`
- `/Users/danielsinkin/GitHub_private/HabitRPG/src/data/sqlite_repository.cpp`
- `/Users/danielsinkin/GitHub_private/HabitRPG/src/data/migrations.cpp`

### 8.2 Contract Delta Validation (Locked Taxonomy)

| Contract Area | Status | Evidence | Notes |
| --- | --- | --- | --- |
| `action_unit_state` support (`ready|active|partial|missed|paused|completed`) | PASS | `/Users/danielsinkin/GitHub_private/HabitRPG/include/habitrpg/domain/entities.hpp:26`, `/Users/danielsinkin/GitHub_private/HabitRPG/src/domain/entities.cpp:73`, `/Users/danielsinkin/GitHub_private/HabitRPG/src/data/migrations.cpp:187` | Persisted via `runtime_state` and mapped lifecycle values |
| `learning_session_state` contract alignment | PARTIAL | `/Users/danielsinkin/GitHub_private/HabitRPG/include/habitrpg/domain/entities.hpp:66`, `/Users/danielsinkin/GitHub_private/HabitRPG/src/data/migrations.cpp:187` | Uses shared `lifecycle_state` set; does not implement separate `idle/abandoned` model |
| Checkpoint candidate behavior exists | PASS | `/Users/danielsinkin/GitHub_private/HabitRPG/src/domain/interaction_flow.cpp:183`, `/Users/danielsinkin/GitHub_private/HabitRPG/src/ui/dockspace_shell.cpp:412`, `/Users/danielsinkin/GitHub_private/HabitRPG/tests/state_transition_tests.cpp:75` | Candidate state + note write are implemented |
| Candidate promote/reject lifecycle (`candidate|confirmed|rejected`) | FAIL | `/Users/danielsinkin/GitHub_private/HabitRPG/src/domain/interaction_flow.cpp:183`, `/Users/danielsinkin/GitHub_private/HabitRPG/src/data/migrations.cpp:183` | No confirmed/rejected state machine or dedicated checkpoint table |
| Preset model (`calm|spark|custom`) | FAIL | `/Users/danielsinkin/GitHub_private/HabitRPG/include/habitrpg/ui/contracts.hpp:42`, `/Users/danielsinkin/GitHub_private/HabitRPG/src/ui/dockspace_shell.cpp:524` | Two booleans still used (`calm_mode`, `spark_mode`); no `custom` mode enum/state |

### 8.3 QA Matrix Re-Run (PASS / PARTIAL / FAIL)

| QA ID | Status | Evidence Paths | Notes |
| --- | --- | --- | --- |
| `QA-LIFE-ADD-001` | FAIL | `/Users/danielsinkin/GitHub_private/HabitRPG/src/ui/dockspace_shell.cpp:157` | UI creates life `ActionUnit` only; no Habit entity creation flow in Today panel |
| `QA-LIFE-START-001` | PASS | `/Users/danielsinkin/GitHub_private/HabitRPG/src/ui/dockspace_shell.cpp:296`, `/Users/danielsinkin/GitHub_private/HabitRPG/src/domain/interaction_flow.cpp:96`, `/Users/danielsinkin/GitHub_private/HabitRPG/src/app/application.cpp:170` | Start transitions to active, enforces single-active semantics, persists mutation |
| `QA-LIFE-COMPLETE-001` | PARTIAL | `/Users/danielsinkin/GitHub_private/HabitRPG/src/ui/dockspace_shell.cpp:429`, `/Users/danielsinkin/GitHub_private/HabitRPG/src/domain/interaction_flow.cpp:124`, `/Users/danielsinkin/GitHub_private/HabitRPG/src/data/sqlite_repository.cpp:582` | Completion + reward persistence works; outcome sheet (`full/partial/could_not_finish`) not implemented |
| `QA-LIFE-RECOVER-001` | PARTIAL | `/Users/danielsinkin/GitHub_private/HabitRPG/src/ui/dockspace_shell.cpp:380` | Missed state button exists; no dedicated missed-day recovery banner/plan builder |
| `QA-LEARN-ADD-001` | PARTIAL | `/Users/danielsinkin/GitHub_private/HabitRPG/src/ui/dockspace_shell.cpp:173`, `/Users/danielsinkin/GitHub_private/HabitRPG/src/ui/dockspace_shell.cpp:197` | Goal creation exists; first session is not auto-created from goal create action |
| `QA-LEARN-START-001` | PASS | `/Users/danielsinkin/GitHub_private/HabitRPG/src/ui/dockspace_shell.cpp:304`, `/Users/danielsinkin/GitHub_private/HabitRPG/src/domain/interaction_flow.cpp:155`, `/Users/danielsinkin/GitHub_private/HabitRPG/tests/state_transition_tests.cpp:70` | Learning session transitions active and pauses other active units |
| `QA-LEARN-COMPLETE-001` | PARTIAL | `/Users/danielsinkin/GitHub_private/HabitRPG/src/ui/dockspace_shell.cpp:444`, `/Users/danielsinkin/GitHub_private/HabitRPG/src/domain/interaction_flow.cpp:209`, `/Users/danielsinkin/GitHub_private/HabitRPG/src/data/sqlite_repository.cpp:437` | Completion + XP works; no end-of-session artifact capture panel at completion point |
| `QA-LEARN-CHECK-001` | PARTIAL | `/Users/danielsinkin/GitHub_private/HabitRPG/src/ui/dockspace_shell.cpp:414`, `/Users/danielsinkin/GitHub_private/HabitRPG/src/domain/interaction_flow.cpp:200`, `/Users/danielsinkin/GitHub_private/HabitRPG/tests/state_transition_tests.cpp:79` | Candidate checkpoint implemented; confirm/reject and milestone reward gating not implemented |
| `QA-PRESET-001` | PARTIAL | `/Users/danielsinkin/GitHub_private/HabitRPG/src/ui/dockspace_shell.cpp:524`, `/Users/danielsinkin/GitHub_private/HabitRPG/src/ui/dockspace_shell.cpp:76`, `/Users/danielsinkin/GitHub_private/HabitRPG/include/habitrpg/ui/contracts.hpp:86` | Calm visual tokens apply; calm default bundle rules are not auto-enforced |
| `QA-PRESET-002` | PARTIAL | `/Users/danielsinkin/GitHub_private/HabitRPG/src/ui/dockspace_shell.cpp:526`, `/Users/danielsinkin/GitHub_private/HabitRPG/include/habitrpg/ui/contracts.hpp:98`, `/Users/danielsinkin/GitHub_private/HabitRPG/src/ui/dockspace_shell.cpp:456` | Spark visual/reward tier behavior exists; bounded nudge rules are missing |
| `QA-PRESET-003` | FAIL | `/Users/danielsinkin/GitHub_private/HabitRPG/include/habitrpg/ui/contracts.hpp:46`, `/Users/danielsinkin/GitHub_private/HabitRPG/src/ui/dockspace_shell.cpp:525` | Calm and Spark can be enabled simultaneously |
| `QA-PRESET-004` | FAIL | `/Users/danielsinkin/GitHub_private/HabitRPG/src/app/application.cpp:116`, `/Users/danielsinkin/GitHub_private/HabitRPG/src/app/application.cpp:170`, `/Users/danielsinkin/GitHub_private/HabitRPG/src/data/sqlite_repository.cpp:636` | UI preferences are neither loaded nor saved in persistence layer |
| `QA-QUEUE-MIXED-001` | PASS | `/Users/danielsinkin/GitHub_private/HabitRPG/src/domain/today_queue.cpp:140`, `/Users/danielsinkin/GitHub_private/HabitRPG/tests/queue_tests.cpp:68` | Mixed queue composes life+learning with deterministic alternation while both exist |
| `QA-QUEUE-LIFE-001` | PASS | `/Users/danielsinkin/GitHub_private/HabitRPG/src/domain/today_queue.cpp:148`, `/Users/danielsinkin/GitHub_private/HabitRPG/src/ui/dockspace_shell.cpp:150`, `/Users/danielsinkin/GitHub_private/HabitRPG/tests/queue_tests.cpp:56` | Life-only filter returns life queue only |
| `QA-QUEUE-LEARN-001` | PASS | `/Users/danielsinkin/GitHub_private/HabitRPG/src/domain/today_queue.cpp:153`, `/Users/danielsinkin/GitHub_private/HabitRPG/src/ui/dockspace_shell.cpp:151` | Learning-only filter returns learning queue only |
| `QA-QUEUE-PERSIST-001` | FAIL | `/Users/danielsinkin/GitHub_private/HabitRPG/include/habitrpg/ui/contracts.hpp:44`, `/Users/danielsinkin/GitHub_private/HabitRPG/src/app/application.cpp:116` | Track filter persists only in memory; resets to default on restart |
| `QA-QUEUE-EMPTY-001` | PARTIAL | `/Users/danielsinkin/GitHub_private/HabitRPG/src/ui/dockspace_shell.cpp:253` | Generic empty message present; no targeted empty copy per mode |

### 8.4 Queue Behavior Acceptance Re-Check

| Test ID | Status | Evidence |
| --- | --- | --- |
| `QA-QUEUE-MIXED-001` | PASS | `/Users/danielsinkin/GitHub_private/HabitRPG/tests/queue_tests.cpp:83` |
| `QA-QUEUE-LIFE-001` | PASS | `/Users/danielsinkin/GitHub_private/HabitRPG/tests/queue_tests.cpp:56` |
| `QA-QUEUE-LEARN-001` | PASS | `/Users/danielsinkin/GitHub_private/HabitRPG/src/domain/today_queue.cpp:153` |
| `QA-QUEUE-PERSIST-001` | FAIL | `/Users/danielsinkin/GitHub_private/HabitRPG/src/app/application.cpp:190` |
| `QA-QUEUE-EMPTY-001` | PARTIAL | `/Users/danielsinkin/GitHub_private/HabitRPG/src/ui/dockspace_shell.cpp:255` |

### 8.5 Preset Verification and Signoff (Round 3)

| Verification Target | Result |
| --- | --- |
| `calm` / `spark` / `custom` mutually exclusive | FAIL |
| Preset persistence across restarts | FAIL |
| Low-cognitive-load copy quality under all presets | PASS |
| Visual/motion differentiation between calm and spark | PARTIAL |

Signoff:
- Preset behavior signoff is **not granted** due failed exclusivity and persistence requirements.

### 8.6 Implementation Gaps Remaining

1. No dedicated persisted `ui_preferences` model for preset mode and sensory controls.
2. No checkpoint confirmation/rejection lifecycle or dedicated `milestone_checkpoints` persistence.
3. No habit creation flow in Today panel (only direct life action creation).
4. No completion-sheet variant capture for life/learning completion outcomes.

### 8.7 Blockers

1. **Owner: Engineer**  
Need persistence contract implementation for UI preferences (`preset_mode`, `motion`, `sound`, `density`, `track_filter`) before final preset signoff can be issued.

2. **Owner: Engineer**  
Need milestone checkpoint storage and lifecycle (`candidate -> confirmed/rejected`) to complete locked contract validation.
