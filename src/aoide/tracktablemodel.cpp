#include "aoide/tracktablemodel.h"

#include <QRegularExpression>

#include "aoide/web/searchcollectedtrackstask.h"
#include "library/coverartdelegate.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "mixer/playermanager.h"
#include "track/bpm.h"
#include "util/assert.h"
#include "util/duration.h"
#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("aoide TrackTableModel");

const char* const kSettingsNamespace = "aoide";

// The initial search request seems to take longer than
// subsequent requests!? To prevent a timeout without a
// result we need to be generous here.
constexpr int kSearchTimeoutMillis = 60000;

// Currently complete track objects are deserialized from the database,
// serialized as JSON and then transmitted. Until optimizations for the
// track listing use case are in place the number of tracks that are
// loaded at once should be strictly limited to keep the UI responsive.
constexpr int kRowsPerPage = 200;

constexpr qint64 kSecondsPerHour = 60 * 60;

constexpr qint64 kResetPlayedIndicatorAfterSeconds = 6 * kSecondsPerHour;

} // anonymous namespace

namespace aoide {

namespace {

const TrackTableModel::RowItem kEmptyItem;

} // anonymous namespace

TrackTableModel::RowItem::RowItem(
        json::TrackEntity&& argEntity,
        const mixxx::TaggingConfig& taggingConfig)
        : entity(std::move(argEntity)),
          facets(
                  entity.body().removeTags().toFacets().value_or(mixxx::Facets())),
          genre(
                  taggingConfig.joinTagsLabel(
                          facets,
                          mixxx::library::tags::kFacetGenre)),
          mood(
                  taggingConfig.joinTagsLabel(
                          facets,
                          mixxx::library::tags::kFacetMood)),
          comment(
                  facets.getSingleTagLabel(
                                mixxx::library::tags::kFacetComment)
                          .value_or(mixxx::library::tags::Label{})),
          grouping(
                  facets.getSingleTagLabel(
                                mixxx::library::tags::kFacetGrouping)
                          .value_or(mixxx::library::tags::Label{})) {
    facets.removeTags(mixxx::library::tags::kFacetComment);
    facets.removeTags(mixxx::library::tags::kFacetGrouping);
}

TrackTableModel::TrackTableModel(
        TrackCollectionManager* trackCollectionManager,
        Subsystem* subsystem,
        QObject* parent)
        : BaseTrackTableModel(
                  parent,
                  trackCollectionManager,
                  kSettingsNamespace),
          m_subsystem(subsystem),
          m_rowsPerPage(kRowsPerPage),
          m_canFetchMore(false),
          m_pendingRequestFirstRow(0),
          m_pendingRequestLastRow(0) {
    initTableColumnsAndHeaderProperties();
    connect(m_pTrackCollectionManager->internalCollection(),
            &TrackCollection::trackClean,
            this,
            [this](TrackId trackId) { slotTracksChangedOrRemoved(QSet<TrackId>{trackId}); });
    connect(m_pTrackCollectionManager->internalCollection(),
            &TrackCollection::trackDirty,
            this,
            [this](TrackId trackId) { slotTracksChangedOrRemoved(QSet<TrackId>{trackId}); });
    connect(m_pTrackCollectionManager->internalCollection(),
            &TrackCollection::tracksChanged,
            this,
            &TrackTableModel::slotTracksChangedOrRemoved);
    connect(m_pTrackCollectionManager->internalCollection(),
            &TrackCollection::tracksRemoved,
            this,
            &TrackTableModel::slotTracksChangedOrRemoved);
    kLogger.debug() << "Created instance" << this;
}

TrackTableModel::~TrackTableModel() {
    kLogger.debug() << "Destroying instance" << this;
}

TrackModel::Capabilities TrackTableModel::getCapabilities() const {
    return Capability::AddToTrackSet |
            Capability::AddToAutoDJ |
            Capability::EditMetadata |
            Capability::LoadToDeck |
            Capability::LoadToSampler |
            Capability::LoadToPreviewDeck |
            Capability::Hide |
            Capability::ResetPlayed;
}

bool TrackTableModel::setTrackValueForColumn(
        const TrackPointer& pTrack,
        int column,
        const QVariant& value,
        int role) {
    DEBUG_ASSERT(!"TODO");
    kLogger.warning()
            << "TODO: setTrackValueForColumn"
            << pTrack->getId()
            << column
            << value
            << role;
    return false;
}

int TrackTableModel::rowCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    DEBUG_ASSERT(!parent.isValid());
    int rowCount;
    if (m_rowItemPages.isEmpty()) {
        rowCount = 0;
    } else {
        const auto& lastPage = m_rowItemPages.last();
        rowCount = lastPage.m_firstRow + lastPage.m_rowItems.size();
    }
    return rowCount;
}

int TrackTableModel::findRowItemPageIndex(int row) const {
    DEBUG_ASSERT(row >= 0);
    if (row >= rowCount()) {
        return -1;
    }
    int lowerIndex = 0;
    int upperIndex = m_rowItemPages.size();
    while (lowerIndex < upperIndex) {
        if (lowerIndex == (upperIndex - 1)) {
            DEBUG_ASSERT(lowerIndex < m_rowItemPages.size());
            const auto& lowerItemPage = m_rowItemPages[lowerIndex];
            Q_UNUSED(lowerItemPage);
            DEBUG_ASSERT(lowerItemPage.m_firstRow <= row);
            DEBUG_ASSERT((row - lowerItemPage.m_firstRow) < lowerItemPage.m_rowItems.size());
            return lowerIndex;
        }
        auto middleIndex = lowerIndex + (upperIndex - lowerIndex) / 2;
        DEBUG_ASSERT(middleIndex < m_rowItemPages.size());
        const auto& middleItemPage = m_rowItemPages[middleIndex];
        if (row < middleItemPage.m_firstRow) {
            upperIndex = middleIndex;
        } else {
            lowerIndex = middleIndex;
        }
    }
    DEBUG_ASSERT(!"unreachable");
    return -1;
}

const TrackTableModel::RowItem& TrackTableModel::rowItem(int row) const {
    const auto pageIndex = findRowItemPageIndex(row);
    VERIFY_OR_DEBUG_ASSERT((pageIndex >= 0) && (pageIndex < m_rowItemPages.size())) {
        // Not available
        return kEmptyItem;
    }
    const auto& page = m_rowItemPages[pageIndex];
    DEBUG_ASSERT(row >= page.m_firstRow);
    const auto pageRow = row - page.m_firstRow;
    DEBUG_ASSERT(pageRow < page.m_rowItems.size());
    return page.m_rowItems[pageRow];
}

const TrackTableModel::RowItem& TrackTableModel::rowItem(
        const QModelIndex& index) const {
    int row = index.row();
    if (row < 0 || row >= rowCount()) {
        return kEmptyItem;
    }
    return rowItem(row);
}

Qt::ItemFlags TrackTableModel::flags(
        const QModelIndex& index) const {
    // TODO: Enable in-place editing if row is not stale
    return readOnlyFlags(index);
}

QVariant TrackTableModel::roleValue(
        const QModelIndex& index,
        QVariant&& rawValue,
        int role) const {
    DEBUG_ASSERT(index.isValid());
    // TODO: Display rows that might contain outdated or missing data differently?
    // if (m_staleRows.contains(index.row())) {
    //     return QVariant();
    // }
    return BaseTrackTableModel::roleValue(
            index,
            std::move(rawValue),
            role);
}

mixxx::FileInfo TrackTableModel::rowItemFileInfo(
        const RowItem& rowItem) const {
    const auto subsystem = m_subsystem.data();
    VERIFY_OR_DEBUG_ASSERT(subsystem) {
        return mixxx::FileInfo{};
    }
    const auto activeCollection = subsystem->activeCollection();
    VERIFY_OR_DEBUG_ASSERT(activeCollection) {
        return mixxx::FileInfo{};
    }
    const auto pathUrl = rowItem.entity.body().mediaSource().pathUrl();
    DEBUG_ASSERT(pathUrl.isValid());
    return mixxx::FileInfo::fromQUrl(pathUrl.toQUrl());
}

QVariant TrackTableModel::rawSiblingValue(
        const QModelIndex& index,
        ColumnCache::Column siblingField) const {
    const auto item = rowItem(index.row());
    switch (siblingField) {
    case ColumnCache::COLUMN_LIBRARYTABLE_PREVIEW: {
        if (!previewDeckTrackId().isValid()) {
            return false;
        }
        auto cachedRow = m_trackIdRowCache.value(previewDeckTrackId(), -1);
        if (cachedRow >= 0) {
            return cachedRow == index.row();
        }
        return previewDeckTrackId() == getTrackId(index);
    }
    case ColumnCache::COLUMN_LIBRARYTABLE_ALBUM: {
        const auto& titles = item.entity.body().album().titles();
        DEBUG_ASSERT(titles.size() <= 1);
        return titles.isEmpty() ? QString() : titles.first().name();
    }
    case ColumnCache::COLUMN_LIBRARYTABLE_ALBUMARTIST: {
        const auto& artists = item.entity.body().album().artists();
        DEBUG_ASSERT(artists.size() <= 1);
        return artists.isEmpty() ? QString() : artists.first().name();
    }
    case ColumnCache::COLUMN_LIBRARYTABLE_ARTIST: {
        const auto& artists = item.entity.body().artists();
        DEBUG_ASSERT(artists.size() <= 1);
        return artists.isEmpty() ? QString() : artists.first().name();
    }
    case ColumnCache::COLUMN_LIBRARYTABLE_BITRATE: {
        return QVariant::fromValue(item.entity.body().mediaSource().audioContent().bitrate());
    }
    case ColumnCache::COLUMN_LIBRARYTABLE_BPM: {
        const auto bpm = item.entity.body().musicMetrics().bpm();
        return QVariant::fromValue(bpm);
    }
    case ColumnCache::COLUMN_LIBRARYTABLE_BPM_LOCK: {
        return item.entity.body().musicMetrics().bpmLocked();
    }
    case ColumnCache::COLUMN_LIBRARYTABLE_CHANNELS: {
        return QVariant::fromValue(item.entity.body().mediaSource().audioContent().channelCount());
    }
    case ColumnCache::COLUMN_LIBRARYTABLE_COLOR: {
        return mixxx::RgbColor::toQVariant(item.entity.body().color());
    }
    case ColumnCache::COLUMN_LIBRARYTABLE_COMMENT: {
        return item.comment;
    }
    case ColumnCache::COLUMN_LIBRARYTABLE_COMPOSER: {
        const auto& actors = item.entity.body().actors(json::Actor::kRoleComposer);
        DEBUG_ASSERT(actors.size() <= 1);
        return actors.isEmpty() ? QString() : actors.first().name();
    }
    case ColumnCache::COLUMN_LIBRARYTABLE_DATETIMEADDED: {
        const auto collectedAt = item.entity.body().mediaSource().collectedAt();
        VERIFY_OR_DEBUG_ASSERT(collectedAt) {
            return QVariant{};
        }
        return *collectedAt;
    }
    case ColumnCache::COLUMN_LIBRARYTABLE_DURATION: {
        return QVariant::fromValue(item.entity.body().mediaSource().audioContent().duration());
    }
    case ColumnCache::COLUMN_LIBRARYTABLE_GENRE: {
        return item.genre;
    }
    case ColumnCache::COLUMN_LIBRARYTABLE_GROUPING: {
        return item.grouping;
    }
    case ColumnCache::COLUMN_LIBRARYTABLE_FILETYPE: {
        auto contentType = item.entity.body().mediaSource().contentTypeName();
        if (contentType.startsWith("audio/")) {
            return contentType.right(contentType.size() - 6);
        } else {
            return contentType;
        }
    }
    case ColumnCache::COLUMN_LIBRARYTABLE_KEY_ID: {
        return item.entity.body().musicMetrics().key();
    }
    case ColumnCache::COLUMN_TRACKLOCATIONSTABLE_LOCATION: {
        return rowItemFileInfo(item).location();
    }
    case ColumnCache::COLUMN_LIBRARYTABLE_RATING: {
        return QVariant::fromValue(mixxx::TrackRecord::getRatingFromFacets(item.facets));
    }
    case ColumnCache::COLUMN_LIBRARYTABLE_REPLAYGAIN: {
        return QVariant::fromValue(item.entity.body().mediaSource().audioContent().replayGain());
    }
    case ColumnCache::COLUMN_LIBRARYTABLE_SAMPLERATE: {
        return QVariant::fromValue(item.entity.body().mediaSource().audioContent().sampleRate());
    }
    case ColumnCache::COLUMN_LIBRARYTABLE_TIMESPLAYED: {
        return item.entity.body().playCounter().timesPlayed();
    }
    case ColumnCache::COLUMN_LIBRARYTABLE_LAST_PLAYED_AT: {
        return item.entity.body().playCounter().lastPlayedAt().value_or(QDateTime());
    }
    case ColumnCache::COLUMN_LIBRARYTABLE_PLAYED: {
        const auto lastPlayedAt = item.entity.body().playCounter().lastPlayedAt();
        if (!lastPlayedAt || !lastPlayedAt->isValid()) {
            // Undefined or never played before
            return false;
        }
        const auto now = QDateTime::currentDateTime();
        return lastPlayedAt->secsTo(now) < kResetPlayedIndicatorAfterSeconds;
    }
    case ColumnCache::COLUMN_LIBRARYTABLE_TITLE: {
        const auto& titles = item.entity.body().titles();
        DEBUG_ASSERT(titles.size() <= 1);
        return titles.isEmpty() ? QString() : titles.first().name();
    }
    case ColumnCache::COLUMN_LIBRARYTABLE_TRACKNUMBER: {
        return item.entity.body().trackNumbers();
    }
    case ColumnCache::COLUMN_LIBRARYTABLE_YEAR: {
        return item.entity.body().release().releasedAt();
    }
    case ColumnCache::COLUMN_LIBRARYTABLE_COVERART:
    case ColumnCache::COLUMN_LIBRARYTABLE_KEY:
        // Skip
        break;
    default:
        kLogger.critical()
                << "Unmapped field"
                << siblingField
                << '@'
                << index.row();
        DEBUG_ASSERT(!"unreachable");
        break;
    }
    return QVariant();
}

bool TrackTableModel::canFetchMore(
        const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return m_canFetchMore;
}

void TrackTableModel::fetchMore(
        const QModelIndex& parent) {
    VERIFY_OR_DEBUG_ASSERT(canFetchMore(parent)) {
        return;
    }
    if (m_pendingSearchTask) {
        // Await the pending search results and ignore all
        // intermediate requests
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_canFetchMore) {
        kLogger.debug()
                << "No more rows available for fetching";
        return;
    }
    m_pendingRequestFirstRow = rowCount();
    DEBUG_ASSERT(m_rowsPerPage > 0);
    m_pendingRequestLastRow = m_pendingRequestFirstRow + (m_rowsPerPage - 1);
    Pagination pagination;
    pagination.offset = m_pendingRequestFirstRow;
    pagination.limit = (m_pendingRequestLastRow - m_pendingRequestFirstRow) + 1;
    startNewSearch(pagination);
}

void TrackTableModel::startNewSearch(
        const Pagination& pagination) {
    abortPendingSearch();
    DEBUG_ASSERT(!m_pendingSearchTask);
    auto* const pendingSearchTask = m_subsystem.data()->searchTracks(
            m_baseQuery,
            m_searchOverlayFilter,
            m_searchTerms,
            pagination);
    DEBUG_ASSERT(pendingSearchTask);
    connect(pendingSearchTask,
            &SearchCollectedTracksTask::succeeded,
            this,
            &TrackTableModel::slotSearchTracksSucceeded,
            Qt::UniqueConnection);
    pendingSearchTask->invokeStart(kSearchTimeoutMillis);
    m_pendingSearchTask = pendingSearchTask;
}

namespace {

const QRegularExpression kRegexpWhitespace("\\s+");

} // anonymous namespace

void TrackTableModel::abortPendingSearch() {
    auto* const pendingSearchTask = m_pendingSearchTask.data();
    if (pendingSearchTask) {
        kLogger.debug()
                << "Aborting pending search task"
                << pendingSearchTask;
        pendingSearchTask->invokeAbort();
        // FIXME: Who will finally delete the task or do we have
        // a memory leak? Invoking pendingSearchTask->deleteLater()
        // at this point will cause spurious SIGSEGV crashes!!
        m_pendingSearchTask.clear();
    }
}

void TrackTableModel::reset() {
    beginResetModel();
    abortPendingSearch();
    clearRowItems();
    m_collectionUid = QString();
    m_baseQuery = QJsonObject();
    m_searchText = QString();
    m_canFetchMore = false;
    m_pendingRequestFirstRow = 0;
    m_pendingRequestLastRow = 0;
    endResetModel();
}

void TrackTableModel::clearRows() {
    if (rowCount() <= 0) {
        return;
    }
    beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
    clearRowItems();
    endRemoveRows();
}

void TrackTableModel::searchTracks(
        const QJsonObject& baseQuery,
        const TrackSearchOverlayFilter& overlayFilter,
        const QString& searchText) {
    if (!m_subsystem.data()->activeCollection()) {
        kLogger.warning()
                << "Search not available without an active collection";
        return;
    }
    abortPendingSearch();
    DEBUG_ASSERT(m_rowsPerPage > 0);
    Pagination pagination;
    pagination.offset = 0;
    pagination.limit = m_rowsPerPage;
    m_collectionUid = m_subsystem.data()->activeCollection()->header().uid();
    m_baseQuery = baseQuery;
    m_searchOverlayFilter = overlayFilter;
    m_searchText = searchText;
    // TODO: Parse the query string
    m_searchTerms = m_searchText.split(
            kRegexpWhitespace,
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            Qt::SkipEmptyParts);
#else
            QString::SkipEmptyParts);
#endif
    m_canFetchMore = true;
    m_pendingRequestFirstRow = pagination.offset;
    m_pendingRequestLastRow = m_pendingRequestFirstRow + (pagination.limit - 1);
    startNewSearch(pagination);
}

void TrackTableModel::clearRowItems() {
    m_rowItemPages.clear();
    m_staleRows.clear();
    m_staleTrackIds.clear();
    m_trackIdRowCache.clear();
}

void TrackTableModel::slotSearchTracksSucceeded(
        const QJsonArray& searchResults) {
    auto* const finishedSearchTask = qobject_cast<SearchCollectedTracksTask*>(sender());
    if (!finishedSearchTask) {
        // The sender might have been dropped already
        return;
    }
    const auto finishedSearchTaskDeleter = mixxx::ScopedDeleteLater(finishedSearchTask);

    VERIFY_OR_DEBUG_ASSERT(finishedSearchTask == m_pendingSearchTask.data()) {
        // Previously aborted?
        return;
    }
    m_pendingSearchTask.clear();

    if (m_pendingRequestFirstRow == 0) {
        clearRows();
    }
    DEBUG_ASSERT(m_pendingRequestFirstRow == rowCount());
    kLogger.debug()
            << "Received"
            << searchResults.size()
            << "search results from subsystem";
    if (searchResults.isEmpty()) {
        // No more results available
        m_canFetchMore = false;
        return;
    }
    int firstRow = m_pendingRequestFirstRow;
    DEBUG_ASSERT(searchResults.size() >= 1);
    int lastRow = m_pendingRequestFirstRow + (searchResults.size() - 1);
    if (lastRow < m_pendingRequestLastRow) {
        // No more results available
        m_canFetchMore = false;
    }
    beginInsertRows(QModelIndex(), firstRow, lastRow);
    QVector<RowItem> rowItems;
    rowItems.reserve(searchResults.size());
    for (const auto& searchResult : searchResults) {
        // TODO: Improve parsing and validation
        DEBUG_ASSERT(searchResult.isArray());
        auto entity = json::TrackEntity(searchResult.toArray());
        rowItems += RowItem(
                std::move(entity),
                m_pTrackCollectionManager->taggingConfig());
    }
    m_rowItemPages += RowItemPage(m_pendingRequestFirstRow, std::move(rowItems));
    endInsertRows();
    if (m_trackIdRowCache.capacity() <= 0) {
        // Initially reserve some capacity for row cache
        m_trackIdRowCache.reserve(rowCount());
    }
}

mixxx::FileInfo TrackTableModel::getTrackFileInfoByRow(int row) const {
    DEBUG_ASSERT(row >= 0);
    VERIFY_OR_DEBUG_ASSERT(row < rowCount()) {
        return mixxx::FileInfo{};
    }
    return rowItemFileInfo(rowItem(row));
}

TrackId TrackTableModel::getTrackIdByRow(int row) const {
    const auto trackFileRef = getTrackFileRefByRow(row);
    if (!trackFileRef.isValid()) {
        return TrackId();
    }
    if (kLogger.debugEnabled()) {
        kLogger.debug()
                << "Looking up id of track in internal collection:"
                << trackFileRef;
    }
    DEBUG_ASSERT(m_pTrackCollectionManager);
    const auto trackId =
            m_pTrackCollectionManager->internalCollection()->getTrackIdByRef(
                    trackFileRef);
    if (trackId.isValid()) {
        const auto iter = m_trackIdRowCache.find(trackId);
        // Each track is expected to appear only once, i.e. no duplicates!
        DEBUG_ASSERT(
                iter == m_trackIdRowCache.end() ||
                iter.value() == row);
        if (iter == m_trackIdRowCache.end()) {
            m_trackIdRowCache.insert(trackId, row);
            if (m_staleTrackIds.contains(trackId)) {
                invalidateRow(row);
            }
        }
    }
    return trackId;
}

TrackRef TrackTableModel::getTrackFileRef(const QModelIndex& index) const {
    if (!index.isValid()) {
        return TrackRef();
    }
    return getTrackFileRefByRow(index.row());
}

TrackPointer TrackTableModel::getTrack(const QModelIndex& index) const {
    const auto trackRef = getTrackFileRef(index);
    if (!trackRef.isValid()) {
        return TrackPointer();
    }
    if (kLogger.debugEnabled()) {
        kLogger.debug()
                << "Loading track from internal collection:"
                << trackRef;
    }
    DEBUG_ASSERT(m_pTrackCollectionManager);
    return m_pTrackCollectionManager->getTrackByRef(trackRef);
}

TrackId TrackTableModel::getTrackId(const QModelIndex& index) const {
    if (!index.isValid()) {
        return TrackId();
    }
    return getTrackIdByRow(index.row());
}

QString TrackTableModel::getTrackLocation(const QModelIndex& index) const {
    return getTrackFileRef(index).getLocation();
}

CoverInfo TrackTableModel::getCoverInfo(
        const QModelIndex& index) const {
    CoverInfo coverInfo;
    coverInfo.trackLocation = getTrackLocation(index);
    const auto artwork = rowItem(index).entity.body().mediaSource().artwork();
    if (artwork.isEmpty()) {
        return coverInfo;
    }
    const auto artworkImage = artwork.image();
    if (artworkImage.isEmpty()) {
        return coverInfo;
    }
    const auto uri = artwork.uri();
    if (uri.isValid()) {
        coverInfo.coverLocation =
                mixxx::FileInfo::fromQUrl(uri.toQUrl()).location();
    }
    const QImage thumbnail = artworkImage.thumbnail();
    if (!thumbnail.isNull()) {
        coverInfo.color = mixxx::RgbColor::fromQColor(
                mixxx::extractImageBackgroundColor(thumbnail));
    }
    coverInfo.setImageDigest(artworkImage.digest());
    DEBUG_ASSERT(coverInfo.imageDigest().isEmpty() == thumbnail.isNull());
    if (coverInfo.imageDigest().isEmpty()) {
        DEBUG_ASSERT(artwork.source().isEmpty() ||
                artwork.source() == QStringLiteral("missing"));
    } else {
        DEBUG_ASSERT(!artwork.source().isEmpty());
        if (coverInfo.coverLocation.isEmpty()) {
            DEBUG_ASSERT(artwork.source() == QStringLiteral("embedded"));
            coverInfo.type = CoverInfo::Type::METADATA;
        } else {
            DEBUG_ASSERT(artwork.source() == QStringLiteral("linked"));
            coverInfo.type = CoverInfo::Type::FILE;
        }
    }
    // The following properties are not available
    DEBUG_ASSERT(coverInfo.source == CoverInfo::Source::UNKNOWN);
    DEBUG_ASSERT(coverInfo.legacyHash() == CoverInfo::defaultLegacyHash());
    return coverInfo;
}

QImage TrackTableModel::getCoverThumbnail(
        const QModelIndex& index) const {
    return rowItem(index).entity.body().mediaSource().artwork().image().thumbnail();
}

const QVector<int> TrackTableModel::getTrackRows(TrackId trackId) const {
    // Each track is expected to appear only once, i.e. no duplicates!
    QVector<int> rows;
    VERIFY_OR_DEBUG_ASSERT(trackId.isValid()) {
        return rows;
    }
    int row = m_trackIdRowCache.value(trackId, -1);
    if (row >= 0) {
        DEBUG_ASSERT(row < rowCount());
        rows.append(row);
    } else {
        // Not cached -> full table scan
        kLogger.debug()
                << "Starting full table scan to"
                << "find row of track with id"
                << trackId;
        for (row = 0; row < rowCount(); ++row) {
            if (getTrackIdByRow(row) == trackId) {
                kLogger.debug()
                        << "Found track with id"
                        << trackId
                        << "in row"
                        << row;
                rows.append(row);
                break;
            }
        }
    }
    return rows;
}

void TrackTableModel::search(const QString& searchText, const QString& extraFilter) {
    Q_UNUSED(extraFilter) // only used in DEBUG_ASSERT
    DEBUG_ASSERT(extraFilter.isEmpty());
    m_searchText = searchText;
    select();
}

const QString TrackTableModel::currentSearch() const {
    return m_searchText;
}

bool TrackTableModel::isColumnInternal(int column) {
    return column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PLAYED) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM_LOCK) ||
            (PlayerManager::numPreviewDecks() == 0 &&
                    column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PREVIEW));
}

void TrackTableModel::select() {
    searchTracks(m_searchOverlayFilter, m_searchText);
}

void TrackTableModel::emitRowDataChanged(
        int row) {
    const auto topLeft = index(row, 0);
    const auto bottomRight = index(row, columnCount() - 1);
    emit dataChanged(topLeft, bottomRight);
}

void TrackTableModel::invalidateRow(
        int row) const {
    if (m_staleRows.contains(row)) {
        return;
    }
    m_staleRows.insert(row);
    const_cast<TrackTableModel*>(this)->emitRowDataChanged(row);
}

void TrackTableModel::invalidateTrackId(
        TrackId trackId) const {
    DEBUG_ASSERT(trackId.isValid());
    m_staleTrackIds.insert(trackId);
    auto row = m_trackIdRowCache.value(trackId, -1);
    if (row < 0) {
        // Row not cached, but might still be visible
        // TODO: How to find and invalidate the corresponding
        // rows?
        return;
    }
    invalidateRow(row);
}

void TrackTableModel::slotTracksChangedOrRemoved(
        QSet<TrackId> trackIds) {
    for (auto trackId : qAsConst(trackIds)) {
        invalidateTrackId(trackId);
    }
}

} // namespace aoide
