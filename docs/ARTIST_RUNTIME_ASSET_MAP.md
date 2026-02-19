# HabitRPG Runtime Asset Map (Round 3)

Date: February 19, 2026
Owner: Artist Agent
Source shell: `/Users/danielsinkin/GitHub_private/HabitRPG/src/ui/dockspace_shell.cpp`
Machine maps:
- `/Users/danielsinkin/GitHub_private/HabitRPG/assets/ui/runtime/asset_map_v3.json`
- `/Users/danielsinkin/GitHub_private/HabitRPG/assets/ui/runtime/raster_map_v1.json`

## 1. Purpose
Provide deterministic component/state-to-asset mapping for the implemented shell, including production raster packs for low-overhead loading.

## 2. Canonical vs Raster Contract

1. SVG remains canonical master for every icon/effect.
2. PNG raster pack exists at required sizes: `16`, `20`, `24`, `48`, `64`.
3. Runtime resolution order:
- exact requested size PNG
- next larger size PNG
- nearest smaller size PNG
- canonical SVG

## 3. Raster Pack Structure

Root: `/Users/danielsinkin/GitHub_private/HabitRPG/assets/ui/raster`

- `assets/ui/raster/16/{asset}_s16.png`
- `assets/ui/raster/20/{asset}_s20.png`
- `assets/ui/raster/24/{asset}_s24.png`
- `assets/ui/raster/48/{asset}_s48.png`
- `assets/ui/raster/64/{asset}_s64.png`

Deterministic mapping table is machine-readable in:
- `/Users/danielsinkin/GitHub_private/HabitRPG/assets/ui/runtime/raster_map_v1.json`

## 4. Implemented Shell Component Mapping

### 4.1 Navigation (`RenderLeftNavigation`)

| Component State | Canonical Asset |
|---|---|
| `Navigation.NavItem.Today.default/hover/active/disabled` | `assets/ui/icons/icon_today_outline_v1.svg` |
| `Navigation.NavItem.Quests.default/hover/active/disabled` | `assets/ui/icons/icon_quests_outline_v1.svg` |
| `Navigation.NavItem.Habits.default/hover/active/disabled` | `assets/ui/icons/icon_habits_outline_v1.svg` |
| `Navigation.NavItem.Learning.default/hover/active/disabled` | `assets/ui/icons/icon_learning_outline_v1.svg` |
| `Navigation.NavItem.Character.default/hover/active/disabled` | `assets/ui/icons/icon_character_outline_v1.svg` |
| `Navigation.NavItem.Insights.default/hover/active/disabled` | `assets/ui/icons/icon_insights_outline_v1.svg` |
| `Navigation.NavItem.Settings.default/hover/active/disabled` | `assets/ui/icons/icon_settings_outline_v1.svg` |
| `Navigation.TrackFilter.*.selected` | `assets/ui/feedback/indicator_completion_ring_v1.svg` |

### 4.2 Center Action Panel (`RenderCenterActionPanel`)

| Component State | Canonical Asset |
|---|---|
| `ActionPanel.DoNextQueueItem.Life.ready` | `assets/ui/icons/icon_habits_outline_v1.svg` |
| `ActionPanel.DoNextQueueItem.Life.active` | `assets/ui/feedback/fx_life_complete_xlow_v1.svg` |
| `ActionPanel.DoNextQueueItem.Learning.ready` | `assets/ui/icons/icon_learning_outline_v1.svg` |
| `ActionPanel.DoNextQueueItem.Learning.active` | `assets/ui/feedback/fx_learning_complete_xlow_v1.svg` |
| `ActionPanel.StartNextButton.pressed_feedback` | `assets/ui/feedback/fx_xp_gain_xlow_v1.svg` |
| `ActionPanel.CompleteCurrentButton.life_complete` | `assets/ui/feedback/fx_life_complete_low_v1.svg` |
| `ActionPanel.CompleteCurrentButton.learning_complete` | `assets/ui/feedback/fx_learning_complete_low_v1.svg` |
| `ActionPanel.ScreenPlaceholder.*` | matching nav icon for active screen |

### 4.3 Right State Panel (`RenderRightStatePanel`)

| Component State | Canonical Asset |
|---|---|
| `StatePanel.XPProgress.default` | `assets/ui/icons/icon_xp_level_outline_v1.svg` |
| `StatePanel.XPProgress.gain_feedback` | `assets/ui/feedback/fx_xp_gain_low_v1.svg` |
| `StatePanel.Energy.default` | `assets/ui/icons/icon_energy_outline_v1.svg` |
| `StatePanel.Milestone.default` | `assets/ui/icons/icon_mastery_milestone_outline_v1.svg` |
| `StatePanel.Milestone.badge` | `assets/ui/feedback/badge_mastery_milestone_marker_v1.svg` |

### 4.4 Bottom Session Controls (`RenderBottomControlStrip`)

| Component State | Canonical Asset |
|---|---|
| `SessionControls.StartFocusSession.pressed_feedback` | `assets/ui/feedback/fx_xp_gain_xlow_v1.svg` |
| `SessionControls.StopFocusSession.pressed_feedback` | `assets/ui/feedback/fx_life_complete_xlow_v1.svg` |
| `SessionControls.BreakPrompt.pressed_feedback` | `assets/ui/feedback/fx_xp_gain_xlow_v1.svg` |
| `SessionControls.DoneForNow.pressed_feedback` | `assets/ui/feedback/indicator_completion_ring_v1.svg` |

## 5. Tiered Feedback Families

- `life_complete`: `off`, `xlow`, `low`, `full`
- `learning_complete`: `off`, `xlow`, `low`, `full`
- `milestone_unlock`: `off`, `low`, `full`
- `xp_gain`: `xlow`, `low`, `full`

## 6. Deterministic Fallback Mapping

For requested tier `T`, resolve first existing tier:

1. `full -> low -> xlow -> off`
2. `low -> xlow -> off`
3. `xlow -> off`
4. `off -> off`

If signature family is missing:

1. `life_complete` and `learning_complete` -> `assets/ui/feedback/indicator_completion_ring_v1.svg`
2. `milestone_unlock` -> `assets/ui/feedback/badge_mastery_milestone_marker_v1.svg`
3. `xp_gain` -> `assets/ui/feedback/indicator_completion_ring_v1.svg`

If no candidate resolves, render static text confirmation only.

## 7. Cooldown-Safe Chained Behavior

1. High-intensity cooldown: `30,000ms`
2. Repeat window: `12,000ms`
3. Repeat threshold: `>=3` completion events in window
4. During repeat window after threshold:
- force `xlow` for life/learning completion and xp gain
- keep milestone unlock at requested tier unless motion is `Off`
5. Learning session chain limit:
- max one chained follow-up effect per completion
- max chain effect duration `120ms`
