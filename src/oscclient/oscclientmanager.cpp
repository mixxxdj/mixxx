#include <QMutex>
#include <QDir>
#include <QtDebug>

#include "oscclient/oscclientmanager.h"
#include "engine/sidechain/enginesidechain.h"
#include "engine/sidechain/engineoscclient.h"
#include "engine/enginemaster.h"


OscClientManager::OscClientManager(UserSettingsPointer& pConfig, EngineMaster* pEngine):
    timer(this)
         {

    //Register EngineRecord with the engine sidechain.
    EngineSideChain* pSidechain = pEngine->getSideChain();
    if (pSidechain) {
        EngineOscClient* pEngineRecord = new EngineOscClient(pConfig);
        pSidechain->addSideChainWorker(pEngineRecord);
    }


}

OscClientManager::~OscClientManager()
{
    qDebug() << "Delete OscClient";
}


void OscClientManager::sendState(){

}


void OscClientManager::connectServer()
{

}

void OscClientManager::maybeSendState()
{

}
