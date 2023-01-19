#pragma once

#include <QObject>
#include <memory>

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

    void setFrameInfo(mixxx::audio::FramePos currentPosition,
            mixxx::audio::FramePos trackEndPosition,
            mixxx::audio::SampleRate sampleRate) override;
    void trackLoaded(TrackPointer pNewTrack) override;
    void trackBeatsUpdated(mixxx::BeatsPointer pBeats) override;

  private:
    // Update positions of previous and next beats from beatgrid.
    void lookupBeatPositions(mixxx::audio::FramePos position);
    // Update position of the closest beat based on existing previous and
    // next beat values.  Usually callers will call lookupBeatPositions first.
    void updateClosestBeat(mixxx::audio::FramePos position);
    void playPosChanged(mixxx::audio::FramePos position);

    std::unique_ptr<ControlPushButton> m_pCOQuantizeEnabled;
    std::unique_ptr<ControlObject> m_pCONextBeat;
    std::unique_ptr<ControlObject> m_pCOPrevBeat;
    std::unique_ptr<ControlObject> m_pCOClosestBeat;

    // m_pBeats is written from an engine worker thread
    mixxx::BeatsPointer m_pBeats;
};
