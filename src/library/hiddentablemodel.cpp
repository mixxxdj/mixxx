#include "library/hiddentablemodel.h"

#include "library/dao/trackschema.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "moc_hiddentablemodel.cpp"

HiddenTableModel::HiddenTableModel(QObject* parent,
        TrackCollectionManager* pTrackCollectionManager)
        : BaseSqlTableModel(parent, pTrackCollectionManager, "mixxx.db.model.missing") {
    setTableModel();
}

HiddenTableModel::~HiddenTableModel() {
}

void HiddenTableModel::setTableModel() {
    const QString tableName("hidden_songs");

    QStringList columns;
    columns << "library." + LIBRARYTABLE_ID;

    QSqlQuery query(m_database);
    query.prepare(
            "CREATE TEMPORARY VIEW IF NOT EXISTS " + tableName +
            " AS SELECT " + columns.join(",") +
            " FROM library "
            "INNER JOIN track_locations "
            "ON library.location=track_locations.id "
            "WHERE mixxx_deleted=1");
    if (!query.exec()) {
        qDebug() << query.executedQuery() << query.lastError();
    }

    //Print out any SQL error, if there was one.
    if (query.lastError().isValid()) {
        qDebug() << __FILE__ << __LINE__ << query.lastError();
    }

    QStringList tableColumns;
    tableColumns << LIBRARYTABLE_ID;
    setTable(tableName,
            LIBRARYTABLE_ID,
            tableColumns,
            m_pTrackCollectionManager->internalCollection()->getTrackSource());
    setDefaultSort(fieldIndex("artist"), Qt::AscendingOrder);
    setSearch("");
}

void HiddenTableModel::purgeTracks(const QModelIndexList& indices) {
    m_pTrackCollectionManager->purgeTracks(getTrackRefs(indices));

    // TODO(rryan) : do not select, instead route event to BTC and notify from
    // there.
    select(); // Repopulate the data model.
}

void HiddenTableModel::unhideTracks(const QModelIndexList& indices) {
    QList<TrackId> trackIds;

    foreach (QModelIndex index, indices) {
        trackIds.append(getTrackId(index));
    }

    m_pTrackCollectionManager->unhideTracks(trackIds);

    // TODO(rryan) : do not select, instead route event to BTC and notify from there.
    select(); // Repopulate the data model.
}

bool HiddenTableModel::isColumnInternal(int column) {
    return column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ID) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PLAYED) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM_LOCK) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_MIXXXDELETED) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_KEY_ID) ||
            column == fieldIndex(ColumnCache::COLUMN_TRACKLOCATIONSTABLE_FSDELETED) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_SOURCE) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_TYPE) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_LOCATION) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_COLOR) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_DIGEST) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_HASH);
}

// Override flags from BaseSqlModel since we don't want edit this model
Qt::ItemFlags HiddenTableModel::flags(const QModelIndex& index) const {
    return readOnlyFlags(index);
}

TrackModel::Capabilities HiddenTableModel::getCapabilities() const {
    return Capability::Purge | Capability::Unhide;
}
