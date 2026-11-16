#include "library/tabledelegates/tableitemdelegate.h"

#include <QPainter>
#include <QPixmapCache>
#include <algorithm>
#include <chrono>

#include "moc_tableitemdelegate.cpp"
#include "util/painterscope.h"
#include "widget/wtracktableview.h"

namespace {
using namespace std::chrono_literals;
constexpr auto kPrefetchInterval = 50ms;
// Don't restart the timer for every painted cell
constexpr auto kRescheduleSlack = 5ms;
// Scrolling counts as stopped after this, then the window is centered again
constexpr auto kScrollSettle = 250ms;
constexpr int kPagesAhead = 3;
constexpr int kPagesBehind = 1;
constexpr int kPagesIdle = 2;
// Covers and overviews share QPixmapCache
constexpr int kCacheShareDivisor = 4;

std::chrono::milliseconds elapsed(const QElapsedTimer& timer) {
    return std::chrono::milliseconds(timer.elapsed());
}
} // namespace

TableItemDelegate::TableItemDelegate(QTableView* pTableView)
        : DefaultDelegate(pTableView),
          m_pTableView(pTableView) {
    DEBUG_ASSERT(m_pTableView);
    auto* pTrackTableView = qobject_cast<WTrackTableView*>(m_pTableView);
    if (pTrackTableView) {
        // This gets a color from the stylesheet:
        // WTrackTableView {
        //   qproperty-focusBorderColor: red;
        // }
        m_focusBorderColor = pTrackTableView->getFocusBorderColor();
        // For some reason the color is not initialized from the stylesheet for
        // some WTrackTableViews (in DlgAutoDJ, DlgAnalysis, ...)
        // Listen to the property changed signal.
        connect(pTrackTableView,
                &WTrackTableView::focusBorderColorChanged,
                this,
                [this](QColor col) {
                    m_focusBorderColor = col;
                });
    }
}

void TableItemDelegate::paint(
        QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    m_prefetchColumn = index.column();
    PainterScope painterScope(painter);
    painter->setClipRect(option.rect);

    // Clone the const option so we can change palette colors.
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    if (opt.state & QStyle::State_Selected) {
        setTextColor(opt, index);
    }

    QBrush brush;
    if (opt.state & QStyle::State_Selected) {
        brush = opt.palette.highlightedText();
    } else {
        brush = opt.palette.text();
    }
    // Brush color for the star rating polygons:
    painter->setBrush(brush);
    // Pen for the 'location' text
    painter->setPen(brush.color());

    QStyle* style = m_pTableView->style();
    if (style) {
        style->drawControl(
                QStyle::CE_ItemViewItem,
                // Use the original option here to not screw up
                // the Key and Preview delegates.
                &option,
                painter,
                m_pTableView);
    }

    paintItem(painter, opt, index);
}

int TableItemDelegate::columnWidth(const QModelIndex &index) const {
    return m_pTableView->columnWidth(index.column());
}

// static
void TableItemDelegate::drawBorder(
        QPainter* painter,
        const QColor borderColor,
        const QRect& rect) {
    // Draws a border around the cell with official skin's default focus border width
    QPen borderPen(
            borderColor,
            1,
            Qt::SolidLine,
            Qt::SquareCap);
    painter->setPen(borderPen);
    painter->setBrush(QBrush(Qt::transparent));
    painter->drawRect(
            rect.left(),
            rect.top(),
            rect.width() - 1,
            rect.height() - 1);
}

void TableItemDelegate::paintItem(
        QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    // We don't want to call DefaultDelegate::paint() because that would again
    // try to set the 'selected' color (which we already did in paint() above).
    // Relevant only for derived classes that don't implement paintItem()
    QStyledItemDelegate::paint(painter, option, index); // clazy:exclude=skipped-base-method
}

void TableItemDelegate::enablePrefetch() {
    m_pPrefetchModel = m_pTableView->model();
    const auto rowsReplaced = [this]() {
        m_forcePrefetch = true;
        prefetchRowsReplaced();
    };
    connect(m_pPrefetchModel, &QAbstractItemModel::modelReset, this, rowsReplaced);
    connect(m_pPrefetchModel, &QAbstractItemModel::layoutChanged, this, rowsReplaced);
    connect(m_pPrefetchModel, &QAbstractItemModel::rowsInserted, this, rowsReplaced);
    connect(m_pPrefetchModel, &QAbstractItemModel::rowsRemoved, this, rowsReplaced);
    m_pPrefetchTimer = make_parented<QTimer>(this);
    m_pPrefetchTimer->setSingleShot(true);
    connect(m_pPrefetchTimer.get(), &QTimer::timeout, this, &TableItemDelegate::runPrefetch);
}

void TableItemDelegate::runPrefetch() {
    m_lastPrefetch.start();
    if (!isPrefetchActive()) {
        return;
    }
    PrefetchWindow window;
    window.rowCount = m_pTableView->model()->rowCount();
    const int height = m_pTableView->viewport()->height();
    if (window.rowCount > 0 && height > 0) {
        window.first = std::max(0, m_pTableView->rowAt(0));
        window.last = m_pTableView->rowAt(height - 1);
        if (window.last < 0) {
            window.last = window.rowCount - 1;
        }
    }
    if (m_prefetchWindow.first >= 0 && window.first != m_prefetchWindow.first) {
        window.direction = window.first > m_prefetchWindow.first ? 1 : -1;
        m_lastScroll.start();
    } else if (m_lastScroll.isValid() && elapsed(m_lastScroll) < kScrollSettle) {
        window.direction = m_prefetchWindow.direction;
    }
    if (window.direction != 0) {
        // Center the window again once scrolling stopped
        m_pPrefetchTimer->start(kScrollSettle + 1ms - elapsed(m_lastScroll));
    }
    if (window == m_prefetchWindow && !m_forcePrefetch) {
        return;
    }
    m_prefetchWindow = window;
    m_forcePrefetch = false;
    prefetchWindow();
}

bool TableItemDelegate::isPrefetchActive() const {
    if (!m_pPrefetchModel ||
            m_pTableView->model() != m_pPrefetchModel ||
            !m_pTableView->isVisible() ||
            m_prefetchColumn < 0 ||
            m_pTableView->isColumnHidden(m_prefetchColumn)) {
        return false;
    }
    const int x = m_pTableView->columnViewportPosition(m_prefetchColumn);
    return x + m_pTableView->columnWidth(m_prefetchColumn) > 0 &&
            x < m_pTableView->viewport()->width();
}

void TableItemDelegate::schedulePrefetch(bool immediately) const {
    if (!m_pPrefetchTimer) {
        return;
    }
    std::chrono::milliseconds delay = 0ms;
    if (immediately) {
        // Right after the current paint, so all misses go into one pass
        m_forcePrefetch = true;
    } else if (m_lastPrefetch.isValid()) {
        delay = std::max(0ms, kPrefetchInterval - elapsed(m_lastPrefetch));
    }
    if (!m_pPrefetchTimer->isActive() ||
            m_pPrefetchTimer->remainingTimeAsDuration() > delay + kRescheduleSlack) {
        m_pPrefetchTimer->start(delay);
    }
}

QList<int> TableItemDelegate::prefetchRows(qint64 bytesPerRow, int* pVisibleCount) const {
    QList<int> rows;
    *pVisibleCount = 0;
    const PrefetchWindow& w = m_prefetchWindow;
    if (w.first < 0) {
        return rows;
    }
    const int page = w.last - w.first + 1;
    int ahead = (w.direction != 0 ? kPagesAhead : kPagesIdle) * page;
    int behind = (w.direction != 0 ? kPagesBehind : kPagesIdle) * page;
    const qint64 budgetRows = static_cast<qint64>(QPixmapCache::cacheLimit()) *
            1024 / kCacheShareDivisor / std::max<qint64>(1, bytesPerRow);
    const qint64 extraRows = std::max<qint64>(0, budgetRows - page);
    if (ahead + behind > extraRows) {
        const int total = ahead + behind;
        ahead = static_cast<int>(extraRows * ahead / total);
        behind = static_cast<int>(extraRows) - ahead;
    }
    const int below = w.direction < 0 ? behind : ahead;
    const int above = w.direction < 0 ? ahead : behind;
    const int lo = std::max(0, w.first - above);
    const int hi = std::min(w.rowCount - 1, w.last + below);

    rows.reserve(hi - lo + 1);
    for (int r = w.first; r <= w.last; ++r) {
        rows.append(r);
    }
    *pVisibleCount = static_cast<int>(rows.size());
    for (int d = 1; w.last + d <= hi || w.first - d >= lo; ++d) {
        if (w.last + d <= hi) {
            rows.append(w.last + d);
        }
        if (w.first - d >= lo) {
            rows.append(w.first - d);
        }
    }
    return rows;
}
