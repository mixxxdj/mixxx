#include "library/missing_hidden/missingtablemodel.h"

#include "library/dao/trackschema.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "moc_missingtablemodel.cpp"

namespace {

const QString kMissingFilter = "mixxx_deleted=0 AND fs_deleted=1";
const QString kModelName = "missing:";

} // anonymous namespace

MissingTableModel::MissingTableModel(QObject* parent,
                                     TrackCollectionManager* pTrackCollectionManager)
        : BaseSqlTableModel(parent, pTrackCollectionManager,
                            "mixxx.db.model.missing") {
    setTableModel();
}

void MissingTableModel::setTableModel(int id) {
    Q_UNUSED(id);
    QSqlQuery query(m_database);
    //query.prepare("DROP VIEW " + playlistTableName);
    //query.exec();
    QString tableName("missing_songs");

    QStringList columns;
    columns << "library." + LIBRARYTABLE_ID;

    query.prepare("CREATE TEMPORARY VIEW IF NOT EXISTS " + tableName + " AS "
                  "SELECT "
                  + columns.join(",") +
                  " FROM library "
                  "INNER JOIN track_locations "
                  "ON library.location=track_locations.id "
                  "WHERE " + kMissingFilter);
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
            std::move(tableColumns),
            m_pTrackCollectionManager->internalCollection()->getTrackSource());
    setDefaultSort(fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ARTIST), Qt::AscendingOrder);
    setSearch("");

}

MissingTableModel::~MissingTableModel() {
}

void MissingTableModel::purgeTracks(const QModelIndexList& indices) {
    m_pTrackCollectionManager->purgeTracks(getTrackRefs(indices));

    // TODO(rryan) : do not select, instead route event to BTC and notify from
    // there.
    select(); //Repopulate the data model.
}

bool MissingTableModel::isColumnInternal(int column) {
    return column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ID) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PLAYED) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM_LOCK) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_KEY_ID) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_MIXXXDELETED) ||
            column == fieldIndex(ColumnCache::COLUMN_TRACKLOCATIONSTABLE_FSDELETED) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_SOURCE) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_TYPE) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_LOCATION) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_COLOR) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_DIGEST) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_HASH);
}

// Override flags from BaseSqlModel since we don't want edit this model
Qt::ItemFlags MissingTableModel::flags(const QModelIndex &index) const {
    return readOnlyFlags(index);
}

TrackModel::Capabilities MissingTableModel::getCapabilities() const {
    return Capability::Purge |
            Capability::Properties |
            Capability::Sorting;
}

QString MissingTableModel::modelKey(bool noSearch) const {
    if (noSearch) {
        return kModelName + m_tableName;
    }
    return kModelName + m_tableName +
            QStringLiteral("#") +
            currentSearch();
}
