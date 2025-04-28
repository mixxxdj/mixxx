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

    // If the item is a bookmark, draw the indicator on top of the qss style.
    // We draw a rectangle a the left, narrow enough to not cover the label and
    // inset by 1px to not cover the focus border.
    if (m_bookmarkColor.isValid() && m_pSidebarModel->indexIsBookmark(index)) {
        pPainter->fillRect(
                option.rect.x(),
                option.rect.y() + 1,
                3, // width
                option.rect.height() - 2,
                m_bookmarkColor);
    }
}
