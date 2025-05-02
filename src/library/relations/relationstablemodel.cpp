#include "library/relations/relationstablemodel.h"

#include "library/dao/trackschema.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "mixer/playermanager.h"
#include "moc_relationstablemodel.cpp"

namespace {

const QString kModelName = QStringLiteral("relations");

} // anonymous namespace

RelationsTableModel::RelationsTableModel(
        QObject* parent,
        TrackCollectionManager* pTrackCollectionManager)
        : BaseSqlTableModel(
                  parent,
                  pTrackCollectionManager,
                  "mixxx.db.model.relations") {
    showAllRelations();
}

bool RelationsTableModel::isColumnInternal(int column) {
    return column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ID) ||
            column == fieldIndex(ColumnCache::COLUMN_RELATIONTABLE_ID) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PLAYED) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_MIXXXDELETED) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM_LOCK) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_KEY_ID) ||
            column == fieldIndex(ColumnCache::COLUMN_TRACKLOCATIONSTABLE_FSDELETED) ||
            (PlayerManager::numPreviewDecks() == 0 &&
                    column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PREVIEW)) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_SOURCE) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_TYPE) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_LOCATION) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_COLOR) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_DIGEST) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_HASH);
}

bool RelationsTableModel::isColumnHiddenByDefault(int column) {
    return column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_DATETIMEADDED) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COMMENT) ||
            BaseSqlTableModel::isColumnHiddenByDefault(column);
}

void RelationsTableModel::showRelatedTracks(TrackPointer pTrack) {
    if (!pTrack) {
        return;
    }
    m_pDeckTrack = pTrack;
    TrackId trackId = pTrack->getId();
    QString trackTableName = QStringLiteral("relations_track_%1").arg(trackId.toString());

    QStringList columns;
    columns << "library." + LIBRARYTABLE_ID
            << "r." + RELATIONTABLE_ID
            << "r." + RELATIONTABLE_DATETIMEADDED
            << "r." + RELATIONTABLE_COMMENT
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
    columns[1] = RELATIONTABLE_ID;
    columns[2] = RELATIONTABLE_DATETIMEADDED;
    columns[3] = RELATIONTABLE_COMMENT;
    columns[4] = LIBRARYTABLE_PREVIEW;
    columns[5] = LIBRARYTABLE_COVERART;
    setTable(trackTableName,
            LIBRARYTABLE_ID,
            columns,
            m_pTrackCollectionManager->internalCollection()->getTrackSource());
    setSearch("");
    setDefaultSort(fieldIndex("artist"), Qt::AscendingOrder);
}

void RelationsTableModel::showAllRelations() {
    QString tableName = QStringLiteral("all_relations");

    QStringList columns;
    columns << "l." + LIBRARYTABLE_ID
            << "r." + RELATIONTABLE_ID
            << "r." + RELATIONTABLE_DATETIMEADDED
            << "r." + RELATIONTABLE_COMMENT
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
    columns[1] = RELATIONTABLE_ID;
    columns[2] = RELATIONTABLE_DATETIMEADDED;
    columns[3] = RELATIONTABLE_COMMENT;
    columns[4] = LIBRARYTABLE_PREVIEW;
    columns[5] = LIBRARYTABLE_COVERART;
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
            Capability::Properties;

    return caps;
}

QString RelationsTableModel::modelKey(bool noSearch) const {
    if (m_pDeckTrack) {
        if (noSearch) {
            return kModelName + QChar(':') + m_pDeckTrack->getId().toString();
        }
        return kModelName + QChar(':') +
                m_pDeckTrack->getId().toString() +
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
