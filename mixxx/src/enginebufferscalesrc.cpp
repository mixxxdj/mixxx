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

EngineBufferScaleSRC::EngineBufferScaleSRC(ReaderExtractWave *wave) : EngineBufferScale(wave)
{
    wave_buffer = (float *)wave->getBasePtr();

    buffer_back = new CSAMPLE[MAX_BUFFER_LEN];
    
    // Initialize converter using low quality sinc interpolation and two channels
    int error;
    converter = src_new(2, 2, &error);

    // Initialize data struct. Assume that the audio file is never ending
    data = new SRC_DATA;
    data->end_of_input = 0; // HACK

    backwards = false;
    src_set_ratio(converter, rate);
}

EngineBufferScaleSRC::~EngineBufferScaleSRC()
{
    src_delete(converter);
    delete data;
    delete buffer_back;
}

void EngineBufferScaleSRC::setRate(double _rate)
{                                   
    if (_rate<0.)
    {
        backwards = true;
        rate = 1./(-_rate);
    }
    else
    {
        backwards = false;
        rate = 1./_rate;
    }
        
    src_set_ratio(converter, rate);
}   

CSAMPLE *EngineBufferScaleSRC::scale(double playpos, int buf_size)
{
    data->data_in = &wave_buffer[(int)playpos];
    data->data_out = buffer;
    data->input_frames = READBUFFERSIZE-(int)playpos;
    data->output_frames = buf_size;
    data->src_ratio = rate;

    // Perform conversion
    int error = src_process(converter, data);
    if (error!=0)
        qDebug("EngineBufferScaleSRC: %s, rate: %f",src_strerror(error),rate);

//    qDebug("in: %i, out: %i, rate: %f",data->input_frames_used, data->output_frames_gen, rate);
    
    // Check if wave_buffer is wrapped
    if (data->input_frames < data->output_frames)
    {
        data->data_in = &wave_buffer[0];
        data->data_out = &buffer[data->input_frames];
        data->input_frames = READBUFFERSIZE;
        data->output_frames = buf_size-data->input_frames;

        // Perform conversion
        src_process(converter, data);
    }
    
    // Reverse buffer if playback direction is backwards
    if (backwards==true)
    {
        for (int i=0; i<buf_size; i++)
            buffer_back[i] = buffer[buf_size-i];
        return buffer_back;
    }   

    return buffer;
}
