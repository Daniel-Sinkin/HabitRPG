#include "habitrpg/app/application.hpp"

#include <algorithm>
#include <stdexcept>
#include <string>

#include <SDL3/SDL_opengl.h>

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl3.h"

namespace habitrpg::app {

Application::Application(std::string sqlite_path)
    : sqlite_path_(std::move(sqlite_path)),
      repository_(sqlite_path_),
      interaction_flow_service_(),
      reward_engine_(),
      today_queue_service_() {}

Application::~Application() {
  Shutdown();
}

bool Application::Initialize() {
  if (initialized_) {
    return true;
  }

  init_error_.clear();
  const auto fail = [this](const std::string& message) {
    init_error_ = message;
    return false;
  };

  // SDL3 returns bool (true = success). This must not use SDL2-style checks.
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    return fail("SDL_Init(SDL_INIT_VIDEO) failed: " + std::string(SDL_GetError()));
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  window_ = SDL_CreateWindow(
      "HabitRPG",
      1400,
      900,
      SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);

  if (window_ == nullptr) {
    return fail("SDL_CreateWindow failed: " + std::string(SDL_GetError()));
  }

  gl_context_ = SDL_GL_CreateContext(window_);
  if (gl_context_ == nullptr) {
    return fail("SDL_GL_CreateContext failed: " + std::string(SDL_GetError()));
  }

  if (!SDL_GL_MakeCurrent(window_, gl_context_)) {
    return fail("SDL_GL_MakeCurrent failed: " + std::string(SDL_GetError()));
  }

  if (!SDL_GL_SetSwapInterval(1)) {
    return fail("SDL_GL_SetSwapInterval(1) failed: " + std::string(SDL_GetError()));
  }

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  ImGui::StyleColorsDark();

  if (!ImGui_ImplSDL3_InitForOpenGL(window_, gl_context_)) {
    return fail("ImGui_ImplSDL3_InitForOpenGL failed");
  }

  if (!ImGui_ImplOpenGL3_Init("#version 150")) {
    return fail("ImGui_ImplOpenGL3_Init(\"#version 150\") failed");
  }

  LoadStartupState();
  initialized_ = true;
  return true;
}

void Application::Shutdown() {
  if (!initialized_) {
    return;
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();

  if (gl_context_ != nullptr) {
    SDL_GL_DestroyContext(gl_context_);
    gl_context_ = nullptr;
  }

  if (window_ != nullptr) {
    SDL_DestroyWindow(window_);
    window_ = nullptr;
  }

  SDL_Quit();
  initialized_ = false;
}

void Application::LoadStartupState() {
  repository_.Migrate();
  app_state_.user_state = repository_.LoadUserState();
  LoadUiPreferencesAndResources();

  app_state_.runtime.life_actions = repository_.ListActionUnitsByTrack(domain::TrackType::Life);
  app_state_.runtime.learning_goals = repository_.ListLearningGoals();
  app_state_.runtime.learning_sessions = repository_.ListLearningSessions();
  app_state_.runtime.milestone_checkpoints = repository_.ListMilestoneCheckpoints();

  auto life_rewards = repository_.ListRewardEventsByTrack(domain::TrackType::Life);
  auto learning_rewards = repository_.ListRewardEventsByTrack(domain::TrackType::Learning);
  app_state_.runtime.reward_events.clear();
  app_state_.runtime.reward_events.reserve(life_rewards.size() + learning_rewards.size());
  app_state_.runtime.reward_events.insert(
      app_state_.runtime.reward_events.end(), life_rewards.begin(), life_rewards.end());
  app_state_.runtime.reward_events.insert(
      app_state_.runtime.reward_events.end(), learning_rewards.begin(), learning_rewards.end());

  SeedDefaultsIfEmpty();
  RefreshTodayQueue();

  for (const auto& action_unit : app_state_.runtime.life_actions) {
    if (action_unit.lifecycle_state == domain::LifecycleState::Active) {
      app_state_.active_unit_id = action_unit.id;
      app_state_.active_track_type = domain::TrackType::Life;
      break;
    }
  }

  if (app_state_.active_unit_id.empty()) {
    for (const auto& learning_session : app_state_.runtime.learning_sessions) {
      if (learning_session.lifecycle_state == domain::LifecycleState::Active) {
        app_state_.active_unit_id = learning_session.id;
        app_state_.active_track_type = domain::TrackType::Learning;
        break;
      }
    }
  }

  app_state_.focus_status = "Ready for next action";
  app_state_.persisted_revision = app_state_.mutation_revision;
}

void Application::LoadUiPreferencesAndResources() {
  const auto preferences = repository_.LoadUiPreferences();

  app_state_.ui_state.queue_mode = preferences.queue_mode;
  app_state_.ui_state.last_non_custom_preset = preferences.last_non_custom_preset;
  if (app_state_.ui_state.last_non_custom_preset == ui::contracts::PresetMode::Custom) {
    app_state_.ui_state.last_non_custom_preset = ui::contracts::PresetMode::Calm;
  }

  if (preferences.preset_mode == ui::contracts::PresetMode::Custom) {
    app_state_.ui_state.preset_mode = ui::contracts::PresetMode::Custom;
    ui::contracts::ApplySensoryOverride(
        &app_state_.ui_state,
        preferences.motion_level,
        preferences.sound_level,
        preferences.density_level);
  } else {
    ui::contracts::ApplyPresetBundle(&app_state_.ui_state, preferences.preset_mode);
  }

  const std::string preferred_theme = (app_state_.ui_state.preset_mode == ui::contracts::PresetMode::Calm)
                                          ? "assets/ui/themes/theme_paper_console_v2.json"
                                          : "assets/ui/themes/theme_signal_garden_v2.json";
  const std::string fallback_theme = (preferred_theme.find("paper_console") != std::string::npos)
                                         ? "assets/ui/themes/theme_signal_garden_v2.json"
                                         : "assets/ui/themes/theme_paper_console_v2.json";

  app_state_.theme_runtime = ui::LoadThemeFromJson(preferred_theme);
  if (!app_state_.theme_runtime.loaded) {
    app_state_.theme_runtime = ui::LoadThemeFromJson(fallback_theme);
  }

  app_state_.asset_runtime_map = ui::LoadAssetRuntimeMapFromJson("assets/ui/runtime/asset_map_v2.json");
  app_state_.copy_pack = ui::LoadCopyPackFromMarkdown("docs/DESIGNER_COPY_PACK_V1.md");
}

void Application::SeedDefaultsIfEmpty() {
  if (app_state_.runtime.learning_goals.empty()) {
    auto learning_goal = interaction_flow_service_.CreateLearningGoal(
        "C++ Momentum",
        "Complete one short C++ coding exercise with notes");
    app_state_.runtime.learning_goals.push_back(std::move(learning_goal));
    MarkMutated(&app_state_);
  }

  if (app_state_.runtime.life_actions.empty()) {
    auto life_action = interaction_flow_service_.CreateLifeAction(
        "habit.seed",
        "Pick one high-impact life task",
        120);
    app_state_.runtime.life_actions.push_back(std::move(life_action));
    MarkMutated(&app_state_);
  }

  if (app_state_.runtime.learning_sessions.empty() && !app_state_.runtime.learning_goals.empty()) {
    auto learning_session = interaction_flow_service_.CreateLearningSession(
        app_state_.runtime.learning_goals.front().id,
        "C++ focused practice",
        25,
        110,
        "note",
        "seed-session");
    app_state_.runtime.learning_sessions.push_back(std::move(learning_session));
    MarkMutated(&app_state_);
  }
}

bool Application::PersistRuntimeState() {
  try {
    repository_.SaveUserState(app_state_.user_state);

    data::UiPreferences preferences{};
    preferences.preset_mode = app_state_.ui_state.preset_mode;
    preferences.last_non_custom_preset = app_state_.ui_state.last_non_custom_preset;
    preferences.motion_level = app_state_.ui_state.motion_level;
    preferences.sound_level = app_state_.ui_state.sound_level;
    preferences.density_level = app_state_.ui_state.density_level;
    preferences.queue_mode = app_state_.ui_state.queue_mode;
    preferences.prompt_concurrency_limit = 2;
    preferences.nudge_cooldown_seconds = 30;
    preferences.updated_at = domain::CurrentTimestampUtc();
    repository_.SaveUiPreferences(preferences);

    for (const auto& action : app_state_.runtime.life_actions) {
      repository_.UpsertActionUnit(action);
    }
    for (const auto& learning_goal : app_state_.runtime.learning_goals) {
      repository_.UpsertLearningGoal(learning_goal);
    }
    for (const auto& learning_session : app_state_.runtime.learning_sessions) {
      repository_.UpsertLearningSession(learning_session);
    }
    for (const auto& checkpoint : app_state_.runtime.milestone_checkpoints) {
      repository_.UpsertMilestoneCheckpoint(checkpoint);
    }
    for (const auto& reward_event : app_state_.runtime.reward_events) {
      repository_.AppendRewardEvent(reward_event);
    }

    app_state_.save_error_pending_retry = false;
    app_state_.last_save_error.clear();
    app_state_.persisted_revision = app_state_.mutation_revision;
    return true;
  } catch (const std::exception& ex) {
    app_state_.save_error_pending_retry = true;
    app_state_.last_save_error = ex.what();
    app_state_.focus_status = app_state_.copy_pack.error_save_primary;
    return false;
  }
}

void Application::RefreshTodayQueue() {
  app_state_.today_queue = today_queue_service_.BuildQueue(
      app_state_.ui_state.queue_mode,
      app_state_.runtime.life_actions,
      app_state_.runtime.learning_sessions);
}

int Application::Run() {
  if (!Initialize()) {
    if (init_error_.empty()) {
      throw std::runtime_error("Failed to initialize SDL3/OpenGL3 ImGui runtime");
    }
    throw std::runtime_error(init_error_);
  }

  bool running = true;
  while (running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL3_ProcessEvent(&event);

      if (event.type == SDL_EVENT_QUIT) {
        running = false;
      }

      if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window_)) {
        running = false;
      }
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    RefreshTodayQueue();
    dockspace_shell_.Render(&app_state_);

    ImGui::Render();

    int width = 0;
    int height = 0;
    SDL_GetWindowSizeInPixels(window_, &width, &height);

    glViewport(0, 0, width, height);
    glClearColor(0.08f, 0.09f, 0.10f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window_);

    if (app_state_.mutation_revision != app_state_.persisted_revision && !app_state_.save_error_pending_retry) {
      PersistRuntimeState();
    }
  }

  PersistRuntimeState();
  return 0;
}

}  // namespace habitrpg::app
