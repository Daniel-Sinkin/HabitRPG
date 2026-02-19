#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

#include "habitrpg/data/sqlite_repository.hpp"
#include "habitrpg/domain/interaction_flow.hpp"
#include "habitrpg/domain/reward_engine.hpp"
#include "habitrpg/domain/today_queue.hpp"
#include "habitrpg/ui/contracts.hpp"

namespace {

void Expect(bool condition, const std::string& message) {
  if (!condition) {
    throw std::runtime_error(message);
  }
}

std::string BuildTempDbPath(const std::string& suffix) {
  const auto temp_dir = std::filesystem::temp_directory_path();
  const auto file_name = "habitrpg_test_" + suffix + "_" + habitrpg::domain::GenerateStableId("db") + ".sqlite3";
  return (temp_dir / file_name).string();
}

habitrpg::domain::ActionUnit BuildAction(
    const std::string& id,
    const habitrpg::domain::LifecycleState state,
    const int priority) {
  habitrpg::domain::ActionUnit action{};
  action.id = id;
  action.parent_id = "habit";
  action.title = id;
  action.track_type = habitrpg::domain::TrackType::Life;
  action.lifecycle_state = state;
  action.priority_score = priority;
  return action;
}

habitrpg::domain::LearningSession BuildSession(
    const std::string& id,
    const habitrpg::domain::LifecycleState state,
    const int priority) {
  habitrpg::domain::LearningSession session{};
  session.id = id;
  session.goal_id = "goal";
  session.title = id;
  session.lifecycle_state = state;
  session.priority_score = priority;
  return session;
}

}  // namespace

bool RunPresetModeExclusivityAndPersistenceTest() {
  using habitrpg::ui::contracts::PresetMode;
  using habitrpg::ui::contracts::TrackFilter;
  using habitrpg::ui::contracts::UiViewState;

  UiViewState state{};
  habitrpg::ui::contracts::ApplyPresetBundle(&state, PresetMode::Calm);
  Expect(state.preset_mode == PresetMode::Calm, "Calm preset should be selected");
  Expect(state.motion_level == 0, "Calm preset should set motion=0");
  Expect(state.sound_level == 0, "Calm preset should set sound=0");
  Expect(state.density_level == 2, "Calm preset should set density=2");

  habitrpg::ui::contracts::ApplyPresetBundle(&state, PresetMode::Spark);
  Expect(state.preset_mode == PresetMode::Spark, "Spark preset should be selected");
  Expect(state.motion_level == 1, "Spark preset should set motion=1");
  Expect(state.sound_level == 1, "Spark preset should set sound=1");
  Expect(state.density_level == 1, "Spark preset should set density=1");
  Expect(state.last_non_custom_preset == PresetMode::Spark, "Spark should be remembered as last preset");

  habitrpg::ui::contracts::ApplySensoryOverride(&state, 2, 0, 2);
  Expect(state.preset_mode == PresetMode::Custom, "Sensory override should switch to custom");
  Expect(state.motion_level == 2, "Custom motion override should persist");
  Expect(state.sound_level == 0, "Custom sound override should persist");
  Expect(state.density_level == 2, "Custom density override should persist");
  Expect(state.last_non_custom_preset == PresetMode::Spark, "Last non-custom preset should remain spark");

  const std::string sqlite_path = BuildTempDbPath("preset_persistence");
  {
    habitrpg::data::SqliteRepository repository(sqlite_path);
    Expect(repository.SchemaVersion() == habitrpg::data::kSchemaVersionV3, "Expected schema version v3");

    habitrpg::data::UiPreferences preferences{};
    preferences.preset_mode = state.preset_mode;
    preferences.last_non_custom_preset = state.last_non_custom_preset;
    preferences.motion_level = state.motion_level;
    preferences.sound_level = state.sound_level;
    preferences.density_level = state.density_level;
    preferences.queue_mode = TrackFilter::LearningOnly;
    preferences.prompt_concurrency_limit = 3;
    preferences.nudge_cooldown_seconds = 45;
    preferences.updated_at = habitrpg::domain::CurrentTimestampUtc();
    repository.SaveUiPreferences(preferences);

    const auto loaded = repository.LoadUiPreferences();
    Expect(loaded.preset_mode == PresetMode::Custom, "Loaded preset mode should remain custom");
    Expect(loaded.last_non_custom_preset == PresetMode::Spark, "Loaded last_non_custom_preset should be spark");
    Expect(loaded.motion_level == 2, "Loaded motion level mismatch");
    Expect(loaded.sound_level == 0, "Loaded sound level mismatch");
    Expect(loaded.density_level == 2, "Loaded density level mismatch");
    Expect(loaded.queue_mode == TrackFilter::LearningOnly, "Loaded queue mode mismatch");

    UiViewState restored{};
    restored.preset_mode = loaded.preset_mode;
    restored.last_non_custom_preset = loaded.last_non_custom_preset;
    restored.motion_level = loaded.motion_level;
    restored.sound_level = loaded.sound_level;
    restored.density_level = loaded.density_level;

    habitrpg::ui::contracts::RestoreLastNonCustomPreset(&restored);
    Expect(restored.preset_mode == PresetMode::Spark, "Restored preset should return to spark");
    Expect(restored.motion_level == 1, "Restored spark preset should set motion=1");
    Expect(restored.sound_level == 1, "Restored spark preset should set sound=1");
    Expect(restored.density_level == 1, "Restored spark preset should set density=1");
  }

  std::error_code remove_error;
  std::filesystem::remove(sqlite_path, remove_error);
  return true;
}

bool RunMilestoneCheckpointPromotionIdempotencyTest() {
  habitrpg::domain::InteractionFlowService flow_service;
  habitrpg::domain::RewardEngine reward_engine;
  habitrpg::domain::UserState user_state{};
  std::vector<habitrpg::domain::MilestoneCheckpoint> checkpoints;
  std::vector<habitrpg::domain::RewardEvent> reward_events;

  const auto session = flow_service.CreateLearningSession(
      "goal_1",
      "Session for checkpointing",
      35,
      140,
      "snippet",
      "examples/checkpoint.cpp",
      "2026-02-19T00:00:00Z");

  checkpoints.push_back(flow_service.CreateMilestoneCheckpointCandidate(
      session,
      "cpp.value_categories",
      "snippet",
      "examples/value_categories.cpp",
      4,
      "Can explain ownership and value categories",
      "2026-02-19T00:05:00Z"));

  const std::string checkpoint_id = checkpoints.front().id;

  Expect(
      flow_service.PromoteMilestoneCheckpointToConfirmed(
          checkpoint_id,
          &checkpoints,
          &reward_engine,
          &user_state,
          &reward_events),
      "First promotion should confirm candidate");
  Expect(
      checkpoints.front().state == habitrpg::domain::MilestoneCheckpointState::Confirmed,
      "Checkpoint should be confirmed");
  Expect(!checkpoints.front().reward_event_id.empty(), "Confirmed checkpoint should have reward event id");
  Expect(reward_events.size() == 1, "First promotion should append exactly one reward");
  const int xp_after_first_promotion = user_state.total_xp;

  Expect(
      !flow_service.PromoteMilestoneCheckpointToConfirmed(
          checkpoint_id,
          &checkpoints,
          &reward_engine,
          &user_state,
          &reward_events),
      "Second promotion should short-circuit for already confirmed checkpoint");
  Expect(reward_events.size() == 1, "Repeated promotion should not append additional rewards");
  Expect(user_state.total_xp == xp_after_first_promotion, "Repeated promotion should not add XP");

  auto retry_candidate = flow_service.CreateMilestoneCheckpointCandidate(
      session,
      "cpp.references",
      "reference",
      "https://en.cppreference.com",
      3,
      "Reference captured",
      "2026-02-19T00:07:00Z");
  retry_candidate.reward_event_id = "reward_milestone_" + retry_candidate.id;
  checkpoints.push_back(retry_candidate);

  reward_events.push_back(reward_engine.BuildMilestoneCheckpointConfirmedReward(
      retry_candidate,
      "2026-02-19T00:08:00Z",
      retry_candidate.reward_event_id));
  const size_t reward_count_before_retry = reward_events.size();
  const int xp_before_retry = user_state.total_xp;

  Expect(
      flow_service.PromoteMilestoneCheckpointToConfirmed(
          retry_candidate.id,
          &checkpoints,
          &reward_engine,
          &user_state,
          &reward_events),
      "Promotion should be idempotent when reward event already exists");
  Expect(reward_events.size() == reward_count_before_retry, "No duplicate reward should be emitted on retry");
  Expect(user_state.total_xp == xp_before_retry, "Retry confirmation should not double-award XP");
  Expect(
      checkpoints.back().state == habitrpg::domain::MilestoneCheckpointState::Confirmed,
      "Retry candidate should still transition to confirmed");

  return true;
}

bool RunQueueModePersistenceAndFilteringTest() {
  const std::string sqlite_path = BuildTempDbPath("queue_mode_persistence");
  {
    habitrpg::data::SqliteRepository repository(sqlite_path);
    Expect(repository.SchemaVersion() == habitrpg::data::kSchemaVersionV3, "Expected schema version v3");

    habitrpg::data::UiPreferences preferences = repository.LoadUiPreferences();
    preferences.preset_mode = habitrpg::ui::contracts::PresetMode::Calm;
    preferences.last_non_custom_preset = habitrpg::ui::contracts::PresetMode::Calm;
    preferences.motion_level = 0;
    preferences.sound_level = 0;
    preferences.density_level = 2;
    preferences.queue_mode = habitrpg::ui::contracts::TrackFilter::LearningOnly;
    preferences.prompt_concurrency_limit = 2;
    preferences.nudge_cooldown_seconds = 30;
    preferences.updated_at = habitrpg::domain::CurrentTimestampUtc();
    repository.SaveUiPreferences(preferences);

    const auto loaded_learning_only = repository.LoadUiPreferences();
    Expect(
        loaded_learning_only.queue_mode == habitrpg::ui::contracts::TrackFilter::LearningOnly,
        "Queue mode should persist as learning_only");

    const std::vector<habitrpg::domain::ActionUnit> actions{
        BuildAction("life_active", habitrpg::domain::LifecycleState::Active, 130),
        BuildAction("life_ready", habitrpg::domain::LifecycleState::Ready, 120),
    };
    const std::vector<habitrpg::domain::LearningSession> sessions{
        BuildSession("learn_active", habitrpg::domain::LifecycleState::Active, 125),
        BuildSession("learn_ready", habitrpg::domain::LifecycleState::Ready, 110),
    };

    habitrpg::domain::TodayQueueService queue_service;
    const auto learning_queue = queue_service.BuildQueue(loaded_learning_only.queue_mode, actions, sessions, 10);
    Expect(!learning_queue.empty(), "Learning-only queue should contain learning items");
    for (const auto& item : learning_queue) {
      Expect(item.track_type == habitrpg::domain::TrackType::Learning, "Learning-only queue must exclude life items");
    }

    preferences.queue_mode = habitrpg::ui::contracts::TrackFilter::LifeOnly;
    preferences.updated_at = habitrpg::domain::CurrentTimestampUtc();
    repository.SaveUiPreferences(preferences);
    const auto loaded_life_only = repository.LoadUiPreferences();
    Expect(
        loaded_life_only.queue_mode == habitrpg::ui::contracts::TrackFilter::LifeOnly,
        "Queue mode should persist as life_only");

    const auto life_queue = queue_service.BuildQueue(loaded_life_only.queue_mode, actions, sessions, 10);
    Expect(!life_queue.empty(), "Life-only queue should contain life items");
    for (const auto& item : life_queue) {
      Expect(item.track_type == habitrpg::domain::TrackType::Life, "Life-only queue must exclude learning items");
    }
  }

  std::error_code remove_error;
  std::filesystem::remove(sqlite_path, remove_error);
  return true;
}
