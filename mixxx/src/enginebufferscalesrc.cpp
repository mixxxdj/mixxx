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

#include "enginebufferscalesrc.h"
#include "readerextractwave.h"
#include "mathstuff.h"

EngineBufferScaleSRC::EngineBufferScaleSRC(ReaderExtractWave *wave) : EngineBufferScale(wave)
{
    buffer_back = new CSAMPLE[MAX_BUFFER_LEN];

    // Initialize converter for three qualities and two channels
    int error;
    converter2 = src_new(2, 2, &error);    
    if (error!=0)
        qDebug("EngineBufferScaleSRC: %s",src_strerror(error));
    converter3 = src_new(3, 2, &error);    
    if (error!=0)
        qDebug("EngineBufferScaleSRC: %s",src_strerror(error));
    converter4 = src_new(4, 2, &error);    
    if (error!=0)
        qDebug("EngineBufferScaleSRC: %s",src_strerror(error));

    m_iQuality = 4;
    converterActive = converter4;    
        
    // Initialize data struct. Assume that the audio file is never ending
    data = new SRC_DATA;
    data->end_of_input = 0; // HACK

    backwards = false;
    src_set_ratio(converter2, rate);
    src_set_ratio(converter3, rate);
    src_set_ratio(converter4, rate);
}

EngineBufferScaleSRC::~EngineBufferScaleSRC()
{
    src_delete(converter2);
    src_delete(converter3);
    src_delete(converter4);
    delete data;
    delete [] buffer_back;
}

void EngineBufferScaleSRC::setQuality(int q)
{
    if (q>4 || q<2)
        m_iQuality = 4;
    else
        m_iQuality = q;
    
    switch (m_iQuality)
    {
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
        //qDebug("fast");
        converterActive = converter4;
    }
    else
    {
        //qDebug("slow");
        setQuality(m_iQuality);
    }
}

double EngineBufferScaleSRC::setRate(double _rate)
{
    double rateold = rate;

    if (_rate==0.)
        rate = 0.;
    else if (_rate<0.)
    {
        backwards = true;
        rate = 1./(-_rate);
    }
    else
    {
        backwards = false;
        rate = 1./_rate;
    }
    
    // Force dirty interpolation if speed is above 2x
    if (rate<0.5 && rateold>=0.5)
        setFastMode(true);
    else if (rate>=0.5 && rateold<0.5)
	setFastMode(false);

    // Ensure valid range of rate
    if (rate==0.)
        return 0.;
    else if (rate>12.)
        rate = 12.;
    else if (rate<1./12.)
        rate = 1./12.;

    src_set_ratio(converter2, rate);
    src_set_ratio(converter3, rate);
    src_set_ratio(converter4, rate);

    if (backwards)
        return -(1./rate);
    else
        return (1./rate);
}

CSAMPLE *EngineBufferScaleSRC::scale(double playpos, int buf_size)
{
    int consumed = 0;

    // Invert wavebuffer is backwards playback
    if (backwards)
    {
        int no = (int)(buf_size*(1./rate)+4); // Read some extra samples
        if (!even(no))
            no++;

        int pos = (int)playpos;
        for (int i=0; i<no; ++i)
        {
            if (pos-i<0)
                pos = READBUFFERSIZE+i;
            buffer_back[i] = wavebuffer[pos-i];
        }
        data->data_in = buffer_back;
        data->input_frames = no/2;
    }
    else
    {
        data->data_in = &wavebuffer[(int)playpos];
        data->input_frames = (READBUFFERSIZE-(int)playpos)/2;
    }

    data->data_out = buffer;
    data->output_frames = buf_size/2;
    data->src_ratio = rate;

    // Perform conversion
    int error = src_process(converterActive, data);
    if (error!=0)
        qFatal("EngineBufferScaleSRC: %s",src_strerror(error));

    consumed += data->input_frames_used;

    // Check if wave_buffer is wrapped
    if (data->output_frames_gen*2 < buf_size && !backwards)
    {
        data->data_in = &wavebuffer[0];
        data->data_out = &buffer[data->output_frames_gen*2];
        data->input_frames = READBUFFERSIZE/2;
        data->output_frames = (buf_size-data->output_frames_gen*2)/2;

        // Perform conversion
        int error = src_process(converterActive, data);
        if (error!=0)
            qDebug("EngineBufferScaleSRC: %s",src_strerror(error));

        consumed += data->input_frames_used;
   }

    // Calculate new playpos
    if (backwards)
        new_playpos = playpos - (double)consumed*2.;
    else
        new_playpos = playpos + (double)consumed*2.;

    return buffer;
}
