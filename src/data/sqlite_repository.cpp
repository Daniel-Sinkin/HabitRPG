#include "habitrpg/data/sqlite_repository.hpp"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <utility>

namespace habitrpg::data {
namespace {

class Statement final {
 public:
  Statement(sqlite3* db, const std::string& sql) : db_(db) {
    const int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &statement_, nullptr);
    if (rc != SQLITE_OK) {
      throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db_)));
    }
  }

  ~Statement() {
    if (statement_ != nullptr) {
      sqlite3_finalize(statement_);
    }
  }

  Statement(const Statement&) = delete;
  Statement& operator=(const Statement&) = delete;

  sqlite3_stmt* get() { return statement_; }

 private:
  sqlite3* db_{nullptr};
  sqlite3_stmt* statement_{nullptr};
};

void CheckResult(const int rc, sqlite3* db, const std::string& context) {
  if (rc == SQLITE_OK || rc == SQLITE_ROW || rc == SQLITE_DONE) {
    return;
  }

  throw std::runtime_error(context + ": " + sqlite3_errmsg(db));
}

void BindText(sqlite3* db, sqlite3_stmt* statement, const int index, const std::string& value) {
  const int rc = sqlite3_bind_text(statement, index, value.c_str(), -1, SQLITE_TRANSIENT);
  CheckResult(rc, db, "sqlite3_bind_text failed");
}

void BindInt(sqlite3* db, sqlite3_stmt* statement, const int index, const int value) {
  const int rc = sqlite3_bind_int(statement, index, value);
  CheckResult(rc, db, "sqlite3_bind_int failed");
}

domain::ActionStatus StatusFromLifecycle(const domain::LifecycleState state) {
  switch (state) {
    case domain::LifecycleState::Completed:
      return domain::ActionStatus::Completed;
    case domain::LifecycleState::Active:
      return domain::ActionStatus::InProgress;
    case domain::LifecycleState::Ready:
    case domain::LifecycleState::Partial:
    case domain::LifecycleState::Missed:
    case domain::LifecycleState::Paused:
    case domain::LifecycleState::CheckpointCandidate:
      return domain::ActionStatus::Todo;
  }

  return domain::ActionStatus::Todo;
}

domain::LifecycleState LifecycleFromLegacyStatus(const domain::ActionStatus status) {
  switch (status) {
    case domain::ActionStatus::Completed:
      return domain::LifecycleState::Completed;
    case domain::ActionStatus::InProgress:
      return domain::LifecycleState::Active;
    case domain::ActionStatus::Todo:
      return domain::LifecycleState::Ready;
  }

  return domain::LifecycleState::Ready;
}

std::string ColumnText(sqlite3_stmt* statement, const int index) {
  const auto* raw = sqlite3_column_text(statement, index);
  if (raw == nullptr) {
    return "";
  }

  return reinterpret_cast<const char*>(raw);
}

}  // namespace

SqliteRepository::SqliteRepository(std::string sqlite_path) : sqlite_path_(std::move(sqlite_path)) {
  const int rc = sqlite3_open_v2(
      sqlite_path_.c_str(),
      &db_,
      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX,
      nullptr);

  if (rc != SQLITE_OK) {
    const std::string details = db_ != nullptr ? sqlite3_errmsg(db_) : "unknown sqlite open error";
    if (db_ != nullptr) {
      sqlite3_close(db_);
      db_ = nullptr;
    }
    throw std::runtime_error("Failed to open sqlite database at path " + sqlite_path_ + ": " + details);
  }

  Migrate();
}

SqliteRepository::~SqliteRepository() {
  if (db_ != nullptr) {
    sqlite3_close(db_);
    db_ = nullptr;
  }
}

void SqliteRepository::Migrate() {
  RunMigrations(db_);
}

int SqliteRepository::SchemaVersion() const {
  return ReadSchemaVersion(db_);
}

void SqliteRepository::UpsertHabit(const domain::Habit& habit) {
  Statement statement(
      db_,
      R"SQL(
        INSERT INTO habits(id, title, cadence, is_active, created_at)
        VALUES(?, ?, ?, ?, ?)
        ON CONFLICT(id) DO UPDATE SET
          title = excluded.title,
          cadence = excluded.cadence,
          is_active = excluded.is_active;
      )SQL");

  BindText(db_, statement.get(), 1, habit.id);
  BindText(db_, statement.get(), 2, habit.title);
  BindText(db_, statement.get(), 3, habit.cadence);
  BindInt(db_, statement.get(), 4, habit.is_active ? 1 : 0);
  BindText(db_, statement.get(), 5, habit.created_at);

  CheckResult(sqlite3_step(statement.get()), db_, "UpsertHabit failed");
}

std::optional<domain::Habit> SqliteRepository::FindHabitById(const std::string& id) const {
  Statement statement(
      db_,
      "SELECT id, title, cadence, is_active, created_at FROM habits WHERE id = ? LIMIT 1;");

  BindText(db_, statement.get(), 1, id);

  const int rc = sqlite3_step(statement.get());
  if (rc == SQLITE_DONE) {
    return std::nullopt;
  }
  CheckResult(rc, db_, "FindHabitById failed");

  domain::Habit habit{};
  habit.id = ColumnText(statement.get(), 0);
  habit.title = ColumnText(statement.get(), 1);
  habit.cadence = ColumnText(statement.get(), 2);
  habit.is_active = sqlite3_column_int(statement.get(), 3) == 1;
  habit.created_at = ColumnText(statement.get(), 4);
  return habit;
}

std::vector<domain::Habit> SqliteRepository::ListHabits() const {
  Statement statement(
      db_,
      "SELECT id, title, cadence, is_active, created_at FROM habits ORDER BY created_at ASC;");

  std::vector<domain::Habit> habits;
  while (true) {
    const int rc = sqlite3_step(statement.get());
    if (rc == SQLITE_DONE) {
      break;
    }
    CheckResult(rc, db_, "ListHabits failed");

    domain::Habit habit{};
    habit.id = ColumnText(statement.get(), 0);
    habit.title = ColumnText(statement.get(), 1);
    habit.cadence = ColumnText(statement.get(), 2);
    habit.is_active = sqlite3_column_int(statement.get(), 3) == 1;
    habit.created_at = ColumnText(statement.get(), 4);
    habits.push_back(std::move(habit));
  }

  return habits;
}

void SqliteRepository::UpsertQuest(const domain::Quest& quest) {
  Statement statement(
      db_,
      R"SQL(
        INSERT INTO quests(id, title, track_type, is_completed, created_at)
        VALUES(?, ?, ?, ?, ?)
        ON CONFLICT(id) DO UPDATE SET
          title = excluded.title,
          track_type = excluded.track_type,
          is_completed = excluded.is_completed;
      )SQL");

  BindText(db_, statement.get(), 1, quest.id);
  BindText(db_, statement.get(), 2, quest.title);
  BindText(db_, statement.get(), 3, std::string(domain::TrackTypeToString(quest.track_type)));
  BindInt(db_, statement.get(), 4, quest.is_completed ? 1 : 0);
  BindText(db_, statement.get(), 5, quest.created_at);

  CheckResult(sqlite3_step(statement.get()), db_, "UpsertQuest failed");
}

std::vector<domain::Quest> SqliteRepository::ListQuests() const {
  Statement statement(
      db_,
      "SELECT id, title, track_type, is_completed, created_at FROM quests ORDER BY created_at ASC;");

  std::vector<domain::Quest> quests;
  while (true) {
    const int rc = sqlite3_step(statement.get());
    if (rc == SQLITE_DONE) {
      break;
    }
    CheckResult(rc, db_, "ListQuests failed");

    domain::Quest quest{};
    quest.id = ColumnText(statement.get(), 0);
    quest.title = ColumnText(statement.get(), 1);
    quest.track_type = domain::TrackTypeFromString(ColumnText(statement.get(), 2));
    quest.is_completed = sqlite3_column_int(statement.get(), 3) == 1;
    quest.created_at = ColumnText(statement.get(), 4);
    quests.push_back(std::move(quest));
  }

  return quests;
}

void SqliteRepository::UpsertActionUnit(const domain::ActionUnit& action_unit) {
  Statement statement(
      db_,
      R"SQL(
        INSERT INTO action_units(
          id,
          parent_id,
          title,
          track_type,
          status,
          runtime_state,
          priority_score,
          started_at,
          completed_at
        )
        VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?)
        ON CONFLICT(id) DO UPDATE SET
          parent_id = excluded.parent_id,
          title = excluded.title,
          track_type = excluded.track_type,
          status = excluded.status,
          runtime_state = excluded.runtime_state,
          priority_score = excluded.priority_score,
          started_at = excluded.started_at,
          completed_at = excluded.completed_at;
      )SQL");

  BindText(db_, statement.get(), 1, action_unit.id);
  BindText(db_, statement.get(), 2, action_unit.parent_id);
  BindText(db_, statement.get(), 3, action_unit.title);
  BindText(db_, statement.get(), 4, std::string(domain::TrackTypeToString(action_unit.track_type)));
  const auto status = StatusFromLifecycle(action_unit.lifecycle_state);
  BindText(db_, statement.get(), 5, std::string(domain::ActionStatusToString(status)));
  BindText(db_, statement.get(), 6, std::string(domain::LifecycleStateToString(action_unit.lifecycle_state)));
  BindInt(db_, statement.get(), 7, std::max(action_unit.priority_score, 0));
  BindText(db_, statement.get(), 8, action_unit.started_at);
  BindText(db_, statement.get(), 9, action_unit.completed_at);

  CheckResult(sqlite3_step(statement.get()), db_, "UpsertActionUnit failed");
}

std::optional<domain::ActionUnit> SqliteRepository::FindActionUnitById(const std::string& id) const {
  Statement statement(
      db_,
      R"SQL(
        SELECT id, parent_id, title, track_type, status, runtime_state, priority_score, started_at, completed_at
        FROM action_units
        WHERE id = ? LIMIT 1;
      )SQL");

  BindText(db_, statement.get(), 1, id);

  const int rc = sqlite3_step(statement.get());
  if (rc == SQLITE_DONE) {
    return std::nullopt;
  }
  CheckResult(rc, db_, "FindActionUnitById failed");

  domain::ActionUnit action_unit{};
  action_unit.id = ColumnText(statement.get(), 0);
  action_unit.parent_id = ColumnText(statement.get(), 1);
  action_unit.title = ColumnText(statement.get(), 2);
  action_unit.track_type = domain::TrackTypeFromString(ColumnText(statement.get(), 3));
  action_unit.status = domain::ActionStatusFromString(ColumnText(statement.get(), 4));
  const auto lifecycle_raw = ColumnText(statement.get(), 5);
  if (lifecycle_raw.empty()) {
    action_unit.lifecycle_state = LifecycleFromLegacyStatus(action_unit.status);
  } else {
    action_unit.lifecycle_state = domain::LifecycleStateFromString(lifecycle_raw);
  }
  action_unit.priority_score = sqlite3_column_int(statement.get(), 6);
  action_unit.started_at = ColumnText(statement.get(), 7);
  action_unit.completed_at = ColumnText(statement.get(), 8);
  return action_unit;
}

std::vector<domain::ActionUnit> SqliteRepository::ListActionUnitsByTrack(const domain::TrackType track_type) const {
  Statement statement(
      db_,
      R"SQL(
        SELECT id, parent_id, title, track_type, status, runtime_state, priority_score, started_at, completed_at
        FROM action_units
        WHERE track_type = ?
        ORDER BY id ASC;
      )SQL");

  BindText(db_, statement.get(), 1, std::string(domain::TrackTypeToString(track_type)));

  std::vector<domain::ActionUnit> action_units;
  while (true) {
    const int rc = sqlite3_step(statement.get());
    if (rc == SQLITE_DONE) {
      break;
    }
    CheckResult(rc, db_, "ListActionUnitsByTrack failed");

    domain::ActionUnit action_unit{};
    action_unit.id = ColumnText(statement.get(), 0);
    action_unit.parent_id = ColumnText(statement.get(), 1);
    action_unit.title = ColumnText(statement.get(), 2);
    action_unit.track_type = domain::TrackTypeFromString(ColumnText(statement.get(), 3));
    action_unit.status = domain::ActionStatusFromString(ColumnText(statement.get(), 4));
    const auto lifecycle_raw = ColumnText(statement.get(), 5);
    if (lifecycle_raw.empty()) {
      action_unit.lifecycle_state = LifecycleFromLegacyStatus(action_unit.status);
    } else {
      action_unit.lifecycle_state = domain::LifecycleStateFromString(lifecycle_raw);
    }
    action_unit.priority_score = sqlite3_column_int(statement.get(), 6);
    action_unit.started_at = ColumnText(statement.get(), 7);
    action_unit.completed_at = ColumnText(statement.get(), 8);
    action_units.push_back(std::move(action_unit));
  }

  return action_units;
}

void SqliteRepository::UpsertLearningGoal(const domain::LearningGoal& goal) {
  Statement statement(
      db_,
      R"SQL(
        INSERT INTO learning_goals(id, title, milestone, confidence_level, created_at)
        VALUES(?, ?, ?, ?, ?)
        ON CONFLICT(id) DO UPDATE SET
          title = excluded.title,
          milestone = excluded.milestone,
          confidence_level = excluded.confidence_level;
      )SQL");

  BindText(db_, statement.get(), 1, goal.id);
  BindText(db_, statement.get(), 2, goal.title);
  BindText(db_, statement.get(), 3, goal.milestone);
  BindInt(db_, statement.get(), 4, goal.confidence_level);
  BindText(db_, statement.get(), 5, goal.created_at);

  CheckResult(sqlite3_step(statement.get()), db_, "UpsertLearningGoal failed");
}

std::optional<domain::LearningGoal> SqliteRepository::FindLearningGoalById(const std::string& id) const {
  Statement statement(
      db_,
      R"SQL(
        SELECT id, title, milestone, confidence_level, created_at
        FROM learning_goals
        WHERE id = ?
        LIMIT 1;
      )SQL");

  BindText(db_, statement.get(), 1, id);

  const int rc = sqlite3_step(statement.get());
  if (rc == SQLITE_DONE) {
    return std::nullopt;
  }
  CheckResult(rc, db_, "FindLearningGoalById failed");

  domain::LearningGoal goal{};
  goal.id = ColumnText(statement.get(), 0);
  goal.title = ColumnText(statement.get(), 1);
  goal.milestone = ColumnText(statement.get(), 2);
  goal.confidence_level = sqlite3_column_int(statement.get(), 3);
  goal.created_at = ColumnText(statement.get(), 4);
  return goal;
}

std::vector<domain::LearningGoal> SqliteRepository::ListLearningGoals() const {
  Statement statement(
      db_,
      R"SQL(
        SELECT id, title, milestone, confidence_level, created_at
        FROM learning_goals
        ORDER BY created_at ASC;
      )SQL");

  std::vector<domain::LearningGoal> goals;
  while (true) {
    const int rc = sqlite3_step(statement.get());
    if (rc == SQLITE_DONE) {
      break;
    }
    CheckResult(rc, db_, "ListLearningGoals failed");

    domain::LearningGoal goal{};
    goal.id = ColumnText(statement.get(), 0);
    goal.title = ColumnText(statement.get(), 1);
    goal.milestone = ColumnText(statement.get(), 2);
    goal.confidence_level = sqlite3_column_int(statement.get(), 3);
    goal.created_at = ColumnText(statement.get(), 4);
    goals.push_back(std::move(goal));
  }

  return goals;
}

void SqliteRepository::UpsertLearningSession(const domain::LearningSession& session) {
  Statement statement(
      db_,
      R"SQL(
        INSERT INTO learning_sessions(
          id,
          goal_id,
          title,
          lifecycle_state,
          priority_score,
          duration_minutes,
          artifact_kind,
          artifact_ref,
          checkpoint_note,
          started_at,
          completed_at
        )
        VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        ON CONFLICT(id) DO UPDATE SET
          goal_id = excluded.goal_id,
          title = excluded.title,
          lifecycle_state = excluded.lifecycle_state,
          priority_score = excluded.priority_score,
          duration_minutes = excluded.duration_minutes,
          artifact_kind = excluded.artifact_kind,
          artifact_ref = excluded.artifact_ref,
          checkpoint_note = excluded.checkpoint_note,
          started_at = excluded.started_at,
          completed_at = excluded.completed_at;
      )SQL");

  BindText(db_, statement.get(), 1, session.id);
  BindText(db_, statement.get(), 2, session.goal_id);
  BindText(db_, statement.get(), 3, session.title);
  BindText(db_, statement.get(), 4, std::string(domain::LifecycleStateToString(session.lifecycle_state)));
  BindInt(db_, statement.get(), 5, std::max(session.priority_score, 0));
  BindInt(db_, statement.get(), 6, std::max(session.duration_minutes, 0));
  BindText(db_, statement.get(), 7, session.artifact_kind);
  BindText(db_, statement.get(), 8, session.artifact_ref);
  BindText(db_, statement.get(), 9, session.checkpoint_note);
  BindText(db_, statement.get(), 10, session.started_at);
  BindText(db_, statement.get(), 11, session.completed_at);

  CheckResult(sqlite3_step(statement.get()), db_, "UpsertLearningSession failed");
}

std::vector<domain::LearningSession> SqliteRepository::ListLearningSessionsByGoal(const std::string& goal_id) const {
  Statement statement(
      db_,
      R"SQL(
        SELECT
          id,
          goal_id,
          title,
          lifecycle_state,
          priority_score,
          duration_minutes,
          artifact_kind,
          artifact_ref,
          checkpoint_note,
          started_at,
          completed_at
        FROM learning_sessions
        WHERE goal_id = ?
        ORDER BY COALESCE(completed_at, started_at, id) ASC;
      )SQL");

  BindText(db_, statement.get(), 1, goal_id);

  std::vector<domain::LearningSession> sessions;
  while (true) {
    const int rc = sqlite3_step(statement.get());
    if (rc == SQLITE_DONE) {
      break;
    }
    CheckResult(rc, db_, "ListLearningSessionsByGoal failed");

    domain::LearningSession session{};
    session.id = ColumnText(statement.get(), 0);
    session.goal_id = ColumnText(statement.get(), 1);
    session.title = ColumnText(statement.get(), 2);
    const auto lifecycle_raw = ColumnText(statement.get(), 3);
    session.lifecycle_state = lifecycle_raw.empty() ? domain::LifecycleState::Completed
                                                    : domain::LifecycleStateFromString(lifecycle_raw);
    session.priority_score = sqlite3_column_int(statement.get(), 4);
    session.duration_minutes = sqlite3_column_int(statement.get(), 5);
    session.artifact_kind = ColumnText(statement.get(), 6);
    session.artifact_ref = ColumnText(statement.get(), 7);
    session.checkpoint_note = ColumnText(statement.get(), 8);
    session.started_at = ColumnText(statement.get(), 9);
    session.completed_at = ColumnText(statement.get(), 10);
    sessions.push_back(std::move(session));
  }

  return sessions;
}

std::vector<domain::LearningSession> SqliteRepository::ListLearningSessions() const {
  Statement statement(
      db_,
      R"SQL(
        SELECT
          id,
          goal_id,
          title,
          lifecycle_state,
          priority_score,
          duration_minutes,
          artifact_kind,
          artifact_ref,
          checkpoint_note,
          started_at,
          completed_at
        FROM learning_sessions
        ORDER BY id ASC;
      )SQL");

  std::vector<domain::LearningSession> sessions;
  while (true) {
    const int rc = sqlite3_step(statement.get());
    if (rc == SQLITE_DONE) {
      break;
    }
    CheckResult(rc, db_, "ListLearningSessions failed");

    domain::LearningSession session{};
    session.id = ColumnText(statement.get(), 0);
    session.goal_id = ColumnText(statement.get(), 1);
    session.title = ColumnText(statement.get(), 2);
    const auto lifecycle_raw = ColumnText(statement.get(), 3);
    session.lifecycle_state = lifecycle_raw.empty() ? domain::LifecycleState::Completed
                                                    : domain::LifecycleStateFromString(lifecycle_raw);
    session.priority_score = sqlite3_column_int(statement.get(), 4);
    session.duration_minutes = sqlite3_column_int(statement.get(), 5);
    session.artifact_kind = ColumnText(statement.get(), 6);
    session.artifact_ref = ColumnText(statement.get(), 7);
    session.checkpoint_note = ColumnText(statement.get(), 8);
    session.started_at = ColumnText(statement.get(), 9);
    session.completed_at = ColumnText(statement.get(), 10);
    sessions.push_back(std::move(session));
  }

  return sessions;
}

void SqliteRepository::UpsertMilestoneCheckpoint(const domain::MilestoneCheckpoint& checkpoint) {
  Statement statement(
      db_,
      R"SQL(
        INSERT INTO milestone_checkpoints(
          id,
          goal_id,
          learning_session_id,
          milestone_key,
          state,
          evidence_kind,
          evidence_ref,
          confidence_level,
          candidate_reason,
          reward_event_id,
          submitted_at,
          reviewed_at,
          confirmed_at,
          rejected_at,
          created_at,
          updated_at
        )
        VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        ON CONFLICT(id) DO UPDATE SET
          goal_id = excluded.goal_id,
          learning_session_id = excluded.learning_session_id,
          milestone_key = excluded.milestone_key,
          state = excluded.state,
          evidence_kind = excluded.evidence_kind,
          evidence_ref = excluded.evidence_ref,
          confidence_level = excluded.confidence_level,
          candidate_reason = excluded.candidate_reason,
          reward_event_id = excluded.reward_event_id,
          submitted_at = excluded.submitted_at,
          reviewed_at = excluded.reviewed_at,
          confirmed_at = excluded.confirmed_at,
          rejected_at = excluded.rejected_at,
          created_at = excluded.created_at,
          updated_at = excluded.updated_at;
      )SQL");

  BindText(db_, statement.get(), 1, checkpoint.id);
  BindText(db_, statement.get(), 2, checkpoint.goal_id);
  BindText(db_, statement.get(), 3, checkpoint.learning_session_id);
  BindText(db_, statement.get(), 4, checkpoint.milestone_key);
  BindText(db_, statement.get(), 5, std::string(domain::MilestoneCheckpointStateToString(checkpoint.state)));
  BindText(db_, statement.get(), 6, checkpoint.evidence_kind);
  BindText(db_, statement.get(), 7, checkpoint.evidence_ref);
  BindInt(db_, statement.get(), 8, std::clamp(checkpoint.confidence_level, 1, 5));
  BindText(db_, statement.get(), 9, checkpoint.candidate_reason);
  BindText(db_, statement.get(), 10, checkpoint.reward_event_id);
  BindText(db_, statement.get(), 11, checkpoint.submitted_at);
  BindText(db_, statement.get(), 12, checkpoint.reviewed_at);
  BindText(db_, statement.get(), 13, checkpoint.confirmed_at);
  BindText(db_, statement.get(), 14, checkpoint.rejected_at);
  BindText(db_, statement.get(), 15, checkpoint.created_at);
  BindText(db_, statement.get(), 16, checkpoint.updated_at);

  CheckResult(sqlite3_step(statement.get()), db_, "UpsertMilestoneCheckpoint failed");
}

std::optional<domain::MilestoneCheckpoint> SqliteRepository::FindMilestoneCheckpointById(const std::string& id) const {
  Statement statement(
      db_,
      R"SQL(
        SELECT
          id,
          goal_id,
          learning_session_id,
          milestone_key,
          state,
          evidence_kind,
          evidence_ref,
          confidence_level,
          candidate_reason,
          reward_event_id,
          submitted_at,
          reviewed_at,
          confirmed_at,
          rejected_at,
          created_at,
          updated_at
        FROM milestone_checkpoints
        WHERE id = ?
        LIMIT 1;
      )SQL");

  BindText(db_, statement.get(), 1, id);

  const int rc = sqlite3_step(statement.get());
  if (rc == SQLITE_DONE) {
    return std::nullopt;
  }
  CheckResult(rc, db_, "FindMilestoneCheckpointById failed");

  domain::MilestoneCheckpoint checkpoint{};
  checkpoint.id = ColumnText(statement.get(), 0);
  checkpoint.goal_id = ColumnText(statement.get(), 1);
  checkpoint.learning_session_id = ColumnText(statement.get(), 2);
  checkpoint.milestone_key = ColumnText(statement.get(), 3);
  checkpoint.state = domain::MilestoneCheckpointStateFromString(ColumnText(statement.get(), 4));
  checkpoint.evidence_kind = ColumnText(statement.get(), 5);
  checkpoint.evidence_ref = ColumnText(statement.get(), 6);
  checkpoint.confidence_level = sqlite3_column_int(statement.get(), 7);
  checkpoint.candidate_reason = ColumnText(statement.get(), 8);
  checkpoint.reward_event_id = ColumnText(statement.get(), 9);
  checkpoint.submitted_at = ColumnText(statement.get(), 10);
  checkpoint.reviewed_at = ColumnText(statement.get(), 11);
  checkpoint.confirmed_at = ColumnText(statement.get(), 12);
  checkpoint.rejected_at = ColumnText(statement.get(), 13);
  checkpoint.created_at = ColumnText(statement.get(), 14);
  checkpoint.updated_at = ColumnText(statement.get(), 15);
  return checkpoint;
}

std::vector<domain::MilestoneCheckpoint> SqliteRepository::ListMilestoneCheckpointsByGoal(
    const std::string& goal_id) const {
  Statement statement(
      db_,
      R"SQL(
        SELECT
          id,
          goal_id,
          learning_session_id,
          milestone_key,
          state,
          evidence_kind,
          evidence_ref,
          confidence_level,
          candidate_reason,
          reward_event_id,
          submitted_at,
          reviewed_at,
          confirmed_at,
          rejected_at,
          created_at,
          updated_at
        FROM milestone_checkpoints
        WHERE goal_id = ?
        ORDER BY submitted_at ASC;
      )SQL");

  BindText(db_, statement.get(), 1, goal_id);

  std::vector<domain::MilestoneCheckpoint> checkpoints;
  while (true) {
    const int rc = sqlite3_step(statement.get());
    if (rc == SQLITE_DONE) {
      break;
    }
    CheckResult(rc, db_, "ListMilestoneCheckpointsByGoal failed");

    domain::MilestoneCheckpoint checkpoint{};
    checkpoint.id = ColumnText(statement.get(), 0);
    checkpoint.goal_id = ColumnText(statement.get(), 1);
    checkpoint.learning_session_id = ColumnText(statement.get(), 2);
    checkpoint.milestone_key = ColumnText(statement.get(), 3);
    checkpoint.state = domain::MilestoneCheckpointStateFromString(ColumnText(statement.get(), 4));
    checkpoint.evidence_kind = ColumnText(statement.get(), 5);
    checkpoint.evidence_ref = ColumnText(statement.get(), 6);
    checkpoint.confidence_level = sqlite3_column_int(statement.get(), 7);
    checkpoint.candidate_reason = ColumnText(statement.get(), 8);
    checkpoint.reward_event_id = ColumnText(statement.get(), 9);
    checkpoint.submitted_at = ColumnText(statement.get(), 10);
    checkpoint.reviewed_at = ColumnText(statement.get(), 11);
    checkpoint.confirmed_at = ColumnText(statement.get(), 12);
    checkpoint.rejected_at = ColumnText(statement.get(), 13);
    checkpoint.created_at = ColumnText(statement.get(), 14);
    checkpoint.updated_at = ColumnText(statement.get(), 15);
    checkpoints.push_back(std::move(checkpoint));
  }

  return checkpoints;
}

std::vector<domain::MilestoneCheckpoint> SqliteRepository::ListMilestoneCheckpoints() const {
  Statement statement(
      db_,
      R"SQL(
        SELECT
          id,
          goal_id,
          learning_session_id,
          milestone_key,
          state,
          evidence_kind,
          evidence_ref,
          confidence_level,
          candidate_reason,
          reward_event_id,
          submitted_at,
          reviewed_at,
          confirmed_at,
          rejected_at,
          created_at,
          updated_at
        FROM milestone_checkpoints
        ORDER BY submitted_at ASC;
      )SQL");

  std::vector<domain::MilestoneCheckpoint> checkpoints;
  while (true) {
    const int rc = sqlite3_step(statement.get());
    if (rc == SQLITE_DONE) {
      break;
    }
    CheckResult(rc, db_, "ListMilestoneCheckpoints failed");

    domain::MilestoneCheckpoint checkpoint{};
    checkpoint.id = ColumnText(statement.get(), 0);
    checkpoint.goal_id = ColumnText(statement.get(), 1);
    checkpoint.learning_session_id = ColumnText(statement.get(), 2);
    checkpoint.milestone_key = ColumnText(statement.get(), 3);
    checkpoint.state = domain::MilestoneCheckpointStateFromString(ColumnText(statement.get(), 4));
    checkpoint.evidence_kind = ColumnText(statement.get(), 5);
    checkpoint.evidence_ref = ColumnText(statement.get(), 6);
    checkpoint.confidence_level = sqlite3_column_int(statement.get(), 7);
    checkpoint.candidate_reason = ColumnText(statement.get(), 8);
    checkpoint.reward_event_id = ColumnText(statement.get(), 9);
    checkpoint.submitted_at = ColumnText(statement.get(), 10);
    checkpoint.reviewed_at = ColumnText(statement.get(), 11);
    checkpoint.confirmed_at = ColumnText(statement.get(), 12);
    checkpoint.rejected_at = ColumnText(statement.get(), 13);
    checkpoint.created_at = ColumnText(statement.get(), 14);
    checkpoint.updated_at = ColumnText(statement.get(), 15);
    checkpoints.push_back(std::move(checkpoint));
  }

  return checkpoints;
}

void SqliteRepository::AppendRewardEvent(const domain::RewardEvent& reward_event) {
  Statement statement(
      db_,
      R"SQL(
        INSERT INTO reward_events(id, source_type, source_id, track_type, xp_delta, reward_kind, created_at)
        VALUES(?, ?, ?, ?, ?, ?, ?)
        ON CONFLICT(id) DO NOTHING;
      )SQL");

  BindText(db_, statement.get(), 1, reward_event.id);
  BindText(db_, statement.get(), 2, reward_event.source_type);
  BindText(db_, statement.get(), 3, reward_event.source_id);
  BindText(db_, statement.get(), 4, std::string(domain::TrackTypeToString(reward_event.track_type)));
  BindInt(db_, statement.get(), 5, reward_event.xp_delta);
  BindText(db_, statement.get(), 6, reward_event.reward_kind);
  BindText(db_, statement.get(), 7, reward_event.created_at);

  CheckResult(sqlite3_step(statement.get()), db_, "AppendRewardEvent failed");
}

std::vector<domain::RewardEvent> SqliteRepository::ListRewardEventsByTrack(const domain::TrackType track_type) const {
  Statement statement(
      db_,
      R"SQL(
        SELECT id, source_type, source_id, track_type, xp_delta, reward_kind, created_at
        FROM reward_events
        WHERE track_type = ?
        ORDER BY created_at ASC;
      )SQL");

  BindText(db_, statement.get(), 1, std::string(domain::TrackTypeToString(track_type)));

  std::vector<domain::RewardEvent> reward_events;
  while (true) {
    const int rc = sqlite3_step(statement.get());
    if (rc == SQLITE_DONE) {
      break;
    }
    CheckResult(rc, db_, "ListRewardEventsByTrack failed");

    domain::RewardEvent event{};
    event.id = ColumnText(statement.get(), 0);
    event.source_type = ColumnText(statement.get(), 1);
    event.source_id = ColumnText(statement.get(), 2);
    event.track_type = domain::TrackTypeFromString(ColumnText(statement.get(), 3));
    event.xp_delta = sqlite3_column_int(statement.get(), 4);
    event.reward_kind = ColumnText(statement.get(), 5);
    event.created_at = ColumnText(statement.get(), 6);
    reward_events.push_back(std::move(event));
  }

  return reward_events;
}

domain::UserState SqliteRepository::LoadUserState() const {
  Statement statement(
      db_,
      "SELECT level, total_xp, life_xp, learning_xp, recovery_tokens FROM user_state WHERE id = 1;");

  const int rc = sqlite3_step(statement.get());
  if (rc == SQLITE_DONE) {
    return {};
  }
  CheckResult(rc, db_, "LoadUserState failed");

  domain::UserState state{};
  state.level = sqlite3_column_int(statement.get(), 0);
  state.total_xp = sqlite3_column_int(statement.get(), 1);
  state.life_xp = sqlite3_column_int(statement.get(), 2);
  state.learning_xp = sqlite3_column_int(statement.get(), 3);
  state.recovery_tokens = sqlite3_column_int(statement.get(), 4);
  return state;
}

void SqliteRepository::SaveUserState(const domain::UserState& user_state) {
  Statement statement(
      db_,
      R"SQL(
        INSERT INTO user_state(id, level, total_xp, life_xp, learning_xp, recovery_tokens)
        VALUES(1, ?, ?, ?, ?, ?)
        ON CONFLICT(id) DO UPDATE SET
          level = excluded.level,
          total_xp = excluded.total_xp,
          life_xp = excluded.life_xp,
          learning_xp = excluded.learning_xp,
          recovery_tokens = excluded.recovery_tokens;
      )SQL");

  BindInt(db_, statement.get(), 1, user_state.level);
  BindInt(db_, statement.get(), 2, user_state.total_xp);
  BindInt(db_, statement.get(), 3, user_state.life_xp);
  BindInt(db_, statement.get(), 4, user_state.learning_xp);
  BindInt(db_, statement.get(), 5, user_state.recovery_tokens);

  CheckResult(sqlite3_step(statement.get()), db_, "SaveUserState failed");
}

UiPreferences SqliteRepository::LoadUiPreferences() const {
  Statement statement(
      db_,
      R"SQL(
        SELECT
          preset_mode,
          last_non_custom_preset,
          motion_level,
          sound_level,
          density_level,
          queue_mode,
          prompt_concurrency_limit,
          nudge_cooldown_seconds,
          updated_at
        FROM ui_preferences
        WHERE id = 1;
      )SQL");

  const int rc = sqlite3_step(statement.get());
  if (rc == SQLITE_DONE) {
    return {};
  }
  CheckResult(rc, db_, "LoadUiPreferences failed");

  UiPreferences preferences{};
  preferences.preset_mode = ui::contracts::PresetModeFromString(ColumnText(statement.get(), 0));
  preferences.last_non_custom_preset = ui::contracts::PresetModeFromString(ColumnText(statement.get(), 1));
  if (preferences.last_non_custom_preset == ui::contracts::PresetMode::Custom) {
    preferences.last_non_custom_preset = ui::contracts::PresetMode::Calm;
  }
  preferences.motion_level = sqlite3_column_int(statement.get(), 2);
  preferences.sound_level = sqlite3_column_int(statement.get(), 3);
  preferences.density_level = sqlite3_column_int(statement.get(), 4);
  preferences.queue_mode = ui::contracts::TrackFilterFromQueueModeString(ColumnText(statement.get(), 5));
  preferences.prompt_concurrency_limit = sqlite3_column_int(statement.get(), 6);
  preferences.nudge_cooldown_seconds = sqlite3_column_int(statement.get(), 7);
  preferences.updated_at = ColumnText(statement.get(), 8);
  return preferences;
}

void SqliteRepository::SaveUiPreferences(const UiPreferences& preferences) {
  Statement statement(
      db_,
      R"SQL(
        INSERT INTO ui_preferences(
          id,
          preset_mode,
          last_non_custom_preset,
          motion_level,
          sound_level,
          density_level,
          queue_mode,
          prompt_concurrency_limit,
          nudge_cooldown_seconds,
          updated_at
        )
        VALUES(1, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        ON CONFLICT(id) DO UPDATE SET
          preset_mode = excluded.preset_mode,
          last_non_custom_preset = excluded.last_non_custom_preset,
          motion_level = excluded.motion_level,
          sound_level = excluded.sound_level,
          density_level = excluded.density_level,
          queue_mode = excluded.queue_mode,
          prompt_concurrency_limit = excluded.prompt_concurrency_limit,
          nudge_cooldown_seconds = excluded.nudge_cooldown_seconds,
          updated_at = excluded.updated_at;
      )SQL");

  BindText(db_, statement.get(), 1, std::string(ui::contracts::PresetModeToString(preferences.preset_mode)));
  BindText(
      db_,
      statement.get(),
      2,
      std::string(ui::contracts::PresetModeToString(preferences.last_non_custom_preset)));
  BindInt(db_, statement.get(), 3, preferences.motion_level);
  BindInt(db_, statement.get(), 4, preferences.sound_level);
  BindInt(db_, statement.get(), 5, preferences.density_level);
  BindText(db_, statement.get(), 6, std::string(ui::contracts::TrackFilterToQueueModeString(preferences.queue_mode)));
  BindInt(db_, statement.get(), 7, preferences.prompt_concurrency_limit);
  BindInt(db_, statement.get(), 8, preferences.nudge_cooldown_seconds);
  BindText(db_, statement.get(), 9, preferences.updated_at);

  CheckResult(sqlite3_step(statement.get()), db_, "SaveUiPreferences failed");
}

void SqliteRepository::ExecOrThrow(const std::string& sql) const {
  char* error_message = nullptr;
  const int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &error_message);
  if (rc != SQLITE_OK) {
    const std::string details = error_message != nullptr ? error_message : "unknown sqlite error";
    sqlite3_free(error_message);
    throw std::runtime_error("SQLite exec failed: " + details);
  }
}

}  // namespace habitrpg::data
