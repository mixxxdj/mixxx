#pragma once

#include <QMutex>
#include <vector>

#include "util/types.h"
#include "vinylcontrol/vinylcontrol.h"

#ifdef _MSC_VER
#include "timecoder.h"
#else
extern "C" {
#include "timecoder.h"
}
#endif

#define XWAX_DEVICE_FRAME 32
#define XWAX_SMOOTHING (128 / XWAX_DEVICE_FRAME) /* result value is in frames */
#define QUALITY_RING_SIZE 32

class ControlProxy;
class SteadyPitch;
struct VinylSignalQualityReport;

class VinylControlXwax : public VinylControl {
    Q_OBJECT
  public:
    VinylControlXwax(UserSettingsPointer pConfig, const QString& group);
    virtual ~VinylControlXwax();

    static void freeLUTs();
    QString getLutDir();
    void analyzeSamples(CSAMPLE* pSamples, size_t nFrames);

    virtual bool writeQualityReport(VinylSignalQualityReport* qualityReportFifo);

  protected:
    float getAngle();

  private:
    /// Convert the samples to shorts with the input gain applied and pass
    /// them to the xwax timecoder.
    void submitSamples(const CSAMPLE* pSamples, size_t nFrames);
    /// Force a resync and leave record end mode when a new track was loaded.
    void handleTrackChange();
    /// Apply the requested vinyl control mode, refusing absolute mode while
    /// playing or looping and disabling cueing in absolute mode.
    void updateMode(bool playing);
    /// Enter or leave record end mode and blink the status while in it.
    void updateRecordEndMode(bool playing, double pitch, double filePosition);
    /// Follow the decoded timecode position. Returns false if playback was
    /// stopped at the end of the track.
    bool followTimecodePosition(double pitch, double filePosition, double fractionalPos);
    /// Resync to the timecode position if a pending resync, the lead-in or a
    /// position jump requires it. Returns true if one of these cases applied.
    bool resyncToTimecode(double pitch, double filePosition);
    /// Follow the pitch while no timecode position could be decoded. Returns
    /// false if playback was stopped at the end of the track.
    bool followTimecodePitch(double pitch, double filePosition, double fractionalPos);
    /// Stop playback and reset the pitch tracking at the end of the track.
    void stopAtTrackEnd();
    /// Set the playback rate to the detected pitch and update the displayed
    /// rate.
    void updateRateFromPitch(double pitch, double filePosition);
    /// Record the pitch while playing, reset the ring otherwise.
    void updatePitchRing(double pitch, bool playing);
    /// Stop playback once the signal has been missing for too long.
    void handleMissingSignal(double filePosition);
    /// Reset the timecode quality shown in the UI.
    void resetTimecodeQuality();
    void syncPosition();
    void togglePlayButton(bool on);
    bool checkEnabled(bool was, bool is);
    void doTrackSelection(bool valid_pos, double pitch, double position);
    void resetSteadyPitch(double pitch, double time);
    double checkSteadyPitch(double pitch, double time);
    double calcDeltaRelativeDriftAmount(double deltaFilePosition);
    void enableRecordEndMode();
    void disableRecordEndMode();
    void enableConstantMode();
    void enableConstantMode(double rate);
    bool uiUpdateTime(double time);
    void establishQuality(double& pitch);
    int getPositionQuality();
    int getPitchQuality(double& pitch);

    // Cache the position of the end of record
    unsigned int m_uiSafeZone;

    // The position read last time it was polled.
    double m_dVinylPositionOld;

    // Scratch buffer for CSAMPLE -> short conversions.
    std::vector<short> m_pWorkBuffer;
    size_t m_workBufferSize;

    // Signal quality ring buffer.
    // TODO(XXX): Replace with CircularBuffer instead of handling the ring logic
    // in VinylControlXwax.
    int m_iQualityRing[QUALITY_RING_SIZE];
    int m_iQualityRingIndex;
    int m_iQualityRingFilled;

    int m_iQualityLastPosition;
    double m_dQualityLastPitch;

    // Keeps track of the most recent position as reported by xwax.
    int m_iPosition;

    // Records whether we reached the end of the record.
    bool m_bAtRecordEnd;

    // Whether to force a resync on the next analysis loop.
    bool m_bForceResync;

    // The Vinyl Control mode and the previous mode.
    int m_iVCMode;
    int m_iOldVCMode;

    // The file position from the last run of analyzeSamples.
    double m_dOldFilePos;
    // The delta file position in relative mode
    double m_deltaFilePos;

    // The loaded track duration from the last run of analyzeSamples.
    double m_dOldDuration;

    // The approximate duration used to tell if a new track is loaded.
    double m_dOldDurationInaccurate;

    // Was the reverse button pressed last go-round?
    bool m_bWasReversed;

    // The pitch ring buffer.
    // TODO(XXX): Replace with CircularBuffer instead of handling the ring logic
    // in VinylControlXwax.
    std::vector<double> m_pPitchRing;
    // How large the pitch ring buffer is.
    int m_iPitchRingSize;
    // Our current position in the pitch ring buffer.
    int m_iPitchRingPos;
    // How much of the pitch ring buffer is "filled" versus empty (used before
    // it fills up completely).
    int m_iPitchRingFilled;

    // Steady pitch trackers.  "Subtle" will be more likely to return true,
    // so it is used to set the play button.  "Gross" is more likely to return
    // false, so it is used to trigger the "scratching" CO.
    SteadyPitch* m_pSteadySubtle;
    SteadyPitch* m_pSteadyGross;

    // Whether the configured timecode is CD-based or not.
    bool m_bCDControl;

    // The drift between the vinyl position and the file position from the most
    // recent run of analyzeSamples.
    double m_dDriftAmt;
    double m_initialRelativeDriftAmt;
    double m_deltaRelativeDriftAmount;

    // Records the time of the last UI update. Used to prevent hammering the GUI
    // with updates.
    double m_dUiUpdateTime;

    // Contains information that xwax's code needs internally about the timecode
    // and how to process it.
    struct timecoder timecoder;
    // Static mutex that protects our creation/destruction of the xwax LUTs
    static QMutex s_xwaxLUTMutex;
    static bool s_bLUTInitialized;
};
