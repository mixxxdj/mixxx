#ifndef CLOCKCONTROL_H
#define CLOCKCONTROL_H

#include "configobject.h"
#include "engine/enginecontrol.h"

#include "trackinfoobject.h"
#include "track/beats.h"

class ControlObjectSlave;

class ClockControl: public EngineControl {
    Q_OBJECT
  public:
    ClockControl(QString group,
                 ConfigObject<ConfigValue>* pConfig);

    virtual ~ClockControl();

    double process(const double dRate, const double currentSample,
                   const double totalSamples, const int iBufferSize);

  public slots:
    virtual void trackLoaded(TrackPointer pTrack);
    virtual void trackUnloaded(TrackPointer pTrack);
    void slotBeatsUpdated();

  private:
    ControlObject* m_pCOBeatActive;
    ControlObjectSlave* m_pCOSampleRate;
    TrackPointer m_pTrack;
    BeatsPointer m_pBeats;
};

#endif /* CLOCKCONTROL_H */
