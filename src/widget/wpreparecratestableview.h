#ifndef WPREPARECRATESTABLEVIEW_H_
#define WPREPARECRATESTABLEVIEW_H_

#include <QTableView> 
class TrackCollection;
class QDropEvent;
class QDragMoveEvent;
class QDragEnterEvent;

class WPrepareCratesTableView : public QTableView 
{
    public:
        WPrepareCratesTableView(QWidget* parent, TrackCollection* pTrackCollection);
        ~WPrepareCratesTableView();

        virtual void dropEvent(QDropEvent * event);
        virtual void dragMoveEvent(QDragMoveEvent * event);
        virtual void dragEnterEvent(QDragEnterEvent * event);
    private:
        TrackCollection* m_pTrackCollection;

};

#endif
