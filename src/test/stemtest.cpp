#include <gtest/gtest.h>

#include <QtDebug>

#include "sources/soundsourceproxy.cpp"
#include "test/mixxxtest.h"
#include "track/track.h"
#include "util/samplebuffer.h"

using namespace mixxx;

#define STEM_FILE QStringLiteral("stems/sin_%1.stem.mp4").arg(QString::fromStdString(GetParam()))

namespace {

const std::vector<std::string> supportedCodecs = {
#if !defined(Q_OS_WIN)
        "AAC_256kbps_VBR",
#endif
        "ALAC_24bit"};

const QList<QString> kStemFiles = {
        "01-drum.wav",
        "02-bass.wav",
        "03-melody.wav",
        "04-vocal.wav",
};

class StemFixture : public MixxxTest, public ::testing::WithParamInterface<std::string> {
  protected:
    void SetUp() override {
        ASSERT_TRUE(SoundSourceProxy::isFileTypeSupported("stem.mp4") ||
                SoundSourceProxy::registerProviders());
    }
};

TEST_P(StemFixture, FetchStemInfo) {
    TrackPointer pTrack(Track::newTemporary(getTestDir().filePath(STEM_FILE)));

    mixxx::AudioSource::OpenParams config;
    config.setChannelCount(mixxx::audio::ChannelCount(2));

    ASSERT_NE(SoundSourceProxy(pTrack).openAudioSource(config), nullptr);

    auto stemInfo = pTrack->getStemInfo();
    ASSERT_EQ(stemInfo.size(), 4);
    ASSERT_EQ(stemInfo.at(0), StemInfo("Drums", QColor(0xfd, 0x4a, 0x4a)));  // #fd4a4a
    ASSERT_EQ(stemInfo.at(1), StemInfo("Bass", QColor(0xff, 0xff, 0x00)));   // #ffff00
    ASSERT_EQ(stemInfo.at(2), StemInfo("Synths", QColor(0x00, 0xe8, 0xe8))); // #00e8e8
    ASSERT_EQ(stemInfo.at(3), StemInfo("Vox", QColor(0xad, 0x65, 0xff)));    // #ad65ff
}

TEST_P(StemFixture, FetchStemEmptyInfo) {
    TrackPointer pTrack(Track::newTemporary(
            getTestDir().filePath("stems/test_missing_stem_details.stem.mp4")));

    mixxx::AudioSource::OpenParams config;
    config.setChannelCount(mixxx::audio::ChannelCount(2));

    ASSERT_NE(SoundSourceProxy(pTrack).openAudioSource(config), nullptr);

    auto stemInfo = pTrack->getStemInfo();
    ASSERT_EQ(stemInfo.size(), 4);
    ASSERT_EQ(stemInfo.at(0), StemInfo("Stem #1", QColor(0x00, 0x9E, 0x73)));
    ASSERT_EQ(stemInfo.at(1), StemInfo("Stem #2", QColor(0xD5, 0x5E, 0x00)));
    ASSERT_EQ(stemInfo.at(2), StemInfo("Stem #3", QColor(0xCC, 0x79, 0xA7)));
    ASSERT_EQ(stemInfo.at(3), StemInfo("Stem #4", QColor(0x56, 0xB4, 0xE9)));
}

TEST_P(StemFixture, ReadMainMix) {
    SoundSourceFFmpeg sourceMainMix(
            QUrl::fromLocalFile(getTestDir().filePath("stems/mainmix.wav")));
    SoundSourceSTEM sourceStem(QUrl::fromLocalFile(getTestDir().filePath(STEM_FILE)));

    mixxx::AudioSource::OpenParams config;
    config.setChannelCount(mixxx::audio::ChannelCount(2));

    ASSERT_EQ(sourceMainMix.open(AudioSource::OpenMode::Strict, config),
            AudioSource::OpenResult::Succeeded);
    ASSERT_EQ(sourceStem.open(AudioSource::OpenMode::Strict, config),
            AudioSource::OpenResult::Succeeded);

    ASSERT_EQ(sourceMainMix.getSignalInfo(), sourceStem.getSignalInfo());

    SampleBuffer buffer1(1024), buffer2(1024);
    ASSERT_EQ(sourceMainMix.readSampleFrames(WritableSampleFrames(
                                                     IndexRange::between(
                                                             0,
                                                             512),
                                                     SampleBuffer::WritableSlice(
                                                             buffer1.data(),
                                                             buffer1.size())))
                      .readableLength(),
            buffer1.size());
    ASSERT_EQ(sourceStem.readSampleFrames(WritableSampleFrames(
                                                  IndexRange::between(
                                                          0,
                                                          512),
                                                  SampleBuffer::WritableSlice(
                                                          buffer2.data(),
                                                          buffer2.size())))
                      .readableLength(),
            buffer2.size());
    EXPECT_TRUE(0 == std::memcmp(buffer1.data(), buffer1.data(), sizeof(buffer1)));
}

TEST_P(StemFixture, ReadEachStem) {
    int stemIdx = 0;
    for (auto& stem : kStemFiles) {
        SoundSourceFFmpeg sourceStandaloneStem(
                QUrl::fromLocalFile(getTestDir().filePath("stems/" + stem)));
        SoundSourceSingleSTEM sourceStem(
                QUrl::fromLocalFile(
                        getTestDir().filePath(STEM_FILE)),
                stemIdx++);

        mixxx::AudioSource::OpenParams config;
        config.setChannelCount(mixxx::audio::ChannelCount(2));

        ASSERT_EQ(sourceStandaloneStem.open(AudioSource::OpenMode::Strict, config),
                AudioSource::OpenResult::Succeeded);
        ASSERT_EQ(sourceStem.open(AudioSource::OpenMode::Strict, config),
                AudioSource::OpenResult::Succeeded);

        ASSERT_EQ(sourceStandaloneStem.getSignalInfo(), sourceStem.getSignalInfo());

        SampleBuffer buffer1(1024), buffer2(1024);
        ASSERT_EQ(sourceStandaloneStem.readSampleFrames(WritableSampleFrames(
                                                                IndexRange::between(
                                                                        0,
                                                                        512),
                                                                SampleBuffer::WritableSlice(
                                                                        buffer1.data(),
                                                                        buffer1.size())))
                          .readableLength(),
                buffer1.size());
        ASSERT_EQ(sourceStem.readSampleFrames(WritableSampleFrames(
                                                      IndexRange::between(
                                                              0,
                                                              512),
                                                      SampleBuffer::WritableSlice(
                                                              buffer2.data(),
                                                              buffer2.size())))
                          .readableLength(),
                buffer2.size());
        EXPECT_TRUE(0 == std::memcmp(buffer1.data(), buffer1.data(), sizeof(buffer1)));
    }
}

TEST_P(StemFixture, OpenStem) {
    SoundSourceSTEM sourceStem(QUrl::fromLocalFile(getTestDir().filePath(STEM_FILE)));

    mixxx::AudioSource::OpenParams config;
    config.setChannelCount(mixxx::audio::ChannelCount(8));
    ASSERT_EQ(sourceStem.open(AudioSource::OpenMode::Strict, config),
            AudioSource::OpenResult::Succeeded);

    ASSERT_EQ(mixxx::audio::SignalInfo(mixxx::audio::ChannelCount::stem(),
                      mixxx::audio::SampleRate(44100)),
            sourceStem.getSignalInfo());
}

INSTANTIATE_TEST_SUITE_P(
        StemTest,
        StemFixture,
        ::testing::ValuesIn(supportedCodecs),
        [](const testing::TestParamInfo<StemFixture::ParamType>& info) {
            return info.param;
        });

} // namespace
