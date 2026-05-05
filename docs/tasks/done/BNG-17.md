---
id: BNG-17
title: "Capture Bungee dependency integration assumptions and decision prompts"
status: done
priority: high
effort: small
group: BNG
package: root
labels: [bungee, planning, maintainer-feedback]
blocked_by: []
created: '2026-05-05'
completed: '2026-05-05'
---

## BNG-17: Capture Bungee dependency integration assumptions and decision prompts

## Status

open

## Context

The remaining Bungee work should move Mixxx from a checked-in `lib/bungee/` source copy to a normal dependency integration that maintainers are more likely to accept.

Current available direction:

- `BUNGEE` should default to `ON` for this PR.
- It is acceptable to increase Mixxx's recommended CMake version for the most recent Bungee release.
- If the installed CMake is older than the latest Bungee requires, the integration may adapt by using an older Bungee version that supports Mixxx's current CMake minimum.
- Future maintainer/user decisions should be requested with a clear pros/cons list and explanation.

## Scope

- Update the dependency-integration plan to record the above assumptions.
- Convert unresolved maintainer decisions into explicit decision points with pros/cons.
- Make the CMake/Bungee version strategy explicit before code changes begin.

## Acceptance criteria

- Plan docs state that `BUNGEE` remains default `ON` unless maintainers reject that.
- Plan docs describe a two-tier Bungee version strategy: latest Bungee for sufficiently new CMake, older compatible Bungee for older CMake environments if needed.
- Any remaining decision requests include pros/cons rather than yes/no prompts.
- No source or build-system behavior changes are made in this ticket.
