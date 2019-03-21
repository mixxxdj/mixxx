#include <gtest/gtest.h>

#include <QtGlobal>
#include <QDebug>
#include <QUrl>

#include "library/parserm3u.h"


class PlaylistTest : public testing::Test {};

TEST_F(PlaylistTest, Normalize) {
    ParserM3u parser;

    EXPECT_STREQ(parser.playlistEntrytoLocalFile("file:///foo/bar.mp3").toStdString().c_str(),
            "/foo/bar.mp3");
    EXPECT_STREQ(parser.playlistEntrytoLocalFile("file:foo/bar.mp3").toStdString().c_str(),
            "foo/bar.mp3");
#ifdef Q_OS_WIN
    EXPECT_STREQ(parser.playlistEntrytoLocalFile("file:///c:/foo/bar.mp3").toStdString().c_str(),
            "c:/foo/bar.mp3");
#else
    EXPECT_STREQ(parser.playlistEntrytoLocalFile("file:///c:/foo/bar.mp3").toStdString().c_str(),
            "/c:/foo/bar.mp3");
#endif
    EXPECT_STREQ(parser.playlistEntrytoLocalFile("file:///foo%20/bar.mp3").toStdString().c_str(),
            "/foo /bar.mp3");
    EXPECT_STREQ(parser.playlistEntrytoLocalFile("c:/foo/bar.mp3").toStdString().c_str(),
            "c:/foo/bar.mp3");
    EXPECT_STREQ(parser.playlistEntrytoLocalFile("c:\\foo\\bar.mp3").toStdString().c_str(),
            "c:/foo/bar.mp3");
}
