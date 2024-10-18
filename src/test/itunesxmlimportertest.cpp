#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QDateTime>
#include <QDir>
#include <QString>
#include <atomic>
#include <memory>

#include "library/itunes/itunesdao.h"
#include "library/itunes/itunesimporter.h"
#include "library/itunes/itunespathmapping.h"
#include "library/itunes/itunesxmlimporter.h"
#include "library/treeitem.h"
#include "test/mixxxtest.h"

class ITunesXMLImporterTest : public MixxxTest {
  protected:
    QDir getITunesTestDir() {
        return MixxxTest::getOrInitTestDir().filePath("itunes/");
    }

    std::unique_ptr<ITunesXMLImporter> makeImporter(
            const QString& xmlFileName, std::unique_ptr<ITunesDAO> dao) {
        QString xmlFilePath = getITunesTestDir().absoluteFilePath(xmlFileName);
        auto importer = std::make_unique<ITunesXMLImporter>(
                nullptr, xmlFilePath, std::move(dao));
        return importer;
    }
};

class MockITunesDAO : public ITunesDAO {
  public:
    MOCK_METHOD(bool, importTrack, (const ITunesTrack&));
    MOCK_METHOD(bool, importPlaylist, (const ITunesPlaylist&));
    MOCK_METHOD(bool, importPlaylistRelation, (int, int));
    MOCK_METHOD(bool, importPlaylistTrack, (int, int, int));
    MOCK_METHOD(bool, applyPathMapping, (const ITunesPathMapping&));
};

using testing::_;
using testing::Return;

namespace {
std::unique_ptr<MockITunesDAO> makeMockDAO() {
    std::unique_ptr<MockITunesDAO> dao = std::make_unique<MockITunesDAO>();
    MockITunesDAO* pDao = dao.get();

    // Delegate the calls to the original implementations so they can still
    // perform internal bookkeeping for the TreeItem. Note that we mock them
    // still so we can place expectations on them. Since the mock dao lives on
    // the heap, capturing it by pointer for its own mock implementations should
    // be safe.
    ON_CALL(*dao, importTrack(_)).WillByDefault([pDao](const ITunesTrack& track) {
        return pDao->ITunesDAO::importTrack(track);
    });
    ON_CALL(*dao, importPlaylist(_)).WillByDefault([pDao](const ITunesPlaylist& playlist) {
        return pDao->ITunesDAO::importPlaylist(playlist);
    });
    ON_CALL(*dao, importPlaylistRelation(_, _)).WillByDefault([pDao](int parentId, int childId) {
        return pDao->ITunesDAO::importPlaylistRelation(parentId, childId);
    });
    ON_CALL(*dao, applyPathMapping(_)).WillByDefault([pDao](const ITunesPathMapping& pathMapping) {
        return pDao->ITunesDAO::applyPathMapping(pathMapping);
    });

    return dao;
}

} // anonymous namespace

TEST_F(ITunesXMLImporterTest, ParseMacOSMusicXML) {
    std::unique_ptr<MockITunesDAO> dao = makeMockDAO();

    QString musicRoot = getITunesTestDir().absoluteFilePath(
            "macOS Music Library/Media.localized/Music");
    EXPECT_CALL(*dao,
            importTrack(ITunesTrack{
                    .id = 479,
                    .artist = "AC/DC",
                    .title = "Highway to Hell",
                    .album = "Highway to Hell",
                    .albumArtist = "AC/DC",
                    .composer = "Angus Young, Bon Scott & Malcolm Young",
                    .genre = "Hard Rock",
                    .grouping = "",
                    .year = 1979,
                    .duration = 208,
                    .location = musicRoot +
                            "/AC_DC/Highway to Hell/01 Highway to Hell.m4a",
                    .rating = 0,
                    .comment = "",
                    .trackNumber = 1,
                    .bpm = 0,
                    .bitrate = 256,
                    .playCount = 0,
                    .lastPlayedAt = {},
                    .dateAdded = QDateTime::fromString("2023-04-25T14:13:36Z", Qt::ISODate),
            }));
    EXPECT_CALL(*dao,
            importTrack(ITunesTrack{
                    .id = 482,
                    .artist = "AC/DC",
                    .title = "Play Ball",
                    .album = "Rock or Bust",
                    .albumArtist = "AC/DC",
                    .composer = "Angus Young & Malcolm Young",
                    .genre = "Rock",
                    .grouping = "",
                    .year = 2014,
                    .duration = 167,
                    .location =
                            musicRoot + "/AC_DC/Rock or Bust/02 Play Ball.m4a",
                    .rating = 0,
                    .comment = "",
                    .trackNumber = 2,
                    .bpm = 0,
                    .bitrate = 256,
                    .playCount = 0,
                    .lastPlayedAt = {},
                    .dateAdded = QDateTime::fromString(
                            "2023-04-25T14:13:36Z", Qt::ISODate),
            }));
    EXPECT_CALL(*dao,
            importTrack(ITunesTrack{
                    .id = 476,
                    .artist = "Jet",
                    .title = "Are You Gonna Be My Girl",
                    .album = "Are You Gonna Be My Girl - Single",
                    .albumArtist = "Jet",
                    .composer = "Cam Muncey & Nic Cester",
                    .genre = "Rock",
                    .grouping = "",
                    .year = 2003,
                    .duration = 213,
                    .location = musicRoot +
                            "/Jet/Are You Gonna Be My Girl - Single/01 Are You "
                            "Gonna Be My Girl.m4a",
                    .rating = 0,
                    .comment = "",
                    .trackNumber = 1,
                    .bpm = 0,
                    .bitrate = 256,
                    .playCount = 0,
                    .lastPlayedAt = {},
                    .dateAdded = QDateTime::fromString("2023-04-25T14:13:36Z", Qt::ISODate),
            }));
    EXPECT_CALL(*dao,
            importTrack(ITunesTrack{
                    .id = 473,
                    .artist = "Ray Charles",
                    .title = "What I'd Say",
                    .album = "What I'd Say",
                    .albumArtist = "Ray Charles",
                    .composer = {},
                    .genre = "Rock",
                    .grouping = "",
                    .year = 2012,
                    .duration = 304,
                    .location = musicRoot + "/Ray Charles/What I'd Say/01 What I'd Say.m4a",
                    .rating = 0,
                    .comment = "",
                    .trackNumber = 1,
                    .bpm = 0,
                    .bitrate = 256,
                    .playCount = 0,
                    .lastPlayedAt = {},
                    .dateAdded = QDateTime::fromString("2023-04-25T14:13:36Z", Qt::ISODate),
            }));
    EXPECT_CALL(*dao,
            importTrack(ITunesTrack{
                    .id = 470,
                    .artist = "The Rolling Stones",
                    .title = "Beast of Burden (Live)",
                    .album = "Sweet Summer Sun, Live in Hyde Park 2013 (Live) "
                             "- Single",
                    .albumArtist = "The Rolling Stones",
                    .composer = "Keith Richards & Mick Jagger",
                    .genre = "Rock",
                    .grouping = "",
                    .year = 2013,
                    .duration = 306,
                    .location = musicRoot +
                            "/The Rolling Stones/Sweet Summer Sun, Live in "
                            "Hyde Park 2013 (Live) - Single/01 Beast of Burden "
                            "(Live).m4a",
                    .rating = 0,
                    .comment = "",
                    .trackNumber = 1,
                    .bpm = 0,
                    .bitrate = 256,
                    .playCount = 0,
                    .lastPlayedAt = {},
                    .dateAdded = QDateTime::fromString("2023-04-25T14:13:36Z", Qt::ISODate),
            }));
    EXPECT_CALL(*dao,
            importTrack(ITunesTrack{
                    .id = 467,
                    .artist = "Mungo Jerry",
                    .title = "In the Summertime",
                    .album = "In the Summertime",
                    .albumArtist = "Mungo Jerry",
                    .composer = "Dorset",
                    .genre = "Rock",
                    .grouping = "",
                    .year = 1970,
                    .duration = 214,
                    .location = musicRoot +
                            "/Compilations/In the Summertime/01 In the "
                            "Summertime.m4a",
                    .rating = 0,
                    .comment = "",
                    .trackNumber = 1,
                    .bpm = 0,
                    .bitrate = 256,
                    .playCount = 0,
                    .lastPlayedAt = {},
                    .dateAdded = QDateTime::fromString("2023-04-25T14:13:36Z", Qt::ISODate),
            }));

    EXPECT_CALL(*dao, importPlaylist(ITunesPlaylist{
                              .id = 1425,
                              .name = "Folder A",
                      }));
    EXPECT_CALL(*dao, importPlaylist(ITunesPlaylist{
                              .id = 1498,
                              .name = "Folder B",
                      }));
    EXPECT_CALL(*dao, importPlaylist(ITunesPlaylist{
                              .id = 1431,
                              .name = "Playlist A",
                      }));
    EXPECT_CALL(*dao, importPlaylist(ITunesPlaylist{
                              .id = 1436,
                              .name = "Playlist B",
                      }));
    EXPECT_CALL(*dao, importPlaylist(ITunesPlaylist{
                              .id = 1494,
                              .name = "Playlist C",
                      }));
    EXPECT_CALL(*dao, importPlaylist(ITunesPlaylist{
                              .id = 1440,
                              .name = "Downloaded",
                      }));
    EXPECT_CALL(*dao, importPlaylist(ITunesPlaylist{
                              .id = 1449,
                              .name = "Playlist D",
                      }));

    int root = kRootITunesPlaylistId;
    EXPECT_CALL(*dao, importPlaylistRelation(root, 1425)); // Folder A
    EXPECT_CALL(*dao, importPlaylistRelation(1425, 1498)); // - Folder B
    EXPECT_CALL(*dao, importPlaylistRelation(1498, 1431)); //   - Playlist A
    EXPECT_CALL(*dao, importPlaylistRelation(1498, 1436)); //   - Playlist B
    EXPECT_CALL(*dao, importPlaylistRelation(1425, 1494)); // - Playlist C
    EXPECT_CALL(*dao, importPlaylistRelation(root, 1440)); // Downloaded (smart playlist)
    EXPECT_CALL(*dao, importPlaylistRelation(root, 1449)); // Playlist D

    std::unique_ptr<ITunesXMLImporter> importer =
            makeImporter("macOS Music Library.xml", std::move(dao));
    ITunesImport import = importer->importLibrary();

    TreeItem* rootItem = import.playlistRoot.get();
    EXPECT_EQ(rootItem->children().size(), 3);
    EXPECT_EQ(rootItem->child(0)->getLabel().toStdString(), "Folder A");
    EXPECT_EQ(rootItem->child(1)->getLabel().toStdString(), "Downloaded");
    EXPECT_EQ(rootItem->child(2)->getLabel().toStdString(), "Playlist D");

    TreeItem* folderA = rootItem->child(0);
    EXPECT_EQ(folderA->children().size(), 2);
    EXPECT_EQ(folderA->child(0)->getLabel().toStdString(), "Folder B");
    EXPECT_EQ(folderA->child(1)->getLabel().toStdString(), "Playlist C");

    TreeItem* folderB = folderA->child(0);
    EXPECT_EQ(folderB->children().size(), 2);
    EXPECT_EQ(folderB->child(0)->getLabel().toStdString(), "Playlist A");
    EXPECT_EQ(folderB->child(1)->getLabel().toStdString(), "Playlist B");
}

TEST_F(ITunesXMLImporterTest, ParseITunesMusicXML) {
    std::unique_ptr<MockITunesDAO> dao = makeMockDAO();

    QString musicRoot =
#ifdef _WIN32
            "";
#else
            "/";
#endif
    musicRoot += "Z:/Media.localized/Music";
    EXPECT_CALL(*dao,
            importTrack(ITunesTrack{
                    .id = 77,
                    .artist = "AC/DC",
                    .title = "Highway to Hell",
                    .album = "Highway to Hell",
                    .albumArtist = "AC/DC",
                    .composer = "Angus Young, Bon Scott & Malcolm Young",
                    .genre = "Hard Rock",
                    .grouping = "",
                    .year = 1979,
                    .duration = 208,
                    .location = musicRoot +
                            "/AC_DC/Highway to Hell/01 Highway to Hell.m4a",
                    .rating = 0,
                    .comment = "",
                    .trackNumber = 1,
                    .bpm = 0,
                    .bitrate = 256,
                    .playCount = 0,
                    .lastPlayedAt = {},
                    .dateAdded = QDateTime::fromString("2023-04-25T14:13:36Z", Qt::ISODate),
            }));
    EXPECT_CALL(*dao,
            importTrack(ITunesTrack{
                    .id = 79,
                    .artist = "AC/DC",
                    .title = "Play Ball",
                    .album = "Rock or Bust",
                    .albumArtist = "AC/DC",
                    .composer = "Angus Young & Malcolm Young",
                    .genre = "Rock",
                    .grouping = "",
                    .year = 2014,
                    .duration = 167,
                    .location =
                            musicRoot + "/AC_DC/Rock or Bust/02 Play Ball.m4a",
                    .rating = 0,
                    .comment = "",
                    .trackNumber = 2,
                    .bpm = 0,
                    .bitrate = 256,
                    .playCount = 0,
                    .lastPlayedAt = {},
                    .dateAdded = QDateTime::fromString(
                            "2023-04-25T14:13:36Z", Qt::ISODate),
            }));
    EXPECT_CALL(*dao,
            importTrack(ITunesTrack{
                    .id = 81,
                    .artist = "Jet",
                    .title = "Are You Gonna Be My Girl",
                    .album = "Are You Gonna Be My Girl - Single",
                    .albumArtist = "Jet",
                    .composer = "Cam Muncey & Nic Cester",
                    .genre = "Rock",
                    .grouping = "",
                    .year = 2003,
                    .duration = 213,
                    .location = musicRoot + "/Jet/Are You Gonna Be My Girl - Single/01 Are You "
                                            "Gonna Be My Girl.m4a",
                    .rating = 0,
                    .comment = "",
                    .trackNumber = 1,
                    .bpm = 0,
                    .bitrate = 256,
                    .playCount = 0,
                    .lastPlayedAt = {},
                    .dateAdded = QDateTime::fromString("2023-04-25T14:13:36Z", Qt::ISODate),
            }));
    EXPECT_CALL(*dao,
            importTrack(ITunesTrack{
                    .id = 83,
                    .artist = "Ray Charles",
                    .title = "What I'd Say",
                    .album = "What I'd Say",
                    .albumArtist = "Ray Charles",
                    .composer = {},
                    .genre = "Rock",
                    .grouping = "",
                    .year = 2012,
                    .duration = 304,
                    .location = musicRoot + "/Ray Charles/What I'd Say/01 What I'd Say.m4a",
                    .rating = 0,
                    .comment = "",
                    .trackNumber = 1,
                    .bpm = 0,
                    .bitrate = 256,
                    .playCount = 1,
                    .lastPlayedAt = QDateTime::fromString("2023-04-25T14:18:39Z", Qt::ISODate),
                    .dateAdded = QDateTime::fromString("2023-04-25T14:13:36Z", Qt::ISODate),
            }));
    EXPECT_CALL(*dao,
            importTrack(ITunesTrack{
                    .id = 85,
                    .artist = "The Rolling Stones",
                    .title = "Beast of Burden (Live)",
                    .album = "Sweet Summer Sun, Live in Hyde Park 2013 (Live) "
                             "- Single",
                    .albumArtist = "The Rolling Stones",
                    .composer = "Keith Richards & Mick Jagger",
                    .genre = "Rock",
                    .grouping = "",
                    .year = 2013,
                    .duration = 306,
                    .location =
                            musicRoot + "/The Rolling "
                                        "Stones/Sweet Summer Sun, Live in Hyde Park 2013 "
                                        "(Live) - Single/01 Beast of Burden (Live).m4a",
                    .rating = 0,
                    .comment = "",
                    .trackNumber = 1,
                    .bpm = 0,
                    .bitrate = 256,
                    .playCount = 0,
                    .lastPlayedAt = {},
                    .dateAdded = QDateTime::fromString("2023-04-25T14:13:36Z", Qt::ISODate),
            }));
    EXPECT_CALL(*dao,
            importTrack(ITunesTrack{
                    .id = 87,
                    .artist = "Mungo Jerry",
                    .title = "In the Summertime",
                    .album = "In the Summertime",
                    .albumArtist = "Mungo Jerry",
                    .composer = "Dorset",
                    .genre = "Rock",
                    .grouping = "",
                    .year = 1970,
                    .duration = 214,
                    .location =
                            musicRoot + "/Compilations/"
                                        "In the Summertime/01 In the Summertime.m4a",
                    .rating = 0,
                    .comment = "",
                    .trackNumber = 1,
                    .bpm = 0,
                    .bitrate = 256,
                    .playCount = 0,
                    .lastPlayedAt = {},
                    .dateAdded = QDateTime::fromString("2023-04-25T14:13:36Z", Qt::ISODate),
            }));

    EXPECT_CALL(*dao, importPlaylist(ITunesPlaylist{
                              .id = 153,
                              .name = "Folder A",
                      }));
    EXPECT_CALL(*dao, importPlaylist(ITunesPlaylist{
                              .id = 159,
                              .name = "Folder B",
                      }));
    EXPECT_CALL(*dao, importPlaylist(ITunesPlaylist{
                              .id = 165,
                              .name = "Playlist A",
                      }));
    EXPECT_CALL(*dao, importPlaylist(ITunesPlaylist{
                              .id = 170,
                              .name = "Playlist B",
                      }));
    EXPECT_CALL(*dao, importPlaylist(ITunesPlaylist{
                              .id = 174,
                              .name = "Playlist C",
                      }));
    EXPECT_CALL(*dao, importPlaylist(ITunesPlaylist{
                              .id = 177,
                              .name = "Downloaded",
                      }));
    EXPECT_CALL(*dao, importPlaylist(ITunesPlaylist{
                              .id = 186,
                              .name = "Playlist D",
                      }));

    int root = kRootITunesPlaylistId;
    EXPECT_CALL(*dao, importPlaylistRelation(root, 153)); // Folder A
    EXPECT_CALL(*dao, importPlaylistRelation(153, 159));  // - Folder B
    EXPECT_CALL(*dao, importPlaylistRelation(159, 165));  //   - Playlist A
    EXPECT_CALL(*dao, importPlaylistRelation(159, 170));  //   - Playlist B
    EXPECT_CALL(*dao, importPlaylistRelation(153, 174));  // - Playlist C
    EXPECT_CALL(*dao, importPlaylistRelation(root, 177)); // Downloaded (smart playlist)
    EXPECT_CALL(*dao, importPlaylistRelation(root, 186)); // Playlist D

    std::unique_ptr<ITunesXMLImporter> importer =
            makeImporter("iTunes Music Library.xml", std::move(dao));
    ITunesImport import = importer->importLibrary();

    TreeItem* rootItem = import.playlistRoot.get();
    EXPECT_EQ(rootItem->children().size(), 3);
    EXPECT_EQ(rootItem->child(0)->getLabel().toStdString(), "Folder A");
    EXPECT_EQ(rootItem->child(1)->getLabel().toStdString(), "Downloaded");
    EXPECT_EQ(rootItem->child(2)->getLabel().toStdString(), "Playlist D");

    TreeItem* folderA = rootItem->child(0);
    EXPECT_EQ(folderA->children().size(), 2);
    EXPECT_EQ(folderA->child(0)->getLabel().toStdString(), "Folder B");
    EXPECT_EQ(folderA->child(1)->getLabel().toStdString(), "Playlist C");

    TreeItem* folderB = folderA->child(0);
    EXPECT_EQ(folderB->children().size(), 2);
    EXPECT_EQ(folderB->child(0)->getLabel().toStdString(), "Playlist A");
    EXPECT_EQ(folderB->child(1)->getLabel().toStdString(), "Playlist B");
}
