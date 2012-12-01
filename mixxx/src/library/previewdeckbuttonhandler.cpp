#include "previewdeckbuttonhandler.h"
#include "controlobjectthreadmain.h"
#include "controlobject.h"
#include "library/trackmodel.h"
#include "trackinfoobject.h"
#include "playermanager.h"
#include "playerinfo.h"

PreviewDeckButtonHandler::PreviewDeckButtonHandler(QObject *parent,
                                                   const QModelIndex &index,
                                                   QTableView *pTableView)
        : QObject(parent),
          m_index(index),
          m_pTableView(pTableView) {
}

PreviewDeckButtonHandler::~PreviewDeckButtonHandler(){
}

void PreviewDeckButtonHandler::buttonclicked(){
    TrackModel *pTrackModel = dynamic_cast<TrackModel*>(m_pTableView->model());
    QString group = PlayerManager::groupForPreviewDeck(0);
    TrackPointer pOldTrack = PlayerInfo::Instance().getTrackInfo("[PreviewDeck1]");

    ControlObjectThreadMain* playStatus = new ControlObjectThreadMain(
                ControlObject::getControl(ConfigKey(group, "play")));

    TrackPointer pTrack = pTrackModel->getTrack(m_index);
    if (pTrack && pTrack!=pOldTrack) {
        emit(loadTrackToPlayer(pTrack, group));
        playStatus->slotSet(1.0);
    } else if (pTrack==pOldTrack && playStatus->get()==0.0) {
        playStatus->slotSet(1.0);
    } else {
        playStatus->slotSet(0.0);
    }
    delete playStatus;
}
