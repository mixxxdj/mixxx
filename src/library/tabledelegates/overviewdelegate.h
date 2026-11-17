#pragma once

#include <QPainter>
#include <QStyledItemDelegate>
#include <QTableView>

#include "library/tabledelegates/cachedtableitemdelegate.h"
#include "track/trackid.h"
#include "util/parented_ptr.h"
#include "waveform/overviewtype.h"
#include "waveform/renderers/waveformsignalcolors.h"

class ControlProxy;
class OverviewCache;
class TrackModel;

class OverviewDelegate : public CachedTableItemDelegate {
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
    void emitOverviewRowsChanged(QSet<TrackId>&& trackIds, bool visibleOnly = false);
    // Renders overviews around the viewport into QPixmapCache
    void prefetchRow(int row, bool visible) override;
    int prefetchInFlightCount() const override {
        return static_cast<int>(m_prefetchInFlight.size());
    }
    qint64 prefetchBytesPerRow() const override;

    OverviewCache* const m_pCache;
    mixxx::OverviewType m_type;
    parented_ptr<ControlProxy> m_pTypeControl;
    WaveformSignalColors m_signalColors;
    mutable QSize m_cellSize; // device pixels
    QSet<TrackId> m_prefetchInFlight;
};
