
#include "library/crate/cratetablemodel.h"

#include "library/dao/trackschema.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "mixer/playermanager.h"
#include "util/db/fwdsqlquery.h"

#include <QtDebug>

CrateTableModel::CrateTableModel(QObject* pParent,
                                 TrackCollectionManager* pTrackCollectionManager)
        : BaseSqlTableModel(pParent, pTrackCollectionManager,
                            "mixxx.db.model.crate") {
}

CrateTableModel::~CrateTableModel() {
}

void CrateTableModel::selectCrate(CrateId crateId) {
    //qDebug() << "CrateTableModel::setCrate()" << crateId;
    if (crateId == m_selectedCrate) {
        qDebug() << "Already focused on crate " << crateId;
        return;
    }
    m_selectedCrate = crateId;

    QString tableName = QString("crate_%1").arg(m_selectedCrate.toString());
    QStringList columns;
    columns << LIBRARYTABLE_ID
            << "'' AS " + LIBRARYTABLE_PREVIEW
            // For sorting the cover art column we give LIBRARYTABLE_COVERART
            // the same value as the cover hash.
            << LIBRARYTABLE_COVERART_HASH + " AS " + LIBRARYTABLE_COVERART;
    // We hide files that have been explicitly deleted in the library
    // (mixxx_deleted = 0) from the view.
    // They are kept in the database, because we treat crate membership as a
    // track property, which persist over a hide / unhide cycle.
    QString queryString = QString("CREATE TEMPORARY VIEW IF NOT EXISTS %1 AS "
                                  "SELECT %2 FROM %3 "
                                  "WHERE %4 IN (%5) "
                                  "AND %6=0")
                          .arg(tableName,
                               columns.join(","),
                               LIBRARY_TABLE,
                               LIBRARYTABLE_ID,
                               CrateStorage::formatSubselectQueryForCrateTrackIds(crateId),
                               LIBRARYTABLE_MIXXXDELETED);
    FwdSqlQuery(m_database, queryString).execPrepared();

    columns[0] = LIBRARYTABLE_ID;
    columns[1] = LIBRARYTABLE_PREVIEW;
    columns[2] = LIBRARYTABLE_COVERART;
    setTable(tableName, LIBRARYTABLE_ID, columns,
             m_pTrackCollectionManager->internalCollection()->getTrackSource());
    setSearch("");
    setDefaultSort(fieldIndex("artist"), Qt::AscendingOrder);
}

bool CrateTableModel::addTrack(const QModelIndex& index, QString location) {
    Q_UNUSED(index);

    // This will only succeed if the file actually exist.
    TrackFile fileInfo(location);
    if (!fileInfo.checkFileExists()) {
        qDebug() << "CrateTableModel::addTrack:"
                << "File"
                << location
                << "not found";
        return false;
    }

    // If a track is dropped but it isn't in the library, then add it because
    // the user probably dropped a file from outside Mixxx into this crate.
    // If the track is already contained in the library it will not insert
    // a duplicate. It also handles unremoving logic if the track has been
    // removed from the library recently and re-adds it.
    const TrackPointer pTrack = m_pTrackCollectionManager->getOrAddTrack(
            TrackRef::fromFileInfo(fileInfo));
    if (!pTrack) {
        qDebug() << "CrateTableModel::addTrack:"
                << "Failed to add track"
                << location
                << "to library";
        return false;
    }

    QList<TrackId> trackIds;
    trackIds.append(pTrack->getId());
    if (!m_pTrackCollectionManager->internalCollection()->addCrateTracks(m_selectedCrate, trackIds)) {
        qDebug() << "CrateTableModel::addTrack:"
                << "Failed to add track"
                << location
                << "to crate"
                << m_selectedCrate;
        return false;
    }

    // TODO(rryan) just add the track don't select
    select();
    return true;
}

TrackModel::CapabilitiesFlags CrateTableModel::getCapabilities() const {
    CapabilitiesFlags caps =  TRACKMODELCAPS_NONE
            | TRACKMODELCAPS_RECEIVEDROPS
            | TRACKMODELCAPS_ADDTOPLAYLIST
            | TRACKMODELCAPS_ADDTOCRATE
            | TRACKMODELCAPS_ADDTOAUTODJ
            | TRACKMODELCAPS_EDITMETADATA
            | TRACKMODELCAPS_LOADTODECK
            | TRACKMODELCAPS_LOADTOSAMPLER
            | TRACKMODELCAPS_LOADTOPREVIEWDECK
            | TRACKMODELCAPS_REMOVE_CRATE
            | TRACKMODELCAPS_RESETPLAYED;
    if (m_selectedCrate.isValid()) {
        Crate crate;
        if (m_pTrackCollectionManager->internalCollection()->crates().readCrateById(m_selectedCrate, &crate)) {
            if (crate.isLocked()) {
                caps |= TRACKMODELCAPS_LOCKED;
            }
        } else {
            qWarning() << "Failed to read create" << m_selectedCrate;
        }
    }
    return caps;
}

bool CrateTableModel::isColumnInternal(int column) {
    return column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ID) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PLAYED) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_MIXXXDELETED) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM_LOCK) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_KEY_ID)||
            column == fieldIndex(ColumnCache::COLUMN_TRACKLOCATIONSTABLE_FSDELETED) ||
            (PlayerManager::numPreviewDecks() == 0 &&
             column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PREVIEW)) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_SOURCE) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_TYPE) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_LOCATION) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_HASH);;
}

int CrateTableModel::addTracks(const QModelIndex& index,
                               const QList<QString>& locations) {
    Q_UNUSED(index);
    // If a track is dropped but it isn't in the library, then add it because
    // the user probably dropped a file from outside Mixxx into this crate.
    QList<TrackId> trackIds = m_pTrackCollectionManager->internalCollection()->resolveTrackIdsFromLocations(
            locations);
    if (!m_pTrackCollectionManager->internalCollection()->addCrateTracks(m_selectedCrate, trackIds)) {
        qWarning() << "CrateTableModel::addTracks could not add"
                 << locations.size()
                 << "tracks to crate" << m_selectedCrate;
        return 0;
    }

    select();
    return trackIds.size();
}

void CrateTableModel::removeTracks(const QModelIndexList& indices) {
    VERIFY_OR_DEBUG_ASSERT(m_selectedCrate.isValid()) {
        return;
    }
    if (indices.empty()) {
        return;
    }

    Crate crate;
    if (!m_pTrackCollectionManager->internalCollection()->crates().readCrateById(m_selectedCrate, &crate)) {
        qWarning() << "Failed to read create" << m_selectedCrate;
        return;
    }

    VERIFY_OR_DEBUG_ASSERT(!crate.isLocked()) {
        return;
    }

    QList<TrackId> trackIds;
    trackIds.reserve(indices.size());
    for (const QModelIndex& index: indices) {
        trackIds.append(getTrackId(index));
    }
    if (!m_pTrackCollectionManager->internalCollection()->removeCrateTracks(crate.getId(), trackIds)) {
        qWarning() << "Failed to remove tracks from crate" << crate;
        return;
    }

    select();
}
