#pragma once

#include <string>
#include <string_view>

namespace habitrpg::domain {

enum class TrackType {
  Life,
  Learning,
};

std::string_view TrackTypeToString(TrackType track_type);
TrackType TrackTypeFromString(std::string_view raw);

enum class ActionStatus {
  Todo,
  InProgress,
  Completed,
};

std::string_view ActionStatusToString(ActionStatus status);
ActionStatus ActionStatusFromString(std::string_view raw);

enum class LifecycleState {
  Ready,
  Active,
  Partial,
  Missed,
  Paused,
  Completed,
  CheckpointCandidate,
};

std::string_view LifecycleStateToString(LifecycleState state);
LifecycleState LifecycleStateFromString(std::string_view raw);
bool LifecycleStateIsPending(LifecycleState state);

struct Habit {
  std::string id;
  std::string title;
  std::string cadence;
  bool is_active{true};
  std::string created_at;
};

struct Quest {
  std::string id;
  std::string title;
  TrackType track_type{TrackType::Life};
  bool is_completed{false};
  std::string created_at;
};

struct ActionUnit {
  std::string id;
  std::string parent_id;
  std::string title;
  TrackType track_type{TrackType::Life};
  ActionStatus status{ActionStatus::Todo};
  LifecycleState lifecycle_state{LifecycleState::Ready};
  int priority_score{100};
  std::string started_at;
  std::string completed_at;
};

struct LearningGoal {
  std::string id;
  std::string title;
  std::string milestone;
  int confidence_level{0};
  std::string created_at;
};

struct LearningSession {
  std::string id;
  std::string goal_id;
  std::string title;
  LifecycleState lifecycle_state{LifecycleState::Ready};
  int priority_score{100};
  int duration_minutes{0};
  std::string artifact_kind;
  std::string artifact_ref;
  std::string checkpoint_note;
  std::string started_at;
  std::string completed_at;
};

enum class MilestoneCheckpointState {
  Candidate,
  Confirmed,
  Rejected,
};

std::string_view MilestoneCheckpointStateToString(MilestoneCheckpointState state);
MilestoneCheckpointState MilestoneCheckpointStateFromString(std::string_view raw);

struct MilestoneCheckpoint {
  std::string id;
  std::string goal_id;
  std::string learning_session_id;
  std::string milestone_key;
  MilestoneCheckpointState state{MilestoneCheckpointState::Candidate};
  std::string evidence_kind;
  std::string evidence_ref;
  int confidence_level{1};
  std::string candidate_reason;
  std::string reward_event_id;
  std::string submitted_at;
  std::string reviewed_at;
  std::string confirmed_at;
  std::string rejected_at;
  std::string created_at;
  std::string updated_at;
};

struct UserState {
  int level{1};
  int total_xp{0};
  int life_xp{0};
  int learning_xp{0};
  int recovery_tokens{3};
};

struct RewardEvent {
  std::string id;
  std::string source_type;
  std::string source_id;
  TrackType track_type{TrackType::Life};
  int xp_delta{0};
  std::string reward_kind;
  std::string created_at;
};

std::string CurrentTimestampUtc();
std::string GenerateStableId(std::string_view prefix);

}  // namespace habitrpg::domain
