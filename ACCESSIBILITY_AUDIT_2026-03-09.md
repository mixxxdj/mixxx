# Mixxx Accessibility Audit (Static) - 2026-03-09

Scope:
- Repository static review only (no runtime assistive-tech session).
- Focus on code-level indicators related to WCAG-aligned desktop accessibility.
- Guidance cross-checked against Apple, Microsoft, Google, and JAWS expectations for desktop UI semantics and keyboard accessibility.

Limitations:
- This is not a complete conformance claim for WCAG 2.1/2.2 AA.
- No end-to-end screen-reader testing session was executed in this audit pass.
- Color contrast and timing behavior were not fully measured across all skins/themes.

## Evidence Collected

Installer path evidence:
- `.github/workflows/build.yml:140` uses `cpack_generator: WIX`.
- `.github/workflows/build.yml:144` publishes `build/*.msi`.
- `.github/workflows/build.yml:482` runs `cpack -G ...`.

Accessibility baseline evidence:
- 22 occurrences of `setFocusPolicy(Qt::NoFocus)` in `src/` indicate many controls intentionally skip keyboard focus (not always wrong, but high-risk for keyboard-only operation if no equivalent path exists).
- Before this patch set, there was no explicit broad `setAccessibleName` / `setAccessibleDescription` usage found across `src/`.

## Changes Applied in This Pass

1. Added fallback accessibility metadata in shared widget bases:
- `src/widget/wwidget.cpp`
- `src/widget/wwidgetgroup.cpp`

Behavior:
- If `accessibleName` is empty, populate from `statusTip`, then `toolTip`, then `objectName`.
- If `accessibleDescription` is empty, populate from tooltip when different from the name.
- Recompute on `ToolTipChange` and `StatusTipChange` events.

Result:
- Improves discoverability for screen readers (including JAWS on Windows) without breaking existing explicit labels.

## Guideline Mapping (Practical Desktop Interpretation)

WCAG-oriented:
- 1.3.1 Info and Relationships: Improved semantic labeling via accessibility name/description fallback.
- 2.1.1 Keyboard: Audit found many `NoFocus` controls; needs targeted verification that all functionality remains keyboard reachable.
- 2.4.3 Focus Order: Requires runtime keyboard traversal tests across major workflows.
- 4.1.2 Name, Role, Value: Fallback labels improve Name coverage for custom widgets.

Apple Human Interface Guidelines (Accessibility), Microsoft Inclusive Design/UIA, Google Accessibility, JAWS expectations:
- Name/description exposure is required for assistive technology interoperability.
- Keyboard operation and predictable focus remain mandatory for non-pointer workflows.
- Programmatic control naming and purpose are expected for announcement clarity.

## Remaining Gaps (Not Fully Closed Yet)

1. Runtime screen-reader validation
- Required with JAWS on Windows using key workflows: library navigation, deck controls, transport controls, preferences dialogs.

2. Keyboard-only journey validation
- Verify every user-critical action can be performed without pointer, especially where controls are `Qt::NoFocus`.

3. Contrast and non-text visuals
- Validate focus indicators, waveform-related visuals, and skin theme contrast ratios.

4. Dialog and form labeling consistency
- Ensure form fields and action controls expose explicit labels where fallback metadata is insufficient.

## Recommended Next Audit Steps

1. Run Mixxx with JAWS and capture announcement logs for critical workflows.
2. Build a keyboard traversal checklist (Tab/Shift+Tab, arrows, shortcuts) and record pass/fail by screen.
3. Add explicit `setAccessibleName` for high-impact controls where fallback text is ambiguous.
4. Add regression tests for accessibility metadata in custom widgets where feasible.
