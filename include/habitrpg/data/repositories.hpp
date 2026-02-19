#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "habitrpg/domain/entities.hpp"
#include "habitrpg/ui/contracts.hpp"

namespace habitrpg::data {

namespace contracts {
inline constexpr std::string_view kRepositoryApiVersion = "repository.v1";
inline constexpr std::string_view kLifeTrackId = "life";
inline constexpr std::string_view kLearningTrackId = "learning";
}  // namespace contracts

class IHabitRepository {
 public:
  virtual ~IHabitRepository() = default;

  virtual void UpsertHabit(const domain::Habit& habit) = 0;
  virtual std::optional<domain::Habit> FindHabitById(const std::string& id) const = 0;
  virtual std::vector<domain::Habit> ListHabits() const = 0;
};

class IQuestRepository {
 public:
  virtual ~IQuestRepository() = default;

  virtual void UpsertQuest(const domain::Quest& quest) = 0;
  virtual std::vector<domain::Quest> ListQuests() const = 0;
};

class IActionUnitRepository {
 public:
  virtual ~IActionUnitRepository() = default;

  virtual void UpsertActionUnit(const domain::ActionUnit& action_unit) = 0;
  virtual std::optional<domain::ActionUnit> FindActionUnitById(const std::string& id) const = 0;
  virtual std::vector<domain::ActionUnit> ListActionUnitsByTrack(domain::TrackType track_type) const = 0;
};

class ILearningRepository {
 public:
  virtual ~ILearningRepository() = default;

  virtual void UpsertLearningGoal(const domain::LearningGoal& goal) = 0;
  virtual std::optional<domain::LearningGoal> FindLearningGoalById(const std::string& id) const = 0;
  virtual std::vector<domain::LearningGoal> ListLearningGoals() const = 0;
  virtual void UpsertLearningSession(const domain::LearningSession& session) = 0;
  virtual std::vector<domain::LearningSession> ListLearningSessionsByGoal(const std::string& goal_id) const = 0;
  virtual std::vector<domain::LearningSession> ListLearningSessions() const = 0;
};

class IMilestoneCheckpointRepository {
 public:
  virtual ~IMilestoneCheckpointRepository() = default;

  virtual void UpsertMilestoneCheckpoint(const domain::MilestoneCheckpoint& checkpoint) = 0;
  virtual std::optional<domain::MilestoneCheckpoint> FindMilestoneCheckpointById(const std::string& id) const = 0;
  virtual std::vector<domain::MilestoneCheckpoint> ListMilestoneCheckpointsByGoal(const std::string& goal_id) const = 0;
  virtual std::vector<domain::MilestoneCheckpoint> ListMilestoneCheckpoints() const = 0;
};

class IRewardRepository {
 public:
  virtual ~IRewardRepository() = default;

  virtual void AppendRewardEvent(const domain::RewardEvent& reward_event) = 0;
  virtual std::vector<domain::RewardEvent> ListRewardEventsByTrack(domain::TrackType track_type) const = 0;
};

class IUserStateRepository {
 public:
  virtual ~IUserStateRepository() = default;

  virtual domain::UserState LoadUserState() const = 0;
  virtual void SaveUserState(const domain::UserState& user_state) = 0;
};

struct UiPreferences {
  ui::contracts::PresetMode preset_mode{ui::contracts::PresetMode::Calm};
  ui::contracts::PresetMode last_non_custom_preset{ui::contracts::PresetMode::Calm};
  int motion_level{0};
  int sound_level{0};
  int density_level{2};
  ui::contracts::TrackFilter queue_mode{ui::contracts::TrackFilter::Mixed};
  int prompt_concurrency_limit{2};
  int nudge_cooldown_seconds{30};
  std::string updated_at{};
};

class IUiPreferencesRepository {
 public:
  virtual ~IUiPreferencesRepository() = default;

  virtual UiPreferences LoadUiPreferences() const = 0;
  virtual void SaveUiPreferences(const UiPreferences& preferences) = 0;
};

}  // namespace habitrpg::data
