#include <gtest/gtest.h>

#include <QDataStream>
#include <QDebug>
#include <QTemporaryFile>
#include <QUrl>
#include <QtGlobal>

#include "library/parser.h"
#include "library/parsercsv.h"
#include "library/parserm3u.h"
#include "library/parserpls.h"

class DummyParser : public Parser {
  public:
    QString playlistEntryToFilePath(
            const QString& playlistEntry,
            const QString& basePath = QString()) {
        const auto fileInfo = Parser::playlistEntryToFileInfo(playlistEntry, basePath);
        // Return the plain, literal file path, because the location
        // is undefined if relative paths.
        return fileInfo.asQFileInfo().filePath();
    }
};

class PlaylistTest : public testing::Test {};

TEST_F(PlaylistTest, IsPlaylistFilenameSupported) {
    EXPECT_TRUE(ParserCsv::isPlaylistFilenameSupported("test.csv"));
    EXPECT_FALSE(ParserCsv::isPlaylistFilenameSupported("test.mp3"));
}

TEST_F(PlaylistTest, ParseAllLocations) {
    QTemporaryFile csvFile;
    ASSERT_TRUE(csvFile.open());
    csvFile.write("Location,OtherData\n");
    csvFile.write("/path/to/file1,/other/data\n");
    csvFile.write("/path/to/file2,/other/data\n");
    csvFile.close();

    QList<QString> locations = ParserCsv::parseAllLocations(csvFile.fileName());

    EXPECT_EQ(locations.size(), 2);
    EXPECT_EQ(locations[0], "/path/to/file1");
    EXPECT_EQ(locations[1], "/path/to/file2");
}

TEST_F(PlaylistTest, ParseEmptyFile) {
    QTemporaryFile csvFile;
    ASSERT_TRUE(csvFile.open());
    csvFile.close();

    const QList<QString> entries = ParserCsv().parseAllLocations(csvFile.fileName());

    // Check that the entries list is empty
    EXPECT_TRUE(entries.isEmpty());
}
TEST_F(PlaylistTest, ParseWithDifferentLocationColumnNamesAndFormats) {
    // Test with different location column named "Dateiname" instead of "Location"
    QStringList paths = {
            "C:\\path\\to\\file1",
            "%USERPROFILE%\\Music\\file2",
            "Music\\file3",
            "C:/path/to/file4",
            "/path/to/file5",
            "$HOME/Music/file6",
            "Music/file7",
            "\"Michael Jackson\"",
            "Michael Jackson"};

    for (const QString& path : paths) {
        QTemporaryFile csvFile;
        ASSERT_TRUE(csvFile.open());
        csvFile.write("Dateiname,OtherData\n");
        csvFile.write(path.toUtf8() + ",other-data\n");
        csvFile.close();

        const QList<QString> entries = ParserCsv().parseAllLocations(csvFile.fileName());

        // Check the contents of the entries list
#ifdef Q_OS_WIN
        // Note, that $HOME is not expanded on Windows, but legal path syntax
        if (path == "\"Michael Jackson\"" || path == "Michael Jackson") {
            // Check that the entries list is empty for invalid path syntax
            EXPECT_TRUE(entries.isEmpty());
        } else {
            // Check the size of the entries list
            EXPECT_EQ(entries.size(), 1);
            if (!entries.isEmpty()) {
                EXPECT_EQ(entries[0], path);
            }
        }
#else
        if (path == "C:\\path\\to\\file1" ||
                path == "%USERPROFILE%\\Music\\file2" ||
                path == "Music\\file3" || path == "\"Michael Jackson\"" ||
                path == "Michael Jackson") {
            // Check that the entries list is empty for invalid path syntax
            EXPECT_TRUE(entries.isEmpty());
        } else {
            // Check the size of the entries list
            EXPECT_EQ(entries.size(), 1);
            if (!entries.isEmpty()) {
                EXPECT_EQ(entries[0], path);
            }
        }
#endif
    }
}

TEST_F(PlaylistTest, ParseWithLocationColumn) {
    QTemporaryFile csvFile;
    ASSERT_TRUE(csvFile.open());
    csvFile.write("#,Location\n");
    csvFile.write("1,/path/to/file1\n");
    csvFile.write("2,/path/to/file2\n");
    csvFile.close();

    const QList<QString> entries = ParserCsv().parseAllLocations(csvFile.fileName());

    // Check the size of the entries list
    EXPECT_EQ(entries.size(), 2);

    // Check the contents of the entries list
    EXPECT_EQ(entries[0], "/path/to/file1");
    EXPECT_EQ(entries[1], "/path/to/file2");
}

TEST_F(PlaylistTest, Normalize) {
    DummyParser parser;

    EXPECT_EQ(QString("/foo/bar.mp3"),
            parser.playlistEntryToFilePath("file:///foo/bar.mp3"));
    EXPECT_EQ(QString("foo/bar.mp3"),
            parser.playlistEntryToFilePath("file:foo/bar.mp3"));
#ifdef _WIN32
    EXPECT_EQ(QString("c:/foo/bar.mp3"),
            parser.playlistEntryToFilePath("file:///c:/foo/bar.mp3"));
#else
    EXPECT_EQ(QString("/c:/foo/bar.mp3"),
            parser.playlistEntryToFilePath("file:///c:/foo/bar.mp3"));
#endif
    EXPECT_EQ(QString("/foo /bar.mp3"),
            parser.playlistEntryToFilePath("file:///foo%20/bar.mp3"));
    EXPECT_EQ(QString("c:/foo/bar.mp3"),
            parser.playlistEntryToFilePath("c:/foo/bar.mp3"));
    EXPECT_EQ(QString("c:/foo/bar.mp3"),
            parser.playlistEntryToFilePath("c:\\foo\\bar.mp3"));
}

TEST_F(PlaylistTest, Relative) {
    DummyParser parser;

    EXPECT_EQ(QString("../foo/bar.mp3"),
            parser.playlistEntryToFilePath("../foo/bar.mp3", ""));
    EXPECT_EQ(QString("base/folder/../foo/bar.mp3"),
            parser.playlistEntryToFilePath("../foo/bar.mp3", "base/folder"));
    EXPECT_EQ(QString("base/folder/../../bar.mp3"),
            parser.playlistEntryToFilePath("../../bar.mp3", "base/folder"));
}

TEST_F(PlaylistTest, M3uEndOfLine) {
    QTemporaryFile m3uFile;
    ASSERT_TRUE(m3uFile.open());
    m3uFile.write("crlf.mp3\r\n");
    m3uFile.write("cr.mp3\r");
    m3uFile.write("lf.mp3\n");
    // Check for Windows-1250 Euro Sign
    m3uFile.write("EuroSign\x80.mp3\n");
    m3uFile.write("end.mp3");
    m3uFile.close();

    const QList<QString> entries = ParserM3u().parseAllLocations(m3uFile.fileName());
    ASSERT_EQ(entries.size(), 5);
    EXPECT_TRUE(entries.at(0).endsWith(QStringLiteral("crlf.mp3")));
    EXPECT_TRUE(entries.at(1).endsWith(QStringLiteral("cr.mp3")));
    EXPECT_TRUE(entries.at(2).endsWith(QStringLiteral("lf.mp3")));
    EXPECT_TRUE(entries.at(3).endsWith(QStringLiteral("EuroSign\u20AC.mp3")));
    EXPECT_TRUE(entries.at(4).endsWith(QStringLiteral("end.mp3")));
}

TEST_F(PlaylistTest, CsvEndOfLine) {
    QTemporaryFile csvFile;
    ASSERT_TRUE(csvFile.open());
    csvFile.write("#,Location\r\n");
    csvFile.write("1,cr.mp3\r");
    csvFile.write("2,lf.mp3\n");
    csvFile.close();

    const QList<QString> entries = ParserCsv().parseAllLocations(csvFile.fileName());
    ASSERT_EQ(entries.size(), 2);
    EXPECT_TRUE(entries.at(0).endsWith(QStringLiteral("cr.mp3")));
    EXPECT_TRUE(entries.at(1).endsWith(QStringLiteral("lf.mp3")));
}

TEST_F(PlaylistTest, PlsEndOfLine) {
    QTemporaryFile plsFile;
    ASSERT_TRUE(plsFile.open());
    plsFile.write("[playlist]\n");
    plsFile.write("NumberOfEntries=2\r");
    plsFile.write("File0=cr.mp3\r");
    plsFile.write("File1=lf.mp3\n");
    plsFile.close();

    const QList<QString> entries = ParserPls().parseAllLocations(plsFile.fileName());
    ASSERT_EQ(entries.size(), 2);
    EXPECT_TRUE(entries.at(0).endsWith(QStringLiteral("cr.mp3")));
    EXPECT_TRUE(entries.at(1).endsWith(QStringLiteral("lf.mp3")));
}
