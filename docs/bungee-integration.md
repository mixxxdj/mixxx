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
|---|---|
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

This mirrors the pattern in `lib/bungee/bungee/Stream.h` (Bungee's own
`Stream` helper class, lines 89–95).

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
|---|---|---|
| `m_contiguousChannelBuffer` | Planar input window (all channels contiguous) | `(maxGrainFrames + headroom) × channelCount` floats |
| `m_interleavedReadBuffer` | Temporary for `ReadAheadManager` reads | `kMaxGrainFrames × channelCount` floats |

The `memmove` in `discardBufferedInputBefore()` moves at most ~96 KB per grain
(stereo, ~12 000 frames).  This is acceptable given Bungee's large grain sizes.
A ring-buffer would eliminate the move but is not currently warranted.

---

## The Windows / MSVC compatibility patch

`lib/bungee/0001-MSVC-compatibility.patch` removes `<unistd.h>` (POSIX-only)
and adds `lib/bungee/src/Platform.h` (MSVC workarounds).  It is applied by
CMake only when `MSVC` is defined (`if(MSVC)` guard in `CMakeLists.txt`).

**Do not apply this patch on Linux or macOS** — it is unnecessary and would
dirty the `lib/bungee` working tree.

---

## What not to change casually

| Boundary | Reason |
|---|---|
| `lib/bungee/` source files | Treat as a maintained vendor library. The only Mixxx-owned modification is the Windows patch above. |
| `muteHead` / `muteTail` computation | Any incorrect value here violates the `analyseGrain()` contract and causes audio corruption. |
| `appendInputFrames()` call sites | All `ReadAheadManager` reads must be consumed by Bungee. Discarding reads silently advances the reader's position and causes playback drift. |
| `scaleBuffer()` return value | `EngineBuffer` uses this to advance its frame cursor. The sign and magnitude must match `effectiveRate × framesProduced`. |

---

## Enabling Bungee

Bungee is enabled by default (`option(BUNGEE … ON)` in `CMakeLists.txt`).
To build without it: `cmake -DBUNGEE=OFF …`.

The keylock engine is selected at runtime from
**Preferences → Sound → Keylock engine → Bungee (high quality)**.
When built with `-DBUNGEE=ON`, Bungee is the compile-time default engine
(`defaultKeylockEngine()` in `enginebuffer.h`).
