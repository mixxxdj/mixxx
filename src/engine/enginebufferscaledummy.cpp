#include "engine/enginebufferscaledummy.h"

#include "engine/enginebufferscale.h"
#include "engine/readaheadmanager.h"
#include "sampleutil.h"


EngineBufferScaleDummy::EngineBufferScaleDummy(ReadAheadManager* pReadAheadManager)
    : EngineBufferScale(),
      m_pReadAheadManager(pReadAheadManager)
{
    m_samplesRead = 0.0;
}

EngineBufferScaleDummy::~EngineBufferScaleDummy()
{

}

double EngineBufferScaleDummy::getNewPlaypos()
{
    return m_samplesRead;
}

void EngineBufferScaleDummy::clear()
{
}


CSAMPLE* EngineBufferScaleDummy::getScaled(unsigned long buf_size) {
    m_samplesRead = 0.0;
    double rate = m_dBaseRate * m_dSpeedAdjust;
    if (rate == 0.0) {
        SampleUtil::clear(m_buffer, buf_size);
        return m_buffer;
    }
    int samples_remaining = buf_size;
    CSAMPLE* buffer_back = m_buffer;
    while (samples_remaining > 0) {
        int read_samples = m_pReadAheadManager->getNextSamples(rate,
                                                               buffer_back,
                                                               samples_remaining);
        samples_remaining -= read_samples;
        buffer_back += read_samples;
    }

    // Interpreted as number of virtual song samples consumed.
    m_samplesRead = buf_size;

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

    return m_buffer;
}
