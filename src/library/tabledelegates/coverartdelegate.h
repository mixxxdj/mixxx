#pragma once

#include <QList>
#include <QSet>

#include "library/coverart.h"
#include "library/tabledelegates/tableitemdelegate.h"
#include "track/track_decl.h"
#include "util/cache.h"

class CoverArtCache;
class TrackModel;
class QTableView;

class CoverArtDelegate : public TableItemDelegate {
    Q_OBJECT

  public:
    explicit CoverArtDelegate(
            QTableView* parent);
    ~CoverArtDelegate() override = default;

    void paintItem(
            QPainter* painter,
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const final;

  signals:
    // Sent when rows need to be refreshed
    void rowsChanged(const QList<int>& rows);

  private slots:
    void slotCoverFound(
            const QObject* pRequester,
            const CoverInfo& coverInfo,
            const QPixmap& pixmap);

  protected:
    TrackModel* const m_pTrackModel;

  private:
    void emitRowsChanged(
            QList<int>&& rows);
    // Returns false if no coverFound() will arrive for coverInfo.cacheKey()
    bool requestUncachedCover(
            const CoverInfo& coverInfo,
            int width);

    // Loads covers around the viewport into QPixmapCache
    void prefetchWindow() override;
    void prefetchRowsReplaced() override;
    void pumpPrefetchQueue();

    struct PrefetchItem {
        CoverInfo coverInfo;
        QList<int> rows;
        bool visible;
    };
    mutable int m_cellWidth; // device pixels
    QList<PrefetchItem> m_prefetchQueue;
    QSet<mixxx::cache_key_t> m_prefetchInFlight;
    // Covers that were expected but not found, e.g. on a missing drive.
    // CoverArtCache doesn't cache them, so we must not retry on every pass.
    QSet<mixxx::cache_key_t> m_noImageKeys;

    CoverArtCache* const m_pCache;
    int m_column;

    QMultiHash<mixxx::cache_key_t, int> m_pendingCacheRows;
};
