#include <gtest/gtest.h>

#include <QtGlobal>
#include <QDebug>
#include <QUrl>

#include "library/parserm3u.h"


class PlaylistTest : public testing::Test {};

TEST_F(PlaylistTest, Normalize) {
    ParserM3u parser;

    EXPECT_EQ(parser.playlistEntryToTrackFile("file:///foo/bar.mp3"),
            TrackFile("/foo/bar.mp3"));
    EXPECT_EQ(parser.playlistEntryToTrackFile("file:foo/bar.mp3"),
            TrackFile("foo/bar.mp3"));
#ifdef Q_OS_WIN
    EXPECT_EQ(parser.playlistEntryToTrackFile("file:///c:/foo/bar.mp3"),
            TrackFile("c:/foo/bar.mp3"));
#else
    EXPECT_EQ(parser.playlistEntryToTrackFile("file:///c:/foo/bar.mp3"),
            TrackFile("/c:/foo/bar.mp3"));
#endif
    EXPECT_EQ(parser.playlistEntryToTrackFile("file:///foo%20/bar.mp3"),
            TrackFile("/foo /bar.mp3"));
    EXPECT_EQ(parser.playlistEntryToTrackFile("c:/foo/bar.mp3"),
            TrackFile("c:/foo/bar.mp3"));
    EXPECT_EQ(parser.playlistEntryToTrackFile("c:\\foo\\bar.mp3"),
            TrackFile("c:/foo/bar.mp3"));
}
