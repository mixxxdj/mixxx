#include "analyzer/analyzerstemseparation.h"

#include <QtGlobal>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QUrl>

#include "analyzer/analyzertrack.h"
#include "engine/stems/stemmp4writer.h"
#include "sources/soundsource.h"
#include "track/track.h"
#include "util/logger.h"

#ifdef __STEM_SEPARATOR__
#include "OnnxInferenceEngine.h"
#endif

#include <cmath>

namespace {
const mixxx::Logger kLogger("AnalyzerStemSeparation");

constexpr int kNumStems = 4;
constexpr char kConfigGroup[] = "[StemSeparation]";
constexpr char kConfigEnabledKey[] = "offline_enabled";
// Documented default location. The file is NOT shipped with Mixxx and must
// never be assumed present (see class doc).
constexpr char kDefaultModelPath[] = "/usr/local/share/stem-models/htdemucs_fp16weights.onnx";

bool trackHasNativeStems(TrackPointer pTrack) {
#ifdef __STEM__
    if (pTrack && pTrack->hasStem()) {
        return true;
    }
#else
    Q_UNUSED(pTrack);
#endif
    return false;
}

} // namespace

AnalyzerStemSeparation::AnalyzerStemSeparation(UserSettingsPointer pConfig)
        : m_pConfig(pConfig) {
}

// static
QString AnalyzerStemSeparation::defaultModelPath() {
    return QString::fromLatin1(kDefaultModelPath);
}

// static
QString AnalyzerStemSeparation::effectiveModelPath(const QString& overridePath) {
    if (!overridePath.isEmpty()) {
        return overridePath;
    }
    const QString env = qEnvironmentVariable("MIXXX_STEM_MODEL");
    if (!env.isEmpty()) {
        return env;
    }
    return defaultModelPath();
}

// static
bool AnalyzerStemSeparation::isEnabled(const UserSettingsPointer& pConfig) {
    if (!pConfig) {
        return true;
    }
    return pConfig->getValue(ConfigKey(kConfigGroup, kConfigEnabledKey), true);
}

// static
QVector<float> AnalyzerStemSeparation::makeHannWindow(int size) {
    QVector<float> window(size);
    if (size <= 1) {
        if (size == 1) {
            window[0] = 1.0f;
        }
        return window;
    }
    // Identical formula to OfflineSeparator::run():
    //   w[i] = 0.5 * (1 - cos(2*pi*i / (N-1)))
    for (int i = 0; i < size; ++i) {
        window[i] = 0.5f * (1.0f - std::cos(2.0 * M_PI * i / (size - 1)));
    }
    return window;
}

// static
QVector<float> AnalyzerStemSeparation::resampleStereo(
        const QVector<float>& in, int srcRate, int dstRate) {
    if (in.isEmpty() || srcRate <= 0 || dstRate <= 0 || srcRate == dstRate) {
        return in;
    }
    const int inFrames = static_cast<int>(in.size() / 2);
    if (inFrames <= 0) {
        return in;
    }
    const long long outFramesLL =
            (static_cast<long long>(inFrames) * dstRate + srcRate / 2) / srcRate;
    const int outFrames = static_cast<int>(outFramesLL);
    if (outFrames <= 0) {
        return QVector<float>();
    }
    QVector<float> out(outFrames * 2);
    const double step = static_cast<double>(srcRate) / dstRate;
    const float* src = in.constData();
    float* dst = out.data();
    for (int i = 0; i < outFrames; ++i) {
        const double pos = i * step;
        int idx = static_cast<int>(pos);
        double frac = pos - idx;
        if (idx < 0) {
            idx = 0;
            frac = 0.0;
        }
        if (idx + 1 >= inFrames) {
            dst[i * 2] = src[(inFrames - 1) * 2];
            dst[i * 2 + 1] = src[(inFrames - 1) * 2 + 1];
        } else {
            const float f = static_cast<float>(frac);
            dst[i * 2] = src[idx * 2] + (src[(idx + 1) * 2] - src[idx * 2]) * f;
            dst[i * 2 + 1] = src[idx * 2 + 1] +
                    (src[(idx + 1) * 2 + 1] - src[idx * 2 + 1]) * f;
        }
    }
    return out;
}

// static
void AnalyzerStemSeparation::overlapAdd(
        const QVector<QVector<QVector<float>>>& srcChunks,
        int chunkSize,
        int totalFrames,
        const QVector<float>& hannWindow,
        QVector<float> outStems[4]) {
    const int hopSize = chunkSize / 2;
    const int leftPad = hopSize;
    const int paddedFrames = totalFrames + 2 * hopSize;
    const int paddedSamples = paddedFrames * 2;
    const int numChunks = srcChunks.size();
    for (int s = 0; s < kNumStems; ++s) {
        outStems[s].fill(0.0f);
        outStems[s].resize(paddedSamples);
    }
    for (int chunk = 0; chunk < numChunks; ++chunk) {
        const int startFrame = chunk * hopSize;
        for (int s = 0; s < kNumStems; ++s) {
            const QVector<float>& src = srcChunks[chunk][s];
            float* dst = outStems[s].data();
            const int outFrames = static_cast<int>(src.size() / 2);
            for (int i = 0; i < outFrames * 2; ++i) {
                const int frameIdx = i / 2;
                const float win = hannWindow[frameIdx];
                const int outIdx = startFrame * 2 + i;
                if (outIdx < paddedSamples) {
                    dst[outIdx] += src[i] * win;
                }
            }
        }
    }
    // Trim padding, keep original length.
    for (int s = 0; s < kNumStems; ++s) {
        outStems[s] = outStems[s].mid(leftPad * 2, totalFrames * 2);
    }
}

// static
void AnalyzerStemSeparation::accumulateChunk(float* srcStems[4],
        int outFrames,
        const QVector<float>& hannWindow,
        int startFrame,
        int paddedSamples,
        QVector<float>* outStems[4]) {
    for (int s = 0; s < kNumStems; ++s) {
        const float* src = srcStems[s];
        // ONNX emits Demucs order [drums, bass, other, vocals]; Mixxx
        // slots are [vocals, drums, bass, other].
        float* dst = outStems[kMixxxSlotForOnnxStem[s]]->data();
        for (int i = 0; i < outFrames * 2; ++i) {
            const float win = hannWindow[i / 2];
            const int outIdx = startFrame * 2 + i;
            if (outIdx < paddedSamples) {
                dst[outIdx] += src[i] * win;
            }
        }
    }
}

// static
void AnalyzerStemSeparation::passthrough(
        const QVector<float>& mix, QVector<float>* outStems[4]) {
    *outStems[0] = mix;
    for (int s = 1; s < kNumStems; ++s) {
        outStems[s]->fill(0.0f);
        outStems[s]->resize(mix.size());
    }
}

bool AnalyzerStemSeparation::initialize(const AnalyzerTrack& track,
        mixxx::audio::SampleRate sampleRate,
        mixxx::audio::ChannelCount channelCount,
        SINT frameLength) {
    Q_UNUSED(frameLength);
    if (!isEnabled(m_pConfig)) {
        return false;
    }
    TrackPointer pTrack = track.getTrack();
    if (!pTrack) {
        return false;
    }
    // Never separate native stem files or tracks that already carry stems.
    if (mixxx::SoundSource::getTypeFromUrl(QUrl::fromLocalFile(pTrack->getLocation())) ==
            QStringLiteral("stem.mp4")) {
        return false;
    }
    if (trackHasNativeStems(pTrack)) {
        return false;
    }
    m_location = pTrack->getLocation();
    m_key = StemCacheManager::generateKey(m_location);
    // Cache hit: nothing to do, no inference on second load.
    if (StemCacheManager::instance().hasStems(m_key)) {
        kLogger.debug() << "Stem cache hit, skipping analysis for" << m_location;
        return false;
    }
    m_sampleRate = sampleRate;
    m_channelCount = channelCount;
    m_buffer.clear();
    // Reserve a rough estimate (AnalyzerThread feeds 4096-frame chunks).
    if (frameLength > 0) {
        m_buffer.reserve(static_cast<int>(qMin<SINT>(frameLength * 2, 1 << 26)));
    }
    m_initialized = true;
    return true;
}

bool AnalyzerStemSeparation::processSamples(const CSAMPLE* pIn, SINT count) {
    if (!m_initialized) {
        return false;
    }
    if (!pIn || count <= 0) {
        return true;
    }
    // AnalyzerThread guarantees stereo (kAnalysisChannels) via
    // AudioSourceStereoProxy, so pIn is interleaved L,R.
    const int oldSize = m_buffer.size();
    m_buffer.resize(oldSize + count);
    std::copy_n(pIn, count, m_buffer.begin() + oldSize);
    return true;
}

void AnalyzerStemSeparation::storeResults(TrackPointer pTrack) {
    Q_UNUSED(pTrack);
    if (!m_initialized || m_buffer.isEmpty()) {
        return;
    }
    // This runs in AnalyzerThread (offline context), never in the RT callback.
    runSeparationAndCache();
}

void AnalyzerStemSeparation::cleanup() {
    m_buffer.clear();
    m_buffer.squeeze();
    m_location.clear();
    m_key.clear();
    m_initialized = false;
}

void AnalyzerStemSeparation::runSeparationAndCache() {
    if (StemCacheManager::instance().hasStems(m_key)) {
        return; // raced with EngineDeck/offline job; cache won
    }
    if (!StemCacheManager::instance().tryMarkProcessing(m_key)) {
        kLogger.debug() << "Stem separation already in progress for" << m_location;
        return;
    }

    const int totalSamples = m_buffer.size();
    const int totalFrames = totalSamples / 2;
    const int sampleRate = static_cast<int>(m_sampleRate);

    // The ONNX model is trained at kModelSampleRate. Resample a copy for
    // inference; outputs are resampled back to the native rate below so
    // the cached .stem.mp4 keeps native length/rate. Offline context only.
    const bool needResample =
            sampleRate > 0 && sampleRate != kModelSampleRate;
    QVector<float> inferBuffer = needResample
            ? resampleStereo(m_buffer, sampleRate, kModelSampleRate)
            : m_buffer;
    if (needResample && inferBuffer.isEmpty() && totalSamples > 0) {
        kLogger.warning() << "Stem resample to" << kModelSampleRate
                          << "Hz failed - passthrough stems";
        inferBuffer = m_buffer;
    }
    const int inferSamples = inferBuffer.size();
    const int inferFrames = inferSamples / 2;

    QVector<float> outVocals(totalSamples, 0.0f);
    QVector<float> outDrums(totalSamples, 0.0f);
    QVector<float> outBass(totalSamples, 0.0f);
    QVector<float> outOther(totalSamples, 0.0f);
    QVector<float>* outStems[kNumStems] = {&outVocals, &outDrums, &outBass, &outOther};

    bool separated = false;
#ifdef __STEM_SEPARATOR__
    const QString modelPath = effectiveModelPath();
    if (QFile::exists(modelPath)) {
        StemEngine::OnnxInferenceEngine onnx;
        kLogger.info() << "Loading ONNX stem model:" << modelPath;
        if (onnx.loadModel(modelPath.toStdString(), 4, false) &&
                onnx.modelInfo().mode == StemEngine::ModelMode::WAVEFORM) {
            const int kChunkSize = onnx.modelInfo().waveformInputSamples;
            if (kChunkSize > 0 && kChunkSize % 2 == 0) {
                const int kHopSize = kChunkSize / 2;
                const QVector<float> hannWindow = makeHannWindow(kChunkSize);
                const int leftPad = kHopSize;
                const int paddedFrames = inferFrames + 2 * kHopSize;
                const int paddedSamples = paddedFrames * 2;
                const int numChunks = (paddedFrames + kHopSize - 1) / kHopSize;
                QVector<float> paddedInput(paddedSamples, 0.0f);
                std::copy_n(inferBuffer.constData(), inferSamples,
                        paddedInput.begin() + leftPad * 2);
                for (int s = 0; s < kNumStems; ++s) {
                    outStems[s]->resize(paddedSamples, 0.0f);
                }
                std::vector<float> inChunk(static_cast<size_t>(kChunkSize) * 2, 0.0f);
                std::vector<float> stemOut[kNumStems];
                for (int s = 0; s < kNumStems; ++s) {
                    stemOut[s].resize(static_cast<size_t>(kChunkSize) * 2, 0.0f);
                }
                bool ok = true;
                for (int chunk = 0; chunk < numChunks; ++chunk) {
                    const int startFrame = chunk * kHopSize;
                    const int framesIn = std::min(kChunkSize, paddedFrames - startFrame);
                    std::fill(inChunk.begin(), inChunk.end(), 0.0f);
                    std::copy_n(paddedInput.constData() + startFrame * 2,
                            framesIn * 2, inChunk.data());
                    StemEngine::WaveformResult result;
                    for (int s = 0; s < kNumStems; ++s) {
                        result.stems[s] = stemOut[s].data();
                    }
                    result.numSamples = 0;
                    if (!onnx.inferWaveform(inChunk.data(), kChunkSize, result) ||
                            result.numSamples <= 0) {
                        kLogger.warning() << "inferWaveform failed at chunk" << chunk
                                          << "- falling back to passthrough";
                        ok = false;
                        break;
                    }
                    const int outFrames = result.numSamples;
                    accumulateChunk(result.stems, outFrames, hannWindow,
                            startFrame, paddedSamples, outStems);
                }
                if (ok) {
                    outVocals = outVocals.mid(leftPad * 2, inferSamples);
                    outDrums = outDrums.mid(leftPad * 2, inferSamples);
                    outBass = outBass.mid(leftPad * 2, inferSamples);
                    outOther = outOther.mid(leftPad * 2, inferSamples);
                    if (needResample) {
                        // Back to the track-native rate; force the exact
                        // native length (roundtrip rounding may be off by ~1).
                        outVocals = resampleStereo(
                                outVocals, kModelSampleRate, sampleRate);
                        outDrums = resampleStereo(
                                outDrums, kModelSampleRate, sampleRate);
                        outBass = resampleStereo(
                                outBass, kModelSampleRate, sampleRate);
                        outOther = resampleStereo(
                                outOther, kModelSampleRate, sampleRate);
                        outVocals.resize(totalSamples);
                        outDrums.resize(totalSamples);
                        outBass.resize(totalSamples);
                        outOther.resize(totalSamples);
                    }
                    separated = true;
                    kLogger.info() << "ONNX stem separation finished for" << m_location;
                }
            }
        } else {
            kLogger.warning() << "ONNX model not usable - passthrough stems";
        }
    } else {
        kLogger.info() << "No stem model at" << modelPath << "- passthrough stems";
    }
#else
    Q_UNUSED(separated);
#endif

    if (!separated) {
        // Passthrough fallback: vocals = mix, rest = silence. Analysis
        // always succeeds even with SIN modelo (offline, CPU-only).
        passthrough(m_buffer, outStems);
    }

    // Write native .stem.mp4 named {hash}.stem.mp4 into the per-track dir.
    const QString dir = StemCacheManager::stemDir(m_key);
    QDir().mkpath(dir);
    const QString stemPath = StemCacheManager::stemFilePath(m_key);
    mixxx::StemMp4Writer::Config cfg;
    cfg.outputPath = stemPath;
    cfg.sampleRate = sampleRate > 0 ? sampleRate : kModelSampleRate;
    cfg.numFrames = totalFrames;
    cfg.stems[0] = outVocals.constData();
    cfg.stems[1] = outDrums.constData();
    cfg.stems[2] = outBass.constData();
    cfg.stems[3] = outOther.constData();

    StemCacheManager::StemFiles files;
    files.complete = true;
    files.created = QDateTime::currentDateTime();
    if (mixxx::StemMp4Writer::write(cfg)) {
        files.stemFile = stemPath;
        kLogger.info() << "Wrote cached stem file:" << stemPath;
    } else {
        kLogger.warning() << "Failed to write stem file for" << m_location;
        StemCacheManager::instance().markFailed(m_key);
        return;
    }
    StemCacheManager::instance().markComplete(m_key, files);
}
