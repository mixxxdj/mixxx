#pragma once

#include <QElapsedTimer>
#include <QPointer>
#include <QTimer>

#include "library/tabledelegates/defaultdelegate.h"
#include "util/parented_ptr.h"

class TableItemDelegate : public DefaultDelegate {
    Q_OBJECT
  public:
    explicit TableItemDelegate(QTableView* pTableView);
    ~TableItemDelegate() override = default;

    void paint(
            QPainter* painter,
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const override;

    virtual void paintItem(
            QPainter* painter,
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const;

    static void drawBorder(
            QPainter* painter,
            const QColor borderColor,
            const QRect& rect);

  protected:
    // schedulePrefetch() calls prefetchWindow() at most every 50 ms,
    // so it also runs while the user keeps scrolling. Passes are skipped
    // if the viewport didn't move. Visible cache misses pass `immediately`
    // to skip the wait and force a pass.
    void enablePrefetch();
    void schedulePrefetch(bool immediately = false) const;
    virtual void prefetchWindow() {
    }
    // The model replaced its rows, e.g. after sorting or a new search
    virtual void prefetchRowsReplaced() {
    }
    // False if the view switched models (we are about to be deleted),
    // or the column is hidden or scrolled out horizontally
    bool isPrefetchActive() const;
    // Visible rows first, then outwards: 3 pages in the scroll direction
    // and 1 behind, or 2 each way when not scrolling. Fewer if they'd
    // take more than a quarter of QPixmapCache at `bytesPerRow`.
    QList<int> prefetchRows(qint64 bytesPerRow, int* pVisibleCount) const;

    // Only used by LocationDelegate's text elide.
    // Having this here avoids including QTableView there.
    int columnWidth(const QModelIndex &index) const;

    QTableView* m_pTableView;
    QColor m_focusBorderColor;

  private:
    void runPrefetch();

    struct PrefetchWindow {
        int first = -1;
        int last = -1;
        int rowCount = 0;
        int direction = 0; // 1 down, -1 up, 0 not scrolling
        bool operator==(const PrefetchWindow&) const = default;
    };
    PrefetchWindow m_prefetchWindow;
    parented_ptr<QTimer> m_pPrefetchTimer;
    QPointer<const QAbstractItemModel> m_pPrefetchModel;
    mutable int m_prefetchColumn = -1;
    mutable bool m_forcePrefetch = false;
    QElapsedTimer m_lastPrefetch;
    QElapsedTimer m_lastScroll;
};
