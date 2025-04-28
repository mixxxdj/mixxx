#pragma once

#include <QColor>
#include <QStyledItemDelegate>

class SidebarModel;
class WLibrarySidebar;

class SidebarItemDelegate : public QStyledItemDelegate {
    Q_OBJECT
  public:
    explicit SidebarItemDelegate(
            WLibrarySidebar* pSidebarwidget,
            SidebarModel* pSidebarModel);
    ~SidebarItemDelegate() override = default;

    void paint(
            QPainter* pPainter,
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const override;

    void setBookmarkColor(const QColor& color) {
        if (color.isValid()) {
            m_bookmarkColor = color;
        }
    }

  private:
    SidebarModel* m_pSidebarModel; // shared_ptr?
    QColor m_bookmarkColor;
};
