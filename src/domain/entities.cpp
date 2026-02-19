#include "habitrpg/domain/entities.hpp"

#include <atomic>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace habitrpg::domain {

namespace {
std::atomic<uint64_t> g_id_counter{0};

std::string ToIso8601Utc(std::time_t now_seconds) {
  std::tm utc_tm{};
#if defined(_WIN32)
  gmtime_s(&utc_tm, &now_seconds);
#else
  gmtime_r(&now_seconds, &utc_tm);
#endif

  std::ostringstream builder;
  builder << std::put_time(&utc_tm, "%Y-%m-%dT%H:%M:%SZ");
  return builder.str();
}

}  // namespace

std::string_view TrackTypeToString(const TrackType track_type) {
  switch (track_type) {
    case TrackType::Life:
      return "life";
    case TrackType::Learning:
      return "learning";
  }

  throw std::invalid_argument("Unknown TrackType");
}

TrackType TrackTypeFromString(const std::string_view raw) {
  if (raw == "life") {
    return TrackType::Life;
  }
  if (raw == "learning") {
    return TrackType::Learning;
  }
  throw std::invalid_argument("Unknown track type: " + std::string(raw));
}

std::string_view ActionStatusToString(const ActionStatus status) {
  switch (status) {
    case ActionStatus::Todo:
      return "todo";
    case ActionStatus::InProgress:
      return "in_progress";
    case ActionStatus::Completed:
      return "completed";
  }

  throw std::invalid_argument("Unknown ActionStatus");
}

ActionStatus ActionStatusFromString(const std::string_view raw) {
  if (raw == "todo") {
    return ActionStatus::Todo;
  }
  if (raw == "in_progress") {
    return ActionStatus::InProgress;
  }
  if (raw == "completed") {
    return ActionStatus::Completed;
  }

  throw std::invalid_argument("Unknown action status: " + std::string(raw));
}

std::string_view LifecycleStateToString(const LifecycleState state) {
  switch (state) {
    case LifecycleState::Ready:
      return "ready";
    case LifecycleState::Active:
      return "active";
    case LifecycleState::Partial:
      return "partial";
    case LifecycleState::Missed:
      return "missed";
    case LifecycleState::Paused:
      return "paused";
    case LifecycleState::Completed:
      return "completed";
    case LifecycleState::CheckpointCandidate:
      return "checkpoint_candidate";
  }

  throw std::invalid_argument("Unknown LifecycleState");
}

LifecycleState LifecycleStateFromString(const std::string_view raw) {
  if (raw == "ready") {
    return LifecycleState::Ready;
  }
  if (raw == "active") {
    return LifecycleState::Active;
  }
  if (raw == "partial") {
    return LifecycleState::Partial;
  }
  if (raw == "missed") {
    return LifecycleState::Missed;
  }
  if (raw == "paused") {
    return LifecycleState::Paused;
  }
  if (raw == "completed") {
    return LifecycleState::Completed;
  }
  if (raw == "checkpoint_candidate") {
    return LifecycleState::CheckpointCandidate;
  }

  throw std::invalid_argument("Unknown lifecycle state: " + std::string(raw));
}

bool LifecycleStateIsPending(const LifecycleState state) {
  switch (state) {
    case LifecycleState::Completed:
      return false;
    case LifecycleState::Ready:
    case LifecycleState::Active:
    case LifecycleState::Partial:
    case LifecycleState::Missed:
    case LifecycleState::Paused:
    case LifecycleState::CheckpointCandidate:
      return true;
  }

  return false;
}

std::string_view MilestoneCheckpointStateToString(const MilestoneCheckpointState state) {
  switch (state) {
    case MilestoneCheckpointState::Candidate:
      return "candidate";
    case MilestoneCheckpointState::Confirmed:
      return "confirmed";
    case MilestoneCheckpointState::Rejected:
      return "rejected";
  }

  throw std::invalid_argument("Unknown MilestoneCheckpointState");
}

MilestoneCheckpointState MilestoneCheckpointStateFromString(const std::string_view raw) {
  if (raw == "candidate") {
    return MilestoneCheckpointState::Candidate;
  }
  if (raw == "confirmed") {
    return MilestoneCheckpointState::Confirmed;
  }
  if (raw == "rejected") {
    return MilestoneCheckpointState::Rejected;
  }

  throw std::invalid_argument("Unknown milestone checkpoint state: " + std::string(raw));
}

std::string CurrentTimestampUtc() {
  const auto now = std::chrono::system_clock::now();
  return ToIso8601Utc(std::chrono::system_clock::to_time_t(now));
}

std::string GenerateStableId(const std::string_view prefix) {
  const auto now = std::chrono::system_clock::now();
  const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
  const auto sequence = g_id_counter.fetch_add(1, std::memory_order_relaxed);

  std::ostringstream id;
  id << prefix << '_' << now_ms << '_' << sequence;
  return id.str();
}

}  // namespace habitrpg::domain
