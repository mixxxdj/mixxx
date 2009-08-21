


#ifndef WTRACKSOURCESVIEW_H_
#define WTRACKSOURCESVIEW_H_

#include <QtCore>
#include <QtGui>

class WTrackSourcesView : public QTreeView
{
    Q_OBJECT
    public:
        WTrackSourcesView();
        ~WTrackSourcesView();
    private slots:
        void activatedSignalProxy(const QModelIndex& index);
    signals:
        void libraryItemActivated();
        void cheeseburgerItemActivated();
        void rhythmboxPlaylistItemActivated(QString);
    private:
        
};
#endif
