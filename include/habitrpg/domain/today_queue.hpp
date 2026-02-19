#pragma once

#include <vector>

#include "habitrpg/domain/entities.hpp"
#include "habitrpg/ui/contracts.hpp"

namespace habitrpg::domain {

struct TodayQueueItem {
  std::string unit_id;
  std::string parent_id;
  std::string title;
  TrackType track_type{TrackType::Life};
  LifecycleState lifecycle_state{LifecycleState::Ready};
  int priority_score{100};
  std::string source_kind;  // action_unit | learning_session
};

class TodayQueueService {
 public:
  std::vector<TodayQueueItem> BuildQueue(
      ui::contracts::TrackFilter filter,
      const std::vector<ActionUnit>& action_units,
      const std::vector<LearningSession>& learning_sessions,
      size_t max_items = 12) const;

 private:
  static int RankWeight(LifecycleState state);
  static bool HigherPriority(const TodayQueueItem& left, const TodayQueueItem& right);
  static std::vector<TodayQueueItem> BuildLifeItems(const std::vector<ActionUnit>& action_units);
  static std::vector<TodayQueueItem> BuildLearningItems(const std::vector<LearningSession>& learning_sessions);
  static std::vector<TodayQueueItem> ComposeMixed(
      const std::vector<TodayQueueItem>& life_items,
      const std::vector<TodayQueueItem>& learning_items,
      size_t max_items);
};

}  // namespace habitrpg::domain
