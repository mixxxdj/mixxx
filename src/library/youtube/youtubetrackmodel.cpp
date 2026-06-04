#include "library/youtube/youtubetrackmodel.h"

#include <QFileInfo>
#include <QSharedPointer>
#include <QString>
#include <QUrl>

#include "library/baseexternaltrackmodel.h"
#include "library/basetrackcache.h"
#include "library/columncache.h"
#include "library/coverart.h"
#include "library/dao/trackschema.h"
#include "library/trackcollectionmanager.h"
#include "moc_youtubetrackmodel.cpp"
#include "track/track.h"

const QString YouTubeTrackModel::kPlaceholderScheme =
        QStringLiteral("youtube://");

YouTubeTrackModel::YouTubeTrackModel(QObject* parent,
        TrackCollectionManager* pTrackCollectionManager,
        QSharedPointer<BaseTrackCache> trackSource)
        : BaseExternalTrackModel(parent,
                  pTrackCollectionManager,
                  "mixxx.db.model.youtube",
                  "youtube_library",
                  trackSource) {
}

TrackPointer YouTubeTrackModel::getTrack(const QModelIndex& index) const {
    // Use the BaseSqlTableModel raw-location accessor so we see exactly
    // what's stored in the youtube_library row, without the
    // QDir::fromNativeSeparators() massaging that getTrackLocation()
    // applies (which would corrupt the "youtube://" scheme).
    const QString rawLocation = getFieldString(
            index, ColumnCache::COLUMN_TRACKLOCATIONSTABLE_LOCATION);
    if (rawLocation.startsWith(kPlaceholderScheme)) {
        // Return null — callers that only need metadata (e.g. context-menu
        // builders) must not trigger unintended downloads. Actual load
        // actions go through getTrackUrl() and the loadTrackLocationToPlayer
        // signal path which carries the target deck group.
        return TrackPointer();
    }
    return BaseExternalTrackModel::getTrack(index);
}

QUrl YouTubeTrackModel::getTrackUrl(const QModelIndex& index) const {
    const QString rawLocation = getFieldString(
            index, ColumnCache::COLUMN_TRACKLOCATIONSTABLE_LOCATION);
    if (rawLocation.startsWith(kPlaceholderScheme)) {
        QUrl url;
        url.setScheme(QStringLiteral("youtube"));
        url.setPath(rawLocation.mid(kPlaceholderScheme.size()));
        return url;
    }
    return BaseExternalTrackModel::getTrackUrl(index);
}

TrackId YouTubeTrackModel::getTrackId(const QModelIndex& index) const {
    const QString rawLocation = getFieldString(
            index, ColumnCache::COLUMN_TRACKLOCATIONSTABLE_LOCATION);
    if (rawLocation.startsWith(kPlaceholderScheme)) {
        return TrackId();
    }
    TrackPointer pTrack = BaseExternalTrackModel::getTrack(index);
    return pTrack ? pTrack->getId() : TrackId();
}

TrackModel::Capabilities YouTubeTrackModel::getCapabilities() const {
    // Include Analyze so the right-click Analyze submenu (and its
    // "Download and Analyze" action) is shown for YouTube rows.
    // We intentionally omit EditMetadata — cells remain read-only;
    // analysis results are written back via Track signals, not setData().
    return BaseExternalTrackModel::getCapabilities() | Capability::Analyze;
}

void YouTubeTrackModel::search(const QString& searchText) {
    // Forward filter to the SQL backing — narrows already-loaded rows so
    // there's instant feedback even while the network call is in flight.
    BaseExternalTrackModel::search(searchText);
    // Then ask the feature to fire a fresh YouTube search with the same
    // query. Empty queries are common (the search box clears when the user
    // backspaces everything) — skip the network call in that case.
    const QString trimmed = searchText.trimmed();
    if (trimmed.isEmpty()) {
        return;
    }
    emit searchRequested(trimmed);
}

QString YouTubeTrackModel::resolveLocation(const QString& nativeLocation) const {
    // Placeholders must NEVER be passed through as a real path — that would
    // make BaseSqlTableModel::getTrackLocation return "youtube://VIDEOID"
    // and downstream code would try to fopen() it. Return an empty string
    // so any "do I have a real file for this row?" caller sees "no". The
    // placeholder path through getTrack() above handles activation.
    if (nativeLocation.startsWith(kPlaceholderScheme)) {
        return QString();
    }
    return BaseExternalTrackModel::resolveLocation(nativeLocation);
}

void YouTubeTrackModel::setThumbnailDir(const QString& dir) {
    m_thumbnailDir = dir;
}

void YouTubeTrackModel::setHasMore(bool hasMore) {
    m_hasMore = hasMore;
}

bool YouTubeTrackModel::canFetchMore(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return false; // table model: only root
    }
    return m_hasMore;
}

void YouTubeTrackModel::fetchMore(const QModelIndex& parent) {
    if (parent.isValid() || !m_hasMore) {
        return;
    }
    // Prevent duplicate fetches until the service responds and setHasMore is
    // called again with the next token's availability.
    m_hasMore = false;
    emit fetchMoreRequested();
}

CoverInfo YouTubeTrackModel::getCoverInfo(const QModelIndex& index) const {
    // Use the thumbnail cached in m_thumbnailDir when available. The videoId
    // is stored in the comment column so we can derive the thumbnail path
    // without an extra column or schema change. Both downloaded rows (real
    // location path) and placeholder rows (youtube://VIDEOID) carry the
    // videoId in comment — it's set unconditionally in replaceTrackTable().
    if (m_thumbnailDir.isEmpty()) {
        return CoverInfo();
    }
    const QString videoId = getFieldString(
            index, ColumnCache::COLUMN_LIBRARYTABLE_COMMENT);
    if (videoId.isEmpty()) {
        return CoverInfo();
    }
    const QString thumbPath =
            m_thumbnailDir + QLatin1Char('/') + videoId + QStringLiteral(".jpg");
    if (!QFileInfo::exists(thumbPath)) {
        return CoverInfo();
    }

    CoverInfo coverInfo;
    coverInfo.type = CoverInfo::FILE;
    coverInfo.source = CoverInfo::GUESSED;
    // Use the videoId bytes as a stable per-video digest. This gives each
    // thumbnail a unique QPixmapCache key derived from the videoId rather
    // than the image content, which is fine — we never write these back to
    // the main library DB, so a content mismatch doesn't matter.
    coverInfo.setImageDigest(videoId.toUtf8(), /*legacyHash=*/0);
    // Absolute path — avoids the "relative to trackLocation" join that would
    // fail for placeholder rows where trackLocation is empty.
    coverInfo.coverLocation = thumbPath;
    return coverInfo;
}
