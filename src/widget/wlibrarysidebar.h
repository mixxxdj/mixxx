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

    void contextMenuEvent(QContextMenuEvent * event) override;
    void dragMoveEvent(QDragMoveEvent * event) override;
    void dragEnterEvent(QDragEnterEvent * event) override;
    void dropEvent(QDropEvent * event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void timerEvent(QTimerEvent* event) override;
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

    void toggleBookmark();
    void goToNextPrevBookmark(int direction);

    SidebarModel* m_pSidebarModel;
    SidebarItemDelegate* m_pItemDelegate;
    QBasicTimer m_expandTimer;
    QModelIndex m_hoverIndex;
    QColor m_bookmarkColor;
};
