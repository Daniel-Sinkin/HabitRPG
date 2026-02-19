# HabitRPG Round 3 Onboarding + Insights UX Spec

Date: February 19, 2026  
Owner: Designer Agent  
Scope: Onboarding + first-week loop and Insights screen v1 interaction model.

## 1) Goals

1. Get user from cold start to first life completion in under 2 minutes.
2. Get user to first C++ learning session completion in the same first-run flow.
3. Prevent day-2/day-3 drop-off with low-friction recovery paths.
4. Make Insights actionable, non-judgmental, and directly tied to next actions.

## 2) First-Run Onboarding Flow

### 2.1 Flow: Cold Start -> First Life Completion

Start state:
1. New launch with empty DB.
2. `Today` route active.

Sequence:
1. Show `WelcomeCard` with two CTA options:
- `Start tiny (recommended)`
- `Customize setup`
2. `Start tiny` path:
- auto-select `calm` preset
- open `QuickLifeActionComposer`
- prompt: one title field + priority slider
3. On submit:
- create one life action in `ready`
- queue refresh to place it in top slot
4. Prompt immediate action:
- primary CTA `Start now`
5. On `Start now`:
- transition to `active`
- session strip expands
6. On `Complete`:
- transition to `completed`
- instant XP confirmation
- show next CTA `Try first C++ session`

Success state:
1. At least one life action completed.
2. XP updated and persisted.

Failure state:
1. Empty title submission.
2. Persistence write failure.

Recovery:
1. Keep composer open with inline validation.
2. Keep typed input in memory until successful write.

### 2.2 Flow: First C++ Session Completion

Start state:
1. First life completion done.
2. No active learning session.

Sequence:
1. Show `QuickLearningSetupCard`:
- Goal title
- Milestone text
- Session title (default prefilled)
- Duration preset (10/25/45)
2. Submit:
- create learning goal
- create first learning session
- queue refresh and highlight learning row
3. `Start Session`:
- session enters `active`
4. `Complete`:
- open lightweight completion sheet with two variants:
- artifact-light (optional one-line takeaway)
- artifact-rich (note/snippet/exercise)
5. Submit:
- session enters `completed`
- reward event emitted
- prompt `Done for now` or `Continue`

Success state:
1. First learning session completed with optional artifact.

Failure state:
1. Missing required goal/milestone.
2. Session completion save failure.

Recovery:
1. Keep form/sheet data in draft until save succeeds.
2. Allow fallback to artifact-light with one click.

## 3) First-Week Experience Model

### 3.1 Day 0 (Setup Day)

Targets:
1. One life completion.
2. One learning session completion.

UX posture:
1. Minimize choice count.
2. Always expose one clear next action.

### 3.2 Day 1 (Reinforcement)

Targets:
1. Repeat one life action.
2. Repeat one learning session.

UX behavior:
1. Prefill last used session duration and artifact type.
2. Show "resume momentum" banner.

### 3.3 Day 2 and Day 3 (Drop-Off Risk Window)

Drop-off detection:
1. No completion in prior 24h -> day-2 recovery prompt.
2. No completion in prior 48h -> day-3 light plan prompt.

Recovery options:
1. `Quick Restart`:
- one 2-minute life action + one 10-minute learning session template
2. `Light Day Plan`:
- low-energy queue only
3. `Backlog Sweep`:
- mark missed, split, or reschedule in batch

Rules:
1. No punitive copy.
2. No hard streak reset messaging.
3. Recovery CTAs always include lowest-friction option first.

### 3.4 Day 4-7 (Stability Loop)

Targets:
1. Minimum one completion/day.
2. At least three learning sessions by week end.

UX behavior:
1. Show weekly progress band in right panel.
2. Prompt weekly review at day 7 after first completion.

## 4) Insights Screen v1 Interaction Model

### 4.1 Information Architecture

Top row cards:
1. `Life Completion Rate (7d)`
2. `Learning Sessions (7d)`
3. `Recovery Response Rate (7d)`
4. `Artifact Capture Rate (7d)`

Middle section:
1. Life trend chart (daily completions)
2. Learning trend chart (sessions + checkpoint candidates)
3. Friction tags summary

Bottom section:
1. Recommendation cards (max 3 visible)
2. `Start Weekly Review` CTA

### 4.2 Actionable Metric Definitions

Life metrics:
1. Completion rate (7d) = completed life units / scheduled life units.
2. Missed-to-recovered ratio = missed life units with follow-up completion within 48h / total missed life units.

Learning metrics:
1. Sessions completed (7d).
2. Artifact capture rate = sessions with artifact kind != `none` / completed learning sessions.
3. Checkpoint conversion rate = confirmed checkpoints / candidate checkpoints.
4. Stuck duration median (days).

Cross-track:
1. Balance index = abs(life completions - learning sessions) / max(1, total completions).

### 4.3 Weekly Review Flow

Trigger:
1. First completion on day 7 or manual open from Insights.

Steps:
1. `What worked this week?` (single line optional)
2. `What felt heavy?` (select friction tags)
3. Auto-summary card with neutral language.
4. Choose one next-week adjustment:
- reduce scope
- keep current cadence
- stretch slightly
5. Confirm and apply.

Output:
1. Persist selected adjustment and timestamp.
2. Generate one recommendation card for next session.

### 4.4 Recommendation Card Rules (Non-Judgmental)

Card constraints:
1. Max 3 cards visible.
2. Each card must include one action button.
3. Tone: suggestive, never corrective.

Card types:
1. `Scope Down`:
- Trigger: >=2 consecutive missed events.
2. `Capture Artifact`:
- Trigger: artifact capture rate < 40%.
3. `Rebalance Tracks`:
- Trigger: balance index > 0.6.
4. `Checkpoint Review`:
- Trigger: candidate checkpoints pending > 0.

Copy pattern:
1. Observation line.
2. Optional rationale line.
3. Single next-step CTA.

## 5) Component Contracts (Onboarding + Insights)

| Component Name | Trigger | Expected State Update | Edge Cases |
| --- | --- | --- | --- |
| `WelcomeCard` | First run | Route remains `Today`; onboarding mode enabled | If seeded data exists, skip first-run card |
| `QuickLifeActionComposer` | Submit valid title | Create life action in `ready`; queue refresh | Empty title keeps form open with inline message |
| `QuickLearningSetupCard` | Submit valid goal+milestone | Create learning goal + first learning session | If goal already exists, offer merge/keep separate |
| `OnboardingProgressStepper` | Any onboarding milestone reached | Increment onboarding step index | If user exits app, resume from last completed step |
| `DropoffRecoveryBanner` | No completion in 24h/48h window | Surface recovery mode choices | Dismiss should snooze, not disable permanently |
| `RecoveryModeSelector` | Choose quick/light/backlog mode | Apply queue mutation policy | Mutation failure should rollback and show retry |
| `InsightsMetricCard` | Insights open or data refresh | Recompute card values from persisted data | Insufficient data shows neutral placeholder text |
| `InsightsTrendChart` | Insights open | Render trend series for selected period | Missing days should render as zero, not gaps |
| `RecommendationCardStack` | Recommendation engine emits cards | Up to 3 cards shown sorted by urgency | Duplicate recommendation types collapse into one |
| `WeeklyReviewLauncher` | Day-7 trigger or manual click | Launch review flow modal | If review already completed this week, offer edit/view |
| `WeeklyReviewWizard` | Submit step data | Persist weekly reflection + next-week adjustment | Partial submission saves draft state |

## 6) Preset-Specific UX Rules for Onboarding/Insights

Calm:
1. Show only one recommendation card at a time.
2. Use compact chart labels and reduce color contrast variance.
3. Default onboarding path is `Start tiny`.

Spark:
1. Show up to three recommendation cards.
2. Highlight metric deltas with accent color.
3. Allow optional milestone nudge after review completion.

Custom:
1. Preserve user-selected motion/sound/density while reusing same flow steps.

## 7) Architect Ambiguities

1. Whether checkpoint conversion should count only confirmed checkpoints or include candidates.
2. Whether weekly review is mandatory after day 7 or can remain always optional.
3. Whether drop-off detection should use local timezone day boundaries or rolling 24h windows.
