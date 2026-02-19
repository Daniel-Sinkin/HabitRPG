#pragma once

#include <string>

#include <SDL3/SDL.h>

#include "habitrpg/app/app_state.hpp"
#include "habitrpg/data/sqlite_repository.hpp"
#include "habitrpg/domain/interaction_flow.hpp"
#include "habitrpg/domain/reward_engine.hpp"
#include "habitrpg/domain/today_queue.hpp"
#include "habitrpg/ui/dockspace_shell.hpp"

namespace habitrpg::app {

class Application {
 public:
  explicit Application(std::string sqlite_path);
  ~Application();

  Application(const Application&) = delete;
  Application& operator=(const Application&) = delete;
  Application(Application&&) = delete;
  Application& operator=(Application&&) = delete;

  int Run();

 private:
  bool Initialize();
  void Shutdown();
  void LoadStartupState();
  void LoadUiPreferencesAndResources();
  void SeedDefaultsIfEmpty();
  bool PersistRuntimeState();
  void RefreshTodayQueue();

  std::string sqlite_path_;
  data::SqliteRepository repository_;
  domain::InteractionFlowService interaction_flow_service_;
  domain::RewardEngine reward_engine_;
  domain::TodayQueueService today_queue_service_;
  AppState app_state_;
  ui::DockspaceShell dockspace_shell_;

  SDL_Window* window_{nullptr};
  SDL_GLContext gl_context_{nullptr};
  bool initialized_{false};
  std::string init_error_{};
};

}  // namespace habitrpg::app
