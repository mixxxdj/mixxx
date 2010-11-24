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

//#define RATE_LERP_LENGTH 200
#define RATE_LERP_LENGTH 400

EngineBufferScaleLinear::EngineBufferScaleLinear(ReadAheadManager *pReadAheadManager) :
    EngineBufferScale(),
    m_pReadAheadManager(pReadAheadManager)
{
    m_dBaseRate = 0.0f;
    m_dTempo = 0.0f;
    m_fOldTempo = 1.0f;
    m_fOldBaseRate = 1.0f;
    m_dCurSampleIndex = 0.0f;
    m_dNextSampleIndex = 0.0f;
    
    for (int i=0; i<2; i++)
    	m_fPrevSample[i] = 0.0f;

    buffer_int = new CSAMPLE[kiLinearScaleReadAheadLength];
    buffer_int_size = 0;
    
    //df.setFileName("mixxx-debug.csv");
    //df.open(QIODevice::WriteOnly | QIODevice::Text);
    //writer.setDevice(&df);
}

EngineBufferScaleLinear::~EngineBufferScaleLinear()
{
	//df.close();
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
    bool reversing = false;

    //Update the old base rate because we only need to
    //interpolate/ramp up the pitch changes once.
    m_fOldBaseRate = m_dBaseRate;
    
    // Determine position in read_buffer to start from. (This is always 0 with
    // the new EngineBuffer implementation)
    new_playpos = playpos;

    const int iRateLerpLength = math_min(RATE_LERP_LENGTH, buf_size); //= buf_size;

    // Guard against buf_size == 0
    if (iRateLerpLength == 0)
        return buffer;
      
    if (rate_add_new * rate_add_old < 0)
    {
    	//qDebug() << "Reverse direction" << rate_add_old << rate_add_new;
    	
    	//calculate some buffer going one way, and some buffer going
    	//the other way.  Then crossfade between the two, and dip volume
    	//in the middle. Voila, a scratch
    	
    	int overlap = math_min(buf_size / 4, RATE_LERP_LENGTH);
    	CSAMPLE *pOldRate = new CSAMPLE[buf_size/2 + overlap];
    	CSAMPLE *pNewRate = new CSAMPLE[buf_size/2 + overlap];
    	CSAMPLE *buffer_save = buffer;
    	
    	
    	float old_rate = rate_add_old;
    	float new_rate = rate_add_new;
    	
    	m_fOldBaseRate = old_rate;
    	m_dBaseRate = old_rate;
    	//is this insanely not threadsafe?  I haven't had problems...
    	buffer = pOldRate;
    	pOldRate = scale(0, buf_size/2 + overlap, pBase, iBaseLength);
    	
    	buffer_int_size=0;
    	m_dCurSampleIndex = -1.0;
    	m_dNextSampleIndex = 0;
    	
    	m_fPrevSample[0] = pOldRate[buf_size/2 - 2];
    	m_fPrevSample[1] = pOldRate[buf_size/2 - 1];
    	m_fOldBaseRate = new_rate;
    	m_dBaseRate = new_rate;
    	buffer = pNewRate;
    	pNewRate = scale(0, buf_size/2 + overlap, pBase, iBaseLength);
    	
    	buffer = buffer_save;
    	
    	int cross_start = buf_size / 2 - overlap;
    	int cross_end = buf_size / 2 + overlap;
    	//qDebug() << "cross" << cross_start << cross_end;
    	for (int i=0; i<buf_size; i+=2)
    	{
    		
    		if (i<cross_start)
    		{
    			buffer[i] = pOldRate[i];
    			buffer[i+1] = pOldRate[i+1];
    		}
    		else if (i>=cross_start && i < cross_end)
    		{
    			float frac = (float)(i - cross_start) / ((float)overlap * 2.0);
    			//dip volume across speed change
    			float gain = 2 * frac - 1.0;
    			//float gain = 1.0;
    			buffer[i] =  gain * ((1.0 - frac) * (float)pOldRate[i] + frac * (float)pNewRate[i-cross_start]);
    			buffer[i+1] = gain * ((1.0 - frac) * (float)pOldRate[i+1] + frac * (float)pNewRate[i+1-cross_start]);
    		}
    		else // if (i>=cross_end)
    		{
    			buffer[i] = pNewRate[i-cross_start];
    			buffer[i+1] = pNewRate[i+1-cross_start];
    		}
    	}
    	
    	delete pOldRate;
    	delete pNewRate;
    	
		return buffer;
    }
    
    // Simulate the loop to estimate how many samples we need
    double samples = 0;

    for (int j = 0; j < iRateLerpLength; j+=2)
    {
        rate_add = (rate_add_diff) / (float)iRateLerpLength * (float)j + rate_add_old;
        samples += fabs(rate_add);
    }
    
    rate_add = rate_add_new;
    rate_add_abs = fabs(rate_add);

	//we're calculating mono samples, so divide remaining buffer by 2;
    samples += (rate_add_abs * ((float)(buf_size - iRateLerpLength)/2));
   	unscaled_samples_needed = floor(samples);
   	
   	unscaled_samples_needed = math_max(4,unscaled_samples_needed); 
    	
   	//if the current position fraction plus the future position fraction 
   	//loops over 1.0, we need to round up
    if (m_dNextSampleIndex - floor(m_dNextSampleIndex) + samples - floor(samples) > 1.0)
    	unscaled_samples_needed++;
    
    // Multiply by 2 because it is predicting mono rates, while we want a stereo
    // number of samples.
    unscaled_samples_needed *= 2;
    
    //Q_ASSERT(unscaled_samples_needed >= 0);
    //Q_ASSERT(unscaled_samples_needed != 0);

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
    bool zeroed=false;
    //qDebug() << "rates:" << rate_add_old << rate_add_new;
    //qDebug() <<  m_dCurSampleIndex;
    while(i < buf_size)
    {
    	//shift indicies
    	prevIndex = m_dCurSampleIndex;
    	m_dCurSampleIndex = m_dNextSampleIndex;
    	
    	//we're going to be interpolating between two samples, a lower (prev)
    	//and upper (cur) sample.  If the lower sample is off the end of the buffer,
    	//load it from the saved globals
    	if (m_dCurSampleIndex < 0.0)
		{
			prev_sample[0] = m_fPrevSample[0];
			prev_sample[1] = m_fPrevSample[1];
		}
		else
		{
			//if the lower sample is in the buffer, cool
			if ((int)floor(m_dCurSampleIndex)*2+1 < buffer_int_size)
			{
				prev_sample[0] = buffer_int[(int)floor(m_dCurSampleIndex)*2];
				prev_sample[1] = buffer_int[(int)floor(m_dCurSampleIndex)*2+1];
			}
			else
			{
				//this happens if we're just starting, but maybe it should
				//be set below to whatever the current one is?
				///qDebug() << "zero" << m_fPrevSample[0];
				////if (buffer_int_size >0)
				//	qDebug() << "or maybe" << buffer[0] << buffer[buffer_int_size-2];
				prev_sample[0] = m_fPrevSample[0];
				prev_sample[1] = m_fPrevSample[1];
			}
		}
		
		//if (rate_add_abs < 0.1)
		//	zeroed=true;
		
    	// if we don't have enough samples, load some more
    	while ((int)ceil(m_dCurSampleIndex)*2+1 >= buffer_int_size) {
    		int old_bufsize = buffer_int_size;
    		
            //Q_ASSERT(unscaled_samples_needed > 0);
            if (unscaled_samples_needed == 0) {
            	//qDebug() << "screwup" << m_dCurSampleIndex << (int)ceil(m_dCurSampleIndex)*2+1 << buffer_int_size;
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
            //shift the index by the size of the old buffer
            m_dCurSampleIndex -= old_bufsize / 2;
            //fractions below 0 is ok, the ceil will bring it up to 0
            Q_ASSERT(m_dCurSampleIndex > -1.0);
            prevIndex -= old_bufsize / 2;
            if (prevIndex > -2.0)
            {
            	if (old_bufsize > 0)
            	{
            		//if (rate_add_abs < 0.1)
	            	//	qDebug() << "cursample";
	            	//at slower speeds, the lower sample will just be the current
	            	//sample (we just had to read a new buffer to get the 
	            	//actual new current sample, remember?)
	            	m_fPrevSample[0] = prev_sample[0] = cur_sample[0];
		    		m_fPrevSample[1] = prev_sample[1] = cur_sample[1];
		    	}
		    	//else... just use whatever the prev_sample already was.
            }
            else
            {
            	//at higher speeds, take the one we've already got
            	m_fPrevSample[0] = prev_sample[0];
            	m_fPrevSample[1] = prev_sample[1];
            }
            //qDebug() << m_dCurSampleIndex << "buffer" << i << buffer_int_size;
        }
        //I guess?
        if (last_read_failed)
        	break;

 		cur_sample[0] = buffer_int[(int)ceil(m_dCurSampleIndex)*2];
		cur_sample[1] = buffer_int[(int)ceil(m_dCurSampleIndex)*2+1];
		
        //Smooth any changes in the playback rate over iRateLerpLength
        //samples. This prevents the change from being discontinuous and helps
        //improve sound quality.
        if (i < iRateLerpLength) {
            rate_add = (float)i * (rate_add_diff) / (float)iRateLerpLength + rate_add_old;
        }
        else {
            rate_add = rate_add_new;
        }

		//for the current index, what percentage is it between the previous and the next?
        CSAMPLE frac = m_dCurSampleIndex - floor(m_dCurSampleIndex);

        //Perform linear interpolation
        buffer[i] = (float)prev_sample[0] + frac * ((float)cur_sample[0] - (float)prev_sample[0]);
        buffer[i+1] = (float)prev_sample[1] + frac * ((float)cur_sample[1] - (float)prev_sample[1]);
        
        /*if (i>2)
        {
        	if (fabs((buffer[i] - buffer[i-2])/buffer[i]) > 100.0)
        	{
        		qDebug() << "pop?" <<fabs((buffer[i] - buffer[i-2])/buffer[i]) <<  i << buffer[i-2] << buffer[i]  << prevIndex << m_dCurSampleIndex << prev_sample[0] <<cur_sample[0] <<frac;
        		qDebug() << buffer[i] << "=" <<prev_sample[0] << "+" << frac << "*" << (cur_sample[0] - prev_sample[0]);
        		for (int k=math_max(0,2*(int(m_dCurSampleIndex)-5)); k<math_min(buffer_int_size, 2*(int(m_dCurSampleIndex)+5)); k+=2)
        		{
        			qDebug() << buffer_int[k];
        		}
        		qDebug() << "--";
        	}
        }*/
        
        //if (buffer[i] == 0)
        //	qDebug() << i << "zero???" << prev_sample[0] << frac  <<cur_sample[0];
        	
        //at extremely low speeds, dampen the gain to hide pops and clicks
       	if (fabs(rate_add) < 0.2)
       	{
       	//	qDebug() << buffer[i] << prev_sample[0] << cur_sample[0];
       	//}
       	
       		float dither = (float)(rand() % 32768) / 32768 - 0.5; // dither
       		CSAMPLE gainfrac = fabs(rate_add) / 0.2;
       		//qDebug() << buffer[i] << dither << buffer[i] + dither;
       		buffer[i] = gainfrac * (float)buffer[i] + dither;
       		buffer[i+1] = gainfrac * (float)buffer[i+1] + dither;
       		
       		if (i>=2)
       		{
       			buffer[i] = ((float)buffer[i] + (float)buffer[i-2]) / 2;
       			buffer[i+1] = ((float)buffer[i+1] + (float)buffer[i-1]) / 2;
       		}
       	}
       	
       	
        
        
        
        /*if (rate_add_abs < 0.1)
        {
		    if (floor(prevIndex) != floor(m_dCurSampleIndex))
		    {
		    	for (int k=math_max(0, i-10); k<=i; k+=2)
		    		qDebug() << buffer[k];
		    	qDebug() << "---";
		    }
        }*/
        
        if (i < iRateLerpLength)
            m_dNextSampleIndex = m_dCurSampleIndex + fabs(rate_add);
        else
            m_dNextSampleIndex = m_dCurSampleIndex + rate_add_abs;
            
            
        /*if (rate_add_abs < 0.1)
        {
	        if (floor(m_dNextSampleIndex) != floor(m_dCurSampleIndex))
	        {
	        	for (int k=math_max(0, i-10); k<=i; k+=2)
	        		qDebug() << buffer[k];
	        }
	    }*/
            
        i+=2;
    }
    //qDebug() << "out";
    //qDebug() <<  m_dNextSampleIndex;
    // If we broke out of the loop, zero the remaining samples
    // TODO(XXX) memset
    for (; i < buf_size; i += 2) {
        buffer[i] = 0.0f;
        buffer[i+1] = 0.0f;
    }
    
    //if (zeroed)
    /*if (rate_add_abs < 0.1)
    {
    	//writer << rate_add_old << " " << rate_add_new << "\n";
    	for (int k=0; k<buf_size; k+=2)
		    writer << k << "," << buffer[k] << "\n";
    }*/
    
	Q_ASSERT(i>=buf_size);

    // It's possible that we will exit this function without having satisfied
    // this requirement. We may be trying to read past the end of the file.
    //Q_ASSERT(unscaled_samples_needed == 0);
    
    return buffer;
}
