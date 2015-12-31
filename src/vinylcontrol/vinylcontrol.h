#ifndef VINYLCONTROL_H
#define VINYLCONTROL_H

#include <QString>

#include "util/types.h"
#include "configobject.h"
#include "vinylcontrol/vinylsignalquality.h"

class ControlObjectThread;
class ControlObjectSlave;

class VinylControl : public QObject {
  public:
    VinylControl(ConfigObject<ConfigValue> *pConfig, QString group);
    virtual ~VinylControl();

    virtual void toggleVinylControl(bool enable);
    virtual bool isEnabled();
    virtual void analyzeSamples(CSAMPLE* pSamples, size_t nFrames) = 0;
    virtual bool writeQualityReport(VinylSignalQualityReport* qualityReportFifo) = 0;

  protected:
    virtual float getAngle() = 0;

    ConfigObject<ConfigValue>* m_pConfig;
    QString m_group;

    // The VC input gain preference.
    ControlObjectThread* m_pVinylControlInputGain;

    ControlObjectThread *playButton; //The ControlObject used to start/stop playback of the song.
    ControlObjectThread *playPos; //The ControlObject used to read the playback position in the song.
    ControlObjectThread *trackSamples;
    ControlObjectThread *trackSampleRate;
    ControlObjectThread *vinylSeek; //The ControlObject used to change the playback position in the song.
    // this rate is used in engine buffer for transport
    // 1.0 = original rate
    ControlObjectSlave* m_pVCRate;
    // Reflects the mean value (filtered for display) used of m_pVCRate during VC and
    // and is used to change the speed/pitch of the song without VC
    // 0.0 = original rate
    ControlObjectSlave* m_pRateSlider;
    ControlObjectThread *duration; //The ControlObject used to get the duration of the current song.
    ControlObjectThread *mode; //The ControlObject used to get the vinyl control mode (absolute/relative/scratch)
    ControlObjectThread *enabled; //The ControlObject used to get if the vinyl control is enabled or disabled.
    ControlObjectThread *wantenabled; //The ControlObject used to get if the vinyl control should try to enable itself
    ControlObjectThread *cueing; //Should cueing mode be active?
    ControlObjectThread *scratching; //Is pitch changing very quickly?
    ControlObjectThread *rateRange; //The ControlObject used to the get the pitch range from the prefs.
    ControlObjectThread *vinylStatus;
    ControlObjectThread *rateDir; //direction of rate
    ControlObjectThread *loopEnabled; //looping enabled?
    ControlObjectThread *signalenabled; //show the signal in the skin?
    ControlObjectThread *reverseButton; // When the user has pressed the "reverse" button.

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
