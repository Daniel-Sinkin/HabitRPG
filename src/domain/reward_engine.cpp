#include "habitrpg/domain/reward_engine.hpp"

#include <algorithm>
#include <stdexcept>
#include <string>

namespace habitrpg::domain {

RewardEngine::RewardEngine(const RewardEngineConfig config) : config_(config) {}

RewardEvent RewardEngine::BuildActionCompletionReward(
    const ActionUnit& action_unit,
    const std::string_view created_at) const {
  RewardEvent reward_event{};
  reward_event.id = GenerateStableId("reward");
  reward_event.source_type = "action_unit";
  reward_event.source_id = action_unit.id;
  reward_event.track_type = action_unit.track_type;
  reward_event.xp_delta = config_.action_completion_xp;
  reward_event.reward_kind = "xp.action_completion";
  reward_event.created_at = created_at.empty() ? CurrentTimestampUtc() : std::string(created_at);
  return reward_event;
}

RewardEvent RewardEngine::BuildLearningSessionCompletionReward(
    const LearningSession& learning_session,
    const std::string_view created_at) const {
  const auto bonus_chunks = std::max(learning_session.duration_minutes, 0) / 10;

  RewardEvent reward_event{};
  reward_event.id = GenerateStableId("reward");
  reward_event.source_type = "learning_session";
  reward_event.source_id = learning_session.id;
  reward_event.track_type = TrackType::Learning;
  reward_event.xp_delta = config_.learning_session_base_xp +
                          (bonus_chunks * config_.learning_session_xp_per_ten_minutes);
  reward_event.reward_kind = "xp.learning_session_completion";
  reward_event.created_at = created_at.empty() ? CurrentTimestampUtc() : std::string(created_at);
  return reward_event;
}

RewardEvent RewardEngine::BuildMilestoneCheckpointConfirmedReward(
    const MilestoneCheckpoint& milestone_checkpoint,
    const std::string_view created_at,
    const std::string_view event_id_override) const {
  RewardEvent reward_event{};
  if (!event_id_override.empty()) {
    reward_event.id = std::string(event_id_override);
  } else {
    reward_event.id = "reward_milestone_" + milestone_checkpoint.id;
  }
  reward_event.source_type = "milestone_checkpoint";
  reward_event.source_id = milestone_checkpoint.id;
  reward_event.track_type = TrackType::Learning;
  reward_event.xp_delta = config_.milestone_checkpoint_confirmed_xp;
  reward_event.reward_kind = "xp.milestone_checkpoint_confirmed";
  reward_event.created_at = created_at.empty() ? CurrentTimestampUtc() : std::string(created_at);
  return reward_event;
}

void RewardEngine::ApplyReward(const RewardEvent& reward_event, UserState* user_state) const {
  if (user_state == nullptr) {
    throw std::invalid_argument("RewardEngine::ApplyReward requires a user_state output pointer");
  }

  user_state->total_xp += reward_event.xp_delta;

  if (reward_event.track_type == TrackType::Life) {
    user_state->life_xp += reward_event.xp_delta;
  } else {
    user_state->learning_xp += reward_event.xp_delta;
  }

  user_state->level = (user_state->total_xp / 100) + 1;
}

}  // namespace habitrpg::domain
