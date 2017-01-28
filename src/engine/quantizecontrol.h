#ifndef QUANTIZECONTROL_H
#define QUANTIZECONTROL_H

#include <QObject>

#include "preferences/usersettings.h"
#include "engine/enginecontrol.h"

#include "track/track.h"
#include "track/beats.h"

class ControlObject;
class ControlPushButton;

class QuantizeControl : public EngineControl {
    Q_OBJECT
  public:
    QuantizeControl(QString group, UserSettingsPointer pConfig);
    virtual ~QuantizeControl();

    virtual void setCurrentSample(const double dCurrentSample,
                                  const double dTotalSamples);

  public slots:
    void trackLoaded(TrackPointer pNewTrack, TrackPointer pOldTrack) override;

  private slots:
    void slotBeatsUpdated();

  private:
    // Update positions of previous and next beats from beatgrid.
    void lookupBeatPositions(double dCurrentSample);
    // Update position of the closest beat based on existing previous and
    // next beat values.  Usually callers will call lookupBeatPositions first.
    void updateClosestBeat(double dCurrentSample);

    ControlPushButton* m_pCOQuantizeEnabled;
    ControlObject* m_pCONextBeat;
    ControlObject* m_pCOPrevBeat;
    ControlObject* m_pCOClosestBeat;

    TrackPointer m_pTrack;
    BeatsPointer m_pBeats;
};

#endif // QUANTIZECONTROL_H
