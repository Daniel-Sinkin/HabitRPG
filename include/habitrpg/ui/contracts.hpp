#pragma once

#include <array>
#include <stdexcept>
#include <string_view>

namespace habitrpg::ui::contracts {

enum class ScreenKey {
  Today,
  Quests,
  Habits,
  Learning,
  Character,
  Insights,
  Settings,
};

enum class TrackFilter {
  Mixed,
  LifeOnly,
  LearningOnly,
};

enum class PresetMode {
  Calm,
  Spark,
  Custom,
};

enum class RewardEffectTier {
  Off,
  Low,
  Full,
};

struct UiViewState {
  ScreenKey active_screen{ScreenKey::Today};
  TrackFilter queue_mode{TrackFilter::Mixed};

  PresetMode preset_mode{PresetMode::Calm};
  PresetMode last_non_custom_preset{PresetMode::Calm};

  int motion_level{0};   // 0=Off, 1=Low, 2=Full
  int sound_level{0};    // 0=Off, 1=Minimal, 2=Rich
  int density_level{2};  // 0=Compact, 1=Comfortable, 2=Spacious
};

inline constexpr std::array<std::pair<ScreenKey, std::string_view>, 7> kScreenIds{{
    {ScreenKey::Today, "view.today.v1"},
    {ScreenKey::Quests, "view.quests.v1"},
    {ScreenKey::Habits, "view.habits.v1"},
    {ScreenKey::Learning, "view.learning.v1"},
    {ScreenKey::Character, "view.character.v1"},
    {ScreenKey::Insights, "view.insights.v1"},
    {ScreenKey::Settings, "view.settings.v1"},
}};

inline constexpr std::array<std::pair<TrackFilter, std::string_view>, 3> kQueueModeIds{{
    {TrackFilter::Mixed, "vm.queue_mode.mixed.v2"},
    {TrackFilter::LifeOnly, "vm.queue_mode.life_only.v2"},
    {TrackFilter::LearningOnly, "vm.queue_mode.learning_only.v2"},
}};

inline constexpr std::array<std::pair<RewardEffectTier, std::string_view>, 3> kRewardTierIds{{
    {RewardEffectTier::Off, "vm.reward_tier.off.v1"},
    {RewardEffectTier::Low, "vm.reward_tier.low.v1"},
    {RewardEffectTier::Full, "vm.reward_tier.full.v1"},
}};

inline constexpr std::string_view ScreenLabel(const ScreenKey screen) {
  switch (screen) {
    case ScreenKey::Today:
      return "Today";
    case ScreenKey::Quests:
      return "Quests";
    case ScreenKey::Habits:
      return "Habits";
    case ScreenKey::Learning:
      return "Learning";
    case ScreenKey::Character:
      return "Character";
    case ScreenKey::Insights:
      return "Insights";
    case ScreenKey::Settings:
      return "Settings";
  }

  return "Unknown";
}

inline constexpr std::string_view TrackFilterLabel(const TrackFilter filter) {
  switch (filter) {
    case TrackFilter::Mixed:
      return "Mixed";
    case TrackFilter::LifeOnly:
      return "Life";
    case TrackFilter::LearningOnly:
      return "Learning";
  }

  return "Mixed";
}

inline constexpr std::string_view TrackFilterToQueueModeString(const TrackFilter filter) {
  switch (filter) {
    case TrackFilter::Mixed:
      return "mixed";
    case TrackFilter::LifeOnly:
      return "life_only";
    case TrackFilter::LearningOnly:
      return "learning_only";
  }

  return "mixed";
}

inline constexpr TrackFilter TrackFilterFromQueueModeString(const std::string_view raw) {
  if (raw == "mixed") {
    return TrackFilter::Mixed;
  }
  if (raw == "life_only") {
    return TrackFilter::LifeOnly;
  }
  if (raw == "learning_only") {
    return TrackFilter::LearningOnly;
  }

  return TrackFilter::Mixed;
}

inline constexpr std::string_view PresetModeToString(const PresetMode mode) {
  switch (mode) {
    case PresetMode::Calm:
      return "calm";
    case PresetMode::Spark:
      return "spark";
    case PresetMode::Custom:
      return "custom";
  }

  return "calm";
}

inline constexpr PresetMode PresetModeFromString(const std::string_view raw) {
  if (raw == "calm") {
    return PresetMode::Calm;
  }
  if (raw == "spark") {
    return PresetMode::Spark;
  }
  if (raw == "custom") {
    return PresetMode::Custom;
  }

  return PresetMode::Calm;
}

inline constexpr std::string_view PresetModeLabel(const PresetMode mode) {
  switch (mode) {
    case PresetMode::Calm:
      return "Calm";
    case PresetMode::Spark:
      return "Spark";
    case PresetMode::Custom:
      return "Custom";
  }

  return "Calm";
}

inline constexpr void ClampSensoryLevels(UiViewState* state) {
  if (state == nullptr) {
    return;
  }

  if (state->motion_level < 0) {
    state->motion_level = 0;
  }
  if (state->motion_level > 2) {
    state->motion_level = 2;
  }

  if (state->sound_level < 0) {
    state->sound_level = 0;
  }
  if (state->sound_level > 2) {
    state->sound_level = 2;
  }

  if (state->density_level < 0) {
    state->density_level = 0;
  }
  if (state->density_level > 2) {
    state->density_level = 2;
  }
}

inline constexpr void ApplyPresetBundle(UiViewState* state, const PresetMode preset_mode) {
  if (state == nullptr) {
    return;
  }

  state->preset_mode = preset_mode;

  switch (preset_mode) {
    case PresetMode::Calm:
      state->motion_level = 0;
      state->sound_level = 0;
      state->density_level = 2;
      state->last_non_custom_preset = PresetMode::Calm;
      break;
    case PresetMode::Spark:
      state->motion_level = 1;
      state->sound_level = 1;
      state->density_level = 1;
      state->last_non_custom_preset = PresetMode::Spark;
      break;
    case PresetMode::Custom:
      ClampSensoryLevels(state);
      break;
  }
}

inline constexpr void ApplySensoryOverride(UiViewState* state, const int motion, const int sound, const int density) {
  if (state == nullptr) {
    return;
  }

  state->motion_level = motion;
  state->sound_level = sound;
  state->density_level = density;
  ClampSensoryLevels(state);
  state->preset_mode = PresetMode::Custom;
}

inline constexpr void RestoreLastNonCustomPreset(UiViewState* state) {
  if (state == nullptr) {
    return;
  }

  ApplyPresetBundle(state, state->last_non_custom_preset);
}

inline constexpr RewardEffectTier ResolveRewardEffectTier(const int motion_level, const int sound_level) {
  if (motion_level <= 0 && sound_level <= 0) {
    return RewardEffectTier::Off;
  }

  if (motion_level >= 2 || sound_level >= 2) {
    return RewardEffectTier::Full;
  }

  return RewardEffectTier::Low;
}

inline constexpr std::string_view RewardEffectTierLabel(const RewardEffectTier tier) {
  switch (tier) {
    case RewardEffectTier::Off:
      return "Off";
    case RewardEffectTier::Low:
      return "Low";
    case RewardEffectTier::Full:
      return "Full";
  }

  return "Off";
}

inline constexpr std::string_view RewardEffectTierKey(const RewardEffectTier tier) {
  switch (tier) {
    case RewardEffectTier::Off:
      return "off";
    case RewardEffectTier::Low:
      return "low";
    case RewardEffectTier::Full:
      return "full";
  }

  return "off";
}

inline constexpr RewardEffectTier RewardEffectTierFromString(const std::string_view raw) {
  if (raw == "off") {
    return RewardEffectTier::Off;
  }
  if (raw == "low") {
    return RewardEffectTier::Low;
  }
  if (raw == "full") {
    return RewardEffectTier::Full;
  }

  return RewardEffectTier::Off;
}

}  // namespace habitrpg::ui::contracts
