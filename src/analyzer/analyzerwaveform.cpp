#include "analyzer/analyzerwaveform.h"

#include <fidlib.h>

#include <algorithm>
#include <memory>
#include <vector>

#include "analyzer/analyzertrack.h"
#include "analyzer/constants.h"
#include "track/track.h"
#include "util/logger.h"
#include "waveform/waveform.h"
#include "waveform/waveformfactory.h"

namespace {

mixxx::Logger kLogger("AnalyzerWaveform");

constexpr double kLowMidFreqHz = 600.0;

constexpr double kMidHighFreqHz = 4000.0;

// Updates the maxima of the absolute values of an interleaved stereo signal
// with frames that are frameStride samples apart.
inline void storeMaxAbs(float* pMaxLeft,
        float* pMaxRight,
        const CSAMPLE* pIn,
        SINT frameCount,
        int frameStride) {
    float maxLeft = *pMaxLeft;
    float maxRight = *pMaxRight;
    for (SINT i = 0; i < frameCount; ++i) {
        maxLeft = std::max(maxLeft, std::fabs(pIn[i * frameStride]));
        maxRight = std::max(maxRight, std::fabs(pIn[i * frameStride + 1]));
    }
    *pMaxLeft = maxLeft;
    *pMaxRight = maxRight;
}

#if defined(__GNUC__) && defined(FP_FAST_FMA)
// A value of both channels in one SIMD register. Without FMA, the compiler
// generates faster code for the plain struct.
typedef double StereoValue __attribute__((vector_size(2 * sizeof(double))));
#else
struct StereoValue {
    double value[ChannelCount];
    double& operator[](int channel) {
        return value[channel];
    }
    double operator[](int channel) const {
        return value[channel];
    }
};
#endif

// a * b + c, fused if the CPU supports it. The fused operation shortens the
// loop carried dependency of the filter state, which limits the filter speed.
// It also keeps the compiler from reordering the additions.
inline double mulAdd(double a, double b, double c) {
#ifdef FP_FAST_FMA
    return std::fma(a, b, c);
#else
    return a * b + c;
#endif
}

// a - b * c for both channels
inline StereoValue mulSub(StereoValue a, double b, StereoValue c) {
    StereoValue result;
    for (int ch = 0; ch < ChannelCount; ++ch) {
        result[ch] = mulAdd(-b, c[ch], a[ch]);
    }
    return result;
}

// Updates the maximum of the absolute values for both channels
inline void storeMaxAbs(StereoValue* pMax, StereoValue value) {
    for (int ch = 0; ch < ChannelCount; ++ch) {
        (*pMax)[ch] = std::max((*pMax)[ch], std::fabs(value[ch]));
    }
}

inline StereoValue mul(StereoValue a, double b) {
    return StereoValue{a[Left] * b, a[Right] * b};
}

} // namespace

// The Bessel low pass, band pass and high pass filters of EngineFilterBessel4Low,
// EngineFilterBessel4Band and EngineFilterBessel4High, settled for silence.
// The samples are processed like EngineFilterIIR::processSample() does, but all
// bands and both channels in a single pass, which allows the CPU to compute the
// independent second order sections in parallel.
class WaveformBandFilters {
  public:
    explicit WaveformBandFilters(mixxx::audio::SampleRate sampleRate) {
        designCoefs(m_lowCoef, "LpBe4", sampleRate, kLowMidFreqHz);
        designCoefs(m_midCoef, "BpBe4", sampleRate, kLowMidFreqHz, kMidHighFreqHz);
        designCoefs(m_highCoef, "HpBe4", sampleRate, kMidHighFreqHz);
    }

    // Filters the frames and updates the stride maxima of the absolute values
    // of the input and of the filtered bands.
    void processAndStoreMaxima(const CSAMPLE* pIn, SINT frameCount, WaveformStride* pStride) {
        // Copy the state for keeping it in registers
        StereoValue state[kStateSize];
        std::copy(std::begin(m_state), std::end(m_state), state);
        // The maxima are tracked before rounding to float. That gives the same
        // result, because the rounding is monotonic and symmetric.
        StereoValue maxAll, maxLow, maxMid, maxHigh;
        for (int ch = 0; ch < ChannelCount; ++ch) {
            maxAll[ch] = pStride->m_overallData[ch];
            maxLow[ch] = pStride->m_filteredData[ch][Low];
            maxMid[ch] = pStride->m_filteredData[ch][Mid];
            maxHigh[ch] = pStride->m_filteredData[ch][High];
        }
        for (SINT i = 0; i < frameCount; ++i) {
            const StereoValue in = {pIn[i * ChannelCount + Left], pIn[i * ChannelCount + Right]};
            StereoValue low = mul(in, m_lowCoef[0]);
            StereoValue mid = mul(in, m_midCoef[0]);
            StereoValue high = mul(in, m_highCoef[0]);
            low = section<true>(low, &m_lowCoef[1], &state[0]);
            mid = section<false>(mid, &m_midCoef[1], &state[4]);
            high = section<false>(high, &m_highCoef[1], &state[12]);
            low = section<true>(low, &m_lowCoef[3], &state[2]);
            mid = section<false>(mid, &m_midCoef[3], &state[6]);
            high = section<false>(high, &m_highCoef[3], &state[14]);
            mid = section<true>(mid, &m_midCoef[5], &state[8]);
            mid = section<true>(mid, &m_midCoef[7], &state[10]);
            storeMaxAbs(&maxAll, in);
            storeMaxAbs(&maxLow, low);
            storeMaxAbs(&maxMid, mid);
            storeMaxAbs(&maxHigh, high);
        }
        std::copy(std::begin(state), std::end(state), m_state);
        for (int ch = 0; ch < ChannelCount; ++ch) {
            pStride->m_overallData[ch] = static_cast<float>(maxAll[ch]);
            pStride->m_filteredData[ch][Low] = static_cast<float>(maxLow[ch]);
            pStride->m_filteredData[ch][Mid] = static_cast<float>(maxMid[ch]);
            pStride->m_filteredData[ch][High] = static_cast<float>(maxHigh[ch]);
        }
    }

  private:
    static constexpr int kLowSize = 4;
    static constexpr int kMidSize = 8;
    static constexpr int kHighSize = 4;
    // z2 and z1 of each second order section: low pass at 0..3,
    // band pass at 4..11 and high pass at 12..15
    static constexpr int kStateSize = kLowSize + kMidSize + kHighSize;

    // Designs the gain coef[0] and the coefficients of the sections
    template<int size>
    static void designCoefs(double (&coef)[size],
            const char* spec,
            mixxx::audio::SampleRate sampleRate,
            double freq0,
            double freq1 = 0) {
        coef[0] = fid_design_coef(coef + 1, size - 1, spec, sampleRate, freq0, freq1, 0);
    }

    // A second order section of EngineFilterIIR::processSample() with the
    // two coefficients pCoef[0..1]. pState[0..1] are z2 and z1, the two
    // previous outputs of the recursive part.
    template<bool lowPass>
    static inline StereoValue section(StereoValue in, const double* pCoef, StereoValue* pState) {
        const StereoValue z2 = pState[0];
        const StereoValue z1 = pState[1];
        const StereoValue iir = mulSub(mulSub(in, pCoef[0], z2), pCoef[1], z1);
        StereoValue fir;
        for (int ch = 0; ch < ChannelCount; ++ch) {
            // iir is computed last, so it is added last
            fir[ch] = mulAdd(lowPass ? 2.0 : -2.0, z1[ch], z2[ch]) + iir[ch];
        }
        pState[0] = z1;
        pState[1] = iir;
        return fir;
    }

    double m_lowCoef[kLowSize + 1];
    double m_midCoef[kMidSize + 1];
    double m_highCoef[kHighSize + 1];
    StereoValue m_state[kStateSize] = {};
};

WaveformStrideEnds::WaveformStrideEnds(double length) {
    if (!(length > 0)) {
        // Not a valid length, no stride ends
        m_next = std::numeric_limits<int>::max();
        return;
    }
    // For a length < 1, every position ends a stride, like for length 1
    int exponent;
    const double mantissa = std::frexp(std::max(length, 1.0), &exponent);
    // length = mantissa * 2^exponent with an integer mantissa of 53 bits
    constexpr int kMantissaBits = std::numeric_limits<double>::digits;
    m_lengthFraction = static_cast<int64_t>(std::ldexp(mantissa, kMantissaBits));
    m_shift = kMantissaBits - exponent;
    VERIFY_OR_DEBUG_ASSERT(m_shift >= 0) {
        m_next = std::numeric_limits<int>::max();
        return;
    }
    advance();
}

void WaveformStrideEnds::advance() {
    // A stride ends at p = ceil(n * length) for each n. The distance to the next
    // end is ceil(length - fmod(p, length)).
    const int64_t distanceToNextMultiple = m_lengthFraction - m_remainder;
    const int64_t one = int64_t{1} << m_shift;
    const int64_t distance = (distanceToNextMultiple + one - 1) >> m_shift;
    m_remainder = (distance << m_shift) - distanceToNextMultiple;
    m_next += static_cast<int>(distance);
}

AnalyzerWaveform::AnalyzerWaveform(
        UserSettingsPointer pConfig,
        const QSqlDatabase& dbConnection)
        : m_analysisDao(pConfig),
          m_waveformData(nullptr),
          m_waveformSummaryData(nullptr),
          m_stride(0, 0, 0),
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

    // If we don't need to calculate the waveform/wavesummary, skip.
    if (!shouldAnalyze(track.getTrack())) {
        return false;
    }

    m_timer.start();

    // Now actually initialize the AnalyzerWaveform:
    destroyFilters();
    createFilters(sampleRate);

    //TODO (vrince) Do we want to expose this as settings or whatever ?
    constexpr int mainWaveformSampleRate = 441;
    // two visual sample per pixel in full width overview in full hd
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

    m_stride = WaveformStride(m_waveform->getAudioVisualRatio(),
            m_waveformSummary->getAudioVisualRatio(),
            stemCount);

    m_currentStride = 0;
    m_currentSummaryStride = 0;
    m_storeEnds = WaveformStrideEnds(m_stride.m_length);
    m_summaryEnds = WaveformStrideEnds(m_stride.m_averageLength);
    m_channelCount = channelCount;

    //debug
    //m_waveform->dump();
    //m_waveformSummary->dump();

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

    TrackId trackId = pTrack->getId();
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

    // If we don't need to calculate the waveform/wavesummary, skip.
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
    // settle filters for silence in preroll to avoids ramping (Issue #7776)
    m_filters = std::make_unique<WaveformBandFilters>(sampleRate);
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

    m_waveform->setSaveState(Waveform::SaveState::NotSaved);
    m_waveformSummary->setSaveState(Waveform::SaveState::NotSaved);

    SINT frame = 0;
    while (frame < numFrames) {
        // Process all frames up to the next position, where a stride ends
        const int nextPosition = std::min(m_storeEnds.next(), m_summaryEnds.next());
        const SINT frameCount = std::min<SINT>(
                numFrames - frame, nextPosition - m_stride.m_position);

        // Filter and record the max across this stride.
        // Take max value, not average of data.
        m_filters->processAndStoreMaxima(
                pWaveformInput + frame * mixxx::kAnalysisChannels, frameCount, &m_stride);

        for (int s = 0; s < stemCount; s++) {
            storeMaxAbs(&m_stride.m_stemData[Left][s],
                    &m_stride.m_stemData[Right][s],
                    pIn + frame * m_channelCount + s * mixxx::kAnalysisChannels,
                    frameCount,
                    m_channelCount);
        }

        frame += frameCount;
        m_stride.m_position += static_cast<int>(frameCount);

        if (m_stride.m_position == m_storeEnds.next()) {
            VERIFY_OR_DEBUG_ASSERT(m_currentStride + ChannelCount <= m_waveform->getDataSize()) {
                qWarning() << "AnalyzerWaveform::process - currentStride > waveform size";
                return false;
            }
            m_stride.store(m_waveformData + m_currentStride);
            m_currentStride += ChannelCount;
            m_waveform->setCompletion(m_currentStride);
            m_storeEnds.advance();
        }

        if (m_stride.m_position == m_summaryEnds.next()) {
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
            m_summaryEnds.advance();
        }
    }

    //kLogger.debug() << "process - m_waveform->getCompletion()" << m_waveform->getCompletion() << "off" << m_waveform->getDataSize();
    //kLogger.debug() << "process - m_waveformSummary->getCompletion()" << m_waveformSummary->getCompletion() << "off" << m_waveformSummary->getDataSize();
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
    // Force completion to waveform size
    if (m_waveform) {
        m_waveform->setSaveState(Waveform::SaveState::SavePending);
        m_waveform->setCompletion(m_waveform->getDataSize());
        m_waveform->setVersion(WaveformFactory::currentWaveformVersion());
        m_waveform->setDescription(WaveformFactory::currentWaveformDescription());
    }

    // Force completion to waveform size
    if (m_waveformSummary) {
        m_waveformSummary->setSaveState(Waveform::SaveState::SavePending);
        m_waveformSummary->setCompletion(m_waveformSummary->getDataSize());
        m_waveformSummary->setVersion(WaveformFactory::currentWaveformSummaryVersion());
        m_waveformSummary->setDescription(WaveformFactory::currentWaveformSummaryDescription());
    }

#ifdef TEST_HEAT_MAP
    test_heatMap->save("heatMap.png");
#endif
    // Ensure that the analyses get saved. This is also called from
    // TrackDAO.updateTrack(), but it can happen that we analyze only the
    // waveforms (i.e. if the config setting was disabled in a previous scan)
    // and then it is not called. The other analyzers have signals which control
    // the update of their data.
    m_analysisDao.saveTrackAnalyses(
            pTrack->getId(),
            m_waveform,
            m_waveformSummary);

    // Set waveforms on track AFTER they'been written to disk in order to have
    // a consistency when OverviewCache asks AnalysisDAO for a waveform summary.
    pTrack->setWaveform(m_waveform);
    pTrack->setWaveformSummary(m_waveformSummary);

    kLogger.debug() << "Waveform generation for track" << pTrack->getId() << "done"
                    << m_timer.elapsed().debugSecondsWithUnit();
}
