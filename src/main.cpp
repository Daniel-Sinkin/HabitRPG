#include <exception>
#include <iostream>
#include <string>

#include "habitrpg/app/application.hpp"

int main() {
  try {
    habitrpg::app::Application application("habitrpg.sqlite3");
    return application.Run();
  } catch (const std::exception& ex) {
    std::cerr << "Fatal startup error: " << ex.what() << '\n';
    return 1;
  }
}
