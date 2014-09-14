#ifndef WCOVERARTMENU_H
#define WCOVERARTMENU_H

#include <QAction>
#include <QMenu>
#include <QWidget>

#include "trackinfoobject.h"
#include "library/coverartcache.h"

class WCoverArtMenu : public QMenu {
    Q_OBJECT
  public:
    // This class implements a context-menu with all CoverArt actions. Callers
    // MUST use the method show(...) to open the menu. do NOT use exec() or
    // popup(). This class does NOT CHANGE the database. When the cover is
    // changed through any of the offered options the 'coverLocationUpdated'
    // signal will be emitted and the widget using this menu can decide when and
    // if the new cover should be saved.
    WCoverArtMenu(QWidget *parent = 0);
    virtual ~WCoverArtMenu();

    void show(QPoint pos, CoverInfo info, TrackPointer pTrack);

  signals:
    void coverLocationUpdated(const QString& newLocation,
                              const QString& oldLocation,
                              QPixmap px);

  private slots:
    void slotChange();
    void slotReload();
    void slotUnset();

  private:
    void createActions();
    void addActions();

    QAction* m_pChange;
    QAction* m_pReload;
    QAction* m_pUnset;

    TrackPointer m_pTrack;
    CoverInfo m_coverInfo;
};

#endif // WCOVERARTMENU_H
