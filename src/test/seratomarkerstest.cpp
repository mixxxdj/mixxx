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
    void parseEntry(const QByteArray inputValue, bool valid, bool isSet, quint32 startPosition, quint32 endPosition, QRgb color, bool isLocked, quint8 type) {
        const mixxx::SeratoMarkersEntry* pEntry = mixxx::SeratoMarkersEntry::parse(inputValue);
        if (!pEntry) {
            EXPECT_FALSE(valid);
            return;
        }
        EXPECT_TRUE(valid);

        EXPECT_EQ(isSet, pEntry->isSet());
        EXPECT_EQ(startPosition, pEntry->getStartPosition());
        EXPECT_EQ(endPosition, pEntry->getEndPosition());
        EXPECT_EQ(color, pEntry->getColor());
        EXPECT_EQ(isLocked, pEntry->isLocked());
        EXPECT_EQ(type, pEntry->type());

        EXPECT_EQ(inputValue, pEntry->data());
    }

    void parseMarkersData(const QByteArray inputValue, bool valid) {
        mixxx::SeratoMarkers seratoMarkers;
        bool parseOk = mixxx::SeratoMarkers::parse(&seratoMarkers, inputValue);
        EXPECT_EQ(valid, parseOk);
        if (!parseOk) {
            return;
        }

        EXPECT_EQ(inputValue, seratoMarkers.data());
    }
};

TEST_F(SeratoMarkersTest, ParseEntry) {
    parseEntry(QByteArray("\x00\x00\x00\x00\x00\x7f\x7f\x7f\x7f\x7f\x00\x7f\x7f\x7f\x7f\x7f\x06\x30\x00\x00\x01", 21), false, false, -1, -1, QRgb(0x000000), false, 0);
    parseEntry(QByteArray("\x00\x00\x00\x00\x00\x7f\x7f\x7f\x7f\x7f\x00\x7f\x7f\x7f\x7f\x7f\x06\x30\x00\x00\x01\x00", 22), true, true, 0, -1, QRgb(0xcc0000), false, 1);
    parseEntry(QByteArray("\x00\x00\x0d\x2a\x58\x7f\x7f\x7f\x7f\x7f\x00\x7f\x7f\x7f\x7f\x7f\x06\x32\x10\x00\x01\x00", 22), true, true, 862808, -1, QRgb(0xcc8800), false, 1);
    parseEntry(QByteArray("\x00\x00\x03\x54\x64\x7f\x7f\x7f\x7f\x7f\x00\x7f\x7f\x7f\x7f\x7f\x00\x00\x01\x4c\x01\x00", 22), true, true, 218212, -1, QRgb(0x0000cc), false, 1);
    parseEntry(QByteArray("\x00\x00\x00\x00\x6c\x7f\x7f\x7f\x7f\x7f\x00\x7f\x7f\x7f\x7f\x7f\x06\x33\x18\x00\x01\x00", 22), true, true, 108, -1, QRgb(0xcccc00), false, 1);
    parseEntry(QByteArray("\x00\x00\x00\x07\x77\x7f\x7f\x7f\x7f\x7f\x00\x7f\x7f\x7f\x7f\x7f\x00\x03\x18\x00\x01\x00", 22), true, true, 1911, -1, QRgb(0x00cc00), false, 1);
    parseEntry(QByteArray("\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x00\x7f\x7f\x7f\x7f\x7f\x00\x00\x00\x00\x01\x00", 22), true, false, -1, -1, QRgb(0x000000), false, 1);
    parseEntry(QByteArray("\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x00\x7f\x7f\x7f\x7f\x7f\x00\x00\x00\x00\x03\x00", 22), true, false, -1, -1, QRgb(0x000000), false, 3);
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

} // namespace
