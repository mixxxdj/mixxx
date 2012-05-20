#include <QtCore>
#include <QTableView>
#include <QDebug>

#include "previewdeckbuttonhandler.h"
#include "controlobjectthreadmain.h"
#include "controlobject.h"
#include "library/trackmodel.h"
#include "trackinfoobject.h"

PreviewdeckButtonHandler::PreviewdeckButtonHandler(const QObject *parent, 
                                                   const QModelIndex &index,
                                                   QTableView *pTableView){
    
    m_index = index;
    m_pTrackModel = dynamic_cast<TrackModel*>(pTableView->model());
    m_group = QString("[PreviewDeck1]");//currently there is only one previewDeck
    connect(this, SIGNAL(loadTrackToPlayer(TrackPointer, QString)), parent, SIGNAL(loadTrackToPlayer(TrackPointer, QString)));
    
}

PreviewdeckButtonHandler::~PreviewdeckButtonHandler(){
    
}

void PreviewdeckButtonHandler::buttonclicked(){
    qDebug() << "kain88 button clicked handle";
    //check if deck is playing and stop it
    ControlObjectThreadMain* playStatus = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(m_group, "play")));
    if(!playStatus->get()){
        playStatus->slotSet(0);
    } 
    TrackPointer Track = m_pTrackModel->getTrack(m_index);
    qDebug() << "kain88 Track="<<Track;
    // qDebug() <<index.row();
    emit(loadTrackToPlayer(Track,m_group));
}
