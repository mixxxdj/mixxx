#ifndef VINYLCONTROL_H
#define VINYLCONTROL_H

#include <qthread.h>
#include "configobject.h"
#include "controlobject.h"
#include "soundmanagerconfig.h"
//XXX: this is only so we have access to vinylcontrol status consts
#include "engine/enginebuffer.h"

class ControlObjectThread;

#define MIXXX_VINYL_FINALSCRATCH "Final Scratch (crappy)"
#define MIXXX_VINYL_MIXVIBESDVS "MixVibes DVS"
#define MIXXX_VINYL_SERATOCV02VINYLSIDEA "Serato CV02 Vinyl, Side A"
#define MIXXX_VINYL_SERATOCV02VINYLSIDEB "Serato CV02 Vinyl, Side B"
#define MIXXX_VINYL_SERATOCD "Serato CD"
#define MIXXX_VINYL_TRAKTORSCRATCHSIDEA "Traktor Scratch, side A"
#define MIXXX_VINYL_TRAKTORSCRATCHSIDEB "Traktor Scratch, side B"

#define MIXXX_VINYL_SPEED_33 "33.3 RPM"
#define MIXXX_VINYL_SPEED_45 "45 RPM"

#define MIXXX_VINYL_SPEED_33_NUM 33.3f 
#define MIXXX_VINYL_SPEED_45_NUM 45.0f 

#define MIXXX_VCMODE_ABSOLUTE 0
#define MIXXX_VCMODE_RELATIVE 1
#define MIXXX_VCMODE_CONSTANT  2
#define MIXXX_VCMODE_SCRATCH  3

#define MIXXX_RELATIVE_CUE_OFF 0
#define MIXXX_RELATIVE_CUE_ONECUE 1
#define MIXXX_RELATIVE_CUE_HOTCUE 2

#define MIXXX_VC_DEFAULT_LEADINTIME 0

#define MIXXX_VINYL_SCOPE_SIZE 100

//TODO: Make this an EngineObject instead one day? (need to route all the input audio through the engine that way too...)

class VinylControl : public QThread
{
  public:
    VinylControl(ConfigObject<ConfigValue> *pConfig, QString group);
    virtual ~VinylControl();
    virtual void ToggleVinylControl(bool enable) = 0;
    virtual bool isEnabled() = 0;
    /*virtual void syncPitch(double pitch) = 0;
    virtual void syncPosition() = 0; */
    virtual void AnalyseSamples(const short* samples, size_t size) = 0;
    virtual float getSpeed();
    virtual float getTimecodeQuality();
    virtual unsigned char* getScopeBytemap();
    virtual float getAngle();
  protected:
    virtual void run() = 0; // main thread loop

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
    int iRIAACorrection;
    int iVCMode;
    bool atRecordEnd;

    QWaitCondition waitForNextInput;
    QMutex lockInput;
    QMutex lockSamples;
};

#endif
