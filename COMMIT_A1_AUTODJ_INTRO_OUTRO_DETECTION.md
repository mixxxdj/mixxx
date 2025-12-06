# Commit A1: Auto DJ - Intro/Outro End Detection

## Status: 📋 Prepared (Ready for Implementation)

## Summary
Implement automatic detection of intro end and outro start points in tracks
using waveform energy analysis.

## Goal
- Detect where the intro ends (track "really starts")
- Detect where the outro begins (track "winds down")
- Use these points for intelligent crossfade timing

## Files to Investigate First

```bash
# Find Auto DJ related files
find src -name "*autodj*" -type f
find src -name "*intro*" -o -name "*outro*" | grep -v test

# Find waveform analysis
find src -name "*waveform*" -type f | head -20
```

### Key Files to Examine
- `src/library/autodj/autodjprocessor.h/cpp` - Main Auto DJ logic
- `src/analyzer/analyzerebur128.cpp` - Loudness/energy analysis
- `src/waveform/waveform.h/cpp` - Waveform data structures
- `src/engine/controls/cuecontrol.h/cpp` - Cue point management

## Current Cue Point System

Mixxx already has intro/outro cue points:
- `intro_start_position` / `intro_end_position`
- `outro_start_position` / `outro_end_position`

**Question**: Are these auto-detected or manually set?

## Proposed Implementation

### 1. Create IntroOutroDetector Class

**`src/analyzer/analyzerintrooutro.h`**
```cpp
#pragma once

#include "analyzer/analyzer.h"
#include "track/track_decl.h"

class AnalyzerIntroOutro : public Analyzer {
  public:
    explicit AnalyzerIntroOutro(UserSettingsPointer pConfig);

    bool initialize(TrackPointer pTrack,
                    mixxx::audio::SampleRate sampleRate,
                    mixxx::audio::ChannelCount channelCount,
                    SINT totalFrames) override;
    bool processSamples(const CSAMPLE* pIn, SINT iLen) override;
    void storeResults(TrackPointer pTrack) override;
    void cleanup() override;

    // Results
    double getIntroEndPosition() const { return m_introEndPosition; }
    double getOutroStartPosition() const { return m_outroStartPosition; }

  private:
    void analyzeEnergyProfile();
    double detectIntroEnd();
    double detectOutroStart();

    UserSettingsPointer m_pConfig;
    std::vector<float> m_energyProfile;
    double m_introEndPosition = 0.0;
    double m_outroStartPosition = 1.0;
    SINT m_totalFrames = 0;
    mixxx::audio::SampleRate m_sampleRate;
};
```

### 2. Energy Profile Analysis

**Concept:**
- Divide track into small segments (e.g., 0.5 seconds each)
- Calculate RMS energy for each segment
- Build energy profile over time

```cpp
void AnalyzerIntroOutro::analyzeEnergyProfile() {
    // Energy profile already built during processSamples()

    // Find baseline energy (average of middle 50% of track)
    double baselineEnergy = calculateBaseline();

    // Intro end: First point where energy stays above baseline
    m_introEndPosition = detectIntroEnd(baselineEnergy);

    // Outro start: Last point where energy drops below baseline
    m_outroStartPosition = detectOutroStart(baselineEnergy);
}
```

### 3. Intro End Detection Algorithm

```cpp
double AnalyzerIntroOutro::detectIntroEnd() {
    // Parameters
    const double ENERGY_THRESHOLD = 0.7;  // 70% of peak energy
    const int MIN_SUSTAINED_SEGMENTS = 4; // ~2 seconds

    double peakEnergy = *std::max_element(m_energyProfile.begin(),
                                           m_energyProfile.end());
    double threshold = peakEnergy * ENERGY_THRESHOLD;

    int sustainedCount = 0;
    for (size_t i = 0; i < m_energyProfile.size(); i++) {
        if (m_energyProfile[i] >= threshold) {
            sustainedCount++;
            if (sustainedCount >= MIN_SUSTAINED_SEGMENTS) {
                // Found intro end - go back to where it started
                size_t introEndIndex = i - MIN_SUSTAINED_SEGMENTS + 1;
                return static_cast<double>(introEndIndex) / m_energyProfile.size();
            }
        } else {
            sustainedCount = 0;
        }
    }

    return 0.0; // No clear intro detected
}
```

### 4. Outro Start Detection Algorithm

```cpp
double AnalyzerIntroOutro::detectOutroStart() {
    // Similar to intro detection, but from the end
    const double ENERGY_THRESHOLD = 0.5;  // 50% of peak energy
    const int MIN_FADEOUT_SEGMENTS = 4;

    double peakEnergy = *std::max_element(m_energyProfile.begin(),
                                           m_energyProfile.end());
    double threshold = peakEnergy * ENERGY_THRESHOLD;

    int fadeoutCount = 0;
    for (size_t i = m_energyProfile.size() - 1; i > 0; i--) {
        if (m_energyProfile[i] < threshold) {
            fadeoutCount++;
            if (fadeoutCount >= MIN_FADEOUT_SEGMENTS) {
                // Found outro start
                size_t outroStartIndex = i + MIN_FADEOUT_SEGMENTS;
                return static_cast<double>(outroStartIndex) / m_energyProfile.size();
            }
        } else {
            fadeoutCount = 0;
        }
    }

    return 1.0; // No clear outro detected
}
```

## Integration Points

### Option A: Extend Existing Analyzer
Add to `src/analyzer/analyzerqueue.cpp` to run during track analysis

### Option B: On-Demand Analysis
Calculate intro/outro only when Auto DJ needs it

### Option C: Use Existing Cue Points
If user has manually set intro_end/outro_start, respect those

## Data Storage

### Store in Track Metadata
```cpp
// In Track class
void setAnalyzedIntroEnd(double position);
void setAnalyzedOutroStart(double position);
double getAnalyzedIntroEnd() const;
double getAnalyzedOutroStart() const;
```

### Or Use Existing Cue System
Create cue points of type INTRO_END, OUTRO_START if not manually set

## Testing

1. **Track with clear intro** (buildup → drop)
   - Should detect intro end at drop point

2. **Track with no intro** (starts immediately)
   - Should detect intro end near beginning

3. **Track with long outro/fadeout**
   - Should detect outro start at fadeout beginning

4. **Track with abrupt ending**
   - Should detect outro start near end

## Dependencies
- None (standalone analyzer)
- Used by: Commit A3 (Crossfade Duration), Commit A5 (Auto DJ Integration)
