#include "library/browse/browselibrarytablemodel.h"

#include "library/dao/trackschema.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "mixer/playermanager.h"
#include "moc_browselibrarytablemodel.cpp"
#include "recording/recordingmanager.h"

BrowseLibraryTableModel::BrowseLibraryTableModel(
        QObject* pParent,
        TrackCollectionManager* pTrackCollectionManager)
        : BaseSqlTableModel(pParent, pTrackCollectionManager, "mixxx.db.model.browseLibrary") {
    setTableModel();
}

void BrowseLibraryTableModel::setTableModel(int id) {
    // copied from LibraryTableModel
    Q_UNUSED(id);
    const QString tableName("browse_library_view");

    QStringList columns;
    columns << "library." + LIBRARYTABLE_ID
            << "'' AS " + LIBRARYTABLE_PREVIEW
            // For sorting the cover art column we give LIBRARYTABLE_COVERART
            // the same value as the cover digest.
            << LIBRARYTABLE_COVERART_DIGEST + " AS " + LIBRARYTABLE_COVERART;

    QSqlQuery query(m_database);
    query.prepare(
            "CREATE TEMPORARY VIEW IF NOT EXISTS " + tableName +
            " AS SELECT " + columns.join(",") +
            " FROM library "
            "INNER JOIN track_locations "
            "ON library.location=track_locations.id "
            "WHERE fs_deleted=0");
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }

    QStringList tableColumns;
    tableColumns << LIBRARYTABLE_ID;
    tableColumns << LIBRARYTABLE_PREVIEW;
    tableColumns << LIBRARYTABLE_COVERART;
    setTable(tableName,
            LIBRARYTABLE_ID,
            std::move(tableColumns),
            m_pTrackCollectionManager->internalCollection()->getTrackSource());
    setSearch("");
    setDefaultSort(fieldIndex("artist"), Qt::AscendingOrder);

    // Set tooltip for random sorting
    int fi = fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PREVIEW);
    setHeaderData(fi, Qt::Horizontal, tr("Sort items randomly"), Qt::ToolTipRole);
}

void BrowseLibraryTableModel::setPath(QString path) {
    while (path.endsWith('/')) {
        path.chop(1);
    }
    FieldEscaper escaper(m_pTrackCollectionManager->internalCollection()->database());
    // Note: don't use leading or trailing '%' for the LIKE pattern. We only want
    // tracks from exactly that path, not from it's children or any other path
    // that coincidentally contains 'path'.
    const QString escapedDirPath = escaper.escapeString(path);
    // Note: don't use 'LIKE' to find matches, that would interpret trailing _
    // as wildcard, but for some reason that would not match _
    // Use '=' which is reported to be more efficient anyway.
    m_directoryFilter = QStringLiteral(
            "%1 IN (SELECT %2.%1 "
            "FROM %2 JOIN %3 "
            "ON %2.%4=%3.id "
            "WHERE %3.%5 = %6)")
                                .arg(LIBRARYTABLE_ID,
                                        LIBRARY_TABLE,
                                        TRACKLOCATIONS_TABLE,
                                        LIBRARYTABLE_LOCATION,
                                        TRACKLOCATIONSTABLE_DIRECTORY,
                                        escapedDirPath);
}

void BrowseLibraryTableModel::search(const QString& searchText, const QString& /* extraFilter */) {
    BaseSqlTableModel::search(searchText, m_directoryFilter);
}

bool BrowseLibraryTableModel::isColumnInternal(int column) {
    // copied from LibraryTableModel
    return column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ID) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_URL) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_CUEPOINT) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_SAMPLERATE) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_MIXXXDELETED) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_HEADERPARSED) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PLAYED) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_KEY_ID) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM_LOCK) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_CHANNELS) ||
            column == fieldIndex(ColumnCache::COLUMN_TRACKLOCATIONSTABLE_FSDELETED) ||
            (PlayerManager::numPreviewDecks() == 0 &&
                    column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PREVIEW)) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_SOURCE) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_TYPE) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_LOCATION) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_COLOR) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_DIGEST) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_HASH);
}

TrackModel::Capabilities BrowseLibraryTableModel::getCapabilities() const {
    // copied from LibraryTableModel
    return Capability::ReceiveDrops |
            Capability::AddToTrackSet |
            Capability::AddToAutoDJ |
            Capability::EditMetadata |
            Capability::LoadToDeck |
            Capability::LoadToSampler |
            Capability::LoadToPreviewDeck |
            Capability::Hide |
            Capability::ResetPlayed |
            Capability::RemoveFromDisk |
            Capability::Analyze |
            Capability::Properties |
            Capability::Sorting;
}
