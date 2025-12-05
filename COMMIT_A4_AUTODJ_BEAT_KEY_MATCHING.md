# Commit A4: Auto DJ - Beat and Key Matching Prerequisites

## Status: 📋 Prepared (Ready for Implementation)

## Summary
Ensure intelligent crossfading only happens when tracks are properly synced:
- Beat/tempo alignment
- Key/harmonic compatibility

If conditions aren't met, fall back to traditional crossfade.

## Prerequisites for Intelligent Crossfade

### 1. Beat Matching
- Both tracks must have valid BPM
- Tempo should be within sync range (adjustable via rate slider)
- Beats must be aligned at crossfade point

### 2. Key Matching (Optional but Recommended)
- Keys should be harmonically compatible
- Use Camelot wheel / Open Key notation rules

## Files to Create/Modify

### 1. Create TransitionValidator Class

**`src/library/autodj/transitionvalidator.h`**
```cpp
#pragma once

#include "track/track_decl.h"

class TransitionValidator {
  public:
    struct ValidationResult {
        bool beatMatchPossible;
        bool keyCompatible;
        bool canUseIntelligentCrossfade;
        QString reason;  // Why it failed, if applicable
    };

    static ValidationResult validate(
        TrackPointer outgoingTrack,
        TrackPointer incomingTrack,
        double maxBpmDifference = 8.0);  // Default: ±8% range

    // Individual checks
    static bool checkBeatMatch(
        TrackPointer track1,
        TrackPointer track2,
        double maxBpmDifference);

    static bool checkKeyCompatibility(
        TrackPointer track1,
        TrackPointer track2);

  private:
    static bool areKeysCompatible(
        mixxx::track::io::key::ChromaticKey key1,
        mixxx::track::io::key::ChromaticKey key2);
};
```

### 2. Implementation

**`src/library/autodj/transitionvalidator.cpp`**
```cpp
#include "library/autodj/transitionvalidator.h"
#include "track/track.h"
#include "track/keyutils.h"

TransitionValidator::ValidationResult TransitionValidator::validate(
        TrackPointer outgoingTrack,
        TrackPointer incomingTrack,
        double maxBpmDifference) {

    ValidationResult result;
    result.beatMatchPossible = true;
    result.keyCompatible = true;
    result.canUseIntelligentCrossfade = true;

    // Check beat matching
    if (!checkBeatMatch(outgoingTrack, incomingTrack, maxBpmDifference)) {
        result.beatMatchPossible = false;
        result.canUseIntelligentCrossfade = false;
        result.reason = "BPM difference too large for sync";
    }

    // Check key compatibility (warning only, doesn't block)
    if (!checkKeyCompatibility(outgoingTrack, incomingTrack)) {
        result.keyCompatible = false;
        // Don't block intelligent crossfade, just warn
        result.reason += result.reason.isEmpty() ? "" : "; ";
        result.reason += "Keys not harmonically compatible";
    }

    return result;
}

bool TransitionValidator::checkBeatMatch(
        TrackPointer track1,
        TrackPointer track2,
        double maxBpmDifference) {

    double bpm1 = track1->getBpm();
    double bpm2 = track2->getBpm();

    // Both must have valid BPM
    if (bpm1 <= 0 || bpm2 <= 0) {
        return false;
    }

    // Calculate percentage difference
    double ratio = bpm2 / bpm1;

    // Allow for half-time / double-time (ratio near 0.5, 1.0, or 2.0)
    double normalizedRatio = ratio;
    if (ratio < 0.75) {
        normalizedRatio = ratio * 2.0;  // Half-time
    } else if (ratio > 1.5) {
        normalizedRatio = ratio / 2.0;  // Double-time
    }

    // Check if within range
    double percentDiff = std::abs(normalizedRatio - 1.0) * 100.0;
    return percentDiff <= maxBpmDifference;
}

bool TransitionValidator::checkKeyCompatibility(
        TrackPointer track1,
        TrackPointer track2) {

    auto key1 = KeyUtils::keyFromNumericValue(track1->getKey());
    auto key2 = KeyUtils::keyFromNumericValue(track2->getKey());

    // If either key is invalid/unknown, allow transition
    if (key1 == mixxx::track::io::key::INVALID ||
        key2 == mixxx::track::io::key::INVALID) {
        return true;
    }

    return areKeysCompatible(key1, key2);
}

bool TransitionValidator::areKeysCompatible(
        mixxx::track::io::key::ChromaticKey key1,
        mixxx::track::io::key::ChromaticKey key2) {

    // Use KeyUtils for harmonic compatibility check
    // Compatible keys: same key, relative major/minor, ±1 on Camelot wheel

    if (key1 == key2) {
        return true;  // Same key
    }

    // Check if harmonically compatible (within 1 step on Camelot wheel)
    int steps = KeyUtils::shortestStepsToCompatibleKey(key1, key2);
    return std::abs(steps) <= 1;
}
```

## Camelot Wheel Reference

```
        MINOR              MAJOR
         (A)                (B)

    5A = F minor      5B = Ab major
    6A = C minor      6B = Eb major
    7A = G minor      7B = Bb major
    8A = D minor      8B = F major
    9A = A minor      9B = C major
   10A = E minor     10B = G major
   11A = B minor     11B = D major
   12A = F# minor    12B = A major
    1A = Db minor     1B = E major
    2A = Ab minor     2B = B major
    3A = Eb minor     3B = F# major
    4A = Bb minor     4B = Db major

Compatible moves:
- Same number (8A ↔ 8B) = Relative major/minor
- ±1 number, same letter (8A ↔ 7A, 9A) = Energy shift
- Same number + letter = Perfect match
```

## Beat Alignment at Crossfade Point

### Quantize to Beat Grid

```cpp
double quantizeToBeat(double position, TrackPointer track) {
    const auto& beats = track->getBeats();
    if (!beats) {
        return position;
    }

    // Find nearest beat to position
    mixxx::audio::FramePos frame =
        mixxx::audio::FramePos(position * track->getDuration() *
                               track->getSampleRate());

    auto nearestBeat = beats->findClosestBeat(frame);
    if (nearestBeat.isValid()) {
        return nearestBeat.value() /
               (track->getDuration() * track->getSampleRate());
    }

    return position;
}
```

### Align Incoming Track

```cpp
void alignIncomingTrack(
        TrackPointer outgoing,
        TrackPointer incoming,
        double crossfadeStart) {

    // Get beat positions
    const auto& outgoingBeats = outgoing->getBeats();
    const auto& incomingBeats = incoming->getBeats();

    if (!outgoingBeats || !incomingBeats) {
        return;  // Can't align without beat grids
    }

    // Find phase difference and adjust incoming start position
    // ... (complex beat alignment logic)
}
```

---

## ⚠️ WICHTIG: Tatsächliche Beat-Synchronisation

**Beim Übergang muss echte Beat-Synchronisation stattfinden!**

Es reicht NICHT aus, nur zu prüfen ob BPMs kompatibel sind. Der Auto DJ muss:

1. **BPM anpassen**: Incoming Track auf exakt gleiche BPM wie Outgoing bringen
2. **Phase synchronisieren**: Beats müssen exakt übereinander liegen
3. **Sync während Crossfade halten**: Beide Tracks bleiben während der Überblendung synchron

### Sync-Aktivierung beim Übergang

```cpp
void AutoDJProcessor::executeIntelligentCrossfade() {
    // 1. BPM des ausgehenden Tracks ermitteln
    double masterBpm = m_pOutgoingDeck->getBpm();

    // 2. Eingehenden Track auf Master-BPM setzen
    m_pIncomingDeck->setRateRatio(masterBpm / m_pIncomingTrack->getFileBpm());

    // 3. Sync aktivieren für Phase-Lock
    ControlObject::set(ConfigKey(m_incomingGroup, "sync_enabled"), 1.0);

    // 4. Beat-Phase synchronisieren
    ControlObject::set(ConfigKey(m_incomingGroup, "beatsync_phase"), 1.0);

    // 5. Crossfade starten (Beats sind jetzt synchron)
    startCrossfade();
}
```

### Unterschied: Manueller Sync vs. Auto DJ Sync

| Aspekt | Manueller Sync | Auto DJ Intelligent Sync |
|--------|----------------|--------------------------|
| BPM Match | DJ passt manuell an | Automatisch berechnet |
| Phase Sync | DJ drückt Sync-Button | Automatisch bei Crossfade-Start |
| Während Mix | DJ überwacht | System hält Sync aufrecht |
| Bei Drift | DJ korrigiert | Auto-Korrektur aktiv |

---

## BPM-Verlauf Steuerung (Track-Auswahl)

### Neue Option: BPM-Tendenz für Auto DJ

Der Auto DJ soll bei der Track-Auswahl die BPM berücksichtigen können:

### Einstellungen

```cpp
enum class BpmTendency {
    KEEP_CONSTANT,    // BPM bleibt gleich (±2 BPM Toleranz)
    INCREASE_SLOWLY,  // BPM steigt langsam (+1-3 BPM pro Track)
    INCREASE_FAST,    // BPM steigt schnell (+3-8 BPM pro Track)
    DECREASE_SLOWLY,  // BPM sinkt langsam (-1-3 BPM pro Track)
    DECREASE_FAST,    // BPM sinkt schnell (-3-8 BPM pro Track)
    ENERGY_WAVE,      // BPM steigt und fällt (Wellenform)
    NO_PREFERENCE     // BPM wird nicht berücksichtigt
};
```

### Track-Auswahl-Logik

```cpp
TrackPointer AutoDJProcessor::selectNextTrack(
        const QList<TrackPointer>& queue,
        double currentBpm,
        BpmTendency tendency) {

    // Ziel-BPM-Bereich basierend auf Tendenz
    double targetBpmMin, targetBpmMax;

    switch (tendency) {
        case BpmTendency::KEEP_CONSTANT:
            targetBpmMin = currentBpm - 2.0;
            targetBpmMax = currentBpm + 2.0;
            break;

        case BpmTendency::INCREASE_SLOWLY:
            targetBpmMin = currentBpm + 1.0;
            targetBpmMax = currentBpm + 3.0;
            break;

        case BpmTendency::INCREASE_FAST:
            targetBpmMin = currentBpm + 3.0;
            targetBpmMax = currentBpm + 8.0;
            break;

        case BpmTendency::DECREASE_SLOWLY:
            targetBpmMin = currentBpm - 3.0;
            targetBpmMax = currentBpm - 1.0;
            break;

        case BpmTendency::DECREASE_FAST:
            targetBpmMin = currentBpm - 8.0;
            targetBpmMax = currentBpm - 3.0;
            break;

        case BpmTendency::NO_PREFERENCE:
        default:
            // Alle Tracks in sync-barem Bereich
            targetBpmMin = currentBpm * 0.92;  // -8%
            targetBpmMax = currentBpm * 1.08;  // +8%
            break;
    }

    // Track im Zielbereich finden
    for (const auto& track : queue) {
        double trackBpm = track->getBpm();
        if (trackBpm >= targetBpmMin && trackBpm <= targetBpmMax) {
            return track;
        }
    }

    // Fallback: Nächstbester Track
    return findClosestBpmTrack(queue, currentBpm);
}
```

### UI-Element (für Commit A6)

```xml
<!-- BPM Tendenz Auswahl -->
<ComboBox name="comboBoxBpmTendency">
  <Item value="0">BPM konstant halten</Item>
  <Item value="1">BPM langsam steigern</Item>
  <Item value="2">BPM schnell steigern</Item>
  <Item value="3">BPM langsam senken</Item>
  <Item value="4">BPM schnell senken</Item>
  <Item value="5">Energie-Welle</Item>
  <Item value="6">Keine Präferenz</Item>
</ComboBox>
```

### Anwendungsbeispiele

| Szenario | BPM-Tendenz | Effekt |
|----------|-------------|--------|
| Warm-up Set | INCREASE_SLOWLY | 115 → 118 → 121 → 124 BPM |
| Peak Time | KEEP_CONSTANT | 128 → 128 → 127 → 128 BPM |
| Cool-down | DECREASE_SLOWLY | 130 → 127 → 124 → 120 BPM |
| Energy Build | INCREASE_FAST | 120 → 126 → 132 → 138 BPM |

---

## Fallback Behavior

When validation fails:
```cpp
void AutoDJProcessor::executeTransition() {
    auto validation = TransitionValidator::validate(
        m_currentTrack, m_nextTrack);

    if (validation.canUseIntelligentCrossfade) {
        executeIntelligentCrossfade();
    } else {
        // Fallback to traditional fixed-duration crossfade
        qInfo() << "Falling back to standard crossfade:"
                << validation.reason;
        executeStandardCrossfade();
    }
}
```

## User Configuration

### Settings in Auto DJ Preferences
```cpp
// Require beat match for intelligent crossfade
bool m_bRequireBeatMatch = true;

// Require key compatibility (stricter mode)
bool m_bRequireKeyMatch = false;

// Maximum BPM difference for sync
double m_maxBpmDifference = 8.0;  // ±8%
```

## Testing

### Test Cases

1. **Same BPM, same key**
   - Should allow intelligent crossfade ✓

2. **Same BPM, incompatible keys**
   - Should warn but allow (if not strict) ✓

3. **BPM within range (±8%)**
   - Should allow intelligent crossfade ✓

4. **BPM too different (>8%)**
   - Should fall back to standard crossfade

5. **Half-time/double-time (80 BPM → 160 BPM)**
   - Should recognize and allow sync ✓

6. **Missing BPM data**
   - Should fall back to standard crossfade

## Dependencies
- Uses: `src/track/keyutils.h` for key compatibility
- Uses: `src/track/beats.h` for beat grid access
- Used by: Commit A5 (Auto DJ Integration)
