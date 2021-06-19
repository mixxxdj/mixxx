#include <gtest/gtest.h>
#include <taglib/textidentificationframe.h>
#include <taglib/tstring.h>

#include <QDir>
#include <QtDebug>

#include "track/beatgrid.h"
#include "track/beatmap.h"
#include "track/serato/beatgrid.h"
#include "track/serato/beatsimporter.h"
#include "util/memory.h"

namespace {

// Maximum allowed frame position inaccuracy after reimport
constexpr double kEpsilon = 0.1;

} // namespace

namespace mixxx {

class SeratoBeatGridTest : public testing::Test {
  protected:
    bool parseBeatGridData(const QByteArray& inputValue,
            bool valid,
            mixxx::taglib::FileType fileType,
            QByteArray* output = nullptr) {
        mixxx::SeratoBeatGrid seratoBeatGrid;
        bool parseOk = mixxx::SeratoBeatGrid::parse(
                &seratoBeatGrid, inputValue, fileType);
        EXPECT_EQ(valid, parseOk);
        if (!parseOk) {
            return false;
        }
        QByteArray outputValue = seratoBeatGrid.dump(fileType);
        EXPECT_EQ(inputValue, outputValue);
        if (inputValue != outputValue) {
            if (output) {
                output->clear();
                output->append(outputValue);
            }
            return false;
        }
        return true;
    }

    void parseBeatGridDataInDirectory(
            QDir dir, mixxx::taglib::FileType fileType) {
        EXPECT_TRUE(dir.exists());
        dir.setFilter(QDir::Files);
        dir.setNameFilters(QStringList() << "*.octet-stream");

        QFileInfoList fileList = dir.entryInfoList();
        EXPECT_FALSE(fileList.isEmpty());
        for (const QFileInfo& fileInfo : fileList) {
            qDebug() << "--- File:" << fileInfo.fileName();
            QFile file(dir.filePath(fileInfo.fileName()));
            bool openOk = file.open(QIODevice::ReadOnly);
            EXPECT_TRUE(openOk);
            if (!openOk) {
                continue;
            }
            QByteArray data = file.readAll();
            QByteArray actualData;
            if (!parseBeatGridData(data, true, fileType, &actualData)) {
                QFile outfile(file.fileName() + QStringLiteral(".actual"));
                openOk = outfile.open(QIODevice::WriteOnly);
                EXPECT_TRUE(openOk);
                if (!openOk) {
                    continue;
                }
                EXPECT_EQ(actualData.size(), outfile.write(actualData));
            }
        }
    }

    void parseEmptyBeatGridData(mixxx::taglib::FileType fileType) {
        QByteArray inputValue;
        mixxx::SeratoBeatGrid seratoBeatGrid;
        mixxx::SeratoBeatGrid::parse(&seratoBeatGrid, inputValue, fileType);
        QByteArray outputValue = seratoBeatGrid.dump(fileType);
        EXPECT_EQ(inputValue, outputValue);
    }
};

TEST_F(SeratoBeatGridTest, ParseBeatGridDataMP3) {
    parseBeatGridDataInDirectory(QDir("src/test/serato/data/mp3/beatgrid"),
            mixxx::taglib::FileType::MP3);
}

TEST_F(SeratoBeatGridTest, ParseEmptyDataMP3) {
    parseEmptyBeatGridData(mixxx::taglib::FileType::MP3);
}

TEST_F(SeratoBeatGridTest, ParseBeatGridDataMP4) {
    parseBeatGridDataInDirectory(QDir("src/test/serato/data/mp4/beatgrid"),
            mixxx::taglib::FileType::MP4);
}

TEST_F(SeratoBeatGridTest, ParseEmptyDataMP4) {
    parseEmptyBeatGridData(mixxx::taglib::FileType::MP4);
}

TEST_F(SeratoBeatGridTest, ParseBeatGridDataFLAC) {
    parseBeatGridDataInDirectory(QDir("src/test/serato/data/flac/beatgrid"),
            mixxx::taglib::FileType::FLAC);
}

TEST_F(SeratoBeatGridTest, ParseEmptyDataFLAC) {
    parseEmptyBeatGridData(mixxx::taglib::FileType::FLAC);
}

TEST_F(SeratoBeatGridTest, SerializeBeatgrid) {
    // Create a const beatgrid at 120 BPM
    constexpr double bpm = 120.0;
    const auto sampleRate = mixxx::audio::SampleRate(44100);
    EXPECT_EQ(sampleRate.isValid(), true);
    const auto pBeats = mixxx::BeatGrid::makeBeatGrid(sampleRate, QString("Test"), bpm, 0);
    const auto signalInfo = mixxx::audio::SignalInfo(mixxx::audio::ChannelCount(2), sampleRate);
    const auto duration = mixxx::Duration::fromSeconds<int>(300);

    // Serialize that beatgrid into Serato BeatGrid data and check if it's correct
    mixxx::SeratoBeatGrid seratoBeatGrid;
    seratoBeatGrid.setBeats(pBeats, signalInfo, duration, 0);
    EXPECT_EQ(seratoBeatGrid.nonTerminalMarkers().size(), 0);
    EXPECT_NE(seratoBeatGrid.terminalMarker(), nullptr);
    EXPECT_FLOAT_EQ(seratoBeatGrid.terminalMarker()->bpm(), static_cast<float>(bpm));
}

TEST_F(SeratoBeatGridTest, SerializeBeatMap) {
    // Create a non-const beatmap
    constexpr double timingOffsetMillis = -10;
    constexpr int bpm = 120;
    const auto sampleRate = mixxx::audio::SampleRate(44100);
    const auto signalInfo = mixxx::audio::SignalInfo(mixxx::audio::ChannelCount(2), sampleRate);
    const auto duration = mixxx::Duration::fromSeconds<int>(300);
    const double framesPerMinute = signalInfo.getSampleRate() * 60;
    const double framesPerBeat = framesPerMinute / bpm;
    const double initialFrameOffset = framesPerBeat / 2;

    QVector<double> beatPositionsFrames;
    double beatPositionFrames = initialFrameOffset;

    constexpr int kNumBeats120BPM = 4;
    qInfo() << "Step 1: Add" << kNumBeats120BPM << "beats at 100 bpm to the beatgrid";
    for (int i = 0; i < kNumBeats120BPM; i++) {
        beatPositionsFrames.append(beatPositionFrames);
        beatPositionFrames += framesPerBeat;
    }
    ASSERT_EQ(beatPositionsFrames.size(), kNumBeats120BPM);

    // Check the const beatmap
    {
        const auto pBeats = mixxx::BeatMap::makeBeatMap(
                sampleRate, QString("Test"), beatPositionsFrames);
        // Check that the first section's BPM is 100
        EXPECT_EQ(pBeats->getBpmAroundPosition(
                          signalInfo.frames2samples(
                                  static_cast<SINT>(initialFrameOffset +
                                          framesPerBeat * kNumBeats120BPM / 2)),
                          1),
                bpm);

        mixxx::SeratoBeatGrid seratoBeatGrid;
        seratoBeatGrid.setBeats(pBeats, signalInfo, duration, timingOffsetMillis);
        EXPECT_EQ(seratoBeatGrid.nonTerminalMarkers().size(), 0);
        EXPECT_NE(seratoBeatGrid.terminalMarker(), nullptr);
        EXPECT_FLOAT_EQ(seratoBeatGrid.terminalMarker()->bpm(), static_cast<float>(bpm));

        // Check if the beats can be re-imported losslessly
        mixxx::SeratoBeatsImporter beatsImporter(
                seratoBeatGrid.nonTerminalMarkers(),
                seratoBeatGrid.terminalMarker());
        QVector<double> importedBeatPositionsFrames =
                beatsImporter.importBeatsAndApplyTimingOffset(timingOffsetMillis, signalInfo);
        ASSERT_EQ(beatPositionsFrames.size(), importedBeatPositionsFrames.size());
        for (int i = 0; i < beatPositionsFrames.size(); i++) {
            EXPECT_NEAR(beatPositionsFrames[i], importedBeatPositionsFrames[i], kEpsilon);
        }
    }

    constexpr int kNumBeats60BPM = 4;
    qInfo() << "Step 2: Add" << kNumBeats60BPM << "beats at 50 bpm to the beatgrid";
    for (int i = 0; i < kNumBeats60BPM; i++) {
        beatPositionsFrames.append(beatPositionFrames);
        beatPositionFrames += framesPerBeat * 2;
    }
    ASSERT_EQ(beatPositionsFrames.size(), kNumBeats120BPM + kNumBeats60BPM);

    {
        const auto pBeats = mixxx::BeatMap::makeBeatMap(
                sampleRate, QString("Test"), beatPositionsFrames);
        // Check that the first section'd BPM is 100
        EXPECT_EQ(pBeats->getBpmAroundPosition(
                          signalInfo.frames2samples(
                                  static_cast<SINT>(initialFrameOffset +
                                          framesPerBeat * kNumBeats120BPM / 2)),
                          1),
                bpm);
        // Check that the second section'd BPM is 50
        EXPECT_EQ(pBeats->getBpmAroundPosition(
                          signalInfo.frames2samples(
                                  static_cast<SINT>(initialFrameOffset +
                                          framesPerBeat * kNumBeats120BPM +
                                          framesPerBeat * kNumBeats60BPM / 2)),
                          1),
                bpm / 2);

        mixxx::SeratoBeatGrid seratoBeatGrid;
        seratoBeatGrid.setBeats(pBeats, signalInfo, duration, timingOffsetMillis);
        ASSERT_EQ(seratoBeatGrid.nonTerminalMarkers().size(), 2);
        ASSERT_EQ(seratoBeatGrid.nonTerminalMarkers()[0]->beatsTillNextMarker(), kNumBeats120BPM);
        ASSERT_EQ(seratoBeatGrid.nonTerminalMarkers()[1]->beatsTillNextMarker(),
                kNumBeats60BPM - 1);
        EXPECT_NE(seratoBeatGrid.terminalMarker(), nullptr);
        EXPECT_FLOAT_EQ(seratoBeatGrid.terminalMarker()->bpm(), static_cast<float>(bpm / 2));

        // Check if the beats can be re-imported losslessly
        mixxx::SeratoBeatsImporter beatsImporter(
                seratoBeatGrid.nonTerminalMarkers(),
                seratoBeatGrid.terminalMarker());
        QVector<double> importedBeatPositionsFrames =
                beatsImporter.importBeatsAndApplyTimingOffset(timingOffsetMillis, signalInfo);
        ASSERT_EQ(beatPositionsFrames.size(), importedBeatPositionsFrames.size());
        for (int i = 0; i < beatPositionsFrames.size(); i++) {
            EXPECT_NEAR(beatPositionsFrames[i], importedBeatPositionsFrames[i], kEpsilon);
        }
    }

    qInfo() << "Step 3: Add" << kNumBeats120BPM << "beats at 100 bpm to the beatgrid";
    for (int i = 0; i < kNumBeats120BPM; i++) {
        beatPositionsFrames.append(beatPositionFrames);
        beatPositionFrames += framesPerBeat;
    }
    ASSERT_EQ(beatPositionsFrames.size(), 2 * kNumBeats120BPM + kNumBeats60BPM);

    // Add the last beat
    beatPositionsFrames.append(beatPositionFrames);

    {
        const auto pBeats = mixxx::BeatMap::makeBeatMap(
                sampleRate, QString("Test"), beatPositionsFrames);
        // Check that the first section's BPM is 100
        EXPECT_EQ(pBeats->getBpmAroundPosition(
                          signalInfo.frames2samples(
                                  static_cast<SINT>(initialFrameOffset +
                                          framesPerBeat * kNumBeats120BPM / 2)),
                          1),
                bpm);
        // Check that the second section's BPM is 50
        EXPECT_EQ(pBeats->getBpmAroundPosition(
                          signalInfo.frames2samples(
                                  static_cast<SINT>(initialFrameOffset +
                                          framesPerBeat * kNumBeats120BPM +
                                          framesPerBeat * kNumBeats60BPM / 2)),
                          1),
                bpm / 2);
        // Check that the third section's BPM is 100
        EXPECT_EQ(pBeats->getBpmAroundPosition(
                          signalInfo.frames2samples(static_cast<SINT>(
                                  initialFrameOffset +
                                  framesPerBeat * kNumBeats120BPM * 1.5 +
                                  framesPerBeat * kNumBeats60BPM)),
                          1),
                bpm / 2);

        mixxx::SeratoBeatGrid seratoBeatGrid;
        seratoBeatGrid.setBeats(pBeats, signalInfo, duration, timingOffsetMillis);
        ASSERT_EQ(seratoBeatGrid.nonTerminalMarkers().size(), 3);
        ASSERT_EQ(seratoBeatGrid.nonTerminalMarkers()[0]->beatsTillNextMarker(), kNumBeats120BPM);
        ASSERT_EQ(seratoBeatGrid.nonTerminalMarkers()[1]->beatsTillNextMarker(), kNumBeats60BPM);
        ASSERT_EQ(seratoBeatGrid.nonTerminalMarkers()[2]->beatsTillNextMarker(), kNumBeats60BPM);
        EXPECT_NE(seratoBeatGrid.terminalMarker(), nullptr);
        EXPECT_FLOAT_EQ(seratoBeatGrid.terminalMarker()->bpm(), static_cast<float>(bpm));

        // Check if the beats can be re-imported losslessly
        mixxx::SeratoBeatsImporter beatsImporter(
                seratoBeatGrid.nonTerminalMarkers(),
                seratoBeatGrid.terminalMarker());
        QVector<double> importedBeatPositionsFrames =
                beatsImporter.importBeatsAndApplyTimingOffset(timingOffsetMillis, signalInfo);
        ASSERT_EQ(beatPositionsFrames.size(), importedBeatPositionsFrames.size());
        for (int i = 0; i < beatPositionsFrames.size(); i++) {
            EXPECT_NEAR(beatPositionsFrames[i], importedBeatPositionsFrames[i], kEpsilon);
        }
    }
}

} // namespace mixxx
