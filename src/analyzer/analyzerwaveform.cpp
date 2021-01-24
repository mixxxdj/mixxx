#include "analyzer/analyzerwaveform.h"

#include "engine/engineobject.h"
#include "engine/filters/enginefilterbessel4.h"
#include "engine/filters/enginefilterbutterworth8.h"
#include "library/trackcollection.h"
#include "track/track.h"
#include "util/logger.h"
#include "waveform/waveformfactory.h"

namespace {

mixxx::Logger kLogger("AnalyzerWaveform");

} // namespace

AnalyzerWaveform::AnalyzerWaveform(
        UserSettingsPointer pConfig,
        const QSqlDatabase& dbConnection)
        : m_analysisDao(pConfig),
          m_waveformData(nullptr),
          m_waveformSummaryData(nullptr),
          m_stride(0, 0),
          m_currentStride(0),
          m_currentSummaryStride(0) {
    m_filter[0] = nullptr;
    m_filter[1] = nullptr;
    m_filter[2] = nullptr;
    m_analysisDao.initialize(dbConnection);
}

AnalyzerWaveform::~AnalyzerWaveform() {
    kLogger.debug() << "~AnalyzerWaveform():";
    destroyFilters();
}

bool AnalyzerWaveform::initialize(TrackPointer tio, int sampleRate, int totalSamples) {
    if (totalSamples == 0) {
        qWarning() << "AnalyzerWaveform::initialize - no waveform/waveform summary";
        return false;
    }

    // If we don't need to calculate the waveform/wavesummary, skip.
    if (!shouldAnalyze(tio)) {
        return false;
    }

    m_timer.start();

    // Now actually initialize the AnalyzerWaveform:
    destroyFilters();
    createFilters(sampleRate);

    //TODO (vrince) Do we want to expose this as settings or whatever ?
    const int mainWaveformSampleRate = 441;
    // two visual sample per pixel in full width overview in full hd
    const int summaryWaveformSamples = 2 * 1920;

    m_waveform = WaveformPointer(new Waveform(
            sampleRate, totalSamples, mainWaveformSampleRate, -1));
    m_waveformSummary = WaveformPointer(new Waveform(
            sampleRate, totalSamples, mainWaveformSampleRate, summaryWaveformSamples));

    // Now, that the Waveform memory is initialized, we can set set them to
    // the TIO. Be aware that other threads of Mixxx can touch them from
    // now.
    tio->setWaveform(m_waveform);
    tio->setWaveformSummary(m_waveformSummary);

    m_waveformData = m_waveform->data();
    m_waveformSummaryData = m_waveformSummary->data();

    m_stride = WaveformStride(m_waveform->getAudioVisualRatio(),
            m_waveformSummary->getAudioVisualRatio());

    m_currentStride = 0;
    m_currentSummaryStride = 0;

    //debug
    //m_waveform->dump();
    //m_waveformSummary->dump();

#ifdef TEST_HEAT_MAP
    test_heatMap = new QImage(256, 256, QImage::Format_RGB32);
    test_heatMap->fill(0xFFFFFFFF);
#endif
    return true;
}

bool AnalyzerWaveform::shouldAnalyze(TrackPointer tio) const {
    ConstWaveformPointer pTrackWaveform = tio->getWaveform();
    ConstWaveformPointer pTrackWaveformSummary = tio->getWaveformSummary();
    ConstWaveformPointer pLoadedTrackWaveform;
    ConstWaveformPointer pLoadedTrackWaveformSummary;

    TrackId trackId = tio->getId();
    bool missingWaveform = pTrackWaveform.isNull();
    bool missingWavesummary = pTrackWaveformSummary.isNull();

    if (trackId.isValid() && (missingWaveform || missingWavesummary)) {
        QList<AnalysisDao::AnalysisInfo> analyses =
                m_analysisDao.getAnalysesForTrack(trackId);

        QListIterator<AnalysisDao::AnalysisInfo> it(analyses);
        while (it.hasNext()) {
            const AnalysisDao::AnalysisInfo& analysis = it.next();
            WaveformFactory::VersionClass vc;

            if (analysis.type == AnalysisDao::TYPE_WAVEFORM) {
                vc = WaveformFactory::waveformVersionToVersionClass(analysis.version);
                if (missingWaveform && vc == WaveformFactory::VC_USE) {
                    pLoadedTrackWaveform = ConstWaveformPointer(
                            WaveformFactory::loadWaveformFromAnalysis(analysis));
                    missingWaveform = false;
                } else if (vc != WaveformFactory::VC_KEEP) {
                    // remove all other Analysis except that one we should keep
                    m_analysisDao.deleteAnalysis(analysis.analysisId);
                }
            }
            if (analysis.type == AnalysisDao::TYPE_WAVESUMMARY) {
                vc = WaveformFactory::waveformSummaryVersionToVersionClass(analysis.version);
                if (missingWavesummary && vc == WaveformFactory::VC_USE) {
                    pLoadedTrackWaveformSummary = ConstWaveformPointer(
                            WaveformFactory::loadWaveformFromAnalysis(analysis));
                    missingWavesummary = false;
                } else if (vc != WaveformFactory::VC_KEEP) {
                    // remove all other Analysis except that one we should keep
                    m_analysisDao.deleteAnalysis(analysis.analysisId);
                }
            }
        }
    }

    // If we don't need to calculate the waveform/wavesummary, skip.
    if (!missingWaveform && !missingWavesummary) {
        kLogger.debug() << "loadStored - Stored waveform loaded";
        if (pLoadedTrackWaveform) {
            tio->setWaveform(pLoadedTrackWaveform);
        }
        if (pLoadedTrackWaveformSummary) {
            tio->setWaveformSummary(pLoadedTrackWaveformSummary);
        }
        return false;
    }
    return true;
}

void AnalyzerWaveform::createFilters(int sampleRate) {
    // m_filter[Low] = new EngineFilterButterworth8(FILTER_LOWPASS, sampleRate, 200);
    // m_filter[Mid] = new EngineFilterButterworth8(FILTER_BANDPASS, sampleRate, 200, 2000);
    // m_filter[High] = new EngineFilterButterworth8(FILTER_HIGHPASS, sampleRate, 2000);
    m_filter[Low] = new EngineFilterBessel4Low(sampleRate, 600);
    m_filter[Mid] = new EngineFilterBessel4Band(sampleRate, 600, 4000);
    m_filter[High] = new EngineFilterBessel4High(sampleRate, 4000);
    // settle filters for silence in preroll to avoids ramping (Bug #1406389)
    for (int i = 0; i < FilterCount; ++i) {
        m_filter[i]->assumeSettled();
    }
}

void AnalyzerWaveform::destroyFilters() {
    for (int i = 0; i < FilterCount; ++i) {
        if (m_filter[i]) {
            delete m_filter[i];
            m_filter[i] = nullptr;
        }
    }
}

bool AnalyzerWaveform::processSamples(const CSAMPLE* buffer, const int bufferLength) {
    VERIFY_OR_DEBUG_ASSERT(m_waveform) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(m_waveformSummary) {
        return false;
    }

    //this should only append once if bufferLength is constant
    if (bufferLength > (int)m_buffers[0].size()) {
        m_buffers[Low].resize(bufferLength);
        m_buffers[Mid].resize(bufferLength);
        m_buffers[High].resize(bufferLength);
    }

    m_filter[Low]->process(buffer, &m_buffers[Low][0], bufferLength);
    m_filter[Mid]->process(buffer, &m_buffers[Mid][0], bufferLength);
    m_filter[High]->process(buffer, &m_buffers[High][0], bufferLength);

    m_waveform->setSaveState(Waveform::SaveState::NotSaved);
    m_waveformSummary->setSaveState(Waveform::SaveState::NotSaved);

    for (int i = 0; i < bufferLength; i += 2) {
        // Take max value, not average of data
        CSAMPLE cover[2] = {fabs(buffer[i]), fabs(buffer[i + 1])};
        CSAMPLE clow[2] = {fabs(m_buffers[Low][i]), fabs(m_buffers[Low][i + 1])};
        CSAMPLE cmid[2] = {fabs(m_buffers[Mid][i]), fabs(m_buffers[Mid][i + 1])};
        CSAMPLE chigh[2] = {fabs(m_buffers[High][i]), fabs(m_buffers[High][i + 1])};

        // This is for if you want to experiment with averaging instead of
        // maxing.
        // m_stride.m_overallData[Right] += buffer[i]*buffer[i];
        // m_stride.m_overallData[Left] += buffer[i + 1]*buffer[i + 1];
        // m_stride.m_filteredData[Right][Low] += m_buffers[Low][i]*m_buffers[Low][i];
        // m_stride.m_filteredData[Left][Low] += m_buffers[Low][i + 1]*m_buffers[Low][i + 1];
        // m_stride.m_filteredData[Right][Mid] += m_buffers[Mid][i]*m_buffers[Mid][i];
        // m_stride.m_filteredData[Left][Mid] += m_buffers[Mid][i + 1]*m_buffers[Mid][i + 1];
        // m_stride.m_filteredData[Right][High] += m_buffers[High][i]*m_buffers[High][i];
        // m_stride.m_filteredData[Left][High] += m_buffers[High][i + 1]*m_buffers[High][i + 1];

        // Record the max across this stride.
        storeIfGreater(&m_stride.m_overallData[Left], cover[Left]);
        storeIfGreater(&m_stride.m_overallData[Right], cover[Right]);
        storeIfGreater(&m_stride.m_filteredData[Left][Low], clow[Left]);
        storeIfGreater(&m_stride.m_filteredData[Right][Low], clow[Right]);
        storeIfGreater(&m_stride.m_filteredData[Left][Mid], cmid[Left]);
        storeIfGreater(&m_stride.m_filteredData[Right][Mid], cmid[Right]);
        storeIfGreater(&m_stride.m_filteredData[Left][High], chigh[Left]);
        storeIfGreater(&m_stride.m_filteredData[Right][High], chigh[Right]);

        m_stride.m_position++;

        if (fmod(m_stride.m_position, m_stride.m_length) < 1) {
            VERIFY_OR_DEBUG_ASSERT(m_currentStride + ChannelCount <= m_waveform->getDataSize()) {
                qWarning() << "AnalyzerWaveform::process - currentStride > waveform size";
                return false;
            }
            m_stride.store(m_waveformData + m_currentStride);
            m_currentStride += ChannelCount;
            m_waveform->setCompletion(m_currentStride);
        }

        if (fmod(m_stride.m_position, m_stride.m_averageLength) < 1) {
            VERIFY_OR_DEBUG_ASSERT(m_currentSummaryStride + ChannelCount <= m_waveformSummary->getDataSize()) {
                qWarning() << "AnalyzerWaveform::process - current summary stride > waveform summary size";
                return false;
            }
            m_stride.averageStore(m_waveformSummaryData + m_currentSummaryStride);
            m_currentSummaryStride += ChannelCount;
            m_waveformSummary->setCompletion(m_currentSummaryStride);

#ifdef TEST_HEAT_MAP
            QPointF point(m_stride.m_filteredData[Right][High],
                    m_stride.m_filteredData[Right][Mid]);

            float norm = sqrt(point.x() * point.x() + point.y() * point.y());
            point /= norm;

            point *= m_stride.m_filteredData[Right][Low];
            test_heatMap->setPixel(point.toPoint(), 0xFF0000FF);
#endif
        }
    }

    //kLogger.debug() << "process - m_waveform->getCompletion()" << m_waveform->getCompletion() << "off" << m_waveform->getDataSize();
    //kLogger.debug() << "process - m_waveformSummary->getCompletion()" << m_waveformSummary->getCompletion() << "off" << m_waveformSummary->getDataSize();
    return true;
}

void AnalyzerWaveform::cleanup() {
    m_waveform.clear();
    m_waveformData = nullptr;
    m_waveformSummary.clear();
    m_waveformSummaryData = nullptr;
}

void AnalyzerWaveform::storeResults(TrackPointer tio) {
    // Force completion to waveform size
    if (m_waveform) {
        m_waveform->setSaveState(Waveform::SaveState::SavePending);
        m_waveform->setCompletion(m_waveform->getDataSize());
        m_waveform->setVersion(WaveformFactory::currentWaveformVersion());
        m_waveform->setDescription(WaveformFactory::currentWaveformDescription());
    }
    tio->setWaveform(m_waveform);

    // Force completion to waveform size
    if (m_waveformSummary) {
        m_waveformSummary->setSaveState(Waveform::SaveState::SavePending);
        m_waveformSummary->setCompletion(m_waveformSummary->getDataSize());
        m_waveformSummary->setVersion(WaveformFactory::currentWaveformSummaryVersion());
        m_waveformSummary->setDescription(WaveformFactory::currentWaveformSummaryDescription());
    }
    tio->setWaveformSummary(m_waveformSummary);

#ifdef TEST_HEAT_MAP
    test_heatMap->save("heatMap.png");
#endif
    // Ensure that the analyses get saved. This is also called from
    // TrackDAO.updateTrack(), but it can happen that we analyze only the
    // waveforms (i.e. if the config setting was disabled in a previous scan)
    // and then it is not called. The other analyzers have signals which control
    // the update of their data.
    m_analysisDao.saveTrackAnalyses(
            tio->getId(),
            m_waveform,
            m_waveformSummary);

    kLogger.debug() << "Waveform generation for track" << tio->getId() << "done"
                    << m_timer.elapsed().debugSecondsWithUnit();
}

void AnalyzerWaveform::storeIfGreater(float* pDest, float source) {
    if (*pDest < source) {
        *pDest = source;
    }
}
