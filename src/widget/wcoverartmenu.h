#ifndef WCOVERARTMENU_H
#define WCOVERARTMENU_H

#include <QAction>
#include <QMenu>
#include <QWidget>

#include "trackinfoobject.h"

class WCoverArtMenu : public QMenu {
    Q_OBJECT
  public:
    // This class implements a context-menu with all CoverArt actions.
    // Callers MUST use the method show(...) to open the menu.
    // do NOT use exec() or popup()
    WCoverArtMenu(QWidget *parent = 0);
    virtual ~WCoverArtMenu();

    void show(QPoint pos, QPair<QString, QString> cover,
              int trackId, TrackPointer pTrack=TrackPointer());

  signals:
    void coverLocationUpdated(const QString& newLocation,
                              const QString& oldLocation);

  private slots:
    void slotChange();
    void slotShowFullSize();
    void slotReload();
    void slotUnset();

  private:
    void createActions();
    void addActions();

    QAction* m_pChange;
    QAction* m_pFullSize;
    QAction* m_pReload;
    QAction* m_pUnset;

    int m_iTrackId;
    TrackPointer m_pTrack;
    QString m_sCoverLocation;
    QString m_sMd5;
};

#endif // WCOVERARTMENU_H
