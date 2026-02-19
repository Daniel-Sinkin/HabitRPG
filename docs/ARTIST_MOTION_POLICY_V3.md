# HabitRPG Motion Policy (Round 3)

Date: February 19, 2026
Owner: Artist Agent

## 1. Policy Goals

1. Keep reward feedback motivating without sensory overload.
2. Make motion deterministic and bounded per signature and tier.
3. Enforce strict `motion=Off` override with no chained effects.

## 2. Tier Definitions

- `off`: no transform animation; static swap only.
- `xlow`: single opacity emphasis or one-frame pulse.
- `low`: single transform channel (scale or alpha).
- `full`: up to two transform channels (scale + alpha or sweep + alpha).

## 3. Timing and Easing Table

| Signature | Tier | Duration (ms) | Easing | Chained Effects |
|---|---|---:|---|---|
| `life_complete` | `off` | 0 | none | none |
| `life_complete` | `xlow` | 60-100 | linear-out | none |
| `life_complete` | `low` | 100-150 | cubic-out | none |
| `life_complete` | `full` | 160-220 | cubic-out | optional 1 follow-up (<=120ms) |
| `learning_complete` | `off` | 0 | none | none |
| `learning_complete` | `xlow` | 60-100 | linear-out | none |
| `learning_complete` | `low` | 110-160 | cubic-out | none |
| `learning_complete` | `full` | 170-240 | cubic-out | optional 1 follow-up (<=120ms) |
| `milestone_unlock` | `off` | 0 | none | none |
| `milestone_unlock` | `xlow` | 80-110 | linear-out | none |
| `milestone_unlock` | `low` | 140-190 | cubic-out | none |
| `milestone_unlock` | `full` | 220-320 | quint-out | optional 1 follow-up (<=120ms) |
| `xp_gain` | `off` | 0 | none | none |
| `xp_gain` | `xlow` | 60-90 | linear-out | none |
| `xp_gain` | `low` | 90-130 | cubic-out | none |
| `xp_gain` | `full` | 130-180 | cubic-out | none |

## 4. Global Safety Rules

1. Never animate all dock panels simultaneously.
2. No camera shake.
3. No infinite loops.
4. High-intensity cooldown: `>=30s` between full-tier celebration effects.
5. If `>=3` completion events occur within `12s`, downgrade completion/xp effects to `xlow`.

## 5. Strict Motion-Off Override

When `motion=Off`:

1. Force tier to `off` regardless of `sound` level.
2. Disable all chained follow-ups.
3. Disable sweep/scale transforms.
4. Render static confirmation only (icon, color, or text state update).

This override is absolute and must not be bypassed by Spark/custom settings.

## 6. Preset Defaults

1. `calm` default tier: `off`
2. `spark` default tier: `low`
3. `custom`: user-selected tier, with guardrails and cooldown still applied
