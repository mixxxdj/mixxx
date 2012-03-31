#include "analyserwaveform.h"

#include <QtDebug>

#include <time.h>

#include "trackinfoobject.h"

#include "engine/enginefilterbutterworth8.h"
#include "engine/enginefilteriir.h"

#include "library/dao/waveformdao.h"

#include <QTime>
#include <QDebug>

//test (vrince)
#include <QImage>

AnalyserWaveform::AnalyserWaveform() {
    qDebug() << "AnalyserWaveform::AnalyserWaveform()";
    m_skipProcessing = false;

    m_waveform = 0;
    m_waveformSummary = 0;

    m_filter[0] = 0;
    m_filter[1] = 0;
    m_filter[2] = 0;

    m_currentStride = 0;
    m_currentSummaryStride = 0;

    m_timer = new QTime();
    m_waveformDao = new WaveformDao();

    m_waveformDao->setDatabase( QSqlDatabase::database("mixxx_sql_connection"));
}

AnalyserWaveform::~AnalyserWaveform() {
    qDebug() << "AnalyserWaveform::~AnalyserWaveform()";
    destroyFilters();
    delete m_timer;
    delete m_waveformDao;
}

void AnalyserWaveform::initialise(TrackPointer tio, int sampleRate, int totalSamples) {

    m_skipProcessing = false;

    m_timer->start();
    m_waveform = tio->getWaveform();
    m_waveformSummary = tio->getWaveformSummary();

    if( !m_waveform || !m_waveformSummary || totalSamples == 0) {
        qWarning() << "AnalyserWaveform::initialise - no waveform/waveform summary";
        return;
    }

    //pre waveform existance test
    if( m_waveformDao->getWaveform(*(tio.data()))) {
        qDebug() << "AnalyserWaveform::initialise - Waveform loaded";
        m_skipProcessing = true;
        return;
    }

    m_waveform->getMutex()->lock();
    m_waveformSummary->getMutex()->lock();

    destroyFilters();
    resetFilters(tio);

    //TODO (vrince) Do we want to expose this as settings or whatever ?
    const double mainWaveformSampleRate = 441;
    //two visual sample per pixel in full width overview in full hd
    int summaryWaveformSamples = 2*1920;

    const double summaryWaveformSampleRate = (double)summaryWaveformSamples * (double)sampleRate / (double)totalSamples;

    //qDebug() << summaryWaveformSampleRate;

    m_waveform->computeBestVisualSampleRate(sampleRate,mainWaveformSampleRate);
    m_waveformSummary->computeBestVisualSampleRate(sampleRate,summaryWaveformSampleRate);

    m_waveform->allocateForAudioSamples(totalSamples);
    m_waveformSummary->allocateForAudioSamples(totalSamples);

    m_stride.init(m_waveform->getAudioSamplesPerVisualSample());
    const double mainSummaryRatio = m_waveform->getVisualSampleRate() / m_waveformSummary->getVisualSampleRate();
    const int summaryStrideLength = ceil(mainSummaryRatio);
    m_strideSummary.init(summaryStrideLength);
    m_strideSummary.m_convertionFactor /= (double)mainSummaryRatio;

    m_currentStride = 0;
    m_currentSummaryStride = 0;

    //debug
    //m_waveform->dump();
    //m_waveformSummary->dump();

#ifdef TEST_HEAT_MAP
    test_heatMap = new QImage(256,256,QImage::Format_RGB32);
    test_heatMap->fill(0xFFFFFFFF);
#endif

    m_waveform->getMutex()->unlock();
    m_waveformSummary->getMutex()->unlock();
}

void AnalyserWaveform::resetFilters(TrackPointer tio) {
    //TODO: (vRince) bind this with *actual* filter values ...
    m_filter[Low] = new EngineFilterButterworth8(FILTER_LOWPASS, tio->getSampleRate(), 200);
    m_filter[Mid] = new EngineFilterButterworth8(FILTER_BANDPASS, tio->getSampleRate(), 200, 2000);
    m_filter[High] = new EngineFilterButterworth8(FILTER_HIGHPASS, tio->getSampleRate(), 2000);
}

void AnalyserWaveform::destroyFilters() {
    for( int i = 0; i < FilterCount; i++) {
        if( m_filter[i]) {
            delete m_filter[i];
            m_filter[i] = 0;
        }
    }
}

void AnalyserWaveform::process(const CSAMPLE *buffer, const int bufferLength) {
    if( m_skipProcessing || !m_waveform || !m_waveformSummary)
        return;

    //this should only append once if bufferLength is constant
    if( bufferLength > (int)m_buffers[0].size()) {
        m_buffers[Low].resize(bufferLength);
        m_buffers[Mid].resize(bufferLength);
        m_buffers[High].resize(bufferLength);
    }

    m_filter[Low]->process( buffer, &m_buffers[Low][0], bufferLength);
    m_filter[Mid]->process( buffer, &m_buffers[Mid][0], bufferLength);
    m_filter[High]->process( buffer, &m_buffers[High][0], bufferLength);

    for( int i = 0; i < bufferLength; i+=2) {
        //accumulate signal power of the stride
        m_stride.m_overallData[Right] += buffer[i]*buffer[i];
        m_stride.m_overallData[ Left] += buffer[i+1]*buffer[i+1];
        m_stride.m_filteredData[Right][ Low] += m_buffers[ Low][i  ]*m_buffers[ Low][i];
        m_stride.m_filteredData[ Left][ Low] += m_buffers[ Low][i+1]*m_buffers[ Low][i+1];
        m_stride.m_filteredData[Right][ Mid] += m_buffers[ Mid][i  ]*m_buffers[ Mid][i];
        m_stride.m_filteredData[ Left][ Mid] += m_buffers[ Mid][i+1]*m_buffers[ Mid][i+1];
        m_stride.m_filteredData[Right][High] += m_buffers[High][i  ]*m_buffers[High][i];
        m_stride.m_filteredData[ Left][High] += m_buffers[High][i+1]*m_buffers[High][i+1];

        if( m_stride.m_position >= m_stride.m_length) {
            if( m_currentStride >= m_waveform->getDataSize()) {
                qWarning() << "AnalyserWaveform::process - currentStride >= waveform size";
                return;
            }

            m_stride.store(m_waveform,m_currentStride);

            //summary
            m_strideSummary.m_overallData[Right] += m_stride.m_overallData[Right];
            m_strideSummary.m_overallData[ Left] += m_stride.m_overallData[ Left];
            m_strideSummary.m_filteredData[Right][ Low] += m_stride.m_filteredData[Right][ Low];
            m_strideSummary.m_filteredData[ Left][ Low] += m_stride.m_filteredData[ Left][ Low];
            m_strideSummary.m_filteredData[Right][ Mid] += m_stride.m_filteredData[Right][ Mid];
            m_strideSummary.m_filteredData[ Left][ Mid] += m_stride.m_filteredData[ Left][ Mid];
            m_strideSummary.m_filteredData[Right][High] += m_stride.m_filteredData[Right][High];
            m_strideSummary.m_filteredData[ Left][High] += m_stride.m_filteredData[ Left][High];

            if( m_strideSummary.m_position >= m_strideSummary.m_length) {
                if( m_currentSummaryStride >= m_waveformSummary->getDataSize()) {
                    qWarning() << "AnalyserWaveform::process - current summary stride >= waveform summary size";
                    return;
                }

#ifdef TEST_HEAT_MAP
                QPointF point(float(m_strideSummary.m_filteredData[Right][High]),
                              float(m_strideSummary.m_filteredData[Right][ Mid]));

                float norm = sqrt(point.x()*point.x() + point.y()*point.y());
                point /= norm;

                point *= m_strideSummary.m_filteredData[Right][ Low];
                test_heatMap->setPixel(point.toPoint(),0xFF0000FF);
#endif

                m_strideSummary.store(m_waveformSummary,m_currentSummaryStride);
                m_strideSummary.reset();
                m_currentSummaryStride += 2;
            }
            m_strideSummary.m_position += 2;
            m_stride.reset();
            m_currentStride += 2;
        }
        m_stride.m_position += 2;
    }

    m_waveform->setCompletion(m_currentStride);
    m_waveformSummary->setCompletion(m_currentSummaryStride);

    //qDebug() << "AnalyserWaveform::process - m_waveform->getCompletion()" << m_waveform->getCompletion();
    //qDebug() << "AnalyserWaveform::process - m_waveformSummary->getCompletion()" << m_waveformSummary->getCompletion();
}
void AnalyserWaveform::finalise(TrackPointer tio) {
    //Force completion to waveform size
    m_waveform->setCompletion(m_waveform->getDataSize());
    m_waveformSummary->setCompletion(m_waveformSummary->getDataSize());

#ifdef TEST_HEAT_MAP
    test_heatMap->save("heatMap.png");
#endif

    if( !m_skipProcessing) {
        bool waveformSaved = m_waveformDao->saveWaveform(*(tio.data()));
        qDebug() << "AnalyserWaveform::finalise - Waveform saved :" << waveformSaved;
    }

    qDebug() << "Waveform gerenration done" << m_timer->elapsed()/1000.0 << "s";

}
