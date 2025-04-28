#include "library/trackset/crate/cratetablemodel.h"

#include <QtDebug>

#include "library/dao/trackschema.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "library/trackset/crate/crate.h"
#include "moc_cratetablemodel.cpp"
#include "track/track.h"
#include "util/db/fwdsqlquery.h"

namespace {

const bool sDebug = false;
const QString kModelName = QStringLiteral("crate");

} // anonymous namespace

CrateTableModel::CrateTableModel(
        QObject* pParent,
        TrackCollectionManager* pTrackCollectionManager,
        UserSettingsPointer pConfig)
        : TrackSetTableModel(
                  pParent,
                  pTrackCollectionManager,
                  "mixxx.db.model.crate"),
          m_pConfig(pConfig) {
}

QList<QVariantMap> CrateTableModel::getGroupedCrates() {
    if (sDebug) {
        qDebug() << "[GROUPEDCRATESTABLEMODEL] Generating grouped crates list.";
    }

    QList<QVariantMap> groupedCrates;

    QSqlQuery query(m_database);
    // QString queryString;

    if (sDebug) {
        qDebug() << "[GROUPEDCRATESTABLEMODEL] configvalue GroupedCratesLength = "
                 << m_pConfig->getValue<int>(ConfigKey("[Library]", "GroupedCratesLength"));
        qDebug() << "[GROUPEDCRATESTABLEMODEL] configvalue GroupedCratesFixedLength = "
                 << m_pConfig->getValue<int>(ConfigKey("[Library]", "GroupedCratesFixedLength"), 0);
        qDebug() << "[GROUPEDCRATESTABLEMODEL] configvalue GroupedCratesVarLengthMask = "
                 << m_pConfig->getValue(ConfigKey("[Library]", "GroupedCratesVarLengthMask"));
    }

    // fixed prefix length
    if (m_pConfig->getValue<int>(ConfigKey("[Library]", "GroupedCratesLength")) == 0) {
        QString queryString =
                QStringLiteral(
                        "SELECT DISTINCT "
                        "  SUBSTR(name, 1, %1) AS group_name, "
                        "  id AS crate_id, "
                        "  name AS crate_name "
                        "FROM crates "
                        "WHERE show = 1 "
                        "ORDER BY LOWER(name)")
                        .arg(m_pConfig->getValue(
                                ConfigKey("[Library]",
                                        "GroupedCratesFixedLength"),
                                0));
        if (sDebug) {
            qDebug() << "[GROUPEDCRATESTABLEMODEL] queryString: " << queryString;
        }
        if (!query.exec(queryString)) {
            qWarning() << "[GROUPEDCRATESTABLEMODEL] Failed to execute grouped "
                          "crates query:"
                       << query.lastError();
            return groupedCrates;
        }

        while (query.next()) {
            QVariantMap crateData;
            // QString groupName = query.value("group_name").toString().trimmed();
            QString groupName = query.value("group_name").toString();
            crateData["group_name"] = groupName;
            crateData["crate_id"] = query.value("crate_id");
            crateData["crate_name"] = query.value("crate_name");
            groupedCrates.append(crateData);
            if (sDebug) {
                qDebug() << "[GROUPEDCRATESTABLEMODEL] Grouped crates -> crate "
                            "added: "
                         << crateData;
            }
        }
    } else {
        // Variable prefix length with mask, multiple occurrences -> multilevel subgroups
        QString queryString = QStringLiteral(
                "SELECT DISTINCT "
                "  id AS crate_id, "
                "  name AS crate_name "
                "FROM crates "
                "WHERE show = 1 "
                "ORDER BY LOWER(name)");
        if (sDebug) {
            qDebug() << "[GROUPEDCRATESTABLEMODEL] queryString: " << queryString;
        }
        if (!query.exec(queryString)) {
            qWarning() << "[GROUPEDCRATESTABLEMODEL] Failed to execute grouped "
                          "crates query:"
                       << query.lastError();
            return groupedCrates;
        }
        const QString& searchDelimit = m_pConfig->getValue(
                ConfigKey("[Library]", "GroupedCratesVarLengthMask"));

        while (query.next()) {
            QString crateName = query.value("crate_name").toString();
            if (crateName.contains(searchDelimit)) {
                // QStringList groupHierarchy = crateName.split(searchDelimit, Qt::SkipEmptyParts);
                QStringList groupHierarchy = crateName.split(searchDelimit);
                QString currentGroup;

                for (int i = 0; i < groupHierarchy.size(); ++i) {
                    // currentGroup += (i > 0 ? searchDelimit : "") + groupHierarchy[i].trimmed();
                    currentGroup += (i > 0 ? searchDelimit : "") + groupHierarchy[i];

                    QVariantMap crateData;
                    crateData["group_name"] = currentGroup;
                    crateData["crate_id"] = query.value("crate_id");
                    crateData["crate_name"] = crateName;

                    // Add only the full crate record for the last level
                    if (i == groupHierarchy.size() - 1) {
                        groupedCrates.append(crateData);
                    }

                    if (sDebug) {
                        qDebug() << "[GROUPEDCRATESTABLEMODEL] Grouped crates -> crate added: "
                                 << crateData;
                    }
                }
            } else {
                // No delimiter in crate name -> root-level
                QVariantMap crateData;
                // crateData["group_name"] = crateName.trimmed();
                crateData["group_name"] = crateName;
                crateData["crate_id"] = query.value("crate_id");
                crateData["crate_name"] = crateName;
                groupedCrates.append(crateData);

                if (sDebug) {
                    qDebug() << "[GROUPEDCRATESTABLEMODEL] Grouped crates -> "
                                "crate added at root level: "
                             << crateData;
                }
            }
        }
    }
    if (sDebug) {
        qDebug() << "[GROUPEDCRATESTABLEMODEL] Grouped crates list generated "
                    "with"
                 << groupedCrates.size() << "entries.";
    }
    return groupedCrates;
}

void CrateTableModel::selectCrate(CrateId crateId) {
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
    setDefaultSort(fieldIndex("artist"), Qt::AscendingOrder);
}

void CrateTableModel::selectCrateGroup(const QString& groupName) {
    if (sDebug) {
        qDebug() << "[CrateTableModel] -> selectCrateGroup() -> Searching for "
                    "tracks in groups starting with:"
                 << groupName;
    }
    const QString& checkStamp = QDateTime::currentDateTime().toString("hhmmss");

    const QString& tableName = QStringLiteral("crate_%1").arg(checkStamp);

    QStringList columns;
    columns << LIBRARYTABLE_ID
            << "'' AS " + LIBRARYTABLE_PREVIEW
            << LIBRARYTABLE_COVERART_DIGEST + " AS " + LIBRARYTABLE_COVERART;

    QString queryString =
            QString("CREATE TEMPORARY VIEW IF NOT EXISTS %1 AS "
                    "SELECT %2 FROM %3 "
                    "WHERE library.id IN(SELECT crate_tracks.track_id from "
                    "crate_tracks "
                    "WHERE crate_tracks.crate_id IN(SELECT crates.id from "
                    "crates WHERE crates.name LIKE '%4%')) "
                    "AND %5=0")
                    .arg(tableName,
                            columns.join(","),
                            LIBRARY_TABLE,
                            groupName,
                            LIBRARYTABLE_MIXXXDELETED);

    if (sDebug) {
        qDebug() << "[CrateTableModel] -> Generated SQL Query:" << queryString;
    }

    // Execute the query
    FwdSqlQuery(m_database, queryString).execPrepared();
    columns[0] = LIBRARYTABLE_ID;
    columns[1] = LIBRARYTABLE_PREVIEW;
    columns[2] = LIBRARYTABLE_COVERART;

    // Update the table and view
    setTable(tableName,
            LIBRARYTABLE_ID,
            columns,
            m_pTrackCollectionManager->internalCollection()->getTrackSource());
    setDefaultSort(fieldIndex("artist"), Qt::AscendingOrder);

    if (sDebug) {
        qDebug() << "[CrateTableModel] -> Group table successfully created "
                    "with provided Crate IDs.";
    }
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
