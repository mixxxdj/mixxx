# Maintainer Questions — Bungee Dependency Integration

## Current working answers / assumptions

These are the current starting assumptions. Revisit them only if Mixxx maintainers request a different direction.

- `BUNGEE` should default to `ON` for this PR.
- It is acceptable to increase the recommended CMake version for Bungee-enabled builds that use the most recent Bungee release.
- If the installed CMake is older than the newest Bungee requires, it is acceptable to investigate and use an older validated Bungee release that still supports Mixxx's current CMake minimum.
- Future requests for decisions should include pros/cons and a recommendation.

## Questions for Mixxx maintainers

1. **Preferred dependency strategy**
   For a new optional audio engine dependency that is not widely distro-packaged yet, do you prefer:
   - `find_package` only and `BUNGEE=OFF` when unavailable;
   - `find_package` plus `ExternalProject_Add` fallback;
   - vcpkg/Flatpak packaging first, then enable in Mixxx;
   - another approach?

2. **Default value of `BUNGEE`**
   Should `BUNGEE` default to `ON` immediately, or default to `OFF` until Bungee is available in the official build environments and packaging flows?

3. **ExternalProject fallback acceptability**
   Would an `ExternalProject_Add` fallback modeled after `libdjinterop`/`libkeyfinder` be acceptable for Bungee, provided it uses pinned archive URLs and hashes?

4. **CMake minimum compatibility**
   Mixxx currently requires CMake `3.21`, but current Bungee upstream CMake appears to require newer CMake. Should we patch/lower Bungee's CMake requirement in the dependency port/fallback instead of raising Mixxx's requirement?

5. **System package vs bundled build for Linux**
   For Debian/Ubuntu/Fedora packaging, should Bungee support be disabled until Bungee is packaged independently, or is a source fallback acceptable for Mixxx-provided packages?

6. **vcpkg source/provenance**
   Microsoft vcpkg PR #50120 has already added Bungee `2.4.15` as an upstream port exposing `unofficial-bungee`. Should Mixxx:
   - update its vcpkg fork/buildenv baseline to include that merge;
   - cherry-pick/backport the port into `mixxxdj/vcpkg`;
   - temporarily copy it into `overlay/ports/bungee`;
   - or wait for the next normal vcpkg buildenv update?

7. **Static vs shared Bungee**
   For Mixxx release builds, should Bungee be linked statically like the current integration, or should it follow the platform's normal shared/static dependency policy?

8. **Eigen/PFFFT handling**
   Is it acceptable to require `Eigen3` and `pffft` as separate dependencies, or should the Bungee package own any fallback for PFFFT internally?

9. **Patch policy**
   If Bungee needs temporary patches for MSVC or install/export fixes, should those patches live in:
   - upstream Bungee PRs only;
   - the vcpkg port;
   - Mixxx's `ExternalProject_Add` fallback;
   - Mixxx tree under `cmake/patches` or similar?

10. **Platform enablement**
    Which platforms should enable Bungee initially?
    - Linux desktop
    - Windows x64/arm64
    - macOS x64/arm64
    - Android
    - iOS

11. **Review sequencing**
    Would maintainers prefer one integrated PR, or a sequence of PRs: Bungee packaging, Mixxx CMake dependency path, vendor removal, packaging/CI enablement?

12. **License/compliance expectations**
    What license files/notices must be added to Mixxx packaging when Bungee is included via vcpkg/Flatpak/system package?

13. **Find module naming**
    The vcpkg port exposes `unofficial::bungee::bungee`; should Mixxx wrap that as `Bungee::Bungee` for consistency with system/package discovery, or link vcpkg's unofficial target directly?

14. **Fallback failure mode**
    If `BUNGEE=ON` and Bungee cannot be found/fetched, should configure fail hard, or should Mixxx warn and continue with Bungee disabled?

15. **Performance acceptance criteria**
    Are there specific benchmark/audio-quality thresholds maintainers want before enabling Bungee by default?

16. **Branch and CI discipline**
    Is the expected integration branch named `feature/bungee`, and do maintainers require proof that every commit on that branch has 100% passing CI, or is branch-head CI sufficient?

17. **Cherry-pick/backport policy**
    For Bungee-related commits currently made on other local/feature branches, should they be cherry-picked into `feature/bungee` preserving commit boundaries, or squashed/reworked into a cleaner history?

## Questions for Bungee maintainers

1. Can Bungee's CMake minimum version be lowered enough for Mixxx's supported build environments, or should downstreams rely on the vcpkg patch from PR #50120?
2. Can Bungee install a proper CMake package config and exported target?
3. Can the library target be built without the command-line executable and `cxxopts`? The vcpkg PR removed the unused standalone executable for the port; should that be supported upstream as an option?
4. Can Bungee use external `Eigen3::Eigen` and external `pffft` instead of requiring submodules? The vcpkg port already patches this path.
5. Can the MSVC compatibility patches represented by vcpkg's `assert-win32-compat.patch` and `resample-msvc-noinline.patch` be accepted upstream?
6. What version/tag should Mixxx consume for a stable integration?
7. What is the intended ABI/API stability policy for Bungee?
8. Should downstreams link Bungee statically or shared?
9. What license files should downstream packages install?
10. Are there platform-specific requirements for Android/iOS/Windows/macOS that Mixxx should know about?
