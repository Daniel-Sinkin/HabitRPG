# HabitRPG Accessibility Validation (Round 3)

Date: February 19, 2026
Owner: Artist Agent

## 1. Scope

1. Validate critical text and control contrast for dense panels and smallest supported text sizes.
2. Confirm high-contrast variants exist for both primary and fallback themes.

Smallest supported text size from visual system:
- `type.xs = 12px`

Target ratios used:
1. Body/dense small text: `>= 4.5:1`
2. Critical controls and focus indicators: `>= 3:1` minimum, `>= 4.5:1` preferred

## 2. Theme Variants Evaluated

1. `/Users/danielsinkin/GitHub_private/HabitRPG/assets/ui/themes/theme_signal_garden_v2.json`
2. `/Users/danielsinkin/GitHub_private/HabitRPG/assets/ui/themes/theme_paper_console_v2.json`
3. `/Users/danielsinkin/GitHub_private/HabitRPG/assets/ui/themes/theme_signal_garden_high_contrast_v1.json`
4. `/Users/danielsinkin/GitHub_private/HabitRPG/assets/ui/themes/theme_paper_console_high_contrast_v1.json`

## 3. Critical Contrast Results

| Theme | Text/WindowBg | TextDisabled/WindowBg | Text/ButtonActive (effective) | Text/HeaderActive (effective) | NavCursor/WindowBg |
|---|---:|---:|---:|---:|---:|
| Signal Garden v2 | 15.04 | 5.18 | 5.73 | 6.13 | 9.48 |
| Paper Console v2 | 15.06 | 4.83 | 8.76 | 9.60 | 5.29 |
| Signal Garden High Contrast v1 | 18.32 | 11.15 | 4.69 | 5.26 | 13.24 |
| Paper Console High Contrast v1 | 18.83 | 6.42 | 11.18 | 11.95 | 5.57 |

## 4. Assessment

1. All variants pass dense/small text target (`>=4.5`) for primary and disabled text on window backgrounds.
2. All variants pass control-label readability for active button/header surfaces using effective composited backgrounds.
3. High-contrast variants substantially increase readability margins, especially for dense telemetry panels.

## 5. High-Contrast Compatibility Deliverable

Provided both required sets:

1. Primary high-contrast: `theme_signal_garden_high_contrast_v1.json`
2. Fallback high-contrast: `theme_paper_console_high_contrast_v1.json`

Both are compatible with existing theme handoff structure (`tokens` + `imgui.style_vars` + `imgui.colors`).
