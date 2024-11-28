#include "library/trackset/smarties/smartiestablemodel.h"

#include <QSqlError>
#include <QtDebug>

#include "library/dao/trackschema.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "library/trackset/smarties/smarties.h"
#include "moc_smartiestablemodel.cpp"
#include "track/track.h"
#include "util/db/fwdsqlquery.h"

// EVE
#include "library/trackset/smarties/smartiesfuntions.h"
#include "library/trackset/smarties/smartiesschema.h"
#include "library/trackset/smarties/smartiesstorage.h"
// #include "util/db/fwdsqlqueryselectresult.cpp"
#include "util/logger.h"
// EVE
// const bool sDebug = false;

namespace {

const QString kModelName = QStringLiteral("smarties");

} // anonymous namespace

SmartiesTableModel::SmartiesTableModel(
        QObject* pParent,
        TrackCollectionManager* pTrackCollectionManager,
        TrackCollection* pTrackCollection)
        : TrackSetTableModel(
                  pParent,
                  pTrackCollectionManager,
                  "mixxx.db.model.smarties"),
          m_pTrackCollection(pTrackCollection) {
}

void SmartiesTableModel::selectSmarties(SmartiesId smartiesId) {
    // qDebug() << "SmartiesTableModel::setSmarties()" << smartiesId;
    if (smartiesId == m_selectedSmarties) {
        if (sDebug) {
            qDebug() << "[SMARTIESTABLEMODEL] [SELECT] -> Already focused on "
                        "smarties "
                     << smartiesId;
        }
        //        return;
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

    QString checkStamp = QDateTime::currentDateTime().toString("hhmmss");
    QString tableName = QStringLiteral("smarties_%1").arg(m_selectedSmarties.toString());
    QString tableNameOld =
            QStringLiteral("smarties_%1_%2")
                    .arg(m_selectedSmarties.toString(), checkStamp);
    QStringList columns;
    columns << LIBRARYTABLE_ID
            << "'' AS " + LIBRARYTABLE_PREVIEW
            // For sorting the cover art column we give LIBRARYTABLE_COVERART
            // the same value as the cover digest.
            << LIBRARYTABLE_COVERART_DIGEST + " AS " + LIBRARYTABLE_COVERART;

    bool getLocked;
    FwdSqlQuery queryGetLocked(m_database,
            QStringLiteral("SELECT locked from smarties where id=:smartiesId"));
    queryGetLocked.bindValue(":smartiesId", smartiesId);
    if (sDebug) {
        qDebug() << "[SMARTIESTABLEMODEL] [SELECT] -> LOCKED ? -> get locked: queryGetLocked "
                 << "SELECT locked from smarties where id = "
                 << smartiesId;
    }

    if (queryGetLocked.execPrepared() && queryGetLocked.next()) {
        getLocked = queryGetLocked.fieldValue(0).toBool();
    } else {
        getLocked = false;
    }
    if (sDebug) {
        qDebug() << "[SMARTIESTABLEMODEL] [SELECT] -> LOCKED ? -> locked: " << getLocked;
    }
    if (getLocked) {
        // read cache = tracks from smarties_tracks
        if (sDebug) {
            qDebug() << "[SMARTIESTABLEMODEL] [SELECT] -> LOCKED -> GET CACHED TRACKS ";
        }
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
        if (sDebug) {
            qDebug() << "[SMARTIESTABLEMODEL] [SELECT] -> LOCKED -> GET CACHED TRACKS "
                        "queryStringTempView "
                     << queryStringTempView;
        }
        FwdSqlQuery(m_database, queryStringTempView).execPrepared();
    } else {
        if (sDebug) {
            qDebug() << "[SMARTIESTABLEMODEL] [SELECT] -> NOT LOCKED";
        }
        // delete cache = delete tracks in smarties_tracks table witg selected id
        QString queryStringDeleteIDFromSmartiesTracks = QString(
                "DELETE FROM smarties_tracks "
                "WHERE smarties_id = " +
                smartiesId.toVariant().toString());
        if (sDebug) {
            qDebug() << "[SMARTIESTABLEMODEL] [SELECT] -> NOT LOCKED -> DELETE CACHED TRACKS "
                        "queryStringDeleteIDFromSmartiesTracks "
                     << queryStringDeleteIDFromSmartiesTracks;
        }
        FwdSqlQuery(m_database, queryStringDeleteIDFromSmartiesTracks).execPrepared();

        // Create SQl based on conditions in smarties
        QVariantList smartiesData;
        smartiesData.clear();
        if (sDebug) {
            qDebug() << "[SMARTIESTABLEMODEL] [SELECT] -> NOT LOCKED -> "
                        "CONSTRUCT SQL -> selectSmarties2QVL ";
        }
        selectSmarties2QVL(smartiesId, smartiesData); // Fetch smarties data
        if (sDebug) {
            qDebug() << "[SMARTIESTABLEMODEL] [SELECT] -> NOT LOCKED -> "
                        "CONSTRUCT SQL -> whereClause ";
        }
        QString whereClause = buildWhereClause(smartiesData); // Get the WHERE clause
        if (sDebug) {
            qDebug() << "[SMARTIESTABLEMODEL] [SELECT] -> NOT LOCKED -> "
                        "CONSTRUCT SQL -> whereClause "
                        "generated:"
                     << whereClause;
        }
        // create cache = put tracks in smarties_tracks table witg selected id
        QString queryStringIDtoSmartiesTracks = QString(
                "INSERT OR IGNORE INTO smarties_tracks (smarties_id, track_id) "
                "SELECT " +
                smartiesId.toVariant().toString() +
                ", library.id FROM library "
                //        "SELECT :smartiesId, library.id FROM library "
                "WHERE " +
                whereClause);
        FwdSqlQuery(m_database, queryStringIDtoSmartiesTracks).execPrepared();
        if (sDebug) {
            qDebug() << "[SMARTIESTABLEMODEL] [SELECT] -> NOT LOCKED -> CREATE "
                        "CACHE -> Create temp "
                        "view queryStringIDtoSmartiesTracks "
                     << queryStringIDtoSmartiesTracks;
        }
        // QString queryStringDropView = QString("DROP VIEW IF EXISTS %1 ").arg(tableName);
        QString queryStringDropView = QString("Alter table rename %1 to %2 ")
                                              .arg(tableName, tableNameOld);
        FwdSqlQuery(m_database, queryStringIDtoSmartiesTracks).execPrepared();
        // qDebug() << "[SMARTIESTABLEMODEL] [NOT LOCKED] [CREATE CACHE] -> Drop view "
        if (sDebug) {
            qDebug() << "[SMARTIESTABLEMODEL] [SELECT] -> NOT LOCKED -> CREATE "
                        "CACHE -> Rename TEMP table"
                        "queryStringDropView "
                     << queryStringDropView;
        }

        QString queryStringTempView =
                QString("CREATE TEMPORARY VIEW IF NOT EXISTS %1 AS "
                        "SELECT %2 FROM %3 "
                        "WHERE " +
                        whereClause)
                        .arg(tableName,            // 1
                                columns.join(","), // 2
                                LIBRARY_TABLE);    // 3

        if (sDebug) {
            qDebug() << "[SMARTIESTABLEMODEL] [SELECT] -> NOT LOCKED -> CREATE "
                        "CACHE -> CREATE TEMP VIEW "
                        "queryStringTempView "
                     << queryStringTempView;
        }
        FwdSqlQuery(m_database, queryStringTempView).execPrepared();
    }

    columns[0] = LIBRARYTABLE_ID;
    columns[1] = LIBRARYTABLE_PREVIEW;
    columns[2] = LIBRARYTABLE_COVERART;

    if (sDebug) {
        qDebug() << "[SMARTIESTABLEMODEL] [SELECT] -> LOCKED / NOT LOCKED -> LOAD TRACKS IN TABLE";
    }
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
            //            Capability::RemoveSmarties |
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
            qWarning() << "[SMARTIESTABLEMODEL] Failed to read smarties" << m_selectedSmarties;
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

void SmartiesTableModel::selectPlaylistsCrates2QVL(QVariantList& playlistsCratesData) {
    if (sDebug) {
        qDebug() << "[SMARTIESTABLEMODEL] [SELECT PLAYLISTS CRATES 2 QVL] -> Start";
    }
    playlistsCratesData.clear();

    // Playlists
    QSqlQuery playlistQuery(m_database);
    playlistQuery.prepare(
            "SELECT id, name FROM Playlists WHERE hidden=0 ORDER BY name ASC, "
            "id ASC");
    if (playlistQuery.exec()) {
        while (playlistQuery.next()) {
            QVariantMap playlistEntry;
            playlistEntry["type"] = "playlist";
            playlistEntry["id"] = playlistQuery.value("id");
            playlistEntry["name"] = playlistQuery.value("name");
            playlistsCratesData.append(playlistEntry);
        }
    } else {
        qWarning() << "[SMARTIESTABLEMODEL] [SELECT PLAYLISTS CRATES 2 QVL] -> "
                      "Playlists Failed:"
                   << playlistQuery.lastError();
    }

    // Playlists - history
    QSqlQuery historyQuery(m_database);
    historyQuery.prepare(
            "SELECT id, name FROM Playlists WHERE hidden=2 ORDER BY name DESC, "
            "id ASC");
    if (historyQuery.exec()) {
        while (historyQuery.next()) {
            QVariantMap historyEntry;
            historyEntry["type"] = "history";
            historyEntry["id"] = historyQuery.value("id");
            historyEntry["name"] = historyQuery.value("name");
            playlistsCratesData.append(historyEntry);
        }
    } else {
        qWarning() << "[SMARTIESTABLEMODEL] [SELECT PLAYLISTS CRATES 2 QVL] -> "
                      "History Failed:"
                   << historyQuery.lastError();
    }

    // Crates
    QSqlQuery crateQuery(m_database);
    crateQuery.prepare("SELECT id, name FROM crates WHERE show=1 ORDER BY name ASC, id ASC");
    if (crateQuery.exec()) {
        while (crateQuery.next()) {
            QVariantMap crateEntry;
            crateEntry["type"] = "crate";
            crateEntry["id"] = crateQuery.value("id");
            crateEntry["name"] = crateQuery.value("name");
            playlistsCratesData.append(crateEntry);
        }
    } else {
        qWarning() << "[SMARTIESTABLEMODEL] [SELECT PLAYLISTS CRATES 2 QVL] -> "
                      "Crates Failed:"
                   << crateQuery.lastError();
    }

    if (sDebug) {
        qDebug() << "[SMARTIESTABLEMODEL] [SELECT PLAYLISTS CRATES 2 QVL] -> "
                    "Completed:"
                 << playlistsCratesData;
    }
}

void SmartiesTableModel::selectSmarties2QVL(SmartiesId smartiesId, QVariantList& smartiesData) {
    if (sDebug) {
        qDebug() << "[SMARTIESTABLEMODEL] [SELECTSMARTIES2QVL] -> start with "
                    "SmartiesId:"
                 << smartiesId;
    }

    // Assuming m_database is properly connected
    QSqlQuery* query = new QSqlQuery(m_database);
    query->prepare("SELECT * FROM smarties WHERE id = :id");
    query->addBindValue(smartiesId.toVariant());

    if (query->exec()) {
        if (query->next()) {
            smartiesData.clear(); // Clear any existing data before appending

            // Populate smartiesData with the fields from the database row
            smartiesData.append(query->value("id").toString());           // id
            smartiesData.append(query->value("name").toString());         // name
            smartiesData.append(query->value("count").toInt());           // count
            smartiesData.append(query->value("show").toBool());           // show
            smartiesData.append(query->value("locked").toBool());         // locked
            smartiesData.append(query->value("autodj_source").toBool());  // autoDJ
            smartiesData.append(query->value("search_input").toString()); // search_input
            smartiesData.append(query->value("search_sql").toString());   // search_sql

            for (int i = 1; i <= 12; ++i) { // Handle conditions
                smartiesData.append(query->value(QString("condition%1_field").arg(i)).toString());
                smartiesData.append(
                        query->value(QString("condition%1_operator").arg(i))
                                .toString());
                smartiesData.append(query->value(QString("condition%1_value").arg(i)).toString());
                smartiesData.append(
                        query->value(QString("condition%1_combiner").arg(i))
                                .toString());
            }

            if (sDebug) {
                qDebug() << "[SMARTIESTABLEMODEL] [SELECTSMARTIES2QVL] -> loaded data into "
                            "QVariantList:"
                         << smartiesData;
            }
            // Retrieve previous and next record IDs
            //            QVariant previousId = getPreviousRecordId(smartiesId);
            //            QVariant nextId = getNextRecordId(smartiesId);

            // Append previous and next IDs to the QVariantList
            //            smartiesData.append(previousId); // Appending previous ID
            //            smartiesData.append(nextId);     // Appending next ID

            // Check if BOF and EOF
            //            bool isBOF = (previousId.isNull()); // If no previous ID, at beginning
            //            bool isEOF = (nextId.isNull());     // If no next ID, at end

            // Optionally append BOF and EOF flags
            //            smartiesData.append(isBOF);
            //            smartiesData.append(isEOF);
        } else {
            if (sDebug) {
                qDebug() << "[SMARTIESTABLEMODEL] [SELECTSMARTIES2QVL] -> No data found for "
                            "SmartiesId:"
                         << smartiesId;
            }
        }
    } else {
        if (sDebug) {
            qDebug() << "[SMARTIESTABLEMODEL] [SELECTSMARTIES2QVL] -> Failed to execute query -"
                     << query->lastError();
        }
    }
    delete query;
}

QVariant SmartiesTableModel::getPreviousRecordId(SmartiesId currentId) {
    QSqlQuery query(m_database);
    query.prepare("SELECT id FROM smarties WHERE id < :id ORDER BY id DESC LIMIT 1");
    query.bindValue(":id", currentId.toVariant());
    if (query.exec() && query.next()) {
        return query.value("id");
    }
    return {}; // Return a null QVariant if no previous ID
}

QVariant SmartiesTableModel::getNextRecordId(SmartiesId currentId) {
    QSqlQuery query(m_database);
    query.prepare("SELECT id FROM smarties WHERE id > :id ORDER BY id ASC LIMIT 1");
    query.bindValue(":id", currentId.toVariant());
    if (query.exec() && query.next()) {
        return query.value("id");
    }
    return {}; // Return a null QVariant if no next ID
}

void SmartiesTableModel::saveQVL2Smarties(SmartiesId smartiesId, const QVariantList& smartiesData) {
    if (sDebug) {
        qDebug() << "[SMARTIESTABLEMODEL] [SAVEQVL2SMARTIES] -> starts for ID:" << smartiesId;
        qDebug() << "[SMARTIESTABLEMODEL] [SAVEQVL2SMARTIES] -> UPDATE SQL WhereClause "
                 << buildWhereClause(smartiesData).replace("'", "");
    }
    QString whereClause2Save = buildWhereClause(smartiesData);
    if (sDebug) {
        qDebug() << "[SMARTIESTABLEMODEL] [SAVEQVL2SMARTIES] -> UPDATE SQL WhereClause "
                 << whereClause2Save.replace("'", "");
    }
    // Core update for basic fields
    QString baseInfoUpdate = QString(
            "UPDATE smarties SET name = '" + smartiesData[1].toString() +
            "', "
            "count = " +
            smartiesData[2].toString() +
            ", "
            "show = " +
            smartiesData[3].toString() +
            ", "
            "locked = " +
            smartiesData[4].toString() +
            ", "
            "autodj_source = " +
            smartiesData[5].toString() +
            ", "
            "search_input = '" +
            smartiesData[6].toString() +
            "', "
            //            "search_sql = '" +
            //            smartiesData[7].toString() +
            "search_sql = '" +
            whereClause2Save.replace("'", "") +
            "' "
            "WHERE id = " +
            smartiesData[0].toString());
    //    qDebug() << "UPDATE SQL WhereClause " <<
    //    buildWhereClause(smartiesData).replace("'", "\'");

    if (!FwdSqlQuery(m_database, baseInfoUpdate).execPrepared()) {
        if (sDebug) {
            qDebug() << "[SMARTIESTABLEMODEL] [SAVEQVL2SMARTIES] -> baseInfo Update failed:";
        }
        return;
    }

    baseInfoUpdate.clear(); // Unlock the database

    // Update for condition1 through condition3 fields
    QString conditionUpdate1 = buildConditionUpdateQuery(smartiesData, 8, 19);
    //    query.prepare(conditionUpdate1);
    if (!FwdSqlQuery(m_database, conditionUpdate1).execPrepared()) {
        if (sDebug) {
            qDebug() << "[SMARTIESTABLEMODEL] [SAVEQVL2SMARTIES] -> Condition Update 1 failed:";
        }
        return;
    }
    conditionUpdate1.clear();

    // Update for condition4 through condition6 fields
    QString conditionUpdate2 = buildConditionUpdateQuery(smartiesData, 20, 31);
    //    query.prepare(conditionUpdate2);
    if (!FwdSqlQuery(m_database, conditionUpdate2).execPrepared()) {
        if (sDebug) {
            qDebug() << "[SMARTIESTABLEMODEL] [SAVEQVL2SMARTIES] -> Condition Update 2 failed:";
        }
        return;
    }
    conditionUpdate2.clear();

    // Update for condition7 through condition9 fields
    QString conditionUpdate3 = buildConditionUpdateQuery(smartiesData, 32, 43);
    //    query.prepare(conditionUpdate3);
    if (!FwdSqlQuery(m_database, conditionUpdate3).execPrepared()) {
        if (sDebug) {
            qDebug() << "[SMARTIESTABLEMODEL] [SAVEQVL2SMARTIES] -> Condition Update 3 failed:";
        }
        return;
    }
    conditionUpdate3.clear();

    // Update for condition10 through condition12 fields
    QString conditionUpdate4 = buildConditionUpdateQuery(smartiesData, 44, 55);
    //    query.prepare(conditionUpdate4);
    if (!FwdSqlQuery(m_database, conditionUpdate4).execPrepared()) {
        if (sDebug) {
            qDebug() << "[SMARTIESTABLEMODEL] [SAVEQVL2SMARTIES] -> Condition Update 4 failed:";
        }
        return;
    }
    conditionUpdate4.clear();
    if (sDebug) {
        qDebug() << "[SMARTIESTABLEMODEL] [SAVEQVL2SMARTIES] -> completed for ID:" << smartiesId;
    }
}

QString SmartiesTableModel::buildConditionUpdateQuery(
        const QVariantList& smartiesData, int startIdx, int endIdx) {
    QString queryStr = "UPDATE smarties SET ";
    QStringList fieldNames = {"condition1_field",
            "condition1_operator",
            "condition1_value",
            "condition1_combiner",
            "condition2_field",
            "condition2_operator",
            "condition2_value",
            "condition2_combiner",
            "condition3_field",
            "condition3_operator",
            "condition3_value",
            "condition3_combiner",
            "condition4_field",
            "condition4_operator",
            "condition4_value",
            "condition4_combiner",
            "condition5_field",
            "condition5_operator",
            "condition5_value",
            "condition5_combiner",
            "condition6_field",
            "condition6_operator",
            "condition6_value",
            "condition6_combiner",
            "condition7_field",
            "condition7_operator",
            "condition7_value",
            "condition7_combiner",
            "condition8_field",
            "condition8_operator",
            "condition8_value",
            "condition8_combiner",
            "condition9_field",
            "condition9_operator",
            "condition9_value",
            "condition9_combiner",
            "condition10_field",
            "condition10_operator",
            "condition10_value",
            "condition10_combiner",
            "condition11_field",
            "condition11_operator",
            "condition11_value",
            "condition11_combiner",
            "condition12_field",
            "condition12_operator",
            "condition12_value",
            "condition12_combiner"};

    for (int i = startIdx; i <= endIdx; ++i) {
        queryStr += fieldNames[i - 8] + " = '" + smartiesData[i].toString() + "'";
        if (i < endIdx) {
            queryStr += ", ";
        }
    }

    queryStr += " WHERE id = " + smartiesData[0].toString();
    return queryStr;
}

void SmartiesTableModel::getWhereClauseForSmarties(SmartiesId smartiesId) {
    if (sDebug) {
        qDebug() << "[SMARTIESTABLEMODEL] [GETWHERECLAUSEFORSMARTIES] starts "
                    "for SmartiesId:"
                 << smartiesId;
    }

    QVariantList smartiesData;

    // Step 1: Query to get the smarties data
    QSqlQuery* query = new QSqlQuery(m_database);
    query->prepare("SELECT * FROM smarties WHERE id = :id");
    query->addBindValue(smartiesId.toVariant());

    if (query->exec()) {
        if (query->next()) {
            smartiesData.clear(); // Clear any existing data before appending

            // Populate smartiesData with the fields from the database row
            smartiesData.append(query->value("id").toString());           // id
            smartiesData.append(query->value("name").toString());         // name
            smartiesData.append(query->value("count").toInt());           // count
            smartiesData.append(query->value("show").toBool());           // show
            smartiesData.append(query->value("locked").toBool());         // locked
            smartiesData.append(query->value("autodj_source").toBool());  // autoDJ
            smartiesData.append(query->value("search_input").toString()); // search_input
            smartiesData.append(query->value("search_sql").toString());   // search_sql

            for (int i = 1; i <= 12; ++i) { // Handle conditions
                smartiesData.append(query->value(QString("condition%1_field").arg(i)).toString());
                smartiesData.append(
                        query->value(QString("condition%1_operator").arg(i))
                                .toString());
                smartiesData.append(query->value(QString("condition%1_value").arg(i)).toString());
                smartiesData.append(
                        query->value(QString("condition%1_combiner").arg(i))
                                .toString());
            }

            if (sDebug) {
                qDebug() << "[SMARTIESTABLEMODEL] [GETWHERECLAUSEFORSMARTIES] [CONSTRUCT SQL] -> "
                            "loaded data into QVariantList:"
                         << smartiesData;
            }

            // Step 2: Build the WHERE clause using the populated smartiesData
            QString whereClause = buildWhereClause(smartiesData);
            if (sDebug) {
                qDebug() << "[SMARTIESTABLEMODEL] [GETWHERECLAUSEFORSMARTIES] "
                            "[CONSTRUCT SQL] -> WHERE clause generated:"
                         << whereClause;
            }

            // Example of using the whereClause in another SQL query
            QString finalQuery = QString("SELECT * FROM your_table %1").arg(whereClause);
            if (sDebug) {
                qDebug() << "[SMARTIESTABLEMODEL] [GETWHERECLAUSEFORSMARTIES] "
                            "[CONSTRUCT SQL] -> Final SQL query:"
                         << finalQuery;
            }

        } else {
            if (sDebug) {
                qDebug() << "[SMARTIESTABLEMODEL] [GETWHERECLAUSEFORSMARTIES] "
                            "[CONSTRUCT SQL] -> No data found for SmartiesId:"
                         << smartiesId;
            }
        }
    } else {
        if (sDebug) {
            qDebug() << "[SMARTIESTABLEMODEL] [GETWHERECLAUSEFORSMARTIES] "
                        "[CONSTRUCT SQL] -> Failed to execute query -"
                     << query->lastError();
        }
    }

    delete query;
}

QString SmartiesTableModel::buildWhereClause(const QVariantList& smartiesData) {
    QString whereClause = "(";
    bool hasConditions = false;

    QStringList combinerOptions = {") END", "AND", "OR", ") AND (", ") OR ("};
    // Assuming searchValue is at index 6 (search_input)
    QString searchValue = smartiesData[6].toString(); // search_input
                                                      //    QString condition;
    // Assuming searchValue is at index 7 (search_dql)
    // QString searchValue = smartiesData[7].toString(); // seatch_dql

    for (int i = 1; i <= 12; ++i) {
        int baseIndex = 8 + (i - 1) * 4; // Adjusting for the correct index in smartiesData

        const QString& field = smartiesData[baseIndex].toString();
        const QString& op = smartiesData[baseIndex + 1].toString();
        const QString& value = smartiesData[baseIndex + 2].toString();
        QString combiner = smartiesData[baseIndex + 3].toString();

        //  begin build condition
        //  function moved to smartiesfunctions.h to share it with dlgsmartiesinfo to create preview
        QString condition = buildCondition(field, op, value);

        //  end build condition
        if (condition != "") {
            hasConditions = true;
            whereClause += condition;
            // Add combiner if not the last condition
            if (i < 12 && combinerOptions.contains(combiner)) {
                whereClause += " " + combiner.replace(") END", "") +
                        " "; // Adding spaces around the combiner
            }
        }
    }

    if (!hasConditions) {
        whereClause += QString(
                "library.artist LIKE '%%1%' OR "
                "library.title LIKE '%%1%' OR "
                "library.album LIKE '%%1%' OR "
                "library.album_artist LIKE '%%1%' OR "
                "library.composer LIKE '%%1%' OR "
                "library.genre LIKE '%%1%' OR "
                "library.comment LIKE '%%1%'")
                               .arg(searchValue);
    }

    whereClause += ")";

    if (sDebug) {
        qDebug() << "[SMARTIESTABLEMODEL] [GETWHERECLAUSEFORSMARTIES] "
                    "[CONSTRUCT SQL] -> Constructed WHERE clause:"
                 << whereClause;
    }
    return whereClause;
}
