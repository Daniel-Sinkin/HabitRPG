#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "habitrpg/domain/entities.hpp"
#include "habitrpg/domain/today_queue.hpp"
#include "habitrpg/ui/contracts.hpp"
#include "habitrpg/ui/runtime_resources.hpp"

namespace habitrpg::app {

struct RuntimeCollections {
  std::vector<domain::ActionUnit> life_actions{};
  std::vector<domain::LearningGoal> learning_goals{};
  std::vector<domain::LearningSession> learning_sessions{};
  std::vector<domain::MilestoneCheckpoint> milestone_checkpoints{};
  std::vector<domain::RewardEvent> reward_events{};
};

struct AppState {
  domain::UserState user_state{};
  ui::contracts::UiViewState ui_state{};
  RuntimeCollections runtime{};
  std::vector<domain::TodayQueueItem> today_queue{};

  std::array<char, 128> new_life_action_title{};
  std::array<char, 128> new_learning_goal_title{};
  std::array<char, 256> new_learning_goal_milestone{};
  std::array<char, 128> new_learning_session_title{};
  std::array<char, 128> new_learning_artifact_ref{};
  int input_priority_score{100};
  int input_learning_duration_minutes{25};
  int selected_learning_goal_index{0};
  int selected_checkpoint_index{0};

  std::string active_unit_id{};
  domain::TrackType active_track_type{domain::TrackType::Life};
  std::string pending_start_unit_id{};
  domain::TrackType pending_start_track_type{domain::TrackType::Life};
  bool show_active_conflict_modal{false};

  int last_reward_xp{0};
  ui::contracts::RewardEffectTier last_reward_tier{ui::contracts::RewardEffectTier::Off};
  std::string last_reward_kind{"none"};
  std::string last_feedback_asset{};

  bool focus_session_running{false};
  int focus_session_minutes{25};
  std::string focus_status{"Ready"};
  std::string last_save_error{};
  bool save_error_pending_retry{false};

  ui::ThemeRuntime theme_runtime{};
  ui::AssetRuntimeMap asset_runtime_map{};
  ui::CopyPack copy_pack{};

  uint64_t mutation_revision{0};
  uint64_t persisted_revision{0};
};

bool StartupSmokeCheck(const std::string& sqlite_path, std::string* error_out = nullptr);

inline void MarkMutated(AppState* app_state) {
  if (app_state == nullptr) {
    return;
  }
  app_state->save_error_pending_retry = false;
  app_state->mutation_revision += 1;
}

}  // namespace habitrpg::app
