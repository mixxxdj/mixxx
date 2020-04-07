#include <gtest/gtest.h>

#include "track/serato/markers2.h"
#include "util/memory.h"

#include <taglib/tstring.h>
#include <taglib/textidentificationframe.h>
#include <QtDebug>
#include <QDir>

namespace {

class SeratoMarkers2Test : public testing::Test {
  protected:
    void parseBpmlockEntry(const QByteArray inputValue, bool valid, bool locked) {
        const mixxx::SeratoMarkers2EntryPointer parsedEntry = mixxx::SeratoMarkers2BpmlockEntry::parse(inputValue);
        if (!parsedEntry) {
            EXPECT_FALSE(valid);
            return;
        }
        EXPECT_TRUE(valid);
        const mixxx::SeratoMarkers2BpmlockEntry *bpmlockEntry = static_cast<mixxx::SeratoMarkers2BpmlockEntry*>(parsedEntry.get());

        EXPECT_EQ(locked, bpmlockEntry->isLocked());

        EXPECT_EQ(inputValue, bpmlockEntry->dump());
    }

    void parseColorEntry(const QByteArray inputValue, bool valid, mixxx::RgbColor color) {
        const mixxx::SeratoMarkers2EntryPointer parsedEntry = mixxx::SeratoMarkers2ColorEntry::parse(inputValue);
        if (!parsedEntry) {
            EXPECT_FALSE(valid);
            return;
        }
        EXPECT_TRUE(valid);
        const mixxx::SeratoMarkers2ColorEntry *colorEntry = static_cast<mixxx::SeratoMarkers2ColorEntry*>(parsedEntry.get());

        EXPECT_EQ(color, colorEntry->getColor());

        EXPECT_EQ(inputValue, colorEntry->dump());
    }

    void parseCueEntry(
            const QByteArray inputValue,
            bool valid,
            quint8 index,
            quint32 position,
            mixxx::RgbColor color,
            QString label) {
        const mixxx::SeratoMarkers2EntryPointer parsedEntry = mixxx::SeratoMarkers2CueEntry::parse(inputValue);
        if (!parsedEntry) {
            EXPECT_FALSE(valid);
            return;
        }
        EXPECT_TRUE(valid);
        const mixxx::SeratoMarkers2CueEntry *cueEntry = static_cast<mixxx::SeratoMarkers2CueEntry*>(parsedEntry.get());

        EXPECT_EQ(index, cueEntry->getIndex());
        EXPECT_EQ(position, cueEntry->getPosition());
        EXPECT_EQ(color, cueEntry->getColor());
        EXPECT_EQ(label, cueEntry->getLabel());

        EXPECT_EQ(inputValue, cueEntry->dump());
    }

    void parseLoopEntry(const QByteArray inputValue,
            bool valid,
            quint8 index,
            quint32 startposition,
            quint32 endposition,
            bool locked,
            QString label) {
        const mixxx::SeratoMarkers2EntryPointer parsedEntry = mixxx::SeratoMarkers2LoopEntry::parse(inputValue);
        if (!parsedEntry) {
            EXPECT_FALSE(valid);
            return;
        }
        EXPECT_TRUE(valid);
        const mixxx::SeratoMarkers2LoopEntry *loopEntry = static_cast<mixxx::SeratoMarkers2LoopEntry*>(parsedEntry.get());

        EXPECT_EQ(index, loopEntry->getIndex());
        EXPECT_EQ(startposition, loopEntry->getStartPosition());
        EXPECT_EQ(endposition, loopEntry->getEndPosition());
        EXPECT_EQ(locked, loopEntry->isLocked());
        EXPECT_EQ(label, loopEntry->getLabel());

        EXPECT_EQ(inputValue, loopEntry->dump());
    }

    void parseMarkers2Data(const QByteArray inputValue, bool valid) {
        mixxx::SeratoMarkers2 seratoMarkers2;
        bool parseOk = mixxx::SeratoMarkers2::parse(&seratoMarkers2, inputValue);
        EXPECT_EQ(valid, parseOk);
        if (!parseOk) {
            return;
        }
        EXPECT_EQ(inputValue, seratoMarkers2.dump());
    }
};

TEST_F(SeratoMarkers2Test, ParseBpmlockEntry) {
    parseBpmlockEntry(QByteArray("\x00", 1), true, false);
    parseBpmlockEntry(QByteArray("\x01", 1), true, true);
    parseBpmlockEntry(QByteArray("\x00\x00", 2), false, false); // Invalid size
}

TEST_F(SeratoMarkers2Test, ParseColorEntry) {
    parseColorEntry(QByteArray("\x00\xcc\x00\x00", 4), true, mixxx::RgbColor(qRgb(0xcc, 0, 0)));
    parseColorEntry(QByteArray("\x00\x00\xcc\x00", 4), true, mixxx::RgbColor(qRgb(0, 0xcc, 0)));
    parseColorEntry(QByteArray("\x00\x00\x00\xcc", 4), true, mixxx::RgbColor(qRgb(0, 0, 0xcc)));
    parseColorEntry(QByteArray("\x00\x89\xab\xcd", 4), true, mixxx::RgbColor(qRgb(0x89, 0xab, 0xcd)));

    // Invalid value
    parseColorEntry(QByteArray("\x01\xff\x00\x00", 1), false, mixxx::RgbColor(qRgb(0, 0, 0)));

    // Invalid size
    parseColorEntry(QByteArray("\x00", 1), false, mixxx::RgbColor(qRgb(0, 0, 0)));
    parseColorEntry(QByteArray("\x00\xff\x00\x00\x00", 5), false, mixxx::RgbColor(qRgb(0, 0, 0)));
}

TEST_F(SeratoMarkers2Test, ParseCueEntry) {
    parseCueEntry(
            QByteArray("\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 13),
            true,
            0,
            0,
            mixxx::RgbColor(qRgb(0, 0, 0)),
            QString(""));
    parseCueEntry(
            QByteArray("\x00\x01\x00\x00\x10\x00\x00\xcc\x00\x00\x00\x00Test\x00", 17),
            true,
            1,
            4096,
            mixxx::RgbColor(qRgb(0xcc, 0, 0)),
            QString("Test"));
    parseCueEntry(
            QByteArray("\x00\x02\x00\x00\x00\xff\x00\x00\xcc\x00\x00\x00\xc3\xa4\xc3\xbc\xc3\xb6\xc3\x9f\xc3\xa9\xc3\xa8!\x00", 26),
            true,
            2,
            255,
            mixxx::RgbColor(qRgb(0, 0xcc, 0)),
            QString("äüößéè!"));
    parseCueEntry(
            QByteArray("\x00\x03\x02\x03\x04\x05\x00\x06\x07\x08\x00\x00Hello World\x00", 24),
            true,
            3,
            33752069,
            mixxx::RgbColor(qRgb(0x06, 0x07, 0x08)),
            QString("Hello World"));

    // Invalid value
    parseCueEntry(
            QByteArray("\x01\x04\x00\x00\x10\x00\x00\xcc\x00\x00\x00\x00Test\x00", 17),
            false,
            0,
            0,
            mixxx::RgbColor(qRgb(0, 0, 0)),
            QString());
    parseCueEntry(
            QByteArray("\x00\x05\x00\x00\x10\x00\x01\xcc\x00\x00\x00\x00Test\x00", 17),
            false,
            0,
            0,
            mixxx::RgbColor(qRgb(0, 0, 0)),
            QString());
    parseCueEntry(
            QByteArray("\x00\x06\x00\x00\x10\x00\x00\xcc\x00\x00\x01\x00Test\x00", 17),
            false,
            0,
            0,
            mixxx::RgbColor(qRgb(0, 0, 0)),
            QString());
    parseCueEntry(
            QByteArray("\x00\x07\x00\x00\x10\x00\x00\xcc\x00\x00\x00\x01Test\x00", 17),
            false,
            0,
            0,
            mixxx::RgbColor(qRgb(0, 0, 0)),
            QString());

    // Missing null terminator
    parseCueEntry(
            QByteArray("\x00\x08\x00\x00\x10\x00\x00\xcc\x00\x00\x00\x00Test", 16),
            false,
            0,
            0,
            mixxx::RgbColor(qRgb(0, 0, 0)),
            QString());

    //Invalid size
    parseCueEntry(
            QByteArray("\x00\x09\x00\x00\x10\x00\x00\xcc\x00\x00\x00\x00", 12),
            false,
            0,
            0,
            mixxx::RgbColor(qRgb(0, 0, 0)),
            QString());
    parseCueEntry(
            QByteArray("\x00\x0a\x00\x00\x10\x00\x00\xcc\x00\x00\x00\x00\x00\x00", 14),
            false,
            0,
            0,
            mixxx::RgbColor(qRgb(0, 0, 0)),
            QString());
}

TEST_F(SeratoMarkers2Test, ParseLoopEntry) {
    parseLoopEntry(
        QByteArray("\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\x00\x27\xaa\xe1\x00\x00\x00", 21),
        true,
        0, 0, 0, false, QString(""));
    parseLoopEntry(
        QByteArray("\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\xff\xff\xff\xff\x00\x27\xaa\xe1\x00\x01Test\x00", 25),
        true,
        1, 33752069, 101124105, true, QString("Test"));
    parseLoopEntry(
        QByteArray("\x00\x02\x00\x00\x00\xff\x00\x00\x10\x00\xff\xff\xff\xff\x00\x27\xaa\xe1\x00\x00\xc3\xa4\xc3\xbc\xc3\xb6\xc3\x9f\xc3\xa9\xc3\xa8!\x00", 34),
        true,
        2, 255, 4096, false, QString("äüößéè!"));
    parseLoopEntry(
        QByteArray("\x00\x03\x00\x00\x00\x00\x00\x00\x01\x00\xff\xff\xff\xff\x00\x27\xaa\xe1\x00\x01Hello World\x00", 32),
        true,
        3, 0, 256, true, QString("Hello World"));

    // Invalid value
    parseLoopEntry(
        QByteArray("\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x0f\xff\xff\xff\x00\x27\xaa\xe1\x00\x00\x00", 21),
        false,
        0, 0, 0, false, QString());
    parseLoopEntry(
        QByteArray("\x00\x05\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\x01\x27\xaa\xe1\x00\x00\x00\x00", 23),
        false,
        0, 0, 0, false, QString());
    parseLoopEntry(
        QByteArray("\x01\x05\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\x00\x27\xaa\xe1\x00\x00\x00\x00", 23),
        false,
        0, 0, 0, false, QString());
    parseLoopEntry(
        QByteArray("\x00\x05\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\x00\x27\xaa\xe1\x01\x00\x00\x00", 23),
        false,
        0, 0, 0, false, QString());

    // Missing null terminator
    parseLoopEntry(
        QByteArray("\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\x00\x27\xaa\xe1\x00\x00Test", 24),
        false,
        0, 0, 0, false, QString());

    //Invalid size
    parseLoopEntry(
        QByteArray("\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\x00\x27\xaa\xe1\x00\x00", 20),
        false,
        0, 0, 0, false, QString());
    parseLoopEntry(
        QByteArray("\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\x00\x27\xaa\xe1\x00\x00\x00\x00", 22),
        false,
        0, 0, 0, false, QString());
}

TEST_F(SeratoMarkers2Test, ParseMarkers2Data) {
    QDir dir("src/test/serato/data/markers2");
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << "*.octet-stream");

    QFileInfoList list = dir.entryInfoList();
    for (int i = 0; i < list.size(); i++) {
        QFileInfo fileInfo = list.at(i);
        qDebug() << "--- File:" << fileInfo.fileName();
        QFile file(dir.filePath(fileInfo.fileName()));
        bool openOk =  file.open(QIODevice::ReadOnly);
        EXPECT_TRUE(openOk);
        if (!openOk) {
            continue;
        }
        QByteArray data = file.readAll();
        parseMarkers2Data(data, true);
    }
}

TEST_F(SeratoMarkers2Test, ParseEmptyData) {
    QByteArray inputValue;
    mixxx::SeratoMarkers2 seratoMarkers2;
    mixxx::SeratoMarkers2::parse(&seratoMarkers2, inputValue);
    QByteArray outputValue = seratoMarkers2.dump();
    EXPECT_EQ(inputValue, outputValue);
}

}  // namespace
