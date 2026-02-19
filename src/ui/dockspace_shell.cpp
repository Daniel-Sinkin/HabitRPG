#include "habitrpg/ui/dockspace_shell.hpp"

#include <algorithm>
#include <array>
#include <filesystem>
#include <sstream>
#include <string>

#include "imgui.h"
#include "imgui_internal.h"

namespace habitrpg::ui {

namespace {

const std::array<contracts::ScreenKey, 7> kNavScreens{
    contracts::ScreenKey::Today,
    contracts::ScreenKey::Quests,
    contracts::ScreenKey::Habits,
    contracts::ScreenKey::Learning,
    contracts::ScreenKey::Character,
    contracts::ScreenKey::Insights,
    contracts::ScreenKey::Settings,
};

const char* TrackLabel(const domain::TrackType track_type) {
  return track_type == domain::TrackType::Life ? "Life" : "Learning";
}

std::string LearningGoalLabel(const app::AppState& app_state, const std::string& goal_id) {
  const auto it = std::find_if(
      app_state.runtime.learning_goals.begin(),
      app_state.runtime.learning_goals.end(),
      [&goal_id](const domain::LearningGoal& goal) { return goal.id == goal_id; });

  if (it == app_state.runtime.learning_goals.end()) {
    return "Unknown Goal";
  }

  return it->title;
}

std::string ShortAssetLabel(const std::string& asset_path) {
  if (asset_path.empty()) {
    return "";
  }
  if (asset_path == "none") {
    return "";
  }

  return std::string("[") + std::filesystem::path(asset_path).filename().string() + "]";
}

std::string NavComponentKey(const contracts::ScreenKey screen) {
  switch (screen) {
    case contracts::ScreenKey::Today:
      return "Navigation.NavItem.Today";
    case contracts::ScreenKey::Quests:
      return "Navigation.NavItem.Quests";
    case contracts::ScreenKey::Habits:
      return "Navigation.NavItem.Habits";
    case contracts::ScreenKey::Learning:
      return "Navigation.NavItem.Learning";
    case contracts::ScreenKey::Character:
      return "Navigation.NavItem.Character";
    case contracts::ScreenKey::Insights:
      return "Navigation.NavItem.Insights";
    case contracts::ScreenKey::Settings:
      return "Navigation.NavItem.Settings";
  }

  return "";
}

bool TryMapImGuiColor(const std::string& color_name, ImGuiCol* out_color) {
  if (out_color == nullptr) {
    return false;
  }

  if (color_name == "ImGuiCol_Text") {
    *out_color = ImGuiCol_Text;
    return true;
  }
  if (color_name == "ImGuiCol_TextDisabled") {
    *out_color = ImGuiCol_TextDisabled;
    return true;
  }
  if (color_name == "ImGuiCol_WindowBg") {
    *out_color = ImGuiCol_WindowBg;
    return true;
  }
  if (color_name == "ImGuiCol_ChildBg") {
    *out_color = ImGuiCol_ChildBg;
    return true;
  }
  if (color_name == "ImGuiCol_PopupBg") {
    *out_color = ImGuiCol_PopupBg;
    return true;
  }
  if (color_name == "ImGuiCol_Border") {
    *out_color = ImGuiCol_Border;
    return true;
  }
  if (color_name == "ImGuiCol_BorderShadow") {
    *out_color = ImGuiCol_BorderShadow;
    return true;
  }
  if (color_name == "ImGuiCol_FrameBg") {
    *out_color = ImGuiCol_FrameBg;
    return true;
  }
  if (color_name == "ImGuiCol_FrameBgHovered") {
    *out_color = ImGuiCol_FrameBgHovered;
    return true;
  }
  if (color_name == "ImGuiCol_FrameBgActive") {
    *out_color = ImGuiCol_FrameBgActive;
    return true;
  }
  if (color_name == "ImGuiCol_TitleBg") {
    *out_color = ImGuiCol_TitleBg;
    return true;
  }
  if (color_name == "ImGuiCol_TitleBgActive") {
    *out_color = ImGuiCol_TitleBgActive;
    return true;
  }
  if (color_name == "ImGuiCol_TitleBgCollapsed") {
    *out_color = ImGuiCol_TitleBgCollapsed;
    return true;
  }
  if (color_name == "ImGuiCol_MenuBarBg") {
    *out_color = ImGuiCol_MenuBarBg;
    return true;
  }
  if (color_name == "ImGuiCol_ScrollbarBg") {
    *out_color = ImGuiCol_ScrollbarBg;
    return true;
  }
  if (color_name == "ImGuiCol_ScrollbarGrab") {
    *out_color = ImGuiCol_ScrollbarGrab;
    return true;
  }
  if (color_name == "ImGuiCol_ScrollbarGrabHovered") {
    *out_color = ImGuiCol_ScrollbarGrabHovered;
    return true;
  }
  if (color_name == "ImGuiCol_ScrollbarGrabActive") {
    *out_color = ImGuiCol_ScrollbarGrabActive;
    return true;
  }
  if (color_name == "ImGuiCol_CheckMark") {
    *out_color = ImGuiCol_CheckMark;
    return true;
  }
  if (color_name == "ImGuiCol_SliderGrab") {
    *out_color = ImGuiCol_SliderGrab;
    return true;
  }
  if (color_name == "ImGuiCol_SliderGrabActive") {
    *out_color = ImGuiCol_SliderGrabActive;
    return true;
  }
  if (color_name == "ImGuiCol_Button") {
    *out_color = ImGuiCol_Button;
    return true;
  }
  if (color_name == "ImGuiCol_ButtonHovered") {
    *out_color = ImGuiCol_ButtonHovered;
    return true;
  }
  if (color_name == "ImGuiCol_ButtonActive") {
    *out_color = ImGuiCol_ButtonActive;
    return true;
  }
  if (color_name == "ImGuiCol_Header") {
    *out_color = ImGuiCol_Header;
    return true;
  }
  if (color_name == "ImGuiCol_HeaderHovered") {
    *out_color = ImGuiCol_HeaderHovered;
    return true;
  }
  if (color_name == "ImGuiCol_HeaderActive") {
    *out_color = ImGuiCol_HeaderActive;
    return true;
  }
  if (color_name == "ImGuiCol_Separator") {
    *out_color = ImGuiCol_Separator;
    return true;
  }
  if (color_name == "ImGuiCol_SeparatorHovered") {
    *out_color = ImGuiCol_SeparatorHovered;
    return true;
  }
  if (color_name == "ImGuiCol_SeparatorActive") {
    *out_color = ImGuiCol_SeparatorActive;
    return true;
  }
  if (color_name == "ImGuiCol_Tab") {
    *out_color = ImGuiCol_Tab;
    return true;
  }
  if (color_name == "ImGuiCol_TabHovered") {
    *out_color = ImGuiCol_TabHovered;
    return true;
  }
  if (color_name == "ImGuiCol_TabSelected") {
    *out_color = ImGuiCol_TabSelected;
    return true;
  }
  if (color_name == "ImGuiCol_TabSelectedOverline") {
    *out_color = ImGuiCol_TabSelectedOverline;
    return true;
  }
  if (color_name == "ImGuiCol_TabDimmed") {
    *out_color = ImGuiCol_TabDimmed;
    return true;
  }
  if (color_name == "ImGuiCol_TabDimmedSelected") {
    *out_color = ImGuiCol_TabDimmedSelected;
    return true;
  }
  if (color_name == "ImGuiCol_TabDimmedSelectedOverline") {
    *out_color = ImGuiCol_TabDimmedSelectedOverline;
    return true;
  }
  if (color_name == "ImGuiCol_DockingPreview") {
    *out_color = ImGuiCol_DockingPreview;
    return true;
  }
  if (color_name == "ImGuiCol_DockingEmptyBg") {
    *out_color = ImGuiCol_DockingEmptyBg;
    return true;
  }
  if (color_name == "ImGuiCol_PlotHistogram") {
    *out_color = ImGuiCol_PlotHistogram;
    return true;
  }
  if (color_name == "ImGuiCol_PlotHistogramHovered") {
    *out_color = ImGuiCol_PlotHistogramHovered;
    return true;
  }
  if (color_name == "ImGuiCol_TextSelectedBg") {
    *out_color = ImGuiCol_TextSelectedBg;
    return true;
  }
  if (color_name == "ImGuiCol_NavCursor") {
    *out_color = ImGuiCol_NavCursor;
    return true;
  }
  if (color_name == "ImGuiCol_NavWindowingHighlight") {
    *out_color = ImGuiCol_NavWindowingHighlight;
    return true;
  }
  if (color_name == "ImGuiCol_NavWindowingDimBg") {
    *out_color = ImGuiCol_NavWindowingDimBg;
    return true;
  }
  if (color_name == "ImGuiCol_ModalWindowDimBg") {
    *out_color = ImGuiCol_ModalWindowDimBg;
    return true;
  }

  return false;
}

}  // namespace

void DockspaceShell::Render(app::AppState* app_state) {
  if (app_state == nullptr) {
    return;
  }

  ApplyThemeTokens(*app_state);

  const ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->WorkPos);
  ImGui::SetNextWindowSize(viewport->WorkSize);
  ImGui::SetNextWindowViewport(viewport->ID);

  const ImGuiWindowFlags host_window_flags =
      ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
      ImGuiWindowFlags_NoNavFocus;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("HabitRPG.DockHost", nullptr, host_window_flags);
  ImGui::PopStyleVar(3);

  const ImGuiID dockspace_id = ImGui::GetID("HabitRPG.Dockspace");
  ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
  EnsureDockLayout();

  ImGui::End();

  RenderLeftNavigation(app_state);
  RenderCenterActionPanel(app_state);
  RenderRightStatePanel(app_state);
  RenderBottomControlStrip(app_state);
}

void DockspaceShell::ApplyThemeTokens(const app::AppState& app_state) const {
  const auto& theme = app_state.theme_runtime;
  if (!theme.loaded) {
    return;
  }

  ImGuiStyle& style = ImGui::GetStyle();

  if (const auto it = theme.style_scalars.find("WindowRounding"); it != theme.style_scalars.end()) {
    style.WindowRounding = it->second;
  }
  if (const auto it = theme.style_scalars.find("ChildRounding"); it != theme.style_scalars.end()) {
    style.ChildRounding = it->second;
  }
  if (const auto it = theme.style_scalars.find("PopupRounding"); it != theme.style_scalars.end()) {
    style.PopupRounding = it->second;
  }
  if (const auto it = theme.style_scalars.find("FrameRounding"); it != theme.style_scalars.end()) {
    style.FrameRounding = it->second;
  }
  if (const auto it = theme.style_scalars.find("GrabRounding"); it != theme.style_scalars.end()) {
    style.GrabRounding = it->second;
  }
  if (const auto it = theme.style_scalars.find("TabRounding"); it != theme.style_scalars.end()) {
    style.TabRounding = it->second;
  }
  if (const auto it = theme.style_scalars.find("IndentSpacing"); it != theme.style_scalars.end()) {
    style.IndentSpacing = it->second;
  }
  if (const auto it = theme.style_scalars.find("ScrollbarSize"); it != theme.style_scalars.end()) {
    style.ScrollbarSize = it->second;
  }

  if (const auto it = theme.style_vec2.find("WindowPadding"); it != theme.style_vec2.end()) {
    style.WindowPadding = ImVec2(it->second[0], it->second[1]);
  }
  if (const auto it = theme.style_vec2.find("FramePadding"); it != theme.style_vec2.end()) {
    style.FramePadding = ImVec2(it->second[0], it->second[1]);
  }
  if (const auto it = theme.style_vec2.find("ItemSpacing"); it != theme.style_vec2.end()) {
    style.ItemSpacing = ImVec2(it->second[0], it->second[1]);
  }
  if (const auto it = theme.style_vec2.find("ItemInnerSpacing"); it != theme.style_vec2.end()) {
    style.ItemInnerSpacing = ImVec2(it->second[0], it->second[1]);
  }

  for (const auto& [color_name, rgba] : theme.colors) {
    ImGuiCol color_key{};
    if (!TryMapImGuiColor(color_name, &color_key)) {
      continue;
    }

    style.Colors[color_key] = ImVec4(rgba[0], rgba[1], rgba[2], rgba[3]);
  }
}

void DockspaceShell::EnsureDockLayout() {
  if (dock_layout_initialized_) {
    return;
  }

  const ImGuiID dockspace_id = ImGui::GetID("HabitRPG.Dockspace");
  if (ImGui::DockBuilderGetNode(dockspace_id) != nullptr) {
    dock_layout_initialized_ = true;
    return;
  }

  ImGui::DockBuilderRemoveNode(dockspace_id);
  ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
  ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

  ImGuiID dock_id_main = dockspace_id;
  ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Left, 0.19f, nullptr, &dock_id_main);
  ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Right, 0.24f, nullptr, &dock_id_main);
  ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Down, 0.22f, nullptr, &dock_id_main);

  ImGui::DockBuilderDockWindow("Navigation", dock_id_left);
  ImGui::DockBuilderDockWindow("Action Panel", dock_id_main);
  ImGui::DockBuilderDockWindow("State Panel", dock_id_right);
  ImGui::DockBuilderDockWindow("Session Controls", dock_id_bottom);

  ImGui::DockBuilderFinish(dockspace_id);
  dock_layout_initialized_ = true;
}

void DockspaceShell::RenderLeftNavigation(app::AppState* app_state) {
  ImGui::Begin("Navigation");
  ImGui::TextUnformatted("HabitRPG");
  ImGui::Separator();

  for (const auto screen : kNavScreens) {
    const bool is_active = app_state->ui_state.active_screen == screen;

    const auto asset = ResolveComponentAsset(
        app_state->asset_runtime_map,
        NavComponentKey(screen),
        is_active ? "active" : "default");

    std::ostringstream nav_button_label;
    const std::string icon = ShortAssetLabel(asset);
    if (!icon.empty()) {
      nav_button_label << icon << " ";
    }
    nav_button_label << contracts::ScreenLabel(screen) << "##nav."
                     << std::string(contracts::kScreenIds[static_cast<int>(screen)].second);

    if (ImGui::Button(nav_button_label.str().c_str(), ImVec2(-1.0f, 0.0f))) {
      app_state->ui_state.active_screen = screen;
    }
  }

  ImGui::SeparatorText("Queue Mode");

  const auto previous_mode = app_state->ui_state.queue_mode;
  int filter_index = static_cast<int>(app_state->ui_state.queue_mode);
  ImGui::RadioButton("Mixed##queue_mode", &filter_index, static_cast<int>(contracts::TrackFilter::Mixed));
  ImGui::RadioButton("Life##queue_mode", &filter_index, static_cast<int>(contracts::TrackFilter::LifeOnly));
  ImGui::RadioButton("Learning##queue_mode", &filter_index, static_cast<int>(contracts::TrackFilter::LearningOnly));
  app_state->ui_state.queue_mode = static_cast<contracts::TrackFilter>(filter_index);
  if (previous_mode != app_state->ui_state.queue_mode) {
    app::MarkMutated(app_state);
  }

  ImGui::End();
}

void DockspaceShell::RenderTodayControls(app::AppState* app_state) {
  ImGui::SeparatorText("Create Life Action");
  ImGui::InputText("Title##new_life_action", app_state->new_life_action_title.data(), app_state->new_life_action_title.size());
  ImGui::SliderInt("Priority##new_life_action", &app_state->input_priority_score, 0, 200);

  if (ImGui::Button("Create Life Action")) {
    const std::string title = app_state->new_life_action_title.data();
    if (!title.empty()) {
      auto action = interaction_flow_service_.CreateLifeAction("habit.manual", title, app_state->input_priority_score);
      app_state->runtime.life_actions.push_back(std::move(action));
      app_state->new_life_action_title[0] = '\0';
      app_state->focus_status = "Life action created";
      app::MarkMutated(app_state);
    }
  }

  ImGui::SeparatorText("Create C++ Learning Goal");
  ImGui::InputText(
      "Goal Title##new_learning_goal",
      app_state->new_learning_goal_title.data(),
      app_state->new_learning_goal_title.size());
  ImGui::InputText(
      "Milestone##new_learning_goal",
      app_state->new_learning_goal_milestone.data(),
      app_state->new_learning_goal_milestone.size());

  if (ImGui::Button("Create Learning Goal")) {
    const std::string title = app_state->new_learning_goal_title.data();
    const std::string milestone = app_state->new_learning_goal_milestone.data();
    if (!title.empty() && !milestone.empty()) {
      const bool duplicate = std::any_of(
          app_state->runtime.learning_goals.begin(),
          app_state->runtime.learning_goals.end(),
          [&title](const domain::LearningGoal& goal) { return goal.title == title; });

      auto learning_goal = interaction_flow_service_.CreateLearningGoal(title, milestone);
      if (!duplicate) {
        app_state->runtime.learning_goals.push_back(std::move(learning_goal));
        app_state->selected_learning_goal_index = static_cast<int>(app_state->runtime.learning_goals.size()) - 1;
        app_state->focus_status = "Learning goal created";
      } else {
        app_state->focus_status = "A similar goal already exists.";
      }

      app_state->new_learning_goal_title[0] = '\0';
      app_state->new_learning_goal_milestone[0] = '\0';
      app::MarkMutated(app_state);
    }
  }

  ImGui::SeparatorText("Create Learning Session");
  if (app_state->runtime.learning_goals.empty()) {
    ImGui::TextUnformatted("No learning goals yet.");
    ImGui::TextUnformatted("Add one C++ milestone to begin.");
  } else {
    if (app_state->selected_learning_goal_index < 0 ||
        app_state->selected_learning_goal_index >= static_cast<int>(app_state->runtime.learning_goals.size())) {
      app_state->selected_learning_goal_index = 0;
    }

    if (ImGui::BeginCombo(
            "Goal##learning_goal_select",
            app_state->runtime.learning_goals[app_state->selected_learning_goal_index].title.c_str())) {
      for (int i = 0; i < static_cast<int>(app_state->runtime.learning_goals.size()); ++i) {
        const bool selected = app_state->selected_learning_goal_index == i;
        if (ImGui::Selectable(app_state->runtime.learning_goals[i].title.c_str(), selected)) {
          app_state->selected_learning_goal_index = i;
        }
        if (selected) {
          ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }

    ImGui::InputText(
        "Session Title##new_learning_session",
        app_state->new_learning_session_title.data(),
        app_state->new_learning_session_title.size());
    ImGui::InputText(
        "Artifact Ref##new_learning_session",
        app_state->new_learning_artifact_ref.data(),
        app_state->new_learning_artifact_ref.size());
    ImGui::SliderInt("Duration (min)##new_learning_session", &app_state->input_learning_duration_minutes, 5, 180);
    ImGui::SliderInt("Priority##new_learning_session", &app_state->input_priority_score, 0, 200);

    if (ImGui::Button("Create Learning Session")) {
      const std::string title = app_state->new_learning_session_title.data();
      if (!title.empty()) {
        const auto& learning_goal = app_state->runtime.learning_goals[app_state->selected_learning_goal_index];
        auto session = interaction_flow_service_.CreateLearningSession(
            learning_goal.id,
            title,
            app_state->input_learning_duration_minutes,
            app_state->input_priority_score,
            "code_snippet",
            app_state->new_learning_artifact_ref.data());

        app_state->runtime.learning_sessions.push_back(std::move(session));
        app_state->new_learning_session_title[0] = '\0';
        app_state->new_learning_artifact_ref[0] = '\0';
        app_state->focus_status = "Learning session created";
        app::MarkMutated(app_state);
      }
    }
  }

  ImGui::SeparatorText("Today Queue");
  if (app_state->today_queue.empty()) {
    ImGui::TextWrapped("%s", app_state->copy_pack.today_empty_primary.c_str());
    ImGui::TextWrapped("%s", app_state->copy_pack.today_empty_secondary.c_str());

    if (ImGui::Button(app_state->copy_pack.today_empty_action_add_habit.c_str())) {
      app_state->focus_status = "Add a habit with one small action.";
    }
    ImGui::SameLine();
    if (ImGui::Button(app_state->copy_pack.today_empty_action_add_learning_goal.c_str())) {
      app_state->focus_status = "Add one C++ milestone to begin.";
    }
    return;
  }

  for (const auto& item : app_state->today_queue) {
    RenderQueueItemRow(app_state, item);
  }
}

const char* DockspaceShell::LifecycleIcon(const domain::LifecycleState state) {
  switch (state) {
    case domain::LifecycleState::Ready:
      return "[R]";
    case domain::LifecycleState::Active:
      return "[A]";
    case domain::LifecycleState::Partial:
      return "[~]";
    case domain::LifecycleState::Missed:
      return "[!]";
    case domain::LifecycleState::Paused:
      return "[P]";
    case domain::LifecycleState::Completed:
      return "[+]";
    case domain::LifecycleState::CheckpointCandidate:
      return "[#]";
  }

  return "[?]";
}

void DockspaceShell::RenderQueueItemRow(app::AppState* app_state, const domain::TodayQueueItem& item) {
  std::ostringstream row_label;
  row_label << LifecycleIcon(item.lifecycle_state) << " " << TrackLabel(item.track_type) << ": " << item.title
            << " (p" << item.priority_score << ")";

  if (item.track_type == domain::TrackType::Learning) {
    row_label << " - Goal: " << LearningGoalLabel(*app_state, item.parent_id);
  }

  ImGui::TextUnformatted(row_label.str().c_str());

  const auto start_selected_unit = [this, app_state](const std::string& unit_id, const domain::TrackType track_type) {
    bool changed = false;
    if (track_type == domain::TrackType::Life) {
      changed = interaction_flow_service_.StartActionUnit(
          unit_id,
          &app_state->runtime.life_actions,
          &app_state->runtime.learning_sessions);
    } else {
      changed = interaction_flow_service_.StartLearningSession(
          unit_id,
          &app_state->runtime.life_actions,
          &app_state->runtime.learning_sessions);
    }

    if (changed) {
      app_state->active_unit_id = unit_id;
      app_state->active_track_type = track_type;
      app_state->focus_status = "Unit started (single-active mode enforced)";
      app::MarkMutated(app_state);
    }
  };

  std::ostringstream start_id;
  start_id << "Start##" << item.source_kind << "." << item.unit_id;
  if (ImGui::Button(start_id.str().c_str())) {
    if (!app_state->active_unit_id.empty() && app_state->active_unit_id != item.unit_id) {
      app_state->pending_start_unit_id = item.unit_id;
      app_state->pending_start_track_type = item.track_type;
      app_state->show_active_conflict_modal = true;
      ImGui::OpenPopup("Active Session Conflict");
    } else {
      start_selected_unit(item.unit_id, item.track_type);
    }
  }

  ImGui::SameLine();
  std::ostringstream partial_id;
  partial_id << "Partial##" << item.source_kind << "." << item.unit_id;
  if (ImGui::Button(partial_id.str().c_str())) {
    bool changed = false;
    if (item.track_type == domain::TrackType::Life) {
      auto it = std::find_if(
          app_state->runtime.life_actions.begin(),
          app_state->runtime.life_actions.end(),
          [&item](const domain::ActionUnit& action) { return action.id == item.unit_id; });
      if (it != app_state->runtime.life_actions.end()) {
        it->lifecycle_state = domain::LifecycleState::Partial;
        it->status = domain::ActionStatus::Todo;
        changed = true;
      }
    } else {
      auto it = std::find_if(
          app_state->runtime.learning_sessions.begin(),
          app_state->runtime.learning_sessions.end(),
          [&item](const domain::LearningSession& session) { return session.id == item.unit_id; });
      if (it != app_state->runtime.learning_sessions.end()) {
        it->lifecycle_state = domain::LifecycleState::Partial;
        changed = true;
      }
    }

    if (changed) {
      app_state->focus_status = "Marked partial";
      app::MarkMutated(app_state);
    }
  }

  ImGui::SameLine();
  std::ostringstream pause_id;
  pause_id << "Pause##" << item.source_kind << "." << item.unit_id;
  if (ImGui::Button(pause_id.str().c_str())) {
    bool changed = false;
    if (item.track_type == domain::TrackType::Life) {
      auto it = std::find_if(
          app_state->runtime.life_actions.begin(),
          app_state->runtime.life_actions.end(),
          [&item](const domain::ActionUnit& action) { return action.id == item.unit_id; });
      if (it != app_state->runtime.life_actions.end()) {
        it->lifecycle_state = domain::LifecycleState::Paused;
        it->status = domain::ActionStatus::Todo;
        changed = true;
      }
    } else {
      auto it = std::find_if(
          app_state->runtime.learning_sessions.begin(),
          app_state->runtime.learning_sessions.end(),
          [&item](const domain::LearningSession& session) { return session.id == item.unit_id; });
      if (it != app_state->runtime.learning_sessions.end()) {
        it->lifecycle_state = domain::LifecycleState::Paused;
        changed = true;
      }
    }

    if (changed) {
      app_state->focus_status = "Paused";
      app::MarkMutated(app_state);
    }
  }

  ImGui::SameLine();
  std::ostringstream missed_id;
  missed_id << "Missed##" << item.source_kind << "." << item.unit_id;
  if (ImGui::Button(missed_id.str().c_str())) {
    bool changed = false;
    if (item.track_type == domain::TrackType::Life) {
      auto it = std::find_if(
          app_state->runtime.life_actions.begin(),
          app_state->runtime.life_actions.end(),
          [&item](const domain::ActionUnit& action) { return action.id == item.unit_id; });
      if (it != app_state->runtime.life_actions.end()) {
        it->lifecycle_state = domain::LifecycleState::Missed;
        it->status = domain::ActionStatus::Todo;
        changed = true;
      }
    } else {
      auto it = std::find_if(
          app_state->runtime.learning_sessions.begin(),
          app_state->runtime.learning_sessions.end(),
          [&item](const domain::LearningSession& session) { return session.id == item.unit_id; });
      if (it != app_state->runtime.learning_sessions.end()) {
        it->lifecycle_state = domain::LifecycleState::Missed;
        changed = true;
      }
    }

    if (changed) {
      app_state->focus_status = "Marked missed";
      app::MarkMutated(app_state);
    }
  }

  if (item.track_type == domain::TrackType::Learning) {
    ImGui::SameLine();
    std::ostringstream checkpoint_id;
    checkpoint_id << "Save Candidate##" << item.source_kind << "." << item.unit_id;
    if (ImGui::Button(checkpoint_id.str().c_str())) {
      const bool changed = interaction_flow_service_.CheckpointLearningSession(
          item.unit_id,
          "Checkpoint candidate logged",
          &app_state->runtime.learning_sessions);

      if (changed) {
        auto session_it = std::find_if(
            app_state->runtime.learning_sessions.begin(),
            app_state->runtime.learning_sessions.end(),
            [&item](const domain::LearningSession& session) { return session.id == item.unit_id; });

        if (session_it != app_state->runtime.learning_sessions.end()) {
          auto checkpoint_it = std::find_if(
              app_state->runtime.milestone_checkpoints.begin(),
              app_state->runtime.milestone_checkpoints.end(),
              [&item](const domain::MilestoneCheckpoint& checkpoint) {
                return checkpoint.learning_session_id == item.unit_id &&
                       checkpoint.state == domain::MilestoneCheckpointState::Candidate;
              });

          if (checkpoint_it == app_state->runtime.milestone_checkpoints.end()) {
            auto checkpoint = interaction_flow_service_.CreateMilestoneCheckpointCandidate(
                *session_it,
                "default",
                "snippet",
                session_it->artifact_ref,
                2,
                "manual_candidate");
            app_state->runtime.milestone_checkpoints.push_back(std::move(checkpoint));
          } else {
            checkpoint_it->updated_at = domain::CurrentTimestampUtc();
            checkpoint_it->candidate_reason = "manual_candidate";
          }
        }

        app_state->focus_status = app_state->copy_pack.completion_milestone_candidate_toast;
        app::MarkMutated(app_state);
      }
    }

    auto candidate_it = std::find_if(
        app_state->runtime.milestone_checkpoints.begin(),
        app_state->runtime.milestone_checkpoints.end(),
        [&item](const domain::MilestoneCheckpoint& checkpoint) {
          return checkpoint.learning_session_id == item.unit_id &&
                 checkpoint.state == domain::MilestoneCheckpointState::Candidate;
        });

    if (candidate_it != app_state->runtime.milestone_checkpoints.end()) {
      ImGui::SameLine();
      std::ostringstream confirm_id;
      confirm_id << "Confirm Candidate##" << item.source_kind << "." << item.unit_id;
      if (ImGui::Button(confirm_id.str().c_str())) {
        domain::RewardEngine reward_engine;
        const size_t reward_count_before = app_state->runtime.reward_events.size();

        const bool changed = interaction_flow_service_.PromoteMilestoneCheckpointToConfirmed(
            candidate_it->id,
            &app_state->runtime.milestone_checkpoints,
            &reward_engine,
            &app_state->user_state,
            &app_state->runtime.reward_events);

        if (changed) {
          if (app_state->runtime.reward_events.size() > reward_count_before) {
            const auto& reward_event = app_state->runtime.reward_events.back();
            app_state->last_reward_xp = reward_event.xp_delta;
            app_state->last_reward_kind = reward_event.reward_kind;
            app_state->last_reward_tier =
                contracts::ResolveRewardEffectTier(app_state->ui_state.motion_level, app_state->ui_state.sound_level);
            app_state->last_feedback_asset = ResolveFeedbackAsset(
                app_state->asset_runtime_map,
                "milestone_unlock",
                std::string(contracts::RewardEffectTierKey(app_state->last_reward_tier)));
          }

          app_state->focus_status = app_state->copy_pack.completion_milestone_confirmed_toast;
          app::MarkMutated(app_state);
        }
      }
    }
  }

  ImGui::SameLine();
  std::ostringstream complete_id;
  complete_id << "Complete##" << item.source_kind << "." << item.unit_id;
  if (ImGui::Button(complete_id.str().c_str())) {
    domain::RewardEngine reward_engine;
    bool changed = false;

    if (item.track_type == domain::TrackType::Life) {
      changed = interaction_flow_service_.CompleteActionUnit(
          item.unit_id,
          &app_state->runtime.life_actions,
          &reward_engine,
          &app_state->user_state,
          &app_state->runtime.reward_events);
    } else {
      changed = interaction_flow_service_.CompleteLearningSession(
          item.unit_id,
          &app_state->runtime.learning_sessions,
          &reward_engine,
          &app_state->user_state,
          &app_state->runtime.reward_events);
    }

    if (changed && !app_state->runtime.reward_events.empty()) {
      const auto& reward_event = app_state->runtime.reward_events.back();
      app_state->last_reward_xp = reward_event.xp_delta;
      app_state->last_reward_kind = reward_event.reward_kind;
      app_state->last_reward_tier =
          contracts::ResolveRewardEffectTier(app_state->ui_state.motion_level, app_state->ui_state.sound_level);

      const std::string signature = item.track_type == domain::TrackType::Life ? "life_complete" : "learning_complete";
      app_state->last_feedback_asset = ResolveFeedbackAsset(
          app_state->asset_runtime_map,
          signature,
          std::string(contracts::RewardEffectTierKey(app_state->last_reward_tier)));

      if (item.track_type == domain::TrackType::Life) {
        app_state->focus_status = app_state->copy_pack.completion_life_toast;
      } else {
        app_state->focus_status = app_state->copy_pack.completion_learning_toast;
      }

      if (app_state->active_unit_id == item.unit_id) {
        app_state->active_unit_id.clear();
      }

      app::MarkMutated(app_state);
    }
  }

  ImGui::Separator();
}

void DockspaceShell::RenderCenterActionPanel(app::AppState* app_state) {
  ImGui::Begin("Action Panel");

  ImGui::Text("Active Screen: %s", std::string(contracts::ScreenLabel(app_state->ui_state.active_screen)).c_str());
  ImGui::Separator();

  if (app_state->ui_state.active_screen == contracts::ScreenKey::Today) {
    RenderTodayControls(app_state);
  } else {
    ImGui::SeparatorText("Screen Placeholder");
    switch (app_state->ui_state.active_screen) {
      case contracts::ScreenKey::Today:
        ImGui::TextUnformatted("Today: mixed queue, one-click completion, and recovery actions.");
        break;
      case contracts::ScreenKey::Quests:
        ImGui::TextUnformatted("Quests: daily/weekly chain progress and decomposed objectives.");
        break;
      case contracts::ScreenKey::Habits:
        ImGui::TextUnformatted("Habits: cadence settings, low-friction habit edits, and pause states.");
        break;
      case contracts::ScreenKey::Learning:
        ImGui::TextUnformatted("Learning: C++ goals, sessions, artifact logging, and milestones.");
        break;
      case contracts::ScreenKey::Character:
        ImGui::TextUnformatted("Character: level, XP, consistency band, and reward history.");
        break;
      case contracts::ScreenKey::Insights:
        ImGui::TextUnformatted("Insights: completion trends, friction notes, and adaptation suggestions.");
        break;
      case contracts::ScreenKey::Settings:
        ImGui::TextUnformatted("Settings: motion/sound/density controls and calm/spark presets.");
        break;
    }
  }

  if (app_state->show_active_conflict_modal) {
    ImGui::OpenPopup("Active Session Conflict");
  }

  if (ImGui::BeginPopupModal("Active Session Conflict", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::TextWrapped("%s", app_state->copy_pack.conflict_active_primary.c_str());
    ImGui::TextWrapped("%s", app_state->copy_pack.conflict_active_secondary.c_str());

    if (ImGui::Button(app_state->copy_pack.conflict_action_continue.c_str())) {
      app_state->show_active_conflict_modal = false;
      app_state->pending_start_unit_id.clear();
      ImGui::CloseCurrentPopup();
    }

    ImGui::SameLine();
    if (ImGui::Button(app_state->copy_pack.conflict_action_pause_switch.c_str())) {
      if (!app_state->pending_start_unit_id.empty()) {
        if (app_state->pending_start_track_type == domain::TrackType::Life) {
          interaction_flow_service_.StartActionUnit(
              app_state->pending_start_unit_id,
              &app_state->runtime.life_actions,
              &app_state->runtime.learning_sessions);
        } else {
          interaction_flow_service_.StartLearningSession(
              app_state->pending_start_unit_id,
              &app_state->runtime.life_actions,
              &app_state->runtime.learning_sessions);
        }

        app_state->active_unit_id = app_state->pending_start_unit_id;
        app_state->active_track_type = app_state->pending_start_track_type;
        app_state->focus_status = "Unit started (single-active mode enforced)";
        app::MarkMutated(app_state);
      }

      app_state->show_active_conflict_modal = false;
      app_state->pending_start_unit_id.clear();
      ImGui::CloseCurrentPopup();
    }

    ImGui::SameLine();
    if (ImGui::Button(app_state->copy_pack.conflict_action_cancel.c_str())) {
      app_state->show_active_conflict_modal = false;
      app_state->pending_start_unit_id.clear();
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  }

  ImGui::End();
}

void DockspaceShell::RenderRightStatePanel(app::AppState* app_state) {
  ImGui::Begin("State Panel");

  const float level_progress = static_cast<float>(app_state->user_state.total_xp % 100) / 100.0f;
  ImGui::Text("Level %d", app_state->user_state.level);
  ImGui::ProgressBar(level_progress, ImVec2(-1.0f, 0.0f), "XP to next");

  ImGui::Text("Total XP: %d", app_state->user_state.total_xp);
  ImGui::Text("Life XP: %d", app_state->user_state.life_xp);
  ImGui::Text("Learning XP: %d", app_state->user_state.learning_xp);
  ImGui::Text("Recovery Tokens: %d", app_state->user_state.recovery_tokens);

  ImGui::SeparatorText("Preset Mode");

  const auto previous_preset = app_state->ui_state.preset_mode;
  int preset_index = static_cast<int>(app_state->ui_state.preset_mode);
  ImGui::RadioButton("Calm##preset", &preset_index, static_cast<int>(contracts::PresetMode::Calm));
  ImGui::RadioButton("Spark##preset", &preset_index, static_cast<int>(contracts::PresetMode::Spark));
  ImGui::RadioButton("Custom##preset", &preset_index, static_cast<int>(contracts::PresetMode::Custom));

  const auto selected_preset = static_cast<contracts::PresetMode>(preset_index);
  if (selected_preset != previous_preset) {
    if (selected_preset == contracts::PresetMode::Custom) {
      app_state->ui_state.preset_mode = contracts::PresetMode::Custom;
    } else {
      contracts::ApplyPresetBundle(&app_state->ui_state, selected_preset);
    }
    app::MarkMutated(app_state);
  }

  int motion_level = app_state->ui_state.motion_level;
  int sound_level = app_state->ui_state.sound_level;
  int density_level = app_state->ui_state.density_level;

  const char* motion_options[] = {"Off", "Low", "Full"};
  const char* sound_options[] = {"Off", "Minimal", "Rich"};
  const char* density_options[] = {"Compact", "Comfortable", "Spacious"};

  const bool motion_changed = ImGui::Combo("Motion", &motion_level, motion_options, IM_ARRAYSIZE(motion_options));
  const bool sound_changed = ImGui::Combo("Sound", &sound_level, sound_options, IM_ARRAYSIZE(sound_options));
  const bool density_changed = ImGui::Combo("Density", &density_level, density_options, IM_ARRAYSIZE(density_options));

  if (motion_changed || sound_changed || density_changed) {
    contracts::ApplySensoryOverride(&app_state->ui_state, motion_level, sound_level, density_level);
    app::MarkMutated(app_state);
  }

  if (app_state->ui_state.preset_mode == contracts::PresetMode::Custom) {
    if (ImGui::Button("Restore Last Non-Custom Preset")) {
      contracts::RestoreLastNonCustomPreset(&app_state->ui_state);
      app::MarkMutated(app_state);
    }
  }

  ImGui::SeparatorText("Queue State");
  ImGui::Text("Queue Mode: %s", std::string(contracts::TrackFilterLabel(app_state->ui_state.queue_mode)).c_str());
  ImGui::Text("Today Queue Items: %zu", app_state->today_queue.size());
  if (!app_state->active_unit_id.empty()) {
    ImGui::Text("Active Unit: %s (%s)", app_state->active_unit_id.c_str(), TrackLabel(app_state->active_track_type));
  }

  ImGui::SeparatorText("Reward Effects");
  ImGui::Text("Tier: %s", std::string(contracts::RewardEffectTierLabel(app_state->last_reward_tier)).c_str());
  ImGui::Text("Last Reward: +%d XP", app_state->last_reward_xp);
  ImGui::TextWrapped("Kind: %s", app_state->last_reward_kind.c_str());
  if (!app_state->last_feedback_asset.empty()) {
    ImGui::TextWrapped("Asset: %s", app_state->last_feedback_asset.c_str());
  }

  if (app_state->save_error_pending_retry) {
    ImGui::SeparatorText("Save Error");
    ImGui::TextWrapped("%s", app_state->copy_pack.error_save_primary.c_str());
    ImGui::TextWrapped("%s", app_state->copy_pack.error_save_secondary.c_str());
    if (!app_state->last_save_error.empty()) {
      ImGui::TextWrapped("Details: %s", app_state->last_save_error.c_str());
    }

    if (ImGui::Button(app_state->copy_pack.error_save_action_retry.c_str())) {
      app_state->save_error_pending_retry = false;
      app_state->focus_status = app_state->copy_pack.error_completion_save_action_retry;
      app::MarkMutated(app_state);
    }
    ImGui::SameLine();
    if (ImGui::Button(app_state->copy_pack.error_save_action_cancel.c_str())) {
      app_state->save_error_pending_retry = false;
      app_state->focus_status = app_state->copy_pack.error_completion_save_action_keep_draft;
    }
  }

  ImGui::SeparatorText("Status");
  ImGui::TextWrapped("%s", app_state->focus_status.c_str());

  ImGui::End();
}

void DockspaceShell::RenderBottomControlStrip(app::AppState* app_state) {
  ImGui::Begin("Session Controls");

  ImGui::Text("Focus Timer: %d min", app_state->focus_session_minutes);

  if (!app_state->focus_session_running) {
    if (ImGui::Button("Start Focus Session")) {
      app_state->focus_session_running = true;
      app_state->focus_status = "Focus session started";
    }
  } else {
    if (ImGui::Button("Stop Focus Session")) {
      app_state->focus_session_running = false;
      app_state->focus_status = "Focus session stopped";
    }
  }

  ImGui::SameLine();
  if (ImGui::Button("Break Prompt")) {
    app_state->focus_status = "Break: hydrate and reset";
  }

  ImGui::SameLine();
  if (ImGui::Button("Done For Now")) {
    app_state->focus_session_running = false;
    app_state->focus_status = "Session closed";
  }

  ImGui::End();
}

}  // namespace habitrpg::ui
