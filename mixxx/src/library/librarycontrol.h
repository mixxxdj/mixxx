#ifndef LIBRARYMIDICONTROL_H
#define LIBRARYMIDICONTROL_H

#include <QObject>

class ControlObjectThreadMain;
class WLibrary;
class WLibrarySidebar;
class MixxxKeyboard;

class LibraryControl : public QObject {
    Q_OBJECT
  public:
    LibraryControl(QObject* pParent=NULL);
    virtual ~LibraryControl();
    void bindWidget(WLibrarySidebar* pLibrarySidebar, WLibrary* pLibrary, MixxxKeyboard* pKeyboard);

  public slots:
    void slotLoadSelectedTrackCh1(double v);
    void slotLoadSelectedTrackCh2(double v);
    void slotSelectNextTrack(double v);
    void slotSelectPrevTrack(double v);
    void slotSelectNextPlaylist(double v);
    void slotSelectPrevPlaylist(double v);
    void slotLoadSelectedIntoFirstStopped(double v);
    void slotSelectTrackKnob(double v);

  private slots:
    void libraryWidgetDeleted();
    void sidebarWidgetDeleted();

  private:
    ControlObjectThreadMain* m_pLoadSelectedTrackCh1;
    ControlObjectThreadMain* m_pLoadSelectedTrackCh2;
    ControlObjectThreadMain* m_pSelectNextTrack;
    ControlObjectThreadMain* m_pSelectPrevTrack;
    ControlObjectThreadMain* m_pSelectNextPlaylist;
    ControlObjectThreadMain* m_pSelectPrevPlaylist;
    ControlObjectThreadMain* m_pLoadSelectedIntoFirstStopped;
    ControlObjectThreadMain* m_pSelectTrackKnob;
    WLibrary* m_pLibraryWidget;
    WLibrarySidebar* m_pSidebarWidget;
};

#endif //LIBRARYMIDICONTROL_H
