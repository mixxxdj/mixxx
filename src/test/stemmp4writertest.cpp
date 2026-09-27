#include <gtest/gtest.h>

#include <QDir>
#include <QFile>
#include <QtDebug>

#include "engine/stems/stemmp4writer.h"
#include "sources/soundsourceffmpeg.h"
#include "sources/soundsourceproxy.h"
#include "sources/soundsourcestem.h"
#include "test/mixxxtest.h"
#include "track/steminfoimporter.h"
#include "track/track.h"
#include "util/samplebuffer.h"

using namespace mixxx;

namespace {

constexpr int kSampleRate = 44100;
constexpr int kNumFrames = 44100; // 1 second of stereo audio

QVector<float> makeTone(double freq, float amp = 0.5f) {
    QVector<float> buffer(kNumFrames * 2);
    for (int i = 0; i < kNumFrames; ++i) {
        const float sample = amp * std::sin(2.0 * M_PI * freq * i / kSampleRate);
        buffer[i * 2] = sample;
        buffer[i * 2 + 1] = sample;
    }
    return buffer;
}

class StemMp4WriterTest : public MixxxTest {
  protected:
    void SetUp() override {
        ASSERT_TRUE(SoundSourceProxy::isFileTypeSupported("stem.mp4") ||
                SoundSourceProxy::registerProviders());
        m_outputPath = QDir(getTestDir()).filePath("generated.stem.mp4");
    }

    void TearDown() override {
        QFile::remove(m_outputPath);
    }

    QString m_outputPath;
};

TEST_F(StemMp4WriterTest, WriteAndReadBack) {
    auto vocals = makeTone(440.0, 0.4f);
    auto drums = makeTone(880.0, 0.3f);
    auto bass = makeTone(110.0, 0.5f);
    auto other = makeTone(220.0, 0.2f);

    StemMp4Writer::Config config;
    config.outputPath = m_outputPath;
    config.sampleRate = kSampleRate;
    config.numFrames = kNumFrames;
    config.stems[0] = vocals.constData();
    config.stems[1] = drums.constData();
    config.stems[2] = bass.constData();
    config.stems[3] = other.constData();

    ASSERT_TRUE(StemMp4Writer::write(config));
    ASSERT_TRUE(QFile::exists(m_outputPath));

    // The file must be detected as a stem file by Mixxx.
    EXPECT_TRUE(StemInfoImporter::hasStemAtom(m_outputPath));

    // Manifest must import with the expected stems.
    auto stems = StemInfoImporter::importStemInfos(m_outputPath);
    ASSERT_EQ(stems.size(), 4);
    EXPECT_EQ(stems.at(0).getLabel(), QStringLiteral("Vocals"));
    EXPECT_EQ(stems.at(1).getLabel(), QStringLiteral("Drums"));
    EXPECT_EQ(stems.at(2).getLabel(), QStringLiteral("Bass"));
    EXPECT_EQ(stems.at(3).getLabel(), QStringLiteral("Other"));
    EXPECT_TRUE(stems.at(0).getColor().isValid());

    // SoundSourceSTEM must be able to open it as 8-channel stem audio.
    SoundSourceSTEM sourceStem(QUrl::fromLocalFile(m_outputPath));
    mixxx::AudioSource::OpenParams stemParams;
    stemParams.setChannelCount(mixxx::audio::ChannelCount(8));
    ASSERT_EQ(sourceStem.open(AudioSource::OpenMode::Strict, stemParams),
            AudioSource::OpenResult::Succeeded);
    EXPECT_EQ(mixxx::audio::SignalInfo(mixxx::audio::ChannelCount::stem(),
                      mixxx::audio::SampleRate(kSampleRate)),
            sourceStem.getSignalInfo());

    // A track pointing at the file must report 4 stems.
    TrackPointer pTrack(Track::newTemporary(m_outputPath));
    mixxx::AudioSource::OpenParams openParams;
    openParams.setChannelCount(mixxx::audio::ChannelCount(2));
    SoundSourceProxy proxy(pTrack);
    ASSERT_NE(proxy.openAudioSource(openParams), nullptr);
    EXPECT_EQ(pTrack->getStemInfo().size(), 4);
}

TEST_F(StemMp4WriterTest, SilentStemsAreEncoded) {
    auto vocals = makeTone(440.0, 0.4f);

    StemMp4Writer::Config config;
    config.outputPath = m_outputPath;
    config.sampleRate = kSampleRate;
    config.numFrames = kNumFrames;
    config.stems[0] = vocals.constData();
    // stems 1..3 left null -> encoded as silence
    config.stemNames[1] = QStringLiteral("Drums");

    ASSERT_TRUE(StemMp4Writer::write(config));

    auto stems = StemInfoImporter::importStemInfos(m_outputPath);
    ASSERT_EQ(stems.size(), 4);
    EXPECT_EQ(stems.at(1).getLabel(), QStringLiteral("Drums"));
}

} // namespace
