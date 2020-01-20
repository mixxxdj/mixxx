#pragma once

#include <QString>

#include "util/types.h"
#include "preferences/usersettings.h"
#include "vinylcontrol/vinylsignalquality.h"

class ControlProxy;

class VinylControl : public QObject {
  public:
    VinylControl(UserSettingsPointer pConfig, const QString& group);
    virtual ~VinylControl();

    virtual void toggleVinylControl(bool enable);
    virtual bool isEnabled();
    virtual void analyzeSamples(CSAMPLE* pSamples, size_t nFrames) = 0;
    virtual bool writeQualityReport(VinylSignalQualityReport* qualityReportFifo) = 0;

  protected:
    virtual float getAngle() = 0;

    UserSettingsPointer m_pConfig;
    const QString m_group;

    // The VC input gain preference.
    ControlProxy* m_pVinylControlInputGain;

    //The ControlObject used to start/stop playback of the song.
    ControlProxy* playButton;
    //The ControlObject used to read the playback position in the song.
    ControlProxy* playPos;
    ControlProxy* trackSamples;
    ControlProxy* trackSampleRate;
    //The ControlObject used to change the playback position in the song.
    ControlProxy* vinylSeek;
    // this rate is used in engine buffer for transport
    // 1.0 = original rate
    ControlProxy* m_pVCRate;
    // Reflects the mean value (filtered for display) used of m_pVCRate during
    // VC and and is used to change the speed/pitch of the song without VC
    // 1.0 = original rate
    ControlProxy* m_pRateRatio;
    // The ControlObject used to get the duration of the current song.
    ControlProxy* duration;
    // The ControlObject used to get the vinyl control mode
    // (absolute/relative/scratch)
    ControlProxy* mode;
    // The ControlObject used to get if the vinyl control is
    // enabled or disabled.
    ControlProxy* enabled;
    // The ControlObject used to get if the vinyl control should try to
    // enable itself
    ControlProxy* wantenabled;
    // Should cueing mode be active?
    ControlProxy* cueing;
    // Is pitch changing very quickly?
    ControlProxy* scratching;
    ControlProxy* vinylStatus;
    // looping enabled?
    ControlProxy* loopEnabled;
    // show the signal in the skin?
    ControlProxy* signalenabled;
    // When the user has pressed the "reverse" button.
    ControlProxy* reverseButton;

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
