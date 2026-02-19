#pragma once

#include <string_view>

#include "habitrpg/domain/entities.hpp"

namespace habitrpg::domain {

struct RewardEngineConfig {
  int action_completion_xp{12};
  int learning_session_base_xp{16};
  int learning_session_xp_per_ten_minutes{2};
  int milestone_checkpoint_confirmed_xp{24};
};

class RewardEngine {
 public:
  explicit RewardEngine(RewardEngineConfig config = {});

  RewardEvent BuildActionCompletionReward(
      const ActionUnit& action_unit,
      std::string_view created_at = {}) const;

  RewardEvent BuildLearningSessionCompletionReward(
      const LearningSession& learning_session,
      std::string_view created_at = {}) const;

  RewardEvent BuildMilestoneCheckpointConfirmedReward(
      const MilestoneCheckpoint& milestone_checkpoint,
      std::string_view created_at = {},
      std::string_view event_id_override = {}) const;

  void ApplyReward(const RewardEvent& reward_event, UserState* user_state) const;

 private:
  RewardEngineConfig config_;
};

}  // namespace habitrpg::domain
