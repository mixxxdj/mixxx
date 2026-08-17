#include <gtest/gtest.h>

#include <QSqlQuery>

#include "library/dao/playlistdao.h"
#include "library/dao/trackschema.h"
#include "test/librarytest.h"
#include "track/trackid.h"

class PlaylistDAOEndMarkerTest : public LibraryTest {
  protected:
    PlaylistDAO& dao() {
        return internalCollection()->getPlaylistDAO();
    }

    int createAutoDJPlaylist() {
        int id = dao().getPlaylistIdFromName(QStringLiteral("Auto DJ"));
        if (id < 0) {
            id = dao().createPlaylist(
                    QStringLiteral("Auto DJ"), PlaylistDAO::PLHT_AUTO_DJ);
        }
        return id;
    }

    int countEndMarkerRows() {
        QSqlQuery q(dbConnection());
        q.prepare(QStringLiteral("SELECT COUNT(*) FROM library WHERE location = :loc"));
        q.bindValue(":loc", LIBRARYTABLE_AUTODJ_END_MARKER_LOCATION);
        if (!q.exec() || !q.next()) {
            return -1;
        }
        return q.value(0).toInt();
    }

    int countPlaylistRows(int playlistId) {
        QSqlQuery q(dbConnection());
        q.prepare(QStringLiteral(
                "SELECT COUNT(*) FROM PlaylistTracks WHERE playlist_id = :id"));
        q.bindValue(":id", playlistId);
        if (!q.exec() || !q.next()) {
            return -1;
        }
        return q.value(0).toInt();
    }

    TrackId getPlaylistTrackId(int playlistId, int position) {
        QSqlQuery q(dbConnection());
        q.prepare(QStringLiteral(
                "SELECT track_id FROM PlaylistTracks "
                "WHERE playlist_id = :id AND position = :pos"));
        q.bindValue(":id", playlistId);
        q.bindValue(":pos", position);
        if (!q.exec() || !q.next()) {
            return TrackId();
        }
        return TrackId(q.value(0));
    }
};

TEST_F(PlaylistDAOEndMarkerTest, CreatesMarkerWhenAbsent) {
    TrackId id = dao().getOrCreateAutoDJEndMarker();
    EXPECT_TRUE(id.isValid());
}

TEST_F(PlaylistDAOEndMarkerTest, ReturnsSameIdOnSecondCall) {
    TrackId first = dao().getOrCreateAutoDJEndMarker();
    TrackId second = dao().getOrCreateAutoDJEndMarker();
    ASSERT_TRUE(first.isValid());
    EXPECT_EQ(first, second);
    EXPECT_EQ(1, countEndMarkerRows());
}

TEST_F(PlaylistDAOEndMarkerTest, EndMarkerNotReturnedByLocationJoin) {
    TrackId id = dao().getOrCreateAutoDJEndMarker();
    ASSERT_TRUE(id.isValid());

    // A query that joins on track_locations (as the normal library view does)
    // must NOT return the end marker. library.location holds the sentinel text
    // 'mixxx://autodj/transition', which is TEXT and never equals any INTEGER
    // track_locations.id in SQLite's type-based comparison.
    QSqlQuery q(dbConnection());
    q.prepare(QStringLiteral(
            "SELECT library.id FROM library "
            "INNER JOIN track_locations ON track_locations.id = library.location "
            "WHERE library.id = :id"));
    q.bindValue(":id", id.toVariant());
    ASSERT_TRUE(q.exec());
    EXPECT_FALSE(q.next());
}

TEST_F(PlaylistDAOEndMarkerTest, InsertsAtCorrectPosition) {
    int playlistId = createAutoDJPlaylist();
    ASSERT_GT(playlistId, 0);

    // Insert at position 1 (empty playlist: max_position=0, so position 1 is valid).
    bool ok = dao().insertEndMarkerIntoPlaylist(playlistId, 1);
    ASSERT_TRUE(ok);

    TrackId markerId = dao().getOrCreateAutoDJEndMarker();
    EXPECT_EQ(markerId, getPlaylistTrackId(playlistId, 1));
}

TEST_F(PlaylistDAOEndMarkerTest, MultipleInserts_ReferencesSameTrackId) {
    int playlistId = createAutoDJPlaylist();
    ASSERT_GT(playlistId, 0);

    ASSERT_TRUE(dao().insertEndMarkerIntoPlaylist(playlistId, 1));
    ASSERT_TRUE(dao().insertEndMarkerIntoPlaylist(playlistId, 2));

    EXPECT_EQ(2, countPlaylistRows(playlistId));

    TrackId markerId = dao().getOrCreateAutoDJEndMarker();
    EXPECT_EQ(markerId, getPlaylistTrackId(playlistId, 1));
    EXPECT_EQ(markerId, getPlaylistTrackId(playlistId, 2));
}

TEST_F(PlaylistDAOEndMarkerTest, InvalidPlaylistId_ReturnsFalse) {
    bool ok = dao().insertEndMarkerIntoPlaylist(-1, 1);
    EXPECT_FALSE(ok);
}

TEST_F(PlaylistDAOEndMarkerTest, RemoveHiddenTracks_DoesNotRemoveEndMarker) {
    int playlistId = createAutoDJPlaylist();
    ASSERT_GT(playlistId, 0);

    ASSERT_TRUE(dao().insertEndMarkerIntoPlaylist(playlistId, 1));
    ASSERT_EQ(1, countPlaylistRows(playlistId));

    dao().removeHiddenTracks(playlistId);

    // End marker must survive: it has mixxx_deleted=0 and matches the
    // INNER JOIN on library, even though it has no track_locations entry.
    EXPECT_EQ(1, countPlaylistRows(playlistId));
}
