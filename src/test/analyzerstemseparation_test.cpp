#include <gtest/gtest.h>

#include <QDir>
#include <QFile>
#include <QTemporaryDir>

#include "analyzer/analyzerstemseparation.h"
#include "analyzer/analyzertrack.h"
#include "engine/stems/stemcachemanager.h"
#include "engine/stems/stemmp4writer.h"
#include "sources/soundsourceproxy.h"
#include "test/mixxxtest.h"
#include "track/steminfoimporter.h"
#include "track/track.h"

namespace {

constexpr int kSampleRate = 44100;
constexpr int kNumFrames = 44100; // 1 s stereo

QVector<float> makeSineMix() {
    QVector<float> buf(kNumFrames * 2);
    for (int i = 0; i < kNumFrames; ++i) {
        const float s = 0.4f * std::sin(2.0 * M_PI * 440.0 * i / kSampleRate);
        buf[i * 2] = s;
        buf[i * 2 + 1] = s;
    }
    return buf;
}

QVector<float> makeSineMixAt(int sampleRate, int numFrames) {
    QVector<float> buf(numFrames * 2);
    for (int i = 0; i < numFrames; ++i) {
        const float s = 0.4f * std::sin(2.0 * M_PI * 440.0 * i / sampleRate);
        buf[i * 2] = s;
        buf[i * 2 + 1] = s;
    }
    return buf;
}

bool writeFloatWav(const QString& path, const QVector<float>& data, int sampleRate) {
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) {
        return false;
    }
    f.write("RIFF");
    quint32 fileSize = 36 + data.size() * sizeof(float);
    f.write(reinterpret_cast<const char*>(&fileSize), 4);
    f.write("WAVEfmt ");
    quint32 fmtSize = 16;
    f.write(reinterpret_cast<const char*>(&fmtSize), 4);
    quint16 audioFormat = 3;
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
    f.write("data");
    quint32 dataSize = data.size() * sizeof(float);
    f.write(reinterpret_cast<const char*>(&dataSize), 4);
    f.write(reinterpret_cast<const char*>(data.constData()), dataSize);
    return true;
}

class AnalyzerStemSeparationTest : public MixxxTest {
  protected:
    void SetUp() override {
        ASSERT_TRUE(SoundSourceProxy::isFileTypeSupported("stem.mp4") ||
                SoundSourceProxy::registerProviders());
    }
};

// Hann 50% OLA with an identity model (each chunk returns its input)
// must reconstruct the input ~= 1.0 in the interior (away from the padded
// edges). Same window formula as OfflineSeparator::run().
TEST_F(AnalyzerStemSeparationTest, OlaHannReconstructionNearUnity) {
    constexpr int kChunkSize = 1024;
    const int hop = kChunkSize / 2;
    const QVector<float> hann = AnalyzerStemSeparation::makeHannWindow(kChunkSize);
    ASSERT_EQ(hann.size(), kChunkSize);

    // Interior sum of two overlapping Hann windows ~= 1.0. The (N-1)
    // symmetric convention used by OfflineSeparator is not exactly
    // constant-overlap-add at 50% (deviation up to ~1.1e-3 for N=1024),
    // so the bound below documents the real behaviour, not ideal unity.
    for (int i = 0; i < hop; ++i) {
        EXPECT_NEAR(hann[i] + hann[i + hop], 1.0f, 2e-3);
    }

    // Build a synthetic mix and identity chunks over the padded signal.
    const QVector<float> mix = makeSineMix();
    const int totalFrames = kNumFrames;
    const int paddedFrames = totalFrames + 2 * hop;
    const int paddedSamples = paddedFrames * 2;
    QVector<float> padded(paddedSamples, 0.0f);
    std::copy_n(mix.constData(), mix.size(), padded.begin() + hop * 2);
    const int numChunks = (paddedFrames + hop - 1) / hop;

    // srcChunks[chunk][stem]: stem 0 = padded input slice, rest = silence
    QVector<QVector<QVector<float>>> chunks;
    chunks.reserve(numChunks);
    for (int c = 0; c < numChunks; ++c) {
        QVector<QVector<float>> stems(4);
        for (int s = 0; s < 4; ++s) {
            stems[s].resize(kChunkSize * 2, 0.0f);
        }
        const int start = c * hop * 2;
        const int avail = std::min(kChunkSize * 2, paddedSamples - start);
        std::copy_n(padded.constData() + start, avail, stems[0].begin());
        chunks.push_back(stems);
    }

    QVector<float> out[4];
    AnalyzerStemSeparation::overlapAdd(chunks, kChunkSize, totalFrames, hann, out);
    ASSERT_EQ(out[0].size(), mix.size());
    // Interior frames (one hop away from each edge) reconstruct ~= 1.0
    double maxErr = 0.0;
    for (int f = hop; f < totalFrames - hop; ++f) {
        maxErr = std::max(maxErr,
                static_cast<double>(std::abs(out[0][f * 2] - mix[f * 2])));
    }
    EXPECT_LT(maxErr, 2e-3) << "OLA Hann interior must reconstruct input";
    // Silent stems stay silent
    for (int s = 1; s < 4; ++s) {
        for (float v : out[s]) {
            EXPECT_EQ(v, 0.0f);
        }
    }
}

TEST_F(AnalyzerStemSeparationTest, PassthroughFallback) {
    const QVector<float> mix = makeSineMix();
    QVector<float> o0, o1, o2, o3;
    QVector<float>* outs[4] = {&o0, &o1, &o2, &o3};
    AnalyzerStemSeparation::passthrough(mix, outs);
    EXPECT_EQ(o0, mix);
    EXPECT_EQ(o1.size(), mix.size());
    for (float v : o1) {
        EXPECT_EQ(v, 0.0f);
    }
}

TEST_F(AnalyzerStemSeparationTest, ModelPathDocumentedAndOverridable) {
    EXPECT_EQ(AnalyzerStemSeparation::defaultModelPath(),
            QStringLiteral("/usr/local/share/stem-models/htdemucs_fp16weights.onnx"));
    qputenv("MIXXX_STEM_MODEL", QByteArray("/tmp/custom.onnx"));
    EXPECT_EQ(AnalyzerStemSeparation::effectiveModelPath(),
            QStringLiteral("/tmp/custom.onnx"));
    qunsetenv("MIXXX_STEM_MODEL");
    EXPECT_EQ(AnalyzerStemSeparation::effectiveModelPath(),
            AnalyzerStemSeparation::defaultModelPath());
}

TEST_F(AnalyzerStemSeparationTest, CacheKeyDependsOnPathMtimeSize) {
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    const QString p1 = tmp.filePath("a.wav");
    const QString p2 = tmp.filePath("b.wav");
    const QVector<float> mix = makeSineMix();
    ASSERT_TRUE(writeFloatWav(p1, mix, kSampleRate));
    ASSERT_TRUE(writeFloatWav(p2, mix, kSampleRate));
    // Same size+content but different path -> different key
    EXPECT_NE(StemCacheManager::generateKey(p1), StemCacheManager::generateKey(p2));
    // Same file twice -> same key
    EXPECT_EQ(StemCacheManager::generateKey(p1), StemCacheManager::generateKey(p1));
    // stemFilePath is hash-named
    const auto key = StemCacheManager::generateKey(p1);
    const QString stemPath = StemCacheManager::stemFilePath(key);
    EXPECT_TRUE(stemPath.endsWith(QString::fromUtf8(key) + ".stem.mp4"));
}

TEST_F(AnalyzerStemSeparationTest, TryMarkProcessingPreventsDoubleWork) {
    const StemCacheManager::CacheKey key = QByteArray("s5-test-key-1234");
    // Ensure clean state
    StemCacheManager::instance().markFailed(key);
    EXPECT_TRUE(StemCacheManager::instance().tryMarkProcessing(key));
    // Second claimant loses
    EXPECT_FALSE(StemCacheManager::instance().tryMarkProcessing(key));
    StemCacheManager::instance().markFailed(key);
    // After failure the key is claimable again
    EXPECT_TRUE(StemCacheManager::instance().tryMarkProcessing(key));
    StemCacheManager::instance().markFailed(key);
}

// Full roundtrip through the Analyzer with synthetic stems: first analysis
// writes {hash}.stem.mp4 once (passthrough when no model is installed);
// second initialize() is a cache hit and performs no inference.
TEST_F(AnalyzerStemSeparationTest, CacheRoundtripSecondLoadHitsWithoutInference) {
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    const QString wavPath = tmp.filePath("synth.wav");
    ASSERT_TRUE(writeFloatWav(wavPath, makeSineMix(), kSampleRate));

    TrackPointer pTrack(Track::newTemporary(wavPath));
    ASSERT_TRUE(pTrack);
    const auto key = StemCacheManager::generateKey(wavPath);
    // Clean any previous run for this key
    {
        QDir d(StemCacheManager::stemDir(key));
        if (d.exists()) {
            d.removeRecursively();
        }
        StemCacheManager::instance().markFailed(key);
    }

    AnalyzerStemSeparation analyzer(config());
    ASSERT_TRUE(analyzer.initialize(AnalyzerTrack(pTrack),
            mixxx::audio::SampleRate(kSampleRate),
            mixxx::audio::ChannelCount::stereo(),
            kNumFrames));
    const QVector<float> mix = makeSineMix();
    // Feed in AnalyzerThread-sized chunks
    constexpr int kFeed = 4096 * 2;
    for (int off = 0; off < mix.size(); off += kFeed) {
        const int n = qMin(kFeed, static_cast<int>(mix.size() - off));
        ASSERT_TRUE(analyzer.processSamples(mix.constData() + off, n));
    }
    analyzer.storeResults(pTrack);
    analyzer.cleanup();

    EXPECT_TRUE(StemCacheManager::instance().hasStems(key));
    const auto files = StemCacheManager::instance().getStemFiles(key);
    ASSERT_FALSE(files.stemFile.isEmpty());
    EXPECT_TRUE(QFile::exists(files.stemFile));
    EXPECT_TRUE(files.stemFile.endsWith(QString::fromUtf8(key) + ".stem.mp4"));
    EXPECT_TRUE(mixxx::StemInfoImporter::hasStemAtom(files.stemFile));

    const QDateTime firstCreated = files.created;

    // Second load: initialize() must decline (cache hit, no inference).
    AnalyzerStemSeparation analyzer2(config());
    EXPECT_FALSE(analyzer2.initialize(AnalyzerTrack(pTrack),
            mixxx::audio::SampleRate(kSampleRate),
            mixxx::audio::ChannelCount::stereo(),
            kNumFrames));
    analyzer2.cleanup();
    EXPECT_TRUE(StemCacheManager::instance().hasStems(key));

    // Cleanup test artifact from the real cache dir
    QDir d(StemCacheManager::stemDir(key));
    d.removeRecursively();
    StemCacheManager::instance().markFailed(key);
    Q_UNUSED(firstCreated);
}

TEST_F(AnalyzerStemSeparationTest, ResampleStereoNoOpOnSameOrInvalidRate) {
    const QVector<float> mix = makeSineMix();
    EXPECT_EQ(AnalyzerStemSeparation::resampleStereo(mix, kSampleRate, kSampleRate), mix);
    EXPECT_EQ(AnalyzerStemSeparation::resampleStereo(mix, 0, 44100), mix);
    EXPECT_EQ(AnalyzerStemSeparation::resampleStereo(mix, 48000, 0), mix);
    EXPECT_TRUE(AnalyzerStemSeparation::resampleStereo(QVector<float>(), 48000, 44100)
            .isEmpty());
}

// 48000 -> 44100 must produce exactly round(48000 * 44100/48000) frames and
// preserve a 440 Hz sine (linear interpolation error is small at this ratio).
TEST_F(AnalyzerStemSeparationTest, ResampleStereo48000To44100) {
    constexpr int kRate48 = 48000;
    constexpr int kFrames48 = 48000; // 1 s stereo @ 48 kHz
    const QVector<float> mix48 = makeSineMixAt(kRate48, kFrames48);
    const QVector<float> mix44 =
            AnalyzerStemSeparation::resampleStereo(mix48, kRate48, 44100);
    EXPECT_EQ(mix44.size(), 44100 * 2);
    const QVector<float> ref44 = makeSineMixAt(44100, 44100);
    double maxErr = 0.0;
    for (int i = 0; i < mix44.size(); ++i) {
        maxErr = std::max(maxErr,
                static_cast<double>(std::abs(mix44[i] - ref44[i])));
    }
    EXPECT_LT(maxErr, 0.05) << "linear resample must track the sine closely";
}

// 48000 -> 44100 -> 48000 roundtrip must restore the native frame count
// with small error (documents the quality of the offline linear resampler).
TEST_F(AnalyzerStemSeparationTest, ResampleStereoRoundtrip48000) {
    constexpr int kRate48 = 48000;
    constexpr int kFrames48 = 48000;
    const QVector<float> mix48 = makeSineMixAt(kRate48, kFrames48);
    const QVector<float> down =
            AnalyzerStemSeparation::resampleStereo(mix48, kRate48, 44100);
    const QVector<float> up =
            AnalyzerStemSeparation::resampleStereo(down, 44100, kRate48);
    ASSERT_EQ(up.size(), mix48.size());
    double maxErr = 0.0;
    for (int i = 0; i < mix48.size(); ++i) {
        maxErr = std::max(
                maxErr, static_cast<double>(std::abs(up[i] - mix48[i])));
    }
    EXPECT_LT(maxErr, 0.05) << "roundtrip must preserve the sine closely";
}

// Full Analyzer roundtrip with a 48000 Hz track: inference runs at 44100 Hz
// internally, the cached .stem.mp4 keeps the native rate/length, and the
// second load is a cache hit performing no inference.
TEST_F(AnalyzerStemSeparationTest, CacheRoundtrip48000HzSecondLoadHitsWithoutInference) {
    constexpr int kRate48 = 48000;
    constexpr int kFrames48 = 48000; // 1 s stereo @ 48 kHz
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    const QString wavPath = tmp.filePath("synth48.wav");
    ASSERT_TRUE(writeFloatWav(wavPath, makeSineMixAt(kRate48, kFrames48), kRate48));

    TrackPointer pTrack(Track::newTemporary(wavPath));
    ASSERT_TRUE(pTrack);
    const auto key = StemCacheManager::generateKey(wavPath);
    {
        QDir d(StemCacheManager::stemDir(key));
        if (d.exists()) {
            d.removeRecursively();
        }
        StemCacheManager::instance().markFailed(key);
    }

    AnalyzerStemSeparation analyzer(config());
    ASSERT_TRUE(analyzer.initialize(AnalyzerTrack(pTrack),
            mixxx::audio::SampleRate(kRate48),
            mixxx::audio::ChannelCount::stereo(),
            kFrames48));
    const QVector<float> mix = makeSineMixAt(kRate48, kFrames48);
    constexpr int kFeed = 4096 * 2;
    for (int off = 0; off < mix.size(); off += kFeed) {
        const int n = qMin(kFeed, static_cast<int>(mix.size() - off));
        ASSERT_TRUE(analyzer.processSamples(mix.constData() + off, n));
    }
    analyzer.storeResults(pTrack);
    analyzer.cleanup();

    EXPECT_TRUE(StemCacheManager::instance().hasStems(key));
    const auto files = StemCacheManager::instance().getStemFiles(key);
    ASSERT_FALSE(files.stemFile.isEmpty());
    EXPECT_TRUE(QFile::exists(files.stemFile));
    EXPECT_TRUE(files.stemFile.endsWith(QString::fromUtf8(key) + ".stem.mp4"));
    EXPECT_TRUE(mixxx::StemInfoImporter::hasStemAtom(files.stemFile));

    // The cached stem must decode at the native rate with native length.
    {
        TrackPointer pStemTrack(Track::newTemporary(files.stemFile));
        ASSERT_TRUE(pStemTrack);
        SoundSourceProxy proxy(pStemTrack);
        mixxx::AudioSource::OpenParams params;
        params.setChannelCount(mixxx::audio::ChannelCount::stereo());
        mixxx::AudioSourcePointer audioSource = proxy.openAudioSource(params);
        ASSERT_TRUE(audioSource);
        EXPECT_EQ(audioSource->getSignalInfo().getSampleRate(),
                mixxx::audio::SampleRate(kRate48));
        EXPECT_EQ(audioSource->frameLength(), kFrames48);
    }

    // Second load: initialize() must decline (cache hit, no inference).
    AnalyzerStemSeparation analyzer2(config());
    EXPECT_FALSE(analyzer2.initialize(AnalyzerTrack(pTrack),
            mixxx::audio::SampleRate(kRate48),
            mixxx::audio::ChannelCount::stereo(),
            kFrames48));
    analyzer2.cleanup();
    EXPECT_TRUE(StemCacheManager::instance().hasStems(key));

    QDir d(StemCacheManager::stemDir(key));
    d.removeRecursively();
    StemCacheManager::instance().markFailed(key);
}

} // namespace
