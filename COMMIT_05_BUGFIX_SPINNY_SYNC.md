# Commit 5: Bug-Fix - Spinny/Vinyl Display Synchronization

## Status: 📋 Prepared (Ready for Implementation)

## Problem Description
When a track is pitched from 440Hz to 432Hz using the pitch lock feature:
- A "shadow" effect appears behind the playback position indicator
- The visual playback position lags behind the actual audio position
- Caused by pitch_adjust affecting playback rate but visual position not compensating

## User Impact
- **Positive**: Some users find it helpful - shows correction is active
- **Negative**: Looks like a visual bug, can be confusing

## Root Cause Analysis

### Hypothesis
The spinny/vinyl widget calculates visual position based on:
- Playback position (correct)
- Possibly NOT accounting for pitch_adjust rate modification

### Files to Investigate

#### 1. `src/widget/wspinny.h` / `src/widget/wspinny.cpp`
Main spinny widget - likely where position is calculated

#### 2. `src/widget/wvumetergl.cpp` or similar
May contain visual position logic

#### 3. `src/waveform/renderers/`
Waveform renderers might have similar sync issues

#### 4. `src/engine/enginebuffer.cpp`
Where actual playback position is maintained

### Key Controls to Check
```cpp
// These control playback rate
[ChannelN],rate_ratio      // Speed slider
[ChannelN],pitch_adjust    // Pitch adjustment (used by 432Hz feature)
[ChannelN],pitch           // Overall pitch

// Visual position
[ChannelN],playposition    // Current position 0.0-1.0
[ChannelN],visual_playposition  // Visual position (may differ)
```

## Proposed Solution

### Option A: Fix Visual Position Calculation
Ensure spinny widget accounts for pitch_adjust when calculating visual rotation:

```cpp
// In WSpinny or equivalent
double getVisualRotation() {
    double position = m_pPlayPosition->get();
    double rateRatio = m_pRateRatio->get();
    double pitchRatio = m_pPitchAdjust->get(); // ADD THIS

    // Apply pitch adjustment to visual calculation
    double effectiveRate = rateRatio * pitchRatio;
    // ... use effectiveRate for rotation
}
```

### Option B: Use Correct Position Control
Ensure spinny uses `visual_playposition` which should already be compensated:

```cpp
// Use visual_playposition instead of playposition
m_pVisualPlayPosition = make_parented<ControlProxy>(
    group, "visual_playposition", this);
```

### Option C: Intentional Feature
If this behavior is considered useful, make it a feature:
- Add option in preferences to enable/disable visual sync
- Document the "shadow" as intentional pitch indicator

## Investigation Steps

1. **Read WSpinny source**
   ```bash
   # Find spinny implementation
   find src -name "*spinny*"
   ```

2. **Trace position calculation**
   - Find where rotation angle is calculated
   - Check which ControlObjects are used

3. **Compare with waveform**
   - Does waveform have same issue?
   - If not, how does it handle pitch_adjust?

4. **Test fix**
   - Modify position calculation
   - Verify visual sync is correct
   - Verify no new issues introduced

## Testing
1. Load track with 432Hz pitch lock enabled
2. Observe spinny rotation
3. Compare visual position with waveform position
4. Verify no "shadow" or lag
5. Test with various pitch_adjust values
6. Test with keylock on/off

## Rollback Plan
If fix causes other issues, can revert to current behavior and document as "feature"
