#pragma once

#include <string>
#include <vector>

#include "habitrpg/domain/entities.hpp"
#include "habitrpg/domain/reward_engine.hpp"

namespace habitrpg::domain {

class InteractionFlowService {
 public:
  ActionUnit CreateLifeAction(
      const std::string& parent_id,
      const std::string& title,
      int priority_score,
      std::string created_at = {}) const;

  LearningGoal CreateLearningGoal(const std::string& title, const std::string& milestone, std::string created_at = {})
      const;

  LearningSession CreateLearningSession(
      const std::string& goal_id,
      const std::string& title,
      int duration_minutes,
      int priority_score,
      const std::string& artifact_kind,
      const std::string& artifact_ref,
      std::string created_at = {}) const;

  bool StartActionUnit(
      const std::string& action_id,
      std::vector<ActionUnit>* action_units,
      std::vector<LearningSession>* learning_sessions) const;

  bool CompleteActionUnit(
      const std::string& action_id,
      std::vector<ActionUnit>* action_units,
      RewardEngine* reward_engine,
      UserState* user_state,
      std::vector<RewardEvent>* reward_events) const;

  bool StartLearningSession(
      const std::string& session_id,
      std::vector<ActionUnit>* action_units,
      std::vector<LearningSession>* learning_sessions) const;

  bool CheckpointLearningSession(
      const std::string& session_id,
      const std::string& checkpoint_note,
      std::vector<LearningSession>* learning_sessions) const;

  MilestoneCheckpoint CreateMilestoneCheckpointCandidate(
      const LearningSession& learning_session,
      const std::string& milestone_key,
      const std::string& evidence_kind,
      const std::string& evidence_ref,
      int confidence_level,
      const std::string& candidate_reason,
      std::string created_at = {}) const;

  bool PromoteMilestoneCheckpointToConfirmed(
      const std::string& checkpoint_id,
      std::vector<MilestoneCheckpoint>* checkpoints,
      RewardEngine* reward_engine,
      UserState* user_state,
      std::vector<RewardEvent>* reward_events) const;

  bool CompleteLearningSession(
      const std::string& session_id,
      std::vector<LearningSession>* learning_sessions,
      RewardEngine* reward_engine,
      UserState* user_state,
      std::vector<RewardEvent>* reward_events) const;
};

}  // namespace habitrpg::domain
