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

    virtual ~ClockControl();

    double process(const double dRate, const double currentSample,
                   const double totalSamples, const int iBufferSize);

  public slots:
    void trackLoaded(TrackPointer pNewTrack, TrackPointer pOldTrack) override;
    void slotBeatsUpdated();

  private:
    ControlObject* m_pCOBeatActive;
    ControlProxy* m_pCOSampleRate;
    TrackPointer m_pTrack;
    BeatsPointer m_pBeats;
};

#endif /* CLOCKCONTROL_H */
