#pragma once

#include <QHash>
#include <QList>
#include <QTableView>

#include "library/coverart.h"
#include "library/tableitemdelegate.h"
#include "track/track_decl.h"
#include "util/cache.h"

class CoverArtCache;
class TrackModel;

class BaseCoverArtDelegate : public TableItemDelegate {
    Q_OBJECT

  public:
    explicit BaseCoverArtDelegate(
            QTableView* parent);
    ~BaseCoverArtDelegate() override = default;

    void paintItem(
            QPainter* painter,
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const final;

  signals:
    // Sent when rows need to be refreshed
    void rowsChanged(
            QList<int> rows);

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
            const QObject* pRequestor,
            const CoverInfo& coverInfo,
            const QPixmap& pixmap,
            mixxx::cache_key_t requestedImageHash,
            bool coverInfoUpdated);

  protected:
    TrackModel* const m_pTrackModel;

  private:
    void emitRowsChanged(
            QList<int>&& rows);

    TrackPointer loadTrackByLocation(
            const QString& trackLocation) const;

    virtual CoverInfo coverInfoForIndex(
            const QModelIndex& index) const = 0;

    CoverArtCache* const m_pCache;
    bool m_inhibitLazyLoading;

    // We need to record rows in paint() (which is const) so
    // these are marked mutable.
    mutable QList<int> m_cacheMissRows;
    mutable QMultiHash<mixxx::cache_key_t, int> m_pendingCacheRows;
};
