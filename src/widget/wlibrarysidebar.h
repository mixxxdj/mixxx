#ifndef WLIBRARYSIDEBAR_H
#define WLIBRARYSIDEBAR_H

#include <QBasicTimer>
#include <QContextMenuEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QKeyEvent>
#include <QModelIndex>
#include <QPoint>
#include <QTimerEvent>
#include <QTreeView>
#include <QEvent>
#include <QStyledItemDelegate>

#include "widget/wbasewidget.h"
#include "library/libraryview.h"

class WSidebarItemDelegate : public QStyledItemDelegate {
    Q_OBJECT
  public:
    WSidebarItemDelegate(QObject* parent = nullptr);
    
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const;
};

class WLibrarySidebar : public QTreeView, public WBaseWidget {
    Q_OBJECT
  public:
    explicit WLibrarySidebar(QWidget* parent = nullptr);

    void contextMenuEvent(QContextMenuEvent * event) override;
    void dragMoveEvent(QDragMoveEvent * event) override;
    void dragEnterEvent(QDragEnterEvent * event) override;
    void dropEvent(QDropEvent * event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void timerEvent(QTimerEvent* event) override;
    void toggleSelectedItem();

  public slots:
    void selectIndex(const QModelIndex&);
    void slotSetFont(const QFont& font);

  signals:
    void rightClicked(const QPoint&, const QModelIndex&);

    void hovered();
    void leaved();
    void focusIn();
    void focusOut();

  protected:
    bool event(QEvent* pEvent) override;
    void enterEvent(QEvent*) override;
    void leaveEvent(QEvent*) override;
    void focusInEvent(QFocusEvent*) override;
    void focusOutEvent(QFocusEvent*) override;

  private:
    bool paste();
    bool isDividerSelected();

    QBasicTimer m_expandTimer;
    QModelIndex m_hoverIndex;
};

#endif /* WLIBRARYSIDEBAR_H */
