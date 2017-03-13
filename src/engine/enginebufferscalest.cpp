/***************************************************************************
                          enginebufferscalest.cpp  -  description
                             -------------------
    begin                : November 2004
    copyright            : (C) 2004 by Tue Haste Andersen
    email                : haste@diku.dk
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "engine/enginebufferscalest.h"

// Fixes redefinition warnings from SoundTouch.
#undef TRUE
#undef FALSE
#include <SoundTouch.h>

#include "controlobject.h"
#include "engine/readaheadmanager.h"
#include "engine/engineobject.h"
#include "track/keyutils.h"
#include "sampleutil.h"
#include "util/math.h"

using namespace soundtouch;

EngineBufferScaleST::EngineBufferScaleST(ReadAheadManager *pReadAheadManager)
    : EngineBufferScale(),
      m_bBackwards(false),
      m_dRateOld(1.0),
      m_dTempoOld(1.0),
      m_pReadAheadManager(pReadAheadManager) {
    m_pSoundTouch = new soundtouch::SoundTouch();
    m_pSoundTouch->setChannels(2);
    m_pSoundTouch->setRate(m_dRateOld);
    m_pSoundTouch->setTempo(m_dTempoOld);
    m_pSoundTouch->setPitch(1.0);
    m_pSoundTouch->setSetting(SETTING_USE_QUICKSEEK, 1);
    m_pSoundTouch->setSampleRate(m_iSampleRate > 0 ? m_iSampleRate : 44100);

    buffer_back = new CSAMPLE[kiSoundTouchReadAheadLength*2];
}

EngineBufferScaleST::~EngineBufferScaleST() {
    delete m_pSoundTouch;
    delete [] buffer_back;
}

void EngineBufferScaleST::setScaleParameters(int iSampleRate,
                                             double base_rate,
                                             bool speed_affects_pitch,
                                             double* speed_adjust,
                                             double* pitch_adjust) {
    if (m_iSampleRate != iSampleRate) {
        m_pSoundTouch->setSampleRate(iSampleRate > 0 ? iSampleRate : 44100);
        m_iSampleRate = iSampleRate;
    }

    // Negative speed means we are going backwards. pitch_adjust does not affect
    // the playback direction.
    m_bBackwards = *speed_adjust < 0;

    // It's an error to pass a rate or tempo smaller than MIN_SEEK_SPEED to
    // SoundTouch (see definition of MIN_SEEK_SPEED for more details).
    double speed_abs = fabs(*speed_adjust);
    if (speed_abs > MAX_SEEK_SPEED) {
        speed_abs = MAX_SEEK_SPEED;
    } else if (speed_abs < MIN_SEEK_SPEED) {
        speed_abs = 0;
    }

    // Let the caller know if we clamped their value.
    *speed_adjust = m_bBackwards ? -speed_abs : speed_abs;

    // Include baserate in rate_abs so that we do samplerate conversion as part
    // of rate adjustment.
    double rate_abs = base_rate;
    double tempo_abs = 1.0;
    if (speed_affects_pitch) {
        rate_abs *= speed_abs;
    } else {
        tempo_abs *= speed_abs;
    }

    // Note that we do not set the tempo if it is zero. This is because of the
    // above clamping which prevents us from going below MIN_SEEK_SPEED. I think
    // we should handle this better but I have left the logic in place. rryan
    // 4/2013.
    if (tempo_abs != m_dTempoOld && tempo_abs != 0.0) {
        m_pSoundTouch->setTempo(tempo_abs);
        m_dTempoOld = tempo_abs;
    }
    if (rate_abs != m_dRateOld) {
        m_pSoundTouch->setRate(rate_abs);
        m_dRateOld = rate_abs;
    }
    if (*pitch_adjust != m_dPitchAdjust) {
        m_pSoundTouch->setPitch(
            KeyUtils::octaveChangeToPowerOf2(*pitch_adjust));
        m_dPitchAdjust = *pitch_adjust;
    }

    // NOTE(rryan) : There used to be logic here that clear()'d when the player
    // changed direction. I removed it because this is handled by EngineBuffer.

    // Used by other methods so we need to keep them up to date.
    m_dBaseRate = base_rate;
    m_bSpeedAffectsPitch = speed_affects_pitch;
    m_dSpeedAdjust = speed_abs;
    m_dPitchAdjust = *pitch_adjust;
}

void EngineBufferScaleST::clear() {
    m_pSoundTouch->clear();
}

CSAMPLE* EngineBufferScaleST::getScaled(unsigned long buf_size) {
    m_samplesRead = 0.0;

    if (m_dRateOld == 0 || m_dTempoOld == 0) {
        SampleUtil::clear(m_buffer, buf_size);
        m_samplesRead = buf_size;
        return m_buffer;
    }

    const int iNumChannels = 2;
    unsigned long total_received_frames = 0;
    unsigned long total_read_frames = 0;

    unsigned long remaining_frames = buf_size/2;
    CSAMPLE* read = m_buffer;
    bool last_read_failed = false;
    while (remaining_frames > 0) {
        unsigned long received_frames = m_pSoundTouch->receiveSamples(
            (SAMPLETYPE*)read, remaining_frames);
        remaining_frames -= received_frames;
        total_received_frames += received_frames;
        read += received_frames * iNumChannels;

        if (remaining_frames > 0) {
            unsigned long iLenFrames = kiSoundTouchReadAheadLength;
            unsigned long iAvailSamples = m_pReadAheadManager
                    ->getNextSamples(
                        // The value doesn't matter here. All that matters is we
                        // are going forward or backward.
                        (m_bBackwards ? -1.0 : 1.0) * m_dBaseRate * m_dSpeedAdjust,
                        buffer_back,
                        iLenFrames * iNumChannels);
            unsigned long iAvailFrames = iAvailSamples / iNumChannels;

            if (iAvailFrames > 0) {
                last_read_failed = false;
                total_read_frames += iAvailFrames;
                m_pSoundTouch->putSamples(buffer_back, iAvailFrames);
            } else {
                if (last_read_failed) {
                    m_pSoundTouch->flush();
                    break;
                }
                last_read_failed = true;
            }
        }
    }

    // qDebug() << "Fed ST" << total_read_frames*2
    //          << "samples to get" << total_received_frames*2 << "samples";
    if (total_received_frames != buf_size/2) {
        qDebug() << __FILE__ << "- only wrote" << total_received_frames
                 << "frames instead of requested" << buf_size;
    }

    // m_samplesRead is interpreted as the total number of virtual samples
    // consumed to produce the scaled buffer. Due to this, we do not take into
    // account directionality or starting point.
    // NOTE(rryan): Why no m_dPitchAdjust here? SoundTouch implements pitch
    // shifting as a tempo shift of (1/m_dPitchAdjust) and a rate shift of
    // (*m_dPitchAdjust) so these two cancel out.
    m_samplesRead = m_dBaseRate * m_dSpeedAdjust *
            total_received_frames * iNumChannels;

    return m_buffer;
}
