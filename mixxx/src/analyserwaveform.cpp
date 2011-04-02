#include "analyserwaveform.h"

#include <QVector>
#include <QtDebug>

#include <time.h>
#include <limits>

#include "trackinfoobject.h"
#include "waveform/waveform.h"

#include "engine/enginefilterbutterworth8.h"
#include "engine/enginefilteriir.h"

AnalyserWaveform::AnalyserWaveform() {
    downsample = NULL;
    downsampleVector = NULL;
    m_ready = false;
    m_timePerStride = 0.0f;

    m_filter[0] = new EngineFilterButterworth8(FILTER_LOWPASS, 44100, 200);
    m_filter[1] = new EngineFilterButterworth8(FILTER_BANDPASS, 44100, 200, 2000);
    m_filter[2] = new EngineFilterButterworth8(FILTER_HIGHPASS, 44100, 2000);

    m_waveform = 0;
}

AnalyserWaveform::~AnalyserWaveform()
{
    delete m_filter[0];
    delete m_filter[1];
    delete m_filter[2];
}

void AnalyserWaveform::initialise(TrackPointer tio, int sampleRate, int totalSamples) {

    if(tio->getVisualWaveform() != NULL) {
        return;
    }

    if(totalSamples == 0) {
        return;  //?
    }

    double visualResampleRate = tio->getVisualResampleRate();
    double n = double(sampleRate) / visualResampleRate;

    QByteArray err_tmp = QString("TrackPointer %1 returned bad data: VisualResampleRate=%2, n=%3") .arg(tio->getId()).arg(visualResampleRate).arg(n).toAscii();

    if (visualResampleRate == 0 || n <= 0) {
        qDebug() << err_tmp
                 << "Track must not be loaded to a player with a waveform display."
                 << "Skipping analysis.";
        return;
    }

    int samplesPerDownsample = n;

    int numberOfVisualSamples = totalSamples / samplesPerDownsample;

    if(numberOfVisualSamples % 2 != 0)
        numberOfVisualSamples++;

    // Downsample from curSamples -> numDownsamples

    downsample = new QVector<float>(numberOfVisualSamples);
    downsampleVector = downsample->data();
    int i;

    // Set the buffer to zero
    for(i=0;i<numberOfVisualSamples;i++) {
        (*downsample)[i] = 0;
    }

    // Allow the visual waveform to display this before we've populated it so
    // that it displays the wave as we work.
    tio->setVisualWaveform(downsample);

    m_waveform = tio->getWaveForm();
    m_waveform->assign(numberOfVisualSamples,0);

    m_iStrideLength = samplesPerDownsample*2;
    m_timePerStride = samplesPerDownsample;
    m_iSamplePos = 0;

    resetCurrentStride();

    m_currentStride = 0;

    m_ready = true;

    //////////
    m_fLMax = 0.0;
    m_fRMax = 0.0;
    m_fOldLMax = 0.0f;
    m_fOldRMax = 0.0f;
    m_iStartTime = clock();
}


void AnalyserWaveform::process_old(const CSAMPLE *pIn, const int iLen) {
    if(downsample == NULL) {
        return;
    }

    qDebug() << "m_iBufferPos" << m_iStrideBufferPos << "m_iStrideLength" << m_iStrideLength << "m_iCurPos" << m_iSamplePos;


    const float fAlpha = 0.5f;

    qDebug() << "AnalyserWaveform::process() processing " << iLen << " samples";
    for(int i=0; i < iLen /*&& (downsampleVector+1-downsample->data()) < downsample->size()*/; i+=2) {

        if(m_iStrideBufferPos >= m_iStrideLength) {
            //(*downsample)[m_iCurPos] = m_fLMax;
            *(downsampleVector++) = m_fOldLMax + fAlpha * (m_fLMax - m_fOldLMax);
            m_iSamplePos++;

            //(*downsample)[m_iCurPos] = m_fRMax;
            *(downsampleVector++) = m_fOldRMax + fAlpha * (m_fRMax - m_fOldRMax);
            m_iSamplePos++;

            m_fOldRMax = m_fRMax;
            m_fOldLMax = m_fLMax;

            m_iStrideBufferPos = 0;
            m_fLMax = -1.0f;
            m_fRMax = -1.0f;

        }

        if(m_iStrideBufferPos < m_iStrideLength/*/4*/) { //m_iStrideLength) {
            CSAMPLE sl = fabs(pIn[i]);
            CSAMPLE sr = fabs(pIn[i+1]);
            if(sl > m_fLMax)
                m_fLMax = sl;
            if(sr > m_fRMax)
                m_fRMax = sr;
        }

        m_iStrideBufferPos += 2;
    }
}

void AnalyserWaveform::process(const CSAMPLE *buffer, const int bufferLength)
{
    if(!m_ready)
        return;

    QVector<float> lowBuffer(bufferLength), midBuffer(bufferLength), highBuffer(bufferLength);

    m_filter[0]->process( buffer, lowBuffer.data(), bufferLength);
    m_filter[1]->process( buffer, midBuffer.data(), bufferLength);
    m_filter[2]->process( buffer, highBuffer.data(), bufferLength);

    for( int i = 0; i < bufferLength; i+=2)
    {
        //accumulate signal power of the stride
        m_currentStridePower[0] += buffer[i]*buffer[i];
        m_currentStridePower[1] += buffer[i+1]*buffer[i+1];
        m_currentStrideFiltredPower[0][0] += lowBuffer[i]*lowBuffer[i];
        m_currentStrideFiltredPower[1][0] += lowBuffer[i+1]*lowBuffer[i+1];
        m_currentStrideFiltredPower[0][1] += midBuffer[i]*midBuffer[i];
        m_currentStrideFiltredPower[1][1] += midBuffer[i+1]*midBuffer[i+1];
        m_currentStrideFiltredPower[0][2] += highBuffer[i]*highBuffer[i];
        m_currentStrideFiltredPower[1][2] += highBuffer[i+1]*highBuffer[i+1];

        if( m_iStrideBufferPos >= m_iStrideLength)
        {
            if( m_currentStride >= m_waveform->size())
            {
                qDebug() << "m_currentStride >= m_power.size() This should never happend !";
                return;
            }

            storeCurentStridePower();
            resetCurrentStride();
            m_currentStride += 2;
        }
        m_iStrideBufferPos += 2;
    }
}

void AnalyserWaveform::storeCurentStridePower()
{
    //TODO make this a member to avoid computign it on each store
    float convertionFactor = (float)std::numeric_limits<unsigned char>::max()/m_timePerStride;
    for( int i = 0; i < 2; i++)
    {
        m_waveform->getData()[m_currentStride+i] = (unsigned char)( convertionFactor * m_currentStridePower[i]);
        m_waveform->getLowData()[m_currentStride+i] = (unsigned char)( convertionFactor * m_currentStrideFiltredPower[i][0]);
        m_waveform->getBandData()[m_currentStride+i] = (unsigned char)( convertionFactor * m_currentStrideFiltredPower[i][1]);
        m_waveform->getHighData()[m_currentStride+i] = (unsigned char)( convertionFactor * m_currentStrideFiltredPower[i][2]);
    }

}

void AnalyserWaveform::resetCurrentStride()
{
    m_iStrideBufferPos = 0;
    for( int i = 0; i < 2; i++)
    {
        m_currentStridePower[i] = 0.0f;
        for( int f = 0; f < 3; f++)
        {
            m_currentStrideFiltredPower[i][f] = 0.0f;
        }
    }
}

void AnalyserWaveform::finalise(TrackPointer tio) {
    Q_UNUSED(tio);
    if(downsample == NULL) {
        return;
    }

    downsample = NULL;
    downsampleVector = NULL;

    qDebug() << "AnalyserWaveform :: Waveform downsampling finished.";
    m_iStartTime = clock() - m_iStartTime;
    qDebug() << "AnalyserWaveform :: Generation took " << double(m_iStartTime) / CLOCKS_PER_SEC << " seconds";
}
