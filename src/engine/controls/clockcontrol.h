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

    void trackLoaded(TrackPointer pNewTrack) override;
    void trackBeatsUpdated(mixxx::BeatsPointer pBeats) override;

  private:
    ControlObject* m_pCOBeatActive;
    ControlProxy* m_pCOSampleRate;

    // m_pBeats is written from an engine worker thread
    mixxx::BeatsPointer m_pBeats;
};
