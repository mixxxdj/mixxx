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
    std::unique_ptr<ControlObject> m_pCOBeatActive;

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

    // m_pBeats is written from an engine worker thread
    mixxx::BeatsPointer m_pBeats;
};
