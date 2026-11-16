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
    parented_ptr<ControlProxy> m_pTypeControl;
    WaveformSignalColors m_signalColors;

    // Renders overviews around the viewport into QPixmapCache
    void prefetchWindow() override;
    void pumpPrefetchQueue();
    mutable QSize m_cellSize; // device pixels
    struct PrefetchItem {
        TrackId trackId;
        bool visible;
    };
    QList<PrefetchItem> m_prefetchQueue;
    QSet<TrackId> m_prefetchInFlight;
};
