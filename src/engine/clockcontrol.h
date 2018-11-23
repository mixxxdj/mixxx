#ifndef CLOCKCONTROL_H
#define CLOCKCONTROL_H

#include "preferences/usersettings.h"
#include "engine/enginecontrol.h"

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
                   const double totalSamples, const int iBufferSize) override;

  public slots:
    void trackLoaded(TrackPointer pNewTrack) override;
    void slotBeatsUpdated();

  private:
    ControlObject* m_pCOBeatActive;
    ControlProxy* m_pCOSampleRate;
    TrackPointer m_pTrack; // is witten from an engine worker thread
    BeatsPointer m_pBeats;
};

#endif /* CLOCKCONTROL_H */
