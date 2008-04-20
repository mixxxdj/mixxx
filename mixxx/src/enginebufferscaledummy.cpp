#include <QtCore>
#include "enginebufferscale.h"
#include "readerextractwave.h"
#include "enginebufferscaledummy.h"


EngineBufferScaleDummy::EngineBufferScaleDummy(ReaderExtractWave *_wave) : EngineBufferScale(_wave)
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
 ,* @param iBaseLength (same units as playpos)
 */
CSAMPLE *EngineBufferScaleDummy::scale(double playpos, int buf_size, float *pBase, int iBaseLength)
{
	//Guessing at this for now...

    if (!pBase)
    {
        pBase = wavebuffer;
        iBaseLength = READBUFFERSIZE;
    }
	
	//I figured out what this memcpy should be based on looking at EngineBufferScaleSRC...
	//(That could be a bad thing though)
	
	long numBytesToCopy = buf_size * sizeof(CSAMPLE); //Convert the buffer size to bytes from samples
	if ((playpos + buf_size) > iBaseLength) 	  //At the end of a buffer, only copy as much as we can fit
		numBytesToCopy = (iBaseLength - (long)playpos)* sizeof(CSAMPLE); //Remembering to convert to samples
		
	memcpy(buffer, &pBase[(long)playpos], numBytesToCopy );

	//Update the "play position"
	long numSamplesCopied = numBytesToCopy / sizeof(CSAMPLE); //Number of samples we scaled...
	new_playpos = ((long)(playpos + numSamplesCopied*m_dBaseRate*m_dTempo));

	if (new_playpos == iBaseLength)
		new_playpos = 0;

    //Whoa, you can do cool looping if you mess with new_playpos (like mod (%) it with READBUFFER_SIZE)...

/*
        //START OF LINEAR INTERPOLATION CODE
        //IF YOU WANT TO USE/FIX THIS, comment out the memcpy and "new_playpos = ..." lines above this.
        // - Albert

        qDebug() << m_dTempo << m_dBaseRate;

        int rate_add = 2 * m_dBaseRate * m_dTempo; //2 channels * baserate
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

	qDebug() << iBaseLength << playpos << new_playpos << buf_size << numBytesToCopy;

	return buffer;
}
