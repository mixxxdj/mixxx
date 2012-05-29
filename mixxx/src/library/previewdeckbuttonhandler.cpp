#include "previewdeckbuttonhandler.h"
#include "controlobjectthreadmain.h"
#include "controlobject.h"
#include "library/trackmodel.h"
#include "trackinfoobject.h"

PreviewdeckButtonHandler::PreviewdeckButtonHandler(const QObject *parent, 
                                                   const QModelIndex &index,
                                                   QTableView *pTableView){

    
    m_index = index;
    m_pTableView = pTableView;

    //TODO(kain88)this shoud check that parent has this SIGNAL before connection
    connect(this, SIGNAL(loadTrackToPlayer(TrackPointer, QString)), parent, SIGNAL(loadTrackToPlayer(TrackPointer, QString)));

}

PreviewdeckButtonHandler::~PreviewdeckButtonHandler(){

}

void PreviewdeckButtonHandler::buttonclicked(){
    TrackModel *pTrackModel = dynamic_cast<TrackModel*>(m_pTableView->model());
    
    ControlObjectThreadMain* playStatus = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[PreviewDeck1]", "play")));
    if (playStatus->get()==PLAYING){
        playStatus->slotSet(STOP);
    }
    //get TrackPointer and emit signal to load track
    if(TrackPointer Track = pTrackModel->getTrack(m_index)){
        emit(loadTrackToPlayer(Track,"[PreviewDeck1]"));
        playStatus->slotSet(PLAYING);
    }
}
