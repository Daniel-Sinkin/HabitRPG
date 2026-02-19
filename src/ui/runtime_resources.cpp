#include "habitrpg/ui/runtime_resources.hpp"

#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>

namespace habitrpg::ui {
namespace {

std::string ReadFileText(const std::string& path) {
  std::ifstream input(path);
  if (!input.is_open()) {
    return "";
  }

  std::ostringstream buffer;
  buffer << input.rdbuf();
  return buffer.str();
}

size_t SkipWhitespace(const std::string& text, size_t pos) {
  while (pos < text.size() && (text[pos] == ' ' || text[pos] == '\n' || text[pos] == '\t' || text[pos] == '\r')) {
    ++pos;
  }
  return pos;
}

std::string ExtractObjectBody(const std::string& text, const std::string& key) {
  const std::string needle = "\"" + key + "\"";
  const size_t key_pos = text.find(needle);
  if (key_pos == std::string::npos) {
    return "";
  }

  size_t colon_pos = text.find(':', key_pos + needle.size());
  if (colon_pos == std::string::npos) {
    return "";
  }

  size_t object_start = SkipWhitespace(text, colon_pos + 1);
  if (object_start >= text.size() || text[object_start] != '{') {
    return "";
  }

  int depth = 1;
  size_t i = object_start + 1;
  while (i < text.size() && depth > 0) {
    if (text[i] == '{') {
      ++depth;
    } else if (text[i] == '}') {
      --depth;
    }
    ++i;
  }

  if (depth != 0 || i <= object_start + 1) {
    return "";
  }

  return text.substr(object_start + 1, (i - object_start) - 2);
}

std::unordered_map<std::string, std::string> ParseStringPairs(const std::string& object_body) {
  std::unordered_map<std::string, std::string> pairs;
  const std::regex pair_regex("\"([^\"]+)\"\\s*:\\s*\"([^\"]*)\"");

  std::sregex_iterator it(object_body.begin(), object_body.end(), pair_regex);
  std::sregex_iterator end;
  for (; it != end; ++it) {
    pairs[(*it)[1].str()] = (*it)[2].str();
  }

  return pairs;
}

std::unordered_map<std::string, std::string> ParseTopLevelObjectBodies(const std::string& object_body) {
  std::unordered_map<std::string, std::string> out;

  size_t i = 0;
  while (i < object_body.size()) {
    i = SkipWhitespace(object_body, i);
    if (i >= object_body.size() || object_body[i] != '"') {
      ++i;
      continue;
    }

    const size_t key_start = i + 1;
    const size_t key_end = object_body.find('"', key_start);
    if (key_end == std::string::npos) {
      break;
    }

    const std::string key = object_body.substr(key_start, key_end - key_start);

    size_t colon = object_body.find(':', key_end + 1);
    if (colon == std::string::npos) {
      break;
    }

    size_t value_start = SkipWhitespace(object_body, colon + 1);
    if (value_start >= object_body.size() || object_body[value_start] != '{') {
      i = value_start + 1;
      continue;
    }

    int depth = 1;
    size_t j = value_start + 1;
    while (j < object_body.size() && depth > 0) {
      if (object_body[j] == '{') {
        ++depth;
      } else if (object_body[j] == '}') {
        --depth;
      }
      ++j;
    }

    if (depth != 0 || j <= value_start + 1) {
      break;
    }

    out[key] = object_body.substr(value_start + 1, (j - value_start) - 2);
    i = j;
  }

  return out;
}

std::vector<std::string> ExtractQuotedBulletsInSection(const std::string& markdown, const std::string& heading) {
  const size_t start = markdown.find(heading);
  if (start == std::string::npos) {
    return {};
  }

  size_t end = markdown.find("\n### ", start + heading.size());
  if (end == std::string::npos) {
    end = markdown.find("\n## ", start + heading.size());
  }
  if (end == std::string::npos) {
    end = markdown.size();
  }

  const std::string section = markdown.substr(start, end - start);
  const std::regex quote_regex("-\\s+\"([^\"]+)\"");

  std::vector<std::string> quotes;
  std::sregex_iterator it(section.begin(), section.end(), quote_regex);
  std::sregex_iterator finish;
  for (; it != finish; ++it) {
    quotes.push_back((*it)[1].str());
  }

  return quotes;
}

float ToFloat(const std::string& raw, const float fallback = 0.0f) {
  try {
    return std::stof(raw);
  } catch (...) {
    return fallback;
  }
}

}  // namespace

ThemeRuntime LoadThemeFromJson(const std::string& path) {
  ThemeRuntime theme;

  const std::string text = ReadFileText(path);
  if (text.empty()) {
    return theme;
  }

  const std::regex theme_id_regex("\"theme_id\"\\s*:\\s*\"([^\"]+)\"");
  std::smatch theme_id_match;
  if (std::regex_search(text, theme_id_match, theme_id_regex)) {
    theme.theme_id = theme_id_match[1].str();
  }

  const std::string style_vars = ExtractObjectBody(text, "style_vars");
  if (!style_vars.empty()) {
    const std::regex vec2_regex(
        "\"([A-Za-z0-9_]+)\"\\s*:\\s*\\[\\s*([-0-9\\.]+)\\s*,\\s*([-0-9\\.]+)\\s*\\]");
    std::sregex_iterator vec2_it(style_vars.begin(), style_vars.end(), vec2_regex);
    std::sregex_iterator vec2_end;
    for (; vec2_it != vec2_end; ++vec2_it) {
      theme.style_vec2[(*vec2_it)[1].str()] = {
          ToFloat((*vec2_it)[2].str()),
          ToFloat((*vec2_it)[3].str()),
      };
    }

    const std::regex scalar_regex("\"([A-Za-z0-9_]+)\"\\s*:\\s*([-0-9\\.]+)");
    std::sregex_iterator scalar_it(style_vars.begin(), style_vars.end(), scalar_regex);
    std::sregex_iterator scalar_end;
    for (; scalar_it != scalar_end; ++scalar_it) {
      const std::string key = (*scalar_it)[1].str();
      if (theme.style_vec2.find(key) != theme.style_vec2.end()) {
        continue;
      }
      theme.style_scalars[key] = ToFloat((*scalar_it)[2].str());
    }
  }

  const std::string colors = ExtractObjectBody(text, "colors");
  if (!colors.empty()) {
    const std::regex color_regex(
        "\"(ImGuiCol_[A-Za-z0-9_]+)\"\\s*:\\s*\\[\\s*([-0-9\\.]+)\\s*,\\s*([-0-9\\.]+)\\s*,\\s*([-0-9\\.]+)\\s*,\\s*([-0-9\\.]+)\\s*\\]");
    std::sregex_iterator color_it(colors.begin(), colors.end(), color_regex);
    std::sregex_iterator color_end;
    for (; color_it != color_end; ++color_it) {
      theme.colors[(*color_it)[1].str()] = {
          ToFloat((*color_it)[2].str()),
          ToFloat((*color_it)[3].str()),
          ToFloat((*color_it)[4].str()),
          ToFloat((*color_it)[5].str()),
      };
    }
  }

  theme.loaded = !theme.colors.empty();
  return theme;
}

AssetRuntimeMap LoadAssetRuntimeMapFromJson(const std::string& path) {
  AssetRuntimeMap asset_map;

  const std::string text = ReadFileText(path);
  if (text.empty()) {
    return asset_map;
  }

  const std::string component_block = ExtractObjectBody(text, "component_state_assets");
  if (!component_block.empty()) {
    const auto component_bodies = ParseTopLevelObjectBodies(component_block);
    for (const auto& [component, body] : component_bodies) {
      asset_map.component_state_assets[component] = ParseStringPairs(body);
    }
  }

  const std::string signatures_block = ExtractObjectBody(text, "feedback_signatures");
  if (!signatures_block.empty()) {
    const auto signature_bodies = ParseTopLevelObjectBodies(signatures_block);
    for (const auto& [signature, body] : signature_bodies) {
      asset_map.feedback_signatures[signature] = ParseStringPairs(body);
    }
  }

  const std::string fallback_rules_block = ExtractObjectBody(text, "fallback_rules");
  if (!fallback_rules_block.empty()) {
    const std::string fallback_signature_block = ExtractObjectBody(fallback_rules_block, "signature_fallback");
    if (!fallback_signature_block.empty()) {
      asset_map.signature_fallback = ParseStringPairs(fallback_signature_block);
    }

    const std::string tier_order_block = ExtractObjectBody(fallback_rules_block, "tier_resolution_order");
    if (!tier_order_block.empty()) {
      const std::regex tier_regex("\"([a-z]+)\"\\s*:\\s*\\[([^\\]]*)\\]");
      std::sregex_iterator tier_it(tier_order_block.begin(), tier_order_block.end(), tier_regex);
      std::sregex_iterator tier_end;

      const std::regex quoted_value_regex("\"([^\"]+)\"");
      for (; tier_it != tier_end; ++tier_it) {
        const std::string tier_name = (*tier_it)[1].str();
        const std::string raw_values = (*tier_it)[2].str();
        std::vector<std::string> values;

        std::sregex_iterator value_it(raw_values.begin(), raw_values.end(), quoted_value_regex);
        std::sregex_iterator value_end;
        for (; value_it != value_end; ++value_it) {
          values.push_back((*value_it)[1].str());
        }

        asset_map.tier_resolution_order[tier_name] = std::move(values);
      }
    }
  }

  asset_map.loaded = !asset_map.component_state_assets.empty() || !asset_map.feedback_signatures.empty();
  return asset_map;
}

CopyPack LoadCopyPackFromMarkdown(const std::string& path) {
  CopyPack copy_pack;

  const std::string markdown = ReadFileText(path);
  if (markdown.empty()) {
    return copy_pack;
  }

  const auto today_empty = ExtractQuotedBulletsInSection(markdown, "### 1.1 Today Empty");
  if (today_empty.size() >= 4) {
    copy_pack.today_empty_primary = today_empty[0];
    copy_pack.today_empty_secondary = today_empty[1];
    copy_pack.today_empty_action_add_habit = today_empty[2];
    copy_pack.today_empty_action_add_learning_goal = today_empty[3];
  }

  const auto conflict = ExtractQuotedBulletsInSection(markdown, "### 3.1 Active Session Conflict");
  if (conflict.size() >= 5) {
    copy_pack.conflict_active_primary = conflict[0];
    copy_pack.conflict_active_secondary = conflict[1];
    copy_pack.conflict_action_continue = conflict[2];
    copy_pack.conflict_action_pause_switch = conflict[3];
    copy_pack.conflict_action_cancel = conflict[4];
  }

  const auto life_completion = ExtractQuotedBulletsInSection(markdown, "### 4.1 Life Completion");
  if (!life_completion.empty()) {
    copy_pack.completion_life_toast = life_completion[0];
  }

  const auto learning_completion = ExtractQuotedBulletsInSection(markdown, "### 4.2 Learning Session Completion");
  if (!learning_completion.empty()) {
    copy_pack.completion_learning_toast = learning_completion[0];
  }

  const auto milestone_confirmed = ExtractQuotedBulletsInSection(markdown, "### 4.3 Milestone Confirmed");
  if (!milestone_confirmed.empty()) {
    copy_pack.completion_milestone_confirmed_toast = milestone_confirmed[0];
  }

  const auto milestone_candidate = ExtractQuotedBulletsInSection(markdown, "### 4.4 Milestone Candidate Saved");
  if (!milestone_candidate.empty()) {
    copy_pack.completion_milestone_candidate_toast = milestone_candidate[0];
  }

  const auto save_failure = ExtractQuotedBulletsInSection(markdown, "### 5.1 Save Failure (Generic)");
  if (save_failure.size() >= 4) {
    copy_pack.error_save_primary = save_failure[0];
    copy_pack.error_save_secondary = save_failure[1];
    copy_pack.error_save_action_retry = save_failure[2];
    copy_pack.error_save_action_cancel = save_failure[3];
  }

  const auto completion_save_failure = ExtractQuotedBulletsInSection(markdown, "### 5.2 Session Completion Save Failure");
  if (completion_save_failure.size() >= 5) {
    copy_pack.error_completion_save_primary = completion_save_failure[0];
    copy_pack.error_completion_save_secondary = completion_save_failure[1];
    copy_pack.error_completion_save_action_retry = completion_save_failure[2];
    copy_pack.error_completion_save_action_keep_draft = completion_save_failure[3];
    copy_pack.error_completion_save_action_cancel = completion_save_failure[4];
  }

  copy_pack.loaded = true;
  return copy_pack;
}

std::string ResolveComponentAsset(
    const AssetRuntimeMap& asset_map,
    const std::string& component,
    const std::string& state,
    const std::string& fallback) {
  const auto component_it = asset_map.component_state_assets.find(component);
  if (component_it == asset_map.component_state_assets.end()) {
    return fallback;
  }

  const auto state_it = component_it->second.find(state);
  if (state_it != component_it->second.end() && !state_it->second.empty() && state_it->second != "none") {
    return state_it->second;
  }

  const auto default_it = component_it->second.find("default");
  if (default_it != component_it->second.end() && !default_it->second.empty() && default_it->second != "none") {
    return default_it->second;
  }

  return fallback;
}

std::string ResolveFeedbackAsset(
    const AssetRuntimeMap& asset_map,
    const std::string& signature,
    const std::string& requested_tier) {
  const auto signature_it = asset_map.feedback_signatures.find(signature);
  if (signature_it == asset_map.feedback_signatures.end()) {
    const auto fallback_it = asset_map.signature_fallback.find(signature);
    if (fallback_it != asset_map.signature_fallback.end()) {
      return fallback_it->second;
    }
    return "";
  }

  std::vector<std::string> candidates;
  const auto order_it = asset_map.tier_resolution_order.find(requested_tier);
  if (order_it != asset_map.tier_resolution_order.end()) {
    candidates = order_it->second;
  } else {
    candidates.push_back(requested_tier);
  }

  for (const auto& tier : candidates) {
    const auto tier_it = signature_it->second.find(tier);
    if (tier_it != signature_it->second.end() && !tier_it->second.empty() && tier_it->second != "none") {
      return tier_it->second;
    }
  }

  const auto fallback_it = asset_map.signature_fallback.find(signature);
  if (fallback_it != asset_map.signature_fallback.end()) {
    return fallback_it->second;
  }

  return "";
}

}  // namespace habitrpg::ui
