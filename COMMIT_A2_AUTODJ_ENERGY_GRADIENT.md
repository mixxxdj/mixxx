# Commit A2: Auto DJ - Energy Gradient Analysis After Intro

## Status: 📋 Prepared (Ready for Implementation)

## Summary
Analyze the energy gradient (rate of change) after the intro ends to determine
how "hard" or "soft" the track starts its main section.

## Goal
- Measure how quickly energy rises after intro
- Use this to determine crossfade speed:
  - **High gradient** (sudden jump) → Fast/hard crossfade
  - **Low gradient** (gradual rise) → Slow/smooth crossfade

## Concept

```
Energy
  ^
  |           ___________
  |          /           \
  |    _____/             \____
  |   /                        \
  |__/                          \___
  +---------------------------------> Time
     ^    ^
     |    |
     |    +-- High gradient (steep rise) → Fast crossfade
     +------- Intro end point
```

## Files to Create/Modify

### 1. Extend IntroOutroDetector

**`src/analyzer/analyzerintrooutro.h`** (from Commit A1)
```cpp
class AnalyzerIntroOutro : public Analyzer {
  public:
    // ... existing from A1 ...

    // NEW: Energy gradient analysis
    double getEnergyGradientAfterIntro() const { return m_energyGradient; }

    enum class TransitionType {
        HARD,       // Sudden energy jump (drop, impact)
        MEDIUM,     // Moderate buildup
        SOFT        // Gradual increase
    };
    TransitionType getRecommendedTransition() const;

  private:
    double calculateEnergyGradient();
    double m_energyGradient = 0.0;
};
```

### 2. Energy Gradient Calculation

**`src/analyzer/analyzerintrooutro.cpp`**
```cpp
double AnalyzerIntroOutro::calculateEnergyGradient() {
    // Get intro end index
    size_t introEndIdx = static_cast<size_t>(
        m_introEndPosition * m_energyProfile.size());

    if (introEndIdx >= m_energyProfile.size() - 1) {
        return 0.0;
    }

    // Analyze energy change in first 2 seconds after intro
    const size_t ANALYSIS_WINDOW = 4;  // ~2 seconds at 0.5s segments
    size_t endIdx = std::min(introEndIdx + ANALYSIS_WINDOW,
                              m_energyProfile.size() - 1);

    // Calculate gradient (dE/dt)
    double startEnergy = m_energyProfile[introEndIdx];
    double endEnergy = m_energyProfile[endIdx];
    double energyDelta = endEnergy - startEnergy;

    // Normalize by peak energy
    double peakEnergy = *std::max_element(m_energyProfile.begin(),
                                           m_energyProfile.end());
    if (peakEnergy > 0) {
        energyDelta /= peakEnergy;
    }

    // Normalize by time window
    double timeDelta = static_cast<double>(endIdx - introEndIdx);
    if (timeDelta > 0) {
        m_energyGradient = energyDelta / timeDelta;
    }

    return m_energyGradient;
}
```

### 3. Transition Type Classification

```cpp
AnalyzerIntroOutro::TransitionType
AnalyzerIntroOutro::getRecommendedTransition() const {
    // Thresholds (tune based on testing)
    const double HARD_THRESHOLD = 0.3;    // 30% energy jump per segment
    const double SOFT_THRESHOLD = 0.1;    // 10% energy jump per segment

    if (m_energyGradient >= HARD_THRESHOLD) {
        return TransitionType::HARD;
    } else if (m_energyGradient >= SOFT_THRESHOLD) {
        return TransitionType::MEDIUM;
    } else {
        return TransitionType::SOFT;
    }
}
```

## Visual Examples

### HARD Transition (Drop)
```
Energy: ____/‾‾‾‾‾‾‾‾
             ↑
             Steep rise = high gradient
             → Fast crossfade (2-4 beats)
```

### MEDIUM Transition (Buildup)
```
Energy: ____/‾‾‾‾‾‾‾‾
            /
           /
          Gradual rise = medium gradient
          → Medium crossfade (8-16 beats)
```

### SOFT Transition (Ambient)
```
Energy: ________/‾‾‾‾
               /
              /
             /
            Very gradual = low gradient
            → Long crossfade (32+ beats)
```

## Waveform-Based Detection

### Alternative: Use Waveform Data Directly

```cpp
double calculateGradientFromWaveform(const WaveformData* waveform,
                                      double introEndPosition) {
    // Get waveform samples around intro end
    int startIdx = static_cast<int>(introEndPosition * waveform->size());
    int windowSize = waveform->size() / 100;  // 1% of track

    // Calculate RMS before intro end
    double rmsBefore = calculateRMS(waveform, startIdx - windowSize, startIdx);

    // Calculate RMS after intro end
    double rmsAfter = calculateRMS(waveform, startIdx, startIdx + windowSize);

    // Gradient = ratio of after/before
    if (rmsBefore > 0) {
        return rmsAfter / rmsBefore;
    }
    return 1.0;
}
```

## Data to Store

### Track Metadata Extension
```cpp
// In track metadata or analyzer results
struct IntroOutroAnalysis {
    double introEndPosition;      // 0.0 - 1.0
    double outroStartPosition;    // 0.0 - 1.0
    double energyGradient;        // Normalized gradient
    TransitionType transitionType; // HARD, MEDIUM, SOFT
};
```

## Testing

### Test Cases

| Track Type | Expected Gradient | Expected Transition |
|------------|-------------------|---------------------|
| EDM with drop | High (>0.3) | HARD |
| Pop with buildup | Medium (0.1-0.3) | MEDIUM |
| Ambient/Chill | Low (<0.1) | SOFT |
| Punk/Rock (immediate) | Very High (>0.5) | HARD |

### Manual Testing
1. Load various genres
2. Verify gradient matches perceived "hardness"
3. Tune thresholds as needed

## Dependencies
- Requires: Commit A1 (Intro/Outro Detection)
- Used by: Commit A3 (Crossfade Duration Calculation)
