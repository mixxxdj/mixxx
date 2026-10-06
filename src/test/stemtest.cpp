#include <gtest/gtest.h>

#include <QtDebug>
#include <cmath>

#include "sources/soundsourceffmpeg.h"
#include "sources/soundsourceproxy.cpp"
#include "test/mixxxtest.h"
#include "track/steminfoimporter.h"
#include "track/track.h"
#include "util/sample.h"
#include "util/samplebuffer.h"

using namespace mixxx;

#define STEM_FILE QStringLiteral("stems/sin_%1.stem.mp4").arg(QString::fromStdString(GetParam()))

namespace {

const std::vector<std::string> supportedCodecs = {
        "AAC_256kbps_VBR",
        "ALAC_24bit"};

const QList<QString> kStemFiles = {
        "01-drum.wav",
        "02-bass.wav",
        "03-melody.wav",
        "04-vocal.wav",
};

// Fixed stem files carrying their own manifest with distinct names and
// colors, covering both the .stem.mp4 and the .stem.m4a file types.
const QList<QString> kSeparatedStemFiles = {
        "stems/test_separated.stem.mp4",
        "stems/test_separated_alac.stem.m4a",
        "stems/sin_separated_alac.stem.m4a",
};

// Number of stems in a STEM file (the premixed master stream is not counted)
constexpr int kStemCount = 4;

// Number of frames read per buffer in the read tests below
constexpr SINT kReadFrameCount = 512;

// Number of frames compared in the lossless content test below
constexpr SINT kContentFrameCount = 44100;

class StemFixture : public MixxxTest, public ::testing::WithParamInterface<std::string> {
  protected:
    void SetUp() override {
        ASSERT_TRUE(SoundSourceProxy::isFileTypeSupported("stem.mp4") ||
                SoundSourceProxy::registerProviders());
    }
};

class SeparatedStemFixture : public MixxxTest {
  protected:
    void SetUp() override {
        ASSERT_TRUE((SoundSourceProxy::isFileTypeSupported("stem.mp4") &&
                            SoundSourceProxy::isFileTypeSupported("stem.m4a")) ||
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
    const QUrl stemUrl = QUrl::fromLocalFile(getTestDir().filePath(STEM_FILE));

    // The stereo mode of a STEM file is the on-the-fly mix of all four stems
    SoundSourceSTEM sourceStem(stemUrl);

    mixxx::AudioSource::OpenParams config;
    config.setChannelCount(mixxx::audio::ChannelCount(2));

    ASSERT_EQ(sourceStem.open(AudioSource::OpenMode::Strict, config),
            AudioSource::OpenResult::Succeeded);

    SampleBuffer mixdown(kReadFrameCount * 2);
    ASSERT_EQ(sourceStem.readSampleFrames(WritableSampleFrames(
                                                  IndexRange::between(0, kReadFrameCount),
                                                  SampleBuffer::WritableSlice(mixdown.data(), mixdown.size())))
                      .readableLength(),
            mixdown.size());

    // Decode every stem stream individually and sum them up the same way the
    // STEM source does, so a broken mixdown is caught and not just a
    // successful open().
    SampleBuffer sum(kReadFrameCount * 2);
    SampleUtil::clear(sum.data(), sum.size());
    SampleBuffer stem(kReadFrameCount * 2);
    for (int stemIdx = 1; stemIdx <= kStemCount; stemIdx++) {
        // Stream 0 holds the premixed master, the stems start at stream 1
        SoundSourceFFmpeg sourceStemStream(stemUrl, stemIdx);
        ASSERT_EQ(sourceStemStream.open(AudioSource::OpenMode::Strict, config),
                AudioSource::OpenResult::Succeeded);
        ASSERT_EQ(sourceStemStream.readSampleFrames(WritableSampleFrames(
                                                            IndexRange::between(0, kReadFrameCount),
                                                            SampleBuffer::WritableSlice(stem.data(), stem.size())))
                          .readableLength(),
                stem.size());
        SampleUtil::add(sum.data(), stem.data(), stem.size());
    }
    for (SINT i = 0; i < mixdown.size(); i++) {
        ASSERT_NEAR(mixdown[i], sum[i], 1e-6f) << "sample index " << i;
    }

    // The reference main mix asset stays covered: it has to open, report the
    // same signal info as the stem file and yield a full buffer. Its audio is
    // an independent mastering of the stems, so it is not compared sample-wise.
    SoundSourceFFmpeg sourceMainMix(
            QUrl::fromLocalFile(getTestDir().filePath("stems/mainmix.wav")));
    ASSERT_EQ(sourceMainMix.open(AudioSource::OpenMode::Strict, config),
            AudioSource::OpenResult::Succeeded);
    ASSERT_EQ(sourceMainMix.getSignalInfo(), sourceStem.getSignalInfo());

    SampleBuffer mainMix(kReadFrameCount * 2);
    ASSERT_EQ(sourceMainMix.readSampleFrames(WritableSampleFrames(
                                                     IndexRange::between(0, kReadFrameCount),
                                                     SampleBuffer::WritableSlice(mainMix.data(), mainMix.size())))
                      .readableLength(),
            mainMix.size());
}

TEST_P(StemFixture, ReadEachStem) {
    const QUrl stemUrl = QUrl::fromLocalFile(getTestDir().filePath(STEM_FILE));

    // In stem mode the source returns all stems interleaved as 1L1R2L2R3L3R4L4R
    SoundSourceSTEM sourceStem(stemUrl);

    mixxx::AudioSource::OpenParams config;
    config.setChannelCount(mixxx::audio::ChannelCount(8));

    ASSERT_EQ(sourceStem.open(AudioSource::OpenMode::Strict, config),
            AudioSource::OpenResult::Succeeded);

    SampleBuffer interleaved(kReadFrameCount * 8);
    ASSERT_EQ(sourceStem.readSampleFrames(WritableSampleFrames(
                                                  IndexRange::between(0, kReadFrameCount),
                                                  SampleBuffer::WritableSlice(interleaved.data(), interleaved.size())))
                      .readableLength(),
            interleaved.size());

    mixxx::AudioSource::OpenParams stereoConfig;
    stereoConfig.setChannelCount(mixxx::audio::ChannelCount(2));

    for (int stemIdx = 0; stemIdx < kStemCount; stemIdx++) {
        SoundSourceFFmpeg sourceStemStream(stemUrl, stemIdx + 1);
        ASSERT_EQ(sourceStemStream.open(AudioSource::OpenMode::Strict, stereoConfig),
                AudioSource::OpenResult::Succeeded);

        SampleBuffer stream(kReadFrameCount * 2);
        ASSERT_EQ(sourceStemStream.readSampleFrames(WritableSampleFrames(
                                                            IndexRange::between(0, kReadFrameCount),
                                                            SampleBuffer::WritableSlice(stream.data(), stream.size())))
                          .readableLength(),
                stream.size());

        CSAMPLE maxDeviation = 0;
        for (SINT frame = 0; frame < kReadFrameCount; frame++) {
            for (int channel = 0; channel < 2; channel++) {
                const CSAMPLE expected = stream[2 * frame + channel];
                const CSAMPLE actual =
                        interleaved[2 * kStemCount * frame + 2 * stemIdx + channel];
                const CSAMPLE deviation = std::fabs(expected - actual);
                if (deviation > maxDeviation) {
                    maxDeviation = deviation;
                }
            }
        }
        EXPECT_NEAR(maxDeviation, 0, 1e-6) << "stem " << stemIdx;
    }

    // The per-stem reference recordings stay covered: they have to open,
    // report the same signal info as the matching stem stream and yield a
    // full buffer. Their audio is an independent recording set for the
    // original fixtures, so it is not compared sample-wise here (the lossless
    // separated fixtures below provide that comparison).
    int referenceIdx = 0;
    for (const auto& stemFile : kStemFiles) {
        SoundSourceFFmpeg sourceReference(
                QUrl::fromLocalFile(getTestDir().filePath("stems/" + stemFile)));
        SoundSourceFFmpeg sourceStemStream(stemUrl, ++referenceIdx);

        ASSERT_EQ(sourceReference.open(AudioSource::OpenMode::Strict, stereoConfig),
                AudioSource::OpenResult::Succeeded);
        ASSERT_EQ(sourceStemStream.open(AudioSource::OpenMode::Strict, stereoConfig),
                AudioSource::OpenResult::Succeeded);
        ASSERT_EQ(sourceReference.getSignalInfo(), sourceStemStream.getSignalInfo());

        SampleBuffer reference(kReadFrameCount * 2);
        SampleBuffer stream(kReadFrameCount * 2);
        ASSERT_EQ(sourceReference.readSampleFrames(WritableSampleFrames(
                                                           IndexRange::between(0, kReadFrameCount),
                                                           SampleBuffer::WritableSlice(reference.data(), reference.size())))
                          .readableLength(),
                reference.size());
        ASSERT_EQ(sourceStemStream.readSampleFrames(WritableSampleFrames(
                                                            IndexRange::between(0, kReadFrameCount),
                                                            SampleBuffer::WritableSlice(stream.data(), stream.size())))
                          .readableLength(),
                stream.size());
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

TEST_F(SeparatedStemFixture, DetectsStemAtom) {
    for (const auto& file : kSeparatedStemFiles) {
        SCOPED_TRACE(file.toStdString());
        const QString path = getTestDir().filePath(file);
        EXPECT_TRUE(StemInfoImporter::hasStemAtom(path));
        EXPECT_TRUE(StemInfoImporter::maybeStemFile(path));
    }

    // Regular audio files must not be mistaken for stem files
    const QString mainMixPath = getTestDir().filePath(QStringLiteral("stems/mainmix.wav"));
    EXPECT_FALSE(StemInfoImporter::hasStemAtom(mainMixPath));
    EXPECT_FALSE(StemInfoImporter::maybeStemFile(mainMixPath));
}

TEST_F(SeparatedStemFixture, FetchSeparatedStemInfo) {
    for (const auto& file : kSeparatedStemFiles) {
        SCOPED_TRACE(file.toStdString());
        TrackPointer pTrack(Track::newTemporary(getTestDir().filePath(file)));

        mixxx::AudioSource::OpenParams config;
        config.setChannelCount(mixxx::audio::ChannelCount(2));

        ASSERT_NE(SoundSourceProxy(pTrack).openAudioSource(config), nullptr);

        auto stemInfo = pTrack->getStemInfo();
        ASSERT_EQ(stemInfo.size(), 4);
        EXPECT_EQ(stemInfo.at(0), StemInfo("Separated Drums", QColor(0xfd, 0x4a, 0x4a)));
        EXPECT_EQ(stemInfo.at(1), StemInfo("Separated Bass", QColor(0xff, 0xff, 0x00)));
        EXPECT_EQ(stemInfo.at(2), StemInfo("Separated Synths", QColor(0x00, 0xe8, 0xe8)));
        EXPECT_EQ(stemInfo.at(3), StemInfo("Separated Vocals", QColor(0xad, 0x65, 0xff)));
    }
}

TEST_F(SeparatedStemFixture, OpenSeparatedStem) {
    for (const auto& file : kSeparatedStemFiles) {
        SCOPED_TRACE(file.toStdString());
        TrackPointer pTrack(Track::newTemporary(getTestDir().filePath(file)));

        mixxx::AudioSource::OpenParams config;
        config.setChannelCount(mixxx::audio::ChannelCount(8));

        auto pSource = SoundSourceProxy(pTrack).openAudioSource(config);
        ASSERT_NE(pSource, nullptr);
        EXPECT_EQ(pSource->getSignalInfo(),
                mixxx::audio::SignalInfo(mixxx::audio::ChannelCount::stem(),
                        mixxx::audio::SampleRate(44100)));
    }
}

TEST_F(SeparatedStemFixture, AlacContentMatchesReferenceWavs) {
    // The ALAC fixtures are lossless copies of the reference recordings:
    // stream 0 is the premixed main mix, streams 1-4 are the individual stems.
    const QList<QString> referenceFiles = {
            "stems/mainmix.wav",
            "stems/01-drum.wav",
            "stems/02-bass.wav",
            "stems/03-melody.wav",
            "stems/04-vocal.wav",
    };
    const QList<QString> alacFiles = {
            "stems/test_separated_alac.stem.m4a",
            "stems/sin_separated_alac.stem.m4a",
    };

    mixxx::AudioSource::OpenParams config;
    config.setChannelCount(mixxx::audio::ChannelCount(2));

    SampleBuffer actual(kContentFrameCount * 2);
    SampleBuffer expected(kContentFrameCount * 2);

    for (const auto& file : alacFiles) {
        SCOPED_TRACE(file.toStdString());
        for (SINT streamIdx = 0; streamIdx < referenceFiles.size(); streamIdx++) {
            SCOPED_TRACE(streamIdx);
            SoundSourceFFmpeg sourceStemStream(
                    QUrl::fromLocalFile(getTestDir().filePath(file)), streamIdx);
            SoundSourceFFmpeg sourceReference(QUrl::fromLocalFile(
                    getTestDir().filePath(referenceFiles.at(streamIdx))));

            ASSERT_EQ(sourceStemStream.open(AudioSource::OpenMode::Strict, config),
                    AudioSource::OpenResult::Succeeded);
            ASSERT_EQ(sourceReference.open(AudioSource::OpenMode::Strict, config),
                    AudioSource::OpenResult::Succeeded);

            ASSERT_EQ(sourceStemStream.readSampleFrames(WritableSampleFrames(
                                                                IndexRange::between(0, kContentFrameCount),
                                                                SampleBuffer::WritableSlice(actual.data(), actual.size())))
                              .readableLength(),
                    actual.size());
            ASSERT_EQ(sourceReference.readSampleFrames(WritableSampleFrames(
                                                               IndexRange::between(0, kContentFrameCount),
                                                               SampleBuffer::WritableSlice(expected.data(), expected.size())))
                              .readableLength(),
                    expected.size());

            for (SINT i = 0; i < actual.size(); i++) {
                ASSERT_EQ(actual[i], expected[i]) << "sample index " << i;
            }
        }
    }
}

} // namespace
