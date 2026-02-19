#include <exception>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

bool RunStartupSmokeTest();
bool RunHabitActionRewardRoundtripTest();
bool RunLearningSessionRewardRoundtripTest();
bool RunTodayQueueRankingTest();
bool RunMixedQueueCompositionTest();
bool RunQueueModePersistenceAndFilteringTest();
bool RunSingleActiveConflictResolutionTest();
bool RunLearningCheckpointLifecycleTest();
bool RunMilestoneCheckpointPromotionIdempotencyTest();
bool RunPresetModeExclusivityAndPersistenceTest();
bool RunSchemaMigrationV1ToV3Test();

int main() {
  struct TestCase {
    std::string name;
    std::function<bool()> fn;
  };

  const std::vector<TestCase> tests{
      {"startup_smoke", RunStartupSmokeTest},
      {"habit_action_reward_roundtrip", RunHabitActionRewardRoundtripTest},
      {"learning_session_reward_roundtrip", RunLearningSessionRewardRoundtripTest},
      {"today_queue_ranking", RunTodayQueueRankingTest},
      {"mixed_queue_composition", RunMixedQueueCompositionTest},
      {"queue_mode_persistence_and_filtering", RunQueueModePersistenceAndFilteringTest},
      {"single_active_conflict_resolution", RunSingleActiveConflictResolutionTest},
      {"learning_checkpoint_lifecycle", RunLearningCheckpointLifecycleTest},
      {"milestone_checkpoint_promotion_idempotency", RunMilestoneCheckpointPromotionIdempotencyTest},
      {"preset_mode_exclusivity_and_persistence", RunPresetModeExclusivityAndPersistenceTest},
      {"schema_migration_v1_to_v3", RunSchemaMigrationV1ToV3Test},
  };

  int failed_count = 0;

  for (const auto& test : tests) {
    try {
      const bool ok = test.fn();
      if (ok) {
        std::cout << "[PASS] " << test.name << '\n';
      } else {
        ++failed_count;
        std::cout << "[FAIL] " << test.name << " returned false" << '\n';
      }
    } catch (const std::exception& ex) {
      ++failed_count;
      std::cout << "[FAIL] " << test.name << " threw exception: " << ex.what() << '\n';
    }
  }

  if (failed_count > 0) {
    std::cout << failed_count << " test(s) failed" << '\n';
    return 1;
  }

  std::cout << "All tests passed" << '\n';
  return 0;
}
