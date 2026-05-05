---
id: BNG-18
title: "Validate Bungee version and CMake compatibility matrix"
status: done
priority: high
effort: small
group: BNG
package: root
labels: [bungee, cmake, dependency-versioning]
blocked_by: [BNG-17]
created: '2026-05-05'
completed: '2026-05-05'
---

## BNG-18: Validate Bungee version and CMake compatibility matrix

## Context

The implementation should prefer the most recent stable Bungee release, even if that means recommending a newer CMake for Bungee-enabled builds. However, Mixxx should not unnecessarily force all builders onto that newest CMake if an older Bungee release can support Mixxx's current CMake minimum.

## Scope

- Identify the latest stable Bungee tag suitable for Mixxx.
- Identify its declared CMake minimum and any practical CMake features it requires.
- Identify the newest older Bungee tag, if any, that builds with Mixxx's current top-level CMake minimum.
- Record compile/link/API differences relevant to `EngineBufferScaleBungee`.
- Decide whether the older-version path is feasible without code forks or unacceptable behavior differences.
- Document the result in the plan before build-system implementation.

## Acceptance criteria

- A table exists in the plan listing candidate Bungee versions, required CMake version, dependency shape, and Mixxx API compatibility.
- The preferred latest-version path and older-CMake fallback path are explicit.
- If no safe older-version fallback exists, the plan explains why and provides pros/cons for raising the required/recommended CMake version instead.
- No changes under `lib/bungee`.
