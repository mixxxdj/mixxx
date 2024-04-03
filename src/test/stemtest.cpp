#include <gtest/gtest.h>

#include <QtDebug>

#include "sources/soundsourceproxy.cpp"
#include "test/mixxxtest.h"
#include "track/track.h"
#include "util/samplebuffer.h"

using namespace mixxx;

namespace {

class StemTest : public MixxxTest {
  protected:
    void SetUp() override {
        ASSERT_TRUE(SoundSourceProxy::registerProviders());
    }
};

TEST_F(StemTest, FetchStemInfo) {
    TrackPointer pTrack(Track::newTemporary(getTestDir().filePath("stems/test.stem.mp4")));

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

TEST_F(StemTest, ReadMainMix) {
    SoundSourceFFmpeg sourceMainMix(
            QUrl::fromLocalFile(getTestDir().filePath("stems/mainmix.wav")));
    SoundSourceSTEM sourceStem(QUrl::fromLocalFile(getTestDir().filePath("stems/test.stem.mp4")));

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

} // namespace
