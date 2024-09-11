#pragma once

#include <QPainter>
#include <QStyledItemDelegate>
#include <QTableView>

#include "library/tabledelegates/tableitemdelegate.h"
#include "track/trackid.h"
#include "waveform/renderers/waveformsignalcolors.h"

class OverviewCache;
class TrackModel;

class OverviewDelegate : public TableItemDelegate {
    Q_OBJECT

  public:
    explicit OverviewDelegate(QTableView* parent);
    ~OverviewDelegate() override = default;

    void paintItem(QPainter* painter,
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const final;

  signals:
    void overviewChanged(const TrackId trackId);

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
    void slotInhibitLazyLoading(bool inhibitLazyLoading);

  private slots:
    void slotOverviewReady(const QObject* pRequester, TrackId trackId);

    void slotOverviewChanged(TrackId trackId);

  protected:
    TrackModel* const m_pTrackModel;

  private:
    void emitRowsChanged(QList<int>&& rows);

    OverviewCache* const m_pCache;
    bool m_inhibitLazyLoading;

    // We need to record rows in paint() (which is const) so
    // these are marked mutable.
    mutable QList<int> m_cacheMissRows;

    WaveformSignalColors m_signalColors;
};
