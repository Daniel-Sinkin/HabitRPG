# HabitRPG Round 1 UX Specification

Date: February 19, 2026  
Owner: Designer Agent  
Scope: Round 1 implementation-ready design deliverables for life + learning MVP in Dear ImGui.

## 1) Scope and Acceptance

This spec covers:

1. Core flow specs for MVP actions (life and learning loops).
2. Docked layout specification (left nav, center do-next, right status, bottom session strip).
3. Interaction state behavior (empty, low-energy, failure, confirmation, learning transitions).
4. Neurodivergent presets (Calm and Spark).
5. Learning UX framing (session output capture and end-of-session retention prompt).
6. Component contracts in handoff format:
- component name
- trigger
- expected state update
- edge cases

## 2) Constraints and Shared Contracts

### 2.1 Platform and Interaction Constraints

1. UI must be implementable with Dear ImGui docking + standard widgets.
2. Single active timed unit at a time (either `ActionUnit` or `LearningSession`).
3. Low-friction defaults: key actions available in <=2 clicks from `Today`.
4. No punitive or shaming copy in failure/recovery states.

### 2.2 Stable UX Identifiers (for engineer integration)

```txt
track_type: life | learning
queue_mode: mixed | life_only | learning_only
energy_state: low | normal | high
preset_mode: calm | spark | custom

action_unit_state: ready | active | completed | partial | missed | paused
learning_goal_state: new | in_progress | stuck | milestone_ready | completed
learning_session_state: idle | active | completed | abandoned
```

### 2.3 Global Interaction Rules

1. `Today` always shows top "Do Next" items with a visible primary action.
2. Queue mode defaults to `mixed` and persists across restarts.
3. If both tracks have pending items in `mixed`, top 3 must include at least one item from each track.
4. Any hard error state must include a retry action in the same panel.

## 3) Docked Layout Specification

### 3.1 Region Sizing and Priority

| Region | Purpose | Default Width/Height | Min Size | Priority |
| --- | --- | --- | --- | --- |
| Left Navigation | View routing + track mode entry points | 220 px width | 180 px | High |
| Center Do-Next | Main execution surface | Fill remaining | 560 px width | Highest |
| Right Status | XP, energy, recovery, milestone context | 300 px width | 260 px | High |
| Bottom Session Strip | Active timer controls + closure | 124 px height | 92 px | Highest while active |

### 3.2 Left Navigation Panel

Order:

1. Today
2. Quests
3. Habits
4. Learning
5. Character
6. Insights
7. Settings

Behavior:

1. Each item includes icon + label.
2. Active route has stronger contrast boundary.
3. Optional numeric badge only for pending items; hide badges in Calm preset.
4. Keyboard focus order follows visual order.

### 3.3 Center Do-Next Panel

Hierarchy:

1. Header row:
- Queue mode segmented control (`mixed`, `life_only`, `learning_only`)
- Quick add actions (`+ Habit`, `+ Learning Goal`)
2. Top queue list:
- Show 1-3 priority cards
- Each card includes title, track pill, effort estimate, energy tag, primary CTA
3. Secondary backlog:
- Collapsed section with next 5 items
4. Inline status:
- Empty/failure/recovery messages rendered in-panel, not modal-first

Card CTA rules:

1. `ready` -> `Start`
2. `active` -> `Complete`
3. `partial` -> `Continue 2-min`
4. `missed` -> `Recover`

### 3.4 Right Status Panel

Order:

1. XP/Level progress block
2. Current quest chain
3. Energy selector (`low`, `normal`, `high`)
4. Focus mode indicator
5. Consistency band + recovery token count
6. Learning milestone progress
7. Gentle reminder block

Behavior:

1. Energy selector directly influences queue ranking in center panel.
2. Recovery controls appear only when missed-day or overload signals exist.
3. Learning milestone block is visible even in `life_only` mode for continuity.

### 3.5 Bottom Session Strip

Visible states:

1. Hidden when no active unit.
2. Expanded when `ActionUnit` or `LearningSession` is active.

Controls:

1. Timer display
2. Pause/Resume
3. Complete
4. Abandon (secondary, guarded confirm)
5. Done for now

Rules:

1. Strip owns timer authority for active units.
2. Starting a new unit while one is active invokes conflict resolver.

### 3.6 Dual-Track Switching Model

Queue mode behavior:

1. `mixed`:
- Compose top queue from both tracks.
- Guarantee at least one life and one learning item in top 3 if both tracks have pending work.
2. `life_only`:
- Filter queue, backlog, and quick actions to life items.
3. `learning_only`:
- Filter queue, backlog, and quick actions to learning items.

Mixed ranking (deterministic):

```txt
urgency_score =
  (overdue ? 40 : 0) +
  (due_today ? 20 : 0) +
  (energy_match ? 10 : 0) +
  (is_pinned ? 30 : 0) +
  (days_waiting * 3) +
  (learning_milestone_near ? 6 : 0)
```

Top 3 composition in `mixed`:

1. Slot 1: highest score overall.
2. Slot 2: highest score from opposite track of slot 1 (if available).
3. Slot 3: next highest remaining score overall.

## 4) Core Flow Specifications

Each flow includes explicit start, success, failure, and recovery states.

### 4.1 Add Habit (`FLOW-LIFE-ADD-HABIT`)

Start state:

1. User on `Today` or `Habits`.
2. No modal open.

Primary path:

1. Trigger `+ Habit`.
2. Open habit form modal with fields:
- Name (required)
- Cadence (daily/weekly/custom days)
- Energy tag (`low`, `normal`, `high`)
- Optional 2-minute starter text (default generated)
3. User selects `Save`.
4. System validates and creates `Habit`.
5. System auto-generates first `ActionUnit` if cadence includes today.
6. New habit appears in list; queue refreshes.

Success state:

1. Toast confirmation.
2. Focus moves to newly created habit row.

Failure states:

1. Missing name -> inline validation error.
2. Save failure -> in-modal error banner with retry.

Recovery states:

1. `Save Draft` preserved if modal closes unexpectedly.
2. Retry save without losing typed values.

### 4.2 Start Action Unit (`FLOW-LIFE-START-ACTION`)

Start state:

1. Selected item in `ready` or `partial`.
2. No other active timed unit.

Primary path:

1. Trigger `Start` from queue card.
2. System sets `action_unit_state=active`, `started_at=now`.
3. Bottom session strip expands with running timer.
4. Center card switches primary CTA to `Complete`.

Success state:

1. Active timer visible.
2. Focus mode indicator updates to `active`.

Failure states:

1. Another unit active -> conflict dialog.
2. Stale item state (already completed elsewhere) -> refresh + message.

Recovery states:

1. Conflict dialog offers:
- Resume current active
- Pause current and start selected
- Cancel
2. If low energy selected, offer `Start 2-min version`.

### 4.3 Complete Action Unit (`FLOW-LIFE-COMPLETE-ACTION`)

Start state:

1. `action_unit_state=active`.

Primary path:

1. Trigger `Complete`.
2. Open quick completion sheet:
- Full complete
- Partial complete
- Could not finish
3. User confirms one option.
4. System logs completion outcome and creates `RewardEvent`.
5. XP/level UI updates instantly in right panel.

Success state:

1. Life completion feedback shown.
2. Unit leaves active state.
3. Queue promotes next item.

Failure states:

1. Active unit missing (desync) -> recoverable error banner.
2. Persistence write failure -> keep completion sheet open, retry available.

Recovery states:

1. `Partial complete` creates smaller follow-up unit due same or next day.
2. `Could not finish` offers:
- Mark as missed
- Re-plan to 2-minute starter

### 4.4 Recover After Missed Day (`FLOW-LIFE-RECOVER-MISSED-DAY`)

Start state:

1. App open detects missed day condition:
- last active date older than yesterday
- pending overdue units exist

Primary path:

1. Show recovery banner in center top.
2. Offer 3 actions:
- Quick Restart (recommended)
- Review Backlog
- Light Day Plan
3. User selects one.
4. System applies non-punitive recovery policy:
- No streak reset penalty
- Mark older units as `missed` or `rescheduled` based on selection
- Build new top queue with low-load options

Success state:

1. Updated queue appears immediately.
2. Recovery confirmation message shown.

Failure states:

1. Recovery token unavailable for optional bonus path -> fallback allowed.
2. Update failure -> rollback and show retry action.

Recovery states:

1. If user dismisses banner, show compact reminder in right panel until a plan is selected.
2. User can switch from backlog review to quick restart without losing context.

### 4.5 Add C++ Learning Goal (`FLOW-LEARN-ADD-GOAL`)

Start state:

1. User on `Learning` view or `Today` quick add.

Primary path:

1. Trigger `+ Learning Goal`.
2. Open goal form with:
- Goal title (required)
- Milestone target (required)
- Session cadence (weekly frequency)
- Default session length (10/25/45 min)
- Optional 2-minute starter
3. Save goal.
4. System creates `LearningGoal` with `learning_goal_state=new`.
5. System generates first session suggestion.

Success state:

1. Goal card appears in Learning list.
2. Do-next queue refresh includes the first learning session.

Failure states:

1. Missing required fields -> inline errors.
2. Duplicate active title -> merge warning dialog.

Recovery states:

1. `Save Draft` stores unfinished goal.
2. Merge warning allows:
- Merge into existing goal
- Keep separate

### 4.6 Start Learning Session (`FLOW-LEARN-START-SESSION`)

Start state:

1. Goal in `new`, `in_progress`, or `stuck`.
2. No active timed unit.

Primary path:

1. Trigger `Start Session` on learning card.
2. Quick setup sheet:
- Session mode (read/build/review)
- Output preference (note/snippet/exercise)
- Duration preset (10/25/45)
3. Confirm start.
4. System creates or updates `LearningSession` with `learning_session_state=active`.
5. Bottom strip starts timer.
6. Goal transitions `new -> in_progress` on first successful start.

Success state:

1. Active learning strip with track accent.
2. Learning card moves to active state.

Failure states:

1. Another unit active -> conflict resolver.
2. Missing goal reference -> refresh and return to learning list.

Recovery states:

1. Conflict resolver offers pause current + start learning.
2. If energy is low, prompt for 10-minute version.

### 4.7 Complete Learning Session (`FLOW-LEARN-COMPLETE-SESSION`)

Start state:

1. `learning_session_state=active`.

Primary path:

1. Trigger `Complete`.
2. Open output capture panel with three tabs:
- Note
- Code Snippet
- Solved Exercise
3. User can submit artifact or select `No artifact this time`.
4. Optional confidence score (1-5) and friction tag.
5. Confirm completion.
6. System sets `learning_session_state=completed`, logs reward, updates mastery progress.

Success state:

1. Learning completion feedback shown.
2. Milestone progress block updates in right panel.

Failure states:

1. Save failure -> panel stays open with retry.
2. Invalid snippet format -> inline validation message only for snippet tab.

Recovery states:

1. Output draft autosaves while panel is open.
2. `No artifact this time` keeps flow low-friction and records artifact type `none`.

### 4.8 Checkpoint C++ Milestone (`FLOW-LEARN-CHECKPOINT`)

Start state:

1. Goal state `milestone_ready` or user-initiated checkpoint review.

Primary path:

1. Trigger `Checkpoint`.
2. Open checkpoint dialog:
- Milestone name
- Evidence input (note, path, or reference text)
- Confidence score (1-5)
3. Confirm checkpoint.
4. System logs milestone event, grants reward badge, updates goal progress.
5. Prompt `Set next milestone now?`

Success state:

1. Milestone unlock feedback appears.
2. Goal either advances to next milestone plan or remains in progress.

Failure states:

1. Empty milestone name -> inline error.
2. Save failure -> retry banner.

Recovery states:

1. `Mark as candidate` stores draft checkpoint if confidence is low.
2. Candidate can be promoted to confirmed later without retyping evidence.

## 5) State Behavior Specification

### 5.1 Empty States

`Today` empty:

1. Message: "Nothing queued yet. Add one tiny action to get momentum."
2. CTAs: `+ Habit`, `+ Learning Goal`
3. Show one-line explanation of mixed mode.

`Habits` empty:

1. Message: "No habits yet. Start with a 2-minute habit."
2. CTA: `Create Habit`

`Learning` empty:

1. Message: "No learning goals yet. Add one C++ milestone."
2. CTA: `Create Learning Goal`

### 5.2 Paused and Low-Energy States

Paused:

1. Active strip shows paused timer and `Resume` primary CTA.
2. Queue item keeps `paused` badge until resume or abandon.

Low-energy:

1. Energy selector set to `low` re-ranks queue toward:
- estimated <= 15 min
- low cognitive load
- 2-minute starters
2. Center panel shows supportive hint: "Low-energy mode active. Small wins only."

### 5.3 Failure States

Missed task:

1. State changes to `missed` with neutral wording.
2. Recovery options:
- Reschedule
- Split smaller
- Drop for today

Partial completion:

1. State changes to `partial`.
2. Auto-suggests follow-up micro-unit.

Persistence/runtime error:

1. Inline panel banner with retry.
2. Keep user input intact.

### 5.4 Confirmation States

Completion confirmations:

1. Life completion:
- Brief pulse + toast text
2. Learning completion:
- Brief pulse + mastery increment indicator
3. Milestone unlock:
- Stronger but bounded effect, then static badge

Rules:

1. Max one major celebration effect per completion.
2. Calm mode suppresses animated effects, keeps static confirmation.

### 5.5 Learning State Transitions

| Current | Trigger | Next | UI Response |
| --- | --- | --- | --- |
| `new` | Start first session | `in_progress` | Goal card shows progress meter |
| `in_progress` | 3 missed planned sessions or manual mark stuck | `stuck` | Show recovery CTA (`Reduce scope`) |
| `stuck` | Start session or adjust plan | `in_progress` | Replace warning with active plan |
| `in_progress` | Milestone criteria met | `milestone_ready` | Checkpoint CTA highlighted |
| `milestone_ready` | Confirm checkpoint | `in_progress` or `completed` | Reward + next milestone prompt |
| `in_progress` | Final milestone complete | `completed` | Goal archived to completed list |

## 6) Neurodivergent Presets

### 6.1 Calm Preset

Defaults:

1. Motion: Off
2. Sound: Off
3. Density: Spacious
4. Prompt concurrency: 1 prompt max at a time
5. Queue display: focus card + collapsed remainder
6. Visual simplification:
- no particle or flash effects
- minimal badges
- low-saturation accent usage

Behavior rules:

1. No auto-opening celebratory modals.
2. Reminders render as quiet inline text, not popups.
3. Suggest 2-minute or 10-minute options first.

### 6.2 Spark Preset

Defaults:

1. Motion: Low
2. Sound: Minimal
3. Density: Comfortable
4. Prompt concurrency: up to 2 prompts
5. Queue display: top 3 fully visible
6. Richer feedback visuals allowed with bounds

Bounded stimulation rules:

1. Celebration cooldown: >= 30 seconds between high-intensity effects.
2. Novelty nudge cap: 2 per hour.
3. If user dismisses 2 nudges in a row, suspend nudges for current day.
4. Auto-downgrade to low-intensity effects when energy state is `low`.

### 6.3 Preset Persistence

1. Preset applies globally across panels and restarts.
2. Any manual override changes mode to `custom` while preserving last base preset.

## 7) Learning UX Framing

### 7.1 Session Output Capture

Output types:

1. Note
2. Code snippet
3. Solved exercise

Minimum friction rules:

1. Artifact is encouraged, not required.
2. `No artifact this time` is always available.
3. Draft persists during completion panel lifetime.

### 7.2 End-of-Session Retention Prompt

Prompt sequence after learning completion:

1. Question A: "What was the key concept?"
2. Question B: "Where will you use it next?"

Rules:

1. Single-line answers supported.
2. Skip allowed in one click.
3. If skipped, show optional reminder in next learning session start.

## 8) Component Behavior Contracts

| Component Name | Trigger | Expected State Update | Edge Cases |
| --- | --- | --- | --- |
| `NavItemButton` | User clicks nav item | `active_route` updated | Disabled if route unavailable |
| `QueueModeSegmentedControl` | User selects mode | `queue_mode` updated + persisted | If filtered mode has no items, show empty state CTA |
| `QuickAddHabitButton` | Click `+ Habit` | Open habit modal | Ignore if another modal lock is active |
| `HabitFormModal` | Save | Create `Habit` + optional first `ActionUnit` | Validation errors must remain inline |
| `DoNextCard` | Card rendered in queue | Reflect current unit/session state | Stale data refresh if entity missing |
| `StartActionButton` | Click `Start` | `action_unit_state=active`; timer start | Conflict resolver if active unit exists |
| `TwoMinuteStartButton` | Click `Start 2-min` | Spawn/activate downsized unit | If template missing, generate default label |
| `SessionTimerStrip` | Tick/pause/resume | Active timer state updated | Timer drift corrected on app refocus |
| `CompleteActionButton` | Click `Complete` | Open completion sheet | Disabled if no active action |
| `ActionCompletionSheet` | Confirm outcome | Set `completed/partial/missed`; log reward if applicable | Persistence failure keeps sheet open with retry |
| `RecoveryBanner` | Missed-day condition true | Banner visible in center panel | Dismiss stores snooze until next launch |
| `QuickRestartButton` | Click on recovery banner | Apply recovery plan and rebuild queue | If apply fails, rollback and show retry |
| `QuickAddLearningGoalButton` | Click `+ Learning Goal` | Open learning goal modal | Modal lock conflict handling |
| `LearningGoalFormModal` | Save | Create `LearningGoal` in `new` state | Duplicate title prompts merge choice |
| `StartLearningSessionButton` | Click `Start Session` | `learning_session_state=active`; timer start | Conflict resolver if another active unit |
| `LearningOutputCapturePanel` | Input in note/snippet/exercise tabs | Update session draft output | Snippet validation only on snippet tab |
| `CompleteLearningSessionButton` | Confirm completion | `learning_session_state=completed`; reward + mastery update | Can submit with artifact `none` |
| `MilestoneCheckpointButton` | Click `Checkpoint` | Open checkpoint dialog | Disabled unless `milestone_ready` or manual override |
| `MilestoneCheckpointDialog` | Confirm | Milestone event logged; badge unlocked | Save failure leaves dialog open |
| `EnergySelector` | User selects low/normal/high | `energy_state` updated + queue re-rank | Persist user setting across sessions |
| `PresetModeToggle` | User selects Calm/Spark | Preset values applied globally | Manual override sets `preset_mode=custom` |
| `ErrorBannerRetryButton` | User clicks retry | Re-executes failed action | Disable during retry in-flight state |
| `DoneForNowButton` | Click in bottom strip | End active session, open closure summary | If active unsaved output exists, confirm before close |

## 9) Engineer Feasibility Notes (ImGui)

1. All required interactions map to standard ImGui primitives:
- docking windows
- buttons
- input text
- combo/selectors
- progress bars
- child regions
- modal popups
2. No dependency on custom gesture systems or complex animated timelines.
3. Feedback animations can be implemented as short-lived alpha/color transitions and can be fully disabled.
4. Single-active-unit rule simplifies state conflict handling and timer ownership.

## 10) Architect Decision Flags

Non-blocking policy knobs that should be considered architect-level if changed:

1. Mixed mode composition guarantee (at least one item from each track in top 3).
2. Missed-day recovery default action (`Quick Restart` recommended).
3. Milestone checkpoint evidence strictness (free text allowed in Round 1).

## 11) Round 1 Test Checklist (Design QA)

1. User can reach first completion from launch in under 2 minutes.
2. Every core flow has visible start, success, failure, and recovery handling.
3. Low-energy mode materially reduces queue difficulty.
4. Calm/Spark presets change behavior predictably and persist.
5. Learning session completion works with and without artifact.
6. Mixed/life-only/learning-only switching updates queue and empty states correctly.
