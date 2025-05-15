#include "widget/wrelationtableview.h"

#include <QPainter>

#include "library/relations/relationstablemodel.h"
#include "moc_wrelationtableview.cpp"

const int SEPERATOR_SIZE = 1;

WRelationTableView::WRelationTableView(
        QWidget* parent,
        UserSettingsPointer pConfig,
        Library* pLibrary,
        double trackTableBackgroundColorOpacity,
        bool relationPairView)
        : WTrackTableView(parent,
                  pConfig,
                  pLibrary,
                  trackTableBackgroundColorOpacity),
          m_bRelationPairView(relationPairView) {
    setDragDropMode(QAbstractItemView::DragOnly);
    setDragEnabled(true);
}

// Need getSelectedRows and getTrackModel to be public.
QList<DbId> WRelationTableView::getSelectedRelationIds() const {
    TrackModel* pTrackModel = getTrackModel();
    RelationsTableModel* pRelationTableModel = dynamic_cast<RelationsTableModel*>(pTrackModel);
    VERIFY_OR_DEBUG_ASSERT(!pRelationTableModel) {
        qWarning() << "No track model available";
        return {};
    }

    const QModelIndexList rows = getSelectedRows();
    QList<DbId> relationIds;
    relationIds.reserve(rows.size());
    for (const QModelIndex& row : rows) {
        const DbId relationId = pRelationTableModel->getRelationId(row);
        if (relationId.isValid()) {
            relationIds.append(relationId);
        } else {
            qDebug() << "Skipping row" << row << "with invalid relation id";
        }
    }

    return relationIds;
}

bool isSeperatorRow(int row) {
    return row % 2 == 0;
}

void WRelationTableView::paintEvent(QPaintEvent* event) {
    QTableView::paintEvent(event);

    if (m_bRelationPairView) {
        QPainter painter(viewport());
        QPen pen(QColor(60, 60, 60), SEPERATOR_SIZE);
        painter.setPen(pen);

        int rowCount = model()->rowCount();
        for (int row = 1; row < rowCount; ++row) {
            if (isSeperatorRow(row)) {
                int y = rowViewportPosition(row);
                painter.drawLine(0, y, viewport()->width(), y);
            }
        }
    }
}
