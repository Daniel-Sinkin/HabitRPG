#include <stdexcept>
#include <string>
#include <vector>

#include "habitrpg/domain/interaction_flow.hpp"

namespace {

void Expect(bool condition, const std::string& message) {
  if (!condition) {
    throw std::runtime_error(message);
  }
}

}  // namespace

bool RunSingleActiveConflictResolutionTest() {
  habitrpg::domain::InteractionFlowService flow_service;

  std::vector<habitrpg::domain::ActionUnit> actions;
  std::vector<habitrpg::domain::LearningSession> sessions;

  actions.push_back(flow_service.CreateLifeAction("habit", "Action A", 120));
  actions.push_back(flow_service.CreateLifeAction("habit", "Action B", 110));

  auto learning_goal = flow_service.CreateLearningGoal("C++ Ownership", "Use unique_ptr/ref patterns");
  sessions.push_back(
      flow_service.CreateLearningSession(learning_goal.id, "Session A", 25, 130, "code", "snippet.cpp"));

  const std::string action_a_id = actions[0].id;
  const std::string action_b_id = actions[1].id;
  const std::string session_a_id = sessions[0].id;

  Expect(flow_service.StartActionUnit(action_a_id, &actions, &sessions), "Action A start should succeed");
  Expect(actions[0].lifecycle_state == habitrpg::domain::LifecycleState::Active, "Action A should be active");

  Expect(
      flow_service.StartLearningSession(session_a_id, &actions, &sessions),
      "Learning session start should succeed");
  Expect(actions[0].lifecycle_state == habitrpg::domain::LifecycleState::Paused, "Action A should be paused");
  Expect(sessions[0].lifecycle_state == habitrpg::domain::LifecycleState::Active, "Session A should be active");

  Expect(flow_service.StartActionUnit(action_b_id, &actions, &sessions), "Action B start should succeed");
  Expect(actions[1].lifecycle_state == habitrpg::domain::LifecycleState::Active, "Action B should be active");
  Expect(sessions[0].lifecycle_state == habitrpg::domain::LifecycleState::Paused, "Session A should be paused");

  return true;
}

bool RunLearningCheckpointLifecycleTest() {
  habitrpg::domain::InteractionFlowService flow_service;
  habitrpg::domain::RewardEngine reward_engine;

  std::vector<habitrpg::domain::ActionUnit> actions;
  std::vector<habitrpg::domain::LearningSession> sessions;
  std::vector<habitrpg::domain::RewardEvent> reward_events;
  habitrpg::domain::UserState user_state{};

  auto learning_goal = flow_service.CreateLearningGoal("C++ Templates", "Implement type-safe wrapper");
  sessions.push_back(flow_service.CreateLearningSession(
      learning_goal.id,
      "Template checkpoint",
      40,
      150,
      "code_snippet",
      "templates/wrapper.cpp"));

  const std::string session_id = sessions.front().id;

  Expect(
      flow_service.StartLearningSession(session_id, &actions, &sessions),
      "Learning session start should succeed");
  Expect(sessions.front().lifecycle_state == habitrpg::domain::LifecycleState::Active, "Session should be active");

  Expect(
      flow_service.CheckpointLearningSession(session_id, "halfway checkpoint", &sessions),
      "Checkpoint should succeed");
  Expect(
      sessions.front().lifecycle_state == habitrpg::domain::LifecycleState::CheckpointCandidate,
      "Session should be checkpoint candidate");
  Expect(sessions.front().checkpoint_note == "halfway checkpoint", "Checkpoint note mismatch");

  Expect(
      flow_service.CompleteLearningSession(session_id, &sessions, &reward_engine, &user_state, &reward_events),
      "Session completion should succeed");
  Expect(
      sessions.front().lifecycle_state == habitrpg::domain::LifecycleState::Completed,
      "Session should be completed");
  Expect(!reward_events.empty(), "Completing session should create reward event");
  Expect(user_state.total_xp > 0, "User state should gain XP after completion");

  return true;
}
