#pragma once

#include <QPainter>
#include <QStyledItemDelegate>
#include <QTableView>

#include "library/tabledelegates/tableitemdelegate.h"
#include "track/trackid.h"

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
    void overviewReady(int row);
    // int column

    void overviewChanged(TrackId trackId);

  private slots:
    void slotOverviewReady(const QObject* pRequester,
            TrackId trackId,
            QPixmap pixmap,
            QSize resizedToSize);

    void slotOverviewChanged(TrackId trackId);

  protected:
    TrackModel* const m_pTrackModel;

  private:
    OverviewCache* const m_pCache;

    // QTableView* m_pTableView;
    // int m_iIdColumn;
    // int m_iOverviewColumn;

    mutable QHash<TrackId, int> m_trackIdToRow;
};
