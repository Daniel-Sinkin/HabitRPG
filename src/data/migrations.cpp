#include "habitrpg/data/migrations.hpp"

#include <stdexcept>
#include <string>

namespace habitrpg::data {
namespace {

void ExecOrThrow(sqlite3* db, const char* sql) {
  char* error_message = nullptr;
  const int rc = sqlite3_exec(db, sql, nullptr, nullptr, &error_message);
  if (rc != SQLITE_OK) {
    const std::string details = error_message != nullptr ? error_message : "unknown sqlite error";
    sqlite3_free(error_message);
    throw std::runtime_error("SQLite exec failed: " + details);
  }
}

bool ColumnExists(sqlite3* db, const char* table_name, const char* column_name) {
  const std::string sql = "PRAGMA table_info(" + std::string(table_name) + ");";
  sqlite3_stmt* statement = nullptr;
  const int prepare_rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &statement, nullptr);
  if (prepare_rc != SQLITE_OK) {
    throw std::runtime_error("Failed to inspect table_info(" + std::string(table_name) + ")");
  }

  bool exists = false;
  while (sqlite3_step(statement) == SQLITE_ROW) {
    const auto* raw_name = sqlite3_column_text(statement, 1);
    const std::string name = raw_name != nullptr ? reinterpret_cast<const char*>(raw_name) : "";
    if (name == column_name) {
      exists = true;
      break;
    }
  }

  sqlite3_finalize(statement);
  return exists;
}

void EnsureSchemaMeta(sqlite3* db) {
  ExecOrThrow(db, R"SQL(
    CREATE TABLE IF NOT EXISTS schema_meta (
      id INTEGER PRIMARY KEY CHECK(id = 1),
      version INTEGER NOT NULL
    );
  )SQL");

  ExecOrThrow(db, R"SQL(
    INSERT INTO schema_meta(id, version)
    VALUES(1, 0)
    ON CONFLICT(id) DO NOTHING;
  )SQL");
}

void ApplyV1(sqlite3* db) {
  ExecOrThrow(db, "BEGIN TRANSACTION;");

  try {
    ExecOrThrow(db, R"SQL(
      CREATE TABLE IF NOT EXISTS habits (
        id TEXT PRIMARY KEY,
        title TEXT NOT NULL,
        cadence TEXT NOT NULL,
        is_active INTEGER NOT NULL,
        created_at TEXT NOT NULL
      );
    )SQL");

    ExecOrThrow(db, R"SQL(
      CREATE TABLE IF NOT EXISTS quests (
        id TEXT PRIMARY KEY,
        title TEXT NOT NULL,
        track_type TEXT NOT NULL CHECK(track_type IN ('life', 'learning')),
        is_completed INTEGER NOT NULL,
        created_at TEXT NOT NULL
      );
    )SQL");

    ExecOrThrow(db, R"SQL(
      CREATE TABLE IF NOT EXISTS action_units (
        id TEXT PRIMARY KEY,
        parent_id TEXT NOT NULL,
        title TEXT NOT NULL,
        track_type TEXT NOT NULL CHECK(track_type IN ('life', 'learning')),
        status TEXT NOT NULL CHECK(status IN ('todo', 'in_progress', 'completed')),
        completed_at TEXT
      );
    )SQL");

    ExecOrThrow(db, R"SQL(
      CREATE TABLE IF NOT EXISTS learning_goals (
        id TEXT PRIMARY KEY,
        title TEXT NOT NULL,
        milestone TEXT NOT NULL,
        confidence_level INTEGER NOT NULL,
        created_at TEXT NOT NULL
      );
    )SQL");

    ExecOrThrow(db, R"SQL(
      CREATE TABLE IF NOT EXISTS learning_sessions (
        id TEXT PRIMARY KEY,
        goal_id TEXT NOT NULL,
        duration_minutes INTEGER NOT NULL,
        artifact_kind TEXT,
        artifact_ref TEXT,
        completed_at TEXT NOT NULL,
        FOREIGN KEY(goal_id) REFERENCES learning_goals(id)
      );
    )SQL");

    ExecOrThrow(db, R"SQL(
      CREATE TABLE IF NOT EXISTS reward_events (
        id TEXT PRIMARY KEY,
        source_type TEXT NOT NULL,
        source_id TEXT NOT NULL,
        track_type TEXT NOT NULL CHECK(track_type IN ('life', 'learning')),
        xp_delta INTEGER NOT NULL,
        reward_kind TEXT NOT NULL,
        created_at TEXT NOT NULL
      );
    )SQL");

    ExecOrThrow(db, R"SQL(
      CREATE TABLE IF NOT EXISTS user_state (
        id INTEGER PRIMARY KEY CHECK(id = 1),
        level INTEGER NOT NULL,
        total_xp INTEGER NOT NULL,
        life_xp INTEGER NOT NULL,
        learning_xp INTEGER NOT NULL,
        recovery_tokens INTEGER NOT NULL
      );
    )SQL");

    ExecOrThrow(db, R"SQL(
      INSERT INTO user_state(id, level, total_xp, life_xp, learning_xp, recovery_tokens)
      VALUES(1, 1, 0, 0, 0, 3)
      ON CONFLICT(id) DO NOTHING;
    )SQL");

    ExecOrThrow(db, "UPDATE schema_meta SET version = 1 WHERE id = 1;");
    ExecOrThrow(db, "COMMIT;");
  } catch (...) {
    ExecOrThrow(db, "ROLLBACK;");
    throw;
  }
}

void ApplyV2(sqlite3* db) {
  ExecOrThrow(db, "BEGIN TRANSACTION;");

  try {
    if (!ColumnExists(db, "action_units", "runtime_state")) {
      ExecOrThrow(db, "ALTER TABLE action_units ADD COLUMN runtime_state TEXT NOT NULL DEFAULT 'ready';");
    }
    if (!ColumnExists(db, "action_units", "priority_score")) {
      ExecOrThrow(db, "ALTER TABLE action_units ADD COLUMN priority_score INTEGER NOT NULL DEFAULT 100;");
    }
    if (!ColumnExists(db, "action_units", "started_at")) {
      ExecOrThrow(db, "ALTER TABLE action_units ADD COLUMN started_at TEXT;");
    }

    ExecOrThrow(db, R"SQL(
      UPDATE action_units
      SET runtime_state = CASE
        WHEN status = 'completed' THEN 'completed'
        WHEN status = 'in_progress' THEN 'active'
        ELSE 'ready'
      END;
    )SQL");

    const bool has_title = ColumnExists(db, "learning_sessions", "title");
    const bool has_state = ColumnExists(db, "learning_sessions", "lifecycle_state");
    const bool has_priority = ColumnExists(db, "learning_sessions", "priority_score");
    const bool has_checkpoint_note = ColumnExists(db, "learning_sessions", "checkpoint_note");
    const bool has_started_at = ColumnExists(db, "learning_sessions", "started_at");

    if (!(has_title && has_state && has_priority && has_checkpoint_note && has_started_at)) {
      ExecOrThrow(db, "ALTER TABLE learning_sessions RENAME TO learning_sessions_legacy;");

      ExecOrThrow(db, R"SQL(
        CREATE TABLE learning_sessions (
          id TEXT PRIMARY KEY,
          goal_id TEXT NOT NULL,
          title TEXT NOT NULL,
          lifecycle_state TEXT NOT NULL CHECK(
            lifecycle_state IN (
              'ready',
              'active',
              'partial',
              'missed',
              'paused',
              'completed',
              'checkpoint_candidate'
            )
          ),
          priority_score INTEGER NOT NULL,
          duration_minutes INTEGER NOT NULL,
          artifact_kind TEXT,
          artifact_ref TEXT,
          checkpoint_note TEXT,
          started_at TEXT,
          completed_at TEXT,
          FOREIGN KEY(goal_id) REFERENCES learning_goals(id)
        );
      )SQL");

      const std::string title_expr = has_title ? "COALESCE(title, 'Learning Session')" : "'Learning Session'";
      const std::string state_expr = has_state ? "COALESCE(lifecycle_state, 'completed')" : "'completed'";
      const std::string priority_expr = has_priority ? "COALESCE(priority_score, 100)" : "100";
      const std::string checkpoint_expr = has_checkpoint_note ? "checkpoint_note" : "NULL";
      const std::string started_expr = has_started_at ? "started_at" : "completed_at";

      const std::string migrate_sql =
          "INSERT INTO learning_sessions(" \
          "id, goal_id, title, lifecycle_state, priority_score, duration_minutes, artifact_kind, artifact_ref, " \
          "checkpoint_note, started_at, completed_at) " \
          "SELECT id, goal_id, " +
          title_expr + ", " + state_expr + ", " + priority_expr +
          ", duration_minutes, artifact_kind, artifact_ref, " + checkpoint_expr + ", " + started_expr +
          ", completed_at "
          "FROM learning_sessions_legacy;";
      ExecOrThrow(db, migrate_sql.c_str());

      ExecOrThrow(db, "DROP TABLE learning_sessions_legacy;");
    }

    ExecOrThrow(db, "UPDATE schema_meta SET version = 2 WHERE id = 1;");
    ExecOrThrow(db, "COMMIT;");
  } catch (...) {
    ExecOrThrow(db, "ROLLBACK;");
    throw;
  }
}

void ApplyV3(sqlite3* db) {
  ExecOrThrow(db, "BEGIN TRANSACTION;");

  try {
    ExecOrThrow(db, R"SQL(
      CREATE TABLE IF NOT EXISTS milestone_checkpoints (
        id TEXT PRIMARY KEY,
        goal_id TEXT NOT NULL,
        learning_session_id TEXT NOT NULL,
        milestone_key TEXT NOT NULL,
        state TEXT NOT NULL CHECK(state IN ('candidate', 'confirmed', 'rejected')),
        evidence_kind TEXT NOT NULL CHECK(evidence_kind IN ('note', 'snippet', 'exercise', 'reference')),
        evidence_ref TEXT,
        confidence_level INTEGER NOT NULL CHECK(confidence_level BETWEEN 1 AND 5),
        candidate_reason TEXT,
        reward_event_id TEXT UNIQUE,
        submitted_at TEXT NOT NULL,
        reviewed_at TEXT,
        confirmed_at TEXT,
        rejected_at TEXT,
        created_at TEXT NOT NULL,
        updated_at TEXT NOT NULL,
        FOREIGN KEY(goal_id) REFERENCES learning_goals(id),
        FOREIGN KEY(learning_session_id) REFERENCES learning_sessions(id)
      );
    )SQL");

    ExecOrThrow(db, R"SQL(
      CREATE TABLE IF NOT EXISTS ui_preferences (
        id INTEGER PRIMARY KEY CHECK(id = 1),
        preset_mode TEXT NOT NULL CHECK(preset_mode IN ('calm', 'spark', 'custom')),
        last_non_custom_preset TEXT NOT NULL CHECK(last_non_custom_preset IN ('calm', 'spark')),
        motion_level INTEGER NOT NULL CHECK(motion_level BETWEEN 0 AND 2),
        sound_level INTEGER NOT NULL CHECK(sound_level BETWEEN 0 AND 2),
        density_level INTEGER NOT NULL CHECK(density_level BETWEEN 0 AND 2),
        queue_mode TEXT NOT NULL CHECK(queue_mode IN ('mixed', 'life_only', 'learning_only')),
        prompt_concurrency_limit INTEGER NOT NULL,
        nudge_cooldown_seconds INTEGER NOT NULL,
        updated_at TEXT NOT NULL
      );
    )SQL");

    ExecOrThrow(db, R"SQL(
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
      VALUES(1, 'calm', 'calm', 0, 0, 2, 'mixed', 2, 30, '1970-01-01T00:00:00Z')
      ON CONFLICT(id) DO NOTHING;
    )SQL");

    ExecOrThrow(db, "UPDATE schema_meta SET version = 3 WHERE id = 1;");
    ExecOrThrow(db, "COMMIT;");
  } catch (...) {
    ExecOrThrow(db, "ROLLBACK;");
    throw;
  }
}

}  // namespace

int ReadSchemaVersion(sqlite3* db) {
  if (db == nullptr) {
    throw std::invalid_argument("ReadSchemaVersion requires a non-null sqlite handle");
  }

  EnsureSchemaMeta(db);

  sqlite3_stmt* statement = nullptr;
  const int prepare_result = sqlite3_prepare_v2(
      db,
      "SELECT version FROM schema_meta WHERE id = 1;",
      -1,
      &statement,
      nullptr);

  if (prepare_result != SQLITE_OK) {
    throw std::runtime_error("Failed to prepare schema version select statement");
  }

  int version = 0;
  const int step_result = sqlite3_step(statement);
  if (step_result == SQLITE_ROW) {
    version = sqlite3_column_int(statement, 0);
  } else if (step_result != SQLITE_DONE) {
    sqlite3_finalize(statement);
    throw std::runtime_error("Failed reading schema version");
  }

  sqlite3_finalize(statement);
  return version;
}

void RunMigrations(sqlite3* db, const int target_version) {
  if (db == nullptr) {
    throw std::invalid_argument("RunMigrations requires a non-null sqlite handle");
  }

  if (target_version < 0) {
    throw std::invalid_argument("RunMigrations target_version must be >= 0");
  }

  int current_version = ReadSchemaVersion(db);
  if (current_version < 1 && target_version >= 1) {
    ApplyV1(db);
    current_version = ReadSchemaVersion(db);
  }

  if (current_version < 2 && target_version >= 2) {
    ApplyV2(db);
    current_version = ReadSchemaVersion(db);
  }

  if (current_version < 3 && target_version >= 3) {
    ApplyV3(db);
  }

  const int final_version = ReadSchemaVersion(db);
  if (final_version < target_version) {
    throw std::runtime_error("Database schema migration incomplete");
  }
}

}  // namespace habitrpg::data
