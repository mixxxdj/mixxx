#ifndef PREVIEWDECKBUTTONHANDLER_H
#define PREVIEWDECKBUTTONHANDLER_H

#include <QWidget>
#include <QTableView>
#include "trackinfoobject.h"
#include "library/trackmodel.h"

class PreviewdeckButtonHandler : public QObject 
{
    Q_OBJECT

    public :

    PreviewdeckButtonHandler(const QObject *parent,const QModelIndex &index,
                             QTableView *pTableView);
    virtual ~PreviewdeckButtonHandler();
    
    
signals:
    void loadTrackToPlayer(TrackPointer Track, QString group);

public slots:
    void buttonclicked();
    
    private:
        QModelIndex m_index;
        TrackModel *m_pTrackModel;
        QString m_group;
};

#endif
