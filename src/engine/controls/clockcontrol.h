#pragma once

#include "engine/controls/enginecontrol.h"
#include "preferences/usersettings.h"
#include "track/beats.h"
#include "track/track_decl.h"

class ControlProxy;
class ControlObject;

class ClockControl: public EngineControl {
    Q_OBJECT
  public:
    ClockControl(const QString& group,
            UserSettingsPointer pConfig);

    ~ClockControl() override;

    void process(const double dRate, const double currentSample,
            const int iBufferSize) override;

    void updateIndicators(const double dRate,
            const double currentSample,
            const double sampleRate);

    void trackLoaded(TrackPointer pNewTrack) override;
    void trackBeatsUpdated(mixxx::BeatsPointer pBeats) override;

  private:
    ControlObject* m_pCOBeatActive;

    // ControlObjects that come from LoopingControl
    ControlProxy* m_pLoopEnabled;
    ControlProxy* m_pLoopStartPosition;
    ControlProxy* m_pLoopEndPosition;

    double m_lastEvaluatedSample;

    enum class StateMachine : int {
        afterBeatDirectionChanged =
                2, /// Direction changed to reverse playing while forward playing indication was on
        afterBeatActive =
                1, /// Forward playing, set at the beat and set back to 0.0 at 20% of beat distance
        outsideIndicationArea =
                0, /// Outside -20% ... +20% of the beat distance
        beforeBeatActive =
                -1, /// Reverse playing, set at the beat and set back to 0.0 at -20% of beat distance
        beforeBeatDirectionChanged =
                -2 /// Direction changed to forward playing while reverse playing indication was on
    };

    StateMachine m_InternalState;

    double m_PrevBeatSamples;
    double m_NextBeatSamples;
    double m_blinkIntervalSamples;

    // True is forward direction, False is reverse
    bool m_lastPlayDirection;

    // m_pBeats is written from an engine worker thread
    mixxx::BeatsPointer m_pBeats;
};
