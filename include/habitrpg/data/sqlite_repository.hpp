#pragma once

#include <sqlite3.h>

#include <string>

#include "habitrpg/data/migrations.hpp"
#include "habitrpg/data/repositories.hpp"

namespace habitrpg::data {

class SqliteRepository final : public IHabitRepository,
                               public IQuestRepository,
                               public IActionUnitRepository,
                               public ILearningRepository,
                               public IMilestoneCheckpointRepository,
                               public IRewardRepository,
                               public IUserStateRepository,
                               public IUiPreferencesRepository {
 public:
  explicit SqliteRepository(std::string sqlite_path);
  ~SqliteRepository() override;

  SqliteRepository(const SqliteRepository&) = delete;
  SqliteRepository& operator=(const SqliteRepository&) = delete;
  SqliteRepository(SqliteRepository&&) = delete;
  SqliteRepository& operator=(SqliteRepository&&) = delete;

  void Migrate();
  int SchemaVersion() const;

  void UpsertHabit(const domain::Habit& habit) override;
  std::optional<domain::Habit> FindHabitById(const std::string& id) const override;
  std::vector<domain::Habit> ListHabits() const override;

  void UpsertQuest(const domain::Quest& quest) override;
  std::vector<domain::Quest> ListQuests() const override;

  void UpsertActionUnit(const domain::ActionUnit& action_unit) override;
  std::optional<domain::ActionUnit> FindActionUnitById(const std::string& id) const override;
  std::vector<domain::ActionUnit> ListActionUnitsByTrack(domain::TrackType track_type) const override;

  void UpsertLearningGoal(const domain::LearningGoal& goal) override;
  std::optional<domain::LearningGoal> FindLearningGoalById(const std::string& id) const override;
  std::vector<domain::LearningGoal> ListLearningGoals() const override;
  void UpsertLearningSession(const domain::LearningSession& session) override;
  std::vector<domain::LearningSession> ListLearningSessionsByGoal(const std::string& goal_id) const override;
  std::vector<domain::LearningSession> ListLearningSessions() const override;

  void UpsertMilestoneCheckpoint(const domain::MilestoneCheckpoint& checkpoint) override;
  std::optional<domain::MilestoneCheckpoint> FindMilestoneCheckpointById(const std::string& id) const override;
  std::vector<domain::MilestoneCheckpoint> ListMilestoneCheckpointsByGoal(const std::string& goal_id) const override;
  std::vector<domain::MilestoneCheckpoint> ListMilestoneCheckpoints() const override;

  void AppendRewardEvent(const domain::RewardEvent& reward_event) override;
  std::vector<domain::RewardEvent> ListRewardEventsByTrack(domain::TrackType track_type) const override;

  domain::UserState LoadUserState() const override;
  void SaveUserState(const domain::UserState& user_state) override;

  UiPreferences LoadUiPreferences() const override;
  void SaveUiPreferences(const UiPreferences& preferences) override;

 private:
  sqlite3* db_{nullptr};
  std::string sqlite_path_;

  void ExecOrThrow(const std::string& sql) const;
};

}  // namespace habitrpg::data
