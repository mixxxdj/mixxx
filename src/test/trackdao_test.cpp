#include "library/dao/trackdao.h"

#include <gmock/gmock-generated-matchers.h>
#include <gmock/gmock-matchers.h>
#include <gtest/gtest-message.h>
#include <gtest/gtest-test-part.h>

#include <QDir>
#include <QList>
#include <QSet>
#include <QSqlQuery>
#include <QStaticStringData>
#include <QString>
#include <QStringBuilder>
#include <QStringList>
#include <QStringLiteral>
#include <QVariant>
#include <QtGlobal>
#include <memory>
#include <type_traits>

#include "gtest/gtest_pred_impl.h"
#include "library/relocatedtrack.h"
#include "library/trackcollection.h"
#include "test/librarytest.h"
#include "track/track.h"
#include "track/track_decl.h"
#include "track/trackfile.h"
#include "track/trackid.h"
#include "track/trackref.h"
#include "util/db/dbid.h"

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

    QList<RelocatedTrack> relocatedTracks;
    QStringList addedTracks(newFile.location());
    bool cancel = false;
    trackDAO.detectMovedTracks(&relocatedTracks, addedTracks, &cancel);

    QSet<TrackId> updatedTrackIds;
    QSet<TrackId> removedTrackIds;
    for (const auto& relocatedTrack : qAsConst(relocatedTracks)) {
        updatedTrackIds.insert(relocatedTrack.updatedTrackRef().getId());
        removedTrackIds.insert(relocatedTrack.deletedTrackId());
    }

    EXPECT_THAT(updatedTrackIds, UnorderedElementsAre(oldId));
    EXPECT_THAT(removedTrackIds, UnorderedElementsAre(newId));

    QSet<QString> trackLocations = trackDAO.getAllTrackLocations();
    EXPECT_THAT(trackLocations, UnorderedElementsAre(newFile.location(), otherFile.location()));
}
