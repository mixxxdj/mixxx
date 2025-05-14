#pragma once

#include <memory>

#include "audio/frame.h"
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

    void updateIndicators(const double dRate,
            mixxx::audio::FramePos currentPosition,
            mixxx::audio::SampleRate sampleRate);

    void trackLoaded(TrackPointer pNewTrack) override;
    void trackBeatsUpdated(mixxx::BeatsPointer pBeats) override;

  private:
    // helper method to update fractional tempo beat indicators
    void updateFractionalTempoIndicator(double dRate,
            mixxx::audio::FramePos currentPosition,
            mixxx::audio::FramePos trackStartPosition,
            mixxx::audio::FrameDiff_t beatLength,
            double tempoRatio,
            std::unique_ptr<ControlObject>& pCOBeatActive);

    std::unique_ptr<ControlObject> m_pCOBeatActive;

    // fractional tempo beat active controls
    std::unique_ptr<ControlObject> m_pCOBeatActiveTempo_0_5;    // half tempo
    std::unique_ptr<ControlObject> m_pCOBeatActiveTempo_0_666;  // 2/3 tempo
    std::unique_ptr<ControlObject> m_pCOBeatActiveTempo_0_75;   // 3/4 tempo
    std::unique_ptr<ControlObject> m_pCOBeatActiveTempo_1_25;   // 5/4 tempo
    std::unique_ptr<ControlObject> m_pCOBeatActiveTempo_1_333;  // 4/3 tempo
    std::unique_ptr<ControlObject> m_pCOBeatActiveTempo_1_5;    // 3/2 tempo

    // ControlObjects that come from LoopingControl
    std::unique_ptr<ControlProxy> m_pLoopEnabled;
    std::unique_ptr<ControlProxy> m_pLoopStartPosition;
    std::unique_ptr<ControlProxy> m_pLoopEndPosition;

    // True is forward direction, False is reverse
    bool m_lastPlayDirectionWasForwards;

    mixxx::audio::FramePos m_lastEvaluatedPosition;
    mixxx::audio::FramePos m_prevBeatPosition;
    mixxx::audio::FramePos m_nextBeatPosition;
    mixxx::audio::FrameDiff_t m_blinkIntervalFrames;

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

    StateMachine m_internalState;

    // internal states for fractional tempo indicators
    struct FractionalTempoState {
        StateMachine state = StateMachine::outsideIndicationArea;
        mixxx::audio::FramePos prevBeatPosition = mixxx::audio::kInvalidFramePos;
        mixxx::audio::FramePos nextBeatPosition = mixxx::audio::kInvalidFramePos;
    };

    FractionalTempoState m_stateTempo_0_5;
    FractionalTempoState m_stateTempo_0_666;
    FractionalTempoState m_stateTempo_0_75;
    FractionalTempoState m_stateTempo_1_25;
    FractionalTempoState m_stateTempo_1_333;
    FractionalTempoState m_stateTempo_1_5;

    // m_pBeats is written from an engine worker thread
    mixxx::BeatsPointer m_pBeats;
};
