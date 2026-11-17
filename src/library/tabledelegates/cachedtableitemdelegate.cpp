#include "library/tabledelegates/cachedtableitemdelegate.h"

#include <QPixmapCache>
#include <QTableView>
#include <algorithm>
#include <chrono>
#include <utility>

#include "moc_cachedtableitemdelegate.cpp"
#include "util/assert.h"

namespace {

using namespace std::chrono_literals;

constexpr auto kPrefetchInterval = 50ms;
// Scrolling counts as stopped after this, then the window is centered again
constexpr auto kScrollSettle = 250ms;
constexpr int kPagesAhead = 3;
constexpr int kPagesBehind = 1;
constexpr int kPagesIdle = 2;
// Each delegate may fill a quarter of QPixmapCache, so covers and
// overviews together leave half of it to everything else
constexpr int kCacheShareDivisor = 4;
// Visible rows are only limited while scrolling, so rows that are
// scrolled past are not all requested
constexpr int kMaxPrefetchInFlight = 4;

std::chrono::milliseconds elapsed(const QElapsedTimer& timer) {
    return std::chrono::milliseconds(timer.elapsed());
}

} // anonymous namespace

CachedTableItemDelegate::CachedTableItemDelegate(QTableView* pTableView, int column)
        : TableItemDelegate(pTableView),
          m_column(column),
          m_pModel(pTableView->model()),
          m_pPrefetchTimer(make_parented<QTimer>(this)),
          m_pSettleTimer(make_parented<QTimer>(this)) {
    VERIFY_OR_DEBUG_ASSERT(m_pModel) {
        return;
    }
    const auto rowsReplaced = [this]() {
        suspendPrefetch();
        prefetchRowsReplaced();
    };
    connect(m_pModel, &QAbstractItemModel::modelReset, this, rowsReplaced);
    connect(m_pModel, &QAbstractItemModel::layoutChanged, this, rowsReplaced);
    connect(m_pModel, &QAbstractItemModel::rowsInserted, this, rowsReplaced);
    connect(m_pModel, &QAbstractItemModel::rowsRemoved, this, rowsReplaced);
    for (QTimer* pTimer : {m_pPrefetchTimer.get(), m_pSettleTimer.get()}) {
        pTimer->setSingleShot(true);
        connect(pTimer, &QTimer::timeout, this, &CachedTableItemDelegate::prefetchViewport);
    }
}

void CachedTableItemDelegate::schedulePrefetch(bool immediately) const {
    if (immediately && m_viewport.direction == 0) {
        // Right after the current paint, so all misses go into one pass
        if (!m_forcePrefetch) {
            m_forcePrefetch = true;
            m_pPrefetchTimer->start(0ms);
        }
        return;
    }
    if (m_pPrefetchTimer->isActive()) {
        // It is due no later than it would be now
        return;
    }
    std::chrono::milliseconds delay = 0ms;
    if (m_lastPrefetch.isValid()) {
        delay = std::max(0ms, kPrefetchInterval - elapsed(m_lastPrefetch));
    }
    m_pPrefetchTimer->start(delay);
}

void CachedTableItemDelegate::prefetchViewport() {
    m_lastPrefetch.start();
    const bool forced = std::exchange(m_forcePrefetch, false);
    if (!isPrefetchActive()) {
        suspendPrefetch();
        return;
    }
    const qint64 bytesPerRow = prefetchBytesPerRow();
    if (bytesPerRow <= 0) {
        return;
    }
    Viewport viewport;
    const int rowCount = m_pTableView->model()->rowCount();
    const int height = m_pTableView->viewport()->height();
    if (rowCount > 0 && height > 0) {
        viewport.first = std::max(0, m_pTableView->rowAt(0));
        viewport.last = m_pTableView->rowAt(height - 1);
        if (viewport.last < 0) {
            viewport.last = rowCount - 1;
        }
    }
    if (m_viewport.first >= 0 && viewport.first != m_viewport.first) {
        viewport.direction = viewport.first > m_viewport.first ? 1 : -1;
        m_lastScroll.start();
    } else if (m_lastScroll.isValid() && elapsed(m_lastScroll) < kScrollSettle) {
        viewport.direction = m_viewport.direction;
    }
    if (viewport.direction != 0) {
        m_pSettleTimer->start(kScrollSettle + 1ms - elapsed(m_lastScroll));
    }
    if (viewport == m_viewport && !forced) {
        return;
    }
    m_viewport = viewport;
    // Rebuilt on every pass, so rows that were scrolled past are dropped
    queuePrefetchRows(bytesPerRow);
    processPrefetchQueue();
}

void CachedTableItemDelegate::processPrefetchQueue() {
    if (!isPrefetchActive()) {
        suspendPrefetch();
        return;
    }
    while (m_prefetchNext < m_prefetchQueue.size()) {
        const bool visible = m_prefetchNext < m_prefetchVisibleCount;
        const bool limited = !visible || m_viewport.direction != 0;
        if (limited && prefetchInFlightCount() >= kMaxPrefetchInFlight) {
            return;
        }
        // Advance first, prefetchRow() may get here again if
        // a request finishes synchronously
        const int row = m_prefetchQueue[m_prefetchNext++];
        prefetchRow(row, visible);
    }
}

void CachedTableItemDelegate::clearPrefetchQueue() {
    m_prefetchQueue.clear();
    m_prefetchVisibleCount = 0;
    m_prefetchNext = 0;
}

void CachedTableItemDelegate::suspendPrefetch() {
    clearPrefetchQueue();
    // The next pass runs and centers the window
    m_viewport = Viewport{};
}

bool CachedTableItemDelegate::isPrefetchActive() const {
    bool active = m_pModel &&
            m_pTableView->model() == m_pModel &&
            m_column >= 0 &&
            // Not isVisible(), which is false in the QML library's offscreen widgets
            m_pTableView->isVisibleTo(m_pTableView->window()) &&
            !m_pTableView->isColumnHidden(m_column);
    if (active) {
        const int x = m_pTableView->columnViewportPosition(m_column);
        active = x + m_pTableView->columnWidth(m_column) > 0 &&
                x < m_pTableView->viewport()->width();
    }
    return active;
}

void CachedTableItemDelegate::queuePrefetchRows(qint64 bytesPerRow) {
    clearPrefetchQueue();
    const Viewport& vp = m_viewport;
    if (vp.first < 0) {
        return;
    }
    const int page = vp.last - vp.first + 1;
    int ahead = (vp.direction != 0 ? kPagesAhead : kPagesIdle) * page;
    int behind = (vp.direction != 0 ? kPagesBehind : kPagesIdle) * page;
    const qint64 budgetRows = static_cast<qint64>(QPixmapCache::cacheLimit()) *
            1024 / kCacheShareDivisor / bytesPerRow;
    const qint64 extraRows = std::max<qint64>(0, budgetRows - page);
    if (ahead + behind > extraRows) {
        const int total = ahead + behind;
        ahead = static_cast<int>(extraRows * ahead / total);
        behind = static_cast<int>(extraRows) - ahead;
    }
    const int below = vp.direction < 0 ? behind : ahead;
    const int above = vp.direction < 0 ? ahead : behind;
    const int lo = std::max(0, vp.first - above);
    const int hi = std::min(m_pTableView->model()->rowCount() - 1, vp.last + below);

    m_prefetchQueue.reserve(hi - lo + 1);
    for (int r = vp.first; r <= vp.last; ++r) {
        m_prefetchQueue.append(r);
    }
    m_prefetchVisibleCount = static_cast<int>(m_prefetchQueue.size());
    for (int d = 1; vp.last + d <= hi || vp.first - d >= lo; ++d) {
        if (vp.last + d <= hi) {
            m_prefetchQueue.append(vp.last + d);
        }
        if (vp.first - d >= lo) {
            m_prefetchQueue.append(vp.first - d);
        }
    }
}
