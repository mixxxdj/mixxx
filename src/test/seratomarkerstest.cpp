#include <gtest/gtest.h>
#include <taglib/textidentificationframe.h>
#include <taglib/tstring.h>

#include <QDir>
#include <QtDebug>

#include "track/serato/markers.h"
#include "util/memory.h"

namespace {

class SeratoMarkersTest : public testing::Test {
  protected:
    void parseEntry(
            const QByteArray& inputValue,
            bool valid,
            bool hasStartPosition,
            quint32 startPosition,
            bool hasEndPosition,
            quint32 endPosition,
            mixxx::RgbColor color,
            mixxx::SeratoMarkersEntry::TypeId typeId,
            bool isLocked) {
        const mixxx::SeratoMarkersEntryPointer pEntry = mixxx::SeratoMarkersEntry::parse(inputValue);
        if (!pEntry) {
            EXPECT_FALSE(valid);
            return;
        }
        EXPECT_TRUE(valid);

        EXPECT_EQ(hasStartPosition, pEntry->hasStartPosition());
        EXPECT_EQ(startPosition, pEntry->getStartPosition());
        EXPECT_EQ(hasEndPosition, pEntry->hasEndPosition());
        EXPECT_EQ(endPosition, pEntry->getEndPosition());
        EXPECT_EQ(color, pEntry->getColor());
        EXPECT_EQ(typeId, pEntry->typeId());
        EXPECT_EQ(isLocked, pEntry->isLocked());

        EXPECT_EQ(inputValue, pEntry->dump());
    }

    void parseMarkersData(const QByteArray& inputValue, bool valid) {
        mixxx::SeratoMarkers seratoMarkers;
        bool parseOk = mixxx::SeratoMarkers::parse(&seratoMarkers, inputValue);
        EXPECT_EQ(valid, parseOk);
        if (!parseOk) {
            return;
        }

        EXPECT_EQ(inputValue, seratoMarkers.dump());
    }
};

TEST_F(SeratoMarkersTest, ParseEntry) {
    parseEntry(
            QByteArray("\x00\x00\x00\x00\x00\x7f\x7f\x7f\x7f\x7f\x00\x7f\x7f\x7f\x7f\x7f\x06\x30\x00\x00\x01", 21),
            false,
            false,
            0x7f7f7f7f,
            false,
            0x7f7f7f7f,
            mixxx::RgbColor(0x000000),
            mixxx::SeratoMarkersEntry::TypeId::Unknown,
            false);
    parseEntry(
            QByteArray("\x00\x00\x00\x00\x00\x7f\x7f\x7f\x7f\x7f\x00\x7f\x7f\x7f\x7f\x7f\x06\x30\x00\x00\x01\x00", 22),
            true,
            true,
            0,
            false,
            0x7f7f7f7f,
            mixxx::RgbColor(0xcc0000),
            mixxx::SeratoMarkersEntry::TypeId::Cue,
            false);
    parseEntry(
            QByteArray("\x00\x00\x0d\x2a\x58\x7f\x7f\x7f\x7f\x7f\x00\x7f\x7f\x7f\x7f\x7f\x06\x32\x10\x00\x01\x00", 22),
            true,
            true,
            218456,
            false,
            0x7f7f7f7f,
            mixxx::RgbColor(0xcc8800),
            mixxx::SeratoMarkersEntry::TypeId::Cue,
            false);
    parseEntry(
            QByteArray("\x00\x00\x03\x54\x64\x7f\x7f\x7f\x7f\x7f\x00\x7f\x7f\x7f\x7f\x7f\x00\x00\x01\x4c\x01\x00", 22),
            true,
            true,
            60004,
            false,
            0x7f7f7f7f,
            mixxx::RgbColor(0x0000cc),
            mixxx::SeratoMarkersEntry::TypeId::Cue,
            false);
    parseEntry(
            QByteArray("\x00\x00\x00\x00\x6c\x7f\x7f\x7f\x7f\x7f\x00\x7f\x7f\x7f\x7f\x7f\x06\x33\x18\x00\x01\x00", 22),
            true,
            true,
            108,
            false,
            0x7f7f7f7f,
            mixxx::RgbColor(0xcccc00),
            mixxx::SeratoMarkersEntry::TypeId::Cue,
            false);
    parseEntry(
            QByteArray("\x00\x00\x00\x07\x77\x7f\x7f\x7f\x7f\x7f\x00\x7f\x7f\x7f\x7f\x7f\x00\x03\x18\x00\x01\x00", 22),
            true,
            true,
            1015,
            false,
            0x7f7f7f7f,
            mixxx::RgbColor(0x00cc00),
            mixxx::SeratoMarkersEntry::TypeId::Cue,
            false);
    parseEntry(
            QByteArray("\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x00\x7f\x7f\x7f\x7f\x7f\x00\x00\x00\x00\x01\x00", 22),
            true,
            false,
            0x7f7f7f7f,
            false,
            0x7f7f7f7f,
            mixxx::RgbColor(0x000000),
            mixxx::SeratoMarkersEntry::TypeId::Cue,
            false);
    parseEntry(
            QByteArray("\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x00\x7f\x7f\x7f\x7f\x7f\x00\x00\x00\x00\x03\x00", 22),
            true,
            false,
            0x7f7f7f7f,
            false,
            0x7f7f7f7f,
            mixxx::RgbColor(0x000000),
            mixxx::SeratoMarkersEntry::TypeId::Loop,
            false);
}

TEST_F(SeratoMarkersTest, ParseMarkersData) {
    QDir dir("src/test/serato/data/markers_");
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << "*.octet-stream");

    QFileInfoList list = dir.entryInfoList();
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
        parseMarkersData(data, true);
    }
}

TEST_F(SeratoMarkersTest, ParseEmptyData) {
    QByteArray inputValue;
    mixxx::SeratoMarkers seratoMarkers;
    mixxx::SeratoMarkers::parse(&seratoMarkers, inputValue);
    QByteArray outputValue = seratoMarkers.dump();
    EXPECT_EQ(inputValue, outputValue);
}

} // namespace
