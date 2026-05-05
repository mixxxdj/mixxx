# 04 — Packaging and CI Updates

## 4.1 Debian/Ubuntu packaging

If Bungee becomes packaged in Debian/Ubuntu:

- add the appropriate `Build-Depends` in `packaging/debian/control.in`;
- use `find_package(Bungee)` normally;
- avoid fallback downloads in official Debian packaging if Debian policy requires no network during build.

If Bungee is not packaged yet:

- ask maintainers whether official Debian packaging should build with `-DBUNGEE=OFF` until Bungee is packaged;
- or ask whether a source-embedded fallback is acceptable for Mixxx-provided packages only.

## 4.2 Flatpak

Add a module similar to existing modules:

```text
packaging/flatpak/modules/bungee.yaml
```

The module should:

- fetch a pinned archive;
- include sha256;
- build with CMake/Ninja;
- install license file;
- disable unnecessary CLI/tests if appropriate.

Then include it in `packaging/flatpak/org.mixxx.Mixxx.yaml` before the Mixxx module.

## 4.3 Windows/macOS CI

Once the vcpkg build environment includes Bungee:

- ensure CI jobs use a buildenv that contains Bungee;
- ensure `find_package(Bungee CONFIG)` resolves from vcpkg;
- verify app-local dependency handling if Bungee is built shared;
- prefer static dependency if that is consistent with the rest of the Mixxx buildenv.

## 4.4 Linux CI

Decide with maintainers whether Linux CI should:

1. install a system Bungee package;
2. use `ExternalProject_Add` fallback;
3. build with `-DBUNGEE=OFF` until packaging exists.

## 4.5 Android/iOS

Questions to answer before enabling by default:

- Does Bungee build on Android/iOS with Mixxx toolchains?
- Does Bungee need platform-specific link libraries, e.g. Android `log`?
- Is CPU cost acceptable on mobile devices?
- Should mobile builds default `BUNGEE=OFF` initially?

## 4.6 Result of this phase

Exit criteria:

- every official Mixxx build target has an explicit Bungee dependency path or an explicit `BUNGEE=OFF` decision;
- no CI job depends on an untracked local `lib/bungee` tree;
- source package/build policy is documented.
