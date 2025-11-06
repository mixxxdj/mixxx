#pragma once

#include <QBasicTimer>
#include <QModelIndex>
#include <QTreeView>

#include "library/library_decl.h"
#include "widget/wbasewidget.h"

class LibraryFeature;
class SidebarItemDelegate;
class SidebarModel;
class QPoint;

class WLibrarySidebar : public QTreeView, public WBaseWidget {
    Q_OBJECT
  public:
    explicit WLibrarySidebar(QWidget* parent = nullptr);

    Q_PROPERTY(QColor bookmarkColor
                    MEMBER m_bookmarkColor
                            NOTIFY bookmarkColorChanged
                                    DESIGNABLE true);

    void setModel(QAbstractItemModel* pModel) override;

    void contextMenuEvent(QContextMenuEvent* pEvent) override;
    void dragMoveEvent(QDragMoveEvent* pEvent) override;
    void dragEnterEvent(QDragEnterEvent* pEvent) override;
    void dragLeaveEvent(QDragLeaveEvent* pEvent) override;
    void dropEvent(QDropEvent* pEvent) override;
    void keyPressEvent(QKeyEvent* pEvent) override;
    void mousePressEvent(QMouseEvent* pEvent) override;
    void focusInEvent(QFocusEvent* pEvent) override;
    void timerEvent(QTimerEvent* pEvent) override;
    void toggleSelectedItem();
    void renameSelectedItem();
    bool isLeafNodeSelected();
    bool isChildIndexSelected(const QModelIndex& index);
    bool isFeatureRootIndexSelected(LibraryFeature* pFeature);

    void setBookmarkColor(const QColor& color);

  public slots:
    void selectIndex(const QModelIndex& index, bool scrollToIndex = true);
    void selectChildIndex(const QModelIndex&, bool selectItem = true);
    void slotSetFont(const QFont& font);
    void slotSetExpandOnHoverDelay(int delay);
    void slotGoToNextPrevBookmark(int direction);

  signals:
    void rightClicked(const QPoint&, const QModelIndex&);
    void renameItem(const QModelIndex&);
    void deleteItem(const QModelIndex&);
    FocusWidget setLibraryFocus(FocusWidget newFocus,
            Qt::FocusReason focusReason = Qt::OtherFocusReason);
    void bookmarkColorChanged(QColor m_bookmarkColor);

  protected:
    bool event(QEvent* pEvent) override;

  private:
    bool focusSelectedIndex();
    bool selectFocusedIndex();
    QModelIndex selectedIndex();

    void toggleDragHoverPropertyAndUpdateStyle(bool enabled);
    void resetHoverIndexAndDragMoveResult();
    void toggleBookmark();

    SidebarModel* m_pSidebarModel;
    SidebarItemDelegate* m_pItemDelegate;
    QBasicTimer m_expandTimer;
    int m_hoverExpandDelay;
    QModelIndex m_hoverIndex;
    bool m_lastDragMoveAccepted;
    QColor m_bookmarkColor;
};
