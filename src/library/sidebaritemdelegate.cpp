#include "library/sidebaritemdelegate.h"

#include <QPainter>
#include <QStyle>

#include "library/sidebarmodel.h"
#include "moc_sidebaritemdelegate.cpp"
#include "util/assert.h"
#include "widget/wlibrarysidebar.h"

SidebarItemDelegate::SidebarItemDelegate(
        WLibrarySidebar* pSidebarWidget,
        SidebarModel* pSidebarModel)
        : QStyledItemDelegate(pSidebarWidget),
          m_pSidebarModel(pSidebarModel) {
    DEBUG_ASSERT(m_pSidebarModel);
}

void SidebarItemDelegate::paint(
        QPainter* pPainter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    QStyledItemDelegate::paint(pPainter, option, index);

    // If the (path) item is a watched path, draw an indicator on top of the qss style.
    // The bounding rect of the dot is half the x-height of the current font.
    int wh = option.fontMetrics.xHeight() / 2;
    if (m_pSidebarModel->indexIsWatchedPathItem(index)) {
        pPainter->setRenderHint(QPainter::Antialiasing);
        pPainter->setBrush(QBrush(m_watchedPathColor));
        pPainter->drawEllipse( // x, y, w, h
                option.rect.x(),
                option.rect.y() + 1,
                wh,
                wh);
    }
}
