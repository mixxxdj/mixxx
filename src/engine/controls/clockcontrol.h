#ifndef CLOCKCONTROL_H
#define CLOCKCONTROL_H

#include "preferences/usersettings.h"
#include "engine/controls/enginecontrol.h"

#include "track/track.h"
#include "track/beats.h"

class ControlProxy;
class ControlObject;

class ClockControl: public EngineControl {
    Q_OBJECT
  public:
    ClockControl(QString group,
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

#endif /* CLOCKCONTROL_H */
