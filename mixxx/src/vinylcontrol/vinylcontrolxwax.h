#ifndef __VINYLCONTROLXWAX_H__
#define __VINYLCONTROLXWAX_H__

#include "vinylcontrol.h"
#include "steadypitch.h"
#include <time.h>
#include <QTime>

#ifdef _MSC_VER
#include "timecoder.h"
#else
extern "C" {
#include "timecoder.h"
}
#endif

#define XWAX_DEVICE_FRAME 32
#define XWAX_SMOOTHING (128 / XWAX_DEVICE_FRAME) /* result value is in frames */
#define RING_SIZE 30
#define QUALITY_RING_SIZE 100
#define MIN_SIGNAL 75


class VinylControlXwax : public VinylControl
{
  public:
    VinylControlXwax(ConfigObject<ConfigValue> *pConfig, QString group);
    virtual ~VinylControlXwax();
    void ToggleVinylControl(bool enable);
    bool isEnabled();
    static void freeLUTs();
    unsigned char* getScopeBytemap();
    float getAngle();
    void AnalyseSamples(const short* samples, size_t size);
  protected:
    void run(); // main thread loop
  private:
    void syncPosition();
    void togglePlayButton(bool on);
    bool checkEnabled(bool was, bool is);
    void doTrackSelection(bool valid_pos, double pitch, double position);
    void resetSteadyPitch(double pitch, double time);
    double checkSteadyPitch(double pitch, double time);
    void enableRecordEndMode();
    void disableRecordEndMode();
    void enableConstantMode();
    void enableConstantMode(double rate);
    bool uiUpdateTime(double time);
    void establishQuality(bool quality_sample);

    double dFileLength; // The length (in samples) of the current song.
    unsigned int m_uiSafeZone; // Cache the position of the end of record

    double dOldPos; // The position read last time it was polled.
    double dOldPitch;

    bool bQualityRing[QUALITY_RING_SIZE];
    int iQualPos;
    int iQualFilled;

    bool bSeeking; // Are we seeking through the record? (ie. is it moving really fast?)
    bool bHaveSignal; // Any signal at all?
    bool bAtRecordEnd;
    bool bForceResync;
    int iOldMode;
    double dOldFilePos;
    SteadyPitch *m_pSteadySubtle, *m_pSteadyGross;
    QTime tSinceSteadyPitch;
    double dUiUpdateTime;

    ControlObjectThread *trackSelector, *trackLoader;
    double dLastTrackSelectPos;
    double dCurTrackSelectPos;
    bool bTrackSelectMode;

    // Contains information that xwax's code needs internally about the timecode and how to process it.
    struct timecoder timecoder;
    static QMutex s_xwaxLUTMutex; /** Static mutex that protects our creation/destruction of the xwax LUTs */
    static bool m_bLUTInitialized;

    short*  m_samples;
    size_t  m_SamplesSize;

    bool bShouldClose;
    bool bIsRunning;
    bool m_bNeedleSkipPrevention;
    bool m_bCDControl;
};

#endif
