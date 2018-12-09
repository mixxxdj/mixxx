
#include "library/librarytablemodel.h"
#include "library/queryutil.h"
#include "library/dao/trackschema.h"
#include "mixer/playermanager.h"

namespace {

const QString kDefaultLibraryFilter =
        "mixxx_deleted=0 AND fs_deleted=0";

} // anonymous namespace

LibraryTableModel::LibraryTableModel(QObject* parent,
                                     TrackCollection* pTrackCollection,
                                     const char* settingsNamespace)
        : BaseSqlTableModel(parent, pTrackCollection, settingsNamespace) {
    setTableModel();
}

LibraryTableModel::~LibraryTableModel() {
}

void LibraryTableModel::setTableModel(int id) {
    Q_UNUSED(id);
    QStringList columns;
    columns << "library." + LIBRARYTABLE_ID
            << "'' AS " + LIBRARYTABLE_PREVIEW
            // For sorting the cover art column we give LIBRARYTABLE_COVERART
            // the same value as the cover hash.
            << LIBRARYTABLE_COVERART_HASH + " AS " + LIBRARYTABLE_COVERART;

    const QString tableName = "library_view";

    QSqlQuery query(m_pTrackCollection->database());
    QString queryString = "CREATE TEMPORARY VIEW IF NOT EXISTS " + tableName + " AS "
            "SELECT " + columns.join(", ") +
            " FROM library INNER JOIN track_locations "
            "ON library.location = track_locations.id "
            "WHERE (" + kDefaultLibraryFilter + ")";
    query.prepare(queryString);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }

    QStringList tableColumns;
    tableColumns << LIBRARYTABLE_ID;
    tableColumns << LIBRARYTABLE_PREVIEW;
    tableColumns << LIBRARYTABLE_COVERART;
    setTable(tableName, LIBRARYTABLE_ID, tableColumns,
             m_pTrackCollection->getTrackSource());
    setSearch("");
    setDefaultSort(fieldIndex("artist"), Qt::AscendingOrder);

    // Set tooltip for random sorting
    int fi = fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PREVIEW);
    setHeaderData(fi, Qt::Horizontal, tr("Sort items randomly"), Qt::ToolTipRole);
}


int LibraryTableModel::addTracks(const QModelIndex& index,
                                 const QList<QString>& locations) {
    Q_UNUSED(index);
    QList<QFileInfo> fileInfoList;
    foreach (QString fileLocation, locations) {
        fileInfoList.append(QFileInfo(fileLocation));
    }
    QList<TrackId> trackIds = m_pTrackCollection->getTrackDAO().addMultipleTracks(fileInfoList, true);
    select();
    return trackIds.size();
}

bool LibraryTableModel::isColumnInternal(int column) {
    if ((column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ID)) ||
            (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_URL)) ||
            (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_CUEPOINT)) ||
            (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_WAVESUMMARYHEX)) ||
            (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_SAMPLERATE)) ||
            (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_MIXXXDELETED)) ||
            (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_HEADERPARSED)) ||
            (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PLAYED)) ||
            (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_KEY_ID))||
            (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM_LOCK)) ||
            (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_CHANNELS)) ||
            (column == fieldIndex(ColumnCache::COLUMN_TRACKLOCATIONSTABLE_FSDELETED)) ||
            (PlayerManager::numPreviewDecks() == 0 &&
             column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PREVIEW)) ||
            (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_SOURCE)) ||
            (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_TYPE)) ||
            (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_LOCATION)) ||
            (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_HASH))) {
        return true;
    }

    return false;
}

TrackModel::CapabilitiesFlags LibraryTableModel::getCapabilities() const {
    return TRACKMODELCAPS_NONE
            | TRACKMODELCAPS_RECEIVEDROPS
            | TRACKMODELCAPS_ADDTOPLAYLIST
            | TRACKMODELCAPS_ADDTOCRATE
            | TRACKMODELCAPS_ADDTOAUTODJ
            | TRACKMODELCAPS_EDITMETADATA
            | TRACKMODELCAPS_LOADTODECK
            | TRACKMODELCAPS_LOADTOSAMPLER
            | TRACKMODELCAPS_LOADTOPREVIEWDECK
            | TRACKMODELCAPS_HIDE
            | TRACKMODELCAPS_RESETPLAYED;
}
