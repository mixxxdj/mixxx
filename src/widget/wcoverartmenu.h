#ifndef WCOVERARTMENU_H
#define WCOVERARTMENU_H

#include <QAction>
#include <QMenu>
#include <QWidget>

#include "trackinfoobject.h"

class WCoverArtMenu : public QMenu {
    Q_OBJECT
  public:
    WCoverArtMenu(QWidget *parent = 0);
    virtual ~WCoverArtMenu();

    void updateData(QString coverLocation, QString md5,
                    int trackId, TrackPointer pTrack=TrackPointer());

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
