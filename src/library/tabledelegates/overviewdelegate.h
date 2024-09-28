#pragma once

#include <QPainter>
#include <QStyledItemDelegate>
#include <QTableView>

#include "library/tabledelegates/tableitemdelegate.h"
#include "track/trackid.h"
#include "util/parented_ptr.h"
#include "waveform/overviewtype.h"
#include "waveform/renderers/waveformsignalcolors.h"

class ControlProxy;
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
    void overviewRowsChanged(const QList<int>& rows);

  public slots:
    // Advise the delegate to temporarily inhibit lazy loading
    // of overview images and to only display those images
    // that have already been cached.
    //
    // It is useful to handle cases when the user scroll down
    // very fast or when they hold an arrow key. In this case
    // it is NOT desirable to start multiple expensive file
    // system operations for loading and scaling images that
    // are not even displayed after scrolling beyond them.
    void slotInhibitLazyLoading(bool inhibitLazyLoading);

  private slots:
    void slotTypeControlChanged(double v);
    void slotOverviewReady(const QObject* pRequester,
            const TrackId trackId,
            bool pixmapValid);
    void slotOverviewChanged(const TrackId trackId);

  protected:
    TrackModel* const m_pTrackModel;

  private:
    void emitOverviewRowsChanged(QSet<TrackId>&& trackIds);
    OverviewCache* const m_pCache;
    mixxx::OverviewType m_type;
    bool m_inhibitLazyLoading;
    parented_ptr<ControlProxy> m_pTypeControl;
    WaveformSignalColors m_signalColors;

    mutable QSet<TrackId> m_cacheMissIds;
};
