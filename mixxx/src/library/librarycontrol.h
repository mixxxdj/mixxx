#ifndef LIBRARYMIDICONTROL_H
#define LIBRARYMIDICONTROL_H

#include <QObject>

class ControlObjectThreadMain;
class ControlObject;
class WLibrary;
class WLibrarySidebar;
class MixxxKeyboard;

class LoadToGroupController : public QObject {
    Q_OBJECT
  public:
    LoadToGroupController(QObject* pParent, const QString group);
    virtual ~LoadToGroupController();
  signals:
    void loadToGroup(QString group, bool);
  public slots:
    void slotLoadToGroup(double v);
    void slotLoadToGroupAndPlay(double v);
  private:
    QString m_group;
    ControlObject* m_pLoadControl;
    ControlObject* m_pLoadAndPlayControl;
    ControlObjectThreadMain* m_pLoadCOTM;
    ControlObjectThreadMain* m_pLoadAndPlayCOTM;
};

class LibraryControl : public QObject {
    Q_OBJECT
  public:
    LibraryControl(QObject* pParent=NULL);
    virtual ~LibraryControl();
    void bindWidget(WLibrary* pLibrary, MixxxKeyboard* pKeyboard);
    void bindSidebarWidget(WLibrarySidebar* pLibrarySidebar);

  private slots:
    void libraryWidgetDeleted();
    void sidebarWidgetDeleted();
    void slotLoadSelectedTrackToGroup(QString group, bool play);
    void slotSelectNextTrack(double v);
    void slotSelectPrevTrack(double v);
    void slotSelectNextSidebarItem(double v);
    void slotSelectPrevSidebarItem(double v);
    void slotToggleSelectedSidebarItem(double v);
    void slotLoadSelectedIntoFirstStopped(double v);
    void slotSelectTrackKnob(double v);


  private:
    ControlObjectThreadMain* m_pSelectNextTrack;
    ControlObjectThreadMain* m_pSelectPrevTrack;
    ControlObjectThreadMain* m_pSelectNextPlaylist;
    ControlObjectThreadMain* m_pSelectPrevPlaylist;
    ControlObjectThreadMain* m_pToggleSidebarItem;
    ControlObjectThreadMain* m_pLoadSelectedIntoFirstStopped;
    ControlObjectThreadMain* m_pSelectTrackKnob;
    WLibrary* m_pLibraryWidget;
    WLibrarySidebar* m_pSidebarWidget;
};

#endif //LIBRARYMIDICONTROL_H
