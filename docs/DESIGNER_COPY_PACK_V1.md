# HabitRPG Interaction Copy Pack v1

Date: February 19, 2026  
Owner: Designer Agent  
Tone goals: non-shaming, direct, low-cognitive-load, action-first.

## 1) Empty States

### 1.1 Today Empty

Primary:
- "Nothing is queued yet."

Secondary:
- "Pick one small action to start momentum."

Actions:
- "Add Habit"
- "Add Learning Goal"

### 1.2 Habits Empty

Primary:
- "No habits yet."

Secondary:
- "Start with a 2-minute habit."

Action:
- "Create Habit"

### 1.3 Learning Empty

Primary:
- "No learning goals yet."

Secondary:
- "Add one C++ milestone to begin."

Action:
- "Create Learning Goal"

## 2) Recovery States

### 2.1 Missed Day Banner

Primary:
- "You are back. Let’s restart clean."

Secondary:
- "No penalties. Choose the easiest way forward."

Actions:
- "Quick Restart"
- "Review Backlog"
- "Light Day Plan"

### 2.2 Missed Task Card

Primary:
- "This task was missed."

Secondary:
- "Choose how to handle it."

Actions:
- "Reschedule"
- "Split Smaller"
- "Drop for Today"

## 3) Conflict States

### 3.1 Active Session Conflict

Primary:
- "A session is already active."

Secondary:
- "Do you want to switch or continue?"

Actions:
- "Continue Current"
- "Pause and Switch"
- "Cancel"

### 3.2 Duplicate Learning Goal Title

Primary:
- "A similar goal already exists."

Secondary:
- "Keep one goal or create a separate track."

Actions:
- "Merge Goals"
- "Keep Separate"
- "Cancel"

## 4) Completion States

### 4.1 Life Completion

Toast:
- "Action complete. XP added."

Inline:
- "Nice progress. Ready for the next step?"

Actions:
- "Start Next"
- "Done for Now"

### 4.2 Learning Session Completion

Toast:
- "Session complete. Learning XP added."

Inline:
- "Capture a quick output to lock in memory."

Actions:
- "Add Note"
- "Add Snippet"
- "Skip for Now"

### 4.3 Milestone Confirmed

Toast:
- "Milestone confirmed."

Inline:
- "Great evidence. Set the next target?"

Actions:
- "Set Next Milestone"
- "Later"

### 4.4 Milestone Candidate Saved

Toast:
- "Checkpoint saved as candidate."

Inline:
- "You can confirm it after one more pass."

Actions:
- "Review Candidate"
- "Continue Learning"

## 5) Error States

### 5.1 Save Failure (Generic)

Primary:
- "We could not save that change."

Secondary:
- "Your input is still here."

Actions:
- "Retry Save"
- "Cancel"

### 5.2 Session Completion Save Failure

Primary:
- "Completion was not saved yet."

Secondary:
- "Retry now or keep your draft."

Actions:
- "Retry"
- "Keep Draft"
- "Cancel"

### 5.3 Sync/Stale State

Primary:
- "This item changed in the background."

Secondary:
- "Refresh to continue with the latest state."

Actions:
- "Refresh"
- "Close"

## 6) Low-Energy and Pause Copy

### 6.1 Low-Energy Mode

Inline note:
- "Low-energy mode is on. Small wins first."

Action hint:
- "Start 2-minute version"

### 6.2 Paused Session

Primary:
- "Session paused."

Secondary:
- "Resume when you are ready."

Actions:
- "Resume"
- "End Session"

## 7) Stuck Learning Copy

### 7.1 Stuck Banner

Primary:
- "This goal looks stuck."

Secondary:
- "Try a smaller step or change session mode."

Actions:
- "Reduce Scope"
- "Switch to Review"
- "Recovery Session"

### 7.2 Artifact-Light Prompt

Prompt:
- "Add a one-line takeaway (optional)."

### 7.3 Artifact-Rich Prompt

Prompt:
- "Add a snippet or solved exercise to mark concrete progress."

## 8) Retention Prompt Copy

Question A:
- "What was the key concept?"

Question B:
- "Where will you use it next?"

Actions:
- "Save Reflection"
- "Skip"

## 9) Copy Rules

1. Keep strings <= 90 characters where possible.
2. Lead with state clarity, then next action.
3. Avoid judgmental language (`failed`, `behind`, `missed streak`).
4. Prefer "next step" framing over "fix what went wrong."
5. Always provide at least one low-friction action.

## 10) Keyed Format v2 (2026-02-19)

This section is canonical for implementation wiring.  
Use `message_key` + fallback text; avoid hard-coded UI strings.

### 10.1 Key Table (High-Frequency Strings)

| Message Key | Fallback Text | Surface |
| --- | --- | --- |
| `onboarding.welcome.primary` | `Start tiny to build momentum.` | First-run welcome card |
| `onboarding.welcome.cta_start_tiny` | `Start tiny` | First-run CTA |
| `onboarding.welcome.cta_customize` | `Customize setup` | First-run CTA |
| `today.empty.primary` | `Nothing is queued yet.` | Today empty state |
| `today.empty.secondary` | `Pick one small action to start momentum.` | Today empty state |
| `today.empty.cta_add_habit` | `Add Habit` | Today empty state |
| `today.empty.cta_add_learning_goal` | `Add Learning Goal` | Today empty state |
| `habits.empty.primary` | `No habits yet.` | Habits empty state |
| `habits.empty.secondary` | `Start with a 2-minute habit.` | Habits empty state |
| `habits.empty.cta_create` | `Create Habit` | Habits empty state |
| `learning.empty.primary` | `No learning goals yet.` | Learning empty state |
| `learning.empty.secondary` | `Add one C++ milestone to begin.` | Learning empty state |
| `learning.empty.cta_create` | `Create Learning Goal` | Learning empty state |
| `queue.empty.filtered` | `Queue is empty for this track filter.` | Today queue empty state |
| `recovery.missed_day.primary` | `You are back. Let’s restart clean.` | Missed-day banner |
| `recovery.missed_day.secondary` | `No penalties. Choose the easiest way forward.` | Missed-day banner |
| `recovery.missed_day.cta_quick_restart` | `Quick Restart` | Missed-day banner |
| `recovery.missed_day.cta_review_backlog` | `Review Backlog` | Missed-day banner |
| `recovery.missed_day.cta_light_day` | `Light Day Plan` | Missed-day banner |
| `recovery.missed_task.primary` | `This task was missed.` | Missed-task row |
| `recovery.missed_task.secondary` | `Choose how to handle it.` | Missed-task row |
| `recovery.missed_task.cta_reschedule` | `Reschedule` | Missed-task row |
| `recovery.missed_task.cta_split` | `Split Smaller` | Missed-task row |
| `recovery.missed_task.cta_drop` | `Drop for Today` | Missed-task row |
| `conflict.active_session.primary` | `A session is already active.` | Start conflict dialog |
| `conflict.active_session.secondary` | `Do you want to switch or continue?` | Start conflict dialog |
| `conflict.active_session.cta_continue` | `Continue Current` | Start conflict dialog |
| `conflict.active_session.cta_switch` | `Pause and Switch` | Start conflict dialog |
| `conflict.active_session.cta_cancel` | `Cancel` | Start conflict dialog |
| `completion.life.toast` | `Action complete. XP added.` | Life completion feedback |
| `completion.life.inline` | `Nice progress. Ready for the next step?` | Life completion feedback |
| `completion.learning.toast` | `Session complete. Learning XP added.` | Learning completion feedback |
| `completion.learning.inline` | `Capture a quick output to lock in memory.` | Learning completion feedback |
| `completion.milestone.confirmed.toast` | `Milestone confirmed.` | Milestone checkpoint feedback |
| `completion.milestone.candidate.toast` | `Checkpoint saved as candidate.` | Milestone checkpoint feedback |
| `error.save.generic.primary` | `We could not save that change.` | Save error banner |
| `error.save.generic.secondary` | `Your input is still here.` | Save error banner |
| `error.save.generic.cta_retry` | `Retry Save` | Save error banner |
| `error.save.generic.cta_cancel` | `Cancel` | Save error banner |
| `error.completion.primary` | `Completion was not saved yet.` | Completion error banner |
| `error.completion.secondary` | `Retry now or keep your draft.` | Completion error banner |
| `error.sync.primary` | `This item changed in the background.` | Stale state banner |
| `error.sync.secondary` | `Refresh to continue with the latest state.` | Stale state banner |
| `energy.low.inline` | `Low-energy mode is on. Small wins first.` | Low-energy hint |
| `energy.low.cta_start_small` | `Start 2-minute version` | Low-energy hint |
| `session.paused.primary` | `Session paused.` | Paused session state |
| `session.paused.secondary` | `Resume when you are ready.` | Paused session state |
| `session.paused.cta_resume` | `Resume` | Paused session state |
| `session.paused.cta_end` | `End Session` | Paused session state |
| `learning.stuck.primary` | `This goal looks stuck.` | Stuck banner |
| `learning.stuck.secondary` | `Try a smaller step or change session mode.` | Stuck banner |
| `learning.stuck.cta_reduce_scope` | `Reduce Scope` | Stuck banner |
| `learning.stuck.cta_switch_review` | `Switch to Review` | Stuck banner |
| `learning.stuck.cta_recovery_session` | `Recovery Session` | Stuck banner |
| `learning.artifact_light.prompt` | `Add a one-line takeaway (optional).` | Learning completion sheet |
| `learning.artifact_rich.prompt` | `Add a snippet or solved exercise to mark concrete progress.` | Learning completion sheet |
| `retention.question.key_concept` | `What was the key concept?` | Retention prompt |
| `retention.question.next_use` | `Where will you use it next?` | Retention prompt |
| `retention.cta_save` | `Save Reflection` | Retention prompt |
| `retention.cta_skip` | `Skip` | Retention prompt |

### 10.2 Deprecated String Map

Deprecated plain strings should be removed from direct UI wiring and replaced by keys.

| Deprecated Plain String | Replacement Key |
| --- | --- |
| `Nothing is queued yet.` | `today.empty.primary` |
| `Queue is empty for this track filter.` | `queue.empty.filtered` |
| `You are back. Let’s restart clean.` | `recovery.missed_day.primary` |
| `Action complete. XP added.` | `completion.life.toast` |
| `Session complete. Learning XP added.` | `completion.learning.toast` |
| `Checkpoint saved as candidate.` | `completion.milestone.candidate.toast` |
| `We could not save that change.` | `error.save.generic.primary` |
| `Low-energy mode is on. Small wins first.` | `energy.low.inline` |
| `This goal looks stuck.` | `learning.stuck.primary` |
| `What was the key concept?` | `retention.question.key_concept` |

### 10.3 Engineer Migration Guidance

1. Add a message lookup helper:
- `std::string ResolveMessage(std::string_view key, std::string_view fallback)`
2. UI components should reference keys, not literal strings.
3. For Round 3, migrate high-frequency paths first:
- Today queue/empty state
- completion toasts
- error banners
- low-energy/stuck banners
4. Keep fallback text local to callsite until localization storage is introduced.
5. Add a lint/check that flags hard-coded strings for components covered in section 10.1.
