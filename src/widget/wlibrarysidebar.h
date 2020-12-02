#ifndef WLIBRARYSIDEBAR_H
#define WLIBRARYSIDEBAR_H

#include <QBasicTimer>
#include <QByteArrayData>
#include <QContextMenuEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QEvent>
#include <QKeyEvent>
#include <QModelIndex>
#include <QPoint>
#include <QString>
#include <QTimerEvent>
#include <QTreeView>

#include "widget/wbasewidget.h"

class QContextMenuEvent;
class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;
class QEvent;
class QFont;
class QKeyEvent;
class QObject;
class QPoint;
class QTimerEvent;
class QWidget;

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
    bool isLeafNodeSelected();

  public slots:
    void selectIndex(const QModelIndex&);
    void slotSetFont(const QFont& font);

  signals:
    void rightClicked(const QPoint&, const QModelIndex&);

  protected:
    bool event(QEvent* pEvent) override;

  private:
    QBasicTimer m_expandTimer;
    QModelIndex m_hoverIndex;
};

#endif /* WLIBRARYSIDEBAR_H */
