/***************************************************************************
                          vinylcontrolxwax.cpp
                             -------------------
    begin                : Sometime in Summer 2007
    copyright            : (C) 2007 Albert Santoni
                           (C) 2007 Mark Hills
                           (C) 2011 Owen Williams
                           Portions of xwax used under the terms of the GPL
    current maintainer   : Owen Williams
    email                : owilliams@mixxx.org
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QtDebug>
#include <limits.h>

#include "vinylcontrol/vinylcontrolxwax.h"
#include "util/timer.h"
#include "control/controlproxy.h"
#include "control/controlobject.h"
#include "util/math.h"
#include "util/defs.h"

/****** TODO *******
   Stuff to maybe implement here
   1) The smoothing thing that xwax does
   2) Tons of cleanup
   3) Speed up needle dropping
   4) Extrapolate small dropouts and keep track of "dynamics"

 ********************/

// Sample threshold below which we consider there to be no signal.
const double kMinSignal = 75.0 / SAMPLE_MAX;

bool VinylControlXwax::s_bLUTInitialized = false;
QMutex VinylControlXwax::s_xwaxLUTMutex;

VinylControlXwax::VinylControlXwax(UserSettingsPointer pConfig, QString group)
        : VinylControl(pConfig, group),
          m_dVinylPositionOld(0.0),
          m_pWorkBuffer(new short[MAX_BUFFER_LEN]),
          m_workBufferSize(MAX_BUFFER_LEN),
          m_iQualPos(0),
          m_iQualFilled(0),
          m_iPosition(-1),
          m_bAtRecordEnd(false),
          m_bForceResync(false),
          m_iVCMode(mode->get()),
          m_iOldVCMode(MIXXX_VCMODE_ABSOLUTE),
          m_dOldFilePos(0.0),
          m_dOldDuration(0.0),
          m_dOldDurationInaccurate(-1.0),
          m_bWasReversed(false),
          m_pPitchRing(NULL),
          m_iPitchRingSize(0),
          m_iPitchRingPos(0),
          m_iPitchRingFilled(0),
          m_dDisplayPitch(0.0),
          m_pSteadySubtle(NULL),
          m_pSteadyGross(NULL),
          m_bCDControl(false),
          m_bTrackSelectMode(false),
          m_pControlTrackSelector(NULL),
          m_pControlTrackLoader(NULL),
          m_dLastTrackSelectPos(0.0),
          m_dCurTrackSelectPos(0.0),
          m_dDriftAmt(0.0),
          m_dUiUpdateTime(-1.0) {
    // TODO(rryan): Should probably live in VinylControlManager since it's not
    // specific to a VC deck.
    signalenabled->slotSet(m_pConfig->getValueString(
        ConfigKey(VINYL_PREF_KEY, "show_signal_quality")).toInt());

    // Get the vinyl type and speed.
    QString strVinylType = m_pConfig->getValueString(
        ConfigKey(group,"vinylcontrol_vinyl_type"));
    QString strVinylSpeed = m_pConfig->getValueString(
        ConfigKey(group,"vinylcontrol_speed_type"));

    // libxwax indexes by C-strings so we pass libxwax string literals so we
    // don't have to deal with freeing the strings later
    char* timecode = NULL;


    if (strVinylType == MIXXX_VINYL_SERATOCV02VINYLSIDEA) {
        timecode = (char*)"serato_2a";
    }
    else if (strVinylType == MIXXX_VINYL_SERATOCV02VINYLSIDEB) {
        timecode = (char*)"serato_2b";
    } else if (strVinylType == MIXXX_VINYL_SERATOCD) {
        timecode = (char*)"serato_cd";
        m_bCDControl = true;
        // Set up very sensitive steady monitors for CDJs.
        m_pSteadySubtle = new SteadyPitch(0.06, true);
        m_pSteadyGross = new SteadyPitch(0.25, true);
    } else if (strVinylType == MIXXX_VINYL_TRAKTORSCRATCHSIDEA) {
        timecode = (char*)"traktor_a";
    } else if (strVinylType == MIXXX_VINYL_TRAKTORSCRATCHSIDEB) {
        timecode = (char*)"traktor_b";
    } else if (strVinylType == MIXXX_VINYL_MIXVIBESDVS) {
        timecode = (char*)"mixvibes_v2";
    } else {
        qDebug() << "Unknown vinyl type, defaulting to serato_2a";
        timecode = (char*)"serato_2a";
    }

    // If we didn't set up the steady monitors already (not CDJ), do it now.
    if (m_pSteadySubtle == NULL) {
        m_pSteadySubtle = new SteadyPitch(0.12, false);
    }
    if (m_pSteadyGross == NULL) {
        m_pSteadyGross = new SteadyPitch(0.5, false);
    }


    timecode_def* tc_def = timecoder_find_definition(timecode);
    if (tc_def == NULL) {
        qDebug() << "Error finding timecode definition for " << timecode << ", defaulting to serato_2a";
        timecode = (char*)"serato_2a";
        tc_def = timecoder_find_definition(timecode);
    }

    double speed = 1.0;
    double rpm = 100.0 / 3.0;
    if (strVinylSpeed == MIXXX_VINYL_SPEED_45) {
        rpm = 45.0;
        speed = 1.35;
    }

    double latency = ControlObject::get(
            ConfigKey("[Master]", "latency"));
    if (latency <= 0 || latency > 200) {
        qDebug() << "Failed to get sane latency, assuming 20 as a reasonable value";
        latency = 20;
    }

    int iSampleRate = m_pConfig->getValueString(
        ConfigKey("[Soundcard]","Samplerate")).toULong();

    // Set pitch ring size to 1/4 of one revolution -- a full revolution adds
    // too much stickiness to the pitch.
    m_iPitchRingSize = static_cast<int>(60000 / (rpm * latency * 4));
    m_pPitchRing = new double[m_iPitchRingSize];

    qDebug() << "Xwax Vinyl control starting with a sample rate of:" << iSampleRate;
    qDebug() << "Building timecode lookup tables for" << strVinylType << "with speed" << strVinylSpeed;

    // Initialize the timecoder structure. Use the static mutex so that we only
    // do this once across the VinylControlXwax instances.
    s_xwaxLUTMutex.lock();

    timecoder_init(&timecoder, tc_def, speed, iSampleRate, /* phono */ false);
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
    delete [] m_pPitchRing;
    delete [] m_pWorkBuffer;

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
    ScopedTimer t("VinylControlXwax::analyzeSamples");
    CSAMPLE gain = m_pVinylControlInputGain->get();
    const int kChannels = 2;

    // We only support amplifying with the VC pre-amp.
    if (gain < 1.0f) {
        gain = 1.0f;
    }

    size_t samplesSize = nFrames * kChannels;

    if (samplesSize > m_workBufferSize) {
        delete [] m_pWorkBuffer;
        m_pWorkBuffer = new short[samplesSize];
        m_workBufferSize = samplesSize;
    }

    // Convert CSAMPLE samples to shorts, preventing overflow.
    for (int i = 0; i < static_cast<int>(samplesSize); ++i) {
        CSAMPLE sample = pSamples[i] * gain * SAMPLE_MAX;

        if (sample > SAMPLE_MAX) {
            m_pWorkBuffer[i] = SAMPLE_MAX;
        } else if (sample < SAMPLE_MIN) {
            m_pWorkBuffer[i] = SAMPLE_MIN;
        } else {
            m_pWorkBuffer[i] = static_cast<short>(sample);
        }
    }

    // Submit the samples to the xwax timecode processor. The size argument is
    // in stereo frames.
    timecoder_submit(&timecoder, m_pWorkBuffer, nFrames);

    bool bHaveSignal = fabs(pSamples[0]) + fabs(pSamples[1]) > kMinSignal;
    //qDebug() << "signal?" << bHaveSignal;

    //TODO: Move all these config object get*() calls to an "updatePrefs()" function,
    //        and make that get called when any options get changed in the preferences dialog, rather than
    //        polling everytime we get a buffer.


    // Check if vinyl control is enabled...
    m_bIsEnabled = enabled == NULL ? false : checkEnabled(m_bIsEnabled, enabled->get());

    if(bHaveSignal) {
        // Always analyze the input samples
        m_iPosition = timecoder_get_position(&timecoder, NULL);
        //Notify the UI if the timecode quality is good
        establishQuality(m_iPosition != -1);
    }

    //are we even playing and enabled at all?
    if (!m_bIsEnabled)
        return;

    double dVinylPitch = timecoder_get_pitch(&timecoder);

    // Has a new track been loaded? Currently we use track duration which is
    // integer seconds in the song. However, for calculations we need the
    // higher-accuracy duration found by dividing the track samples by the
    // samplerate.
    // TODO(XXX): we should really sync on all track changes
    // TODO(rryan): Should we calculate the true duration to check if it
    // changed?  It's just an extra division by trackSampleRate.
    double duration_inaccurate = duration->get();
    if (duration_inaccurate != m_dOldDurationInaccurate) {
        m_bForceResync = true;
        m_bTrackSelectMode = false; //just in case
        m_dOldDurationInaccurate = duration_inaccurate;
        m_dOldDuration = trackSamples->get() / 2 / trackSampleRate->get();

        // we were at record end, so turn it off and restore mode
        if(m_bAtRecordEnd) {
            disableRecordEndMode();
            if (m_iOldVCMode == MIXXX_VCMODE_CONSTANT) {
                m_iVCMode = MIXXX_VCMODE_RELATIVE;
            } else {
                m_iVCMode = m_iOldVCMode;
            }
        }
    }

    // make sure m_dVinylPosition only has good values
    if (m_iPosition != -1) {
        m_dVinylPosition = static_cast<double>(m_iPosition) / 1000.0 - m_iLeadInTime;
    }

    // Initialize drift control to zero in case we don't get any position data
    // to calculate it with.
    double dDriftControl = 0.0;

    // Get the playback position in the file in seconds.
    double filePosition = playPos->get() * m_dOldDuration;

    int reportedMode = mode->get();
    bool reportedPlayButton = playButton->get();

    if (m_iVCMode != reportedMode) {
        //if we are playing, don't allow change
        //to absolute mode (would cause sudden track skip)
        if (reportedPlayButton && reportedMode == MIXXX_VCMODE_ABSOLUTE) {
            m_iVCMode = MIXXX_VCMODE_RELATIVE;
            mode->slotSet((double)m_iVCMode);
        } else {
            // go ahead and switch
            m_iVCMode = reportedMode;
            if (reportedMode == MIXXX_VCMODE_ABSOLUTE) {
                m_bForceResync = true;
            }
        }

        //if we are out of error mode...
        if (vinylStatus->get() == VINYL_STATUS_ERROR &&
                m_iVCMode == MIXXX_VCMODE_RELATIVE) {
            vinylStatus->slotSet(VINYL_STATUS_OK);
        }
    }

    //if looping has been enabled, don't allow absolute mode
    if (loopEnabled->get() && m_iVCMode == MIXXX_VCMODE_ABSOLUTE) {
        m_iVCMode = MIXXX_VCMODE_RELATIVE;
        mode->slotSet((double)m_iVCMode);
    }

    // Don't allow cueing mode to be enabled in absolute mode.
    if (m_iVCMode == MIXXX_VCMODE_ABSOLUTE &&
            cueing->get() != MIXXX_RELATIVE_CUE_OFF) {
        cueing->set(MIXXX_RELATIVE_CUE_OFF);
    }

    //are we newly playing near the end of the record?  (in absolute mode, this happens
    //when the filepos is past safe (more accurate),
    //but it can also happen in relative mode if the vinylpos is nearing the end
    //If so, change to constant mode so DJ can move the needle safely

    if (!m_bAtRecordEnd && reportedPlayButton) {
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

    if (m_bAtRecordEnd) {
        //if m_bAtRecordEnd was true, maybe it no longer applies:

        if (!reportedPlayButton) {
            //if we turned off play button, also disable
            disableRecordEndMode();
        } else if (m_iPosition != -1 &&
                   m_iPosition <= static_cast<int>(m_uiSafeZone) &&
                   m_dVinylPosition > 0 &&
                   checkSteadyPitch(dVinylPitch, filePosition) > 0.5) {
            //if good position, and safe, and not in leadin, and steady,
            //disable
            disableRecordEndMode();
        }

        if (m_bAtRecordEnd) {
            //ok, it's still valid, blink
            if ((reportedPlayButton && (int)(filePosition * 2.0) % 2) ||
                (!reportedPlayButton && (int)(m_iPosition / 500.0) % 2))
                vinylStatus->slotSet(VINYL_STATUS_WARNING);
            else
                vinylStatus->slotSet(VINYL_STATUS_DISABLED);
        }
    }

    //check here for position > safe, and if no record end mode,
    //then trigger track selection mode.  just pass position to it
    //and ignore pitch

    if (!m_bAtRecordEnd) {
        if (m_iPosition != -1 && m_iPosition > static_cast<int>(m_uiSafeZone)) {
            //only enable if pitch is steady, though.  Heavy scratching can
            //produce crazy results and trigger this mode
            if (m_bTrackSelectMode || checkSteadyPitch(dVinylPitch, filePosition) > 0.1) {
                //until I can figure out how to detect "track 2" on serato CD,
                //don't try track selection
                if (!m_bCDControl) {
                    if (!m_bTrackSelectMode) {
                        qDebug() << "position greater than safe, select mode" << m_iPosition << m_uiSafeZone;
                        m_bTrackSelectMode = true;
                        togglePlayButton(false);
                        resetSteadyPitch(0.0, 0.0);
                        m_pVCRate->set(0.0);
                    }
                    doTrackSelection(true, dVinylPitch, m_iPosition);
                }

                //hm I wonder if track will keep playing while this happens?
                //not sure what we want to do here...  probably enforce
                //stopped deck.

                //but if constant mode...  nah, force stop.
                return;
            }
            //if it's not steady yet we process as normal
        } else {
            //so we're not unsafe.... but
            //if no position, but we were in select mode, do select mode
            if (m_iPosition == -1 && m_bTrackSelectMode) {
                //qDebug() << "no position, but were in select mode";
                doTrackSelection(false, dVinylPitch, m_iPosition);

                //again, force stop?
                return;
            } else if (m_bTrackSelectMode) {
                //qDebug() << "discontinuing select mode, selecting track";
                if (m_pControlTrackLoader == NULL) {
                    m_pControlTrackLoader = new ControlProxy(
                            m_group, "LoadSelectedTrack", this);
                }

                m_pControlTrackLoader->slotSet(1.0);
                m_pControlTrackLoader->slotSet(0.0); // I think I have to do this...

                // if position is known and safe then no track select mode
                m_bTrackSelectMode = false;
            }
        }
    }

    if (m_iVCMode == MIXXX_VCMODE_CONSTANT) {
        // when we enabled constant mode we set the rate slider
        // now we just either set scratch val to 0 (stops playback)
        // or 1 (plays back at that rate)

        double newScratch = reportedPlayButton ? calcRateRatio() : 0.0;
        m_pVCRate->set(newScratch);

        // is there any reason we'd need to do anything else?
        return;
    }

    // CONSTANT MODE NO LONGER APPLIES...

    // When there's a timecode signal available
    // This is set when we analyze samples (no need for lock I think)
    if(bHaveSignal) {
        //POSITION: MAYBE  PITCH: YES

        //We have pitch, but not position.  so okay signal but not great (scratching / cueing?)
        //qDebug() << "Pitch" << dVinylPitch;

        if (m_iPosition != -1) {
            //POSITION: YES  PITCH: YES
            //add a value to the pitch ring (for averaging / smoothing the pitch)
            //qDebug() << fabs(((m_dVinylPosition - m_dVinylPositionOld) * (dVinylPitch / fabs(dVinylPitch))));

            bool reversed = static_cast<bool>(reverseButton->get());
            if (!reversed && m_bWasReversed) {
                resetSteadyPitch(dVinylPitch, m_dVinylPosition);
            }
            m_bWasReversed = reversed;

            //save the absolute amount of drift for when we need to estimate vinyl position
            m_dDriftAmt = m_dVinylPosition - filePosition;

            //qDebug() << "drift" << m_dDriftAmt;

            if (m_bForceResync) {
                //if forceresync was set but we're no longer absolute,
                //it no longer applies
                //if we're in relative mode then we'll do a sync
                //because it might select a cue
                if (m_iVCMode == MIXXX_VCMODE_ABSOLUTE ||
                        (m_iVCMode == MIXXX_VCMODE_RELATIVE && cueing->get())) {
                    syncPosition();
                    resetSteadyPitch(dVinylPitch, m_dVinylPosition);
                }
                m_bForceResync = false;
            } else if (fabs(m_dVinylPosition - filePosition) > 0.1 &&
                       m_dVinylPosition < -2.0) {
                //At first I thought it was a bug to resync to leadin in relative mode,
                //but after using it that way it's actually pretty convenient.
                //qDebug() << "Vinyl leadin";
                syncPosition();
                resetSteadyPitch(dVinylPitch, m_dVinylPosition);
                if (uiUpdateTime(filePosition)) {
                    m_pRateSlider->set(m_pRateDir->get() * (fabs(dVinylPitch) - 1.0) / m_pRateRange->get());
                }
            } else if (m_iVCMode == MIXXX_VCMODE_ABSOLUTE &&
                       (fabs(m_dVinylPosition - m_dVinylPositionOld) >= 5.0)) {
                //If the position from the timecode is more than a few seconds off, resync the position.
                //qDebug() << "resync position (>15.0 sec)";
                //qDebug() << m_dVinylPosition << m_dVinylPositionOld << m_dVinylPosition - m_dVinylPositionOld;
                syncPosition();
                resetSteadyPitch(dVinylPitch, m_dVinylPosition);
            } else if (m_iVCMode == MIXXX_VCMODE_ABSOLUTE && m_bCDControl &&
                       fabs(m_dVinylPosition - m_dVinylPositionOld) >= 0.1) {
                //qDebug() << "CDJ resync position (>0.1 sec)";
                syncPosition();
                resetSteadyPitch(dVinylPitch, m_dVinylPosition);
            } else if (playPos->get() >= 1.0 && dVinylPitch > 0) {
                //end of track, force stop
                togglePlayButton(false);
                resetSteadyPitch(0.0, 0.0);
                m_pVCRate->set(0.0);
                m_iPitchRingPos = 0;
                m_iPitchRingFilled = 0;
                return;
            } else {
                togglePlayButton(checkSteadyPitch(dVinylPitch, filePosition) > 0.5);
            }

            // Calculate how much the vinyl's position has drifted from it's timecode and compensate for it.
            // (This is caused by the manufacturing process of the vinyl.)
            if (m_iVCMode == MIXXX_VCMODE_ABSOLUTE &&
                    fabs(m_dDriftAmt) > 0.1 && fabs(m_dDriftAmt) < 5.0) {
                dDriftControl = m_dDriftAmt * .01;
            } else {
                dDriftControl = 0.0;
            }

            m_dVinylPositionOld = m_dVinylPosition;
        } else {
            //POSITION: NO  PITCH: YES
            //if we don't have valid position, we're not playing so reset time to current
            //estimate vinyl position

            if (playPos->get() >= 1.0 && dVinylPitch > 0) {
                //end of track, force stop
                togglePlayButton(false);
                resetSteadyPitch(0.0, 0.0);
                m_pVCRate->set(0.0);
                m_iPitchRingPos = 0;
                m_iPitchRingFilled = 0;
                return;
            }

            if (m_iVCMode == MIXXX_VCMODE_ABSOLUTE &&
                    fabs(dVinylPitch) < 0.05 &&
                    fabs(m_dDriftAmt) >= 0.3) {
                //qDebug() << "slow, out of sync, syncing position";
                syncPosition();
            }

            m_dVinylPositionOld = filePosition + m_dDriftAmt;

            if (dVinylPitch > 0.2) {
                togglePlayButton(checkSteadyPitch(dVinylPitch, filePosition) > 0.5);
            }
        }

        //playbutton status may have changed
        reportedPlayButton = playButton->get();

        if (reportedPlayButton) {
            // Only add to the ring if pitch is stable
            m_pPitchRing[m_iPitchRingPos] = dVinylPitch;
            if (m_iPitchRingFilled < m_iPitchRingSize) {
                m_iPitchRingFilled++;
            }
            m_iPitchRingPos = (m_iPitchRingPos + 1) % m_iPitchRingSize;
        } else {
            // Reset ring if pitch isn't steady
            m_iPitchRingPos = 0;
            m_iPitchRingFilled = 0;
        }

        //only smooth when we have good position (no smoothing for scratching)
        double averagePitch = 0.0;
        if (m_iPosition != -1 && reportedPlayButton) {
            for (int i = 0; i < m_iPitchRingFilled; ++i) {
                averagePitch += m_pPitchRing[i];
            }
            averagePitch /= m_iPitchRingFilled;
            // Round out some of the noise
            averagePitch = round(averagePitch * 10000.0);
            averagePitch /= 10000.0;
        } else {
            averagePitch = dVinylPitch;
        }

        m_pVCRate->set(averagePitch + dDriftControl);
        if (uiUpdateTime(filePosition)) {
            double true_pitch = averagePitch + dDriftControl;
            double pitch_difference = true_pitch - m_dDisplayPitch;

            // The true pitch can show a misleading amount of variance --
            // differences of .1% or less can show up as 1 or 2 bpm changes.
            // Therefore we react slowly to bpm changes to show a more steady
            // number to the user.
            if (fabs(pitch_difference) > 0.5) {
                // For large changes in pitch (start/stop, usually), immediately
                // update the display.
                m_dDisplayPitch = true_pitch;
            } else if (fabs(pitch_difference) > 0.005) {
                // For medium changes in pitch, take 4 callback loops to
                // converge on the correct amount.
                m_dDisplayPitch += pitch_difference * .25;
            } else {
                // For extremely small changes, converge very slowly.
                m_dDisplayPitch += pitch_difference * .01;
            }
            // Don't show extremely high or low speeds in the UI.
            if (reportedPlayButton && !scratching->get() &&
                    m_dDisplayPitch < 1.9 && m_dDisplayPitch > 0.2) {
                m_pRateSlider->set(m_pRateDir->get() *
                                   (m_dDisplayPitch - 1.0) / m_pRateRange->get());
            } else {
                m_pRateSlider->set(0.0);
            }
            m_dUiUpdateTime = filePosition;
        }

        m_dOldFilePos = filePosition;
    } else {
        // No pitch data available (the needle is up/stopped.... or *really*
        // crappy signal)

        //POSITION: NO  PITCH: NO
        //if it's been a long time, we're stopped.
        //if it hasn't been long,
        //let the track play a wee bit more before deciding we've stopped

        m_pRateSlider->set(0.0);

        if (m_iVCMode == MIXXX_VCMODE_ABSOLUTE &&
                fabs(m_dVinylPosition - filePosition) >= 0.1) {
            //qDebug() << "stopped, out of sync, syncing position";
            syncPosition();
        }

        if(fabs(filePosition - m_dOldFilePos) >= 0.1 ||
               filePosition == m_dOldFilePos) {
            //We are not playing any more
            togglePlayButton(false);
            resetSteadyPitch(0.0, 0.0);
            m_pVCRate->set(0.0);
            //resetSteadyPitch(dVinylPitch, filePosition);
            // Notify the UI that the timecode quality is garbage/missing.
            m_fTimecodeQuality = 0.0f;
            m_iPitchRingPos = 0;
            m_iPitchRingFilled = 0;
            m_iQualPos = 0;
            m_iQualFilled = 0;
            m_bForceResync = true;
            vinylStatus->slotSet(VINYL_STATUS_OK);
        }
    }
}

void VinylControlXwax::enableRecordEndMode() {
    qDebug() << "record end, setting constant mode";
    vinylStatus->slotSet(VINYL_STATUS_WARNING);
    enableConstantMode();
    m_bAtRecordEnd = true;
}

void VinylControlXwax::enableConstantMode() {
    m_iOldVCMode = m_iVCMode;
    m_iVCMode = MIXXX_VCMODE_CONSTANT;
    mode->slotSet((double)m_iVCMode);
    togglePlayButton(true);
    double rate = m_pVCRate->get();
    m_pRateSlider->set(m_pRateDir->get() * (fabs(rate) - 1.0) / m_pRateRange->get());
    m_pVCRate->set(rate);
}

void VinylControlXwax::enableConstantMode(double rate) {
    m_iOldVCMode = m_iVCMode;
    m_iVCMode = MIXXX_VCMODE_CONSTANT;
    mode->slotSet((double)m_iVCMode);
    togglePlayButton(true);
    m_pRateSlider->set(m_pRateDir->get() * (fabs(rate) - 1.0) / m_pRateRange->get());
    m_pVCRate->set(rate);
}

void VinylControlXwax::disableRecordEndMode() {
    vinylStatus->slotSet(VINYL_STATUS_OK);
    m_bAtRecordEnd = false;
    m_iVCMode = MIXXX_VCMODE_RELATIVE;
    mode->slotSet((double)m_iVCMode);
}

void VinylControlXwax::togglePlayButton(bool on) {
    if (m_bIsEnabled && (playButton->get() > 0) != on) {
        playButton->slotSet((float)on);  //and we all float on all right
    }
}

void VinylControlXwax::doTrackSelection(bool valid_pos, double pitch, double position) {
    //compare positions, fabricating if we don't have position data, and
    //move the selector every so often
    //track will be selected when the needle is moved back to play area
    //track selection can be cancelled by loading a track manually

    const int SELECT_INTERVAL = 150;
    const double NOPOS_SPEED = 0.50;

    if (m_pControlTrackSelector == NULL) {
        // this isn't done in the constructor because this object
        // doesn't seem to be created yet
        m_pControlTrackSelector = new ControlProxy(
                "[Playlist]","SelectTrackKnob", this);
    }

    if (!valid_pos) {
        if (fabs(pitch) > 0.1) {
            //how to estimate how far the record has moved when we don't have a valid
            //position and no mp3 track to compare with???  just add a bullshit amount?
            m_dCurTrackSelectPos += pitch * NOPOS_SPEED; //MADE UP CONSTANT, needs to be based on frames per second I think
        } else {
            // too slow, do nothing
            return;
        }
    } else {
        // if we have valid pos, use it
        m_dCurTrackSelectPos = position;
    }


    //we have position or at least record is moving, so check if we should
    //change location

    if (fabs(m_dCurTrackSelectPos - m_dLastTrackSelectPos) > 10.0 * 1000) {
        //yeah probably not a valid value
        //qDebug() << "large change in track position, resetting";
        m_dLastTrackSelectPos = m_dCurTrackSelectPos;
    } else if (fabs(m_dCurTrackSelectPos - m_dLastTrackSelectPos) > SELECT_INTERVAL) {
        //only adjust by one at a time.  It's no help jumping around
        m_pControlTrackSelector->set((int)(m_dCurTrackSelectPos - m_dLastTrackSelectPos) / fabs(m_dCurTrackSelectPos - m_dLastTrackSelectPos));
        m_dLastTrackSelectPos = m_dCurTrackSelectPos;
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
        scratching->slotSet(1.0);
    } else {
        scratching->slotSet(0.0);
    }
    return m_pSteadySubtle->check(pitch, time);
}

//Synchronize Mixxx's position to the position of the timecoded vinyl.
void VinylControlXwax::syncPosition() {
    //qDebug() << "sync position" << m_dVinylPosition / m_dOldDuration;
    // VinylPos in seconds / total length of song.
    vinylSeek->slotSet(m_dVinylPosition / m_dOldDuration);
}

bool VinylControlXwax::checkEnabled(bool was, bool is) {
    // if we're not enabled, but the last object was, try turning ourselves on
    // XXX: is this just a race that's working right now?
    if (!is && wantenabled->get() > 0) {
        enabled->slotSet(true);
        wantenabled->slotSet(false); //don't try to do this over and over
        return true; //optimism!
    }

    if (was != is) {
        //we reset the scratch value, but we don't reset the rate slider.
        //This means if we are playing, and we disable vinyl control,
        //the track will keep playing at the previous rate.
        //This allows for single-deck control, dj handoffs, etc.

        togglePlayButton(playButton->get() || fabs(m_pVCRate->get()) > 0.05);
        m_pVCRate->set(calcRateRatio());
        resetSteadyPitch(0.0, 0.0);
        m_bForceResync = true;
        if (!was)
            m_dOldFilePos = 0.0;
        m_iVCMode = mode->get();
        m_bAtRecordEnd = false;
    }

    if (is && !was) {
        vinylStatus->slotSet(VINYL_STATUS_OK);
    } else if (!is) {
        vinylStatus->slotSet(VINYL_STATUS_DISABLED);
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

void VinylControlXwax::establishQuality(bool quality_sample) {
    m_bQualityRing[m_iQualPos] = quality_sample;
    if (m_iQualFilled < QUALITY_RING_SIZE) {
        m_iQualFilled++;
    }

    int quality = 0;
    for (int i = 0; i < m_iQualFilled; ++i) {
        if (m_bQualityRing[i])
            quality++;
    }

    m_fTimecodeQuality = static_cast<float>(quality) /
            static_cast<float>(m_iQualFilled);
    m_iQualPos = (m_iQualPos + 1) % QUALITY_RING_SIZE;
}

float VinylControlXwax::getAngle() {
    float pos = timecoder_get_position(&timecoder, NULL);

    if (pos == -1) {
        return -1.0;
    }

    float rps = timecoder_revs_per_sec(&timecoder);
    // Invert angle to make vinyl spin direction correct.
    return 360 - (static_cast<int>(pos / 1000.0 * 360.0 * rps) % 360);
}

double VinylControlXwax::calcRateRatio() const {
    double rateRatio = 1.0 + m_pRateDir->get() * m_pRateRange->get() *
            m_pRateSlider->get();
    return rateRatio;
}
