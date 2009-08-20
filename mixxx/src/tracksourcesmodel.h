
#ifndef TRACKSOURCESMODEL_H_
#define TRACKSOURCESMODEL_H_

#include <QtCore>
#include <QtGui>

#include "rhythmboxtrackmodel.h"
#include "rhythmboxplaylistmodel.h"

class TrackSourcesModel : public QStandardItemModel
{
    public:
        TrackSourcesModel(RhythmboxTrackModel *, RhythmboxPlaylistModel *);
        ~TrackSourcesModel();
    private:
        QStandardItem* m_pLibraryItem;
        QStandardItem* m_pCheeseburgerItem;
        QStandardItem* m_pRhythmboxLibraryItem;
        QMap<QString, QStandardItem *> m_pRhythmboxPlaylistItems;
};
#endif
