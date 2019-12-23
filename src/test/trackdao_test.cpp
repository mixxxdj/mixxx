#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "test/librarytest.h"

using ::testing::UnorderedElementsAre;

class TrackDAOTest : public LibraryTest {
};


TEST_F(TrackDAOTest, detectMovedTracks) {
    TrackDAO& trackDAO = internalCollection()->getTrackDAO();

    QString filename = QStringLiteral("file.mp3");

    TrackFile oldFile(QDir(QDir::tempPath() + QStringLiteral("/old/dir1")), filename);
    TrackFile newFile(QDir(QDir::tempPath() + QStringLiteral("/new/dir1")), filename);
    TrackFile otherFile(QDir(QDir::tempPath() + QStringLiteral("/new")), filename);

    TrackPointer pOldTrack = Track::newTemporary(oldFile);
    TrackPointer pNewTrack = Track::newTemporary(newFile);
    TrackPointer pOtherTrack = Track::newTemporary(otherFile);

    // Arbitrary duration
    pOldTrack->setDuration(135);
    pNewTrack->setDuration(135.7);
    pOtherTrack->setDuration(135.7);

    TrackId oldId = internalCollection()->addTrack(pOldTrack, false);
    TrackId newId = internalCollection()->addTrack(pNewTrack, false);
    internalCollection()->addTrack(pOtherTrack, false);

    // Mark as missing
    QSqlQuery query(dbConnection());
    query.prepare("UPDATE track_locations SET fs_deleted=1 WHERE location=:location");
    query.bindValue(":location", oldFile.location());
    query.exec();

    QList<QPair<TrackRef, TrackRef>> replacedTracks;
    QStringList addedTracks(newFile.location());
    bool cancel = false;
    trackDAO.detectMovedTracks(&replacedTracks, addedTracks, &cancel);

    QSet<TrackId> removedTrackIds;
    QSet<TrackId> addedTrackIds;
    for (const auto& replacedTrack : replacedTracks) {
        removedTrackIds += replacedTrack.first.getId();
        addedTrackIds += replacedTrack.second.getId();
    }

    EXPECT_THAT(removedTrackIds, UnorderedElementsAre(oldId));
    EXPECT_THAT(addedTrackIds, UnorderedElementsAre(newId));

    QSet<QString> trackLocations = trackDAO.getTrackLocations();
    EXPECT_THAT(trackLocations, UnorderedElementsAre(newFile.location(), otherFile.location()));
}
