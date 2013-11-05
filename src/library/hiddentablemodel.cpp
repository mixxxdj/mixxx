#include "library/hiddentablemodel.h"
#include "library/queryutil.h"

HiddenTableModel::HiddenTableModel(QObject* parent, TrackCollection* pTrackCollection)
        : BaseSqlTableModel(parent, pTrackCollection, "mixxx.db.model.missing") {  // WTF!? COPYPASTE
}

HiddenTableModel::~HiddenTableModel() {
}

void HiddenTableModel::init() {
    setTableModel();
}

// Must be called from TrackCollection thread
void HiddenTableModel::setTableModel(int id){
    Q_UNUSED(id);
    const QString tableName("hidden_songs");
    // tro's lambda idea. This code calls synchronously!
    m_pTrackCollection->callSync(
            [this, &tableName] (void) {
        QSqlQuery query(m_pTrackCollection->getDatabase());

        QStringList columns;
        columns << "library." + LIBRARYTABLE_ID;
        QString filter("mixxx_deleted=1");
        query.prepare("CREATE TEMPORARY VIEW IF NOT EXISTS " + tableName + " AS "
                      "SELECT "
                      + columns.join(",") +
                      " FROM library "
                      "INNER JOIN track_locations "
                      "ON library.location=track_locations.id "
                      "WHERE " + filter);
        if (!query.exec()) {
            LOG_FAILED_QUERY(query);
        }
    }, __PRETTY_FUNCTION__);

    QStringList tableColumns;
    tableColumns << LIBRARYTABLE_ID;
    setTable(tableName, LIBRARYTABLE_ID, tableColumns,
             m_pTrackCollection->getTrackSource("default"));

    initHeaderData();
    setDefaultSort(fieldIndex("artist"), Qt::AscendingOrder);
    setSearch("");
}

// Must be called from Main thread
void HiddenTableModel::purgeTracks(const QModelIndexList& indices) {
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

// Must be called from Main thread
void HiddenTableModel::unhideTracks(const QModelIndexList& indices) {
    QList<int> trackIds;

    foreach (QModelIndex index, indices) {
        int trackId = getTrackId(index);
        trackIds.append(trackId);
    }

    // tro's lambda idea. This code calls asynchronously!
    m_pTrackCollection->callAsync(
                [this, trackIds] (void) {
        m_trackDAO.unhideTracks(trackIds);
        // TODO(rryan) : do not select, instead route event to BTC and notify from 
        // there.
        select(); //Repopulate the data model.
    }, __PRETTY_FUNCTION__);
}

bool HiddenTableModel::isColumnInternal(int column) {
    if (column == fieldIndex(LIBRARYTABLE_ID) ||
        column == fieldIndex(LIBRARYTABLE_PLAYED) ||
        column == fieldIndex(LIBRARYTABLE_BPM_LOCK) ||
        column == fieldIndex(LIBRARYTABLE_MIXXXDELETED) ||
        column == fieldIndex(TRACKLOCATIONSTABLE_FSDELETED)) {
        return true;
    }
    return false;
}
bool HiddenTableModel::isColumnHiddenByDefault(int column) {
    if (column == fieldIndex(LIBRARYTABLE_KEY)) {
        return true;
    }
    return false;
}

// Override flags from BaseSqlModel since we don't want edit this model
Qt::ItemFlags HiddenTableModel::flags(const QModelIndex &index) const {
    return readOnlyFlags(index);
}

TrackModel::CapabilitiesFlags HiddenTableModel::getCapabilities() const {
    return TRACKMODELCAPS_NONE
            | TRACKMODELCAPS_PURGE
            | TRACKMODELCAPS_UNHIDE;
}
