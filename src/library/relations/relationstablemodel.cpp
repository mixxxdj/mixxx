#include "library/relations/relationstablemodel.h"

#include "library/dao/trackschema.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "moc_relationstablemodel.cpp"

namespace {

const QString kModelName = QStringLiteral("relations");

} // anonymous namespace

RelationsTableModel::RelationsTableModel(
        QObject* parent,
        TrackCollectionManager* pTrackCollectionManager)
        : TrackSetTableModel(
                  parent,
                  pTrackCollectionManager,
                  "mixxx.db.model.relations") {
}

void RelationsTableModel::displayRelatedTracks(TrackPointer pTrack) {
    if (!pTrack) {
        return;
    }
    m_pTrack = pTrack;
    TrackId trackId = pTrack->getId();
    QString trackTableName = QStringLiteral("relations_track_%1").arg(trackId.toString());

    QStringList columns;
    columns << "library." + LIBRARYTABLE_ID
            << "'' AS " + LIBRARYTABLE_PREVIEW
            // For sorting the cover art column we give LIBRARYTABLE_COVERART
            // the same value as the cover digest.
            << LIBRARYTABLE_COVERART_DIGEST + " AS " + LIBRARYTABLE_COVERART;

    QSqlQuery query(m_database);
    QString queryString = QStringLiteral(
            "CREATE TEMPORARY VIEW IF NOT EXISTS %1 AS "
            "SELECT %2 "
            "FROM library "
            "INNER JOIN relations r "
            "ON (r.track_a = %3 AND library.id = r.track_b) "
            "OR (r.track_b = %3 AND library.id = r.track_a)")
                                  .arg(trackTableName,
                                          columns.join(','),
                                          trackId.toString());
    query.prepare(queryString);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }

    columns[0] = LIBRARYTABLE_ID;
    columns[1] = LIBRARYTABLE_PREVIEW;
    columns[2] = LIBRARYTABLE_COVERART;
    setTable(trackTableName,
            LIBRARYTABLE_ID,
            columns,
            m_pTrackCollectionManager->internalCollection()->getTrackSource());
    setSearch("");
    setDefaultSort(fieldIndex("artist"), Qt::AscendingOrder);
}

void RelationsTableModel::displayAllRelations() {
    QString tableName = QStringLiteral("all_relations");

    QStringList columns;
    columns << "l." + LIBRARYTABLE_ID
            << "'' AS " + LIBRARYTABLE_PREVIEW
            // For sorting the cover art column we give LIBRARYTABLE_COVERART
            // the same value as the cover digest.
            << LIBRARYTABLE_COVERART_DIGEST + " AS " + LIBRARYTABLE_COVERART;

    QSqlQuery query(m_database);
    QString queryString = QStringLiteral(
            "CREATE TEMPORARY VIEW IF NOT EXISTS %1 AS "
            "SELECT %2, r.rowid * 2 AS sort_order "
            "FROM relations r "
            "JOIN library l ON l.id = r.track_a "
            "UNION ALL "
            "SELECT %2, r.rowid * 2 + 1 AS sort_order "
            "FROM relations r "
            "JOIN library l ON l.id = r.track_b "
            "ORDER BY sort_order")
                                  .arg(tableName,
                                          columns.join(','));
    query.prepare(queryString);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }

    columns[0] = LIBRARYTABLE_ID;
    columns[1] = LIBRARYTABLE_PREVIEW;
    columns[2] = LIBRARYTABLE_COVERART;
    setTable(tableName,
            LIBRARYTABLE_ID,
            columns,
            m_pTrackCollectionManager->internalCollection()->getTrackSource());
}

TrackModel::Capabilities RelationsTableModel::getCapabilities() const {
    Capabilities caps =
            Capability::AddToTrackSet |
            Capability::EditMetadata |
            Capability::LoadToDeck |
            Capability::LoadToSampler |
            Capability::LoadToPreviewDeck |
            Capability::ResetPlayed |
            Capability::Hide |
            Capability::Analyze |
            Capability::Properties |
            Capability::Sorting;

    return caps;
}

QString RelationsTableModel::modelKey(bool noSearch) const {
    if (m_pTrack) {
        if (noSearch) {
            return kModelName + QChar(':') + m_pTrack->getId().toString();
        }
        return kModelName + QChar(':') +
                m_pTrack->getId().toString() +
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
