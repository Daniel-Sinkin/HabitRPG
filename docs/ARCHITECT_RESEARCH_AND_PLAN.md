# HabitRPG Architect Research and Execution Plan

Date: February 19, 2026  
Role: Architect Agent (research + orchestration)

## 1) SOTA Research Snapshot (ADHD, Autism, AuDHD)

### 1.1 What current evidence supports

1. Gamification can help, but effects are usually small-to-moderate and context-dependent.
- A 2024 umbrella-level review in *EClinicalMedicine* found gamification is associated with slight improvements in health and well-being outcomes, with stronger effects in some behavior domains and weaker effects in others.
- Source: [PubMed 38157598](https://pubmed.ncbi.nlm.nih.gov/38157598/)

2. Sustained engagement depends more on motivation quality than points alone.
- A 2024 review integrating Self-Determination Theory reports that autonomy, competence, and relatedness support are key to lasting engagement; purely extrinsic reward loops are fragile.
- Source: [PubMed 39088580](https://pubmed.ncbi.nlm.nih.gov/39088580/)

3. ADHD design should prioritize immediacy and reduced delay.
- Meta-analytic evidence shows delay aversion and altered reward timing sensitivity are relevant in ADHD; immediate, frequent feedback is generally more effective than delayed rewards.
- Sources: [PubMed 38718701](https://pubmed.ncbi.nlm.nih.gov/38718701/), [PubMed 36462514](https://pubmed.ncbi.nlm.nih.gov/36462514/)

4. ADHD app ecosystem quality is mixed; many apps are not evidence-grounded.
- Reviews of ADHD apps show wide variability in quality, privacy, and clinical grounding. Product design should avoid assuming existing app conventions are best practice.
- Sources: [PubMed 33334797](https://pubmed.ncbi.nlm.nih.gov/33334797/), [PubMed 39996431](https://pubmed.ncbi.nlm.nih.gov/39996431/)

5. Autism-oriented gamified interventions can improve social and cognitive outcomes, but heterogeneity is high.
- Meta-analytic results indicate positive signal for some outcomes, with variation across intervention types and participants.
- Source: [PubMed 38492575](https://pubmed.ncbi.nlm.nih.gov/38492575/)

6. AuDHD-specific intervention evidence is still sparse.
- Comorbid ASD+ADHD guidance exists, but high-quality direct AuDHD-targeted gamified-product evidence is limited. Practical design should combine validated ADHD and autism accommodations with strong personalization.
- Source: [PubMed 32867686](https://pubmed.ncbi.nlm.nih.gov/32867686/)

7. Adult neurodivergent digital intervention work is emerging, not mature.
- Early RCT and internet-based interventions in autistic/ADHD adults show feasibility and selective gains, but not a single universal protocol.
- Sources: [PubMed 38743034](https://pubmed.ncbi.nlm.nih.gov/38743034/), [PubMed 39208285](https://pubmed.ncbi.nlm.nih.gov/39208285/)

8. Clinical guidelines align with “externalize structure” and “reduce functional friction.”
- NICE guidance emphasizes tailored support for planning, organization, and daily functioning in ADHD and autism.
- Sources: [NICE NG87 recommendations](https://www.nice.org.uk/guidance/ng87/chapter/recommendations), [NICE CG142 recommendations](https://www.nice.org.uk/guidance/cg142/chapter/Recommendations)

### 1.2 Implications for this product

The product should not be “points-first.” It should be:

1. Immediate-feedback first (seconds/minutes, not days).
2. Low-cognitive-load first (clear next action, no clutter).
3. Predictable by default, customizable for stimulation.
4. Failure-tolerant (no punitive streak-loss spiral).
5. Personalized to neurotype profile and energy state.

## 2) Product Vision and Core Experience

## 2.1 Product vision

Create a C++ desktop life-ops + skill-learning RPG where daily execution feels:

1. Clear: always know the next best action.
2. Rewarding: immediate micro-reward after completion.
3. Safe: no shame loops after missed days.
4. Adjustable: supports both low-stimulation and novelty-seeking modes.

## 2.2 Primary user profile

Single-player personal tracker for a neurodivergent user (ADHD/autistic/AuDHD traits), optimized for:

1. Executive function scaffolding.
2. Time blindness compensation.
3. Sensory/load control.
4. Motivational consistency without over-pressure.
5. Progress in technical learning goals (currently C++).

## 2.3 Game loop (MVP)

1. Capture life task/habit or learning objective.
2. Convert into concrete “action units” (small chunks).
3. Complete action unit.
4. Get immediate XP + optional loot + progress animation.
5. Advance daily quest line and weekly progression.
6. Reflect briefly (what worked, friction score).
7. Adapt tomorrow’s plan automatically.

## 2.4 Dual-track progression model

1. Life track:
- Daily tasks, habits, routines, and maintenance goals.

2. Learning track:
- Technical learning goals (C++), study sessions, exercises, and project milestones.

3. Shared systems:
- Same XP economy and completion loop for both tracks.
- Separate progress lenses so life operations and learning progress can each be reviewed clearly.
- Cross-track balancing to prevent one track from crowding out the other.

## 3) Gamification System Design

## 3.1 Mechanics to include

1. Micro-XP and instant confirmation:
- Reward each completed action unit immediately.
- Use tiny audiovisual confirmation with adjustable intensity.

2. Quest decomposition:
- Every habit/project breaks into small quests with explicit done-state.
- “Start with 2-minute version” always available.

3. Adaptive difficulty:
- If completion drops, auto-downscale quest size.
- If completion is strong, suggest gradual stretch goals.

4. Streak alternatives:
- Use “consistency bands” and “recovery tokens” instead of hard streak punishment.
- Keep progress durable even after off-days.

5. Energy-aware scheduling:
- Tag tasks by cognitive load and context.
- Suggest low-load options during low-energy windows.

6. Learning mastery progression:
- Support learning quests with concrete outputs (read, implement, explain, review).
- Track concept mastery with lightweight confidence levels and revisit prompts.

7. Optional sociality:
- Keep social features optional and off by default for MVP.

## 3.2 Mechanics to avoid

1. Punitive streak resets as primary motivator.
2. High-friction onboarding with many mandatory choices.
3. Forced daily notifications without user tuning.
4. Unbounded visual noise or animation overload.

## 4) Dear ImGui Layout Plan (Desktop)

Use ImGui docking and multi-panel workflow.

## 4.1 Main layout regions

1. Left rail (Navigation):
- Today
- Quests
- Habits
- Learning
- Character
- Insights
- Settings

2. Center panel (Primary action):
- “Do Next” queue (top priority 1-3 items) across life + learning tracks
- Start/Complete buttons, timer, quick notes

3. Right panel (State feedback):
- XP bar, level, current quest chain
- Energy meter, focus mode indicator
- Learning streak band and current C++ milestone
- Gentle reminders and recovery options

4. Bottom strip (Session controls):
- Focus session start/stop
- Break prompt
- “Done for now” closure action

## 4.2 Accessibility and neurodivergent UX controls

1. Motion level: Off / Low / Full.
2. Sound level: Off / Minimal / Rich.
3. Density: Compact / Comfortable / Spacious.
4. Color mode presets with contrast checks.
5. “Calm mode” (reduced color/motion/chrome).
6. “Spark mode” (more novelty cues, still bounded).

## 5) Technical Architecture (C++ + Dear ImGui)

## 5.1 Stack

1. Language: C++20
2. Build: CMake
3. UI: Dear ImGui + docking branch, backend via SDL3 + OpenGL3 (or GLFW + OpenGL3 if preferred)
4. Data:
- SQLite for persistent data (tasks, logs, progression, settings)
- JSON for import/export snapshots
5. Serialization/versioning:
- Schema migrations with explicit `schema_version`

## 5.2 Core modules

1. `app/`:
- app lifecycle, frame loop, scene routing

2. `ui/`:
- docking layout, views, reusable widgets, themes

3. `domain/`:
- entities: Habit, Quest, ActionUnit, LearningGoal, LearningSession, UserState, RewardEvent
- services: Scheduler, RewardEngine, AdaptationEngine, LearningPlanner

4. `data/`:
- repository interfaces + sqlite implementation
- migration and backup

5. `telemetry/`:
- local analytics only (no cloud required for MVP)
- completion rates, friction metrics, adaptation outcomes

## 6) End-to-End Project Task Sequence

## Phase 0: Foundation and decisions
1. Choose rendering backend (SDL3 or GLFW).
2. Define coding standards, folder structure, build presets.
3. Create architecture decision records (ADRs).

## Phase 1: App skeleton
1. Bootstrap CMake project and dependencies.
2. Launch ImGui dockspace with placeholder panels.
3. Implement app state container and event bus.
4. Add learning-track placeholder view and navigation entry.

## Phase 2: Domain model + persistence
1. Define core entities and state transitions.
2. Implement SQLite schema and migration system.
3. Add repository tests for CRUD and migration safety.
4. Include learning entities (`LearningGoal`, `LearningSession`) in schema v1.

## Phase 3: Core gameplay loop
1. Task capture and decomposition into action units.
2. Completion flow with immediate reward feedback.
3. Daily/weekly progression calculations.
4. Learning session capture and completion flow.

## Phase 4: Adaptive support
1. Implement energy-based task suggestions.
2. Implement difficulty auto-scaling logic.
3. Add recovery-token and consistency-band mechanics.

## Phase 5: UX personalization
1. Add calm/spark presets and sensory controls.
2. Add accessible typography/contrast options.
3. Add onboarding that configures defaults quickly.

## Phase 6: Insights and reflection
1. Build completion/energy trend views.
2. Show friction triggers and recommended adjustments.
3. Support weekly reflection prompts.
4. Show learning cadence and C++ milestone progression.

## Phase 7: Stabilization
1. Performance profiling and UI responsiveness checks.
2. Save corruption safeguards and backup/restore.
3. Regression tests and release packaging.

## 7) Round 1 Work Package (for current agent cycle)

## Architect Agent
1. Finalize product requirements doc and ADR draft.
2. Define measurable success metrics for MVP.
3. Define dual-track balancing rules (life vs learning workload).
4. Review cross-agent outputs and accept/reject by rubric.

## Engineer Agent
1. Create C++20 + CMake + Dear ImGui app shell.
2. Implement dockspace layout + placeholder screens.
3. Implement minimal domain model (`Habit`, `Quest`, `ActionUnit`, `LearningGoal`, `LearningSession`).
4. Add local persistence bootstrap (SQLite schema v1).
5. Add smoke test for app startup + life+learning data roundtrip.

## Designer Agent
1. Define user flow for:
- add habit
- start action
- complete action
- recover after missed day
 - add learning goal (C++)
 - start learning session
 - complete learning session
2. Produce low-fi layout specs matching ImGui constraints.
3. Define interaction states and error/empty states.
4. Define neurodivergent preference presets (calm/spark).

## Artist Agent
1. Define visual direction (colors, typography scale, icon style).
2. Deliver MVP asset kit:
- icon set for nav and status
- learning track icon + mastery badges
- XP/reward feedback visuals (lightweight)
3. Define animation language with sensory-safe defaults.

## 8) Risks and Mitigations

1. Risk: Over-gamification causes fatigue.
- Mitigation: adaptive cadence, optional effects, intrinsic progress framing.

2. Risk: Too much configuration overwhelms onboarding.
- Mitigation: presets first, advanced settings later.

3. Risk: Inconsistent motivation after disruption.
- Mitigation: recovery mechanics and non-punitive continuity.

4. Risk: Evidence gap specific to AuDHD.
- Mitigation: instrumentation + iterative personalization + user-in-the-loop tuning.

5. Risk: Learning track overwhelms daily-life operations.
- Mitigation: track quotas, weighted prioritization, and weekly rebalance prompt.

## 9) Success Metrics (MVP)

1. Time-to-first-complete-action under 2 minutes after launch.
2. >=70% days with at least one completed action unit (over 4 weeks).
3. <=15 seconds median time to log a completion.
4. Recovery after missed day: >=60% resume next day.
5. User-reported overwhelm trend decreases over first 4 weeks.
6. >=4 completed C++ learning sessions per week in first month.
7. >=50% of C++ learning sessions produce an artifact (note, code snippet, or checkpoint).

---

This document is the execution baseline for round 1.
