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

constexpr int kXwaxDeviceFrame = 32;
constexpr int kXwaxSmoothing = (128 / kXwaxDeviceFrame); /* result value is in frames */
constexpr int kQualityRingSize = 32;

class SteadyPitch;
struct VinylSignalQualityReport;

class VinylControlXwax : public VinylControl {
    Q_OBJECT
  public:
    VinylControlXwax(UserSettingsPointer pConfig, const QString& group);
    ~VinylControlXwax() override;

    static void freeLUTs();
    QString getLutDir();
    /// Decode the timecode in the samples and update the playback controls.
    void analyzeSamples(CSAMPLE* pSamples, size_t nFrames) override;

    bool writeQualityReport(VinylSignalQualityReport* qualityReportFifo) override;

  protected:
    float getAngle() override;

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
    /// Seek the track to the position of the timecoded vinyl.
    void syncPosition();
    /// Set the play button, but only while vinyl control is enabled.
    void togglePlayButton(bool on);
    /// Handle enabling and disabling of vinyl control. Returns whether it is
    /// enabled now.
    bool checkEnabled(bool was, bool is);
    void doTrackSelection(bool valid_pos, double pitch, double position);
    /// Restart both steady pitch trackers from the given pitch and time.
    void resetSteadyPitch(double pitch, double time);
    /// Update the scratching state and return how steady the pitch is, 0 while
    /// reversed.
    double checkSteadyPitch(double pitch, double time);
    /// Return the drift accumulated since the last reset of the relative
    /// drift, resetting it on jumps, passthrough, reverse or scratching.
    double calcDeltaRelativeDriftAmount(double deltaFilePosition);
    /// Switch to constant mode so the needle can be moved near the record end.
    void enableRecordEndMode();
    /// Leave record end mode and switch to relative mode.
    void disableRecordEndMode();
    /// Switch to constant mode, keeping the current vinyl control rate.
    void enableConstantMode();
    /// Switch to constant mode with the given rate.
    void enableConstantMode(double rate);
    /// Return true if enough time passed since the last UI update.
    bool uiUpdateTime(double time);
    /// Update the timecode quality from the position and pitch quality.
    void establishQuality(const double& pitch);
    /// Rate how plausible the change of the timecode position is, in percent.
    int getPositionQuality();
    /// Rate how stable the pitch is compared to the previous one, in percent.
    int getPitchQuality(const double& pitch);

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
    int m_iQualityRing[kQualityRingSize];
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
    std::unique_ptr<SteadyPitch> m_pSteadySubtle;
    std::unique_ptr<SteadyPitch> m_pSteadyGross;

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
