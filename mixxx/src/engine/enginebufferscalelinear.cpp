/***************************************************************************
                          enginebufferscalelinear.cpp  -  description
                            -------------------
    begin                : Mon Apr 14 2003
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

#include <QtCore>
#include "enginebufferscalelinear.h"
#include "mathstuff.h"

#define RATE_LERP_LENGTH 200

EngineBufferScaleLinear::EngineBufferScaleLinear(ReadAheadManager *pReadAheadManager) :
    EngineBufferScale(),
    m_pReadAheadManager(pReadAheadManager)
{
    m_dBaseRate = 0.0f;
    m_dTempo = 0.0f;
    m_fOldTempo = 1.0f;
    m_fOldBaseRate = 1.0f;
    m_scaleRemainder = 0.0f;
    m_dCurSampleIndex = 0.0f;
    m_dNextSampleIndex = 0.0f;
    
    for (int i=0; i<4; i++)
    	m_fPrevSample[i] = 0.0f;

    buffer_int = new CSAMPLE[kiLinearScaleReadAheadLength];
    buffer_int_size = 0;
}

EngineBufferScaleLinear::~EngineBufferScaleLinear()
{
}

double EngineBufferScaleLinear::setTempo(double _tempo)
{
//    if (m_fOldTempo != m_dTempo)
        m_fOldTempo = m_dTempo; //Save the old tempo when the tempo changes

    m_dTempo = _tempo;

    if (m_dTempo>MAX_SEEK_SPEED)
        m_dTempo = MAX_SEEK_SPEED;
    else if (m_dTempo < -MAX_SEEK_SPEED)
        m_dTempo = -MAX_SEEK_SPEED;

    // Determine playback direction
    if (m_dTempo<0.)
        m_bBackwards = true;
    else
        m_bBackwards = false;

    return m_dTempo;
}

void EngineBufferScaleLinear::setBaseRate(double dBaseRate)
{
//    if (m_fOldBaseRate != m_dBaseRate)
        m_fOldBaseRate = m_dBaseRate; //Save the old baserate when it changes

    m_dBaseRate = dBaseRate*m_dTempo;
}

void EngineBufferScaleLinear::clear()
{
    m_bClear = true;
    m_scaleRemainder = 0.0f;
}


// laurent de soras - punked from musicdsp.org (mad props)
inline float hermite4(float frac_pos, float xm1, float x0, float x1, float x2)
{
    const float c = (x1 - xm1) * 0.5f;
    const float v = x0 - x1;
    const float w = c + v;
    const float a = w + v + (x2 - x0) * 0.5f;
    const float b_neg = w + a;
    return ((((a * frac_pos) - b_neg) * frac_pos + c) * frac_pos + x0);
}

/** Stretch a buffer worth of audio using linear interpolation */
CSAMPLE * EngineBufferScaleLinear::scale(double playpos, unsigned long buf_size,
                                         CSAMPLE* pBase, unsigned long iBaseLength)
{

    long unscaled_samples_needed;
    float rate_add_new = m_dBaseRate;
    float rate_add_old = m_fOldBaseRate; //Smoothly interpolate to new playback rate
    float rate_add = rate_add_new;
    float rate_add_diff = rate_add_new - rate_add_old;
    double rate_add_abs;

    if ( rate_add_diff )
        m_scaleRemainder = 0.0f;

    //Update the old base rate because we only need to
    //interpolate/ramp up the pitch changes once.
    m_fOldBaseRate = m_dBaseRate;

    // Determine position in read_buffer to start from. (This is always 0 with
    // the new EngineBuffer implementation)
    new_playpos = playpos;

    const int iRateLerpLength = math_min(RATE_LERP_LENGTH, buf_size);

    // Guard against buf_size == 0
    if (iRateLerpLength == 0)
        return buffer;

    // Simulate the loop to estimate how many samples we need
    double samples = 0;

    for (int j = 0; j < iRateLerpLength; j+=2)
    {
        rate_add = (rate_add_diff) / iRateLerpLength * j + rate_add_old;
        samples += fabs(rate_add);
    }

    rate_add = rate_add_new;
    rate_add_abs = fabs(rate_add);

	//we're calculating mono samples, so divide remaining buffer by 2;
    samples += (rate_add_abs * ((buf_size - iRateLerpLength)/2));
    //because we always have previous samples, floor the result
    unscaled_samples_needed = ceil(samples);
	
    // Multiply by 2 because it is predicting mono rates, while we want a stereo
    // number of samples.
    unscaled_samples_needed *= 2;
    
    //Q_ASSERT(unscaled_samples_needed >= 0);
    //Q_ASSERT(unscaled_samples_needed != 0);

    long prev_index = 0;
    bool last_read_failed = false;
    CSAMPLE prev_sample[2];
    CSAMPLE cur_sample[2];
    double prevIndex=0;

    // Use new_playpos to count the new samples we touch.
    new_playpos = 0;
    
    prev_sample[0]=0;
    prev_sample[1]=0;
    cur_sample[0]=0;
    cur_sample[1]=0;

    int i = 0;
    int screwups = 0;
    //qDebug() << "rate" << rate_add << "buffer" << buf_size;
    while(i < buf_size)
    {
    	//shift indicies
    	prevIndex = m_dCurSampleIndex;
    	m_dCurSampleIndex = m_dNextSampleIndex;
    	//qDebug() << "indexes" << i << prevIndex << m_dCurSampleIndex;
    	
    	//if we're interpolating between old samples, load them
    	if (prevIndex < -1.0)
		{
			//qDebug() << "saved0";
			prev_sample[0] = m_fPrevSample[0];
			prev_sample[1] = m_fPrevSample[1];
		}
		else if (prevIndex < 0.0)
		{
			//qDebug() << "saved1" << prevIndex;
			prev_sample[0] = m_fPrevSample[2];
			prev_sample[1] = m_fPrevSample[3];
		}
		else
		{
			//this will never happen (right?) because the buffer is loaded
			//and the last time around the loop we were able to get a value
			//for the current sample.  And that was a ceil!
			if ((int)floor(prevIndex)*2+1 < buffer_int_size)
			{
				prev_sample[0] = buffer_int[(int)floor(prevIndex)*2];
				prev_sample[1] = buffer_int[(int)floor(prevIndex)*2+1];
			}
			else
			{
				prev_sample[0] = 0;
				prev_sample[1] = 0;
			}
		}
    	
    	//qDebug() << m_dCurSampleIndex;
    	
    	// if we don't have enough samples, load some more
    	while ((int)ceil(m_dCurSampleIndex)*2+1 >= buffer_int_size) {
    		//qDebug() << "need to buffer" << m_dCurSampleIndex << buffer_int_size;
    		int old_bufsize = buffer_int_size;
    		
    		m_fPrevSample[0] = prev_sample[0];
    		m_fPrevSample[1] = prev_sample[1];
    		m_fPrevSample[2] = cur_sample[0];
    		m_fPrevSample[3] = cur_sample[1];
    		
            //Q_ASSERT(unscaled_samples_needed > 0);
            if (unscaled_samples_needed == 0) {
            	qDebug() << "fuckup";
	            unscaled_samples_needed = 2;
	            screwups++;
            }

            int samples_to_read = math_min(kiLinearScaleReadAheadLength,
                                           unscaled_samples_needed);
                                           
            buffer_int_size = m_pReadAheadManager
                                ->getNextSamples(m_dBaseRate,buffer_int,
                                                 samples_to_read);

            if (m_dBaseRate > 0)
                new_playpos += buffer_int_size;
            else if (m_dBaseRate < 0)
                new_playpos -= buffer_int_size;


            if (buffer_int_size == 0 && last_read_failed) {
                break;
            }
            last_read_failed = buffer_int_size == 0;

            unscaled_samples_needed -= buffer_int_size;
            //shift the index by the size of the old buffer + 1 for prev
            m_dCurSampleIndex -= old_bufsize / 2;
            //qDebug() << "loaded" << m_dCurSampleIndex << buffer_int_size;
        }

 		//load the new sample, which could still be an old sample
    	if (m_dCurSampleIndex < -1.0)
		{
			//qDebug() << "saved2";
			cur_sample[0] = m_fPrevSample[2];
			cur_sample[1] = m_fPrevSample[3];
		}
		else if (m_dCurSampleIndex < 0.0)
		{
			//qDebug() << "saved3" << m_dCurSampleIndex;
			cur_sample[0] = buffer_int[0];
			cur_sample[1] = buffer_int[1];
		}
		else
		{
			//or a new one, rounded UP
			cur_sample[0] = buffer_int[(int)ceil(m_dCurSampleIndex)*2];
			cur_sample[1] = buffer_int[(int)ceil(m_dCurSampleIndex)*2+1];
		}

        //Smooth any changes in the playback rate over iRateLerpLength
        //samples. This prevents the change from being discontinuous and helps
        //improve sound quality.
        if (i < iRateLerpLength) {
            rate_add = (rate_add_diff) / iRateLerpLength * i + rate_add_old;
        }
        else {
            rate_add = rate_add_new;
        }

		//for the current index, what percentage is it between the previous and the next?
        CSAMPLE frac = m_dCurSampleIndex - floor(m_dCurSampleIndex);
        

        //Perform linear interpolation
        buffer[i] = prev_sample[0] + frac * (cur_sample[0] - prev_sample[0]);
        buffer[i+1] = prev_sample[1] + frac * (cur_sample[1] - prev_sample[1]);

        if (i < iRateLerpLength)
            m_dNextSampleIndex = m_dCurSampleIndex + fabs(rate_add);
        else
            m_dNextSampleIndex = m_dCurSampleIndex + rate_add_abs;
            
        i+=2;
    }

    /*m_fPrevSample[0] = prev_sample[0];
    m_fPrevSample[1] = prev_sample[1];
    m_fPrevSample[2] = cur_sample[0];
    m_fPrevSample[3] = cur_sample[1];*/

	Q_ASSERT(i>=buf_size);

    // It's possible that we will exit this function without having satisfied
    // this requirement. We may be trying to read past the end of the file.
    //Q_ASSERT(unscaled_samples_needed == 0);

    return buffer;
}
