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
#include "engine/engineobject.h"
#include "engine/readaheadmanager.h"
#include "track/keyutils.h"
#include "util/math.h"
#include "util/sample.h"

using namespace soundtouch;

// Due to filtering and oversampling, SoundTouch is some samples behind.
// The value below was experimental identified using a saw signal and SoundTouch 1.8
// at a speed of 1.0
// 0.918 (upscaling 44.1 kHz to 48 kHz) will produce an additional offset of 3 Frames
// 0.459 (upscaling 44.1 kHz to 96 kHz) will produce an additional offset of 18 Frames
// (Rubberband does not suffer this issue)
static const int kSeekOffsetFrames = 519;

static const int kNumChannels = 2;

EngineBufferScaleST::EngineBufferScaleST(ReadAheadManager *pReadAheadManager)
    : EngineBufferScale(),
      m_bBackwards(false),
      m_pReadAheadManager(pReadAheadManager) {
    m_pSoundTouch = new soundtouch::SoundTouch();
    m_pSoundTouch->setChannels(kNumChannels);
    m_pSoundTouch->setRate(m_dBaseRate);
    m_pSoundTouch->setPitch(1.0);
    m_pSoundTouch->setSetting(SETTING_USE_QUICKSEEK, 1);
    m_pSoundTouch->setSampleRate(m_iSampleRate > 0 ? m_iSampleRate : 44100);

    buffer_back = new CSAMPLE[kiSoundTouchReadAheadLength*2];

    // Setting the tempo to a very low value will force SoundTouch
    // to preallocate buffers large enough to (almost certainly)
    // avoid memory reallocations during playback.
    m_pSoundTouch->setTempo(0.1);
    m_pSoundTouch->putSamples(buffer_back, kiSoundTouchReadAheadLength);
    m_pSoundTouch->clear();
    m_pSoundTouch->setTempo(m_dTempoRatio);
}

EngineBufferScaleST::~EngineBufferScaleST() {
    delete m_pSoundTouch;
    delete [] buffer_back;
}

void EngineBufferScaleST::setScaleParameters(double base_rate,
                                             double* pTempoRatio,
                                             double* pPitchRatio) {

    // Negative speed means we are going backwards. pitch does not affect
    // the playback direction.
    m_bBackwards = *pTempoRatio < 0;

    // It's an error to pass a rate or tempo smaller than MIN_SEEK_SPEED to
    // SoundTouch (see definition of MIN_SEEK_SPEED for more details).
    double speed_abs = fabs(*pTempoRatio);
    if (speed_abs > MAX_SEEK_SPEED) {
        speed_abs = MAX_SEEK_SPEED;
    } else if (speed_abs < MIN_SEEK_SPEED) {
        speed_abs = 0;
    }

    // Let the caller know if we clamped their value.
    *pTempoRatio = m_bBackwards ? -speed_abs : speed_abs;

    // Include baserate in rate_abs so that we do samplerate conversion as part
    // of rate adjustment.
    if (speed_abs != m_dTempoRatio) {
        // Note: A rate of zero would make Soundtouch crash,
        // this is caught in getScaled()
        m_pSoundTouch->setTempo(speed_abs);
        m_dTempoRatio = speed_abs;
    }
    if (base_rate != m_dBaseRate) {
        m_pSoundTouch->setRate(base_rate);
        m_dBaseRate = base_rate;
    }

    if (*pPitchRatio != m_dPitchRatio) {
        // Note: pitch ratio must be positive
        double pitch = fabs(*pPitchRatio);
        if (pitch > 0.0) {
            m_pSoundTouch->setPitch(pitch);
        }
        m_dPitchRatio = *pPitchRatio;
    }

    // NOTE(rryan) : There used to be logic here that clear()'d when the player
    // changed direction. I removed it because this is handled by EngineBuffer.
}

void EngineBufferScaleST::setSampleRate(int iSampleRate) {
    m_pSoundTouch->setSampleRate(iSampleRate);
    m_iSampleRate = iSampleRate;
}

void EngineBufferScaleST::clear() {
    m_pSoundTouch->clear();

    // compensate seek offset for a rate of 1.0
    SampleUtil::clear(buffer_back, kSeekOffsetFrames * kNumChannels);
    m_pSoundTouch->putSamples(buffer_back, kSeekOffsetFrames);
}

double EngineBufferScaleST::getScaled(CSAMPLE* pOutput, const int buf_size) {
    double samplesRead = 0.0;

    if (m_dBaseRate == 0 || m_dTempoRatio == 0 || m_dPitchRatio == 0) {
        SampleUtil::clear(pOutput, buf_size);
        return samplesRead;
    }

    unsigned int total_received_frames = 0;
    unsigned int total_read_frames = 0;

    unsigned int remaining_frames = buf_size / kNumChannels;
    CSAMPLE* read = pOutput;
    bool last_read_failed = false;
    while (remaining_frames > 0) {
        unsigned int received_frames = m_pSoundTouch->receiveSamples(
                (SAMPLETYPE*)read, remaining_frames);
        remaining_frames -= received_frames;
        total_received_frames += received_frames;
        read += received_frames * kNumChannels;

        if (remaining_frames > 0) {
            unsigned int iLenFrames = kiSoundTouchReadAheadLength;
            unsigned int iAvailSamples = m_pReadAheadManager->getNextSamples(
                        // The value doesn't matter here. All that matters is we
                        // are going forward or backward.
                        (m_bBackwards ? -1.0 : 1.0) * m_dBaseRate * m_dTempoRatio,
                        buffer_back,
                        iLenFrames * kNumChannels);
            unsigned int iAvailFrames = iAvailSamples / kNumChannels;

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
    if (total_received_frames != static_cast<unsigned int>(buf_size / kNumChannels)) {
        qDebug() << __FILE__ << "- only wrote" << total_received_frames
                 << "frames instead of requested" << buf_size;
    }

    // m_samplesRead is interpreted as the total number of virtual samples
    // consumed to produce the scaled buffer. Due to this, we do not take into
    // account directionality or starting point.
    // NOTE(rryan): Why no m_dPitchAdjust here? SoundTouch implements pitch
    // shifting as a tempo shift of (1/m_dPitchAdjust) and a rate shift of
    // (*m_dPitchAdjust) so these two cancel out.
    samplesRead = m_dBaseRate * m_dTempoRatio *
            total_received_frames * kNumChannels;

    return samplesRead;
}
