#include <stdexcept>
#include <string>
#include <vector>

#include "habitrpg/domain/today_queue.hpp"

namespace {

void Expect(bool condition, const std::string& message) {
  if (!condition) {
    throw std::runtime_error(message);
  }
}

habitrpg::domain::ActionUnit BuildAction(
    const std::string& id,
    const habitrpg::domain::LifecycleState state,
    const int priority) {
  habitrpg::domain::ActionUnit action{};
  action.id = id;
  action.parent_id = "habit";
  action.title = id;
  action.track_type = habitrpg::domain::TrackType::Life;
  action.lifecycle_state = state;
  action.priority_score = priority;
  return action;
}

habitrpg::domain::LearningSession BuildSession(
    const std::string& id,
    const habitrpg::domain::LifecycleState state,
    const int priority) {
  habitrpg::domain::LearningSession session{};
  session.id = id;
  session.goal_id = "goal";
  session.title = id;
  session.lifecycle_state = state;
  session.priority_score = priority;
  return session;
}

}  // namespace

bool RunTodayQueueRankingTest() {
  habitrpg::domain::TodayQueueService queue_service;

  const std::vector<habitrpg::domain::ActionUnit> actions{
      BuildAction("life_ready_high", habitrpg::domain::LifecycleState::Ready, 180),
      BuildAction("life_active_low", habitrpg::domain::LifecycleState::Active, 50),
      BuildAction("life_paused_high", habitrpg::domain::LifecycleState::Paused, 200),
      BuildAction("life_complete", habitrpg::domain::LifecycleState::Completed, 300),
  };

  const std::vector<habitrpg::domain::LearningSession> sessions{};

  const auto queue = queue_service.BuildQueue(
      habitrpg::ui::contracts::TrackFilter::LifeOnly,
      actions,
      sessions,
      10);

  Expect(queue.size() == 3, "Completed items must be excluded from pending queue");
  Expect(queue.front().unit_id == "life_active_low", "Active state should outrank all other pending states");
  Expect(queue[1].unit_id == "life_ready_high", "Ready should outrank paused with deterministic ranking");
  return true;
}

bool RunMixedQueueCompositionTest() {
  habitrpg::domain::TodayQueueService queue_service;

  const std::vector<habitrpg::domain::ActionUnit> actions{
      BuildAction("life_1", habitrpg::domain::LifecycleState::Ready, 150),
      BuildAction("life_2", habitrpg::domain::LifecycleState::Ready, 120),
      BuildAction("life_3", habitrpg::domain::LifecycleState::Ready, 90),
  };

  const std::vector<habitrpg::domain::LearningSession> sessions{
      BuildSession("learn_1", habitrpg::domain::LifecycleState::Active, 110),
      BuildSession("learn_2", habitrpg::domain::LifecycleState::Ready, 100),
      BuildSession("learn_3", habitrpg::domain::LifecycleState::Ready, 80),
  };

  const auto queue = queue_service.BuildQueue(
      habitrpg::ui::contracts::TrackFilter::Mixed,
      actions,
      sessions,
      6);

  Expect(queue.size() == 6, "Mixed queue should include deterministic merged items");
  Expect(queue.front().track_type != queue[1].track_type, "Mixed mode must include both tracks early");

  for (size_t i = 1; i < queue.size(); ++i) {
    Expect(
        queue[i - 1].track_type != queue[i].track_type,
        "Mixed mode must alternate tracks while both tracks still have pending items");
  }

  return true;
}
