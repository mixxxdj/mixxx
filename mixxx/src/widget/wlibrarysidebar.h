#ifndef WLIBRARYSIDEBAR_H
#define WLIBRARYSIDEBAR_H

#include <QtGui>
#include <QTreeView>

class WLibrarySidebar : public QTreeView {
  Q_OBJECT
  public:
    WLibrarySidebar(QWidget* parent = 0);
    virtual ~WLibrarySidebar();
};

#endif /* WLIBRARYSIDEBAR_H */
