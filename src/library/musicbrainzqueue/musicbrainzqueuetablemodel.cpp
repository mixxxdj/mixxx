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
    const QString tableName = QStringLiteral("musicbrainz_queue_view");

    // Drop then recreate so that schema changes take effect without restarting
    // Mixxx.  The view is TEMPORARY (per database connection), so DROP here
    // has no effect on any other connection in the process.
    QSqlQuery dropQuery(m_database);
    dropQuery.exec(QStringLiteral("DROP VIEW IF EXISTS ") + tableName);

    // LEFT JOIN acoustid_queue: tracks that have a valid fingerprint but were
    // never enqueued still appear in the view.  Their status column comes back
    // as NULL; rawValue() maps that to the string "pending".
    QSqlQuery query(m_database);
    query.prepare(
            "CREATE TEMPORARY VIEW IF NOT EXISTS " + tableName +
            " AS "
            "SELECT"
            "  library." +
            LIBRARYTABLE_ID +
            ","
            "  aq.status"
            " FROM library"
            " INNER JOIN track_locations"
            "   ON library.location = track_locations.id"
            " INNER JOIN fingerprint_metadata fm"
            "   ON library.id = fm.track_id"
            " LEFT JOIN acoustid_queue aq"
            "   ON library.id = aq.track_id"
            " WHERE library.acoustid_id IS NULL"
            "   AND fm.fingerprint_valid = 1"
            "   AND library.mixxx_deleted = 0"
            "   AND track_locations.fs_deleted = 0");

    if (!query.exec()) {
        qDebug() << query.executedQuery() << query.lastError();
    }

    if (query.lastError().isValid()) {
        qDebug() << __FILE__ << __LINE__ << query.lastError();
    }

    QStringList tableColumns;
    tableColumns << LIBRARYTABLE_ID;
    tableColumns << QStringLiteral("status");

    setTable(tableName,
            LIBRARYTABLE_ID,
            std::move(tableColumns),
            m_pTrackCollectionManager->internalCollection()->getTrackSource());
    setDefaultSort(
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ARTIST),
            Qt::AscendingOrder);
    setSearch(QString());

    // headerData() is final in BaseTrackTableModel, so setHeaderData() is the
    // only way to set a display name for the Status column.  setTable() calls
    // initTableColumnsAndHeaderProperties() first, which assigns names from
    // ColumnCache; we overwrite the non-standard "status" entry here.
    setHeaderData(COL_STATUS, Qt::Horizontal, tr("Status"));
}

QVariant MusicBrainzQueueTableModel::rawValue(
        const QModelIndex& index) const {
    if (index.column() == COL_STATUS) {
        const QVariant raw = BaseSqlTableModel::rawValue(index);
        const QString rawStatus = raw.toString();

        if (raw.isNull() || rawStatus.isEmpty()) {
            return tr("Pending");
        }
        if (rawStatus == QStringLiteral("queued")) {
            return tr("Queued");
        }
        if (rawStatus == QStringLiteral("failed")) {
            return tr("Lookup failed");
        }
        if (rawStatus == QStringLiteral("unmatched")) {
            return tr("No match");
        }

        // Unknown/future status value -- show the raw text
        return raw;
    }
    return BaseSqlTableModel::rawValue(index);
}

bool MusicBrainzQueueTableModel::isColumnInternal(int column) {
    // COL_ID is always hidden.  The remaining internal columns below all live
    // in the track source (indices >= COL_COUNT) and are matched by fieldIndex.
    if (column == COL_ID) {
        return true;
    }
    return column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PLAYED) ||
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
