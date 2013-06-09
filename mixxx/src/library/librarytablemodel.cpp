
#include "library/librarytablemodel.h"
#include "library/queryutil.h"
#include "mixxxutils.cpp"
#include "playermanager.h"

const QString LibraryTableModel::DEFAULT_LIBRARYFILTER =
        "mixxx_deleted=0 AND fs_deleted=0"; 

LibraryTableModel::LibraryTableModel(QObject* parent,
                                     TrackCollection* pTrackCollection,
                                     QString settingsNamespace)
        : BaseSqlTableModel(parent, pTrackCollection, settingsNamespace){
    setTableModel();
}

LibraryTableModel::~LibraryTableModel() {
}

void LibraryTableModel::setTableModel(int id){
    Q_UNUSED(id);
    QStringList columns;
    columns << "library."+LIBRARYTABLE_ID << "'' as preview";

    QString tableName = "library_view";

    QSqlQuery query(m_pTrackCollection->getDatabase());
    QString queryString = "CREATE TEMPORARY VIEW IF NOT EXISTS "+tableName+" AS "
            "SELECT " + columns.join(", ") +
            " FROM library INNER JOIN track_locations "
            "ON library.location = track_locations.id "
            "WHERE (" + LibraryTableModel::DEFAULT_LIBRARYFILTER + ")";
    query.prepare(queryString);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }

    QStringList tableColumns;
    tableColumns << LIBRARYTABLE_ID;
    tableColumns << "preview";
    setTable(tableName, LIBRARYTABLE_ID, tableColumns,
             m_pTrackCollection->getTrackSource("default"));

    // BaseSqlTabelModel will setup the header info
    initHeaderData();

    setSearch("");
    setDefaultSort(fieldIndex("artist"), Qt::AscendingOrder);
}


int LibraryTableModel::addTracks(const QModelIndex& index, QList<QString> locations) {
    Q_UNUSED(index);
    QList<QFileInfo> fileInfoList;
    foreach (QString fileLocation, locations) {
        fileInfoList.append(QFileInfo(fileLocation));
    }
    QList<int> trackIds = m_trackDAO.addTracks(fileInfoList, true);
    select();
    return trackIds.size();
}

bool LibraryTableModel::isColumnInternal(int column) {
    if ((column == fieldIndex(LIBRARYTABLE_ID)) ||
        (column == fieldIndex(LIBRARYTABLE_URL)) ||
        (column == fieldIndex(LIBRARYTABLE_CUEPOINT)) ||
        (column == fieldIndex(LIBRARYTABLE_REPLAYGAIN)) ||
        (column == fieldIndex(LIBRARYTABLE_WAVESUMMARYHEX)) ||
        (column == fieldIndex(LIBRARYTABLE_SAMPLERATE)) ||
        (column == fieldIndex(LIBRARYTABLE_MIXXXDELETED)) ||
        (column == fieldIndex(LIBRARYTABLE_HEADERPARSED)) ||
        (column == fieldIndex(LIBRARYTABLE_PLAYED)) ||
        (column == fieldIndex(LIBRARYTABLE_BPM_LOCK)) ||
        (column == fieldIndex(LIBRARYTABLE_CHANNELS)) ||
        (column == fieldIndex(TRACKLOCATIONSTABLE_FSDELETED)) ||
        (PlayerManager::numPreviewDecks() == 0 && column == fieldIndex("preview"))) {
        return true;
    }

    return false;
}

bool LibraryTableModel::isColumnHiddenByDefault(int column) {
    if (column == fieldIndex(LIBRARYTABLE_KEY))
        return true;
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
