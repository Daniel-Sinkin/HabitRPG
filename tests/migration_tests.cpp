#include <filesystem>
#include <stdexcept>
#include <string>

#include <sqlite3.h>

#include "habitrpg/data/migrations.hpp"
#include "habitrpg/domain/entities.hpp"

namespace {

void Expect(bool condition, const std::string& message) {
  if (!condition) {
    throw std::runtime_error(message);
  }
}

std::string BuildTempDbPath(const std::string& suffix) {
  const auto temp_dir = std::filesystem::temp_directory_path();
  const auto file_name = "habitrpg_test_" + suffix + "_" + habitrpg::domain::GenerateStableId("db") + ".sqlite3";
  return (temp_dir / file_name).string();
}

void Exec(sqlite3* db, const char* sql) {
  char* error = nullptr;
  const int rc = sqlite3_exec(db, sql, nullptr, nullptr, &error);
  if (rc != SQLITE_OK) {
    const std::string details = error != nullptr ? error : "sqlite exec failed";
    sqlite3_free(error);
    throw std::runtime_error(details);
  }
}

bool ColumnExists(sqlite3* db, const char* table, const char* column) {
  const std::string sql = "PRAGMA table_info(" + std::string(table) + ");";
  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    throw std::runtime_error("Failed to query table_info");
  }

  bool exists = false;
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    const auto* raw = sqlite3_column_text(stmt, 1);
    const std::string name = raw != nullptr ? reinterpret_cast<const char*>(raw) : "";
    if (name == column) {
      exists = true;
      break;
    }
  }

  sqlite3_finalize(stmt);
  return exists;
}

std::string QueryText(sqlite3* db, const std::string& sql) {
  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    throw std::runtime_error("Failed to prepare query");
  }

  std::string value;
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    const auto* raw = sqlite3_column_text(stmt, 0);
    value = raw != nullptr ? reinterpret_cast<const char*>(raw) : "";
  }

  sqlite3_finalize(stmt);
  return value;
}

int QueryInt(sqlite3* db, const std::string& sql) {
  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    throw std::runtime_error("Failed to prepare query");
  }

  int value = 0;
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    value = sqlite3_column_int(stmt, 0);
  }

  sqlite3_finalize(stmt);
  return value;
}

bool TableExists(sqlite3* db, const std::string& table_name) {
  return QueryInt(
             db,
             "SELECT COUNT(*) FROM sqlite_master WHERE type = 'table' AND name = '" + table_name + "';") > 0;
}

}  // namespace

bool RunSchemaMigrationV1ToV3Test() {
  const std::string sqlite_path = BuildTempDbPath("migration_v1_v3");
  sqlite3* db = nullptr;
  const int open_rc = sqlite3_open_v2(
      sqlite_path.c_str(),
      &db,
      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX,
      nullptr);
  if (open_rc != SQLITE_OK || db == nullptr) {
    throw std::runtime_error("Failed to open sqlite test database");
  }

  try {
    Exec(db, "CREATE TABLE schema_meta (id INTEGER PRIMARY KEY CHECK(id = 1), version INTEGER NOT NULL);");
    Exec(db, "INSERT INTO schema_meta(id, version) VALUES(1, 1);");

    Exec(db, R"SQL(
      CREATE TABLE action_units (
        id TEXT PRIMARY KEY,
        parent_id TEXT NOT NULL,
        title TEXT NOT NULL,
        track_type TEXT NOT NULL,
        status TEXT NOT NULL,
        completed_at TEXT
      );
    )SQL");

    Exec(db, R"SQL(
      CREATE TABLE learning_goals (
        id TEXT PRIMARY KEY,
        title TEXT NOT NULL,
        milestone TEXT NOT NULL,
        confidence_level INTEGER NOT NULL,
        created_at TEXT NOT NULL
      );
    )SQL");

    Exec(db, R"SQL(
      CREATE TABLE learning_sessions (
        id TEXT PRIMARY KEY,
        goal_id TEXT NOT NULL,
        duration_minutes INTEGER NOT NULL,
        artifact_kind TEXT,
        artifact_ref TEXT,
        completed_at TEXT NOT NULL,
        FOREIGN KEY(goal_id) REFERENCES learning_goals(id)
      );
    )SQL");

    Exec(
        db,
        "INSERT INTO learning_goals(id, title, milestone, confidence_level, created_at) VALUES"
        "('goal_1', 'Goal', 'Milestone', 1, '2026-02-19T00:00:00Z');");
    Exec(
        db,
        "INSERT INTO action_units(id, parent_id, title, track_type, status, completed_at) VALUES"
        "('action_1', 'habit_1', 'Action', 'life', 'in_progress', NULL);");
    Exec(
        db,
        "INSERT INTO learning_sessions(id, goal_id, duration_minutes, artifact_kind, artifact_ref, completed_at) VALUES"
        "('session_1', 'goal_1', 30, 'code', 'snippet.cpp', '2026-02-19T01:00:00Z');");

    habitrpg::data::RunMigrations(db, habitrpg::data::kSchemaVersionV3);

    Expect(
        habitrpg::data::ReadSchemaVersion(db) == habitrpg::data::kSchemaVersionV3,
        "Schema should migrate to v3");

    Expect(ColumnExists(db, "action_units", "runtime_state"), "action_units.runtime_state should exist");
    Expect(ColumnExists(db, "action_units", "priority_score"), "action_units.priority_score should exist");
    Expect(ColumnExists(db, "learning_sessions", "title"), "learning_sessions.title should exist");
    Expect(
        ColumnExists(db, "learning_sessions", "lifecycle_state"),
        "learning_sessions.lifecycle_state should exist");
    Expect(
        ColumnExists(db, "learning_sessions", "checkpoint_note"),
        "learning_sessions.checkpoint_note should exist");

    Expect(
        QueryText(db, "SELECT runtime_state FROM action_units WHERE id = 'action_1';") == "active",
        "In-progress action should migrate to active lifecycle");
    Expect(
        QueryInt(db, "SELECT priority_score FROM action_units WHERE id = 'action_1';") == 100,
        "Default action priority score should be 100");
    Expect(
        QueryText(db, "SELECT lifecycle_state FROM learning_sessions WHERE id = 'session_1';") == "completed",
        "Legacy learning sessions should migrate as completed");
    Expect(
        QueryText(db, "SELECT title FROM learning_sessions WHERE id = 'session_1';") == "Learning Session",
        "Legacy learning sessions should receive deterministic title fallback");

    Expect(TableExists(db, "milestone_checkpoints"), "milestone_checkpoints table should exist");
    Expect(TableExists(db, "ui_preferences"), "ui_preferences table should exist");
    Expect(ColumnExists(db, "milestone_checkpoints", "state"), "milestone_checkpoints.state should exist");
    Expect(
        ColumnExists(db, "milestone_checkpoints", "reward_event_id"),
        "milestone_checkpoints.reward_event_id should exist");
    Expect(ColumnExists(db, "ui_preferences", "preset_mode"), "ui_preferences.preset_mode should exist");
    Expect(ColumnExists(db, "ui_preferences", "queue_mode"), "ui_preferences.queue_mode should exist");
    Expect(
        QueryText(db, "SELECT preset_mode FROM ui_preferences WHERE id = 1;") == "calm",
        "ui_preferences default preset_mode should be calm");
    Expect(
        QueryText(db, "SELECT last_non_custom_preset FROM ui_preferences WHERE id = 1;") == "calm",
        "ui_preferences default last_non_custom_preset should be calm");
    Expect(
        QueryInt(db, "SELECT motion_level FROM ui_preferences WHERE id = 1;") == 0,
        "ui_preferences default motion_level should be 0");
    Expect(
        QueryInt(db, "SELECT sound_level FROM ui_preferences WHERE id = 1;") == 0,
        "ui_preferences default sound_level should be 0");
    Expect(
        QueryInt(db, "SELECT density_level FROM ui_preferences WHERE id = 1;") == 2,
        "ui_preferences default density_level should be 2");
    Expect(
        QueryText(db, "SELECT queue_mode FROM ui_preferences WHERE id = 1;") == "mixed",
        "ui_preferences default queue_mode should be mixed");
  } catch (...) {
    sqlite3_close(db);
    std::error_code remove_error;
    std::filesystem::remove(sqlite_path, remove_error);
    throw;
  }

  sqlite3_close(db);
  std::error_code remove_error;
  std::filesystem::remove(sqlite_path, remove_error);
  return true;
}
