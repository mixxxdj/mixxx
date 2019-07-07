#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "test/librarytest.h"

using ::testing::UnorderedElementsAre;

class TrackDAOTest : public LibraryTest {
};


TEST_F(TrackDAOTest, detectMovedTracks) {
    TrackDAO& trackDAO = collection()->getTrackDAO();

    QString filename("file.mp3");

    TrackFile oldFile(QDir::tempPath() + "/old/dir1", filename);
    TrackFile newFile(QDir::tempPath() + "/new/dir1", filename);
    TrackFile otherFile(QDir::tempPath() + "/new", filename);

    TrackPointer pOldTrack = Track::newTemporary(oldFile);
    TrackPointer pNewTrack = Track::newTemporary(newFile);
    TrackPointer pOtherTrack = Track::newTemporary(otherFile);

    // Arbitrary duration
    pOldTrack->setDuration(135);
    pNewTrack->setDuration(135.7);
    pOtherTrack->setDuration(135.7);

    trackDAO.addTracksPrepare();
    TrackId oldId = trackDAO.addTracksAddTrack(pOldTrack, false);
    TrackId newId = trackDAO.addTracksAddTrack(pNewTrack, false);
    trackDAO.addTracksAddTrack(pOtherTrack, false);
    trackDAO.addTracksFinish(false);

    // Mark as missing
    QSqlQuery query(dbConnection());
    query.prepare("UPDATE track_locations SET fs_deleted=1 WHERE location=:location");
    query.bindValue(":location", oldFile.location());
    query.exec();

    QSet<TrackId> tracksMovedSetOld;
    QSet<TrackId> tracksMovedSetNew;
    QStringList addedTracks(newFile.location());
    bool cancel = false;
    trackDAO.detectMovedTracks(&tracksMovedSetOld, &tracksMovedSetNew, addedTracks, &cancel);

    EXPECT_THAT(tracksMovedSetOld, UnorderedElementsAre(oldId));
    EXPECT_THAT(tracksMovedSetNew, UnorderedElementsAre(newId));

    QSet<QString> trackLocations = trackDAO.getTrackLocations();
    EXPECT_THAT(trackLocations, UnorderedElementsAre(newFile.location(), otherFile.location()));
}
