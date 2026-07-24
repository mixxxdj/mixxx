# Bungee Keylock Engine Integration

This document describes how Mixxx integrates the [Bungee][bungee] real-time
audio time-stretching library as an optional keylock (pitch-independent tempo)
engine.

[bungee]: https://github.com/kupix/bungee

---

## Architecture overview

```text
EngineBuffer
  └── m_pScaleBungee  (EngineBufferScaleBungee)
        ├── ReadAheadManager   — pulls decoded audio frames
        └── Bungee::Stretcher  — performs grain-based time-stretch / pitch-shift
```

`EngineBufferScaleBungee` implements the `EngineBufferScale` interface.
`EngineBuffer` selects it as `m_pScaleKeylock` when the user picks
*"Bungee (high quality)"* from **Preferences → Sound → Keylock engine**.

---

## Rate / position semantics

| Field | Meaning |
| --- | --- |
| `m_dBaseRate` | Input-to-output sample-rate ratio. Almost always `1.0` because Bungee's `resampleMode_autoOut` handles SR conversion internally. |
| `m_dTempoRatio` | Absolute (unsigned) tempo ratio; `1.0` = original speed. Values below `MIN_SEEK_SPEED` are clamped to `0.0` (stopped). |
| `m_bBackwards` | `true` when the caller requested a negative tempo ratio (reverse playback). |
| `m_effectiveRate` | `m_dBaseRate × m_dTempoRatio`; always `≥ 0`. |
| `m_request.speed` | Signed speed sent to Bungee each grain: `±m_dTempoRatio`. |

`scaleBuffer()` returns `readFramesProcessed = m_effectiveRate × framesProduced`.
`EngineBuffer` uses this value to advance its playback-position cursor; it must
account for the direction sign externally.

---

## The InputChunk sliding-window contract

This is the most important design constraint in the integration.

Bungee is a **grain-based** stretcher.  Each call to `specifyGrain()` returns
an `InputChunk {.begin, .end}` that describes the range of *input frame indices*
the next grain requires.  **Consecutive grains overlap** — the same input
frames appear in multiple chunks — so a naïve "read exactly what the current
grain needs, discard everything else" approach violates the API contract and
produces garbled audio.

### What `EngineBufferScaleBungee` does instead

It maintains a **sliding planar input window**:

```text
m_contiguousChannelBuffer
  ch0: [ frame₀ | frame₁ | … | frame_N ]
  ch1: [ frame₀ | frame₁ | … | frame_N ]
       ^                                ^
  m_bufferedInputBeginFrame    m_bufferedInputEndFrame
```

Before each `analyseGrain()` call (`ensureInputForCurrentChunk`):

1. **Discard** frames before `inputChunk.begin` (`discardBufferedInputBefore`).
   Uses `memmove` — acceptable because Bungee grains are large relative to the
   typical advance per grain.
2. **Extend** the window forward until it covers `inputChunk.end`, by calling
   `ReadAheadManager::getNextSamples()` into the interleaved read buffer and
   deinterleaving into the planar window (`appendInputFrames`).
3. Compute **`muteHead`** and **`muteTail`** from the difference between the
   requested range and what the window actually covers (e.g., at the start of
   a track or after a seek the window may not cover the full chunk):

   ```text
   muteHead = inputChunk.begin - bufferedInputBeginFrame  (frames before window)
   muteTail = inputChunk.end   - bufferedInputEndFrame    (frames after window)
   ```

4. Call `analyseGrain(channelBufferPtrs[0] + dataOffset, channelStride,
   muteHead, muteTail)`.  Bungee zero-pads the muted regions internally.

This mirrors the pattern in Bungee's upstream `bungee/Stream.h` helper
class (lines 89–95 in the upstream source used by the current Bungee pin).

### The buffer-window invariant

On every grain boundary, `EngineBufferScaleBungee` must satisfy:

```text
m_bufferedInputBeginFrame <= currentInputChunk.begin
dataOffset = currentInputChunk.begin - m_bufferedInputBeginFrame
dataOffset + grainSize  <= m_channelStride
```

where `grainSize = currentInputChunk.end - currentInputChunk.begin` and
`m_channelStride` is the per-channel capacity of
`m_contiguousChannelBuffer`.  Any violation lets Bungee's Eigen map in
`analyseGrain()` read past the end of the buffer for one or more channels
— that is the heap-corruption regime the buffer-window fixes close.

`discardBufferedInputBefore(framePosition)` preserves the invariant in
two regimes:

- **Partial discard** — `framePosition` lies inside the buffered window.
  `memmove` shifts the unread tail left, `m_bufferedInputBeginFrame` is
  advanced by the number of discarded frames, and
  `m_bufferedInputEndFrame` is unchanged.
- **Full discard** — `framePosition >= m_bufferedInputEndFrame`.  This is
  the **high-speed grain-outrun** regime: at very high playback rates
  Bungee's grain hops jump past everything currently in the read buffer,
  so the next grain's `inputChunk.begin` is far beyond
  `m_bufferedInputEndFrame`.  Both pointers must jump all the way to
  `framePosition`; any lower value (notably the OLD `m_bufferedInputEndFrame`,
  which is what the earlier code wrote) leaves a gap that violates the
  `dataOffset` invariant on the very next grain.

`processGrain()` additionally enforces the invariant defensively: if it
ever computes `dataOffset + grainSize > m_channelStride`, it sets
`m_bResetNeeded = true` and returns 0 without calling `analyseGrain()`.
This runtime guard is belt-and-braces — the buffer-window state machine
in `discardBufferedInputBefore()` and `appendInputFrames()` must not rely
on it for correctness.  The companion regression tests in
`src/test/enginebufferscalebungeetest.cpp` (the
`EngineBufferScaleBungeeBufferWindowTest` fixture) pin both the partial-
and full-discard branches against the invariant so the bug class cannot
return silently.

### Why the input buffer is zero-initialised in `onSignalChanged()`

On the very first grain after a reset (`request.position = 0`), Bungee's
`InputChunk` covers `[−halfFrames, +halfFrames)`.  The scaler only supplies
data for `[0, halfFrames)`; the lower half is muted via `muteHead`.  However
the Eigen map that `analyseGrain()` builds spans the *entire* grain, so the
uninitialised lower half could feed garbage floats (including `NaN`) into
the FFT.  `onSignalChanged()` therefore zero-fills the whole
`m_contiguousChannelBuffer` after (re)allocation.

### Why `muteHead = 0` always is wrong

The original Mixxx integration always passed `muteHead = 0`.  When a grain's
requested range began *before* the data the scaler had buffered (e.g., at
track start or after a reset), Bungee read uninitialised memory or stale
data from a previous track — the root cause of the garbled / 8×-speed audio.

---

## Output interleaving

Bungee writes output in **planar** format: `outputChunk.data[frame + ch * channelStride]`.
`copyOutputFrames()` re-interleaves this for Mixxx's stereo engine buffer.
For the common stereo case it delegates to `SampleUtil::interleaveBuffer()`
(SIMD-optimised); for N-channel audio it falls back to a scalar loop.

---

## Bungee's reset / flush protocol

`Bungee::Request::reset = true` signals Bungee to reset its internal state
(e.g., after a seek or direction change).  `EngineBufferScaleBungee::clear()`
sets `m_bResetNeeded = true`; the next `processGrain()` sends a reset request.

On EOF or `ReadAheadManager` exhaustion, `processGrain()` returns `0` twice in
a row.  `scaleBuffer()` detects this (`lastProcessFailed = true`) and attempts
a graceful flush via `Bungee::Stretcher::isFlushed()` / `synthesiseGrain()`
before zero-padding the remainder of the output buffer.

---

## Memory layout and performance

All buffers are **pre-allocated** in `onSignalChanged()`.  The steady-state
`scaleBuffer()` path contains no heap allocations.

| Buffer | Purpose | Size |
| --- | --- | --- |
| `m_contiguousChannelBuffer` | Planar input window (all channels contiguous) | `(maxGrainFrames + headroom) × channelCount` floats |
| `m_interleavedReadBuffer` | Temporary for `ReadAheadManager` reads | `kMaxGrainFrames × channelCount` floats |

The `memmove` in `discardBufferedInputBefore()` moves at most ~96 KB per grain
(stereo, ~12 000 frames).  This is acceptable given Bungee's large grain sizes.
A ring-buffer would eliminate the move but is not currently warranted.

---

## Bungee dependency source-fetch patches

When no packaged Bungee provider is found and `BUNGEE_FETCH_FALLBACK=ON`,
Mixxx builds the pinned upstream Bungee release with `ExternalProject_Add`.
The source-fetch path applies Mixxx-owned patches from `cmake/patches/bungee/`
so Bungee uses normal Eigen3 and pffft dependency packages instead of bundled
submodules. These patches mirror the official vcpkg Bungee port where possible.
Patching happens in the build tree and must not dirty the Mixxx source checkout.

The source-fetch path requires a `patch` executable at configure time. Install
GNU patch (for example `apt install patch`, `brew install gpatch`, or a Windows
MSVC-compatible patch executable) or configure with `-DBUNGEE=OFF` / use a
packaged Bungee provider.

## What not to change casually

| Boundary | Reason |
| --- | --- |
| Bungee dependency pins and patches | Keep CMake/vcpkg/Flatpak pins in sync and apply Mixxx-owned patches only in dependency build trees, not to checked-in upstream source. |
| `muteHead` / `muteTail` computation | Any incorrect value here violates the `analyseGrain()` contract and causes audio corruption. |
| `appendInputFrames()` call sites | All `ReadAheadManager` reads must be consumed by Bungee. Discarding reads silently advances the reader's position and causes playback drift. |
| `discardBufferedInputBefore()` full-discard branch | When `framePosition >= m_bufferedInputEndFrame`, both buffer-window pointers must be set to `framePosition`. Any other value reintroduces the high-speed grain-outrun heap-overflow class. Pinned by `EngineBufferScaleBungeeBufferWindowTest`. |
| Zero-initialisation of `m_contiguousChannelBuffer` in `onSignalChanged()` | Prevents Bungee's Eigen map from reading uninitialised floats during the muted half of the very first post-reset grain. |
| `scaleBuffer()` return value | `EngineBuffer` uses this to advance its frame cursor. The sign and magnitude must match `effectiveRate × framesProduced`. |

---

## Enabling Bungee

Bungee is enabled by default (`option(BUNGEE … ON)` in `CMakeLists.txt`).
To build without it: `cmake -DBUNGEE=OFF …`.

The keylock engine is selected at runtime from
**Preferences → Sound → Keylock engine → Bungee (high quality)**.
When built with `-DBUNGEE=ON`, Bungee is the compile-time default engine
(`defaultKeylockEngine()` in `enginebuffer.h`).

---

## Test surface

| File | Purpose |
| --- | --- |
| `src/test/enginebufferscalebungeetest.cpp` (`EngineBufferScaleBungeeTest`) | Per-feature unit tests against a mock `ReadAheadManager`: speed sweeps, pitch shifting, reverse playback, parameter changes, buffer reuse. |
| `src/test/enginebufferscalebungeetest.cpp` (`EngineBufferScaleBungeeBufferWindowTest`) | Regression tests for the buffer-window invariant (`discardBufferedInputBefore` post-conditions across partial / full / empty discard plus a high-speed grain-outrun stress test). |
| `src/test/enginebufferbungeetest.cpp` (`EngineBufferBungeeTest`) | End-to-end integration tests through the real `EngineBuffer`: keylock toggling, engine switching, NaN/Inf detection in the mixer output. |

The `.github/workflows/bungee-asan.yml` workflow rebuilds the test binary
with `-DSANITIZE_ADDRESS=ON` on Linux/clang on every change to the Bungee
integration files and runs `*Bungee*` under AddressSanitizer.  This is
the regression net for the heap-overflow class — if a future change
ever re-introduces an out-of-bounds read of `m_contiguousChannelBuffer`,
ASan halts the run with `halt_on_error=1`.
