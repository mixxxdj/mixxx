# 07 — Focused Execution Tickets

This file turns the dependency-integration plan into focused tickets for the final PR-readiness pass.

## Current working assumptions

These assumptions reflect the latest available direction and should be treated as the starting point unless Mixxx maintainers request otherwise:

1. `BUNGEE` remains default `ON` for this PR.
2. It is acceptable to increase the **recommended** CMake version for Bungee-enabled builds that consume the most recent Bungee release.
3. The implementation may support an older Bungee release when the installed CMake is older than the newest Bungee requires, provided that older release is API-compatible with Mixxx's scaler integration and can be validated.
4. Future decision requests must include a short pros/cons list and recommendation, not only an open-ended question.
5. No final PR should rely on checked-in `lib/bungee/` source or configure-time patching of that source.

## Decision format for future maintainer/user questions

When a ticket needs a decision, present it in this format:

```text
Decision needed: <short question>

Option A: <name>
Pros:
- ...
Cons:
- ...

Option B: <name>
Pros:
- ...
Cons:
- ...

Recommendation: <agent recommendation and why>
Impact if deferred: <what work is blocked or what temporary behavior remains>
```

## Ticket sequence

| Ticket | Purpose | Blocks |
| --- | --- | --- |
| BNG-17 | Capture current assumptions and turn unresolved decisions into pros/cons prompts. | BNG-18 |
| BNG-18 | Validate latest-vs-older Bungee version matrix against CMake requirements and Mixxx API needs. | BNG-19, BNG-20, BNG-21 |
| BNG-19 | Normalize Mixxx CMake discovery to a single `Bungee::Bungee` target. | BNG-20, BNG-21, BNG-22 |
| BNG-20 | Choose/implement the Mixxx vcpkg buildenv path using Microsoft vcpkg PR #50120 where possible. | BNG-22, BNG-23 |
| BNG-21 | Implement an approved pinned source fallback and CMake-version selection, or document fallback rejection. | BNG-23 if enabled |
| BNG-22 | Remove vendored `lib/bungee/` source and direct-build wiring. | BNG-23, BNG-24 |
| BNG-23 | Update packaging and CI paths for non-vendored Bungee. | BNG-24 |
| BNG-24 | Prepare trunk-driven PR history, per-commit validation records, and final PR description. | final PR readiness |

## Implementation order

1. Complete BNG-17 and BNG-18 before touching build logic. The version/CMake matrix determines whether the fallback is simple or conditional.
2. Complete BNG-19 before vendor removal. Mixxx should link against `Bungee::Bungee` first, then remove the old provider.
3. Complete BNG-20 before relying on Windows/macOS CI. The official buildenv path should not depend on local source.
4. Do BNG-21 only if the fallback is accepted or needed for developer/Linux builds.
5. Complete BNG-22 as a focused deletion/wiring commit after dependency discovery is working.
6. Complete BNG-23 to make every platform explicit: enabled with a dependency path, or intentionally disabled with rationale.
7. Complete BNG-24 last; do not mark the PR ready until every commit is reviewable and validation evidence is recorded.

## CMake/Bungee version strategy to validate in BNG-18

The preferred shape is:

```text
if BUNGEE=OFF:
  do not find or build Bungee

if BUNGEE=ON:
  first try packaged dependencies:
    - Bungee CONFIG package, preferably latest stable
    - vcpkg unofficial-bungee from PR #50120
    - pkg-config/module discovery if available

  if package discovery fails and an approved source fallback exists:
    - use latest validated Bungee when installed CMake is new enough
    - otherwise use the newest validated older Bungee that supports the installed CMake

  if no dependency/fallback is available:
    - fail configure with an actionable message
```

Important distinction: increasing the recommended CMake version for Bungee-enabled builds is acceptable, but raising Mixxx's global hard minimum should still be a deliberate final choice with pros/cons because it affects all contributors, including those building with `-DBUNGEE=OFF`.

## Trunk-driven PR expectations

BNG-24 should enforce the branch discipline from `06-branch-ci-discipline.md`:

- each commit should be focused and independently buildable;
- no commit should require a later fixup to pass required checks;
- generated/build artifacts must not be included;
- CI run URLs and local validation commands should be recorded per commit;
- any skipped platform/job must have an explicit rationale.
