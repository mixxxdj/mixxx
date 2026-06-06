#include "library/musicbrainzqueue/musicbrainzqueuetablemodel.h"

#include "library/dao/trackschema.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "moc_musicbrainzqueuetablemodel.cpp"

namespace {

const QString kModelName = "musicbrainzqueue:";

} // anonymous namespace

MusicBrainzQueueTableModel::MusicBrainzQueueTableModel(
        QObject* parent,
        TrackCollectionManager* pTrackCollectionManager)
        : BaseSqlTableModel(parent,
                  pTrackCollectionManager,
                  "mixxx.db.model.musicbrainzqueue") {
    setTableModel();
}

MusicBrainzQueueTableModel::~MusicBrainzQueueTableModel() {
}

void MusicBrainzQueueTableModel::setTableModel() {
    const QString tableName("musicbrainz_queue_view");

    QStringList columns;
    columns << "library." + LIBRARYTABLE_ID;

    QSqlQuery query(m_database);
    query.prepare(
            "CREATE TEMPORARY VIEW IF NOT EXISTS " + tableName +
            " AS SELECT " + columns.join(",") +
            " FROM library "
            "INNER JOIN track_locations "
            "ON library.location = track_locations.id "
            "INNER JOIN fingerprint_metadata fm "
            "ON library.id = fm.track_id "
            "WHERE library.acoustid_id IS NULL "
            "AND fm.fingerprint_valid = 1 "
            "AND library.mixxx_deleted = 0 "
            "AND track_locations.fs_deleted = 0");

    if (!query.exec()) {
        qDebug() << query.executedQuery() << query.lastError();
    }

    if (query.lastError().isValid()) {
        qDebug() << __FILE__ << __LINE__ << query.lastError();
    }

    QStringList tableColumns;
    tableColumns << LIBRARYTABLE_ID;
    setTable(tableName,
            LIBRARYTABLE_ID,
            std::move(tableColumns),
            m_pTrackCollectionManager->internalCollection()->getTrackSource());
    setDefaultSort(
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ARTIST),
            Qt::AscendingOrder);
    setSearch(QString());
}

bool MusicBrainzQueueTableModel::isColumnInternal(int column) {
    return column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ID) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PLAYED) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM_LOCK) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BEATS_VERSION) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_KEY_ID) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_MIXXXDELETED) ||
            column == fieldIndex(ColumnCache::COLUMN_TRACKLOCATIONSTABLE_DIRECTORY) ||
            column == fieldIndex(ColumnCache::COLUMN_TRACKLOCATIONSTABLE_FSDELETED) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_SOURCE) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_TYPE) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_LOCATION) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_COLOR) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_DIGEST) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_HASH);
}

Qt::ItemFlags MusicBrainzQueueTableModel::flags(
        const QModelIndex& index) const {
    return readOnlyFlags(index);
}

TrackModel::Capabilities MusicBrainzQueueTableModel::getCapabilities() const {
    return Capability::Properties | Capability::Sorting;
}

QString MusicBrainzQueueTableModel::modelKey(bool noSearch) const {
    if (noSearch) {
        return kModelName + m_tableName;
    }
    return kModelName + m_tableName +
            QStringLiteral("#") +
            currentSearch();
}
