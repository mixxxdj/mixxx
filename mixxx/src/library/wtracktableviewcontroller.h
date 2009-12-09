#ifndef WTTVCONTROLLER_H
#define WTTVCONTROLLER_H

#include <QObject>
class ControlObjectThreadMain;
class WTrackTableView;

class WTrackTableViewController : public QObject
{
    Q_OBJECT
    public:
        WTrackTableViewController(QObject* parent, WTrackTableView* pView);
        ~WTrackTableViewController();
    public slots:
        void slotLoadSelectedTrackCh1(double v); 
        void slotLoadSelectedTrackCh2(double v); 
        void slotSelectNextTrack(double v); 
        void slotSelectPrevTrack(double v); 
        void slotLoadSelectedIntoFirstStopped(double v); 
        void slotSelectTrackKnob(double v); 
    private:
        ControlObjectThreadMain* m_pLoadSelectedTrackCh1;
        ControlObjectThreadMain* m_pLoadSelectedTrackCh2;
        ControlObjectThreadMain* m_pSelectNextTrack;
        ControlObjectThreadMain* m_pSelectPrevTrack;
        ControlObjectThreadMain* m_pLoadSelectedIntoFirstStopped;
        ControlObjectThreadMain* m_pSelectTrackKnob;
        WTrackTableView* m_pView;
        
};

#endif //WTTVCONTROLLER_H
