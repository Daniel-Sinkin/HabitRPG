# Engineer Contracts (Round 3)

## Version
- `domain.contracts.v1` (includes v2 command/event IDs in the current contract header)
- `repository.v1` (expanded API surface for Round 3)
- SQLite schema target: `v3`

## Stable Track Type Identifiers
- `life`
- `learning`

## View Model and UI State Contracts
Primary file: `/Users/danielsinkin/GitHub_private/HabitRPG/include/habitrpg/ui/contracts.hpp`

Stable view IDs:
- `view.today.v1`
- `view.quests.v1`
- `view.habits.v1`
- `view.learning.v1`
- `view.character.v1`
- `view.insights.v1`
- `view.settings.v1`

Stable queue mode IDs:
- `vm.queue_mode.mixed.v2`
- `vm.queue_mode.life_only.v2`
- `vm.queue_mode.learning_only.v2`

Stable reward tier IDs:
- `vm.reward_tier.off.v1`
- `vm.reward_tier.low.v1`
- `vm.reward_tier.full.v1`

Persisted preset mode values:
- `calm`
- `spark`
- `custom`

Persisted queue mode values:
- `mixed`
- `life_only`
- `learning_only`

## Domain Command IDs
Primary file: `/Users/danielsinkin/GitHub_private/HabitRPG/include/habitrpg/domain/contracts.hpp`

- `command.complete_action.v1`
- `command.complete_learning_session.v1`
- `command.create_life_action.v2`
- `command.start_unit.v2`
- `command.checkpoint_learning_session.v2`

## Domain Event IDs
Primary file: `/Users/danielsinkin/GitHub_private/HabitRPG/include/habitrpg/domain/contracts.hpp`

- `event.action_completed.v1`
- `event.learning_session_completed.v1`
- `event.unit_started.v2`
- `event.learning_session_checkpointed.v2`

## Reward and Source Contracts
Primary file: `/Users/danielsinkin/GitHub_private/HabitRPG/src/domain/reward_engine.cpp`

Stable reward kinds:
- `xp.action_completion`
- `xp.learning_session_completion`
- `xp.milestone_checkpoint_confirmed`

Stable reward source types:
- `action_unit`
- `learning_session`
- `milestone_checkpoint`

## Repository API Contracts
Primary files:
- `/Users/danielsinkin/GitHub_private/HabitRPG/include/habitrpg/data/repositories.hpp`
- `/Users/danielsinkin/GitHub_private/HabitRPG/include/habitrpg/data/sqlite_repository.hpp`

Interfaces:
- `IHabitRepository`
- `IQuestRepository`
- `IActionUnitRepository`
- `ILearningRepository`
- `IMilestoneCheckpointRepository`
- `IRewardRepository`
- `IUserStateRepository`
- `IUiPreferencesRepository`

Round 3 additions:
- `IMilestoneCheckpointRepository::UpsertMilestoneCheckpoint`
- `IMilestoneCheckpointRepository::FindMilestoneCheckpointById`
- `IMilestoneCheckpointRepository::ListMilestoneCheckpointsByGoal`
- `IMilestoneCheckpointRepository::ListMilestoneCheckpoints`
- `IUiPreferencesRepository::LoadUiPreferences`
- `IUiPreferencesRepository::SaveUiPreferences`

`UiPreferences` contract fields:
- `preset_mode`
- `last_non_custom_preset`
- `motion_level`
- `sound_level`
- `density_level`
- `queue_mode`
- `prompt_concurrency_limit`
- `nudge_cooldown_seconds`
- `updated_at`

## Entity Contracts Added in Round 3
Primary file: `/Users/danielsinkin/GitHub_private/HabitRPG/include/habitrpg/domain/entities.hpp`

`MilestoneCheckpoint` state values:
- `candidate`
- `confirmed`
- `rejected`

Checkpoint reward idempotency keys:
- `milestone_checkpoints.reward_event_id` (unique in storage)
- `reward_events.id` (primary key with conflict-ignore append behavior)

## Schema v3 Contracts
Primary files:
- `/Users/danielsinkin/GitHub_private/HabitRPG/include/habitrpg/data/migrations.hpp`
- `/Users/danielsinkin/GitHub_private/HabitRPG/src/data/migrations.cpp`

New tables:
- `milestone_checkpoints`
- `ui_preferences`

Default `ui_preferences` row contract (`id = 1`):
- `preset_mode = calm`
- `last_non_custom_preset = calm`
- `motion_level = 0`
- `sound_level = 0`
- `density_level = 2`
- `queue_mode = mixed`

## Runtime Resource Contracts
Primary files:
- `/Users/danielsinkin/GitHub_private/HabitRPG/include/habitrpg/ui/runtime_resources.hpp`
- `/Users/danielsinkin/GitHub_private/HabitRPG/src/ui/runtime_resources.cpp`

Loaded runtime sources:
- `assets/ui/themes/*.json`
- `assets/ui/runtime/asset_map_v2.json`
- `docs/DESIGNER_COPY_PACK_V1.md`
