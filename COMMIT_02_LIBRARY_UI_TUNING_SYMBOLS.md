# Commit 2: Library UI - Tuning Symbols

## Status: 📋 Prepared (Ready for Implementation)

## Summary
Add visual indicators in the library key column for different tuning frequencies:
- ↓ (arrow down): Tracks tuned below 432Hz
- ✧ (sparkle): Tracks at exactly 432Hz (existing)
- ↑ (arrow up): Tracks tuned above 432Hz (including standard 440Hz)

## Files to Modify

### 1. `src/library/tabledelegates/keydelegate.cpp`

**Current State:**
- Shows ✧ symbol only for `is432Hz == true`

**Required Changes:**
```cpp
// Add new symbols
constexpr const char* kAbove432HzSymbol = "\xE2\x86\x91"; // ↑ (U+2191)
constexpr const char* kBelow432HzSymbol = "\xE2\x86\x93"; // ↓ (U+2193)
constexpr const char* k432HzSymbol = "\xE2\x9C\xA7";      // ✧ (existing)

// In paintItem():
// Need to get tuning frequency from model, not just is432Hz boolean
// Then decide which symbol to show:
// - frequency < 432Hz: show ↓
// - frequency == 432Hz: show ✧
// - frequency > 432Hz: show ↑
```

### 2. `src/library/trackmodel.h`

**Add new role:**
```cpp
static constexpr int kTuningFrequencyRole = Qt::UserRole + 4;
```

### 3. `src/library/basetracktablemodel.cpp`

**Handle new role in roleData():**
```cpp
if (role == kTuningFrequencyRole) {
    // Return actual tuning frequency (e.g., 432, 440, 447)
    TrackPointer pTrack = getTrack(index);
    if (pTrack) {
        return pTrack->getTuningFrequency(); // Need to add this method
    }
    return 440; // Default
}
```

### 4. `src/track/track.h` / `src/track/track.cpp`

**Add method:**
```cpp
int Track::getTuningFrequency() const;
```

### 5. `src/track/keys.h` / `src/track/keys.cpp`

**Add method:**
```cpp
int Keys::getTuningFrequency() const;
```

### 6. `src/proto/keys.proto`

**Extend KeyMap message:**
```protobuf
// Detected tuning frequency in Hz (default 440)
optional int32 tuning_frequency_hz = 6 [default = 440];
```

## Dependencies
- Requires Commit 6 (Dynamic frequency detection) for full functionality
- Can be implemented with placeholder logic initially:
  - If is432Hz: frequency = 432
  - Else: frequency = 440

## Symbol Colors
- ↓ (below 432Hz): Blue (#4169E1) - needs pitch up to reach 432Hz
- ✧ (at 432Hz): Gold (#DAA520) - existing, perfect tuning
- ↑ (above 432Hz): Orange (#FF8C00) - needs pitch down to reach 432Hz

## Testing
1. Load track with is432Hz = true → should show ✧
2. Load track with is432Hz = false → should show ↑
3. (After Commit 6) Load track with frequency < 432Hz → should show ↓
