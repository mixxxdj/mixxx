#include <QtCore>
#include "enginebufferscale.h"
#include "enginebufferscaledummy.h"


EngineBufferScaleDummy::EngineBufferScaleDummy() : EngineBufferScale()
{
	new_playpos = 0.0f;
}

EngineBufferScaleDummy::~EngineBufferScaleDummy()
{

}

void EngineBufferScaleDummy::setBaseRate(double baserate)
{
	m_dBaseRate = baserate;
}

double EngineBufferScaleDummy::setTempo(double tempo)
{
	m_dTempo = tempo;
	return m_dTempo;
}

double EngineBufferScaleDummy::getNewPlaypos()
{
	return new_playpos;
}

void EngineBufferScaleDummy::clear()
{
}

/**
 * @param playpos The play position in the EngineBuffer (in samples)
 * @param buf_size The size of the audio buffer to scale (in samples)
 * @param pBase
 * @param iBaseLength (same units as playpos)
 */
CSAMPLE *EngineBufferScaleDummy::scale(double playpos,
                                       unsigned long buf_size,
                                       CSAMPLE* pBase,
                                       unsigned long iBaseLength)
{
	unsigned long baseplaypos = ((long)playpos) % iBaseLength; // Playpos wraps within the base buffer
													  // This is the position within base

	long numSamplesToCopy = buf_size; // If we can copy a whole chunk
	if ((baseplaypos + buf_size) > iBaseLength) 	  // At the end of a buffer, only copy as much as we can fit
		numSamplesToCopy = (iBaseLength - baseplaypos); // Copy however many samples are left
		
	// Write to the modded position inside the base
    // also remember to convert to bytes from samples
	memcpy(buffer, &pBase[baseplaypos], numSamplesToCopy * sizeof(CSAMPLE));

	//If we've hit the end of the circular "base" buffer, copy the remaining samples
	//that we need from the start of the circular buffer over.  
	//In other words, pBase is a circular buffer and we need exactly buf_size 
    //samples so we take some from the beggining
	if (numSamplesToCopy < buf_size)
	{
	    //qDebug() << "Filling the rest of the buffer: " << numSamplesToCopy << buf_size - numSamplesToCopy;
	    memcpy(&buffer[numSamplesToCopy], &pBase[0], (buf_size - numSamplesToCopy) * sizeof(CSAMPLE));
	    numSamplesToCopy = buf_size;
	}

	//Update the "play position"
	new_playpos = ((long)(playpos + numSamplesToCopy*m_dBaseRate*m_dTempo));
	
/*
        //START OF BASIC/ROCKSOLID LINEAR INTERPOLATION CODE
        
        //This code was ripped from EngineBufferScaleLinear so we could experiment
        //with it and understand how it works. We also wanted to test to see if this
        //minimal subset of the code was stable, and it is. -- Albert 04/23/08
         
        float rate_add = 2 * m_dBaseRate * m_dTempo; //2 channels * baserate * tempo
        int i;
        new_playpos = playpos;
        for (i=0; i<buf_size; i+=2)
        {
            long prev = (long)floor(new_playpos)%READBUFFERSIZE;
            if (prev % 2 != 0) prev--;

            long next = (prev+2)%READBUFFERSIZE;

            float frac = new_playpos - floor(new_playpos);
            buffer[i  ] = wavebuffer[prev  ] + frac*(wavebuffer[next  ]-wavebuffer[prev  ]);
            buffer[i+1] = wavebuffer[prev+1] + frac*(wavebuffer[next+1]-wavebuffer[prev+1]);

            new_playpos += rate_add;
        }
        */
        //END OF LINEAR INTERPOLATION CODE

	//qDebug() << iBaseLength << playpos << new_playpos << buf_size << numSamplesToCopy;

	return buffer;
}
