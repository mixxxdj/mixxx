#pragma once

#include <QObject>
#include <QSharedPointer>
#include <QString>
#include <QTimer>
#include <QUrl>
#include <QVariant>

#include "library/baseexternaltrackmodel.h"
#include "library/basetrackcache.h"
#include "library/coverart.h"
#include "library/trackcollectionmanager.h"

namespace mixxx {
struct YouTubeVideoInfo;
}

/// Track-table model for the YouTube library feature. Backed by the
/// persistent `youtube_library` SQL table (see schema.xml revision 41) so
/// that the standard `WTrackTableView` UI Just Works — sortable columns,
/// drag-to-deck, right-click → Add to Auto DJ, BPM/Key after analysis, etc.
///
/// Two kinds of rows live in `youtube_library`:
///
///   1. **Downloaded** rows whose `location` is a real path on disk under
///      the YouTube cache dir. These behave exactly like any other external
///      library track: `getTrack()` calls `getOrAddTrack(location)` and the
///      deck loads the file directly.
///
///   2. **Placeholder** rows whose `location` is the sentinel
///      `youtube://VIDEOID`. `getTrack()` returns null so callers that only
///      need to read metadata don't trigger inadvertent downloads. Actual
///      load actions (double-click, drag-to-deck, right-click → Load to Deck)
///      go through `getTrackUrl()` which returns a `youtube://VIDEOID` QUrl;
///      `WTrackTableView` and `WTrackMenu` detect that scheme and dispatch
///      `loadTrackLocationToPlayer` so the feature downloads and loads the
///      track to the correct deck.
class YouTubeTrackModel : public BaseExternalTrackModel {
    Q_OBJECT
  public:
    /// Sentinel location-column scheme for not-yet-downloaded rows. Stored
    /// as the row's `location` because `BaseSqlTableModel` enforces a
    /// UNIQUE on that column — using the videoId in the URL guarantees
    /// uniqueness without an extra index.
    static const QString kPlaceholderScheme;

    YouTubeTrackModel(QObject* parent,
            TrackCollectionManager* pTrackCollectionManager,
            QSharedPointer<BaseTrackCache> trackSource);

    TrackPointer getTrack(const QModelIndex& index) const override;
    QUrl getTrackUrl(const QModelIndex& index) const override;
    TrackId getTrackId(const QModelIndex& index) const override;
    TrackModel::Capabilities getCapabilities() const override;
    /// Override of BaseSqlTableModel::search — in addition to filtering the
    /// existing rows (parent behaviour), dispatches a fresh
    /// `YouTubeService::searchVideos(searchText)` request via the feature
    /// so the table fills with live results as the user types in the
    /// per-view search box.
    void search(const QString& searchText) override;

    /// Override of BaseSqlTableModel::getCoverInfo — serves the YouTube
    /// thumbnail (hqdefault.jpg) for each row instead of looking in the
    /// `youtube_library` table (which has no coverart columns). Returns a
    /// FILE-type CoverInfo pointing at `<thumbnailDir>/<videoId>.jpg` when
    /// the thumbnail is present on disk, or an empty CoverInfo otherwise.
    CoverInfo getCoverInfo(const QModelIndex& index) const override;

    /// Set the directory where thumbnail images are stored. Must be called
    /// before the view renders for cover art to appear.
    void setThumbnailDir(const QString& dir);

    /// Notify the model that more results are available (or not) for the
    /// current search. Drives canFetchMore() / fetchMore().
    void setHasMore(bool hasMore);

    /// canFetchMore() / fetchMore() — when the user scrolls to the bottom
    /// of the track table, Qt calls fetchMore() if canFetchMore() returns
    /// true. We emit fetchMoreRequested() which the feature connects to
    /// YouTubeService::fetchMoreSearchResults().
    bool canFetchMore(const QModelIndex& parent) const override;
    void fetchMore(const QModelIndex& parent) override;

  signals:
    /// Emitted from `search()` to ask the feature to dispatch a fresh
    /// `YouTubeService::searchVideos(query)`. The feature owns the
    /// service and the network/auth state so we keep that in one place.
    void searchRequested(const QString& query);

    /// Emitted from fetchMore() when the view has scrolled to the end of
    /// the current result set. The feature connects this to
    /// YouTubeService::fetchMoreSearchResults() to load the next page.
    void fetchMoreRequested();

  protected:
    QString resolveLocation(const QString& nativeLocation) const override;

  private:
    /// Directory containing per-video thumbnail images. Set by the feature.
    QString m_thumbnailDir;
    /// True when the service has reported a continuation token for the
    /// current search (i.e. more results are available via fetchMore).
    bool m_hasMore = false;
    /// Debounce timer for the per-view search box. Keystroke search events
    /// are delayed 400 ms so a full word is transmitted rather than one
    /// YouTube request per character typed.
    QTimer* m_searchDebounceTimer = nullptr;
    /// Most recently typed (but not yet dispatched) search text.
    QString m_pendingSearch;
};
