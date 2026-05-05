---
id: BNG-20
title: "Choose and implement Mixxx vcpkg buildenv path for Bungee"
status: done
priority: high
effort: medium
group: BNG
package: root
labels: [bungee, vcpkg, buildenv]
blocked_by: [BNG-18, BNG-19]
created: '2026-05-05'
completed: '2026-05-05'
---

## BNG-20: Choose and implement Mixxx vcpkg buildenv path for Bungee

## Context

Microsoft vcpkg PR #50120 added Bungee 2.4.15 as a port exposing `find_package(unofficial-bungee CONFIG)` and target `unofficial::bungee::bungee`. Mixxx should consume/backport that work rather than inventing an unrelated port.

## Scope

- Check whether the Mixxx vcpkg fork/buildenv already contains Microsoft vcpkg merge `ae92331` or equivalent Bungee port content.
- If not present, prepare a pros/cons decision note for:
  - updating/rebasing the Mixxx vcpkg baseline;
  - cherry-picking/backporting the Bungee port into `mixxxdj/vcpkg`;
  - using an overlay port temporarily.
- Implement the path selected by maintainers/user.
- Validate the actual Mixxx buildenv triplets that matter for this PR.
- Record exact Bungee/vcpkg provenance for the PR description.

## Research findings (from BNG-18)

- `mixxxdj/vcpkg` is a fork of `microsoft/vcpkg` on branch `2.6`.
- It is **477 commits behind** `microsoft/vcpkg:master` and does **not** yet contain the Bungee port.
- `microsoft/vcpkg` merged the Bungee port at commit `ae92331` (2026-03-19), version 2.4.15.
- The buildenvs are pre-built artifact archives downloaded by `tools/macos_buildenv.sh`, `tools/windows_buildenv.bat`, and updated with `tools/update_vcpkg.py`.
- Adding Bungee to the vcpkg buildenv means: (a) adding the port to `mixxxdj/vcpkg`, (b) rebuilding the artifacts, (c) updating hashes in the build-env scripts.

## Decision needed: How to add Bungee to mixxxdj/vcpkg

### Option A — Cherry-pick/backport the Bungee port from microsoft/vcpkg

Cherry-pick the specific Bungee port commit(s) from `microsoft/vcpkg` into `mixxxdj/vcpkg:2.6`.

Pros:

- Self-contained: only the Bungee-related commits arrive, no unrelated microsoft/vcpkg changes.
- Minimal risk to existing mixxxdj/vcpkg builds — only one new port is added.
- Port files can be reviewed exactly as they landed upstream.

Cons:

- mixxxdj/vcpkg stays behind microsoft/vcpkg on everything else.
- Backported port will need to be kept in sync with upstream if Bungee releases a new version before the full baseline is updated.
- Requires a PR against mixxxdj/vcpkg, which is a separate repo.

### Option B — Rebase/update mixxxdj/vcpkg baseline to include ae92331

Advance `mixxxdj/vcpkg:2.6` forward to a microsoft/vcpkg baseline that includes the Bungee port.

Pros:

- Picks up all other microsoft/vcpkg improvements since current baseline.
- Keeps mixxxdj/vcpkg closer to upstream long-term.

Cons:

- 477 commits of microsoft/vcpkg changes need to be validated against Mixxx's existing buildenvs.
- Risk of unintended breakage to existing dependencies.
- Much larger change to review and validate.
- All buildenv artifacts for all platforms must be rebuilt from scratch.

### Option C — Use an overlay port in the Mixxx source repo (temporary)

Add a `cmake/vcpkg-overlay-ports/bungee/` directory in Mixxx with the same port files, and configure `VCPKG_OVERLAY_PORTS` to pick it up during builds.

Pros:

- Completely self-contained within the Mixxx repo — no PR against mixxxdj/vcpkg needed.
- Fastest path to a working vcpkg Bungee build.
- Transparent to reviewers: port files visible in the Mixxx PR.

Cons:

- Duplicates the port files already present in microsoft/vcpkg.
- Mixxx maintainers may prefer the dependency live in mixxxdj/vcpkg rather than the Mixxx source tree.
- Once mixxxdj/vcpkg has the port, the overlay becomes dead code.

### Recommendation: Option A (cherry-pick into mixxxdj/vcpkg)

Option A adds exactly the Bungee port without disturbing anything else. It follows the pattern of how Mixxx would normally update mixxxdj/vcpkg for a new dependency. The overlay (Option C) is the fastest for unblocking this PR and can be used as a bridge if Option A takes time to merge.

**Impact if deferred:** Windows/macOS CI cannot use vcpkg Bungee until a buildenv artifact containing the port exists. Until then, those platforms fall back to the vendored `lib/bungee/` source (which is still present and functional). BNG-22 (vendor removal) is blocked on this work.

## Acceptance criteria

- The selected vcpkg path is documented with rationale.
- `find_package(unofficial-bungee CONFIG)` works from `MIXXX_VCPKG_ROOT` for the selected buildenv.
- At least the Windows and macOS release triplets expected by Mixxx are checked or explicitly deferred with rationale.
- No checked-in `lib/bungee/` source is required for Windows/macOS builds.
- Any future unresolved vcpkg decision is presented with pros/cons.
