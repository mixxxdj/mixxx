# In-tree vcpkg overlay port for Bungee

## Status

**Temporary.** This overlay exists only because `mixxxdj/vcpkg` does not yet
contain the Bungee port. It will be removed once `mixxxdj/vcpkg` is updated
(see [BNG-20](../../../docs/tasks/done/BNG-20.md)).

## Provenance

Verbatim copy of `ports/bungee/` from `microsoft/vcpkg:master`, originally
merged via [microsoft/vcpkg PR #50120](https://github.com/microsoft/vcpkg/pull/50120)
(`[bungee] Added new port`, merged 2026-03-19 at commit `ae92331`).

Files copied:

- `portfile.cmake`
- `vcpkg.json`
- `unofficial-bungee-config.cmake`
- `usage`
- `cmake-use-vcpkg-deps-and-install-layout.patch`
- `pffft-include-path.patch`
- `assert-win32-compat.patch`
- `resample-msvc-noinline.patch`

The port produces:

```cmake
find_package(unofficial-bungee CONFIG REQUIRED)
target_link_libraries(main PRIVATE unofficial::bungee::bungee)
```

Mixxx's top-level `CMakeLists.txt` wraps `unofficial::bungee::bungee` in a
`Bungee::Bungee` interface target so downstream code stays
provider-agnostic (see the BUNGEE block in `CMakeLists.txt`).

## How it is consumed

The directory containing this port is appended to `VCPKG_OVERLAY_PORTS` by
Mixxx's top-level `CMakeLists.txt` whenever vcpkg is in use. It is appended
*after* `MIXXX_VCPKG_ROOT/overlay/ports`, so once `mixxxdj/vcpkg` ships its
own Bungee port that one will take precedence and this overlay becomes inert.

## Removal plan (BNG-20 Option A → final)

When `mixxxdj/vcpkg` is updated (rebased to a microsoft/vcpkg baseline that
includes `ae92331`, or the port is cherry-picked):

1. Verify the buildenv produced from the updated `mixxxdj/vcpkg` exposes
   `unofficial-bungee` for all required Mixxx triplets.
2. Delete this entire `cmake/vcpkg-overlay-ports/` directory.
3. Remove the overlay-append line from the top-level `CMakeLists.txt`.
4. Update [docs/plans/bungee-dependency-integration/02-vcpkg-buildenv.md](../../../docs/plans/bungee-dependency-integration/02-vcpkg-buildenv.md)
   to record that the overlay phase has ended.

## License

The port files are licensed under MIT (vcpkg's license). Bungee itself is
licensed under MPL-2.0; that license does not apply to the port files in
this directory.
