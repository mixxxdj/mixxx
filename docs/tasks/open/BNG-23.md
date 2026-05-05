---
id: BNG-23
title: "Update packaging and CI paths for non-vendored Bungee"
status: open
priority: medium
effort: medium
group: BNG
package: root
labels: [bungee, packaging, ci]
blocked_by: [BNG-20, BNG-22]
created: '2026-05-05'
---

## BNG-23: Update packaging and CI paths for non-vendored Bungee

## Context

After vendor removal, every official build target needs either an explicit Bungee dependency path or an explicit `BUNGEE=OFF` decision. Because this PR should default `BUNGEE=ON`, unsupported platforms need deliberate handling rather than accidental breakage.

## Scope

- Update or document Debian/Ubuntu packaging behavior.
- Add/update Flatpak module if Bungee should be included there.
- Confirm Windows/macOS CI resolve Bungee through the Mixxx vcpkg buildenv.
- Decide Linux CI behavior: system package, source fallback, or temporary explicit disable with rationale.
- Decide Android/iOS behavior: enabled if validated, otherwise explicit `BUNGEE=OFF` with rationale.
- Ensure the Bungee ASan workflow still runs against the non-vendored dependency path or an approved fallback.

## Acceptance criteria

- Every official Mixxx build path has explicit Bungee handling.
- CI no longer relies on local `lib/bungee/`.
- `BUNGEE` remains default `ON` for the PR unless a platform explicitly overrides it with documented rationale.
- Packaging docs mention Bungee license/provenance expectations.
- Any remaining platform decision request includes pros/cons.
