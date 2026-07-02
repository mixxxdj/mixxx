#include "library/librarytablemodel.h"

#include "library/dao/trackschema.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "moc_librarytablemodel.cpp"

LibraryTableModel::LibraryTableModel(QObject* parent,
        TrackCollectionManager* pTrackCollectionManager,
        const char* settingsNamespace)
        : BaseSqlTableModel(parent, pTrackCollectionManager, settingsNamespace) {
    setTableModel();
}

LibraryTableModel::~LibraryTableModel() {
}

void LibraryTableModel::setTableModel() {
    const QString tableName("library_view");

    QStringList columns;
    columns << "library." + LIBRARYTABLE_ID
            << "'' AS " + LIBRARYTABLE_PREVIEW
            // For sorting the cover art column we give LIBRARYTABLE_COVERART
            // the same value as the cover digest.
            << "library." + LIBRARYTABLE_COVERART_DIGEST + " AS " + LIBRARYTABLE_COVERART
            << "(cg.group_id || ' - ' || cmrt_canonical.artist || ' - ' || "
               "cmrt_canonical.title) AS " +
                    LIBRARYTABLE_CMRT_NAME
            << "(cg.canonical_track_id = library.id) AS " +
                    LIBRARYTABLE_CMRT_CANONICAL
            << "CASE WHEN cg.group_id IS NOT NULL "
               "THEN cm.offset_from_canonical ELSE NULL END AS " +
                    LIBRARYTABLE_CMRT_OFFSET;

    QSqlQuery query(m_database);
    query.prepare(
            "CREATE TEMPORARY VIEW IF NOT EXISTS " + tableName +
            " AS SELECT " + columns.join(",") +
            " FROM library "
            "INNER JOIN track_locations "
            "ON library.location=track_locations.id "
            "LEFT JOIN cmrt_members cm ON library.id = cm.track_id "
            "LEFT JOIN cmrt_groups cg "
            "  ON cm.group_id = cg.group_id AND cg.track_count > 1 "
            "LEFT JOIN library cmrt_canonical "
            "  ON cg.canonical_track_id = cmrt_canonical.id "
            "WHERE (library.mixxx_deleted=0 AND fs_deleted=0)");
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }

    QStringList tableColumns;
    tableColumns << LIBRARYTABLE_ID;
    tableColumns << LIBRARYTABLE_PREVIEW;
    tableColumns << LIBRARYTABLE_COVERART;
    tableColumns << LIBRARYTABLE_CMRT_NAME;
    tableColumns << LIBRARYTABLE_CMRT_CANONICAL;
    tableColumns << LIBRARYTABLE_CMRT_OFFSET;
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

int LibraryTableModel::addTracks(const QModelIndex& index,
        const QList<QString>& locations) {
    Q_UNUSED(index);
    const QList<TrackId> trackIds = m_pTrackCollectionManager->resolveTrackIdsFromLocations(
            locations);
    if (trackIds.size() > 0) {
        select();
    }
    return trackIds.size();
}

bool LibraryTableModel::isColumnInternal(int column) {
    return column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ID) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_URL) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_CUEPOINT) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_SAMPLERATE) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_MIXXXDELETED) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_HEADERPARSED) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PLAYED) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_KEY_ID) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM_LOCK) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BEATS_VERSION) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_CHANNELS) ||
            column == fieldIndex(ColumnCache::COLUMN_TRACKLOCATIONSTABLE_DIRECTORY) ||
            column == fieldIndex(ColumnCache::COLUMN_TRACKLOCATIONSTABLE_FSDELETED) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_CMRT_CANONICAL) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_CMRT_OFFSET) ||
            (PlayerInfo::instance().numPreviewDecks() == 0 &&
                    column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PREVIEW)) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_SOURCE) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_TYPE) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_LOCATION) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_COLOR) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_DIGEST) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_HASH);
}

TrackModel::Capabilities LibraryTableModel::getCapabilities() const {
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

void LibraryTableModel::select() {
    BaseSqlTableModel::select();
    emit updateTrackCount();
}
