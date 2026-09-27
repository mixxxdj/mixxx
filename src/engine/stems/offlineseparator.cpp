#include "offlineseparator.h"
#include "moc_offlineseparator.cpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QThreadPool>

#include "sources/audiosourcestereoproxy.h"
#include "sources/soundsourceproxy.h"
#include "analyzer/analyzerstemseparation.h"
#include "stemmp4writer.h"
#include "track/track.h"
#include "util/logger.h"
#include "util/samplebuffer.h"

#ifdef __STEM_SEPARATOR__
#include "OnnxInferenceEngine.h"
#endif

namespace {
const mixxx::Logger kLogger("OfflineSeparator");
} // namespace

namespace mixxx {

OfflineSeparator::OfflineSeparator(const Config& config)
    : QObject(nullptr), m_config(config) {
    setAutoDelete(false); // We manage lifetime via signals
}

OfflineSeparator::~OfflineSeparator() {
    m_cancelled = true;
}

void OfflineSeparator::start() {
    QThreadPool::globalInstance()->start([this]() { run(); });
}

void OfflineSeparator::run() {
    kLogger.info() << "Starting offline separation for:" << m_config.inputPath;

    // 1. Decode entire audio file to float stereo buffer via Mixxx's own
    // decoders (SoundSourceProxy). No QAudioDecoder/QEventLoop: this runs in
    // a worker thread and must not depend on QtMultimedia plugins.
    QVector<float> fullBuffer; // interleaved L,R,L,R...
    int decodedSampleRate = m_config.sampleRate;
    {
        TrackPointer pTrack = Track::newTemporary(m_config.inputPath);
        if (!pTrack) {
            kLogger.warning() << "Cannot create track for" << m_config.inputPath;
            if (m_config.onFinished) m_config.onFinished(false, {});
            emit finished(false, {});
            return;
        }
        SoundSourceProxy proxy(pTrack);
        mixxx::AudioSource::OpenParams params;
        params.setChannelCount(mixxx::audio::ChannelCount::stereo());
        mixxx::AudioSourcePointer audioSource = proxy.openAudioSource(params);
        if (!audioSource) {
            kLogger.warning() << "Failed to open input file:" << m_config.inputPath;
            if (m_config.onFinished) m_config.onFinished(false, {});
            emit finished(false, {});
            return;
        }
        if (audioSource->getSignalInfo().getChannelCount() !=
                mixxx::audio::ChannelCount::stereo()) {
            audioSource = std::make_shared<mixxx::AudioSourceStereoProxy>(
                    audioSource, 4096);
        }
        decodedSampleRate = static_cast<int>(
                audioSource->getSignalInfo().getSampleRate());
        mixxx::SampleBuffer chunk(4096 * 2);
        mixxx::IndexRange remaining = audioSource->frameIndexRange();
        while (!remaining.empty()) {
            if (m_cancelled) {
                kLogger.info() << "Separation cancelled during decode";
                if (m_config.onFinished) m_config.onFinished(false, {});
                emit finished(false, {});
                return;
            }
            auto chunkRange = remaining.splitAndShrinkFront(
                    std::min<SINT>(4096, remaining.length()));
            auto readable = audioSource->readSampleFrames(
                    mixxx::WritableSampleFrames(chunkRange,
                            mixxx::SampleBuffer::WritableSlice(chunk)));
            if (readable.readableLength() <= 0) {
                break;
            }
            const CSAMPLE* data = readable.readableData();
            const int count = readable.readableLength();
            const int oldSize = fullBuffer.size();
            fullBuffer.resize(oldSize + count);
            std::copy_n(data, count, fullBuffer.begin() + oldSize);
            remaining = intersect(remaining, audioSource->frameIndexRange());
        }
    }

    if (fullBuffer.isEmpty()) {
        kLogger.warning() << "Failed to decode input file";
        if (m_config.onFinished) m_config.onFinished(false, {});
        emit finished(false, {});
        return;
    }

    kLogger.info() << "Decoded" << fullBuffer.size()/2 << "stereo samples @"
                   << decodedSampleRate << "Hz";
    
    // 2. Create output directory
    QDir().mkpath(m_config.outputDir);
    
    // 3. Process with ONNX (using existing mixxx-stems-engine)
    // The engine expects a fixed window of samples per inference.
#ifdef __STEM_SEPARATOR__
    StemEngine::OnnxInferenceEngine onnx;
    bool onnxReady = false;

    const QString modelPath = m_config.modelPath.isEmpty()
            ? QStringLiteral("/usr/local/share/stem-models/htdemucs_fp16weights.onnx")
            : m_config.modelPath;

    kLogger.info() << "Loading ONNX model:" << modelPath;
    if (onnx.loadModel(modelPath.toStdString(), 4, /*enableQuantization=*/false)) {
        const auto& info = onnx.modelInfo();
        if (info.mode == StemEngine::ModelMode::WAVEFORM) {
            onnxReady = true;
            kLogger.info() << "ONNX waveform model ready, window:"
                           << info.waveformInputSamples << "samples,"
                           << info.outputStems << "stems";
        } else {
            kLogger.warning() << "Model is not a waveform model (spectrogram mode unsupported offline)";
        }
    } else {
        kLogger.warning() << "Failed to load ONNX model - falling back to passthrough stems";
    }

    constexpr int kNumStems = 4;       // vocals, drums, bass, other
    constexpr int kModelRate = AnalyzerStemSeparation::kModelSampleRate;

    // The ONNX model is trained at kModelRate. Resample a copy for
    // inference; stems are resampled back to the native rate before
    // writing so WAV/.stem.mp4 keep native length/rate. Worker thread.
    const bool needResample =
            decodedSampleRate > 0 && decodedSampleRate != kModelRate;
    QVector<float> inferBuffer = needResample
            ? AnalyzerStemSeparation::resampleStereo(
                      fullBuffer, decodedSampleRate, kModelRate)
            : fullBuffer;
    if (needResample) {
        if (inferBuffer.isEmpty() && !fullBuffer.isEmpty()) {
            kLogger.warning() << "Stem resample to" << kModelRate
                              << "Hz failed - falling back to passthrough stems";
            onnxReady = false;
        } else {
            kLogger.info() << "Resampled" << fullBuffer.size() / 2 << "frames @"
                           << decodedSampleRate << "Hz to" << inferBuffer.size() / 2
                           << "frames @" << kModelRate << "Hz for inference";
        }
    }

    const int totalSamples = fullBuffer.size();      // interleaved floats, native rate
    const int totalFrames = totalSamples / 2;
    const int inferSamples = inferBuffer.size();     // model rate
    const int inferFrames = inferSamples / 2;

    // Output buffers for 4 stems (interleaved, at inference rate)
    QVector<float> outVocals(inferSamples, 0.0f);
    QVector<float> outDrums(inferSamples, 0.0f);
    QVector<float> outBass(inferSamples, 0.0f);
    QVector<float> outOther(inferSamples, 0.0f);
    QVector<float>* outStems[kNumStems] = {&outVocals, &outDrums, &outBass, &outOther};

    if (onnxReady) {
        const int kChunkSize = onnx.modelInfo().waveformInputSamples; // e.g. 343980
        const int kHopSize = kChunkSize / 2;                          // 50% overlap

        // Hann window for overlap-add (Hann at 50% overlap sums to ~1)
        QVector<float> hannWindow(kChunkSize);
        for (int i = 0; i < kChunkSize; ++i) {
            hannWindow[i] = 0.5f * (1.0f - std::cos(2.0 * M_PI * i / (kChunkSize - 1)));
        }

        // Pad input by kHopSize at each end so every output sample has full
        // double-window coverage from the overlap-add (no fade-in/out at the
        // track boundaries). The model sees silence in the padded regions.
        const int leftPad = kHopSize;
        const int rightPad = kHopSize;
        const int paddedFrames = inferFrames + leftPad + rightPad;
        const int paddedSamples = paddedFrames * 2;
        const int numChunks = (paddedFrames + kHopSize - 1) / kHopSize;

        QVector<float> paddedInput(paddedSamples, 0.0f);
        std::copy_n(inferBuffer.constData(), inferSamples,
                paddedInput.begin() + leftPad * 2);

        // Output buffers, padded to match
        outVocals.resize(paddedSamples, 0.0f);
        outDrums.resize(paddedSamples, 0.0f);
        outBass.resize(paddedSamples, 0.0f);
        outOther.resize(paddedSamples, 0.0f);

        kLogger.info() << "Running real ONNX separation on" << inferFrames
                       << "frames @" << kModelRate << "Hz in" << numChunks
                       << "chunks (window=" << kChunkSize << ", hop=" << kHopSize
                       << "); this can take a while on CPU...";

        // Chunked processing with 50% overlap and overlap-add (OLA)
        std::vector<float> inChunk(static_cast<size_t>(kChunkSize) * 2, 0.0f);
        std::vector<float> stemOut[kNumStems];
        for (int s = 0; s < kNumStems; ++s) {
            stemOut[s].resize(static_cast<size_t>(kChunkSize) * 2, 0.0f);
        }

        for (int chunk = 0; chunk < numChunks; ++chunk) {
            if (m_cancelled) {
                kLogger.info() << "Separation cancelled";
                if (m_config.onFinished) m_config.onFinished(false, {});
                emit finished(false, {});
                return;
            }

            const int startFrame = chunk * kHopSize;
            const int framesIn = std::min(kChunkSize, paddedFrames - startFrame);

            // Fill chunk (zero-padded at tail)
            std::fill(inChunk.begin(), inChunk.end(), 0.0f);
            std::copy_n(paddedInput.constData() + startFrame * 2,
                    framesIn * 2,
                    inChunk.data());

            StemEngine::WaveformResult result;
            for (int s = 0; s < kNumStems; ++s) {
                result.stems[s] = stemOut[s].data();
            }
            result.numSamples = 0;

            if (!onnx.inferWaveform(inChunk.data(), kChunkSize, result)) {
                kLogger.warning() << "inferWaveform failed at chunk" << chunk;
                onnxReady = false; // fall through to passthrough below
                break;
            }

            const int outFrames = result.numSamples; // == kChunkSize normally
            if (outFrames <= 0) {
                kLogger.warning() << "Empty inference result at chunk" << chunk;
                onnxReady = false;
                break;
            }

            // Overlap-add: each output sample receives win[i] from each
            // overlapping window that covers it (Hann ⇒ sum ≈ 1).
            for (int s = 0; s < kNumStems; ++s) {
                const float* src = result.stems[s];
                float* dst = outStems[s]->data();

                for (int i = 0; i < outFrames * 2; ++i) {
                    const int frameIdx = i / 2;
                    const float win = hannWindow[frameIdx];
                    const int outIdx = startFrame * 2 + i;
                    if (outIdx < paddedSamples) {
                        dst[outIdx] += src[i] * win;
                    }
                }
            }

            const float progress = static_cast<float>(chunk + 1) / numChunks;
            if (m_config.onProgress) m_config.onProgress(progress);
            emit progressChanged(progress);
        }

        // Trim the padding, keep only the inference signal length, then
        // resample back to the native rate (exact native length).
        outVocals = outVocals.mid(leftPad * 2, inferSamples);
        outDrums = outDrums.mid(leftPad * 2, inferSamples);
        outBass = outBass.mid(leftPad * 2, inferSamples);
        outOther = outOther.mid(leftPad * 2, inferSamples);
        if (needResample && !inferBuffer.isEmpty()) {
            outVocals = AnalyzerStemSeparation::resampleStereo(
                    outVocals, kModelRate, decodedSampleRate);
            outDrums = AnalyzerStemSeparation::resampleStereo(
                    outDrums, kModelRate, decodedSampleRate);
            outBass = AnalyzerStemSeparation::resampleStereo(
                    outBass, kModelRate, decodedSampleRate);
            outOther = AnalyzerStemSeparation::resampleStereo(
                    outOther, kModelRate, decodedSampleRate);
            outVocals.resize(totalSamples);
            outDrums.resize(totalSamples);
            outBass.resize(totalSamples);
            outOther.resize(totalSamples);
        }

        kLogger.info() << "ONNX separation finished";
    }

    if (!onnxReady) {
        // Fallback: passthrough (full track on vocals, silence elsewhere).
        // Reset all buffers to the original size (a partial run may have
        // resized them to the padded length).
        kLogger.warning() << "Using passthrough stems (vocals=track, rest=silence)";
        outVocals = fullBuffer;
        outDrums.fill(0.0f);
        outDrums.resize(totalSamples);
        outBass.fill(0.0f);
        outBass.resize(totalSamples);
        outOther.fill(0.0f);
        outOther.resize(totalSamples);
    }
#else
    // Passthrough fallback when stem-engine is not available
    QVector<float> outVocals(fullBuffer.size(), 0.0f);
    QVector<float> outDrums(fullBuffer.size(), 0.0f);
    QVector<float> outBass(fullBuffer.size(), 0.0f);
    QVector<float> outOther(fullBuffer.size(), 0.0f);
    outVocals = fullBuffer;
    kLogger.warning() << "stem-engine not compiled in - using passthrough stems";
#endif

    // 4. Write stems to WAV files
    StemCacheManager::StemFiles files;
    files.vocals = m_config.outputDir + "/vocals.wav";
    files.drums = m_config.outputDir + "/drums.wav";
    files.bass = m_config.outputDir + "/bass.wav";
    files.other = m_config.outputDir + "/other.wav";
    files.complete = true;
    files.created = QDateTime::currentDateTime();
    
    auto writeWav = [](const QString& path, const QVector<float>& data, int sampleRate) {
        QFile f(path);
        if (!f.open(QIODevice::WriteOnly)) return false;
        // Write minimal WAV header
        // RIFF header
        f.write("RIFF");
        quint32 fileSize = 36 + data.size() * sizeof(float);
        f.write(reinterpret_cast<const char*>(&fileSize), 4);
        f.write("WAVE");
        // fmt chunk
        f.write("fmt ");
        quint32 fmtSize = 16;
        f.write(reinterpret_cast<const char*>(&fmtSize), 4);
        quint16 audioFormat = 3; // IEEE float
        f.write(reinterpret_cast<const char*>(&audioFormat), 2);
        quint16 numChannels = 2;
        f.write(reinterpret_cast<const char*>(&numChannels), 2);
        quint32 sr = sampleRate;
        f.write(reinterpret_cast<const char*>(&sr), 4);
        quint32 byteRate = sr * numChannels * sizeof(float);
        f.write(reinterpret_cast<const char*>(&byteRate), 4);
        quint16 blockAlign = numChannels * sizeof(float);
        f.write(reinterpret_cast<const char*>(&blockAlign), 2);
        quint16 bitsPerSample = 32;
        f.write(reinterpret_cast<const char*>(&bitsPerSample), 2);
        // data chunk
        f.write("data");
        quint32 dataSize = data.size() * sizeof(float);
        f.write(reinterpret_cast<const char*>(&dataSize), 4);
        f.write(reinterpret_cast<const char*>(data.constData()), dataSize);
        f.close();
        return true;
    };
    
    writeWav(files.vocals, outVocals, decodedSampleRate);
    writeWav(files.drums, outDrums, decodedSampleRate);
    writeWav(files.bass, outBass, decodedSampleRate);
    writeWav(files.other, outOther, decodedSampleRate);

    kLogger.info() << "Wrote stem WAVs to:" << m_config.outputDir;

    // 5. Write a native .stem.mp4 (5 AAC streams + stem atom) so Mixxx can
    // load it through its native stem system (SoundSourceSTEM, native stem
    // volume/mute controls and stem UI). The file is hash-named
    // ({hash}.stem.mp4) so it survives renames of the source track.
    // Key derivation is SHA256(path + mtime + size), never the fragile
    // output-dir basename used before S5.
    const StemCacheManager::CacheKey key =
            StemCacheManager::generateKey(m_config.inputPath);
    StemMp4Writer::Config stemMp4Config;
    stemMp4Config.outputPath = StemCacheManager::stemFilePath(key);
    QDir().mkpath(QFileInfo(stemMp4Config.outputPath).absolutePath());
    stemMp4Config.sampleRate = decodedSampleRate;
    stemMp4Config.numFrames = totalFrames;
    stemMp4Config.stems[0] = outVocals.constData();
    stemMp4Config.stems[1] = outDrums.constData();
    stemMp4Config.stems[2] = outBass.constData();
    stemMp4Config.stems[3] = outOther.constData();
    if (StemMp4Writer::write(stemMp4Config)) {
        files.stemFile = stemMp4Config.outputPath;
        kLogger.info() << "Wrote native stem file:" << files.stemFile;
    } else {
        kLogger.warning() << "Failed to write native stem file, "
                             "falling back to virtual stems only";
    }
    
    // Mark complete in the cache index directly so it survives app shutdown
    // (do not rely only on the queued finished() signal, which may not be
    //  processed if the user closes Mixxx right after separation)
    StemCacheManager::instance().markComplete(key, files);
    
    if (m_config.onProgress) m_config.onProgress(1.0f);
    if (m_config.onFinished) m_config.onFinished(true, files);
    emit finished(true, files);
}

} // namespace mixxx