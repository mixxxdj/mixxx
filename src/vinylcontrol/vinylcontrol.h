#ifndef VINYLCONTROL_H
#define VINYLCONTROL_H

#include <QString>

#include "configobject.h"
#include "vinylcontrol/vinylsignalquality.h"

class ControlObjectThread;

class VinylControl : public QObject {
  public:
    VinylControl(ConfigObject<ConfigValue> *pConfig, QString group);
    virtual ~VinylControl();

    virtual void toggleVinylControl(bool enable);
    virtual bool isEnabled();
    virtual void analyzeSamples(const short* samples, size_t nFrames) = 0;
    virtual float getSpeed();
    virtual bool writeQualityReport(VinylSignalQualityReport* qualityReportFifo) = 0;

  protected:
    virtual float getAngle() = 0;

    QString strVinylType;
    QString strVinylSpeed;
    ConfigObject<ConfigValue> *m_pConfig;    /** Pointer to config database */
    QString m_group;
    ControlObjectThread *playButton; //The ControlObject used to start/stop playback of the song.
    ControlObjectThread *playPos; //The ControlObject used to read the playback position in the song.
    ControlObjectThread *trackSamples;
    ControlObjectThread *trackSampleRate;
    ControlObjectThread *vinylSeek; //The ControlObject used to change the playback position in the song.
    ControlObjectThread *controlScratch; //The ControlObject used to seek when the record is spinning fast.
    ControlObjectThread *rateSlider; //The ControlObject used to change the speed/pitch of the song.
    ControlObjectThread *reverseButton; //The ControlObject used to reverse playback of the song.
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
    //ControlObject *vinylStatus;  //Status of vinyl control

    int iLeadInTime; //The lead-in time...
    float dVinylPitch; //The speed/pitch of the timecoded vinyl as read by scratchlib.
    double dVinylPosition; //The position of the needle on the record as read by scratchlib.
    double dVinylScratch;
    double dDriftControl; //The difference between Mixxx's position and the needle's position on the record.
    //... these two values naturally drift apart, so we need to keep making adjustments to the pitch
    //to stop it from getting bad.
    float fRateRange; //The pitch range setting from Mixxx's preferences
    float m_fTimecodeQuality; //Used as a measure of the quality of the timecode signal.

    float fTrackDuration;
    unsigned long iSampleRate;
    bool bIsEnabled;
    int iVCMode;
    bool atRecordEnd;
};

#endif
