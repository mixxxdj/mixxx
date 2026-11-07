#pragma once

#include <QSharedPointer>
#include <QString>

#include "control/pollingcontrolproxy.h"
#include "preferences/usersettings.h"
#include "util/types.h"

class VisualPlayPosition;
struct VinylSignalQualityReport;

class VinylControl : public QObject {
    Q_OBJECT
  public:
    VinylControl(UserSettingsPointer pConfig, const QString& group);
    ~VinylControl() override;

    virtual void toggleVinylControl(bool enable);
    virtual bool isEnabled();
    virtual void analyzeSamples(CSAMPLE* pSamples, size_t nFrames) = 0;
    virtual bool writeQualityReport(VinylSignalQualityReport* qualityReportFifo) = 0;

  protected:
    virtual float getAngle() = 0;

    UserSettingsPointer m_pConfig;
    const QString m_group;

    // The VC input gain preference.
    PollingControlProxy m_vinylControlInputGain;

    // The ControlObject used to start/stop playback of the song.
    PollingControlProxy m_playButton;
    // Playback position in the song. VisualPlayPosition is updated at engine
    // rate, unlike the rate-limited playposition CO.
    QSharedPointer<VisualPlayPosition> m_visualPlayPos;
    PollingControlProxy m_trackSamples;
    PollingControlProxy m_trackSampleRate;
    // The ControlObject used to change the playback position in the song.
    PollingControlProxy m_vinylSeek;
    // this rate is used in engine buffer for transport
    // 1.0 = original rate
    PollingControlProxy m_vinylControlRate;
    // Reflects the mean value (filtered for display) used of m_vinylControlRate during
    // VC and and is used to change the speed/pitch of the song without VC
    // 1.0 = original rate
    PollingControlProxy m_rateRatio;
    // The ControlObject used to get the duration of the current song.
    PollingControlProxy m_duration;
    // The ControlObject used to get the vinyl control mode
    // (absolute/relative/scratch)
    PollingControlProxy m_mode;
    // The ControlObject used to get if the vinyl control is
    // enabled or disabled.
    PollingControlProxy m_enabled;
    // The ControlObject used to get if the vinyl control should try to
    // enable itself
    PollingControlProxy m_wantEnabled;
    // Should cueing mode be active?
    PollingControlProxy m_cueing;
    // Is pitch changing very quickly?
    PollingControlProxy m_scratching;
    PollingControlProxy m_vinylStatus;
    // looping enabled?
    PollingControlProxy m_loopEnabled;
    // show the signal in the skin?
    PollingControlProxy m_signalEnabled;
    // When the user has pressed the "reverse" button.
    PollingControlProxy m_reverseButton;
    PollingControlProxy m_passthroughEnabled;
    PollingControlProxy m_scratchPositionEnabled;

    // The lead-in time...
    int m_iLeadInTime;

    // The position of the needle on the record as read by the VinylControl
    // implementation.
    double m_dVinylPosition;

    // Used as a measure of the quality of the timecode signal.
    float m_fTimecodeQuality;

    // Whether this VinylControl instance is enabled.
    bool m_bIsEnabled;
};
