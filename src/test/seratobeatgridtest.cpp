#include <gtest/gtest.h>
#include <taglib/textidentificationframe.h>
#include <taglib/tstring.h>

#include <QDir>
#include <QtDebug>

#include "track/serato/beatgrid.h"
#include "util/memory.h"

namespace {

class SeratoBeatGridTest : public testing::Test {
  protected:
    void parseBeatGridData(const QByteArray& inputValue,
            bool valid,
            mixxx::taglib::FileType fileType) {
        mixxx::SeratoBeatGrid seratoBeatGrid;
        bool parseOk = mixxx::SeratoBeatGrid::parse(
                &seratoBeatGrid, inputValue, fileType);
        EXPECT_EQ(valid, parseOk);
        if (!parseOk) {
            return;
        }

        EXPECT_EQ(inputValue, seratoBeatGrid.dump(fileType));
    }

    void parseBeatGridDataInDirectory(
            QDir dir, mixxx::taglib::FileType fileType) {
        EXPECT_TRUE(dir.exists());
        dir.setFilter(QDir::Files);
        dir.setNameFilters(QStringList() << "*.octet-stream");

        QFileInfoList list = dir.entryInfoList();
        EXPECT_NE(list.size(), 0);
        for (int i = 0; i < list.size(); i++) {
            QFileInfo fileInfo = list.at(i);
            qDebug() << "--- File:" << fileInfo.fileName();
            QFile file(dir.filePath(fileInfo.fileName()));
            bool openOk = file.open(QIODevice::ReadOnly);
            EXPECT_TRUE(openOk);
            if (!openOk) {
                continue;
            }
            QByteArray data = file.readAll();
            parseBeatGridData(data, true, fileType);
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

TEST_F(SeratoBeatGridTest, ParseBeatGridDataMp3) {
    parseBeatGridDataInDirectory(QDir("src/test/serato/data/mp3/beatgrid"),
            mixxx::taglib::FileType::MP3);
}

TEST_F(SeratoBeatGridTest, ParseEmptyDataMP3) {
    parseEmptyBeatGridData(mixxx::taglib::FileType::MP3);
}

} // namespace
