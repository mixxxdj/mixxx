#pragma once

#include <QObject>

#include "engine/controls/enginecontrol.h"
#include "preferences/usersettings.h"
#include "track/beats.h"
#include "track/track_decl.h"

class ControlObject;
class ControlPushButton;

class QuantizeControl : public EngineControl {
    Q_OBJECT
  public:
    QuantizeControl(const QString& group, UserSettingsPointer pConfig);
    ~QuantizeControl() override;

    void setCurrentSample(const double dCurrentSample,
            const double dTotalSamples, const double dTrackSampleRate) override;
    void notifySeek(double dNewPlaypos) override;
    void trackLoaded(TrackPointer pNewTrack) override;
    void trackBeatsUpdated(mixxx::BeatsPointer pBeats) override;

  private:
    // Update positions of previous and next beats from beatgrid.
    void lookupBeatPositions(double dCurrentSample);
    // Update position of the closest beat based on existing previous and
    // next beat values.  Usually callers will call lookupBeatPositions first.
    void updateClosestBeat(double dCurrentSample);
    void playPosChanged(double dNewPlaypos);

    ControlPushButton* m_pCOQuantizeEnabled;
    ControlObject* m_pCONextBeat;
    ControlObject* m_pCOPrevBeat;
    ControlObject* m_pCOClosestBeat;

    // m_pBeats is written from an engine worker thread
    mixxx::BeatsPointer m_pBeats;
};
