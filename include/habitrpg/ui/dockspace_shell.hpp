#pragma once

#include "habitrpg/app/app_state.hpp"
#include "habitrpg/domain/interaction_flow.hpp"

namespace habitrpg::ui {

class DockspaceShell {
 public:
  void Render(app::AppState* app_state);

 private:
  void ApplyThemeTokens(const app::AppState& app_state) const;
  void EnsureDockLayout();
  void RenderLeftNavigation(app::AppState* app_state);
  void RenderCenterActionPanel(app::AppState* app_state);
  void RenderRightStatePanel(app::AppState* app_state);
  void RenderBottomControlStrip(app::AppState* app_state);
  void RenderTodayControls(app::AppState* app_state);
  void RenderQueueItemRow(app::AppState* app_state, const domain::TodayQueueItem& item);
  static const char* LifecycleIcon(domain::LifecycleState state);

  bool dock_layout_initialized_{false};
  domain::InteractionFlowService interaction_flow_service_{};
};

}  // namespace habitrpg::ui
