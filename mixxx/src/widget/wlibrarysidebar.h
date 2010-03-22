#ifndef WLIBRARYSIDEBAR_H
#define WLIBRARYSIDEBAR_H

#include <QtGui>
#include <QTreeView>

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

  public slots:
    void toggleExpansion( QModelIndex index );

  signals:
    void rightClicked(const QPoint&, const QModelIndex&);
};

#endif /* WLIBRARYSIDEBAR_H */
