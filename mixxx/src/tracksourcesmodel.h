
#ifndef TRACKSOURCESMODEL_H_
#define TRACKSOURCESMODEL_H_

#include <QtCore>
#include <QtGui>

class TrackSourcesModel : public QStandardItemModel
{
    public:
        TrackSourcesModel();
        ~TrackSourcesModel();
    private:
        QStandardItem* m_pLibraryItem;
        QStandardItem* m_pCheeseburgerItem;
        
};
#endif
