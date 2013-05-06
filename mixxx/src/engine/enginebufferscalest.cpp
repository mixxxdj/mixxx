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

#include <QtCore>

#include "enginebufferscalest.h"

// Fixes redefinition warnings from SoundTouch.
#undef TRUE
#undef FALSE
#include "SoundTouch.h"
#include "mathstuff.h"
#include "controlobject.h"
#include "engine/readaheadmanager.h"
#include "engine/engineobject.h"

using namespace soundtouch;

EngineBufferScaleST::EngineBufferScaleST(ReadAheadManager *pReadAheadManager)
    : EngineBufferScale(),
      m_bBackwards(false),
      m_bClear(true),
      m_pReadAheadManager(pReadAheadManager) {
    m_qMutex.lock();
    m_pSoundTouch = new soundtouch::SoundTouch();

    m_pSoundTouch->setChannels(2);
    m_pSoundTouch->setRate(1.0);
    m_pSoundTouch->setTempo(1.0);
    m_pSoundTouch->setPitch(1.0);
    m_pSoundTouch->setSetting(SETTING_USE_QUICKSEEK, 1);
    m_qMutex.unlock();

    slotSetSamplerate(44100.);
    ControlObject * p = ControlObject::getControl(ConfigKey("[Master]","samplerate"));
    connect(p, SIGNAL(valueChanged(double)),
            this, SLOT(slotSetSamplerate(double)));

    buffer_back = new CSAMPLE[kiSoundTouchReadAheadLength*2];
}

EngineBufferScaleST::~EngineBufferScaleST()
{
    delete m_pSoundTouch;
    delete [] buffer_back;
}

void EngineBufferScaleST::setScaleParameters(double* rate_adjust,
                                             double* tempo_adjust,
                                             double* pitch_adjust) {
    // Assumes rate_adjust is just baserate (which cannot be negative) and
    // pitch_adjust cannot be negative because octave change conversion to pitch
    // ratio is an exp(x) function.
    m_bBackwards = (*tempo_adjust * *rate_adjust) < 0;

    // It's an error to pass a rate or tempo smaller than MIN_SEEK_SPEED to
    // SoundTouch (see definition of MIN_SEEK_SPEED for more details).
    double tempo_abs = fabs(*tempo_adjust);
    if (tempo_abs > MAX_SEEK_SPEED) {
        tempo_abs = MAX_SEEK_SPEED;
    } else if (tempo_abs < MIN_SEEK_SPEED) {
        tempo_abs = 0;
    }

    // Let the caller know we clamped their value.
    *tempo_adjust = m_bBackwards ? -tempo_abs : tempo_abs;

    if (*pitch_adjust <= 0.0) {
        qWarning() << "EngineBufferScaleST: Ignoring non-positive pitch adjust.";
        *pitch_adjust = 1.0;
    }

    double rate_abs = fabs(*rate_adjust);

    m_qMutex.lock();
    // Note that we do not set the tempo if it is zero. This is because of the
    // above clamping which prevents us from going below MIN_SEEK_SPEED. I think
    // we should handle this better but I have left the logic in place. rryan
    // 4/2013.
    if (tempo_abs != m_dTempoAdjust && tempo_abs != 0) {
        m_pSoundTouch->setTempo(tempo_abs);
        m_dTempoAdjust = tempo_abs;
    }
    if (rate_abs != m_dRateAdjust) {
        m_pSoundTouch->setRate(rate_abs);
        m_dRateAdjust = rate_abs;
    }
    if (*pitch_adjust != m_dPitchAdjust) {
        m_pSoundTouch->setPitch(*pitch_adjust);
        m_dPitchAdjust = *pitch_adjust;
    }
    m_qMutex.unlock();

    // NOTE(rryan) : There used to be logic here that clear()'d when the player
    // changed direction. I removed it because this is handled by EngineBuffer.
}

void EngineBufferScaleST::clear()
{
    m_qMutex.lock();
    m_pSoundTouch->clear();
    m_bClear = true;
    m_qMutex.unlock();
}

void EngineBufferScaleST::slotSetSamplerate(double dSampleRate)
{
    int iSrate = (int)dSampleRate;

    m_qMutex.lock();
    if (iSrate>0)
        m_pSoundTouch->setSampleRate(iSrate);
    else
        m_pSoundTouch->setSampleRate(44100);
    m_qMutex.unlock();
}

CSAMPLE* EngineBufferScaleST::getScaled(unsigned long buf_size) {
    m_samplesRead = 0.0;

    if (m_dPitchAdjust == 0 || m_dRateAdjust == 0 || m_dTempoAdjust == 0) {
        memset(m_buffer, 0, sizeof(m_buffer[0]) * buf_size);
        return m_buffer;
    }

    m_qMutex.lock();

    //If we've just cleared SoundTouch's FIFO of unprocessed samples,
    //then reset our "read ahead position" because we probably need
    //to read backwards instead of forwards or something like that.
    // if (true || m_bClear)
    // {
    //     m_iReadAheadPos = (unsigned long)playpos;
    //     if (!even(m_iReadAheadPos))
    //         m_iReadAheadPos--;
    //         //m_iReadAheadPos = (m_iReadAheadPos+1)%iBaseLength;
    //     m_bClear = false;
    // }
    //Q_ASSERT(m_iReadAheadPos >= 0);

    const int iNumChannels = 2;
    unsigned long total_received_frames = 0;
    unsigned long total_read_frames = 0;

    unsigned long remaining_frames = buf_size/2;
    //long remaining_source_frames = iBaseLength/2;
    CSAMPLE* read = m_buffer;
    bool last_read_failed = false;
    while (remaining_frames > 0) {
        unsigned long received_frames = m_pSoundTouch->receiveSamples(
            (SAMPLETYPE*)read, remaining_frames);
        remaining_frames -= received_frames;
        total_received_frames += received_frames;
        read += received_frames * iNumChannels;

        if (remaining_frames > 0) {
            // math_min(kiSoundTouchReadAheadLength,remaining_source_frames);
            unsigned long iLenFrames = kiSoundTouchReadAheadLength;
            unsigned long iAvailSamples = m_pReadAheadManager
                    ->getNextSamples(
                        // The value doesn't matter here. All that matters is we
                        // are going forward or backward.
                        (m_bBackwards ? -1.0f : 1.0f) * m_dRateAdjust * m_dTempoAdjust,
                        buffer_back,
                        iLenFrames * iNumChannels);
            unsigned long iAvailFrames = iAvailSamples / iNumChannels;

            if (iAvailFrames > 0) {
                last_read_failed = false;
                total_read_frames += iAvailFrames;
                m_pSoundTouch->putSamples(buffer_back, iAvailFrames);
            } else {
                if (last_read_failed)
                    break;
                last_read_failed = true;
                m_pSoundTouch->flush();
            }
        }
    }

    // Feed more samples into SoundTouch until it has processed enough to
    // fill the audio buffer that we need to fill.
    // SoundTouch::numSamples() returns the number of _FRAMES_ that
    // are in its FIFO audio buffer...

    // Calculate new playpos

    //Get the stretched _frames_ (not Samples, as the function call
    //erroroneously implies)
    //long receivedFrames = m_pSoundTouch->receiveSamples((SAMPLETYPE*)buffer, buf_size/2);

    // qDebug() << "Fed ST" << total_read_frames*2
    //          << "samples to get" << total_received_frames*2 << "samples";
    if (total_received_frames != buf_size/2)
    {
        qDebug() << __FILE__ << "- only wrote" << total_received_frames << "frames instead of requested" << buf_size;
    }

    //for (unsigned long i = 0; i < buf_size; i++)
    //    qDebug() << buffer[i];

    // new_playpos is now interpreted as the total number of virtual samples
    // consumed to produce the scaled buffer. Due to this, we do not take into
    // account directionality or starting point.
    // NOTE(rryan): Why no m_dPitchAdjust here? SoundTouch implements pitch
    // shifting as a tempo shift of (1/m_dPitchAdjust) and a rate shift of
    // (*m_dPitchAdjust) so these two cancel out.
    m_samplesRead = m_dTempoAdjust * m_dRateAdjust *
            total_received_frames * 2;

    m_qMutex.unlock();

    return m_buffer;
}

