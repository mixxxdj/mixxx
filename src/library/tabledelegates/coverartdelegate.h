#pragma once

#include <QList>

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

  public slots:
    // Advise the delegate to temporarily inhibit lazy loading
    // of cover images and to only display those cover images
    // that have already been cached. Otherwise only the solid
    // (background) color is painted.
    //
    // It is useful to handle cases when the user scroll down
    // very fast or when they hold an arrow key. In this case
    // it is NOT desirable to start multiple expensive file
    // system operations in worker threads for loading and
    // scaling cover images that are not even displayed after
    // scrolling beyond them.
    void slotInhibitLazyLoading(
            bool inhibitLazyLoading);

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
    void cleanCacheMissRows() const;
    void requestUncachedCover(
            const CoverInfo& coverInfo,
            int width,
            int row) const;

    CoverArtCache* const m_pCache;
    bool m_inhibitLazyLoading;
    int m_column;

    // We need to record rows in paint() (which is const) so
    // these are marked mutable.
    mutable QSet<int> m_cacheMissRows;
    mutable QMultiHash<mixxx::cache_key_t, int> m_pendingCacheRows;
};
