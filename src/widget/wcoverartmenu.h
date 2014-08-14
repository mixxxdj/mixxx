#ifndef WCOVERARTMENU_H
#define WCOVERARTMENU_H

#include <QAction>
#include <QMenu>
#include <QWidget>

class WCoverArtMenu : public QMenu {
    Q_OBJECT
  public:
    WCoverArtMenu(QWidget *parent = 0);
    virtual ~WCoverArtMenu();

    void updateData(int trackId, QString coverLocation,
                    QString md5, QString trackLocation="");

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
    QString m_sTrackLocation;
    QString m_sCoverLocation;
    QString m_sMd5;
};

#endif // WCOVERARTMENU_H
