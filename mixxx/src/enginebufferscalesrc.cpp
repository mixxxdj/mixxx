/***************************************************************************
                          enginebufferscalesrc.cpp  -  description
                             -------------------
    begin                : Sun Apr 13 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
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

#include <QtDebug>
#include "enginebufferscalesrc.h"
#include "readerextractwave.h"
#include "mathstuff.h"

EngineBufferScaleSRC::EngineBufferScaleSRC(ReaderExtractWave * wave) : EngineBufferScale(wave)
{
    buffer_back = new CSAMPLE[MAX_BUFFER_LEN];

    // Initialize converter for three qualities and two channels
    int error;
    converter0 = src_new(0, 2, &error);
    if (error!=0)
        qDebug() << "EngineBufferScaleSRC: " << src_strerror(error);
    converter1 = src_new(1, 2, &error);
    if (error!=0)
        qDebug() << "EngineBufferScaleSRC: " << src_strerror(error);
    converter2 = src_new(2, 2, &error);
    if (error!=0)
        qDebug() << "EngineBufferScaleSRC: " << src_strerror(error);
    converter3 = src_new(3, 2, &error);
    if (error!=0)
        qDebug() << "EngineBufferScaleSRC: " << src_strerror(error);
    converter4 = src_new(4, 2, &error);
    if (error!=0)
        qDebug() << "EngineBufferScaleSRC: " << src_strerror(error);

    m_iQuality = 4; //Make this the same as the index in the converter right below this.
    converterActive = converter4;

    // Initialize data struct. Assume that the audio file is never ending
    data = new SRC_DATA;
    data->end_of_input = 0; // HACK

    m_bBackwards = false;
    src_set_ratio(converter0, m_dTempo);
    src_set_ratio(converter1, m_dTempo);
    src_set_ratio(converter2, m_dTempo);
    src_set_ratio(converter3, m_dTempo);
    src_set_ratio(converter4, m_dTempo);
}

EngineBufferScaleSRC::~EngineBufferScaleSRC()
{
    src_delete(converter0);
    src_delete(converter1);
    src_delete(converter2);
    src_delete(converter3);
    src_delete(converter4);
    delete data;
    delete [] buffer_back;
}

void EngineBufferScaleSRC::setQuality(int q)
{
    if (q>4 || q<0)
        m_iQuality = 4;
    else
        m_iQuality = q;

    //qDebug() << "quality " << m_iQuality;

    switch (m_iQuality)
    {
    case 0:
        converterActive = converter0;
    case 1:
        converterActive = converter1;
    case 2:
        converterActive = converter2;
    case 3:
        converterActive = converter3;
    case 4:
        converterActive = converter4;
    }
}

void EngineBufferScaleSRC::setFastMode(bool bMode)
{
    if (bMode)
    {
        //qDebug() << "fast";
        converterActive = converter4;
    }
    else
    {
        //qDebug() << "slow";
        setQuality(m_iQuality);
    }
}

double EngineBufferScaleSRC::setTempo(double dTempo)
{
    double dTempoOld = m_dTempo;

    if (dTempo==0.)
        m_dTempo = 0.;
    else if (dTempo<0.)
    {
        m_bBackwards = true;
        m_dTempo = 1./(-dTempo);
    }
    else
    {
        m_bBackwards = false;
        m_dTempo = 1./dTempo;
    }

    // Force dirty interpolation if speed is above 2x
/*
   //Disabled by Albert because I don't think we need this anymore
   //(we're using the same interpolation all the time now...)
    if (m_dTempo<0.5 && dTempoOld>=0.5)
        setFastMode(true);
    else if (m_dTempo>=0.5 && dTempoOld<0.5)
        setFastMode(false);

 */

    // Ensure valid range of rate
    if (m_dTempo==0.)
        return 0.;
    else if (m_dTempo>12.)
        m_dTempo = 12.;
    else if (m_dTempo<1./12.)
        m_dTempo = 1./12.;

    src_set_ratio(converter0, m_dTempo);
    src_set_ratio(converter1, m_dTempo);
    src_set_ratio(converter2, m_dTempo);
    src_set_ratio(converter3, m_dTempo);
    src_set_ratio(converter4, m_dTempo);

    if (m_bBackwards)
        return -(1./m_dTempo);
    else
        return (1./m_dTempo);
}

CSAMPLE * EngineBufferScaleSRC::scale(double playpos, int buf_size, float * pBase, int iBaseLength)
{
    if (!pBase)
    {
        pBase = wavebuffer;
        iBaseLength = READBUFFERSIZE;
    }

    int consumed = 0;

    // Invert wavebuffer is backwards playback
    if (m_bBackwards)
    {
        int no = (int)(buf_size*(1./m_dTempo)+4); // Read some extra samples
        if (!even(no))
            no++;

        int pos = (int)playpos;
        for (int i=0; i<no; ++i)
        {
            if (pos-i<0)
                pos = iBaseLength+i;
            buffer_back[i] = pBase[pos-i];
        }
        data->data_in = buffer_back;
        data->input_frames = no/2;
    }
    else
    {
        data->data_in = &pBase[(int)playpos];
        data->input_frames = (iBaseLength-(int)playpos)/2;
    }

    data->data_out = buffer;
    data->output_frames = buf_size/2;
    data->src_ratio = m_dTempo;

    // Perform conversion
    int error = src_process(converterActive, data);
    if (error!=0)
        qCritical("EngineBufferScaleSRC: %s",src_strerror(error));

    consumed += data->input_frames_used;

    // Check if wave_buffer is wrapped
    if (data->output_frames_gen*2 < buf_size && !m_bBackwards)
    {
        data->data_in = &pBase[0];
        data->data_out = &buffer[data->output_frames_gen*2];
        data->input_frames = iBaseLength/2;
        data->output_frames = (buf_size-data->output_frames_gen*2)/2;

        // Perform conversion
        int error = src_process(converterActive, data);
        if (error!=0)
            qDebug() << "EngineBufferScaleSRC: " << src_strerror(error);

        consumed += data->input_frames_used;
    }

    // Calculate new playpos
    if (m_bBackwards)
        new_playpos = playpos - (double)consumed*2.;
    else
        new_playpos = playpos + (double)consumed*2.;

    return buffer;
}

//TODO: This probably needs to be implemented properly
//in order for seeking to do at the same rate as SoundTouch's
//seeking...
void EngineBufferScaleSRC::setBaseRate(double dBaseRate)

{

    m_dBaseRate = dBaseRate;


/*
    if (m_bPitchIndpTimeStretch)

        m_pSoundTouch->setRate(m_dBaseRate);

    else

        m_pSoundTouch->setRate(m_dBaseRate*m_dTempo);
 */
}



void EngineBufferScaleSRC::clear()

{

    //m_pSoundTouch->clear();

    //m_bClear = true;

}

