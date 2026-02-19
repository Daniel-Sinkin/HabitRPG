# HabitRPG ImGui Theme Handoff (Round 3)

Date: February 19, 2026
Owner: Artist Agent

## 1. Integration Files

Runtime loader bundle (exact loader key schema):
- `/Users/danielsinkin/GitHub_private/HabitRPG/assets/ui/themes/theme_runtime_loader_bundle_v1.json`

Full theme sets (token + `ImGuiCol_*` values):
- `/Users/danielsinkin/GitHub_private/HabitRPG/assets/ui/themes/theme_signal_garden_v2.json`
- `/Users/danielsinkin/GitHub_private/HabitRPG/assets/ui/themes/theme_paper_console_v2.json`
- `/Users/danielsinkin/GitHub_private/HabitRPG/assets/ui/themes/theme_signal_garden_high_contrast_v1.json`
- `/Users/danielsinkin/GitHub_private/HabitRPG/assets/ui/themes/theme_paper_console_high_contrast_v1.json`

## 2. Loader-Key Alignment (Exact)

`theme_runtime_loader_bundle_v1.json` uses the exact `StyleTokenSet` field names currently defined in:
- `/Users/danielsinkin/GitHub_private/HabitRPG/include/habitrpg/ui/contracts.hpp`

Exact keys:
1. `frame_rounding`
2. `item_spacing_x`
3. `item_spacing_y`
4. `window_bg_r`
5. `window_bg_g`
6. `window_bg_b`
7. `accent_r`
8. `accent_g`
9. `accent_b`

## 3. Runtime Contract

1. Load mode entry from `modes[requested_mode]`.
2. If mode missing, fallback to `modes.calm`.
3. For each missing key, fallback in order:
- requested mode key
- `calm` mode key
- `hard_defaults` value
4. Clamp invalid numeric values according to `missing_key_fallback.invalid_value_behavior`.

## 4. Missing-Key Fallback Behavior (Explicit)

Declared in:
- `/Users/danielsinkin/GitHub_private/HabitRPG/assets/ui/themes/theme_runtime_loader_bundle_v1.json`

Behavior:
1. Missing mode -> `default_mode` (`calm`)
2. Missing key -> lookup order `[requested_mode, calm, hard_defaults]`
3. Invalid key value -> clamp to safe range
4. If all fail -> use hard default token set

## 5. Preset and Variant Modes

Supported runtime modes in bundle:
1. `calm`
2. `spark`
3. `fallback_calm`
4. `fallback_spark`
5. `calm_high_contrast`
6. `spark_high_contrast`
7. `fallback_calm_high_contrast`
8. `fallback_spark_high_contrast`

## 6. Required Runtime Cleanup Items

1. Replace hardcoded active-nav color in `/Users/danielsinkin/GitHub_private/HabitRPG/src/ui/dockspace_shell.cpp` with theme-driven value.
2. Avoid direct manual color scaling (`accent * 0.75`, `accent * 0.55`) once loader-fed `ImGuiCol_*` values are applied.
3. Keep motion-tier logic independent from color theme loading.
