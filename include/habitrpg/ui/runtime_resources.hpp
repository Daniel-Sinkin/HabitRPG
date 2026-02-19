#pragma once

#include <array>
#include <string>
#include <unordered_map>
#include <vector>

namespace habitrpg::ui {

struct ThemeRuntime {
  bool loaded{false};
  std::string theme_id;
  std::unordered_map<std::string, float> style_scalars;
  std::unordered_map<std::string, std::array<float, 2>> style_vec2;
  std::unordered_map<std::string, std::array<float, 4>> colors;
};

struct AssetRuntimeMap {
  bool loaded{false};
  std::unordered_map<std::string, std::unordered_map<std::string, std::string>> component_state_assets;
  std::unordered_map<std::string, std::unordered_map<std::string, std::string>> feedback_signatures;
  std::unordered_map<std::string, std::string> signature_fallback;
  std::unordered_map<std::string, std::vector<std::string>> tier_resolution_order;
};

struct CopyPack {
  bool loaded{false};

  std::string today_empty_primary{"Nothing is queued yet."};
  std::string today_empty_secondary{"Pick one small action to start momentum."};
  std::string today_empty_action_add_habit{"Add Habit"};
  std::string today_empty_action_add_learning_goal{"Add Learning Goal"};

  std::string conflict_active_primary{"A session is already active."};
  std::string conflict_active_secondary{"Do you want to switch or continue?"};
  std::string conflict_action_continue{"Continue Current"};
  std::string conflict_action_pause_switch{"Pause and Switch"};
  std::string conflict_action_cancel{"Cancel"};

  std::string completion_life_toast{"Action complete. XP added."};
  std::string completion_learning_toast{"Session complete. Learning XP added."};
  std::string completion_milestone_confirmed_toast{"Milestone confirmed."};
  std::string completion_milestone_candidate_toast{"Checkpoint saved as candidate."};

  std::string error_save_primary{"We could not save that change."};
  std::string error_save_secondary{"Your input is still here."};
  std::string error_save_action_retry{"Retry Save"};
  std::string error_save_action_cancel{"Cancel"};

  std::string error_completion_save_primary{"Completion was not saved yet."};
  std::string error_completion_save_secondary{"Retry now or keep your draft."};
  std::string error_completion_save_action_retry{"Retry"};
  std::string error_completion_save_action_keep_draft{"Keep Draft"};
  std::string error_completion_save_action_cancel{"Cancel"};
};

ThemeRuntime LoadThemeFromJson(const std::string& path);
AssetRuntimeMap LoadAssetRuntimeMapFromJson(const std::string& path);
CopyPack LoadCopyPackFromMarkdown(const std::string& path);

std::string ResolveComponentAsset(
    const AssetRuntimeMap& asset_map,
    const std::string& component,
    const std::string& state,
    const std::string& fallback = "");

std::string ResolveFeedbackAsset(
    const AssetRuntimeMap& asset_map,
    const std::string& signature,
    const std::string& requested_tier);

}  // namespace habitrpg::ui
