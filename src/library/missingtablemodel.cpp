#include <QtCore>
#include <QtGui>
#include <QtSql>
#include "library/trackcollection.h"
#include "library/missingtablemodel.h"
#include "library/librarytablemodel.h"
#include "mixxxutils.cpp"
#include "queryutil.h"

const QString MissingTableModel::MISSINGFILTER = "mixxx_deleted=0 AND fs_deleted=1";

MissingTableModel::MissingTableModel(QObject* parent,
                                     TrackCollection* pTrackCollection)
        : BaseSqlTableModel(parent, pTrackCollection,
                            "mixxx.db.model.missing") {
}

void MissingTableModel::init() {
    // tro's lambda idea. This code calls synchronously!
    m_pTrackCollection->callSync(
                [this] (void) {
        setTableModel();
    }, __PRETTY_FUNCTION__);
}

// Must be called from m_pTrackCollection thread
void MissingTableModel::setTableModel(int id) {
    Q_UNUSED(id);
    QSqlQuery query(m_pTrackCollection->getDatabase());
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
        LOG_FAILED_QUERY(query);
    }

    QStringList tableColumns;
    tableColumns << LIBRARYTABLE_ID;
    setTable(tableName, LIBRARYTABLE_ID, tableColumns,
             m_pTrackCollection->getTrackSource("default"));

    initHeaderData();
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
                [this, trackIds] (void) {
        m_trackDAO.purgeTracks(trackIds);
        // TODO(rryan) : do not select, instead route event to BTC and notify from
        // there.
        select(); //Repopulate the data model.
    }, __PRETTY_FUNCTION__);
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

// Override flags from BaseSqlModel since we don't want edit this model
Qt::ItemFlags MissingTableModel::flags(const QModelIndex &index) const {
    return readOnlyFlags(index);
}

TrackModel::CapabilitiesFlags MissingTableModel::getCapabilities() const {
    return TRACKMODELCAPS_NONE | TRACKMODELCAPS_PURGE;
}
