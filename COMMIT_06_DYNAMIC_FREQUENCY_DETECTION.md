# Commit 6: Core - Dynamic Frequency Detection (427Hz-447Hz Range)

## Status: 📋 Prepared (Ready for Implementation)

## Summary
Extend the Queen Mary Key Detector to detect the actual tuning frequency of tracks,
not just binary 440Hz vs 432Hz detection.

## Current State
- Binary detection: is_432hz = true/false
- Only checks 440Hz and 432Hz
- Fixed pitch adjustment: -0.31767 semitones

## Goal
- Detect actual tuning frequency in range ~427Hz - 450Hz
- Store exact frequency in track metadata
- Enable precise pitch adjustment (Commit 7)

## Files to Modify

### 1. `src/proto/keys.proto`

**Add new field:**
```protobuf
message KeyMap {
  // ... existing fields ...

  // Detected tuning frequency in Hz (default 440)
  // Range: typically 427-450 Hz
  optional int32 tuning_frequency_hz = 6 [default = 440];
}
```

### 2. `src/track/keys.h` / `src/track/keys.cpp`

**Add getter:**
```cpp
// keys.h
int getTuningFrequencyHz() const;

// keys.cpp
int Keys::getTuningFrequencyHz() const {
    return m_keyMap.tuning_frequency_hz();
}
```

### 3. `src/analyzer/plugins/analyzerqueenmarykey.h`

**Update interface:**
```cpp
class AnalyzerQueenMaryKey : public AnalyzerKeyPlugin {
  public:
    // ... existing ...
    int getTuningFrequencyHz() const { return m_detectedTuningHz; }

  private:
    int m_detectedTuningHz = 440;
};
```

### 4. `src/analyzer/plugins/analyzerqueenmarykey.cpp`

**Implement multi-frequency detection:**
```cpp
// Detection frequencies to test
static const std::vector<int> kTuningFrequencies = {
    427, 430, 432, 435, 438, 440, 442, 444, 447, 450
};

bool AnalyzerQueenMaryKey::finalize() {
    // ... existing code ...

    if (m_bDetect432Hz) {
        // Test multiple tuning frequencies
        int bestFrequency = 440;
        int lowestKeyChanges = INT_MAX;

        for (int freq : kTuningFrequencies) {
            // Create analyzer with this tuning frequency
            auto keyMode = std::make_unique<GetKeyMode>(
                m_iSampleRate,
                freq,  // tuning frequency
                /* ... other params ... */);

            // Analyze and count key changes
            int keyChanges = analyzeWithFrequency(keyMode.get());

            if (keyChanges < lowestKeyChanges) {
                lowestKeyChanges = keyChanges;
                bestFrequency = freq;
            }
        }

        m_detectedTuningHz = bestFrequency;
        m_bIs432Hz = (bestFrequency == 432);
    }

    return true;
}
```

### 5. `src/analyzer/plugins/analyzerplugin.h`

**Extend interface:**
```cpp
class AnalyzerKeyPlugin {
  public:
    // ... existing ...
    virtual int getTuningFrequencyHz() const { return 440; }
};
```

### 6. `src/analyzer/analyzerkey.cpp`

**Pass frequency to KeyFactory:**
```cpp
void AnalyzerKey::storeResults(TrackPointer pTrack) {
    // ... existing code ...

    int tuningHz = m_pPlugin->getTuningFrequencyHz();
    bool is432Hz = m_pPlugin->is432Hz();

    Keys keys = KeyFactory::makePreferredKeys(
        key_changes,
        extraVersionInfo,
        m_sampleRate,
        m_totalFrames,
        is432Hz,
        tuningHz);  // NEW PARAMETER

    pTrack->setKeys(keys);
}
```

### 7. `src/track/keyfactory.h` / `src/track/keyfactory.cpp`

**Extend makePreferredKeys:**
```cpp
// keyfactory.h
static Keys makePreferredKeys(
    const KeyChangeList& key_changes,
    const QHash<QString, QString>& extraVersionInfo,
    int iSampleRate,
    int iTotalSamples,
    bool is432Hz = false,
    int tuningFrequencyHz = 440);  // NEW

// keyfactory.cpp
Keys KeyFactory::makePreferredKeys(..., int tuningFrequencyHz) {
    // ... existing code ...
    key_map.set_is_432hz(is432Hz);
    key_map.set_tuning_frequency_hz(tuningFrequencyHz);  // NEW
    // ...
}
```

### 8. `src/track/track.h` / `src/track/track.cpp`

**Add method:**
```cpp
// track.h
int getTuningFrequencyHz() const;

// track.cpp
int Track::getTuningFrequencyHz() const {
    const auto locked = lockMutex(&m_qMutex);
    return m_record.getKeys().getTuningFrequencyHz();
}
```

## Performance Considerations

### Optimization Options

**Option A: Test all frequencies (slow but accurate)**
- Test 10 frequencies
- ~10x slower analysis
- Best accuracy

**Option B: Binary search (faster)**
```cpp
// Start with 432 vs 440
// If 432 better, test 427-435
// If 440 better, test 438-447
// Narrow down with binary search
```

**Option C: Coarse then fine**
```cpp
// First pass: test 430, 440, 450
// Find closest
// Second pass: fine-tune ±5Hz around best
```

### Recommended: Option C
Good balance of speed and accuracy

## Testing
1. Track at 432Hz → should detect 432
2. Track at 440Hz → should detect 440
3. Track at 447Hz → should detect 447 (or closest)
4. Track at 427Hz → should detect 427 (or closest)
5. Performance: analysis time should not increase more than 2-3x

## Backward Compatibility
- Default tuning_frequency_hz = 440
- Existing tracks with only is_432hz will still work
- is_432hz remains for quick boolean check
