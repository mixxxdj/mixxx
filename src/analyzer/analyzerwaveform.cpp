#include "analyzer/analyzerwaveform.h"

#include <algorithm>
#include <memory>
#include <vector>

#include "analyzer/analyzertrack.h"
#include "analyzer/constants.h"
#include "engine/filters/enginefilterbessel4.h"
#include "track/track.h"
#include "util/logger.h"
#include "waveform/waveform.h"
#include "waveform/waveformfactory.h"
#include "waveform/widgets/waveformwidgettype.h"

namespace {

mixxx::Logger kLogger("AnalyzerWaveform");

const ConfigKey kWaveformTypeKey(
        QStringLiteral("[Waveform]"), QStringLiteral("WaveformType"));

constexpr double kLegacyLowMidFreqHz = 600.0;
constexpr double kLegacyMidHighFreqHz = 4000.0;

constexpr double kPerceptualLowHighFreqHz = 300.0;
constexpr double kPerceptualMidLowFreqHz = 230.0;
constexpr double kPerceptualMidHighFreqHz = 1320.0;
constexpr double kPerceptualHighLowFreqHz = 3340.0;
constexpr double kPerceptualHighHighFreqHz = 8420.0;

} // namespace

AnalyzerWaveform::AnalyzerWaveform(
        UserSettingsPointer pConfig,
        const QSqlDatabase& dbConnection)
        : m_pConfig(pConfig),
          m_analysisProfile(mixxx::WaveformAnalysisProfile::Legacy),
          m_analysisDao(pConfig),
          m_waveformData(nullptr),
          m_waveformSummaryData(nullptr),
          m_legacyStride(0, 0, 0),
          m_perceptualStride(0, 0, 0, 0),
          m_currentStride(0),
          m_currentSummaryStride(0) {
    m_analysisDao.initialize(dbConnection);
}

AnalyzerWaveform::~AnalyzerWaveform() {
    kLogger.debug() << "~AnalyzerWaveform():";
    destroyFilters();
}

bool AnalyzerWaveform::initialize(const AnalyzerTrack& track,
        mixxx::audio::SampleRate sampleRate,
        mixxx::audio::ChannelCount channelCount,
        SINT frameLength) {
    if (frameLength <= 0) {
        qWarning() << "AnalyzerWaveform::initialize - no waveform/waveform summary";
        return false;
    }

    const auto waveformType = static_cast<WaveformWidgetType::Type>(m_pConfig->getValue(
            kWaveformTypeKey,
            static_cast<int>(WaveformWidgetType::RGB)));
    m_analysisProfile = WaveformWidgetType::analysisProfile(waveformType);

    // If we don't need to calculate the waveform/wavesummary, skip.
    if (!shouldAnalyze(track.getTrack())) {
        return false;
    }

    m_timer.start();

    destroyFilters();
    createFilters(sampleRate);

    constexpr int mainWaveformSampleRate = 441;
    constexpr int summaryWaveformSamples = 2 * 1920;

    int stemCount = channelCount == mixxx::kAnalysisChannels
            ? 0
            : channelCount / mixxx::kAnalysisChannels;
    m_waveform = WaveformPointer(new Waveform(
            sampleRate, frameLength, mainWaveformSampleRate, -1, stemCount));
    m_waveformSummary = WaveformPointer(new Waveform(
            sampleRate, frameLength, mainWaveformSampleRate, summaryWaveformSamples, stemCount));
    // Now, that the Waveform memory is initialized, we can set set them to
    // the track. Be aware that other threads of Mixxx can touch them from
    // now.
    track.getTrack()->setWaveform(m_waveform);
    track.getTrack()->setWaveformSummary(m_waveformSummary);

    m_waveformData = m_waveform->data();
    m_waveformSummaryData = m_waveformSummary->data();

    if (m_analysisProfile == mixxx::WaveformAnalysisProfile::Perceptual3Band) {
        m_perceptualStride = PerceptualWaveformStride(m_waveform->getAudioVisualRatio(),
                m_waveformSummary->getAudioVisualRatio(),
                sampleRate,
                stemCount);
    } else {
        m_legacyStride = WaveformStride(m_waveform->getAudioVisualRatio(),
                m_waveformSummary->getAudioVisualRatio(),
                stemCount);
    }

    m_currentStride = 0;
    m_currentSummaryStride = 0;
    m_channelCount = channelCount;

#ifdef TEST_HEAT_MAP
    test_heatMap = new QImage(256, 256, QImage::Format_RGB32);
    test_heatMap->fill(0xFFFFFFFF);
#endif
    return true;
}

bool AnalyzerWaveform::shouldAnalyze(TrackPointer pTrack) const {
    ConstWaveformPointer pTrackWaveform = pTrack->getWaveform();
    ConstWaveformPointer pTrackWaveformSummary = pTrack->getWaveformSummary();
    ConstWaveformPointer pLoadedTrackWaveform;
    ConstWaveformPointer pLoadedTrackWaveformSummary;
#ifdef __STEM__
    bool isStemTrack = !pTrack->getStemInfo().isEmpty();
#endif

    bool missingWaveform = pTrackWaveform.isNull() ||
            WaveformFactory::waveformVersionToVersionClass(
                    pTrackWaveform->getVersion(), m_analysisProfile) !=
                    WaveformFactory::VC_USE;
    bool missingWavesummary = pTrackWaveformSummary.isNull() ||
            WaveformFactory::waveformSummaryVersionToVersionClass(
                    pTrackWaveformSummary->getVersion(), m_analysisProfile) !=
                    WaveformFactory::VC_USE;
    if (missingWaveform && !pTrackWaveform.isNull()) {
        pTrack->setWaveform(WaveformPointer());
        pTrackWaveform.clear();
    }
    if (missingWavesummary && !pTrackWaveformSummary.isNull()) {
        pTrack->setWaveformSummary(WaveformPointer());
        pTrackWaveformSummary.clear();
    }

    const TrackId trackId = pTrack->getId();
    if (trackId.isValid() && (missingWaveform || missingWavesummary)) {
        const QList<AnalysisDao::AnalysisInfo> analyses =
                m_analysisDao.getAnalysesForTrack(trackId);

        for (const AnalysisDao::AnalysisInfo& analysis : analyses) {
            if (analysis.type == AnalysisDao::TYPE_WAVEFORM) {
                const auto versionClass = WaveformFactory::waveformVersionToVersionClass(
                        analysis.version, m_analysisProfile);
                if (missingWaveform && versionClass == WaveformFactory::VC_USE) {
                    pLoadedTrackWaveform = ConstWaveformPointer(
                            WaveformFactory::loadWaveformFromAnalysis(analysis));
                    missingWaveform = false;
                } else if (versionClass != WaveformFactory::VC_KEEP) {
                    m_analysisDao.deleteAnalysis(analysis.analysisId);
                }
            }
            if (analysis.type == AnalysisDao::TYPE_WAVESUMMARY) {
                const auto versionClass = WaveformFactory::waveformSummaryVersionToVersionClass(
                        analysis.version, m_analysisProfile);
                if (missingWavesummary && versionClass == WaveformFactory::VC_USE) {
                    pLoadedTrackWaveformSummary = ConstWaveformPointer(
                            WaveformFactory::loadWaveformFromAnalysis(analysis));
                    missingWavesummary = false;
                } else if (versionClass != WaveformFactory::VC_KEEP) {
                    m_analysisDao.deleteAnalysis(analysis.analysisId);
                }
            }
        }
    }

#ifdef __STEM__
    // If the waveform was generated without stem information but the track has
    // some, we need to regenerate the waveform.
    const bool waveformHasStemData = (!pTrackWaveform.isNull() &&
                                             pTrackWaveform->hasStem()) ||
            (!pLoadedTrackWaveform.isNull() &&
                    pLoadedTrackWaveform->hasStem());
    if (!missingWaveform && !waveformHasStemData && isStemTrack) {
        missingWaveform = true;
    }
#endif

    if (!missingWaveform && !missingWavesummary) {
        kLogger.debug() << "loadStored - Stored waveform loaded";
        if (pLoadedTrackWaveform) {
            pTrack->setWaveform(pLoadedTrackWaveform);
        }
        if (pLoadedTrackWaveformSummary) {
            pTrack->setWaveformSummary(pLoadedTrackWaveformSummary);
        }
        return false;
    }
    return true;
}

void AnalyzerWaveform::createFilters(mixxx::audio::SampleRate sampleRate) {
    if (m_analysisProfile == mixxx::WaveformAnalysisProfile::Perceptual3Band) {
        m_filters = {
                std::make_unique<EngineFilterBessel4Low>(sampleRate, kPerceptualLowHighFreqHz),
                std::make_unique<EngineFilterBessel4Band>(
                        sampleRate, kPerceptualMidLowFreqHz, kPerceptualMidHighFreqHz),
                std::make_unique<EngineFilterBessel4Band>(
                        sampleRate, kPerceptualHighLowFreqHz, kPerceptualHighHighFreqHz)};
    } else {
        m_filters = {
                std::make_unique<EngineFilterBessel4Low>(sampleRate, kLegacyLowMidFreqHz),
                std::make_unique<EngineFilterBessel4Band>(
                        sampleRate, kLegacyLowMidFreqHz, kLegacyMidHighFreqHz),
                std::make_unique<EngineFilterBessel4High>(sampleRate, kLegacyMidHighFreqHz)};
    }

    // settle filters for silence in preroll to avoids ramping (Issue #7776)
    m_filters.low->assumeSettled();
    m_filters.mid->assumeSettled();
    m_filters.high->assumeSettled();
}

void AnalyzerWaveform::destroyFilters() {
    m_filters = {};
}

bool AnalyzerWaveform::processSamples(const CSAMPLE* pIn, SINT count) {
    VERIFY_OR_DEBUG_ASSERT(m_waveform) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(m_waveformSummary) {
        return false;
    }

    SINT numFrames = count / m_channelCount;
    count = numFrames * mixxx::audio::ChannelCount::stereo();
    int stemCount = 0;

    const CSAMPLE* pWaveformInput = pIn;
    CSAMPLE* pMixedChannel = nullptr;

    if (m_channelCount > mixxx::audio::ChannelCount::stereo()) {
        DEBUG_ASSERT(0 == m_channelCount % mixxx::audio::ChannelCount::stereo());

        pMixedChannel = SampleUtil::alloc(count);
        VERIFY_OR_DEBUG_ASSERT(pMixedChannel) {
            return false;
        }
        SampleUtil::mixMultichannelToStereo(pMixedChannel, pIn, numFrames, m_channelCount);
        stemCount = m_channelCount / mixxx::audio::ChannelCount::stereo();
        pWaveformInput = pMixedChannel;
    }

    if (count > m_buffers.size) {
        m_buffers.low.resize(count);
        m_buffers.mid.resize(count);
        m_buffers.high.resize(count);
        m_buffers.size = count;
    }

    m_filters.low->process(pWaveformInput, &m_buffers.low[0], count);
    m_filters.mid->process(pWaveformInput, &m_buffers.mid[0], count);
    m_filters.high->process(pWaveformInput, &m_buffers.high[0], count);

    m_waveform->setSaveState(Waveform::SaveState::NotSaved);
    m_waveformSummary->setSaveState(Waveform::SaveState::NotSaved);

    const bool isPerceptual =
            m_analysisProfile == mixxx::WaveformAnalysisProfile::Perceptual3Band;
    for (SINT i = 0; i < count; i += 2) {
        if (isPerceptual) {
            const CSAMPLE overall[ChannelCount] = {pWaveformInput[i], pWaveformInput[i + 1]};
            const CSAMPLE low[ChannelCount] = {m_buffers.low[i], m_buffers.low[i + 1]};
            const CSAMPLE mid[ChannelCount] = {m_buffers.mid[i], m_buffers.mid[i + 1]};
            const CSAMPLE high[ChannelCount] = {m_buffers.high[i], m_buffers.high[i + 1]};

            for (int channel = 0; channel < ChannelCount; ++channel) {
                m_perceptualStride.m_overallData[channel] += overall[channel] * overall[channel];
                m_perceptualStride.m_filteredData[channel][Low] += low[channel] * low[channel];
                m_perceptualStride.m_filteredData[channel][Mid] += mid[channel] * mid[channel];
                m_perceptualStride.m_filteredData[channel][High] += high[channel] * high[channel];

                m_perceptualStride.m_summaryOverallData[channel] +=
                        overall[channel] * overall[channel];
                m_perceptualStride.m_summaryFilteredData[channel][Low] += low[channel] * low[channel];
                m_perceptualStride.m_summaryFilteredData[channel][Mid] += mid[channel] * mid[channel];
                m_perceptualStride.m_summaryFilteredData[channel][High] += high[channel] * high[channel];
            }
            ++m_perceptualStride.m_sampleCount;
            ++m_perceptualStride.m_summarySampleCount;
        } else {
            const CSAMPLE cover[ChannelCount] = {
                    std::fabs(pWaveformInput[i]), std::fabs(pWaveformInput[i + 1])};
            const CSAMPLE low[ChannelCount] = {
                    std::fabs(m_buffers.low[i]), std::fabs(m_buffers.low[i + 1])};
            const CSAMPLE mid[ChannelCount] = {
                    std::fabs(m_buffers.mid[i]), std::fabs(m_buffers.mid[i + 1])};
            const CSAMPLE high[ChannelCount] = {
                    std::fabs(m_buffers.high[i]), std::fabs(m_buffers.high[i + 1])};

            storeIfGreater(&m_legacyStride.m_overallData[Left], cover[Left]);
            storeIfGreater(&m_legacyStride.m_overallData[Right], cover[Right]);
            storeIfGreater(&m_legacyStride.m_filteredData[Left][Low], low[Left]);
            storeIfGreater(&m_legacyStride.m_filteredData[Right][Low], low[Right]);
            storeIfGreater(&m_legacyStride.m_filteredData[Left][Mid], mid[Left]);
            storeIfGreater(&m_legacyStride.m_filteredData[Right][Mid], mid[Right]);
            storeIfGreater(&m_legacyStride.m_filteredData[Left][High], high[Left]);
            storeIfGreater(&m_legacyStride.m_filteredData[Right][High], high[Right]);
        }

        for (int stemIndex = 0; stemIndex < stemCount; ++stemIndex) {
            const CSAMPLE stem[ChannelCount] = {
                    std::fabs(pIn[i * stemCount + stemIndex * mixxx::kAnalysisChannels]),
                    std::fabs(pIn[i * stemCount + stemIndex * mixxx::kAnalysisChannels + 1])};
            if (isPerceptual) {
                storeIfGreater(&m_perceptualStride.m_stemData[Left][stemIndex], stem[Left]);
                storeIfGreater(&m_perceptualStride.m_stemData[Right][stemIndex], stem[Right]);
            } else {
                storeIfGreater(&m_legacyStride.m_stemData[Left][stemIndex], stem[Left]);
                storeIfGreater(&m_legacyStride.m_stemData[Right][stemIndex], stem[Right]);
            }
        }

        int& position = isPerceptual ? m_perceptualStride.m_position : m_legacyStride.m_position;
        const double strideLength =
                isPerceptual ? m_perceptualStride.m_length : m_legacyStride.m_length;
        const double summaryStrideLength = isPerceptual ? m_perceptualStride.m_averageLength
                                                         : m_legacyStride.m_averageLength;
        ++position;

        if (std::fmod(position, strideLength) < 1) {
            VERIFY_OR_DEBUG_ASSERT(m_currentStride + ChannelCount <= m_waveform->getDataSize()) {
                qWarning() << "AnalyzerWaveform::process - currentStride > waveform size";
                return false;
            }
            if (isPerceptual) {
                m_perceptualStride.store(m_waveformData + m_currentStride);
            } else {
                m_legacyStride.store(m_waveformData + m_currentStride);
            }
            m_currentStride += ChannelCount;
            m_waveform->setCompletion(m_currentStride);
        }

        if (std::fmod(position, summaryStrideLength) < 1) {
            VERIFY_OR_DEBUG_ASSERT(m_currentSummaryStride + ChannelCount <=
                    m_waveformSummary->getDataSize()) {
                qWarning() << "AnalyzerWaveform::process - current summary stride > waveform summary size";
                return false;
            }
            if (isPerceptual) {
                m_perceptualStride.averageStore(m_waveformSummaryData + m_currentSummaryStride);
            } else {
                m_legacyStride.averageStore(m_waveformSummaryData + m_currentSummaryStride);
            }
            m_currentSummaryStride += ChannelCount;
            m_waveformSummary->setCompletion(m_currentSummaryStride);
        }
    }

    if (pMixedChannel) {
        SampleUtil::free(pMixedChannel);
    }
    return true;
}

void AnalyzerWaveform::cleanup() {
    m_waveform.clear();
    m_waveformData = nullptr;
    m_waveformSummary.clear();
    m_waveformSummaryData = nullptr;
}

void AnalyzerWaveform::storeResults(TrackPointer pTrack) {
    if (m_waveform) {
        m_waveform->setSaveState(Waveform::SaveState::SavePending);
        m_waveform->setCompletion(m_waveform->getDataSize());
        m_waveform->setVersion(WaveformFactory::currentWaveformVersion(m_analysisProfile));
        m_waveform->setDescription(WaveformFactory::currentWaveformDescription(m_analysisProfile));
    }

    if (m_waveformSummary) {
        m_waveformSummary->setSaveState(Waveform::SaveState::SavePending);
        m_waveformSummary->setCompletion(m_waveformSummary->getDataSize());
        m_waveformSummary->setVersion(
                WaveformFactory::currentWaveformSummaryVersion(m_analysisProfile));
        m_waveformSummary->setDescription(
                WaveformFactory::currentWaveformSummaryDescription(m_analysisProfile));
    }

#ifdef TEST_HEAT_MAP
    test_heatMap->save("heatMap.png");
#endif
    // Ensure that the analyses get saved. This is also called from
    // TrackDAO.updateTrack(), but it can happen that we analyze only the
    // waveforms (i.e. if the config setting was disabled in a previous scan)
    // and then it is not called. The other analyzers have signals which control
    // the update of their data.
    m_analysisDao.saveTrackAnalyses(pTrack->getId(), m_waveform, m_waveformSummary);

    // Set waveforms on track AFTER they'been written to disk in order to have
    // a consistency when OverviewCache asks AnalysisDAO for a waveform summary.
    pTrack->setWaveform(m_waveform);
    pTrack->setWaveformSummary(m_waveformSummary);

    kLogger.debug() << "Waveform generation for track" << pTrack->getId() << "done"
                    << m_timer.elapsed().debugSecondsWithUnit();
}

void AnalyzerWaveform::storeIfGreater(float* pDest, float source) {
    if (*pDest < source) {
        *pDest = source;
    }
}
