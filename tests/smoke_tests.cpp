#include <filesystem>
#include <stdexcept>
#include <string>

#include "habitrpg/app/app_state.hpp"
#include "habitrpg/domain/entities.hpp"

namespace {

std::string BuildTempDbPath(const std::string& suffix) {
  const auto temp_dir = std::filesystem::temp_directory_path();
  const auto file_name = "habitrpg_test_" + suffix + "_" + habitrpg::domain::GenerateStableId("db") + ".sqlite3";
  return (temp_dir / file_name).string();
}

}  // namespace

bool RunStartupSmokeTest() {
  const std::string sqlite_path = BuildTempDbPath("startup_smoke");

  std::string error;
  const bool ok = habitrpg::app::StartupSmokeCheck(sqlite_path, &error);

  std::error_code remove_error;
  std::filesystem::remove(sqlite_path, remove_error);

  if (!ok) {
    throw std::runtime_error("StartupSmokeCheck failed: " + error);
  }

  return true;
}
