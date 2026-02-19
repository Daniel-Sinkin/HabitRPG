# HabitRPG Artist Asset Inventory (Round 3)

Date: February 19, 2026
Owner: Artist Agent

## 1. Package Structure

- `/Users/danielsinkin/GitHub_private/HabitRPG/assets/ui/icons`
- `/Users/danielsinkin/GitHub_private/HabitRPG/assets/ui/feedback`
- `/Users/danielsinkin/GitHub_private/HabitRPG/assets/ui/raster`
- `/Users/danielsinkin/GitHub_private/HabitRPG/assets/ui/runtime`
- `/Users/danielsinkin/GitHub_private/HabitRPG/assets/ui/themes`

## 2. Naming and Versioning

Vector pattern: `{category}_{name}_{variant}_v{major}.svg`
Raster pattern: `{category}_{name}_{variant}_v{major}_s{size}.png`

- SVG is canonical master for all assets.
- PNG is production raster cache artifact.
- Geometry changes increment `v{major}`.
- Color-only remaps do not increment geometry version.

Canonical badge naming:

- Mastery milestone badge marker: `badge_mastery_milestone_marker_v1.svg`
- No alternate mastery marker names are valid in active packages.

## 3. Rasterized Pack (Production)

Required sizes produced for all icon + feedback assets:
- `16`, `20`, `24`, `48`, `64`

Deterministic machine map:
- `/Users/danielsinkin/GitHub_private/HabitRPG/assets/ui/runtime/raster_map_v1.json`

Resolution policy:
1. exact-size PNG
2. next-larger PNG
3. nearest-smaller PNG
4. canonical SVG fallback

## 4. Icon Assets (Navigation + Status)

All icons:
- Native vector: `24x24`
- Production raster sizes: `16`, `20`, `24`, `48`, `64`
- Color: `currentColor` in SVG masters
- States: default, hover, active, disabled (renderer-driven)
- Motion-off fallback: unchanged (icons are static)

| Asset | Intended Use | Interaction State Mapping |
|---|---|---|
| `icon_today_outline_v1.svg` | Left nav: Today | Active when Today panel selected |
| `icon_quests_outline_v1.svg` | Left nav: Quests | Active when Quests panel selected |
| `icon_habits_outline_v1.svg` | Left nav: Habits | Active when Habits panel selected |
| `icon_learning_outline_v1.svg` | Left nav: Learning | Active when Learning panel selected |
| `icon_character_outline_v1.svg` | Left nav: Character | Active when Character panel selected |
| `icon_insights_outline_v1.svg` | Left nav: Insights | Active when Insights panel selected |
| `icon_settings_outline_v1.svg` | Left nav: Settings | Active when Settings panel selected |
| `icon_xp_level_outline_v1.svg` | Right panel: XP/Level widget | Highlight on level-up events |
| `icon_energy_outline_v1.svg` | Right panel: energy meter header | Warning tint when low-energy mode active |
| `icon_mastery_milestone_outline_v1.svg` | Learning milestone status chip | Success tint when milestone unlocked |

## 5. Feedback/Reward Assets

### 5.1 Completion and Progress Signatures

| Asset | Intended Size | Trigger | Fallback when animation disabled |
|---|---|---|---|
| `fx_life_complete_off_v1.svg` | `48x48` | Life action completion with tier Off | Render once, no transition |
| `fx_life_complete_xlow_v1.svg` | `40-48` | Life completion during rapid repeats | Replace with `fx_life_complete_off_v1.svg` |
| `fx_life_complete_low_v1.svg` | `48-64` | Life action completion with tier Low | Replace with `fx_life_complete_off_v1.svg` |
| `fx_life_complete_full_v1.svg` | `64x64` | Life action completion with tier Full | Replace with `fx_life_complete_off_v1.svg` |
| `fx_learning_complete_off_v1.svg` | `48x48` | Learning completion with tier Off | Render once, no transition |
| `fx_learning_complete_xlow_v1.svg` | `40-48` | Learning completion during rapid repeats | Replace with `fx_learning_complete_off_v1.svg` |
| `fx_learning_complete_low_v1.svg` | `48-64` | Learning completion with tier Low | Replace with `fx_learning_complete_off_v1.svg` |
| `fx_learning_complete_full_v1.svg` | `64x64` | Learning completion with tier Full | Replace with `fx_learning_complete_off_v1.svg` |
| `fx_milestone_unlock_off_v1.svg` | `48x48` | Learning milestone unlock with tier Off | Render once, no transition |
| `fx_milestone_unlock_low_v1.svg` | `56x56` | Learning milestone unlock with tier Low | Replace with `fx_milestone_unlock_off_v1.svg` |
| `fx_milestone_unlock_full_v1.svg` | `64x64` | Learning milestone unlock with tier Full | Replace with `fx_milestone_unlock_off_v1.svg` |

### 5.2 Additional Reward Markers

| Asset | Intended Size | Trigger | Fallback when animation disabled |
|---|---|---|---|
| `fx_xp_gain_xlow_v1.svg` | `32-48` | XP gain in high-frequency completion windows | Show static for one frame |
| `fx_xp_gain_low_v1.svg` | `40-56` | XP gain popover in Low mode | Show static for 120ms or single frame |
| `fx_xp_gain_full_v1.svg` | `56-64` | XP gain popover in Full mode | Replace with `fx_xp_gain_low_v1.svg` |
| `badge_mastery_milestone_marker_v1.svg` | `16-24` | Persistent mastery badge marker | Static badge only |
| `indicator_completion_ring_v1.svg` | `16-24` | Inline completion indicator | Static ring/check only |

## 6. Theme Assets

Runtime bundle (loader-key aligned):
- `theme_runtime_loader_bundle_v1.json`

Standard themes:
- `theme_signal_garden_v2.json`
- `theme_paper_console_v2.json`

High-contrast variants:
- `theme_signal_garden_high_contrast_v1.json`
- `theme_paper_console_high_contrast_v1.json`

## 7. Engineer Integration Notes

1. Preferred load order:
- preload nav/status icon rasters (`16`,`20`,`24`)
- lazy-load feedback rasters (`48`,`64`) on first use
- fallback to SVG masters if raster missing

2. Motion switch contract:
- `Off -> *_off_*`
- `XLow -> *_xlow_*`
- `Low -> *_low_*`
- `Full -> *_full_*`

3. Preset defaults:
- `calm -> Off`
- `spark -> Low`
- `custom -> user-selected tier`

4. Repeated completion safeguard:
- if `>=3` completion events in `12s`, downgrade completion + xp feedback to `XLow`

5. Performance guardrails:
- reuse cached rasters at `16/20/24/48/64`
- avoid runtime path tessellation for repeated effects
