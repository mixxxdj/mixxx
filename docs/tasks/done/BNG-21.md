---
id: BNG-21
title: "Implement approved source fallback and CMake-version selection"
status: done
priority: medium
effort: medium
group: BNG
package: root
labels: [bungee, cmake, externalproject, fallback]
blocked_by: [BNG-18, BNG-19]
created: '2026-05-05'
completed: '2026-05-05'
---

## BNG-21: Implement approved source fallback and CMake-version selection

## Context

If maintainers approve a source fallback, it should be declarative and reproducible: pinned release archives, hashes, build-directory downloads, no live git clone, and no mutation of checked-in source. The fallback may also be where the latest-Bungee vs older-CMake-compatible-Bungee selection is implemented.

## Scope

- Confirm whether `ExternalProject_Add` fallback is approved. If not approved, document the decision and close this ticket as not-applicable.
- If approved, implement a fallback using pinned Bungee release archive URL(s), `URL_HASH`, stable `DOWNLOAD_NAME`, and `DOWNLOAD_DIR` under the build tree.
- Forward required toolchain and platform CMake variables.
- Select the latest Bungee when the available CMake is new enough.
- Select an older compatible Bungee only if BNG-18 proves that path is safe.
- Create/import `Bungee::Bungee` for the fallback artifact.

## Acceptance criteria

- No fallback downloads or patches mutate the Mixxx source tree.
- Fallback builds are pinned and reproducible.
- The CMake-version selection behavior is visible in configure output.
- If fallback is disabled/unavailable and `BUNGEE=ON`, configure fails with an actionable message.
- If maintainers reject fallback, the ticket leaves a documented pros/cons decision and no code changes.
