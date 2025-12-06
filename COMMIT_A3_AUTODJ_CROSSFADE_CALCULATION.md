# Commit A3: Auto DJ - Dynamic Crossfade Duration Calculation

## Status: 📋 Prepared (Ready for Implementation)

## Summary
Calculate optimal crossfade duration dynamically based on:
- Energy gradient after intro (from A2)
- Track BPM
- Transition type

## Goal
Replace fixed crossfade duration with intelligent, track-specific timing.

## Crossfade Duration Rules

### Based on Transition Type

| Transition | Beats | Typical Duration (128 BPM) |
|------------|-------|---------------------------|
| HARD | 2-4 beats | 0.9 - 1.9 seconds |
| MEDIUM | 8-16 beats | 3.8 - 7.5 seconds |
| SOFT | 32-64 beats | 15 - 30 seconds |

### Formula
```cpp
duration_seconds = (beats / bpm) * 60
```

## Files to Create/Modify

### 1. Create CrossfadeCalculator Class

**`src/library/autodj/crossfadecalculator.h`**
```cpp
#pragma once

#include "track/track_decl.h"
#include "analyzer/analyzerintrooutro.h"

class CrossfadeCalculator {
  public:
    struct CrossfadeConfig {
        double duration;        // Duration in seconds
        double startPosition;   // When to start (in outgoing track)
        double fadeInStart;     // When to start fade in (in incoming track)
        CurveType curve;        // Fade curve type
    };

    enum class CurveType {
        LINEAR,
        EXPONENTIAL,
        S_CURVE,
        EQUAL_POWER
    };

    static CrossfadeConfig calculate(
        TrackPointer outgoingTrack,
        TrackPointer incomingTrack,
        AnalyzerIntroOutro::TransitionType transitionType,
        double energyGradient);

  private:
    static double calculateDurationBeats(
        AnalyzerIntroOutro::TransitionType type,
        double energyGradient);

    static CurveType selectCurve(
        AnalyzerIntroOutro::TransitionType type);
};
```

### 2. Implementation

**`src/library/autodj/crossfadecalculator.cpp`**
```cpp
#include "library/autodj/crossfadecalculator.h"
#include "track/track.h"

CrossfadeCalculator::CrossfadeConfig CrossfadeCalculator::calculate(
        TrackPointer outgoingTrack,
        TrackPointer incomingTrack,
        AnalyzerIntroOutro::TransitionType transitionType,
        double energyGradient) {

    CrossfadeConfig config;

    // Get BPM (use incoming track's BPM)
    double bpm = incomingTrack->getBpm();
    if (bpm <= 0) {
        bpm = 128.0;  // Default fallback
    }

    // Calculate duration in beats based on transition type
    double beats = calculateDurationBeats(transitionType, energyGradient);

    // Convert to seconds
    config.duration = (beats / bpm) * 60.0;

    // Clamp to reasonable range
    config.duration = std::clamp(config.duration, 1.0, 30.0);

    // Start position: outro start of outgoing track
    config.startPosition = outgoingTrack->getOutroStartPosition();

    // Fade in starts at: intro end of incoming track MINUS duration
    double introEnd = incomingTrack->getIntroEndPosition();
    double trackDuration = incomingTrack->getDuration();
    config.fadeInStart = std::max(0.0,
        (introEnd * trackDuration) - config.duration) / trackDuration;

    // Select appropriate curve
    config.curve = selectCurve(transitionType);

    return config;
}

double CrossfadeCalculator::calculateDurationBeats(
        AnalyzerIntroOutro::TransitionType type,
        double energyGradient) {

    switch (type) {
        case AnalyzerIntroOutro::TransitionType::HARD:
            // Fast transition: 2-4 beats
            // Higher gradient = fewer beats
            return 4.0 - (energyGradient * 4.0);  // 2-4 beats

        case AnalyzerIntroOutro::TransitionType::MEDIUM:
            // Medium transition: 8-16 beats
            return 16.0 - (energyGradient * 16.0);  // 8-16 beats

        case AnalyzerIntroOutro::TransitionType::SOFT:
            // Slow transition: 32-64 beats
            return 64.0 - (energyGradient * 64.0);  // 32-64 beats

        default:
            return 16.0;  // Default: 16 beats
    }
}

CrossfadeCalculator::CurveType CrossfadeCalculator::selectCurve(
        AnalyzerIntroOutro::TransitionType type) {

    switch (type) {
        case AnalyzerIntroOutro::TransitionType::HARD:
            // Hard cuts work best with linear or quick exponential
            return CurveType::LINEAR;

        case AnalyzerIntroOutro::TransitionType::MEDIUM:
            // S-curve for smooth but defined transitions
            return CurveType::S_CURVE;

        case AnalyzerIntroOutro::TransitionType::SOFT:
            // Equal power for gradual, natural-sounding fades
            return CurveType::EQUAL_POWER;

        default:
            return CurveType::S_CURVE;
    }
}
```

### 3. Curve Implementation

```cpp
// Fade curve functions (0.0 to 1.0 input, 0.0 to 1.0 output)

double applyCurve(double t, CurveType curve) {
    switch (curve) {
        case CurveType::LINEAR:
            return t;

        case CurveType::EXPONENTIAL:
            return t * t;

        case CurveType::S_CURVE:
            // Smooth S-curve: 3t² - 2t³
            return t * t * (3.0 - 2.0 * t);

        case CurveType::EQUAL_POWER:
            // Equal power crossfade
            return std::sin(t * M_PI / 2.0);

        default:
            return t;
    }
}
```

## Visual: Crossfade Curves

```
Volume
1.0 |    ___
    |   /   \___  LINEAR (hard)
    |  /        \
    | /          \
0.0 |/____________\____
    Start      End

1.0 |      ___
    |    _/   \_   S_CURVE (medium)
    |  _/       \_
    | /           \
0.0 |/_____________\___
    Start        End

1.0 |        ___
    |     __/   \__   EQUAL_POWER (soft)
    |   _/         \_
    | _/             \_
0.0 |/_________________\
    Start            End
```

## Integration with Auto DJ

```cpp
// In AutoDJProcessor
void AutoDJProcessor::calculateNextTransition() {
    TrackPointer outgoing = getCurrentTrack();
    TrackPointer incoming = getNextTrack();

    // Get analysis data
    auto transitionType = incoming->getTransitionType();
    double energyGradient = incoming->getEnergyGradient();

    // Calculate optimal crossfade
    auto config = CrossfadeCalculator::calculate(
        outgoing, incoming, transitionType, energyGradient);

    // Store for execution
    m_nextCrossfadeConfig = config;
}
```

## Testing

### Test Scenarios

1. **EDM Drop (HARD)**
   - Incoming: high gradient after intro
   - Expected: 2-4 beats, linear curve

2. **Pop Transition (MEDIUM)**
   - Incoming: moderate gradient
   - Expected: 8-16 beats, S-curve

3. **Ambient Mix (SOFT)**
   - Incoming: low gradient
   - Expected: 32+ beats, equal power curve

4. **Fast BPM (170)**
   - Should result in shorter time duration

5. **Slow BPM (90)**
   - Should result in longer time duration

## Dependencies
- Requires: Commit A1 (Intro/Outro Detection)
- Requires: Commit A2 (Energy Gradient)
- Used by: Commit A5 (Auto DJ Integration)
