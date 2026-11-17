#pragma once

#include <QBitArray>
#include <QElapsedTimer>
#include <QList>
#include <QPointer>
#include <QTimer>

#include "library/tabledelegates/tableitemdelegate.h"
#include "util/parented_ptr.h"

/// Base for delegates that paint images from QPixmapCache and load
/// cache misses asynchronously, i.e. cover art and overviews.
///
/// Painting only reads the cache and calls schedulePrefetch(), which runs
/// a throttled pass. A pass queues the visible rows, then the rows around
/// them, and hands them to prefetchRow(). It is skipped if the viewport
/// didn't move since the last pass.
class CachedTableItemDelegate : public TableItemDelegate {
    Q_OBJECT
  public:
    CachedTableItemDelegate(QTableView* pTableView, int column);
    ~CachedTableItemDelegate() override = default;

  protected:
    /// Visible cache misses pass `immediately` to skip the wait and
    /// force a pass right after the current paint, unless scrolling.
    void schedulePrefetch(bool immediately = false) const;
    /// Continue with the queued rows, call this when a request finished
    void processPrefetchQueue();
    /// Sorted and deduplicated, optionally only the visible ones
    QList<int> rowsToRefresh(QList<int> rows, bool visibleOnly) const;

    /// Request the image for `row` unless it is cached, loading or known
    /// to be missing. Rows that are not visible, and all rows while
    /// scrolling, are only passed while few requests are in flight.
    /// Returns true if nothing is left to do, i.e. the image is cached
    /// or missing. Such rows are skipped until they are visible.
    virtual bool prefetchRow(int row, bool visible) = 0;
    virtual int prefetchInFlightCount() const = 0;
    /// Size of an image in the column, 0 while unknown
    virtual qint64 prefetchBytesPerRow() const = 0;
    /// The model replaced its rows, e.g. after sorting or a new search
    virtual void prefetchRowsReplaced() {
    }

    const int m_column;

  private:
    void prefetchViewport();
    // False if the delegate is orphaned (the view switched models),
    // the view or column is hidden, or the column is scrolled out
    bool isPrefetchActive() const;
    void suspendPrefetch();
    // Visible rows first, then outwards, more in the scroll direction.
    // Limited to a share of QPixmapCache at `bytesPerRow`.
    void queuePrefetchRows(qint64 bytesPerRow);
    void clearPrefetchQueue();

    struct Viewport {
        int first = -1;
        int last = -1;
        int direction = 0; // 1 down, -1 up, 0 not scrolling
        bool operator==(const Viewport&) const = default;
    };
    Viewport visibleRows() const;
    Viewport m_viewport;
    const QPointer<const QAbstractItemModel> m_pModel;
    const parented_ptr<QTimer> m_pPrefetchTimer;
    // Centers the window again once scrolling stopped
    const parented_ptr<QTimer> m_pSettleTimer;
    // An immediate pass is pending, it runs even if the viewport didn't move
    mutable bool m_forcePrefetch = false;
    QElapsedTimer m_lastPrefetch;
    QElapsedTimer m_lastScroll;

    // Rows of the current pass, the first m_prefetchVisibleCount are visible
    QList<int> m_prefetchQueue;
    int m_prefetchVisibleCount = 0;
    qsizetype m_prefetchNext = 0;
    // Rows that prefetchRow() found done. Immediate passes reset it, they
    // run when a visible row misses the cache, e.g. after an eviction.
    QBitArray m_prefetchDone;
};
