#include <QVector>
#include <QtDebug>
#include <time.h>

#include "trackinfoobject.h"
#include "analyserwaveform.h"


AnalyserWaveform::AnalyserWaveform() {
    downsample = NULL;
    downsampleVector = NULL;
}

void AnalyserWaveform::initialise(TrackPointer tio, int sampleRate, int totalSamples) {

    if(tio->getVisualWaveform() != NULL) {
        return;
    }

    if(totalSamples == 0) {
        return;  //?
    }

    double mz = tio->getVisualResampleRate();
    double n = double(sampleRate) / mz;

    QByteArray err_tmp = QString("TrackPointer %1 returned bad data: VisualResampleRate=%2, n=%3") .arg(tio->getId()).arg(mz).arg(n).toAscii();

    if (mz == 0 || n <= 0) {
        qDebug() << err_tmp
                 << "Track must not be loaded to a player with a waveform display."
                 << "Skipping analysis.";
        return;
    }

    int samplesPerDownsample = n;
    int numDownsamples = totalSamples / samplesPerDownsample;

    if(numDownsamples % 2 != 0)
        numDownsamples++;

    // Downsample from curSamples -> numDownsamples

    downsample = new QVector<float>(numDownsamples);
    downsampleVector = downsample->data();
    int i;

    // Set the buffer to zero
    for(i=0;i<numDownsamples;i++) {
        (*downsample)[i] = 0;
    }

    // Allow the visual waveform to display this before we've populated it so
    // that it displays the wave as we work.
    tio->setVisualWaveform(downsample);

    //qDebug() << "AnalyserWaveform: f " << sampleRate << " samplesPerDownsample: " << samplesPerDownsample << " downsamples " << numDownsamples << " from " << totalSamples;

    m_iStrideLength = samplesPerDownsample*2;
    m_iCurPos = 0;
    m_iBufferPos = 0;
    m_fLMax = -1.0;
    m_fRMax = -1.0;

    m_iStartTime = clock();
}


void AnalyserWaveform::process(const CSAMPLE *pIn, const int iLen) {
    if(downsample == NULL) {
        return;
    }

    //qDebug() << "AnalyserWaveform::process() processing " << iLen << " samples";
    for(int i=0; i<iLen; i+=2) {

        if(m_iBufferPos >= m_iStrideLength) {
            //(*downsample)[m_iCurPos] = m_fLMax;
            *(downsampleVector++) = m_fLMax;
            m_iCurPos++;

            //(*downsample)[m_iCurPos] = m_fRMax;
            *(downsampleVector++) = m_fRMax;
            m_iCurPos++;

            m_iBufferPos = 0;
            m_fLMax = -1.0f;
            m_fRMax = -1.0f;
        }
        CSAMPLE sl = fabs(pIn[i]);
        CSAMPLE sr = fabs(pIn[i+1]);

        if(m_iBufferPos <= 20) {
            if(sl > m_fLMax)
                m_fLMax = sl;
            if(sr > m_fRMax)
                m_fRMax = sr;
        }

        m_iBufferPos += 2;
    }
}

void AnalyserWaveform::finalise(TrackPointer tio) {
    Q_UNUSED(tio);
    if(downsample == NULL) {
        return;
    }

    downsample = NULL;
    downsampleVector = NULL;

    //qDebug() << "AnalyserWaveform :: Waveform downsampling finished.";
    m_iStartTime = clock() - m_iStartTime;
    //qDebug() << "AnalyserWaveform :: Generation took " << double(m_iStartTime) / CLOCKS_PER_SEC << " seconds";
}
