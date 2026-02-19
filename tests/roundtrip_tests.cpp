#include <filesystem>
#include <stdexcept>
#include <string>

#include "habitrpg/data/sqlite_repository.hpp"
#include "habitrpg/domain/entities.hpp"
#include "habitrpg/domain/reward_engine.hpp"

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

}  // namespace

bool RunHabitActionRewardRoundtripTest() {
  const std::string sqlite_path = BuildTempDbPath("life_roundtrip");

  {
    habitrpg::data::SqliteRepository repository(sqlite_path);
    Expect(repository.SchemaVersion() == habitrpg::data::kSchemaVersionV3, "Expected schema version v3");

    habitrpg::domain::Habit habit{};
    habit.id = habitrpg::domain::GenerateStableId("habit");
    habit.title = "Clear inbox";
    habit.cadence = "daily";
    habit.is_active = true;
    habit.created_at = habitrpg::domain::CurrentTimestampUtc();
    repository.UpsertHabit(habit);

    auto loaded_habit = repository.FindHabitById(habit.id);
    Expect(loaded_habit.has_value(), "Habit should roundtrip");
    Expect(loaded_habit->title == habit.title, "Habit title mismatch");

    habitrpg::domain::ActionUnit action_unit{};
    action_unit.id = habitrpg::domain::GenerateStableId("action");
    action_unit.parent_id = habit.id;
    action_unit.title = "Archive 10 email threads";
    action_unit.track_type = habitrpg::domain::TrackType::Life;
    action_unit.status = habitrpg::domain::ActionStatus::Completed;
    action_unit.lifecycle_state = habitrpg::domain::LifecycleState::Completed;
    action_unit.priority_score = 140;
    action_unit.started_at = habitrpg::domain::CurrentTimestampUtc();
    action_unit.completed_at = habitrpg::domain::CurrentTimestampUtc();
    repository.UpsertActionUnit(action_unit);

    habitrpg::domain::RewardEngine reward_engine;
    const auto reward_event = reward_engine.BuildActionCompletionReward(action_unit, action_unit.completed_at);
    repository.AppendRewardEvent(reward_event);

    auto user_state = repository.LoadUserState();
    reward_engine.ApplyReward(reward_event, &user_state);
    repository.SaveUserState(user_state);

    const auto life_actions = repository.ListActionUnitsByTrack(habitrpg::domain::TrackType::Life);
    Expect(!life_actions.empty(), "Life action unit list should not be empty");
    Expect(life_actions.front().lifecycle_state == habitrpg::domain::LifecycleState::Completed, "Action state mismatch");

    const auto life_rewards = repository.ListRewardEventsByTrack(habitrpg::domain::TrackType::Life);
    Expect(life_rewards.size() == 1, "Expected one life reward event");
    Expect(life_rewards.front().source_id == action_unit.id, "Reward source id mismatch");

    const auto loaded_state = repository.LoadUserState();
    Expect(loaded_state.total_xp == reward_event.xp_delta, "Total XP should include action reward");
    Expect(loaded_state.life_xp == reward_event.xp_delta, "Life XP should include action reward");
    Expect(loaded_state.learning_xp == 0, "Learning XP should stay zero in life-only roundtrip");
  }

  std::error_code remove_error;
  std::filesystem::remove(sqlite_path, remove_error);
  return true;
}

bool RunLearningSessionRewardRoundtripTest() {
  const std::string sqlite_path = BuildTempDbPath("learning_roundtrip");

  {
    habitrpg::data::SqliteRepository repository(sqlite_path);
    Expect(repository.SchemaVersion() == habitrpg::data::kSchemaVersionV3, "Expected schema version v3");

    habitrpg::domain::LearningGoal goal{};
    goal.id = habitrpg::domain::GenerateStableId("goal");
    goal.title = "C++ value categories";
    goal.milestone = "Explain lvalue/xvalue/prvalue with examples";
    goal.confidence_level = 1;
    goal.created_at = habitrpg::domain::CurrentTimestampUtc();
    repository.UpsertLearningGoal(goal);

    const auto loaded_goal = repository.FindLearningGoalById(goal.id);
    Expect(loaded_goal.has_value(), "Learning goal should roundtrip");
    Expect(loaded_goal->milestone == goal.milestone, "Learning goal milestone mismatch");

    habitrpg::domain::LearningSession session{};
    session.id = habitrpg::domain::GenerateStableId("session");
    session.goal_id = goal.id;
    session.title = "Value category kata";
    session.lifecycle_state = habitrpg::domain::LifecycleState::Completed;
    session.priority_score = 130;
    session.duration_minutes = 35;
    session.artifact_kind = "code_snippet";
    session.artifact_ref = "examples/value_categories.cpp";
    session.started_at = habitrpg::domain::CurrentTimestampUtc();
    session.completed_at = habitrpg::domain::CurrentTimestampUtc();
    repository.UpsertLearningSession(session);

    habitrpg::domain::RewardEngine reward_engine;
    const auto reward_event = reward_engine.BuildLearningSessionCompletionReward(session, session.completed_at);
    repository.AppendRewardEvent(reward_event);

    auto user_state = repository.LoadUserState();
    reward_engine.ApplyReward(reward_event, &user_state);
    repository.SaveUserState(user_state);

    const auto sessions = repository.ListLearningSessionsByGoal(goal.id);
    Expect(sessions.size() == 1, "Expected one learning session");
    Expect(sessions.front().artifact_kind == "code_snippet", "Learning session artifact mismatch");
    Expect(sessions.front().lifecycle_state == habitrpg::domain::LifecycleState::Completed, "Learning state mismatch");

    const auto learning_rewards = repository.ListRewardEventsByTrack(habitrpg::domain::TrackType::Learning);
    Expect(learning_rewards.size() == 1, "Expected one learning reward event");
    Expect(learning_rewards.front().source_id == session.id, "Learning reward source id mismatch");

    const auto loaded_state = repository.LoadUserState();
    Expect(loaded_state.total_xp == reward_event.xp_delta, "Total XP should include learning reward");
    Expect(loaded_state.learning_xp == reward_event.xp_delta, "Learning XP should include reward");
    Expect(loaded_state.life_xp == 0, "Life XP should stay zero in learning-only roundtrip");
  }

  std::error_code remove_error;
  std::filesystem::remove(sqlite_path, remove_error);
  return true;
}
