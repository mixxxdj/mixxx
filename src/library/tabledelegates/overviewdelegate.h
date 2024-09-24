#pragma once

#include <QPainter>
#include <QStyledItemDelegate>
#include <QTableView>

#include "library/tabledelegates/tableitemdelegate.h"
#include "track/trackid.h"
#include "util/parented_ptr.h"
#include "waveform/overviews/overviewtype.h"
#include "waveform/renderers/waveformsignalcolors.h"
#include "widget/woverview.h"

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
    void overviewReady(TrackId trackId);

    void overviewChanged(TrackId trackId);

  private slots:
    void slotTypeControlChanged(double v);
    void slotOverviewReady(const QObject* pRequester,
            const TrackId trackId,
            bool pixmapValid,
            const QSize resizedToSize);

  protected:
    TrackModel* const m_pTrackModel;

  private:
    OverviewCache* const m_pCache;
    mixxx::OverviewType m_type;
    parented_ptr<ControlProxy> m_pTypeControl;
    WaveformSignalColors m_signalColors;

    mutable QHash<TrackId, int> m_trackIdToRow;
};
