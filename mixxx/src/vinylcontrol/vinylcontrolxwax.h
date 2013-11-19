#ifndef __VINYLCONTROLXWAX_H__
#define __VINYLCONTROLXWAX_H__

#include <QTime>

#include "vinylcontrol/vinylcontrol.h"
#include "vinylcontrol/steadypitch.h"

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

class VinylControlXwax : public VinylControl {
  public:
    VinylControlXwax(ConfigObject<ConfigValue> *pConfig, QString group);
    virtual ~VinylControlXwax();

    static void freeLUTs();
    void analyzeSamples(const short* samples, size_t nFrames);

    virtual bool writeQualityReport(VinylSignalQualityReport* qualityReportFifo);

  protected:
    float getAngle();

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

    unsigned int m_uiSafeZone; // Cache the position of the end of record

    double dOldPos; // The position read last time it was polled.

    bool bQualityRing[QUALITY_RING_SIZE];
    int iQualPos;
    int iQualFilled;

    // Local variables copied from run(). TODO(XXX): Rename and prefix.
    int iPosition;
    float filePosition;
    double dDriftAmt;
    double dPitchRing[RING_SIZE];
    int ringPos;
    int ringFilled;
    double old_duration;

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

    bool m_bNeedleSkipPrevention;
    bool m_bCDControl;

    // Contains information that xwax's code needs internally about the timecode
    // and how to process it.
    struct timecoder timecoder;
    // Static mutex that protects our creation/destruction of the xwax LUTs
    static QMutex s_xwaxLUTMutex;
    static bool s_bLUTInitialized;
};

#endif
