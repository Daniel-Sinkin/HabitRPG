# HabitRPG Artist Visual System (Round 3)

Date: February 19, 2026
Owner: Artist Agent

## 1. Art Direction Board

### 1.1 Primary Direction: Signal Garden
A calm, dark-interface visual language with teal-and-amber reward accents.

- Tone: grounded, clear, minimally gamey.
- Contrast pattern: dark neutrals + bright text + controlled accents.
- Reward feel: soft ring/glow, no explosive particles.
- Baseline: Calm defaults, Spark is additive.

### 1.2 Fallback Variant: Paper Console
A light-neutral variant for users sensitive to dark themes.

- Tone: low-ink paper UI with slate text and cyan/amber accents.
- Usage: bright environments/daylight legibility.
- Geometry remains identical to primary; only style tokens differ.

### 1.3 High-Contrast Variants (Round 3)

- Primary high-contrast: `/Users/danielsinkin/GitHub_private/HabitRPG/assets/ui/themes/theme_signal_garden_high_contrast_v1.json`
- Fallback high-contrast: `/Users/danielsinkin/GitHub_private/HabitRPG/assets/ui/themes/theme_paper_console_high_contrast_v1.json`

Usage intent:
- Dense panels
- smallest supported text sizes
- keyboard-heavy sessions where focus indicators must remain strong

## 2. Core Style Tokens

## 2.1 Base Palette (Primary)

| Token | Value |
|---|---|
| `color.bg.canvas` | `#0E131A` |
| `color.bg.surface_1` | `#151C25` |
| `color.bg.surface_2` | `#1D2733` |
| `color.bg.surface_3` | `#243243` |
| `color.text.primary` | `#EAF1F7` |
| `color.text.secondary` | `#AAB9C8` |
| `color.text.muted` | `#7F8FA0` |
| `color.stroke.default` | `#314255` |
| `color.accent.primary` | `#43C3AA` |
| `color.accent.secondary` | `#F0B66C` |

## 2.2 Semantic Colors

| Token | Value |
|---|---|
| `color.semantic.success` | `#52C878` |
| `color.semantic.warning` | `#E5AA43` |
| `color.semantic.error` | `#D96B6B` |
| `color.semantic.info` | `#5FAEE6` |
| `color.semantic.neutral` | `#8A99AA` |

## 2.3 Track Accents

| Token | Value |
|---|---|
| `color.track.life` | `#4BC8A8` |
| `color.track.learning` | `#68A8FF` |

## 2.4 Spacing Rhythm (4-point)

| Token | px |
|---|---|
| `space.1` | 4 |
| `space.2` | 8 |
| `space.3` | 12 |
| `space.4` | 16 |
| `space.5` | 20 |
| `space.6` | 24 |
| `space.8` | 32 |
| `space.10` | 40 |

## 2.5 Border Radius (ImGui mapping)

| Token | px | ImGui field |
|---|---|---|
| `radius.sm` | 4 | `TabRounding` |
| `radius.md` | 6 | `FrameRounding`, `GrabRounding` |
| `radius.lg` | 8 | `PopupRounding`, `ChildRounding` |
| `radius.xl` | 10 | `WindowRounding` |

## 2.6 Typography Guidance (ImGui)

Families:
- UI: Atkinson Hyperlegible (fallback Noto Sans)
- Numeric/technical: JetBrains Mono (telemetry only)

Sizes:
- `type.xs=12`
- `type.sm=13`
- `type.md=15`
- `type.lg=18`
- `type.xl=24`

## 3. Iconography System

- Grid: `24x24`
- Stroke: `1.8px` round caps/joins
- Fill: none by default
- Shape grammar:
- circles for state
- rounded rectangles for navigation
- star/shield motifs for milestone and reward

Production exports:
- canonical SVG masters in `assets/ui/icons`
- pre-rasterized PNG set in `assets/ui/raster` at `16/20/24/48/64`

## 4. Feedback Visual Signatures

1. Life completion: check + soft ring
2. Learning completion: bracket + progress arc
3. Milestone unlock: badge shield + restrained rays

Tier family:
- `off`, `xlow`, `low`, `full`

## 5. Stimulation Safety Rails

1. Max saturated accent area per panel: `20%`
2. Max simultaneous animated elements:
- Calm: `2`
- Spark: `4`
3. Max luminance delta for full-screen feedback event:
- Low: `<=8%`
- Full: `<=16%`
4. No continuous pulsing on primary workflow controls.
5. Motion-off is strict override (no chained effects).

## 6. File Naming and Versioning

Vector:
- `{category}_{name}_{variant}_v{major}.svg`

Raster:
- `{category}_{name}_{variant}_v{major}_s{size}.png`

Examples:
- `icon_today_outline_v1.svg`
- `fx_life_complete_xlow_v1.svg`
- `badge_mastery_milestone_marker_v1.svg`
- `icon_today_outline_v1_s20.png`

## 7. Supporting Specs

- Runtime map: `/Users/danielsinkin/GitHub_private/HabitRPG/docs/ARTIST_RUNTIME_ASSET_MAP.md`
- Theme handoff: `/Users/danielsinkin/GitHub_private/HabitRPG/docs/ARTIST_IMGUI_THEME_HANDOFF.md`
- Motion policy: `/Users/danielsinkin/GitHub_private/HabitRPG/docs/ARTIST_MOTION_POLICY_V3.md`
- Accessibility validation: `/Users/danielsinkin/GitHub_private/HabitRPG/docs/ARTIST_ACCESSIBILITY_REPORT_V3.md`
