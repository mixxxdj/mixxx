#include "library/librarytablemodel.h"

#include "library/dao/trackschema.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "mixer/playerinfo.h"
#include "moc_librarytablemodel.cpp"

namespace {

const QString kTableName = QStringLiteral("library_view");
// Track filter: exclude hidden and missing tracks
const QString kTrackFilter = QStringLiteral("(mixxx_deleted=0 AND fs_deleted=0)");

} // anonymous namespace

LibraryTableModel::LibraryTableModel(QObject* parent,
        TrackCollectionManager* pTrackCollectionManager,
        const char* settingsNamespace)
        : BaseSqlTableModel(parent, pTrackCollectionManager, settingsNamespace) {
    setTableModel();
}

LibraryTableModel::~LibraryTableModel() {
}

void LibraryTableModel::setTableModel() {
    QStringList columns;
    columns << "library." + LIBRARYTABLE_ID
            << "'' AS " + LIBRARYTABLE_PREVIEW
            // For sorting the cover art column we give LIBRARYTABLE_COVERART
            // the same value as the cover digest.
            << LIBRARYTABLE_COVERART_DIGEST + " AS " + LIBRARYTABLE_COVERART;

    QSqlQuery query(m_database);
    const QString queryString = QStringLiteral(
            "CREATE TEMPORARY VIEW IF NOT EXISTS %1"
            " AS SELECT %2"
            " FROM library "
            "INNER JOIN track_locations "
            "ON library.location=track_locations.id "
            "WHERE %3")
                                        .arg(kTableName,
                                                columns.join(","),
                                                kTrackFilter);
    query.prepare(queryString);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }

    QStringList tableColumns;
    tableColumns << LIBRARYTABLE_ID;
    tableColumns << LIBRARYTABLE_PREVIEW;
    tableColumns << LIBRARYTABLE_COVERART;
    setTable(kTableName,
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
    // Note(ronso0) When adding capabilities, also check getCapabilities() in
    // BrowseLibraryTableModel which builds on this class.
    // For example BrowseLibraryTableModel does not accept drops and does not
    // allow to hide tracks (it always shows ALL tracks).
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
