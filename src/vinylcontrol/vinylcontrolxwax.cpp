#include "vinylcontrol/vinylcontrolxwax.h"

#include <QtDebug>
#include <algorithm>

#include "audio/types.h"
#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "moc_vinylcontrolxwax.cpp"
#include "util/defs.h"
#include "util/math.h"
#include "util/timer.h"
#include "vinylcontrol/defs_vinylcontrol.h"
#include "vinylcontrol/steadypitch.h"
#include "vinylcontrol/vinylsignalquality.h"
#include "waveform/visualplayposition.h"

/****** TODO *******
   Stuff to maybe implement here
   1) The smoothing thing that xwax does
   2) Tons of cleanup
   3) Speed up needle dropping
   4) Extrapolate small dropouts and keep track of "dynamics"
 ********************/

namespace {
constexpr int kChannels = 2;
} // namespace

// Sample threshold below which we consider there to be no signal.
constexpr double kMinSignal = 75.0 / SAMPLE_MAXIMUM;

bool VinylControlXwax::s_bLUTInitialized = false;
QMutex VinylControlXwax::s_xwaxLUTMutex;

VinylControlXwax::VinylControlXwax(UserSettingsPointer pConfig, const QString& group)
        : VinylControl(pConfig, group),
          m_dVinylPositionOld(0.0),
          m_pWorkBuffer(MAX_BUFFER_LEN),
          m_workBufferSize(MAX_BUFFER_LEN),
          m_iQualityRingIndex(0),
          m_iQualityRingFilled(0),
          m_iQualityLastPosition(-1),
          m_dQualityLastPitch(0.0),
          m_iPosition(-1),
          m_bAtRecordEnd(false),
          m_bForceResync(false),
          m_iVCMode(static_cast<int>(mode->get())),
          m_iOldVCMode(MIXXX_VCMODE_ABSOLUTE),
          m_dOldFilePos(0.0),
          m_deltaFilePos(0.0),
          m_dOldDuration(0.0),
          m_dOldDurationInaccurate(-1.0),
          m_bWasReversed(false),
          m_iPitchRingSize(0),
          m_iPitchRingPos(0),
          m_iPitchRingFilled(0),
          m_pSteadySubtle(nullptr),
          m_pSteadyGross(nullptr),
          m_bCDControl(false),
          m_dDriftAmt(0.0),
          m_initialRelativeDriftAmt(0.0),
          m_deltaRelativeDriftAmount(0.0),
          m_dUiUpdateTime(-1.0) {
    // TODO(rryan): Should probably live in VinylControlManager since it's not
    // specific to a VC deck.
    signalenabled->set(m_pConfig->getValueString(
                                        ConfigKey(VINYL_PREF_KEY, "show_signal_quality"))
                               .toInt());

    // Get the vinyl type and speed.
    QString strVinylType = m_pConfig->getValueString(
        ConfigKey(group,"vinylcontrol_vinyl_type"));
    QString strVinylSpeed = m_pConfig->getValueString(
        ConfigKey(group,"vinylcontrol_speed_type"));
    QString strPitchEstimator = m_pConfig->getValueString(
            ConfigKey(group, "vinylcontrol_pitch_estimator_type"));

    // libxwax indexes by C-strings so we pass libxwax string literals so we
    // don't have to deal with freeing the strings later
    const char* timecode = nullptr;

    if (strVinylType == MIXXX_VINYL_SERATOCV02VINYLSIDEA) {
        timecode = MIXXX_VINYL_SERATOCV02VINYLSIDEA_XWAX_NAME;
    } else if (strVinylType == MIXXX_VINYL_SERATOCV02VINYLSIDEB) {
        timecode = MIXXX_VINYL_SERATOCV02VINYLSIDEB_XWAX_NAME;
    } else if (strVinylType == MIXXX_VINYL_SERATOCD) {
        timecode = MIXXX_VINYL_SERATOCD_XWAX_NAME;
        m_bCDControl = true;
        // Set up very sensitive steady monitors for CDJs.
        m_pSteadySubtle = new SteadyPitch(0.06, true);
        m_pSteadyGross = new SteadyPitch(0.25, true);
    } else if (strVinylType == MIXXX_VINYL_TRAKTORSCRATCHMK1SIDEA) {
        timecode = MIXXX_VINYL_TRAKTORSCRATCHMK1SIDEA_XWAX_NAME;
    } else if (strVinylType == MIXXX_VINYL_TRAKTORSCRATCHMK1SIDEB) {
        timecode = MIXXX_VINYL_TRAKTORSCRATCHMK1SIDEB_XWAX_NAME;
    } else if (strVinylType == MIXXX_VINYL_TRAKTORSCRATCHMK2SIDEA) {
        timecode = MIXXX_VINYL_TRAKTORSCRATCHMK2SIDEA_XWAX_NAME;
    } else if (strVinylType == MIXXX_VINYL_TRAKTORSCRATCHMK2SIDEB) {
        timecode = MIXXX_VINYL_TRAKTORSCRATCHMK2SIDEB_XWAX_NAME;
    } else if (strVinylType == MIXXX_VINYL_TRAKTORSCRATCHMK2CD) {
        timecode = MIXXX_VINYL_TRAKTORSCRATCHMK2CD_XWAX_NAME;
    } else if (strVinylType == MIXXX_VINYL_MIXVIBESDVS) {
        timecode = MIXXX_VINYL_MIXVIBESDVS_XWAX_NAME;
    } else if (strVinylType == MIXXX_VINYL_MIXVIBES7INCH) {
        timecode = MIXXX_VINYL_MIXVIBES7INCH_XWAX_NAME;
    } else if (strVinylType == MIXXX_VINYL_PIONEERA) {
        timecode = MIXXX_VINYL_PIONEERA_XWAX_NAME;
    } else if (strVinylType == MIXXX_VINYL_PIONEERB) {
        timecode = MIXXX_VINYL_PIONEERB_XWAX_NAME;
    } else if (strVinylType == MIXXX_VINYL_ALGORIDDIMA) {
        timecode = MIXXX_VINYL_ALGORIDDIMA_XWAX_NAME;
    } else if (strVinylType == MIXXX_VINYL_ALGORIDDIMB) {
        timecode = MIXXX_VINYL_ALGORIDDIMB_XWAX_NAME;
    } else {
        qDebug() << "Unknown vinyl type, defaulting to" << MIXXX_VINYL_DEFAULT_XWAX_NAME;
        timecode = MIXXX_VINYL_DEFAULT_XWAX_NAME;
    }

    // If we didn't set up the steady monitors already (not CDJ), do it now.
    if (m_pSteadySubtle == nullptr) {
        m_pSteadySubtle = new SteadyPitch(0.12, false);
    }
    if (m_pSteadyGross == nullptr) {
        m_pSteadyGross = new SteadyPitch(0.5, false);
    }

    // Determine the config folder path
    std::string lut_dir_string;
    const char* lut_dir_path = nullptr;

    if (!getLutDir().isEmpty()) {
        lut_dir_string = getLutDir().toStdString();
        lut_dir_path = lut_dir_string.c_str();
    }

    // Pass the config folder path to the timecoder
    timecode_def* tc_def = timecoder_find_definition(timecode, lut_dir_path);
    if (tc_def == nullptr) {
        qDebug() << "Error finding timecode definition for " << timecode
                 << ", defaulting to" << MIXXX_VINYL_DEFAULT_XWAX_NAME;
        timecode = MIXXX_VINYL_DEFAULT_XWAX_NAME;
        tc_def = timecoder_find_definition(timecode, lut_dir_path);
    }

    double speed = 1.0;
    double rpm = 100.0 / 3.0;
    if (strVinylSpeed == MIXXX_VINYL_SPEED_45) {
        rpm = 45.0;
        speed = 1.35;
    }

    double latency = ControlObject::get(
            ConfigKey(QStringLiteral("[App]"), QStringLiteral("output_latency_ms")));
    if (latency <= 0 || latency > 200) {
        qDebug() << "Failed to get sane latency, assuming 20 as a reasonable value";
        latency = 20;
    }

    const auto sampleRate = static_cast<unsigned int>(ControlObject::get(
            ConfigKey(QStringLiteral("[App]"), QStringLiteral("samplerate"))));

    // Set pitch ring size to 1/4 of one revolution -- a full revolution adds
    // too much stickiness to the pitch.
    m_iPitchRingSize = static_cast<int>(60000 / (rpm * latency * 4));
    m_pPitchRing.resize(m_iPitchRingSize);

    qDebug() << "Xwax Vinyl control starting with a sample rate of:" << sampleRate;
    qDebug() << "Building timecode lookup tables for" << strVinylType << "with speed" << strVinylSpeed;

    // Initialize the timecoder structure. Use the static mutex so that we only
    // do this once across the VinylControlXwax instances.
    s_xwaxLUTMutex.lock();

    const bool use_legacy_pitch_filter = strPitchEstimator == MIXXX_VINYL_PITCH_FILTER_LEGACY;

    timecoder_init(&timecoder,
            tc_def,
            speed,
            sampleRate,
            /* phono */ false,
            use_legacy_pitch_filter);
    timecoder_monitor_init(&timecoder, MIXXX_VINYL_SCOPE_SIZE);
    //Note that timecoder_init will not double-malloc the LUTs, and after this we are guaranteed
    //that the LUT has been generated unless we ran out of memory.
    s_bLUTInitialized = true;
    m_uiSafeZone = timecoder_get_safe(&timecoder);
    //}
    s_xwaxLUTMutex.unlock();

    qDebug() << "Starting vinyl control xwax thread";
}

VinylControlXwax::~VinylControlXwax() {
    delete m_pSteadySubtle;
    delete m_pSteadyGross;

    // Cleanup xwax nicely
    timecoder_monitor_clear(&timecoder);
    timecoder_clear(&timecoder);

    // TODO(rryan): This looks wrong. freeLUTs is called later by
    // VinylControlProcessor so we are probably leaking the LUTs.
    s_bLUTInitialized = false;

    m_pVCRate->set(0.0);
}

//static
void VinylControlXwax::freeLUTs() {
    s_xwaxLUTMutex.lock(); //Static mutex! We don't want two threads doing this!
    if (s_bLUTInitialized) {
        timecoder_free_lookup(); //Frees all the LUTs in xwax.
        s_bLUTInitialized = false;
    }
    s_xwaxLUTMutex.unlock();
}

QString VinylControlXwax::getLutDir() {
    QDir lutPath(m_pConfig->getSettingsPath().append("/lut/"));

    if (!lutPath.exists()) {
        if (!lutPath.mkpath(".")) {
            qWarning() << "Failed to create LUT directory at" << lutPath;
            return QString{};
        }
    }

    return lutPath.absolutePath();
}

bool VinylControlXwax::writeQualityReport(VinylSignalQualityReport* pReport) {
    if (pReport) {
        pReport->timecode_quality = m_fTimecodeQuality;
        pReport->angle = getAngle();
        memcpy(pReport->scope, timecoder.mon, sizeof(pReport->scope));
        return true;
    }
    return false;
}


void VinylControlXwax::analyzeSamples(CSAMPLE* pSamples, size_t nFrames) {
    ScopedTimer t(QStringLiteral("VinylControlXwax::analyzeSamples"));

    submitSamples(pSamples, nFrames);

    const bool bHaveSignal = fabs(pSamples[0]) + fabs(pSamples[1]) > kMinSignal;

    // TODO: Move all these config object get*() calls to an "updatePrefs()"
    // function,
    //         and make that get called when any options get changed in the
    //         preferences dialog, rather than polling every time we get a
    //         buffer.

    // Check if vinyl control is enabled...
    m_bIsEnabled = enabled && checkEnabled(m_bIsEnabled, enabled->toBool());

    double dVinylPitch = timecoder_get_pitch(&timecoder);
    if (bHaveSignal) {
        // Always analyze the input samples
        m_iPosition = timecoder_get_position(&timecoder, nullptr);
        // Notify the UI if the timecode quality is good
        establishQuality(dVinylPitch);
    }

    // are we even playing and enabled at all?
    if (!m_bIsEnabled) {
        return;
    }

    handleTrackChange();

    // make sure m_dVinylPosition only has good values
    if (m_iPosition != -1) {
        m_dVinylPosition = static_cast<double>(m_iPosition) / 1000.0 - m_iLeadInTime;
    }

    // Get the playback position in the file in seconds.
    const double fractionalPos = m_visualPlayPos->getEnginePlayPos();
    const double filePosition = fractionalPos * m_dOldDuration;
    const bool reportedPlayButton = playButton->toBool();

    updateMode(reportedPlayButton);
    updateRecordEndMode(reportedPlayButton, dVinylPitch, filePosition);

    if (m_iVCMode == MIXXX_VCMODE_CONSTANT) {
        // when we enabled constant mode we set the rate slider
        // now we just either set scratch val to 0 (stops playback)
        // or 1 (plays back at that rate)
        m_pVCRate->set(reportedPlayButton ? m_pRateRatio->get() : 0.0);

        return;
    }

    // CONSTANT MODE NO LONGER APPLIES...

    if (!bHaveSignal) {
        handleMissingSignal(filePosition);

        return;
    }

    // When there's a timecode signal available
    // POSITION: MAYBE  PITCH: YES
    const bool keepPlaying = m_iPosition != -1
            ? followTimecodePosition(dVinylPitch, filePosition, fractionalPos)
            : followTimecodePitch(dVinylPitch, filePosition, fractionalPos);
    if (!keepPlaying) {
        return;
    }

    updateRateFromPitch(dVinylPitch, filePosition);
}

void VinylControlXwax::submitSamples(const CSAMPLE* pSamples, size_t nFrames) {
    // We only support amplifying with the VC pre-amp.
    const auto gain = std::max(
            static_cast<CSAMPLE_GAIN>(m_pVinylControlInputGain->get()), 1.0f);
    const size_t samplesSize = nFrames * kChannels;

    if (samplesSize > m_workBufferSize) {
        m_pWorkBuffer.resize(samplesSize);
        m_workBufferSize = samplesSize;
    }

    // Convert CSAMPLE samples to shorts, preventing overflow.
    for (size_t i = 0; i < samplesSize; ++i) {
        const CSAMPLE sample = pSamples[i] * gain * SAMPLE_MAXIMUM;
        m_pWorkBuffer[i] = static_cast<short>(std::clamp<CSAMPLE>(
                sample, SAMPLE_MINIMUM, SAMPLE_MAXIMUM));
    }

    // Submit the samples to the xwax timecode processor. The size argument is
    // in stereo frames.
    timecoder_submit(&timecoder, m_pWorkBuffer.data(), nFrames);
}

void VinylControlXwax::handleTrackChange() {
    // Has a new track been loaded? Currently we use track duration which is
    // integer seconds in the song. However, for calculations we need the
    // higher-accuracy duration found by dividing the track samples by the
    // samplerate.
    // TODO(XXX): we should really sync on all track changes
    // TODO(rryan): Should we calculate the true duration to check if it
    // changed?  It's just an extra division by trackSampleRate.
    const double durationInaccurate = duration->get();
    if (durationInaccurate == m_dOldDurationInaccurate) {
        return;
    }

    m_bForceResync = true;
    m_dOldDurationInaccurate = durationInaccurate;
    m_dOldDuration = trackSamples->get() / 2 / trackSampleRate->get();

    // we were at record end, so turn it off and restore mode
    if (m_bAtRecordEnd) {
        disableRecordEndMode();
        m_iVCMode = m_iOldVCMode == MIXXX_VCMODE_CONSTANT ? MIXXX_VCMODE_RELATIVE : m_iOldVCMode;
    }
}

void VinylControlXwax::updateMode(bool playing) {
    const auto reportedMode = static_cast<int>(mode->get());

    if (m_iVCMode != reportedMode) {
        // if we are playing, don't allow change
        // to absolute mode (would cause sudden track skip)
        if (playing && reportedMode == MIXXX_VCMODE_ABSOLUTE) {
            m_iVCMode = MIXXX_VCMODE_RELATIVE;
            mode->set((double)m_iVCMode);
        } else {
            // go ahead and switch
            m_iVCMode = reportedMode;
            if (reportedMode == MIXXX_VCMODE_ABSOLUTE) {
                m_bForceResync = true;
            }
        }

        // if we are out of error mode...
        if (vinylStatus->get() == VINYL_STATUS_ERROR &&
                m_iVCMode == MIXXX_VCMODE_RELATIVE) {
            vinylStatus->set(VINYL_STATUS_OK);
        }
    }

    // if looping has been enabled, don't allow absolute mode
    if (loopEnabled->toBool() && m_iVCMode == MIXXX_VCMODE_ABSOLUTE) {
        m_iVCMode = MIXXX_VCMODE_RELATIVE;
        mode->set((double)m_iVCMode);
    }

    // Don't allow cueing mode to be enabled in absolute mode.
    if (m_iVCMode == MIXXX_VCMODE_ABSOLUTE &&
            cueing->get() != MIXXX_RELATIVE_CUE_OFF) {
        cueing->set(MIXXX_RELATIVE_CUE_OFF);
    }
}

void VinylControlXwax::updateRecordEndMode(bool playing, double pitch, double filePosition) {
    // are we newly playing near the end of the record?  (in absolute mode, this happens
    // when the filepos is past safe (more accurate),
    // but it can also happen in relative mode if the vinylpos is nearing the end
    // If so, change to constant mode so DJ can move the needle safely
    if (!m_bAtRecordEnd && playing) {
        if (m_iVCMode == MIXXX_VCMODE_ABSOLUTE &&
                (filePosition + m_iLeadInTime) * 1000.0 > m_uiSafeZone &&
                !m_bForceResync) {
            // corner case: we are waiting for resync so don't enable just yet
            enableRecordEndMode();
        } else if (m_iVCMode != MIXXX_VCMODE_ABSOLUTE &&
                m_iPosition != -1 &&
                m_iPosition > static_cast<int>(m_uiSafeZone)) {
            enableRecordEndMode();
        }
    }

    if (!m_bAtRecordEnd) {
        return;
    }

    // if m_bAtRecordEnd was true, maybe it no longer applies:
    if (!playing) {
        // if we turned off play button, also disable
        disableRecordEndMode();
    } else if (m_iPosition != -1 &&
            m_iPosition <= static_cast<int>(m_uiSafeZone) &&
            m_dVinylPosition > 0 &&
            checkSteadyPitch(pitch, filePosition) > 0.5) {
        // if good position, and safe, and not in lead-in, and steady,
        // disable
        disableRecordEndMode();
    }

    if (!m_bAtRecordEnd) {
        return;
    }

    // ok, it's still valid, blink
    if ((playing && (int)(filePosition * 2.0) % 2) ||
            (!playing && (int)(m_iPosition / 500.0) % 2)) {
        vinylStatus->set(VINYL_STATUS_WARNING);
    } else {
        vinylStatus->set(VINYL_STATUS_DISABLED);
    }
}

bool VinylControlXwax::followTimecodePosition(
        double pitch, double filePosition, double fractionalPos) {
    // POSITION: YES  PITCH: YES
    const auto reversed = static_cast<bool>(reverseButton->get());
    if (!reversed && m_bWasReversed) {
        resetSteadyPitch(pitch, m_dVinylPosition);
    }

    m_bWasReversed = reversed;

    // Save the absolute amount of drift for when we need to estimate the vinyl
    // position. This is only called with a valid timecode position, so
    // m_dVinylPosition is up to date here.
    m_dDriftAmt = m_dVinylPosition - filePosition;

    if (m_iVCMode == MIXXX_VCMODE_RELATIVE) {
        m_deltaFilePos = filePosition - m_dOldFilePos;
        m_deltaRelativeDriftAmount = calcDeltaRelativeDriftAmount(m_deltaFilePos);
    }

    const bool resynced = resyncToTimecode(pitch, filePosition);

    if (!resynced && fractionalPos >= 1.0 && pitch > 0) {
        stopAtTrackEnd();
        return false;
    }

    if (!resynced) {
        togglePlayButton(checkSteadyPitch(pitch, filePosition) > 0.5);
    }

    m_dVinylPositionOld = m_dVinylPosition;

    return true;
}

bool VinylControlXwax::resyncToTimecode(double pitch, double filePosition) {
    if (m_bForceResync) {
        // if forceresync was set but we're no longer absolute,
        // it no longer applies
        // if we're in relative mode then we'll do a sync
        // because it might select a cue
        if (m_iVCMode == MIXXX_VCMODE_ABSOLUTE ||
                (m_iVCMode == MIXXX_VCMODE_RELATIVE && cueing->toBool())) {
            syncPosition();
            resetSteadyPitch(pitch, m_dVinylPosition);
        }

        m_bForceResync = false;

        return true;
    }

    if (fabs(m_dDriftAmt) > 0.1 && m_dVinylPosition < -2.0) {
        // It's always a good idea to resync in the lead-in period, because
        // this let's the DJ quickly rewind the track without needing to switch to
        // ABS mode or use a cue point.
        syncPosition();
        resetSteadyPitch(pitch, m_dVinylPosition);
        if (uiUpdateTime(filePosition)) {
            m_pRateRatio->set(fabs(pitch));
        }

        return true;
    }

    const double positionJump = fabs(m_dVinylPosition - m_dVinylPositionOld);

    // CDJs report their position precisely enough to resync on small jumps
    if (const double resyncThreshold = m_bCDControl ? 0.1 : 5.0;
            m_iVCMode == MIXXX_VCMODE_ABSOLUTE && positionJump >= resyncThreshold) {
        // If the position from the timecode is too far off, resync the position.
        syncPosition();
        resetSteadyPitch(pitch, m_dVinylPosition);

        return true;
    }

    return false;
}

bool VinylControlXwax::followTimecodePitch(
        double pitch, double filePosition, double fractionalPos) {
    // POSITION: NO  PITCH: YES
    // if we don't have valid position, we're not playing so reset time to current
    // estimate vinyl position
    if (fractionalPos >= 1.0 && pitch > 0) {
        stopAtTrackEnd();

        return false;
    }

    if (m_iVCMode == MIXXX_VCMODE_ABSOLUTE &&
            fabs(pitch) < 0.05 &&
            fabs(m_dDriftAmt) >= 1.0) {
        // slow, out of sync, syncing position
        syncPosition();
    }

    m_dVinylPositionOld = filePosition + m_dDriftAmt;

    if (pitch > 0.2) {
        togglePlayButton(checkSteadyPitch(pitch, filePosition) > 0.5);
    }

    return true;
}

void VinylControlXwax::stopAtTrackEnd() {
    // end of track, force stop
    togglePlayButton(false);
    resetSteadyPitch(0.0, 0.0);
    m_pVCRate->set(0.0);
    m_iPitchRingPos = 0;
    m_iPitchRingFilled = 0;
}

void VinylControlXwax::updateRateFromPitch(double pitch, double filePosition) {
    // Vinyl rate is exactly what is detected.
    m_pVCRate->set(pitch);

    // playbutton status may have changed
    const bool playing = playButton->toBool();

    updatePitchRing(pitch, playing);

    if (uiUpdateTime(filePosition)) {
        // Don't show extremely high or low speeds in the UI.
        if (playing && !scratching->toBool() && pitch < 1.9 && pitch > 0.2) {
            m_pRateRatio->set(pitch);
        } else {
            m_pRateRatio->set(1.0);
        }
        m_dUiUpdateTime = filePosition;
    }

    m_dOldFilePos = filePosition;
}

void VinylControlXwax::updatePitchRing(double pitch, bool playing) {
    if (!playing) {
        // Reset ring if pitch isn't steady
        m_iPitchRingPos = 0;
        m_iPitchRingFilled = 0;

        return;
    }

    // Only add to the ring if pitch is stable
    m_pPitchRing[m_iPitchRingPos] = pitch;
    if (m_iPitchRingFilled < m_iPitchRingSize) {
        m_iPitchRingFilled++;
    }

    m_iPitchRingPos = (m_iPitchRingPos + 1) % m_iPitchRingSize;
}

void VinylControlXwax::handleMissingSignal(double filePosition) {
    // No pitch data available (the needle is up/stopped.... or *really*
    // crappy signal)

    // POSITION: NO  PITCH: NO
    // if it's been a long time, we're stopped.
    // if it hasn't been long,
    // let the track play a bit more before deciding we've stopped
    m_pRateRatio->set(1.0);

    if (fabs(filePosition - m_dOldFilePos) < 0.3 && filePosition != m_dOldFilePos) {
        return;
    }

    // We are not playing any more
    togglePlayButton(false);
    resetSteadyPitch(0.0, 0.0);
    m_pVCRate->set(0.0);
    resetTimecodeQuality();
    m_bForceResync = true;
    vinylStatus->set(VINYL_STATUS_OK);
}

void VinylControlXwax::resetTimecodeQuality() {
    // Notify the UI that the timecode quality is garbage/missing.
    m_fTimecodeQuality = 0.0f;
    m_dQualityLastPitch = 0.0;
    m_iQualityLastPosition = -1;
    m_iQualityRingIndex = 0;
    m_iQualityRingFilled = 0;
}

// returns the delta between the current drift amount and the last reset value of the drift amount.
double VinylControlXwax::calcDeltaRelativeDriftAmount(double deltaFilePos) {
    // Reset m_relativeDriftAmtMem in case of needle drop, file position change (hotcue, loop etc.),
    // when passthrough is enabled or is playing in reverse
    if (std::fabs(m_deltaRelativeDriftAmount) > 1.5 ||
            std::fabs(deltaFilePos) > 0.03 || // TODO: thresholds to adjust probably
            m_passthroughEnabled.toBool() || reverseButton->toBool() ||
            m_scratchPositionEnabled.toBool()) {
        m_initialRelativeDriftAmt = m_dDriftAmt;
    }

    return m_dDriftAmt - m_initialRelativeDriftAmt;
}

void VinylControlXwax::enableRecordEndMode() {
    qDebug() << "record end, setting constant mode";
    vinylStatus->set(VINYL_STATUS_WARNING);
    enableConstantMode();
    m_bAtRecordEnd = true;
}

void VinylControlXwax::enableConstantMode() {
    m_iOldVCMode = m_iVCMode;
    m_iVCMode = MIXXX_VCMODE_CONSTANT;
    mode->set((double)m_iVCMode);
    togglePlayButton(true);
    double rate = m_pVCRate->get();
    m_pRateRatio->set(fabs(rate));
    m_pVCRate->set(rate);
}

void VinylControlXwax::enableConstantMode(double rate) {
    m_iOldVCMode = m_iVCMode;
    m_iVCMode = MIXXX_VCMODE_CONSTANT;
    mode->set((double)m_iVCMode);
    togglePlayButton(true);
    m_pRateRatio->set(fabs(rate));
    m_pVCRate->set(rate);
}

void VinylControlXwax::disableRecordEndMode() {
    vinylStatus->set(VINYL_STATUS_OK);
    m_bAtRecordEnd = false;
    m_iVCMode = MIXXX_VCMODE_RELATIVE;
    mode->set((double)m_iVCMode);
}

void VinylControlXwax::togglePlayButton(bool on) {
    if (m_bIsEnabled && (playButton->get() > 0) != on) {
        playButton->set((float)on); //and we all float on all right
    }
}

void VinylControlXwax::resetSteadyPitch(double pitch, double time) {
    m_pSteadySubtle->reset(pitch, time);
    m_pSteadyGross->reset(pitch, time);
}

double VinylControlXwax::checkSteadyPitch(double pitch, double time) {
    // If the track is in reverse we can't really know what's going on.
    if (m_bWasReversed) {
        return 0;
    }
    if (m_pSteadyGross->check(pitch, time) < 0.5) {
        scratching->set(1.0);
    } else {
        scratching->set(0.0);
    }
    return m_pSteadySubtle->check(pitch, time);
}

//Synchronize Mixxx's position to the position of the timecoded vinyl.
void VinylControlXwax::syncPosition() {
    //qDebug() << "sync position" << m_dVinylPosition / m_dOldDuration;
    // VinylPos in seconds / total length of song.
    vinylSeek->set(m_dVinylPosition / m_dOldDuration);
}

bool VinylControlXwax::checkEnabled(bool was, bool is) {
    // if we're not enabled, but the last object was, try turning ourselves on
    // XXX: is this just a race that's working right now?
    if (!is && wantenabled->get() > 0) {
        enabled->set(true);
        wantenabled->set(false); //don't try to do this over and over
        return true; //optimism!
    }

    if (was != is) {
        // we reset the scratch value, but we don't reset the rate slider.
        // This means if we are playing, and we disable vinyl control,
        // the track will keep playing at the previous rate.
        // This allows for single-deck control, dj handoffs, etc.

        togglePlayButton(playButton->toBool() || fabs(m_pVCRate->get()) > 0.05);
        m_pVCRate->set(m_pRateRatio->get());
        resetSteadyPitch(0.0, 0.0);
        m_bForceResync = true;
        if (!was) {
            m_dOldFilePos = 0.0;
        }
        m_iVCMode = static_cast<int>(mode->get());
        m_bAtRecordEnd = false;
    }

    if (is && !was) {
        vinylStatus->set(VINYL_STATUS_OK);
    } else if (!is) {
        vinylStatus->set(VINYL_STATUS_DISABLED);
    }

    return is;
}

bool VinylControlXwax::uiUpdateTime(double now) {
    if (m_dUiUpdateTime > now || now - m_dUiUpdateTime > 0.05) {
        m_dUiUpdateTime = now;
        return true;
    }
    return false;
}

int VinylControlXwax::getPositionQuality() {
    int positionQualityPercent;

    if (m_iPosition == -1) {
        // No position code - happens when control vinyl is spinning slow (or during scratching) or the setup is incorrect
        positionQualityPercent = 0;
    } else if ((m_iPosition <= m_iQualityLastPosition - 5) ||
            (m_iPosition >= m_iQualityLastPosition + 5)) {
        // Position code changed not by more than 5. This indicates a normal spinning control vinyl.
        positionQualityPercent = 100;
    } else {
        // Position code changed by more than 5. This indicates a fast spinning control vinyl, a jumping needle or a false signal.
        // This covers also the case of m_iQualityLastPosition == -1
        positionQualityPercent = 50;
    }

    m_iQualityLastPosition = m_iPosition;

    return positionQualityPercent;
}

int VinylControlXwax::getPitchQuality(double& pitch) {
    int pitchQualityPercent;

    if (m_iQualityRingFilled == 0) {
        m_dQualityLastPitch = pitch;
        pitchQualityPercent = 0;
        return pitchQualityPercent; // First value - but at least two values needed for calculation
    }

    double pitchDifference = pitch - m_dQualityLastPitch;
    m_dQualityLastPitch = pitch;

    if (pitchDifference != 0) {
        double pitchStability = std::fabs(pitch / pitchDifference);

        if (pitchStability < 3.0) {
            // Unlikely that this pitch difference is from a proper set up control vinyl
            pitchQualityPercent = 0;
        } else if (pitchStability > 6.0) {
            // Typical pitch difference for normal spinning control vinyl
            pitchQualityPercent = 100;
        } else {
            // Weak pitch difference for a control vinyl slow spinning ,or for scratching
            pitchQualityPercent = 75;
        }
    } else {
        // Unlikely case of two identical float values for pitch
        pitchQualityPercent = 0;
    }
    return pitchQualityPercent;
}

void VinylControlXwax::establishQuality(double& pitch) {
    m_iQualityRing[m_iQualityRingIndex] = getPositionQuality() + getPitchQuality(pitch);
    m_fTimecodeQuality = std::max<float>(0.0,
            std::min<float>(1.0,
                    static_cast<float>(std::accumulate(m_iQualityRing,
                            m_iQualityRing + m_iQualityRingFilled,
                            0)) /
                            2.0f / 100.0f /
                            static_cast<float>(
                                    m_iQualityRingFilled))); // Two information in percent per datapoint

    if (m_iQualityRingFilled < QUALITY_RING_SIZE) {
        m_iQualityRingFilled++;
    }

    m_iQualityRingIndex = (m_iQualityRingIndex + 1) % QUALITY_RING_SIZE;
}

float VinylControlXwax::getAngle() {
    float pos = timecoder_get_position(&timecoder, nullptr);

    if (pos == -1) {
        return -1.0;
    }

    const auto rps = static_cast<float>(timecoder_revs_per_sec(&timecoder));
    // Invert angle to make vinyl spin direction correct.
    return 360 - (static_cast<int>(pos / 1000.0f * 360.0f * rps) % 360);
}
