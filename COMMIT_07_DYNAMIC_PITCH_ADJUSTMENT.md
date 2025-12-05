# Commit 7: Core - Dynamic Pitch Adjustment Based on Detected Frequency

## Status: 📋 Prepared (Ready for Implementation)

## Summary
Calculate pitch adjustment dynamically based on the detected tuning frequency,
instead of using a fixed -0.31767 semitones for all tracks.

## Current State
- Fixed adjustment: -0.31767 semitones (440Hz → 432Hz only)
- Doesn't handle tracks at 447Hz, 427Hz, etc.

## Goal
- Calculate exact pitch adjustment for any detected frequency
- Formula: `log₂(432 / detected_frequency) × 12` semitones

## Examples

| Detected Hz | Adjustment (semitones) | Adjustment (cents) |
|-------------|------------------------|-------------------|
| 427         | +0.20                  | +20 cents         |
| 430         | +0.08                  | +8 cents          |
| 432         | 0                      | 0 cents           |
| 435         | -0.12                  | -12 cents         |
| 440         | -0.32                  | -32 cents         |
| 444         | -0.47                  | -47 cents         |
| 447         | -0.58                  | -58 cents         |

## Files to Modify

### 1. `src/engine/controls/keycontrol.h`

**Update constants:**
```cpp
// Remove fixed constant
// static constexpr double k432HzPitchAdjustmentSemitones = -0.31767;

// Add calculation helper
static double calculate432HzAdjustment(int detectedFrequencyHz);
```

### 2. `src/engine/controls/keycontrol.cpp`

**Add calculation function:**
```cpp
#include <cmath>

// Target frequency for 432Hz tuning
static constexpr int kTarget432Hz = 432;

double KeyControl::calculate432HzAdjustment(int detectedFrequencyHz) {
    if (detectedFrequencyHz <= 0 || detectedFrequencyHz == kTarget432Hz) {
        return 0.0;
    }
    // log₂(432 / detected) × 12 semitones
    return std::log2(static_cast<double>(kTarget432Hz) / detectedFrequencyHz) * 12.0;
}
```

**Update apply432HzPitchAdjustment:**
```cpp
void KeyControl::apply432HzPitchAdjustment() {
    const bool is432HzPitchLockEnabled = m_p432HzPitchLock && m_p432HzPitchLock->toBool();

    if (!is432HzPitchLockEnabled) {
        // Reset pitch if it was set by us
        if (m_b432HzAdjustmentApplied) {
            m_pPitchAdjust->set(0);
            slotPitchAdjustChanged(0);
            m_b432HzAdjustmentApplied = false;
        }
        return;
    }

    // Get detected tuning frequency
    int detectedHz = m_pFileTuningHz ? static_cast<int>(m_pFileTuningHz->get()) : 440;

    if (detectedHz == kTarget432Hz) {
        // Track is already at 432Hz, no adjustment needed
        if (m_b432HzAdjustmentApplied) {
            m_pPitchAdjust->set(0);
            slotPitchAdjustChanged(0);
            m_b432HzAdjustmentApplied = false;
        }
        return;
    }

    // Calculate dynamic adjustment
    double adjustment = calculate432HzAdjustment(detectedHz);
    m_pPitchAdjust->set(adjustment);
    slotPitchAdjustChanged(adjustment);
    m_b432HzAdjustmentApplied = true;

    if constexpr (kEnableDebugOutput) {
        qDebug() << "432Hz pitch lock: detected" << detectedHz << "Hz"
                 << "applying" << adjustment << "semitones adjustment";
    }
}
```

### 3. `src/engine/controls/keycontrol.h`

**Add member variables:**
```cpp
private:
    // ... existing ...
    parented_ptr<ControlProxy> m_pFileTuningHz;  // NEW
    bool m_b432HzAdjustmentApplied = false;      // NEW
```

### 4. `src/mixer/basetrackplayer.h` / `src/mixer/basetrackplayer.cpp`

**Add tuning frequency control:**
```cpp
// basetrackplayer.h
std::unique_ptr<ControlObject> m_pFileTuningHz;

// basetrackplayer.cpp (constructor)
m_pFileTuningHz = std::make_unique<ControlObject>(
    ConfigKey(getGroup(), "file_tuning_hz"));

// basetrackplayer.cpp (slotTrackLoaded)
m_pFileTuningHz->set(m_pLoadedTrack->getTuningFrequencyHz());
```

### 5. `src/engine/controls/keycontrol.cpp` (constructor)

**Add tuning frequency proxy:**
```cpp
m_pFileTuningHz = make_parented<ControlProxy>(group, "file_tuning_hz", this);
m_pFileTuningHz->connectValueChanged(this, &KeyControl::slotFileTuningHzChanged,
        Qt::DirectConnection);
```

**Add slot:**
```cpp
void KeyControl::slotFileTuningHzChanged(double value) {
    Q_UNUSED(value);
    apply432HzPitchAdjustment();
}
```

## Edge Cases

### 1. Unknown Frequency
If tuning_frequency_hz is 0 or missing, default to 440Hz behavior

### 2. Very Low/High Frequencies
Clamp adjustment to reasonable range (e.g., ±1 semitone = ±100 cents)

```cpp
double KeyControl::calculate432HzAdjustment(int detectedFrequencyHz) {
    // ... calculation ...

    // Clamp to ±1 semitone to avoid extreme adjustments
    return std::clamp(adjustment, -1.0, 1.0);
}
```

### 3. Track Without Analysis
Unanalyzed tracks: assume 440Hz, apply standard -0.32 adjustment

## Testing

### Unit Tests
```cpp
TEST(KeyControlTest, Calculate432HzAdjustment) {
    EXPECT_NEAR(KeyControl::calculate432HzAdjustment(440), -0.318, 0.01);
    EXPECT_NEAR(KeyControl::calculate432HzAdjustment(447), -0.58, 0.01);
    EXPECT_NEAR(KeyControl::calculate432HzAdjustment(427), 0.20, 0.01);
    EXPECT_EQ(KeyControl::calculate432HzAdjustment(432), 0.0);
}
```

### Integration Tests
1. Load 447Hz track with 432Hz lock → verify -0.58 semitone adjustment
2. Load 427Hz track with 432Hz lock → verify +0.20 semitone adjustment
3. Load 432Hz track with 432Hz lock → verify no adjustment
4. Toggle lock on/off → verify adjustment applies/resets correctly

## Dependencies
- Requires Commit 6 (Dynamic Frequency Detection)
- Requires `file_tuning_hz` control from BaseTrackPlayer
