#ifndef VINYLCONTROL_H
#define VINYLCONTROL_H

#include <QString>

#include "util/types.h"
#include "preferences/usersettings.h"
#include "vinylcontrol/vinylsignalquality.h"

class ControlObjectSlave;

class VinylControl : public QObject {
  public:
    VinylControl(UserSettingsPointer pConfig, QString group);
    virtual ~VinylControl();

    virtual void toggleVinylControl(bool enable);
    virtual bool isEnabled();
    virtual void analyzeSamples(CSAMPLE* pSamples, size_t nFrames) = 0;
    virtual bool writeQualityReport(VinylSignalQualityReport* qualityReportFifo) = 0;

  protected:
    virtual float getAngle() = 0;

    UserSettingsPointer m_pConfig;
    QString m_group;

    // The VC input gain preference.
    ControlObjectSlave* m_pVinylControlInputGain;

    //The ControlObject used to start/stop playback of the song.
    ControlObjectSlave* playButton;
    //The ControlObject used to read the playback position in the song.
    ControlObjectSlave* playPos;
    ControlObjectSlave* trackSamples;
    ControlObjectSlave* trackSampleRate;
    //The ControlObject used to change the playback position in the song.
    ControlObjectSlave* vinylSeek;
    // this rate is used in engine buffer for transport
    // 1.0 = original rate
    ControlObjectSlave* m_pVCRate;
    // Reflects the mean value (filtered for display) used of m_pVCRate during
    // VC and and is used to change the speed/pitch of the song without VC
    // 0.0 = original rate
    ControlObjectSlave* m_pRateSlider;
    // The ControlObject used to get the duration of the current song.
    ControlObjectSlave* duration;
    // The ControlObject used to get the vinyl control mode
    // (absolute/relative/scratch)
    ControlObjectSlave* mode;
    // The ControlObject used to get if the vinyl control is
    // enabled or disabled.
    ControlObjectSlave* enabled;
    // The ControlObject used to get if the vinyl control should try to
    // enable itself
    ControlObjectSlave* wantenabled;
    // Should cueing mode be active?
    ControlObjectSlave* cueing;
    // Is pitch changing very quickly?
    ControlObjectSlave* scratching;
    // The ControlObject used to the get the pitch range from the prefs.
    ControlObjectSlave* m_pRateRange;
    ControlObjectSlave* vinylStatus;
    // direction of rate
    ControlObjectSlave* m_pRateDir;
    // looping enabled?
    ControlObjectSlave* loopEnabled;
    // show the signal in the skin?
    ControlObjectSlave* signalenabled;
    // When the user has pressed the "reverse" button.
    ControlObjectSlave* reverseButton;

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

#endif
