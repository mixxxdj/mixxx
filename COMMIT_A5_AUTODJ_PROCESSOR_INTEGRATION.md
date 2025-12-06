# Commit A5: Auto DJ - Intelligent Crossfade Integration

## Status: 📋 Prepared (Ready for Implementation)

## Summary
Integrate all components (intro/outro detection, energy analysis, crossfade calculation,
beat/key validation) into the Auto DJ Processor for seamless intelligent crossfading.

## Main Integration Point

**`src/library/autodj/autodjprocessor.h/cpp`**

## Current Auto DJ Flow (Simplified)

```
1. Track ends → fadeNow() called
2. Fixed crossfade duration applied
3. Next track starts
```

## New Intelligent Flow

```
1. Next track queued
2. Analyze incoming track (intro end, energy gradient)
3. Validate beat/key compatibility
4. Calculate optimal crossfade config
5. At outro start of current track → begin intelligent crossfade
6. Crossfade completes at intro end of next track
7. Next track "drops" perfectly on beat
```

## Files to Modify

### 1. `src/library/autodj/autodjprocessor.h`

**Add new members:**
```cpp
class AutoDJProcessor : public QObject {
    // ... existing ...

  public:
    // Enable/disable intelligent crossfading
    void setIntelligentCrossfadeEnabled(bool enabled);
    bool isIntelligentCrossfadeEnabled() const;

  private:
    // New components
    std::unique_ptr<CrossfadeCalculator> m_pCrossfadeCalculator;
    std::unique_ptr<TransitionValidator> m_pTransitionValidator;

    // Current transition config
    CrossfadeCalculator::CrossfadeConfig m_currentCrossfadeConfig;
    bool m_bIntelligentCrossfadeEnabled = true;
    bool m_bUsingIntelligentCrossfade = false;

    // Cached analysis results
    struct TrackAnalysis {
        double introEndPosition = 0.0;
        double outroStartPosition = 1.0;
        double energyGradient = 0.0;
        AnalyzerIntroOutro::TransitionType transitionType =
            AnalyzerIntroOutro::TransitionType::MEDIUM;
    };
    QHash<TrackId, TrackAnalysis> m_trackAnalysisCache;

    // New methods
    void prepareIntelligentTransition(TrackPointer incoming);
    void executeIntelligentCrossfade();
    void executeStandardCrossfade();
    TrackAnalysis getOrAnalyzeTrack(TrackPointer track);
    double calculateCrossfadeStartPosition();
};
```

### 2. Implementation Overview

**`src/library/autodj/autodjprocessor.cpp`**

```cpp
void AutoDJProcessor::prepareIntelligentTransition(TrackPointer incoming) {
    if (!m_bIntelligentCrossfadeEnabled) {
        return;
    }

    TrackPointer outgoing = getLoadedTrack(m_primaryDeck);
    if (!outgoing || !incoming) {
        return;
    }

    // Step 1: Get/compute analysis for incoming track
    TrackAnalysis analysis = getOrAnalyzeTrack(incoming);

    // Step 2: Validate beat and key compatibility
    auto validation = TransitionValidator::validate(outgoing, incoming);

    if (!validation.canUseIntelligentCrossfade) {
        qInfo() << "Auto DJ: Cannot use intelligent crossfade -"
                << validation.reason;
        m_bUsingIntelligentCrossfade = false;
        return;
    }

    // Step 3: Calculate crossfade configuration
    m_currentCrossfadeConfig = CrossfadeCalculator::calculate(
        outgoing,
        incoming,
        analysis.transitionType,
        analysis.energyGradient);

    m_bUsingIntelligentCrossfade = true;

    qInfo() << "Auto DJ: Prepared intelligent crossfade"
            << "Duration:" << m_currentCrossfadeConfig.duration << "s"
            << "Curve:" << static_cast<int>(m_currentCrossfadeConfig.curve);
}

AutoDJProcessor::TrackAnalysis
AutoDJProcessor::getOrAnalyzeTrack(TrackPointer track) {
    TrackId trackId = track->getId();

    // Check cache
    if (m_trackAnalysisCache.contains(trackId)) {
        return m_trackAnalysisCache[trackId];
    }

    // Analyze track
    TrackAnalysis analysis;

    // Try to get pre-analyzed intro/outro positions
    analysis.introEndPosition = track->getIntroEndPosition();
    analysis.outroStartPosition = track->getOutroStartPosition();

    // If not available, use defaults or quick analysis
    if (analysis.introEndPosition <= 0) {
        // Quick estimation: first 15% is intro
        analysis.introEndPosition = 0.15;
    }
    if (analysis.outroStartPosition >= 1.0) {
        // Quick estimation: last 15% is outro
        analysis.outroStartPosition = 0.85;
    }

    // Get energy gradient (would need waveform access)
    analysis.energyGradient = estimateEnergyGradient(track);
    analysis.transitionType = classifyTransition(analysis.energyGradient);

    // Cache result
    m_trackAnalysisCache[trackId] = analysis;

    return analysis;
}

void AutoDJProcessor::playerPositionChanged(DeckAttributes* pAttributes) {
    // ... existing code ...

    if (m_bUsingIntelligentCrossfade) {
        // Check if we've reached the crossfade start point
        double currentPosition = pAttributes->playPosition();
        double crossfadeStart = calculateCrossfadeStartPosition();

        if (currentPosition >= crossfadeStart && !m_bCrossfadeInProgress) {
            executeIntelligentCrossfade();
        }
    }
}

void AutoDJProcessor::executeIntelligentCrossfade() {
    // Start crossfade with calculated config
    double duration = m_currentCrossfadeConfig.duration;
    auto curve = m_currentCrossfadeConfig.curve;

    // Cue the next track at the right position
    double fadeInStart = m_currentCrossfadeConfig.fadeInStart;
    seekToPosition(m_secondaryDeck, fadeInStart);

    // Start playing with sync
    playWithSync(m_secondaryDeck);

    // Apply crossfade curve over duration
    startCrossfade(duration, curve);

    m_bCrossfadeInProgress = true;

    qInfo() << "Auto DJ: Executing intelligent crossfade"
            << "Duration:" << duration << "s";
}

void AutoDJProcessor::executeStandardCrossfade() {
    // Existing fade behavior
    fadeNow();
}
```

### 3. Crossfade Execution

```cpp
void AutoDJProcessor::startCrossfade(
        double duration,
        CrossfadeCalculator::CurveType curve) {

    m_crossfadeStartTime = QDateTime::currentMSecsSinceEpoch();
    m_crossfadeDuration = duration * 1000;  // Convert to ms
    m_crossfadeCurve = curve;

    // Start timer for smooth crossfade updates
    m_pCrossfadeTimer->start(20);  // 50 Hz update rate
}

void AutoDJProcessor::updateCrossfade() {
    qint64 elapsed = QDateTime::currentMSecsSinceEpoch() - m_crossfadeStartTime;
    double progress = static_cast<double>(elapsed) / m_crossfadeDuration;

    if (progress >= 1.0) {
        // Crossfade complete
        finishCrossfade();
        return;
    }

    // Apply curve
    double curvedProgress = applyCurve(progress, m_crossfadeCurve);

    // Set volumes
    double outgoingVolume = 1.0 - curvedProgress;
    double incomingVolume = curvedProgress;

    setDeckVolume(m_primaryDeck, outgoingVolume);
    setDeckVolume(m_secondaryDeck, incomingVolume);
}
```

## Configuration Options

### Add to Auto DJ Settings

```cpp
// In AutoDJProcessor or config
struct IntelligentCrossfadeSettings {
    bool enabled = true;
    bool requireBeatMatch = true;
    bool requireKeyMatch = false;
    double maxBpmDifference = 8.0;
    double minCrossfadeDuration = 1.0;
    double maxCrossfadeDuration = 30.0;
};
```

## Logging / Debug Output

```cpp
// Useful debug info during development
qDebug() << "Auto DJ Intelligent Crossfade:"
         << "\n  Outgoing:" << outgoing->getTitle()
         << "\n  Incoming:" << incoming->getTitle()
         << "\n  Intro End:" << analysis.introEndPosition
         << "\n  Energy Gradient:" << analysis.energyGradient
         << "\n  Transition Type:" << static_cast<int>(analysis.transitionType)
         << "\n  Crossfade Duration:" << m_currentCrossfadeConfig.duration << "s"
         << "\n  Beat Match:" << validation.beatMatchPossible
         << "\n  Key Match:" << validation.keyCompatible;
```

## Error Handling

```cpp
void AutoDJProcessor::handleCrossfadeError(const QString& error) {
    qWarning() << "Auto DJ: Crossfade error -" << error;
    qWarning() << "Auto DJ: Falling back to standard crossfade";

    m_bUsingIntelligentCrossfade = false;
    executeStandardCrossfade();
}
```

## Testing

### Integration Tests

1. **Full cycle with compatible tracks**
   - Queue EDM tracks with clear intros
   - Verify intelligent crossfade triggers at correct time
   - Verify smooth transition without beat skip

2. **Fallback to standard**
   - Queue tracks with incompatible BPM
   - Verify fallback is triggered
   - Verify standard crossfade still works

3. **Mixed playlist**
   - Queue mix of compatible/incompatible tracks
   - Verify correct behavior for each transition

4. **Enable/disable toggle**
   - Toggle intelligent crossfade setting
   - Verify behavior changes accordingly

## Dependencies
- Requires: Commit A1 (Intro/Outro Detection)
- Requires: Commit A2 (Energy Gradient)
- Requires: Commit A3 (Crossfade Calculation)
- Requires: Commit A4 (Beat/Key Validation)
- Used by: Commit A6 (UI Settings)
