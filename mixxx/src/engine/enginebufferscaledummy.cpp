#include <QtCore>

#include "engine/enginebufferscaledummy.h"

#include "engine/enginebufferscale.h"
#include "engine/readaheadmanager.h"


EngineBufferScaleDummy::EngineBufferScaleDummy(ReadAheadManager* pReadAheadManager)
    : EngineBufferScale(),
      m_pReadAheadManager(pReadAheadManager)
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
    Q_UNUSED(playpos);
    Q_UNUSED(pBase);
    Q_UNUSED(iBaseLength);
    new_playpos = 0.0;
    if (m_dBaseRate * m_dTempo == 0.0f) {
        memset(buffer, 0, sizeof(CSAMPLE) * buf_size);
        return buffer;
    }
    int samples_remaining = buf_size;
    CSAMPLE* buffer_back = buffer;
    while (samples_remaining > 0) {
        int read_samples = m_pReadAheadManager->getNextSamples(m_dBaseRate*m_dTempo,
                                                               buffer_back,
                                                               samples_remaining);
        samples_remaining -= read_samples;
        buffer_back += read_samples;
    }

    // Interpreted as number of virtual song samples consumed.
    new_playpos = buf_size;

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
