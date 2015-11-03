
#include "library/librarytablemodel.h"
#include "library/queryutil.h"
#include "playermanager.h"

const QString LibraryTableModel::DEFAULT_LIBRARYFILTER =
        "mixxx_deleted=0 AND fs_deleted=0";

LibraryTableModel::LibraryTableModel(QObject* parent,
                                     TrackCollection* pTrackCollection,
                                     const char* settingsNamespace)
        : BaseSqlTableModel(parent, pTrackCollection, settingsNamespace){
}

LibraryTableModel::~LibraryTableModel() {
}

void LibraryTableModel::init() {
    setTableModel();
}

// Must be called from Main thread
void LibraryTableModel::setTableModel(int id) {
    Q_UNUSED(id);
    QStringList columns = QStringList();
    columns << "library." + LIBRARYTABLE_ID << "'' as preview";

    const QString tableName = "library_view";

    // TODO(xxx) move this to separate function on accessing DB (create table)
    m_pTrackCollection->callSync(
            [this, &columns, &tableName] (TrackCollectionPrivate* pTrackCollectionPrivate) {
        QSqlQuery query(pTrackCollectionPrivate->getDatabase());
        QString queryString = "CREATE TEMPORARY VIEW IF NOT EXISTS " + tableName + " AS "
                "SELECT " + columns.join(", ") +
                " FROM library INNER JOIN track_locations "
                "ON library.location = track_locations.id "
                "WHERE (" + LibraryTableModel::DEFAULT_LIBRARYFILTER + ")";
        query.prepare(queryString);
        if (!query.exec()) {
            LOG_FAILED_QUERY(query);
        }
    }, __PRETTY_FUNCTION__);

    QStringList tableColumns;
    tableColumns << LIBRARYTABLE_ID;
    tableColumns << LIBRARYTABLE_PREVIEW;
    setTable(tableName, LIBRARYTABLE_ID, tableColumns,
             m_pTrackCollection->getTrackSource());
    setSearch("");
    setDefaultSort(fieldIndex("artist"), Qt::AscendingOrder);
}

// Must be called from Main thread
int LibraryTableModel::addTracks(const QModelIndex& index,
                                 const QList<QString>& locations) {
    Q_UNUSED(index);
    QList<QFileInfo> fileInfoList;
    foreach (QString fileLocation, locations) {
        fileInfoList.append(QFileInfo(fileLocation));
    }
    int tracksAdded = 0;
    // tro's lambda idea. This code calls synchronously!
    m_pTrackCollection->callSync(
                [this, &fileInfoList, &tracksAdded] (TrackCollectionPrivate* pTrackCollectionPrivate) {
        QList<int> trackIds = pTrackCollectionPrivate->getTrackDAO().addTracks(fileInfoList, true);
        select(pTrackCollectionPrivate);
        tracksAdded = trackIds.count();
    }, __PRETTY_FUNCTION__);
    return tracksAdded;
}

bool LibraryTableModel::isColumnInternal(int column) {
    if ((column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ID)) ||
            (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_URL)) ||
            (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_CUEPOINT)) ||
            (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_REPLAYGAIN)) ||
            (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_WAVESUMMARYHEX)) ||
            (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_SAMPLERATE)) ||
            (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_MIXXXDELETED)) ||
            (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_HEADERPARSED)) ||
            (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PLAYED)) ||
            (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM_LOCK)) ||
            (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_CHANNELS)) ||
            (column == fieldIndex(ColumnCache::COLUMN_TRACKLOCATIONSTABLE_FSDELETED)) ||
            (PlayerManager::numPreviewDecks() == 0 &&
             column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PREVIEW))) {
        return true;
    }

    return false;
}

bool LibraryTableModel::isColumnHiddenByDefault(int column) {
    Q_UNUSED(column);
    return false;
}

TrackModel::CapabilitiesFlags LibraryTableModel::getCapabilities() const {
    return TRACKMODELCAPS_NONE
            | TRACKMODELCAPS_RECEIVEDROPS
            | TRACKMODELCAPS_ADDTOPLAYLIST
            | TRACKMODELCAPS_ADDTOCRATE
            | TRACKMODELCAPS_ADDTOAUTODJ
            | TRACKMODELCAPS_RELOADMETADATA
            | TRACKMODELCAPS_LOADTODECK
            | TRACKMODELCAPS_LOADTOSAMPLER
            | TRACKMODELCAPS_LOADTOPREVIEWDECK
            | TRACKMODELCAPS_HIDE
            | TRACKMODELCAPS_MANIPULATEBEATS
            | TRACKMODELCAPS_CLEAR_BEATS
            | TRACKMODELCAPS_RESETPLAYED;
}
