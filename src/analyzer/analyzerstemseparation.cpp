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
double AnalyzerStemSeparation::overlapRatio(const UserSettingsPointer& pConfig) {
    if (!pConfig) {
        return kDefaultOverlap;
    }
    const double v = pConfig->getValue(
            ConfigKey(kConfigGroup, "overlap"), kDefaultOverlap);
    // N19: default is 0.25 (fast); 0.5 stays available as a manual opt-in
    // for quality. Anything else falls back to the default so quality
    // never silently degrades to an untested ratio.
    if (std::abs(v - 0.5) < 1e-9) {
        return 0.5;
    }
    return kDefaultOverlap;
}

// static
int AnalyzerStemSeparation::hopSizeFor(int chunkSize, double overlap) {
    if (chunkSize <= 0) {
        return 0;
    }
    if (std::abs(overlap - 0.25) < 1e-9) {
        // 25% overlap: hop = 3N/4, must be even (stereo frames are pairs
        // of floats; odd hops would misalign L/R).
        int hop = (chunkSize * 3) / 4;
        return hop - (hop % 2);
    }
    return chunkSize / 2; // 50% overlap (default, COLA with Hann)
}

// static
int AnalyzerStemSeparation::stemMode(const UserSettingsPointer& pConfig) {
    if (!pConfig) {
        return kDefaultStemMode;
    }
    const int v = pConfig->getValue(
            ConfigKey(kConfigGroup, "stem_mode"), kDefaultStemMode);
    // N19: default is 3 (fast); 4 stays available as a manual opt-in.
    return (v == 4) ? 4 : kDefaultStemMode;
}

// static
void AnalyzerStemSeparation::foldTo3StemMode(QVector<float>* outStems[4]) {
    if (!outStems[2] || !outStems[3] ||
            outStems[2]->size() != outStems[3]->size()) {
        return;
    }
    float* instruments = outStems[3]->data();
    const float* bass = outStems[2]->constData();
    const int n = outStems[3]->size();
    for (int i = 0; i < n; ++i) {
        instruments[i] += bass[i];
    }
    outStems[2]->fill(0.0f);
}

// static
void AnalyzerStemSeparation::normalizeWola(
        QVector<float>* outStems[4], const QVector<float>& weight) {
    constexpr float kEps = 1e-8f;
    for (int s = 0; s < kNumStems; ++s) {
        if (!outStems[s] ||
                outStems[s]->size() != weight.size() * 2) {
            return;
        }
    }
    for (int s = 0; s < kNumStems; ++s) {
        float* dst = outStems[s]->data();
        const int n = outStems[s]->size();
        for (int i = 0; i < n; ++i) {
            const float w = weight[i / 2];
            if (w > kEps) {
                dst[i] /= w;
            }
        }
    }
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
bool AnalyzerStemSeparation::shouldEmitPartial(int chunkIndex, int numChunks) {
    return chunkIndex + 1 == kPartialChunks && numChunks > kPartialChunks;
}

// static
int AnalyzerStemSeparation::partialPrefixFrames(
        int chunksDone, int hopSize, int inferFrames) {
    if (chunksDone <= 0 || hopSize <= 0 || inferFrames <= 0) {
        return 0;
    }
    const long long prefix = static_cast<long long>(chunksDone) * hopSize;
    if (prefix <= 0) {
        return 0;
    }
    if (prefix >= inferFrames) {
        return inferFrames;
    }
    return static_cast<int>(prefix);
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
    m_stemMode = stemMode(m_pConfig);
    m_overlap = overlapRatio(m_pConfig);
    // N18: versioned cache key — mode 3 appends "|mode=3" so 3-stem
    // artifacts never poison 4-stem entries (mode 4 keys are unchanged,
    // keeping all existing cache entries valid). N19: versioning is
    // intentionally unchanged after the default flip to mode 3, so legacy
    // mode-4 entries are never served as mode 3.
    m_key = StemCacheManager::generateKeyForMode(m_location, m_stemMode);
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
    // N21: mode upfront so the partial preview folds identically to final.
    const int stemModeCfgEarly = stemMode(m_pConfig);
#ifdef __STEM_SEPARATOR__
    const QString modelPath = effectiveModelPath();
    if (QFile::exists(modelPath)) {
        StemEngine::OnnxInferenceEngine onnx;
        kLogger.info() << "Loading ONNX stem model:" << modelPath;
        // N18: intraOp=0 lets ONNX Runtime pick the thread count
        // automatically (== nproc, 16 here); interOp stays 1 inside the
        // engine (see OnnxInferenceEngine::loadModel: SetInterOpNumThreads(1)).
        // No model change, offline thread only.
        if (onnx.loadModel(modelPath.toStdString(), 0, false) &&
                onnx.modelInfo().mode == StemEngine::ModelMode::WAVEFORM) {
            const int kChunkSize = onnx.modelInfo().waveformInputSamples;
            if (kChunkSize > 0 && kChunkSize % 2 == 0) {
                const double overlap = overlapRatio(m_pConfig);
                const int kHopSize = hopSizeFor(kChunkSize, overlap);
                const QVector<float> hannWindow = makeHannWindow(kChunkSize);
                const int leftPad = kHopSize;
                const int paddedFrames = inferFrames + 2 * kHopSize;
                const int paddedSamples = paddedFrames * 2;
                const int numChunks = (paddedFrames + kHopSize - 1) / kHopSize;
                // N18: at 25% overlap the Hann windows do NOT sum to 1
                // (ripple ~= +/-15%), so accumulate per-frame window weights
                // and renormalize (WOLA) afterwards. The default 50% path
                // skips this to stay bit-identical (COLA ~= 1).
                const bool needWola = (kHopSize != kChunkSize / 2);
                QVector<float> wolaWeight;
                if (needWola) {
                    wolaWeight.fill(0.0f, paddedFrames);
                }
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
                    if (needWola) {
                        const int wFrames = std::min(outFrames,
                                paddedFrames - startFrame);
                        for (int i = 0; i < wFrames; ++i) {
                            wolaWeight[startFrame + i] += hannWindow[i];
                        }
                    }
                    // N21: chunk-streaming preview (same policy as
                    // OfflineSeparator::run): after kPartialChunks chunks
                    // write {hash}.partial.stem.mp4 + markPartial() so
                    // playback can start; the rest keeps processing.
                    if (shouldEmitPartial(chunk, numChunks)) {
                        const int prefixInfer = partialPrefixFrames(
                                chunk + 1, kHopSize, inferFrames);
                        if (prefixInfer > 0) {
                            QVector<float> pV =
                                    outVocals.mid(leftPad * 2, prefixInfer * 2);
                            QVector<float> pD =
                                    outDrums.mid(leftPad * 2, prefixInfer * 2);
                            QVector<float> pB =
                                    outBass.mid(leftPad * 2, prefixInfer * 2);
                            QVector<float> pO =
                                    outOther.mid(leftPad * 2, prefixInfer * 2);
                            if (needWola) {
                                const QVector<float> wPrefix =
                                        wolaWeight.mid(leftPad, prefixInfer);
                                QVector<float>* pStems[kNumStems] = {
                                        &pV, &pD, &pB, &pO};
                                normalizeWola(pStems, wPrefix);
                            }
                            QVector<float> nV = needResample
                                    ? resampleStereo(pV, kModelSampleRate,
                                              sampleRate)
                                    : pV;
                            QVector<float> nD = needResample
                                    ? resampleStereo(pD, kModelSampleRate,
                                              sampleRate)
                                    : pD;
                            QVector<float> nB = needResample
                                    ? resampleStereo(pB, kModelSampleRate,
                                              sampleRate)
                                    : pB;
                            QVector<float> nO = needResample
                                    ? resampleStereo(pO, kModelSampleRate,
                                              sampleRate)
                                    : pO;
                            const int prefixNative = nV.size() / 2;
                            if (prefixNative > 0) {
                                QVector<float>* fStems[4] = {
                                        &nV, &nD, &nB, &nO};
                                if (stemModeCfgEarly == 3) {
                                    foldTo3StemMode(fStems);
                                }
                                const QString partialPath =
                                        StemCacheManager::partialStemFilePath(
                                                m_key);
                                QDir().mkpath(QFileInfo(partialPath)
                                                      .absolutePath());
                                mixxx::StemMp4Writer::Config pcfg;
                                pcfg.outputPath = partialPath;
                                pcfg.sampleRate = sampleRate > 0 ? sampleRate
                                                                 : kModelSampleRate;
                                pcfg.numFrames = prefixNative;
                                if (stemModeCfgEarly == 3) {
                                    pcfg.stemNames[3] =
                                            QStringLiteral("Instruments");
                                }
                                pcfg.stems[0] = nV.constData();
                                pcfg.stems[1] = nD.constData();
                                pcfg.stems[2] = nB.constData();
                                pcfg.stems[3] = nO.constData();
                                if (mixxx::StemMp4Writer::write(pcfg)) {
                                    StemCacheManager::StemFiles partial;
                                    partial.stemFile = partialPath;
                                    partial.complete = false;
                                    partial.created =
                                            QDateTime::currentDateTime();
                                    StemCacheManager::instance().markPartial(
                                            m_key, partial);
                                    kLogger.info()
                                            << "Wrote partial stem preview ("
                                            << prefixNative << "frames) after "
                                            << (chunk + 1) << " chunks:"
                                            << partialPath;
                                }
                            }
                        }
                    }
                }
                if (ok) {
                    if (needWola) {
                        normalizeWola(outStems, wolaWeight);
                    }
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

    // N18: optional 3-stem fold (default off). Slot 3 (other) becomes
    // bass+other ("Instruments"), slot 2 (bass) becomes silence. The
    // 8-channel reader layout is untouched (still 4 slots in the file).
    // (mode resolved upfront as stemModeCfgEarly for the N21 preview.)
    const int stemModeCfg = stemModeCfgEarly;
    if (stemModeCfg == 3) {
        foldTo3StemMode(outStems);
    }

    // Write native .stem.mp4 named {hash}.stem.mp4 into the per-track dir.
    const QString dir = StemCacheManager::stemDir(m_key);
    QDir().mkpath(dir);
    const QString stemPath = StemCacheManager::stemFilePath(m_key);
    mixxx::StemMp4Writer::Config cfg;
    cfg.outputPath = stemPath;
    cfg.sampleRate = sampleRate > 0 ? sampleRate : kModelSampleRate;
    cfg.numFrames = totalFrames;
    if (stemModeCfg == 3) {
        // Manifest: slot 3 carries bass+other, so label it "Instruments".
        // Slot 2 stays "Bass" (silent); the UI should hide it in 3s mode.
        cfg.stemNames[3] = QStringLiteral("Instruments");
    }
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
