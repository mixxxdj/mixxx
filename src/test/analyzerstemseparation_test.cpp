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

QVector<float> makeTone(double freq, int sampleRate = kSampleRate,
        int numFrames = kNumFrames, float amp = 0.4f) {
    QVector<float> buf(numFrames * 2);
    for (int i = 0; i < numFrames; ++i) {
        const float s = amp * std::sin(2.0 * M_PI * freq * i / sampleRate);
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

// htdemucs ONNX emits Demucs order [drums, bass, other, vocals] while
// Mixxx slots (StemMp4Writer::Config::stems, SoundSourceSTEM interleave
// [VL VR DL DR BL BR OL OR]) are [vocals, drums, bass, other]. Without the
// permutation the vocals slot gets drums and the other slot gets vocals.
// Pins the routing with one tone per stem (same tones as StemMp4WriterTest:
// 440 vocals, 880 drums, 110 bass, 220 other).
TEST_F(AnalyzerStemSeparationTest, OnnxDemucsOrderMapsToMixxxSlots) {
    // Both tables must be mutually inverse bijections over {0..3}.
    for (int s = 0; s < 4; ++s) {
        EXPECT_EQ(AnalyzerStemSeparation::kMixxxSlotForOnnxStem
                          [AnalyzerStemSeparation::kOnnxStemForMixxxSlot[s]],
                s);
        EXPECT_EQ(AnalyzerStemSeparation::kOnnxStemForMixxxSlot
                          [AnalyzerStemSeparation::kMixxxSlotForOnnxStem[s]],
                s);
    }
    // Mixxx slot s reads ONNX output kOnnxStemForMixxxSlot[s]:
    // slots must be [440 vocals, 880 drums, 110 bass, 220 other].
    EXPECT_EQ((AnalyzerStemSeparation::kOnnxStemForMixxxSlot[0]), 3); // vocals
    EXPECT_EQ((AnalyzerStemSeparation::kOnnxStemForMixxxSlot[1]), 0); // drums
    EXPECT_EQ((AnalyzerStemSeparation::kOnnxStemForMixxxSlot[2]), 1); // bass
    EXPECT_EQ((AnalyzerStemSeparation::kOnnxStemForMixxxSlot[3]), 2); // other

    // Simulate one OLA routing step through the real shared helper used
    // by both offline paths: ONNX-ordered chunk outputs
    // [drums=880, bass=110, other=220, vocals=440] with a unit window must
    // land each tone in its Mixxx slot.
    constexpr int kChunkFrames = 64;
    const QVector<float> unitWin(kChunkFrames, 1.0f);
    QVector<float> onnxBuf[4] = {
            makeTone(880.0, kSampleRate, kChunkFrames), // ONNX 0 = drums
            makeTone(110.0, kSampleRate, kChunkFrames), // ONNX 1 = bass
            makeTone(220.0, kSampleRate, kChunkFrames), // ONNX 2 = other
            makeTone(440.0, kSampleRate, kChunkFrames), // ONNX 3 = vocals
    };
    float* srcStems[4] = {
            onnxBuf[0].data(), onnxBuf[1].data(), onnxBuf[2].data(), onnxBuf[3].data()};
    QVector<float> mixxxSlot[4] = {
            QVector<float>(kChunkFrames * 2, 0.0f),
            QVector<float>(kChunkFrames * 2, 0.0f),
            QVector<float>(kChunkFrames * 2, 0.0f),
            QVector<float>(kChunkFrames * 2, 0.0f),
    };
    QVector<float>* pMixxxSlots[4] = {
            &mixxxSlot[0], &mixxxSlot[1], &mixxxSlot[2], &mixxxSlot[3]};
    AnalyzerStemSeparation::accumulateChunk(
            srcStems, kChunkFrames, unitWin, 0, kChunkFrames * 2, pMixxxSlots);
    EXPECT_EQ(mixxxSlot[0], makeTone(440.0, kSampleRate, kChunkFrames)); // vocals
    EXPECT_EQ(mixxxSlot[1], makeTone(880.0, kSampleRate, kChunkFrames)); // drums
    EXPECT_EQ(mixxxSlot[2], makeTone(110.0, kSampleRate, kChunkFrames)); // bass
    EXPECT_EQ(mixxxSlot[3], makeTone(220.0, kSampleRate, kChunkFrames)); // other
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
    // N19: expected key follows the configured stem mode (default 3 is
    // versioned, legacy 4 equals generateKey()).
    const auto key = StemCacheManager::generateKeyForMode(
            wavPath, AnalyzerStemSeparation::stemMode(config()));
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
    // N19: expected key follows the configured stem mode (default 3 is
    // versioned, legacy 4 equals generateKey()).
    const auto key = StemCacheManager::generateKeyForMode(
            wavPath, AnalyzerStemSeparation::stemMode(config()));
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

// N9 Step5.5 verdict: NO-APLICA documentado.
// track_locations.location must keep pointing at the ORIGINAL audio file
// (TrackDAO::addTracksPrepare INSERTs the library location; library.location
// is an FK into track_locations.id). Rewriting it to the derived
// {hash}.stem.mp4 cache file would orphan the original, make the next
// library re-scan re-add it as a duplicate, and break DirectoryDAO
// relocate/verify + missing-track detection. The canonical S5/2.6 lookup is
// the JSON index (StemCacheManager::markComplete/getStemFiles) resolved at
// load time by EngineDeck -> aiStemFileReady -> BaseTrackPlayerImpl::
// slotLoadAiStemFile, which loads the stem file as a TEMPORARY track while
// the library row is untouched. This test pins that contract: markComplete
// with a hash-named cache path leaves Track::getLocation() unchanged and
// the cache resolves to {hash}.stem.mp4.
TEST_F(AnalyzerStemSeparationTest, StemCacheDoesNotRewriteTrackLocation) {
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    const QString wavPath = tmp.filePath("orig.wav");
    ASSERT_TRUE(writeFloatWav(wavPath, makeSineMix(), kSampleRate));

    TrackPointer pTrack(Track::newTemporary(wavPath));
    ASSERT_TRUE(pTrack);
    const auto key = StemCacheManager::generateKey(wavPath);
    const QString cachePath = StemCacheManager::stemFilePath(key);

    // Invariant 1: cache artifact is hash-named and distinct from the
    // library location stored in track_locations.
    EXPECT_TRUE(cachePath.endsWith(QString::fromUtf8(key) + ".stem.mp4"));
    EXPECT_NE(pTrack->getLocation(), cachePath);
    EXPECT_EQ(pTrack->getLocation(), wavPath);

    // Invariant 2: markComplete (JSON index write) does not touch the Track.
    // Use an isolated scratch key so no real cache entry is polluted.
    const StemCacheManager::CacheKey docKey = QByteArray("n9-step55-noaplica-doc");
    StemCacheManager::instance().markFailed(docKey);
    StemCacheManager::StemFiles files;
    files.complete = true;
    files.created = QDateTime::currentDateTime();
    files.stemFile = cachePath;
    StemCacheManager::instance().markComplete(docKey, files);
    EXPECT_EQ(pTrack->getLocation(), wavPath);
    EXPECT_EQ(StemCacheManager::instance().getStemFiles(docKey).stemFile, cachePath);
    EXPECT_TRUE(StemCacheManager::instance().hasStems(docKey));
}

// N18: optional overlap + 3-stem mode helpers (pure, no ONNX needed).
TEST_F(AnalyzerStemSeparationTest, HopSizeDefaultsToHalfChunk) {
    EXPECT_EQ(AnalyzerStemSeparation::hopSizeFor(1024, 0.5), 512);
    EXPECT_EQ(AnalyzerStemSeparation::hopSizeFor(1024, 0.0), 512);
    EXPECT_EQ(AnalyzerStemSeparation::hopSizeFor(1024, 0.9), 512);
    // 25% overlap: hop = 3N/4, even.
    EXPECT_EQ(AnalyzerStemSeparation::hopSizeFor(1024, 0.25), 768);
    // htdemucs window 343980: 3N/4 = 257985 -> clamped to even 257984.
    EXPECT_EQ(AnalyzerStemSeparation::hopSizeFor(343980, 0.25), 257984);
    EXPECT_EQ(AnalyzerStemSeparation::hopSizeFor(343980, 0.5), 171990);
    EXPECT_EQ(AnalyzerStemSeparation::hopSizeFor(0, 0.5), 0);
}

TEST_F(AnalyzerStemSeparationTest, OverlapAndStemModeDefaultWithoutConfig) {
    EXPECT_DOUBLE_EQ(AnalyzerStemSeparation::overlapRatio(UserSettingsPointer()), 0.25);
    EXPECT_EQ(AnalyzerStemSeparation::stemMode(UserSettingsPointer()), 3);
}

// N19: legacy 0.5/4 stay available as manual opt-ins via config.
TEST_F(AnalyzerStemSeparationTest, OverlapAndStemModeLegacyManualOptIn) {
    config()->setValue(ConfigKey("[StemSeparation]", "overlap"), 0.5);
    config()->setValue(ConfigKey("[StemSeparation]", "stem_mode"), 4);
    EXPECT_DOUBLE_EQ(AnalyzerStemSeparation::overlapRatio(config()), 0.5);
    EXPECT_EQ(AnalyzerStemSeparation::stemMode(config()), 4);
    EXPECT_EQ(AnalyzerStemSeparation::hopSizeFor(1024, 0.5), 512);
    config()->setValue(ConfigKey("[StemSeparation]", "overlap"), 0.25);
    config()->setValue(ConfigKey("[StemSeparation]", "stem_mode"), 3);
    EXPECT_DOUBLE_EQ(AnalyzerStemSeparation::overlapRatio(config()), 0.25);
    EXPECT_EQ(AnalyzerStemSeparation::stemMode(config()), 3);
}

TEST_F(AnalyzerStemSeparationTest, FoldTo3StemModeSumsBassIntoOther) {
    QVector<float> v(8, 1.0f), d(8, 2.0f), b(8, 3.0f), o(8, 4.0f);
    QVector<float>* outs[4] = {&v, &d, &b, &o};
    AnalyzerStemSeparation::foldTo3StemMode(outs);
    EXPECT_EQ(v, QVector<float>(8, 1.0f));
    EXPECT_EQ(d, QVector<float>(8, 2.0f));
    for (float x : b) {
        EXPECT_EQ(x, 0.0f);
    }
    EXPECT_EQ(o, QVector<float>(8, 7.0f)); // bass + other
}

TEST_F(AnalyzerStemSeparationTest, NormalizeWolaRestoresUnity) {
    // Identity-model simulation at 25% overlap: accumulated = input * weight.
    QVector<float> weight(4, 2.0f);
    QVector<float> s0(8, 2.0f), s1(8, 0.0f), s2(8, 0.0f), s3(8, 0.0f);
    QVector<float>* outs[4] = {&s0, &s1, &s2, &s3};
    AnalyzerStemSeparation::normalizeWola(outs, weight);
    EXPECT_EQ(s0, QVector<float>(8, 1.0f));
}

TEST_F(AnalyzerStemSeparationTest, VersionedKeyIsolatesMode3FromMode4) {
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    const QString p = tmp.filePath("m.wav");
    ASSERT_TRUE(writeFloatWav(p, makeSineMix(), kSampleRate));
    // Mode 4 versioned key == legacy key (existing entries stay valid).
    EXPECT_EQ(StemCacheManager::generateKeyForMode(p, 4),
            StemCacheManager::generateKey(p));
    // Mode 3 key differs (no cache poisoning across modes).
    EXPECT_NE(StemCacheManager::generateKeyForMode(p, 3),
            StemCacheManager::generateKey(p));
    EXPECT_NE(StemCacheManager::generateKeyForMode(p, 3),
            StemCacheManager::generateKeyForMode(p, 4));
}

// N23: lightweight realtime route. Priority: explicit arg >
// $MIXXX_STEM_REALTIME_MODEL > [StemSeparation],realtime_model= > default.
// A synthetic fixture standing in for a small 2-3 stem model is selected
// when it exists; otherwise the live path falls back to htdemucs. No
// downloads; required ONNX input signature for all candidates is [1,2,N].
TEST_F(AnalyzerStemSeparationTest, RealtimeModelPathResolutionAndFallback) {
    // Default documents the small variant, distinct from offline htdemucs.
    EXPECT_EQ(AnalyzerStemSeparation::defaultRealtimeModelPath(),
            QStringLiteral(
                    "/usr/local/share/stem-models/htdemucs_small_fp16weights.onnx"));
    EXPECT_NE(AnalyzerStemSeparation::defaultRealtimeModelPath(),
            AnalyzerStemSeparation::defaultModelPath());

    // Candidate -> live stem count mapping (pure, no filesystem).
    EXPECT_EQ(AnalyzerStemSeparation::liveStemCountForModel(
                      "/m/htdemucs_small_fp16weights.onnx"),
            3);
    EXPECT_EQ(AnalyzerStemSeparation::liveStemCountForModel("/m/umx_small.onnx"), 3);
    EXPECT_EQ(AnalyzerStemSeparation::liveStemCountForModel(
                      "/m/spleeter_2stems.onnx"),
            2);
    EXPECT_EQ(AnalyzerStemSeparation::liveStemCountForModel(
                      AnalyzerStemSeparation::defaultModelPath()),
            4);

    // Env override wins over config and default.
    qputenv("MIXXX_STEM_REALTIME_MODEL", QByteArray("/tmp/rt_env.onnx"));
    config()->setValue(
            ConfigKey("[StemSeparation]", "realtime_model"), QStringLiteral("/tmp/rt_cfg.onnx"));
    EXPECT_EQ(AnalyzerStemSeparation::effectiveRealtimeModelPath(config()),
            QStringLiteral("/tmp/rt_env.onnx"));
    qunsetenv("MIXXX_STEM_REALTIME_MODEL");

    // Config wins over default when env is unset.
    EXPECT_EQ(AnalyzerStemSeparation::effectiveRealtimeModelPath(config()),
            QStringLiteral("/tmp/rt_cfg.onnx"));
    config()->setValue(
            ConfigKey("[StemSeparation]", "realtime_model"), QString());
    EXPECT_EQ(AnalyzerStemSeparation::effectiveRealtimeModelPath(config()),
            AnalyzerStemSeparation::defaultRealtimeModelPath());

    // Explicit arg wins over everything.
    qputenv("MIXXX_STEM_REALTIME_MODEL", QByteArray("/tmp/rt_env.onnx"));
    EXPECT_EQ(AnalyzerStemSeparation::effectiveRealtimeModelPath(
                      config(), QStringLiteral("/tmp/rt_arg.onnx")),
            QStringLiteral("/tmp/rt_arg.onnx"));
    qunsetenv("MIXXX_STEM_REALTIME_MODEL");

    // Fallback: missing realtime file -> htdemucs path.
    qputenv("MIXXX_STEM_MODEL", QByteArray("/tmp/offline_htdemucs.onnx"));
    EXPECT_EQ(
            AnalyzerStemSeparation::liveModelPathForRealtime(
                    config(), QStringLiteral("/tmp/does-not-exist-rt-small.onnx")),
            QStringLiteral("/tmp/offline_htdemucs.onnx"));
    qunsetenv("MIXXX_STEM_MODEL");

    // Live: synthetic fixture exists -> small model selected (no download).
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    const QString fixture = tmp.filePath("htdemucs_small_fixture.onnx");
    {
        QFile f(fixture);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("N23-synthetic-fixture");
    }
    EXPECT_EQ(AnalyzerStemSeparation::liveModelPathForRealtime(config(), fixture),
            fixture);
    EXPECT_EQ(AnalyzerStemSeparation::liveStemCountForModel(fixture), 3);
}

// N21: chunk-streaming parcial. Tras los primeros kPartialChunks chunks
// (~12 s a 50% overlap) se escribe {hash}.partial.stem.mp4 + markPartial()
// para reproducir (aiStemFileReady parcial) mientras el resto sigue en
// background con progreso; markComplete final no se adelanta.
TEST_F(AnalyzerStemSeparationTest, PartialStreamingAfterThreeChunks) {
    // shouldEmitPartial: exactamente una vez (chunk 2) y solo si queda resto.
    EXPECT_EQ(AnalyzerStemSeparation::kPartialChunks, 3);
    EXPECT_FALSE(AnalyzerStemSeparation::shouldEmitPartial(0, 10));
    EXPECT_FALSE(AnalyzerStemSeparation::shouldEmitPartial(1, 10));
    EXPECT_TRUE(AnalyzerStemSeparation::shouldEmitPartial(2, 10));
    EXPECT_FALSE(AnalyzerStemSeparation::shouldEmitPartial(3, 10));
    // Sin resto no hay preview (job corto: directo a finished completo).
    EXPECT_FALSE(AnalyzerStemSeparation::shouldEmitPartial(2, 3));
    EXPECT_FALSE(AnalyzerStemSeparation::shouldEmitPartial(0, 1));

    // Prefijo asentado: chunksDone*hop clamped a [0, inferFrames].
    // htdemucs 343980 @50% overlap: hop 171990 -> 3 hops = 515970 (~11.7 s).
    EXPECT_EQ(AnalyzerStemSeparation::partialPrefixFrames(3, 171990, 1000000),
            515970);
    EXPECT_EQ(AnalyzerStemSeparation::partialPrefixFrames(3, 257984, 10000000),
            773952);
    EXPECT_EQ(AnalyzerStemSeparation::partialPrefixFrames(3, 171990, 100),
            100);
    EXPECT_EQ(AnalyzerStemSeparation::partialPrefixFrames(0, 171990, 1000000), 0);
    EXPECT_EQ(AnalyzerStemSeparation::partialPrefixFrames(3, 0, 1000000), 0);

    // markPartial: preview visible sin completar; no limpia el claim.
    const StemCacheManager::CacheKey pkey = QByteArray("n21-partial-test-key");
    StemCacheManager::instance().markFailed(pkey);
    EXPECT_TRUE(StemCacheManager::instance().tryMarkProcessing(pkey));
    EXPECT_FALSE(StemCacheManager::instance().hasStems(pkey));
    EXPECT_FALSE(StemCacheManager::instance().hasPartial(pkey));
    // partialStemFilePath es distinto del final y con sufijo .partial.
    const QString finalPath = StemCacheManager::stemFilePath(pkey);
    const QString partialPath = StemCacheManager::partialStemFilePath(pkey);
    EXPECT_TRUE(partialPath.endsWith(QStringLiteral(".partial.stem.mp4")));
    EXPECT_NE(partialPath, finalPath);

    // Entrada parcial sin archivo -> hasPartial false (no preview jugable).
    StemCacheManager::StemFiles pending;
    pending.stemFile = partialPath;
    pending.complete = false;
    pending.created = QDateTime::currentDateTime();
    StemCacheManager::instance().markPartial(pkey, pending);
    EXPECT_FALSE(StemCacheManager::instance().hasStems(pkey));
    EXPECT_FALSE(StemCacheManager::instance().hasPartial(pkey));
    // tryMarkProcessing sigue reclamado: el job de fondo sigue vivo.
    EXPECT_FALSE(StemCacheManager::instance().tryMarkProcessing(pkey));

    // Preview jugable: escribe un .stem.mp4 parcial real (4 tonos, 0.5 s)
    // y verifica stem atom + hasPartial; luego markComplete lo promueve.
    QTemporaryDir tmpPreview;
    ASSERT_TRUE(tmpPreview.isValid());
    const QString previewPath = tmpPreview.filePath("preview.partial.stem.mp4");
    constexpr int kFrames = 22050; // 0.5 s @44100
    const QVector<float> v = makeTone(440.0, kSampleRate, kFrames);
    const QVector<float> d = makeTone(880.0, kSampleRate, kFrames);
    const QVector<float> b = makeTone(110.0, kSampleRate, kFrames);
    const QVector<float> o = makeTone(220.0, kSampleRate, kFrames);
    mixxx::StemMp4Writer::Config pcfg;
    pcfg.outputPath = previewPath;
    pcfg.sampleRate = kSampleRate;
    pcfg.numFrames = kFrames;
    pcfg.stems[0] = v.constData();
    pcfg.stems[1] = d.constData();
    pcfg.stems[2] = b.constData();
    pcfg.stems[3] = o.constData();
    ASSERT_TRUE(mixxx::StemMp4Writer::write(pcfg));
    EXPECT_TRUE(mixxx::StemInfoImporter::hasStemAtom(previewPath));

    StemCacheManager::StemFiles playable;
    playable.stemFile = previewPath;
    playable.complete = false;
    playable.created = QDateTime::currentDateTime();
    StemCacheManager::instance().markPartial(pkey, playable);
    EXPECT_FALSE(StemCacheManager::instance().hasStems(pkey));
    EXPECT_TRUE(StemCacheManager::instance().hasPartial(pkey));
    EXPECT_EQ(StemCacheManager::instance().getStemFiles(pkey).stemFile,
            previewPath);

    // El completo final sustituye al parcial (mismo key, complete=true).
    StemCacheManager::StemFiles done = playable;
    done.complete = true;
    StemCacheManager::instance().markComplete(pkey, done);
    EXPECT_TRUE(StemCacheManager::instance().hasStems(pkey));
    EXPECT_FALSE(StemCacheManager::instance().hasPartial(pkey));
    StemCacheManager::instance().markFailed(pkey);
}

} // namespace
