#include "habitrpg/domain/today_queue.hpp"

#include <algorithm>

namespace habitrpg::domain {

int TodayQueueService::RankWeight(const LifecycleState state) {
  switch (state) {
    case LifecycleState::Active:
      return 700;
    case LifecycleState::Partial:
      return 600;
    case LifecycleState::Ready:
      return 500;
    case LifecycleState::CheckpointCandidate:
      return 450;
    case LifecycleState::Missed:
      return 400;
    case LifecycleState::Paused:
      return 300;
    case LifecycleState::Completed:
      return 0;
  }

  return 0;
}

bool TodayQueueService::HigherPriority(const TodayQueueItem& left, const TodayQueueItem& right) {
  const int left_score = RankWeight(left.lifecycle_state) + left.priority_score;
  const int right_score = RankWeight(right.lifecycle_state) + right.priority_score;

  if (left_score != right_score) {
    return left_score > right_score;
  }

  if (left.track_type != right.track_type) {
    return left.track_type == TrackType::Life;
  }

  return left.unit_id < right.unit_id;
}

std::vector<TodayQueueItem> TodayQueueService::BuildLifeItems(const std::vector<ActionUnit>& action_units) {
  std::vector<TodayQueueItem> items;
  items.reserve(action_units.size());

  for (const auto& action_unit : action_units) {
    if (!LifecycleStateIsPending(action_unit.lifecycle_state)) {
      continue;
    }

    TodayQueueItem item{};
    item.unit_id = action_unit.id;
    item.parent_id = action_unit.parent_id;
    item.title = action_unit.title;
    item.track_type = TrackType::Life;
    item.lifecycle_state = action_unit.lifecycle_state;
    item.priority_score = action_unit.priority_score;
    item.source_kind = "action_unit";
    items.push_back(std::move(item));
  }

  std::sort(items.begin(), items.end(), HigherPriority);
  return items;
}

std::vector<TodayQueueItem> TodayQueueService::BuildLearningItems(
    const std::vector<LearningSession>& learning_sessions) {
  std::vector<TodayQueueItem> items;
  items.reserve(learning_sessions.size());

  for (const auto& learning_session : learning_sessions) {
    if (!LifecycleStateIsPending(learning_session.lifecycle_state)) {
      continue;
    }

    TodayQueueItem item{};
    item.unit_id = learning_session.id;
    item.parent_id = learning_session.goal_id;
    item.title = learning_session.title;
    item.track_type = TrackType::Learning;
    item.lifecycle_state = learning_session.lifecycle_state;
    item.priority_score = learning_session.priority_score;
    item.source_kind = "learning_session";
    items.push_back(std::move(item));
  }

  std::sort(items.begin(), items.end(), HigherPriority);
  return items;
}

std::vector<TodayQueueItem> TodayQueueService::ComposeMixed(
    const std::vector<TodayQueueItem>& life_items,
    const std::vector<TodayQueueItem>& learning_items,
    const size_t max_items) {
  if (max_items == 0) {
    return {};
  }

  if (life_items.empty()) {
    return std::vector<TodayQueueItem>(learning_items.begin(), learning_items.begin() +
                                                                   std::min(max_items, learning_items.size()));
  }

  if (learning_items.empty()) {
    return std::vector<TodayQueueItem>(life_items.begin(), life_items.begin() +
                                                               std::min(max_items, life_items.size()));
  }

  std::vector<TodayQueueItem> mixed;
  mixed.reserve(std::min(max_items, life_items.size() + learning_items.size()));

  size_t life_index = 0;
  size_t learning_index = 0;

  const bool life_starts = HigherPriority(life_items.front(), learning_items.front());
  TrackType next_track = life_starts ? TrackType::Life : TrackType::Learning;

  while (mixed.size() < max_items && (life_index < life_items.size() || learning_index < learning_items.size())) {
    if (next_track == TrackType::Life) {
      if (life_index < life_items.size()) {
        mixed.push_back(life_items[life_index++]);
      } else if (learning_index < learning_items.size()) {
        mixed.push_back(learning_items[learning_index++]);
      }
      next_track = TrackType::Learning;
    } else {
      if (learning_index < learning_items.size()) {
        mixed.push_back(learning_items[learning_index++]);
      } else if (life_index < life_items.size()) {
        mixed.push_back(life_items[life_index++]);
      }
      next_track = TrackType::Life;
    }
  }

  return mixed;
}

std::vector<TodayQueueItem> TodayQueueService::BuildQueue(
    const ui::contracts::TrackFilter filter,
    const std::vector<ActionUnit>& action_units,
    const std::vector<LearningSession>& learning_sessions,
    const size_t max_items) const {
  const auto life_items = BuildLifeItems(action_units);
  const auto learning_items = BuildLearningItems(learning_sessions);

  if (filter == ui::contracts::TrackFilter::LifeOnly) {
    return std::vector<TodayQueueItem>(life_items.begin(), life_items.begin() +
                                                              std::min(max_items, life_items.size()));
  }

  if (filter == ui::contracts::TrackFilter::LearningOnly) {
    return std::vector<TodayQueueItem>(learning_items.begin(), learning_items.begin() +
                                                                  std::min(max_items, learning_items.size()));
  }

  return ComposeMixed(life_items, learning_items, max_items);
}

}  // namespace habitrpg::domain
