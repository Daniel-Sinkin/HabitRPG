#include "habitrpg/domain/interaction_flow.hpp"

#include <algorithm>

namespace habitrpg::domain {
namespace {

void PauseAllActiveActions(const std::string& except_id, std::vector<ActionUnit>* action_units) {
  if (action_units == nullptr) {
    return;
  }

  for (auto& action_unit : *action_units) {
    if (action_unit.id == except_id) {
      continue;
    }

    if (action_unit.lifecycle_state == LifecycleState::Active) {
      action_unit.lifecycle_state = LifecycleState::Paused;
      action_unit.status = ActionStatus::Todo;
    }
  }
}

void PauseAllActiveLearning(const std::string& except_id, std::vector<LearningSession>* learning_sessions) {
  if (learning_sessions == nullptr) {
    return;
  }

  for (auto& learning_session : *learning_sessions) {
    if (learning_session.id == except_id) {
      continue;
    }

    if (learning_session.lifecycle_state == LifecycleState::Active) {
      learning_session.lifecycle_state = LifecycleState::Paused;
    }
  }
}

}  // namespace

ActionUnit InteractionFlowService::CreateLifeAction(
    const std::string& parent_id,
    const std::string& title,
    const int priority_score,
    std::string created_at) const {
  ActionUnit action_unit{};
  action_unit.id = GenerateStableId("action");
  action_unit.parent_id = parent_id;
  action_unit.title = title;
  action_unit.track_type = TrackType::Life;
  action_unit.status = ActionStatus::Todo;
  action_unit.lifecycle_state = LifecycleState::Ready;
  action_unit.priority_score = std::max(priority_score, 0);
  action_unit.started_at = std::move(created_at);
  action_unit.completed_at = "";
  return action_unit;
}

LearningGoal InteractionFlowService::CreateLearningGoal(
    const std::string& title,
    const std::string& milestone,
    std::string created_at) const {
  LearningGoal goal{};
  goal.id = GenerateStableId("goal");
  goal.title = title;
  goal.milestone = milestone;
  goal.confidence_level = 0;
  goal.created_at = created_at.empty() ? CurrentTimestampUtc() : std::move(created_at);
  return goal;
}

LearningSession InteractionFlowService::CreateLearningSession(
    const std::string& goal_id,
    const std::string& title,
    const int duration_minutes,
    const int priority_score,
    const std::string& artifact_kind,
    const std::string& artifact_ref,
    std::string created_at) const {
  LearningSession learning_session{};
  learning_session.id = GenerateStableId("session");
  learning_session.goal_id = goal_id;
  learning_session.title = title;
  learning_session.lifecycle_state = LifecycleState::Ready;
  learning_session.priority_score = std::max(priority_score, 0);
  learning_session.duration_minutes = std::max(duration_minutes, 0);
  learning_session.artifact_kind = artifact_kind;
  learning_session.artifact_ref = artifact_ref;
  learning_session.started_at = std::move(created_at);
  learning_session.completed_at = "";
  return learning_session;
}

bool InteractionFlowService::StartActionUnit(
    const std::string& action_id,
    std::vector<ActionUnit>* action_units,
    std::vector<LearningSession>* learning_sessions) const {
  if (action_units == nullptr) {
    return false;
  }

  auto it = std::find_if(action_units->begin(), action_units->end(), [&action_id](const ActionUnit& action_unit) {
    return action_unit.id == action_id;
  });

  if (it == action_units->end()) {
    return false;
  }

  PauseAllActiveActions(action_id, action_units);
  PauseAllActiveLearning("", learning_sessions);

  it->lifecycle_state = LifecycleState::Active;
  it->status = ActionStatus::InProgress;
  if (it->started_at.empty()) {
    it->started_at = CurrentTimestampUtc();
  }

  return true;
}

bool InteractionFlowService::CompleteActionUnit(
    const std::string& action_id,
    std::vector<ActionUnit>* action_units,
    RewardEngine* reward_engine,
    UserState* user_state,
    std::vector<RewardEvent>* reward_events) const {
  if (action_units == nullptr || reward_engine == nullptr || user_state == nullptr || reward_events == nullptr) {
    return false;
  }

  auto it = std::find_if(action_units->begin(), action_units->end(), [&action_id](const ActionUnit& action_unit) {
    return action_unit.id == action_id;
  });

  if (it == action_units->end()) {
    return false;
  }

  it->lifecycle_state = LifecycleState::Completed;
  it->status = ActionStatus::Completed;
  if (it->started_at.empty()) {
    it->started_at = CurrentTimestampUtc();
  }
  it->completed_at = CurrentTimestampUtc();

  const auto reward_event = reward_engine->BuildActionCompletionReward(*it, it->completed_at);
  reward_engine->ApplyReward(reward_event, user_state);
  reward_events->push_back(reward_event);
  return true;
}

bool InteractionFlowService::StartLearningSession(
    const std::string& session_id,
    std::vector<ActionUnit>* action_units,
    std::vector<LearningSession>* learning_sessions) const {
  if (learning_sessions == nullptr) {
    return false;
  }

  auto it = std::find_if(
      learning_sessions->begin(),
      learning_sessions->end(),
      [&session_id](const LearningSession& learning_session) { return learning_session.id == session_id; });

  if (it == learning_sessions->end()) {
    return false;
  }

  PauseAllActiveActions("", action_units);
  PauseAllActiveLearning(session_id, learning_sessions);

  it->lifecycle_state = LifecycleState::Active;
  if (it->started_at.empty()) {
    it->started_at = CurrentTimestampUtc();
  }

  return true;
}

bool InteractionFlowService::CheckpointLearningSession(
    const std::string& session_id,
    const std::string& checkpoint_note,
    std::vector<LearningSession>* learning_sessions) const {
  if (learning_sessions == nullptr) {
    return false;
  }

  auto it = std::find_if(
      learning_sessions->begin(),
      learning_sessions->end(),
      [&session_id](const LearningSession& learning_session) { return learning_session.id == session_id; });

  if (it == learning_sessions->end()) {
    return false;
  }

  it->lifecycle_state = LifecycleState::CheckpointCandidate;
  it->checkpoint_note = checkpoint_note;
  if (it->started_at.empty()) {
    it->started_at = CurrentTimestampUtc();
  }

  return true;
}

MilestoneCheckpoint InteractionFlowService::CreateMilestoneCheckpointCandidate(
    const LearningSession& learning_session,
    const std::string& milestone_key,
    const std::string& evidence_kind,
    const std::string& evidence_ref,
    const int confidence_level,
    const std::string& candidate_reason,
    std::string created_at) const {
  const std::string now = created_at.empty() ? CurrentTimestampUtc() : std::move(created_at);

  MilestoneCheckpoint checkpoint{};
  checkpoint.id = GenerateStableId("checkpoint");
  checkpoint.goal_id = learning_session.goal_id;
  checkpoint.learning_session_id = learning_session.id;
  checkpoint.milestone_key = milestone_key;
  checkpoint.state = MilestoneCheckpointState::Candidate;
  checkpoint.evidence_kind = evidence_kind;
  checkpoint.evidence_ref = evidence_ref;
  checkpoint.confidence_level = std::clamp(confidence_level, 1, 5);
  checkpoint.candidate_reason = candidate_reason;
  checkpoint.reward_event_id = "";
  checkpoint.submitted_at = now;
  checkpoint.reviewed_at = "";
  checkpoint.confirmed_at = "";
  checkpoint.rejected_at = "";
  checkpoint.created_at = now;
  checkpoint.updated_at = now;
  return checkpoint;
}

bool InteractionFlowService::PromoteMilestoneCheckpointToConfirmed(
    const std::string& checkpoint_id,
    std::vector<MilestoneCheckpoint>* checkpoints,
    RewardEngine* reward_engine,
    UserState* user_state,
    std::vector<RewardEvent>* reward_events) const {
  if (checkpoints == nullptr || reward_engine == nullptr || user_state == nullptr || reward_events == nullptr) {
    return false;
  }

  auto checkpoint_it = std::find_if(
      checkpoints->begin(),
      checkpoints->end(),
      [&checkpoint_id](const MilestoneCheckpoint& checkpoint) { return checkpoint.id == checkpoint_id; });

  if (checkpoint_it == checkpoints->end()) {
    return false;
  }

  if (checkpoint_it->state == MilestoneCheckpointState::Confirmed && !checkpoint_it->reward_event_id.empty()) {
    return false;
  }

  checkpoint_it->state = MilestoneCheckpointState::Confirmed;
  checkpoint_it->reviewed_at = CurrentTimestampUtc();
  checkpoint_it->confirmed_at = checkpoint_it->reviewed_at;
  checkpoint_it->updated_at = checkpoint_it->reviewed_at;
  if (checkpoint_it->reward_event_id.empty()) {
    checkpoint_it->reward_event_id = "reward_milestone_" + checkpoint_it->id;
  }

  const bool reward_exists = std::any_of(
      reward_events->begin(),
      reward_events->end(),
      [checkpoint_it](const RewardEvent& reward_event) { return reward_event.id == checkpoint_it->reward_event_id; });

  if (reward_exists) {
    return true;
  }

  const auto reward_event = reward_engine->BuildMilestoneCheckpointConfirmedReward(
      *checkpoint_it,
      checkpoint_it->confirmed_at,
      checkpoint_it->reward_event_id);
  reward_engine->ApplyReward(reward_event, user_state);
  reward_events->push_back(reward_event);
  return true;
}

bool InteractionFlowService::CompleteLearningSession(
    const std::string& session_id,
    std::vector<LearningSession>* learning_sessions,
    RewardEngine* reward_engine,
    UserState* user_state,
    std::vector<RewardEvent>* reward_events) const {
  if (learning_sessions == nullptr || reward_engine == nullptr || user_state == nullptr || reward_events == nullptr) {
    return false;
  }

  auto it = std::find_if(
      learning_sessions->begin(),
      learning_sessions->end(),
      [&session_id](const LearningSession& learning_session) { return learning_session.id == session_id; });

  if (it == learning_sessions->end()) {
    return false;
  }

  it->lifecycle_state = LifecycleState::Completed;
  if (it->started_at.empty()) {
    it->started_at = CurrentTimestampUtc();
  }
  it->completed_at = CurrentTimestampUtc();

  const auto reward_event = reward_engine->BuildLearningSessionCompletionReward(*it, it->completed_at);
  reward_engine->ApplyReward(reward_event, user_state);
  reward_events->push_back(reward_event);
  return true;
}

}  // namespace habitrpg::domain
