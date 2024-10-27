#include "library/trackset/smarties/smartiestablemodel.h"

#include <QtDebug>

#include "library/dao/trackschema.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "library/trackset/smarties/smarties.h"
#include "moc_smartiestablemodel.cpp"
#include "track/track.h"
#include "util/db/fwdsqlquery.h"

// EVE
#include "library/trackset/smarties/smartiesschema.h"
#include "library/trackset/smarties/smartiesstorage.h"
// #include "util/db/fwdsqlqueryselectresult.cpp"
#include "util/logger.h"
// EVE

namespace {

const QString kModelName = QStringLiteral("smarties");

} // anonymous namespace

SmartiesTableModel::SmartiesTableModel(
        QObject* pParent,
        TrackCollectionManager* pTrackCollectionManager)
        : TrackSetTableModel(
                  pParent,
                  pTrackCollectionManager,
                  "mixxx.db.model.smarties") {
}

void SmartiesTableModel::selectSmarties(SmartiesId smartiesId) {
    // qDebug() << "SmartiesTableModel::setSmarties()" << smartiesId;
    if (smartiesId == m_selectedSmarties) {
        qDebug() << "Already focused on smarties " << smartiesId;
        return;
    }
    // Store search text
    QString currSearch = currentSearch();
    if (m_selectedSmarties.isValid()) {
        if (!currSearch.trimmed().isEmpty()) {
            m_searchTexts.insert(m_selectedSmarties, currSearch);
        } else {
            m_searchTexts.remove(m_selectedSmarties);
        }
    }

    m_selectedSmarties = smartiesId;

    QString tableName = QStringLiteral("smarties_%1").arg(m_selectedSmarties.toString());
    QStringList columns;
    columns << LIBRARYTABLE_ID
            << "'' AS " + LIBRARYTABLE_PREVIEW
            // For sorting the cover art column we give LIBRARYTABLE_COVERART
            // the same value as the cover digest.
            << LIBRARYTABLE_COVERART_DIGEST + " AS " + LIBRARYTABLE_COVERART;

    QSqlQuery* queryGetLocked = new QSqlQuery(m_database);
    queryGetLocked->prepare("SELECT locked from smarties where id=:id");
    queryGetLocked->addBindValue(smartiesId.toVariant());
    queryGetLocked->exec();
    queryGetLocked->next();
    bool getLocked = queryGetLocked->value(0).toInt();
    qDebug() << "queryGetLocked " << queryGetLocked;
    //    qDebug() << "searchValue " << searchValue;
    queryGetLocked->clear();

    if (getLocked) {
        qDebug() << "SMARTIES LOCKED ";
        QString queryStringTempView =
                QString("CREATE TEMPORARY VIEW IF NOT EXISTS %1 AS "
                        "SELECT %2 FROM %3 "
                        "WHERE %4 IN (%5) "
                        "AND %6=0")
                        .arg(tableName,            // 1
                                columns.join(","), // 2
                                LIBRARY_TABLE,     // 3
                                LIBRARYTABLE_ID,   // 4
                                SmartiesStorage::formatSubselectQueryForSmartiesTrackIds(
                                        smartiesId),        // 5
                                LIBRARYTABLE_MIXXXDELETED); // 6
        qDebug() << "queryStringTempView " << queryStringTempView;
        FwdSqlQuery(m_database, queryStringTempView).execPrepared();
    } else {
        qDebug() << "SMARTIES NOT LOCKED ";
        QSqlQuery* queryGetSearchValue = new QSqlQuery(m_database);
        queryGetSearchValue->prepare("SELECT search_sql from smarties where id=:id");
        queryGetSearchValue->addBindValue(smartiesId.toVariant());

        queryGetSearchValue->exec();
        queryGetSearchValue->next();
        QString searchValue = queryGetSearchValue->value(0).toString();
        qDebug() << "queryGetSearchValue " << queryGetSearchValue;
        qDebug() << "searchValue " << searchValue;
        queryGetSearchValue->clear();

        QString queryStringDeleteIDFromSmartiesTracks = QString(
                "DELETE FROM smarties_tracks "
                "WHERE smarties_id = " +
                smartiesId.toVariant().toString());

        qDebug() << "queryStringDeleteIDFromSmartiesTracks "
                 << queryStringDeleteIDFromSmartiesTracks;
        FwdSqlQuery(m_database, queryStringDeleteIDFromSmartiesTracks).execPrepared();

        QString queryStringIDtoSmartiesTracks = QString(
                "INSERT OR IGNORE INTO smarties_tracks (smarties_id, track_id) "
                "SELECT " +
                smartiesId.toVariant().toString() +
                ", library.id FROM library "
                //        "SELECT :smartiesId, library.id FROM library "
                "WHERE library.artist like '%" +
                searchValue + "%'OR library.title like '%" + searchValue + "%'");

        qDebug() << "queryStringIDtoSmartiesTracks " << queryStringIDtoSmartiesTracks;
        FwdSqlQuery(m_database, queryStringIDtoSmartiesTracks).execPrepared();

        QString queryStringTempView =
                QString("CREATE TEMPORARY VIEW IF NOT EXISTS %1 AS "
                        "SELECT %2 FROM %3 "
                        "Where %3.artist like '%" +
                        searchValue + "%'OR %3.title like '%" + searchValue + "%'")
                        .arg(tableName,            // 1
                                columns.join(","), // 2
                                LIBRARY_TABLE);    // 3

        qDebug() << "queryStringTempView " << queryStringTempView;
        FwdSqlQuery(m_database, queryStringTempView).execPrepared();
    }

    columns[0] = LIBRARYTABLE_ID;
    columns[1] = LIBRARYTABLE_PREVIEW;
    columns[2] = LIBRARYTABLE_COVERART;

    setTable(tableName,
            LIBRARYTABLE_ID,
            columns,
            m_pTrackCollectionManager->internalCollection()->getTrackSource());

    // Restore search text
    setSearch(m_searchTexts.value(m_selectedSmarties));
    setDefaultSort(fieldIndex("artist"), Qt::AscendingOrder);
}

// bool SmartiesTableModel::addTrack(const QModelIndex& index, const QString& location) {
//     Q_UNUSED(index);

// This will only succeed if the file actually exist.
//    mixxx::FileInfo fileInfo(location);
//    if (!fileInfo.checkFileExists()) {
//        qDebug() << "SmartiesTableModel::addTrack:"
//                 << "File" << location << "not found";
//        return false;
//    }

// If a track is dropped but it isn't in the library, then add it because
// the user probably dropped a file from outside Mixxx into this smarties.
// If the track is already contained in the library it will not insert
// a duplicate. It also handles unremoving logic if the track has been
// removed from the library recently and re-adds it.
//    const TrackPointer pTrack = m_pTrackCollectionManager->getOrAddTrack(
//            TrackRef::fromFileInfo(fileInfo));
//    if (!pTrack) {
//        qDebug() << "SmartiesTableModel::addTrack:"
//                 << "Failed to add track" << location << "to library";
//        return false;
//    }

//    QList<TrackId> trackIds;
//    trackIds.append(pTrack->getId());
//    if (!m_pTrackCollectionManager->internalCollection()->addSmartiesTracks(
//                m_selectedSmarties, trackIds)) {
//        qDebug() << "SmartiesTableModel::addTrack:"
//                 << "Failed to add track" << location << "to smarties"
//                 << m_selectedSmarties;
//        return false;
//    }

// TODO(rryan) just add the track don't select
//    select();
//    return true;
//}

TrackModel::Capabilities SmartiesTableModel::getCapabilities() const {
    Capabilities caps =
            //  Capability::ReceiveDrops |
            Capability::AddToTrackSet |
            Capability::AddToAutoDJ |
            Capability::EditMetadata |
            Capability::LoadToDeck |
            Capability::LoadToSampler |
            Capability::LoadToPreviewDeck |
            Capability::RemoveSmarties |
            Capability::ResetPlayed |
            Capability::Hide |
            Capability::RemoveFromDisk |
            Capability::Analyze |
            Capability::Properties;

    if (m_selectedSmarties.isValid()) {
        Smarties smarties;
        if (m_pTrackCollectionManager->internalCollection()
                        ->smarties()
                        .readSmartiesById(m_selectedSmarties, &smarties)) {
            //            if (smarties.isLocked()) {
            //                caps |= Capability::Locked;
            //            }
        } else {
            qWarning() << "Failed to read smarties" << m_selectedSmarties;
        }
    }

    return caps;
}

// int SmartiesTableModel::addTracksWithTrackIds(
//         const QModelIndex& index, const QList<TrackId>& trackIds, int* pOutInsertionPos) {
//     Q_UNUSED(index);

//    if (pOutInsertionPos != nullptr) {
// smarties insertion is not done by position, and no duplicates will be added,.
// 0 indicates this to the caller.
//        *pOutInsertionPos = 0;
//    }

// If a track is dropped but it isn't in the library, then add it because
// the user probably dropped a file from outside Mixxx into this smarties.
//    if (!m_pTrackCollectionManager->internalCollection()->addSmartiesTracks(
//                m_selectedSmarties, trackIds)) {
//        qWarning() << "SmartiesTableModel::addTracks could not add"
//                   << trackIds.size() << "tracks to smarties" << m_selectedSmarties;
//        return 0;
//    }

//    select();
//    return trackIds.size();
//}

// bool SmartiesTableModel::isLocked() {
//     Smarties smarties;
//     if (!m_pTrackCollectionManager->internalCollection()
//                     ->smarties()
//                     .readSmartiesById(m_selectedSmarties, &smarties)) {
//         qWarning() << "Failed to read smarties" << m_selectedSmarties;
//         return false;
//     }
//     return smarties.isLocked();
// }

// void SmartiesTableModel::removeTracks(const QModelIndexList& indices) {
//     VERIFY_OR_DEBUG_ASSERT(m_selectedSmarties.isValid()) {
//         return;
//     }
//     if (indices.empty()) {
//         return;
//     }

//    Smarties smarties;
//    if (!m_pTrackCollectionManager->internalCollection()
//                    ->smarties()
//                    .readSmartiesById(m_selectedSmarties, &smarties)) {
//        qWarning() << "Failed to read smarties" << m_selectedSmarties;
//        return;
//    }

//    VERIFY_OR_DEBUG_ASSERT(!smarties.isLocked()) {
//        return;
//    }

//    QList<TrackId> trackIds;
//    trackIds.reserve(indices.size());
//    for (const QModelIndex& index : indices) {
//        trackIds.append(getTrackId(index));
//    }
//    if (!m_pTrackCollectionManager->internalCollection()->removeSmartiesTracks(
//                smarties.getId(), trackIds)) {
//        qWarning() << "Failed to remove tracks from smarties" << smarties;
//        return;
//    }

//    select();
//}

QString SmartiesTableModel::modelKey(bool noSearch) const {
    if (m_selectedSmarties.isValid()) {
        if (noSearch) {
            return kModelName + QChar(':') + m_selectedSmarties.toString();
        }
        return kModelName + QChar(':') +
                m_selectedSmarties.toString() +
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
