#include "library/trackset/crate/cratetablemodel.h"

#include <QtDebug>

#include "library/dao/trackschema.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "library/trackset/crate/crate.h"
#include "library/trackset/crate/crateschema.h"
#include "moc_cratetablemodel.cpp"
#include "track/track.h"
#include "util/db/fwdsqlquery.h"

namespace {

const QString kModelName = QStringLiteral("crate");

} // anonymous namespace

CrateTableModel::CrateTableModel(
        QObject* pParent,
        TrackCollectionManager* pTrackCollectionManager)
        : TrackSetTableModel(
                  pParent,
                  pTrackCollectionManager,
                  "mixxx.db.model.crate") {
}

void CrateTableModel::selectCrate(CrateId crateId) {
    //qDebug() << "CrateTableModel::setCrate()" << crateId;
    if (crateId == m_selectedCrate) {
        qDebug() << "Already focused on crate " << crateId;
        return;
    }
    // Store search text
    QString currSearch = currentSearch();
    if (m_selectedCrate.isValid()) {
        if (!currSearch.trimmed().isEmpty()) {
            m_searchTexts.insert(m_selectedCrate, currSearch);
        } else {
            m_searchTexts.remove(m_selectedCrate);
        }
    }

    m_selectedCrate = crateId;

    QString tableName = QStringLiteral("crate_%1").arg(m_selectedCrate.toString());
    QStringList columns;
    columns << LIBRARYTABLE_ID
            << "'' AS " + LIBRARYTABLE_PREVIEW
            // For sorting the cover art column we give LIBRARYTABLE_COVERART
            // the same value as the cover digest.
            << LIBRARYTABLE_COVERART_DIGEST + " AS " + LIBRARYTABLE_COVERART;
    // We hide files that have been explicitly deleted in the library
    // (mixxx_deleted = 0) from the view.
    // They are kept in the database, because we treat crate membership as a
    // track property, which persist over a hide / unhide cycle.
    QString queryString =
            QString("CREATE TEMPORARY VIEW IF NOT EXISTS %1 AS "
                    "SELECT %2 FROM %3 "
                    "WHERE %4 IN (%5) "
                    "AND %6=0")
                    .arg(tableName,
                            columns.join(","),
                            LIBRARY_TABLE,
                            LIBRARYTABLE_ID,
                            CrateStorage::formatSubselectQueryForCrateTrackIds(
                                    crateId),
                            LIBRARYTABLE_MIXXXDELETED);
    FwdSqlQuery(m_database, queryString).execPrepared();

    columns[0] = LIBRARYTABLE_ID;
    columns[1] = LIBRARYTABLE_PREVIEW;
    columns[2] = LIBRARYTABLE_COVERART;
    setTable(tableName,
            LIBRARYTABLE_ID,
            columns,
            m_pTrackCollectionManager->internalCollection()->getTrackSource());

    // Restore search text
    setSearch(m_searchTexts.value(m_selectedCrate));
    setDefaultSort(fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ARTIST), Qt::AscendingOrder);
}

bool CrateTableModel::addTrack(const QModelIndex& index, const QString& location) {
    Q_UNUSED(index);

    // This will only succeed if the file actually exist.
    mixxx::FileInfo fileInfo(location);
    if (!fileInfo.checkFileExists()) {
        qDebug() << "CrateTableModel::addTrack:"
                 << "File" << location << "not found";
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
                 << "Failed to add track" << location << "to library";
        return false;
    }

    QList<TrackId> trackIds;
    trackIds.append(pTrack->getId());
    if (!m_pTrackCollectionManager->internalCollection()->addCrateTracks(
                m_selectedCrate, trackIds)) {
        qDebug() << "CrateTableModel::addTrack:"
                 << "Failed to add track" << location << "to crate"
                 << m_selectedCrate;
        return false;
    }

    // TODO(rryan) just add the track don't select
    select();
    return true;
}

TrackModel::Capabilities CrateTableModel::getCapabilities() const {
    Capabilities caps =
            Capability::ReceiveDrops |
            Capability::AddToTrackSet |
            Capability::AddToAutoDJ |
            Capability::EditMetadata |
            Capability::LoadToDeck |
            Capability::LoadToSampler |
            Capability::LoadToPreviewDeck |
            Capability::RemoveCrate |
            Capability::ResetPlayed |
            Capability::Hide |
            Capability::RemoveFromDisk |
            Capability::Analyze |
            Capability::Properties |
            Capability::Sorting;

    if (m_selectedCrate.isValid()) {
        Crate crate;
        if (m_pTrackCollectionManager->internalCollection()
                        ->crates()
                        .readCrateById(m_selectedCrate, &crate)) {
            if (crate.isLocked()) {
                caps |= Capability::Locked;
            }
        } else {
            qWarning() << "Failed to read create" << m_selectedCrate;
        }
    }

    return caps;
}

int CrateTableModel::addTracksWithTrackIds(
        const QModelIndex& index, const QList<TrackId>& trackIds, int* pOutInsertionPos) {
    Q_UNUSED(index);

    if (pOutInsertionPos != nullptr) {
        // crate insertion is not done by position, and no duplicates will be added,.
        // 0 indicates this to the caller.
        *pOutInsertionPos = 0;
    }

    // If a track is dropped but it isn't in the library, then add it because
    // the user probably dropped a file from outside Mixxx into this crate.
    if (!m_pTrackCollectionManager->internalCollection()->addCrateTracks(
                m_selectedCrate, trackIds)) {
        qWarning() << "CrateTableModel::addTracks could not add"
                   << trackIds.size() << "tracks to crate" << m_selectedCrate;
        return 0;
    }

    select();
    return trackIds.size();
}

bool CrateTableModel::isLocked() {
    Crate crate;
    if (!m_pTrackCollectionManager->internalCollection()
                    ->crates()
                    .readCrateById(m_selectedCrate, &crate)) {
        qWarning() << "Failed to read create" << m_selectedCrate;
        return false;
    }
    return crate.isLocked();
}

void CrateTableModel::removeTracks(const QModelIndexList& indices) {
    VERIFY_OR_DEBUG_ASSERT(m_selectedCrate.isValid()) {
        return;
    }
    if (indices.empty()) {
        return;
    }

    Crate crate;
    if (!m_pTrackCollectionManager->internalCollection()
                    ->crates()
                    .readCrateById(m_selectedCrate, &crate)) {
        qWarning() << "Failed to read create" << m_selectedCrate;
        return;
    }

    VERIFY_OR_DEBUG_ASSERT(!crate.isLocked()) {
        return;
    }

    QList<TrackId> trackIds;
    trackIds.reserve(indices.size());
    for (const QModelIndex& index : indices) {
        trackIds.append(getTrackId(index));
    }
    if (!m_pTrackCollectionManager->internalCollection()->removeCrateTracks(
                crate.getId(), trackIds)) {
        qWarning() << "Failed to remove tracks from crate" << crate;
        return;
    }

    select();
}

QString CrateTableModel::modelKey(bool noSearch) const {
    if (m_selectedCrate.isValid()) {
        if (noSearch) {
            return kModelName + QChar(':') + m_selectedCrate.toString();
        }
        return kModelName + QChar(':') +
                m_selectedCrate.toString() +
                QChar('#') +
                currentSearch();
    } else {
        if (noSearch) {
            return kModelName;
        }
        return kModelName + QChar('#') +
                currentSearch();
    }
}
