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

    Q_PROPERTY(QColor watchedPathColor
                    MEMBER m_watchedPathColor
                            NOTIFY watchedPathColorChanged
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

  public slots:
    void selectIndex(const QModelIndex&);
    void selectChildIndex(const QModelIndex&, bool selectItem = true);
    void slotSetFont(const QFont& font);

  signals:
    void rightClicked(const QPoint&, const QModelIndex&);
    void renameItem(const QModelIndex&);
    void deleteItem(const QModelIndex&);
    FocusWidget setLibraryFocus(FocusWidget newFocus,
            Qt::FocusReason focusReason = Qt::OtherFocusReason);
    void watchedPathColorChanged(QColor m_watchedPathColor);

  protected:
    bool event(QEvent* pEvent) override;

  private:
    void focusSelectedIndex();
    QModelIndex selectedIndex();

    SidebarModel* m_pSidebarModel;
    SidebarItemDelegate* m_pItemDelegate;
    QBasicTimer m_expandTimer;
    QModelIndex m_hoverIndex;

    QColor m_watchedPathColor;
};
