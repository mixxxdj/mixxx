#include <QtSql>

#include "library/trackcollection.h"
#include "library/missingtablemodel.h"
#include "library/librarytablemodel.h"
#include "queryutil.h"

const QString MissingTableModel::MISSINGFILTER = "mixxx_deleted=0 AND fs_deleted=1";

MissingTableModel::MissingTableModel(QObject* parent,
                                     TrackCollection* pTrackCollection)
        : BaseSqlTableModel(parent, pTrackCollection,
                            "mixxx.db.model.missing") {
}

void MissingTableModel::init() {
    setTableModel();
}

// Must be called from m_pTrackCollection thread
void MissingTableModel::setTableModel(int id) {
    Q_UNUSED(id);
    QString tableName("missing_songs");
    m_pTrackCollection->callSync(
            [this, &tableName] (TrackCollectionPrivate* pTrackCollectionPrivate) {
        QSqlQuery query(pTrackCollectionPrivate->getDatabase());
        //query.prepare("DROP VIEW " + playlistTableName);
        //query.exec();

        QStringList columns;
        columns << "library." + LIBRARYTABLE_ID;

        query.prepare("CREATE TEMPORARY VIEW IF NOT EXISTS " + tableName + " AS "
                                    "SELECT "
                                    + columns.join(",") +
                                    " FROM library "
                                    "INNER JOIN track_locations "
                                    "ON library.location=track_locations.id "
                                    "WHERE " + MissingTableModel::MISSINGFILTER);
        if (!query.exec()) {
                LOG_FAILED_QUERY(query);
        }
        //Print out any SQL error, if there was one.
        if (query.lastError().isValid()) {
            qDebug() << __FILE__ << __LINE__ << query.lastError();
        }
    }, __PRETTY_FUNCTION__);

    QStringList tableColumns;
    tableColumns << LIBRARYTABLE_ID;
    setTable(tableName, LIBRARYTABLE_ID, tableColumns,
             m_pTrackCollection->getTrackSource());
    setDefaultSort(fieldIndex("artist"), Qt::AscendingOrder);
    setSearch("");
}

MissingTableModel::~MissingTableModel() {
}

// Must be called from Main thread
void MissingTableModel::purgeTracks(const QModelIndexList& indices) {
    QList<int> trackIds;

    foreach (QModelIndex index, indices) {
        int trackId = getTrackId(index);
        trackIds.append(trackId);
    }

    // tro's lambda idea. This code calls asynchronously!
    m_pTrackCollection->callAsync(
                [this, trackIds] (TrackCollectionPrivate* pTrackCollectionPrivate) {
        pTrackCollectionPrivate->getTrackDAO().purgeTracks(trackIds);
        // TODO(rryan) : do not select, instead route event to BTC and notify from
        // there.
        select(pTrackCollectionPrivate); //Repopulate the data model.
    }, __PRETTY_FUNCTION__);
}


bool MissingTableModel::isColumnInternal(int column) {
    if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ID) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PLAYED) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM_LOCK) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_KEY_ID)||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_MIXXXDELETED) ||
            column == fieldIndex(ColumnCache::COLUMN_TRACKLOCATIONSTABLE_FSDELETED) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_SOURCE) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_TYPE) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_LOCATION) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_HASH)) {
        return true;
    }
    return false;
}

bool MissingTableModel::isColumnHiddenByDefault(int column) {
    Q_UNUSED(column);
    return false;
}

// Override flags from BaseSqlModel since we don't want edit this model
Qt::ItemFlags MissingTableModel::flags(const QModelIndex &index) const {
    return readOnlyFlags(index);
}

TrackModel::CapabilitiesFlags MissingTableModel::getCapabilities() const {
    return TRACKMODELCAPS_NONE | TRACKMODELCAPS_PURGE;
}
