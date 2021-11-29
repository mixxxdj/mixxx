#include <gtest/gtest.h>

#include <QDataStream>
#include <QDebug>
#include <QTemporaryFile>
#include <QUrl>
#include <QtGlobal>

#include "library/parser.h"
#include "library/parserm3u.h"

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

TEST_F(PlaylistTest, Normalize) {
    DummyParser parser;

    EXPECT_EQ(QString("/foo/bar.mp3"),
            parser.playlistEntryToFilePath("file:///foo/bar.mp3"));
    EXPECT_EQ(QString("foo/bar.mp3"),
            parser.playlistEntryToFilePath("file:foo/bar.mp3"));
#ifdef Q_OS_WIN
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

TEST_F(PlaylistTest, m3uEndOfLine) {
    QTemporaryFile m3uFile;
    ASSERT_TRUE(m3uFile.open());
    m3uFile.write("crlf.mp3\r\n");
    m3uFile.write("cr.mp3\r");
    m3uFile.write("lf.mp3\n");
    // Check for Windows-1250 Euro Sign
    m3uFile.write("EuroSign\x80.mp3\n");
    m3uFile.write("end.mp3");
    m3uFile.close();

    QList<QString> entries = ParserM3u().parse(m3uFile.fileName());
    EXPECT_EQ(entries.size(), 5);
    if (entries.size() == 5) {
        EXPECT_TRUE(entries[0].endsWith(QStringLiteral("crlf.mp3")));
        EXPECT_TRUE(entries[1].endsWith(QStringLiteral("cr.mp3")));
        EXPECT_TRUE(entries[2].endsWith(QStringLiteral("lf.mp3")));
        EXPECT_TRUE(entries[3].endsWith(QStringLiteral("EuroSign\u20AC.mp3")));
        EXPECT_TRUE(entries[4].endsWith(QStringLiteral("end.mp3")));
    }
}
