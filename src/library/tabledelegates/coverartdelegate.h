#pragma once

#include <QHash>
#include <QList>
#include <QSet>

#include "library/coverart.h"
#include "library/tabledelegates/cachedtableitemdelegate.h"
#include "track/track_decl.h"
#include "util/cache.h"
#include "util/duration.h"

class CoverArtCache;
class TrackModel;
class QTableView;

class CoverArtDelegate : public CachedTableItemDelegate {
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
    // Returns false if no coverFound() will arrive for coverInfo.cacheKey()
    bool requestUncachedCover(
            const CoverInfo& coverInfo,
            int width);

    // Load covers around the viewport into QPixmapCache
    bool prefetchRow(int row, bool visible) override;
    int prefetchInFlightCount() const override {
        return static_cast<int>(m_prefetchInFlight.size());
    }
    qint64 prefetchBytesPerRow() const override;
    void prefetchRowsReplaced() override;
    bool isKnownMissing(mixxx::cache_key_t key) const;

    mutable int m_cellWidth = 0; // device pixels
    QSet<mixxx::cache_key_t> m_prefetchInFlight;
    // Covers that were expected but not found, e.g. on a missing drive.
    // CoverArtCache doesn't cache them, so we must not retry on every pass.
    QHash<mixxx::cache_key_t, mixxx::Duration> m_missingSince;

    CoverArtCache* const m_pCache;

    // Rows to refresh when a cover is loaded, also recorded in paint
    mutable QMultiHash<mixxx::cache_key_t, int> m_pendingCacheRows;
};
