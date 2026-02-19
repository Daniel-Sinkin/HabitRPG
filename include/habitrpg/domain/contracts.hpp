#pragma once

#include <string>
#include <string_view>

#include "habitrpg/domain/entities.hpp"

namespace habitrpg::domain::contracts {

inline constexpr std::string_view kContractVersion = "domain.contracts.v1";
inline constexpr std::string_view kTrackLife = "life";
inline constexpr std::string_view kTrackLearning = "learning";

struct CompleteActionCommand {
  static constexpr std::string_view kCommandId = "command.complete_action.v1";

  std::string action_unit_id;
  TrackType track_type{TrackType::Life};
  std::string completed_at;
};

struct CreateLifeActionCommand {
  static constexpr std::string_view kCommandId = "command.create_life_action.v2";

  std::string parent_id;
  std::string title;
  int priority_score{100};
};

struct StartUnitCommand {
  static constexpr std::string_view kCommandId = "command.start_unit.v2";

  std::string unit_id;
  TrackType track_type{TrackType::Life};
};

struct CompleteLearningSessionCommand {
  static constexpr std::string_view kCommandId = "command.complete_learning_session.v1";

  std::string learning_session_id;
  std::string learning_goal_id;
  int duration_minutes{0};
  std::string completed_at;
};

struct ActionCompletedEvent {
  static constexpr std::string_view kEventId = "event.action_completed.v1";

  std::string action_unit_id;
  std::string reward_event_id;
  TrackType track_type{TrackType::Life};
  int xp_delta{0};
  std::string created_at;
};

struct CheckpointLearningSessionCommand {
  static constexpr std::string_view kCommandId = "command.checkpoint_learning_session.v2";

  std::string learning_session_id;
  std::string checkpoint_note;
};

struct LearningSessionCompletedEvent {
  static constexpr std::string_view kEventId = "event.learning_session_completed.v1";

  std::string learning_session_id;
  std::string reward_event_id;
  int xp_delta{0};
  std::string created_at;
};

struct UnitStartedEvent {
  static constexpr std::string_view kEventId = "event.unit_started.v2";

  std::string unit_id;
  TrackType track_type{TrackType::Life};
  std::string created_at;
};

struct LearningSessionCheckpointedEvent {
  static constexpr std::string_view kEventId = "event.learning_session_checkpointed.v2";

  std::string learning_session_id;
  std::string checkpoint_note;
  std::string created_at;
};

}  // namespace habitrpg::domain::contracts
