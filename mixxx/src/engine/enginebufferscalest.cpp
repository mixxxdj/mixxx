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

EngineBufferScaleST::EngineBufferScaleST(ReadAheadManager *pReadAheadManager) :
    EngineBufferScale(),
    m_bBackwards(false),
    m_bPitchIndpTimeStretch(false),
    m_bClear(true),
    m_pReadAheadManager(pReadAheadManager)
{
    m_qMutex.lock();
    m_pSoundTouch = new soundtouch::SoundTouch();
    m_dBaseRate = 1.;
    m_dTempo = 1.;

    m_pSoundTouch->setChannels(2);
    m_pSoundTouch->setRate(m_dBaseRate);
    m_pSoundTouch->setTempo(m_dTempo);
    m_pSoundTouch->setSetting(SETTING_USE_QUICKSEEK, 1);
    m_qMutex.unlock();

    slotSetSamplerate(44100.);
    ControlObject * p = ControlObject::getControl(ConfigKey("[Master]","samplerate"));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotSetSamplerate(double)));

    buffer_back = new CSAMPLE[kiSoundTouchReadAheadLength*2];
}

EngineBufferScaleST::~EngineBufferScaleST()
{
    delete m_pSoundTouch;
    delete [] buffer_back;
}

void EngineBufferScaleST::setPitchIndpTimeStretch(bool b)
{
    m_bPitchIndpTimeStretch = b;
    m_qMutex.lock();
    if (m_bPitchIndpTimeStretch)
        m_pSoundTouch->setRate(1.);
    else
        m_pSoundTouch->setTempo(1.);
    m_qMutex.unlock();
}

bool EngineBufferScaleST::getPitchIndpTimeStretch(void)
{
    return m_bPitchIndpTimeStretch;
}


void EngineBufferScaleST::setBaseRate(double dBaseRate)
{
    m_dBaseRate = dBaseRate;

    m_qMutex.lock();
    if (m_bPitchIndpTimeStretch) {
        m_pSoundTouch->setRate(m_dBaseRate);
    }
    //or if if we use ST for linear interpolation...
    else if (m_dBaseRate >= MIN_SEEK_SPEED) {
        m_pSoundTouch->setRate(m_dBaseRate*m_dTempo);
    }
    //It's an error to pass a rate or tempo smaller than MIN_SEEK_SPEED to SoundTouch.
    //if (m_dBaseRate <= MIN_SEEK_SPEED)
    //    m_pSoundTouch->setRate(0.010f);
    //else if(m_dBaseRate >= MIN_SEEK_SPEED)
    //    m_pSoundTouch->setRate(m_dBaseRate*m_dTempo);
    m_qMutex.unlock();
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

double EngineBufferScaleST::setTempo(double dTempo)
{
    double dTempoOld = m_dTempo;
    m_dTempo = fabs(dTempo);

    if (m_dTempo>MAX_SEEK_SPEED)
        m_dTempo = MAX_SEEK_SPEED;
    else if (m_dTempo<MIN_SEEK_SPEED)
        m_dTempo = 0.0;

    m_qMutex.lock();
    //It's an error to pass a rate or tempo smaller than MIN_SEEK_SPEED to SoundTouch.
    if (dTempoOld != m_dTempo && m_dTempo != 0.0)
    {
        if (m_bPitchIndpTimeStretch)
            m_pSoundTouch->setTempo(m_dTempo);
        else
            m_pSoundTouch->setRate(m_dBaseRate*m_dTempo);
    }
    m_qMutex.unlock();

    if (dTempo<0.)
    {
        if (!m_bBackwards)
            clear();

        m_bBackwards = true;
        return -m_dTempo;
    }
    else
    {
        if (m_bBackwards)
            clear();

        m_bBackwards = false;
        return m_dTempo;
    }
}

/**
 * @param playpos The play position in the EngineBuffer (in samples)
 * @param buf_size The size of the audio buffer to scale (in samples)
 * @param pBase Pointer to the source audio to scale.
 * @param iBaseLength the length of the source audio available
 */
CSAMPLE* EngineBufferScaleST::scale(double playpos, unsigned long buf_size,
                                    CSAMPLE* pBase, unsigned long iBaseLength) {
    Q_UNUSED (pBase);
    Q_UNUSED (iBaseLength);
    new_playpos = 0.0;

    m_qMutex.lock();

    int iCurPos = playpos;
    if (!even(iCurPos)) {
        iCurPos--;
    }

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

    unsigned long total_received_frames = 0;
    unsigned long total_read_frames = 0;

    unsigned long remaining_frames = buf_size/2;
    //long remaining_source_frames = iBaseLength/2;
    CSAMPLE* read = buffer;
    bool last_read_failed = false;
    while (remaining_frames > 0) {
        unsigned long received_frames = m_pSoundTouch->receiveSamples((SAMPLETYPE*)read, remaining_frames);
        remaining_frames -= received_frames;
        total_received_frames += received_frames;
        read += received_frames*2;

        if (remaining_frames > 0) {
            // math_min(kiSoundTouchReadAheadLength,remaining_source_frames);
            unsigned long iLenFrames = kiSoundTouchReadAheadLength;
            unsigned long iAvailSamples = m_pReadAheadManager
                ->getNextSamples((m_bBackwards ? -1.0f : 1.0f) * m_dBaseRate * m_dTempo,
                                 buffer_back,
                                 iLenFrames * 2);
            unsigned long iAvailFrames = iAvailSamples / 2;

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

    //Feed more samples into SoundTouch until it has processed enough to
    //fill the audio buffer that we need to fill.
    //SoundTouch::numSamples() returns the number of _FRAMES_ that
    //are in its FIFO audio buffer...


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
    new_playpos = m_dTempo*m_dBaseRate*total_received_frames*2;

    m_qMutex.unlock();

    return buffer;
}

