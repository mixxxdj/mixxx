#ifndef QUANTIZECONTROL_H
#define QUANTIZECONTROL_H

#include <QObject>

#include "configobject.h"
#include "engine/enginecontrol.h"

#include "trackinfoobject.h"
#include "track/beats.h"

class ControlObject;
class ControlPushButton;
class ControlObjectThread;

class QuantizeControl : public EngineControl {
    Q_OBJECT
  public:
    QuantizeControl(QString group, ConfigObject<ConfigValue>* pConfig);
    virtual ~QuantizeControl();

    double process(const double dRate,
                   const double currentSample,
                   const double totalSamples,
                   const int iBufferSize);

  public slots:
    virtual void trackLoaded(TrackPointer pTrack);
    virtual void trackUnloaded(TrackPointer pTrack);

  private slots:
    void slotBeatsUpdated();

  private:
    ControlPushButton* m_pCOQuantizeEnabled;
    ControlObject* m_pCONextBeat;
    ControlObject* m_pCOPrevBeat;
    ControlObject* m_pCOClosestBeat;

    TrackPointer m_pTrack;
    BeatsPointer m_pBeats;
};

#endif // QUANTIZECONTROL_H
