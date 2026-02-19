#include "habitrpg/app/app_state.hpp"

#include <exception>
#include <string>

#include "habitrpg/data/sqlite_repository.hpp"

namespace habitrpg::app {

bool StartupSmokeCheck(const std::string& sqlite_path, std::string* error_out) {
  try {
    data::SqliteRepository repository(sqlite_path);
    repository.Migrate();

    const auto user_state = repository.LoadUserState();
    if (user_state.level < 1) {
      if (error_out != nullptr) {
        *error_out = "Invalid initial user state level";
      }
      return false;
    }

    return true;
  } catch (const std::exception& ex) {
    if (error_out != nullptr) {
      *error_out = ex.what();
    }
    return false;
  }
}

}  // namespace habitrpg::app
