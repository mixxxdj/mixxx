#include <gtest/gtest.h>
#include <taglib/textidentificationframe.h>
#include <taglib/tstring.h>

#include <QDir>
#include <QtDebug>

#include "track/beatgrid.h"
#include "track/beatmap.h"
#include "track/serato/beatgrid.h"
#include "util/memory.h"

namespace {

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
    constexpr double bpm = 100.0;
    const auto sampleRate = mixxx::audio::SampleRate(44100);
    const auto signalInfo = mixxx::audio::SignalInfo(mixxx::audio::ChannelCount(2), sampleRate);
    const auto duration = mixxx::Duration::fromSeconds<int>(300);
    const double framesPerMinute = signalInfo.getSampleRate() * 60;
    const double framesPerBeat = framesPerMinute / bpm;

    QVector<double> beatPositionsFrames;
    double beatPositionFrames = 0;
    // Add 2 minutes of beats at 100 bpm to the beatgrid
    for (int i = 0; i < 2 * bpm; i++) {
        beatPositionsFrames.append(beatPositionFrames);
        beatPositionFrames += framesPerBeat;
    }

    // Check the const beatmap
    {
        const auto pBeats = mixxx::BeatMap::makeBeatMap(
                sampleRate, QString("Test"), beatPositionsFrames);
        // At the 1 minute mark the BPM should be 100
        EXPECT_EQ(pBeats->getBpmAroundPosition(framesPerMinute, 1), bpm);

        mixxx::SeratoBeatGrid seratoBeatGrid;
        seratoBeatGrid.setBeats(pBeats, signalInfo, duration, 0);
        EXPECT_EQ(seratoBeatGrid.nonTerminalMarkers().size(), 0);
        EXPECT_NE(seratoBeatGrid.terminalMarker(), nullptr);
        EXPECT_FLOAT_EQ(seratoBeatGrid.terminalMarker()->bpm(), static_cast<float>(bpm));
    }

    // Now add 3 minutes of beats at 50 bpm to the beatgrid
    for (int i = 0; i < 3 * bpm / 2; i++) {
        beatPositionsFrames.append(beatPositionFrames);
        beatPositionFrames += framesPerBeat * 2;
    }

    // Check the non-const beatmap
    {
        const auto pBeats = mixxx::BeatMap::makeBeatMap(
                sampleRate, QString("Test"), beatPositionsFrames);
        // At the 1 minute mark the BPM should be 100
        EXPECT_EQ(pBeats->getBpmAroundPosition(framesPerMinute, 1), bpm);
        // At the 4 minute mark the BPM should be 50
        EXPECT_EQ(pBeats->getBpmAroundPosition(framesPerMinute * 4, 1), bpm / 2);

        mixxx::SeratoBeatGrid seratoBeatGrid;
        seratoBeatGrid.setBeats(pBeats, signalInfo, duration, 0);
        EXPECT_EQ(seratoBeatGrid.nonTerminalMarkers().size(), 2);
        EXPECT_NE(seratoBeatGrid.terminalMarker(), nullptr);
        EXPECT_FLOAT_EQ(seratoBeatGrid.terminalMarker()->bpm(), static_cast<float>(bpm / 2));
    }
}

} // namespace
