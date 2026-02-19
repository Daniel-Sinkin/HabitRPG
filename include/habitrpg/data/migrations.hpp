#pragma once

#include <sqlite3.h>

namespace habitrpg::data {

inline constexpr int kSchemaVersionV1 = 1;
inline constexpr int kSchemaVersionV2 = 2;
inline constexpr int kSchemaVersionV3 = 3;

int ReadSchemaVersion(sqlite3* db);
void RunMigrations(sqlite3* db, int target_version = kSchemaVersionV3);

}  // namespace habitrpg::data
