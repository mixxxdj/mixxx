#ifndef LIBRARYMIDICONTROL_H 
#define LIBRARYMIDICONTROL_H 

#include <QObject>
class ControlObjectThreadMain;
class WLibrary;
class WLibrarySidebar;

class LibraryMIDIControl : public QObject
{
    Q_OBJECT
    public:
        LibraryMIDIControl(WLibrary* wlibrary, WLibrarySidebar* pSidebar);
        ~LibraryMIDIControl();
    public slots:
        void slotLoadSelectedTrackCh1(double v); 
        void slotLoadSelectedTrackCh2(double v); 
        void slotSelectNextTrack(double v); 
        void slotSelectPrevTrack(double v); 
        void slotSelectNextPlaylist(double v); 
        void slotSelectPrevPlaylist(double v); 
        void slotLoadSelectedIntoFirstStopped(double v); 
        void slotSelectTrackKnob(double v); 
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
