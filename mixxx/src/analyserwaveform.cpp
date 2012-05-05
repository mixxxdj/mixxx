#include <QImage>
#include <QtDebug>
#include <QTime>
#include <QMutexLocker>
#include <QDebug>
#include <time.h>

#include "analyserwaveform.h"
#include "engine/enginefilterbutterworth8.h"
#include "engine/enginefilteriir.h"
#include "library/trackcollection.h"
#include "library/dao/analysisdao.h"
#include "trackinfoobject.h"
#include "waveform/waveformfactory.h"

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

    m_database = QSqlDatabase::addDatabase("QSQLITE", "WAVEFORM_ANALYSIS");
    if (!m_database.isOpen()) {
        m_database.setHostName("localhost");
        m_database.setDatabaseName(MIXXX_DB_PATH);
        m_database.setUserName("mixxx");
        m_database.setPassword("mixxx");

        //Open the database connection in this thread.
        if (!m_database.open()) {
            qDebug() << "Failed to open database from analyser thread."
                     << m_database.lastError();
        }
    }

    m_timer = new QTime();
    m_analysisDao = new AnalysisDao(m_database);
}

AnalyserWaveform::~AnalyserWaveform() {
    qDebug() << "AnalyserWaveform::~AnalyserWaveform()";
    destroyFilters();
    delete m_timer;
    delete m_analysisDao;
}

void AnalyserWaveform::initialise(TrackPointer tio, int sampleRate, int totalSamples) {
    m_skipProcessing = false;

    m_timer->start();
    m_waveform = tio->getWaveform();
    m_waveformSummary = tio->getWaveformSummary();

    if (!m_waveform || !m_waveformSummary || totalSamples == 0) {
        qWarning() << "AnalyserWaveform::initialise - no waveform/waveform summary";
        return;
    }

    int trackId = tio->getId();
    bool missingWaveform = m_waveform->getDataSize() == 0;
    bool missingWavesummary = m_waveformSummary->getDataSize() == 0;
    bool foundWaveform = false;
    bool foundWavesummary = false;
    bool loadedWaveform = false;
    bool loadedWavesummary = false;

    if (trackId != -1 && (missingWaveform || missingWavesummary)) {
        QList<AnalysisDao::AnalysisInfo> analyses =
                m_analysisDao->getAnalysesForTrack(trackId);

        QListIterator<AnalysisDao::AnalysisInfo> it(analyses);
        while (it.hasNext()) {
            const AnalysisDao::AnalysisInfo& analysis = it.next();

            if (analysis.type == AnalysisDao::TYPE_WAVEFORM &&
                    missingWaveform && !loadedWaveform) {
                foundWaveform = true;
                Waveform* pLoadedWaveform =
                        WaveformFactory::loadWaveformFromAnalysis(tio, analysis);
                        
                //XXX added "&& false" here to force waveform regeneration
                if (pLoadedWaveform && pLoadedWaveform->isValid() && false) {
                    //if( m_waveform->getId() != -1) {
                    m_waveform = pLoadedWaveform;
                    loadedWaveform = true;
                    tio->setWaveform(pLoadedWaveform);
                    //}
                } else {
                    delete pLoadedWaveform;
                    m_analysisDao->deleteAnalysis(analysis.analysisId);
                }
            } else if (analysis.type == AnalysisDao::TYPE_WAVESUMMARY &&
                       missingWavesummary && !loadedWavesummary) {
                foundWavesummary = true;
                Waveform* pLoadedWaveformSummary =
                        WaveformFactory::loadWaveformFromAnalysis(tio, analysis);
                //XXX added "&& false" here to force summary regeneration
                if (pLoadedWaveformSummary && pLoadedWaveformSummary->isValid() && false) {
                    //if( m_waveformSummary->getId() != -1) {
                    m_waveformSummary = pLoadedWaveformSummary;
                    tio->setWaveformSummary(pLoadedWaveformSummary);
                    loadedWavesummary = true;
                    //}
                } else {
                    delete pLoadedWaveformSummary;
                    m_analysisDao->deleteAnalysis(analysis.analysisId);
                }
            }
        }
    }

    // If we don't need to calculate the waveform/wavesummary, skip.
    if ((!missingWaveform || (missingWaveform && foundWaveform && loadedWaveform)) &&
        (!missingWavesummary || (missingWavesummary && foundWavesummary && loadedWavesummary))) {
        qDebug() << "AnalyserWaveform::initialise - Waveform loaded";
        m_skipProcessing = true;
        return;
    }

    QMutexLocker waveformLocker(m_waveform->getMutex());
    QMutexLocker waveformSummaryLocker(m_waveformSummary->getMutex());

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

    // getDataSize() of both waveform and waveformSummary are now totalSamples
    m_waveform->allocateForAudioSamples(totalSamples);
    m_waveformSummary->allocateForAudioSamples(totalSamples);
    m_waveformDataSize = m_waveform->getDataSize();
    m_waveformSummaryDataSize = m_waveformSummary->getDataSize();
    m_waveformData = &m_waveform->at(0);
    m_waveformSummaryData = &m_waveformSummary->at(0);

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
    if (m_skipProcessing || !m_waveform || !m_waveformSummary)
        return;

    //this should only append once if bufferLength is constant
    if( bufferLength > (int)m_buffers[0].size()) {
        m_buffers[Low].resize(bufferLength);
        m_buffers[Mid].resize(bufferLength);
        m_buffers[High].resize(bufferLength);
    }

    m_filter[Low]->process(buffer, &m_buffers[Low][0], bufferLength);
    m_filter[Mid]->process(buffer, &m_buffers[Mid][0], bufferLength);
    m_filter[High]->process(buffer, &m_buffers[High][0], bufferLength);

/*    CSAMPLE cmaxOver[2];
    cmaxOver[Left] = 0;
    cmaxOver[Right] = 0;*/
    CSAMPLE cmaxLow[2];
    cmaxLow[Left] = 0;
    cmaxLow[Right] = 0;
    CSAMPLE cmaxBand[2];
    cmaxBand[Left] = 0;
    cmaxBand[Right] = 0;
    CSAMPLE cmaxHigh[2];
    cmaxHigh[Left] = 0;
    cmaxHigh[Right] = 0;

    for( int i = 0; i < bufferLength; i+=2) {
        //accumulate signal power of the stride
        
        //first one didn't work
        //average is too low
        //trying max now -- hey that's good
        
        //need max of both right and left
        //remember to reset maxes when we finish the stride
        
        
        //overall seems to need accum tho
        m_stride.m_overallData[ Left] += buffer[i]*buffer[i];
        m_stride.m_overallData[Right] += buffer[i+1]*buffer[i+1];
        /*m_stride.m_filteredData[Right][ Low] += m_buffers[ Low][i  ]*m_buffers[ Low][i];
        m_stride.m_filteredData[ Left][ Low] += m_buffers[ Low][i+1]*m_buffers[ Low][i+1];
        m_stride.m_filteredData[Right][ Mid] += m_buffers[ Mid][i  ]*m_buffers[ Mid][i];
        m_stride.m_filteredData[ Left][ Mid] += m_buffers[ Mid][i+1]*m_buffers[ Mid][i+1];
        m_stride.m_filteredData[Right][High] += m_buffers[High][i  ]*m_buffers[High][i];
        m_stride.m_filteredData[ Left][High] += m_buffers[High][i+1]*m_buffers[High][i+1];*/
        
/*        m_stride.m_filteredData[Right][ Low] += m_buffers[ Low][i  ]*m_buffers[ Low][i];
        m_stride.m_filteredData[ Left][ Low] += m_buffers[ Low][i+1]*m_buffers[ Low][i+1];*/
        
        //CSAMPLE cover[2] = { buffer[i]*buffer[i], buffer[i]*buffer[i+1] };
        CSAMPLE clow[2] = { m_buffers[ Low][i  ]*m_buffers[ Low][i],  m_buffers[ Low][i+1]*m_buffers[ Low][i+1] };
        CSAMPLE cmid[2] = { m_buffers[ Mid][i  ]*m_buffers[ Mid][i], m_buffers[ Mid][i  ]*m_buffers[ Mid][i+1] };
        CSAMPLE chigh[2] = { m_buffers[High][i  ]*m_buffers[High][i], m_buffers[High][i  ]*m_buffers[High][i+1] };

        //cmaxOver[Left] = math_max(cmaxOver[Left], cover[Left]);
        cmaxLow[Left] = math_max(cmaxLow[Left], clow[Left]);
        cmaxBand[Left] = math_max(cmaxBand[Left], cmid[Left]);
        cmaxHigh[Left] = math_max(cmaxHigh[Left], chigh[Left]);
        //cmaxOver[Right] = math_max(cmaxOver[Right], cover[Right]);
        cmaxLow[Right] = math_max(cmaxLow[Right], clow[Right]);
        cmaxBand[Right] = math_max(cmaxBand[Right], cmid[Right]);
        cmaxHigh[Right] = math_max(cmaxHigh[Right], chigh[Right]);
        
        
        if( m_stride.m_position >= m_stride.m_length) {
            if (m_currentStride + ChannelCount > m_waveformDataSize) {
                qWarning() << "AnalyserWaveform::process - currentStride >= waveform size";
                return;
            }
            
            /*m_stride.m_overallData[Right] /= m_stride.m_length;
            m_stride.m_overallData[ Left] /= m_stride.m_length;
            m_stride.m_filteredData[Right][ Low] /= m_stride.m_length;
            m_stride.m_filteredData[ Left][ Low] /= m_stride.m_length;
            m_stride.m_filteredData[Right][ Mid] /= m_stride.m_length;
            m_stride.m_filteredData[ Left][ Mid] /= m_stride.m_length;
            m_stride.m_filteredData[Right][High] /= m_stride.m_length;
            m_stride.m_filteredData[ Left][High] /= m_stride.m_length;*/
            
/*            m_stride.m_overallData[ Left] = cmaxOver[Left];
            m_stride.m_overallData[Right] = cmaxOver[Right];*/
            m_stride.m_filteredData[ Left][ Low] = cmaxLow[Left];
            m_stride.m_filteredData[Right][ Low] = cmaxLow[Right];
            m_stride.m_filteredData[ Left][ Mid] = cmaxBand[Left];
            m_stride.m_filteredData[Right][ Mid] = cmaxBand[Right];
            m_stride.m_filteredData[ Left][High] = cmaxHigh[Left];
            m_stride.m_filteredData[Right][High] = cmaxHigh[Right];
            
            m_stride.store(m_waveformData + m_currentStride);

            //summary
            m_strideSummary.m_overallData[ Left] += m_stride.m_overallData[ Left];
            m_strideSummary.m_overallData[Right] += m_stride.m_overallData[Right];
            m_strideSummary.m_filteredData[ Left][ Low] += m_stride.m_filteredData[ Left][ Low];
            m_strideSummary.m_filteredData[Right][ Low] += m_stride.m_filteredData[Right][ Low];
            m_strideSummary.m_filteredData[ Left][ Mid] += m_stride.m_filteredData[ Left][ Mid];
            m_strideSummary.m_filteredData[Right][ Mid] += m_stride.m_filteredData[Right][ Mid];
            m_strideSummary.m_filteredData[ Left][High] += m_stride.m_filteredData[ Left][High];
            m_strideSummary.m_filteredData[Right][High] += m_stride.m_filteredData[Right][High];

            //NOTE: (vrince) test save main max in summary
            /*
            m_strideSummary.m_overallData[Right] = math_max(
                        m_strideSummary.m_overallData[Right],
                        m_stride.m_overallData[Right]);
            m_strideSummary.m_overallData[ Left] = math_max(
                        m_strideSummary.m_overallData[ Left],
                        m_stride.m_overallData[ Left]);
            m_strideSummary.m_filteredData[Right][ Low] = math_max(
                        m_strideSummary.m_filteredData[Right][ Low],
                        m_stride.m_filteredData[Right][ Low]);
            m_strideSummary.m_filteredData[ Left][ Low] = math_max(
                        m_strideSummary.m_filteredData[ Left][ Low],
                        m_stride.m_filteredData[ Left][ Low]);
            m_strideSummary.m_filteredData[Right][ Mid] = math_max(
                        m_strideSummary.m_filteredData[Right][ Mid],
                        m_stride.m_filteredData[Right][ Mid]);
            m_strideSummary.m_filteredData[ Left][ Mid] = math_max(
                        m_strideSummary.m_filteredData[ Left][ Mid],
                        m_stride.m_filteredData[ Left][ Mid]);
            m_strideSummary.m_filteredData[Right][High] = math_max(
                        m_strideSummary.m_filteredData[Right][High],
                        m_stride.m_filteredData[Right][High]);
            m_strideSummary.m_filteredData[ Left][High] = math_max(
                        m_strideSummary.m_filteredData[ Left][High],
                        m_stride.m_filteredData[ Left][High]);
                        */

            if( m_strideSummary.m_position >= m_strideSummary.m_length) {
                if (m_currentSummaryStride + ChannelCount > m_waveformSummaryDataSize) {
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

                m_strideSummary.store(m_waveformSummaryData + m_currentSummaryStride);
                m_strideSummary.reset();

                
/*                qDebug() << "m_strideSummary"
                         << (m_waveformSummaryData + m_currentSummaryStride)->filtered.all
                         << (m_waveformSummaryData + m_currentSummaryStride)->filtered.low
                         << (m_waveformSummaryData + m_currentSummaryStride)->filtered.mid
                         << (m_waveformSummaryData + m_currentSummaryStride)->filtered.high;*/
                         

                m_currentSummaryStride += 2;
            }
            m_strideSummary.m_position += 2;
            m_stride.reset();
/*            cmaxOver[Left] = 0;
            cmaxOver[Right] = 0;*/
            cmaxLow[Left] = 0;
            cmaxLow[Right] = 0;
            cmaxBand[Left] = 0;
            cmaxBand[Right] = 0;
            cmaxHigh[Left] = 0;
            cmaxHigh[Right] = 0;

            m_currentStride += 2;
        }
        m_stride.m_position += 2;
    }
    
/*    qDebug() << bufferLength << m_stride.m_length;
    qDebug() << cmaxLow[0] << cmaxBand[0] << cmaxHigh[0];*/
    //qDebug() << log10(cmaxOver * 10000000)*36 << log10(cmaxLow * 10000000)*36 << log10(cmaxBand * 10000000)*36 << log10(cmaxHigh * 10000000)*36;

    m_waveform->setCompletion(m_currentStride);
    m_waveformSummary->setCompletion(m_currentSummaryStride);

    //qDebug() << "AnalyserWaveform::process - m_waveform->getCompletion()" << m_waveform->getCompletion();
    //qDebug() << "AnalyserWaveform::process - m_waveformSummary->getCompletion()" << m_waveformSummary->getCompletion();
}
void AnalyserWaveform::finalise(TrackPointer tio) {
    if (m_waveform == NULL || m_waveformSummary == NULL) {
        return;
    }

    QMutexLocker waveformLocker(m_waveform->getMutex());
    QMutexLocker waveformSummaryLocker(m_waveformSummary->getMutex());

    // Force completion to waveform size
    m_waveform->setCompletion(m_waveform->getDataSize());
    m_waveformSummary->setCompletion(m_waveformSummary->getDataSize());

#ifdef TEST_HEAT_MAP
    test_heatMap->save("heatMap.png");
#endif

    if (!m_skipProcessing) {
        int trackId = tio->getId();
        Waveform* pWaveform = tio->getWaveform();
        Waveform* pWaveSummary = tio->getWaveformSummary();

        qDebug() << "Done building waveform for track" << trackId;

        if (trackId != -1 && pWaveform && pWaveSummary) {
            AnalysisDao::AnalysisInfo analysis;
            analysis.trackId = trackId;

            if (pWaveform->getId() != -1) {
                analysis.analysisId = pWaveform->getId();
            }
            analysis.type = AnalysisDao::TYPE_WAVEFORM;
            analysis.description = "Waveform 2.0";
            analysis.version = "Waveform-2.0";
            analysis.data = pWaveform->toByteArray();

            bool success = m_analysisDao->saveAnalysis(&analysis);
            qDebug() << (success ? "Saved" : "Failed to save")
                     << "waveform analysis for trackId" << trackId
                     << "analysisId" << analysis.analysisId;

            if (pWaveSummary->getId() != -1) {
                analysis.analysisId = pWaveSummary->getId();
            }

            // Clear analysisId since we are re-using the AnalysisInfo
            analysis.analysisId = -1;
            analysis.type = AnalysisDao::TYPE_WAVESUMMARY;
            analysis.description = "Waveform Summary 2.0";
            analysis.version = "WaveformSummary-2.0";
            analysis.data = pWaveSummary->toByteArray();

            success = m_analysisDao->saveAnalysis(&analysis);
            qDebug() << (success ? "Saved" : "Failed to save")
                     << "waveform summary analysis for trackId" << trackId
                     << "analysisId" << analysis.analysisId;
        }
        qDebug() << "Waveform generation done" << m_timer->elapsed()/1000.0 << "s";
    }
}
