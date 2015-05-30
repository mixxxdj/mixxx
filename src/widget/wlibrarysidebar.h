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

#include "widget/wbasewidget.h"

class WLibrarySidebar : public QTreeView, public WBaseWidget {
    Q_OBJECT
  public:
    WLibrarySidebar(QWidget* parent = 0);
    virtual ~WLibrarySidebar();

    void contextMenuEvent(QContextMenuEvent * event);
    void dragMoveEvent(QDragMoveEvent * event);
    void dragEnterEvent(QDragEnterEvent * event);
    void dropEvent(QDropEvent * event);
    void keyPressEvent(QKeyEvent* event);
    void timerEvent(QTimerEvent* event);
    void toggleSelectedItem();

  public slots:
    void selectIndex(const QModelIndex&);
    void slotSetFont(const QFont& font);

  signals:
    void rightClicked(const QPoint&, const QModelIndex&);

  protected:
    bool event(QEvent* pEvent);

  private:
    QBasicTimer m_expandTimer;
    QModelIndex m_hoverIndex;
};

#endif /* WLIBRARYSIDEBAR_H */
