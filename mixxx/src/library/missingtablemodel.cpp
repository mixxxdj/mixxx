#include <QtCore>
#include <QtGui>
#include <QtSql>
#include "library/trackcollection.h"
#include "library/missingtablemodel.h"
#include "library/librarytablemodel.h"
#include "mixxxutils.cpp"

const QString MissingTableModel::MISSINGFILTER = "mixxx_deleted=0 AND fs_deleted=1";

MissingTableModel::MissingTableModel(QObject* parent,
                                     TrackCollection* pTrackCollection)
        : BaseSqlTableModel(parent, pTrackCollection,
                            pTrackCollection->getDatabase(),
                            "mixxx.db.model.missing"),
          m_pTrackCollection(pTrackCollection),
          m_trackDao(m_pTrackCollection->getTrackDAO()) {

    QSqlQuery query(pTrackCollection->getDatabase());
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
                  "WHERE " + MissingTableModel::MISSINGFILTER);
    if (!query.exec()) {
        qDebug() << query.executedQuery() << query.lastError();
    }

    //Print out any SQL error, if there was one.
    if (query.lastError().isValid()) {
     	qDebug() << __FILE__ << __LINE__ << query.lastError();
    }

    QStringList tableColumns;
    tableColumns << LIBRARYTABLE_ID;
    setTable(tableName, LIBRARYTABLE_ID, tableColumns,
             m_pTrackCollection->getTrackSource("default"));

    initHeaderData();
    setDefaultSort(fieldIndex("artist"), Qt::AscendingOrder);
    setSearch("");

    connect(this, SIGNAL(doSearch(const QString&)),
            this, SLOT(slotSearch(const QString&)));
}

MissingTableModel::~MissingTableModel() {
}

bool MissingTableModel::addTrack(const QModelIndex& index, QString location) {
    Q_UNUSED(index);
    Q_UNUSED(location);
    return false;
}

TrackPointer MissingTableModel::getTrack(const QModelIndex& index) const {
    //FIXME: use position instead of location for playlist tracks?

    //const int locationColumnIndex = this->fieldIndex(LIBRARYTABLE_LOCATION);
    //QString location = index.sibling(index.row(), locationColumnIndex).data().toString();
    int trackId = getTrackId(index);
    return m_trackDao.getTrack(trackId);
}

void MissingTableModel::purgeTracks(const QModelIndexList& indices) {
    QList<int> trackIds;

    foreach (QModelIndex index, indices) {
        int trackId = getTrackId(index);
        trackIds.append(trackId);
    }

    m_trackDao.purgeTracks(trackIds);

    // TODO(rryan) : do not select, instead route event to BTC and notify from
    // there.
    select(); //Repopulate the data model.
}


void MissingTableModel::search(const QString& searchText) {
    // qDebug() << "MissingTableModel::search()" << searchText
    //          << QThread::currentThread();
    emit(doSearch(searchText));
}

void MissingTableModel::slotSearch(const QString& searchText) {
    BaseSqlTableModel::search(searchText);
}

bool MissingTableModel::isColumnInternal(int column) {
    if (column == fieldIndex(LIBRARYTABLE_ID) ||
        column == fieldIndex(LIBRARYTABLE_PLAYED) ||
        column == fieldIndex(LIBRARYTABLE_BPM_LOCK) ||
        column == fieldIndex(LIBRARYTABLE_MIXXXDELETED) ||
        column == fieldIndex(TRACKLOCATIONSTABLE_FSDELETED)) {
        return true;
    }
    return false;
}
bool MissingTableModel::isColumnHiddenByDefault(int column) {
    if (column == fieldIndex(LIBRARYTABLE_KEY)) {
        return true;
    }
    return false;
}

/** Override flags from BaseSqlModel since we don't want edit this model */
Qt::ItemFlags MissingTableModel::flags(const QModelIndex &index) const {
    return readOnlyFlags(index);
}

TrackModel::CapabilitiesFlags MissingTableModel::getCapabilities() const {
    return TRACKMODELCAPS_NONE
            | TRACKMODELCAPS_PURGE;
}
