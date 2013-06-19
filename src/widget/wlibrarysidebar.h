#ifndef WLIBRARYSIDEBAR_H
#define WLIBRARYSIDEBAR_H

#include <QtGui>
#include <QTreeView>
#include <QBasicTimer>

class WLibrarySidebar : public QTreeView {
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

  public slots:
    void selectIndex(const QModelIndex&);

  signals:
    void rightClicked(const QPoint&, const QModelIndex&);

  private:
    QBasicTimer m_expandTimer;
    QModelIndex m_hoverIndex;
};

#endif /* WLIBRARYSIDEBAR_H */
