#include <QtCore>
#include <QDebug>
#include "engineeffectsunits.h"

EngineEffectsUnits * EngineEffectsUnits::m_pEngine = NULL;

EngineEffectsUnits::EngineEffectsUnits() {
	// TODO - we shouldnt bound by the number of channels

	m_pEngine = this;
	m_OnChannel1 = new QList<EffectsUnitsInstance * >();
	m_OnChannel2 = new QList<EffectsUnitsInstance * >();

	qDebug() << "FXUNITS: EngineEffectsUnits: new instance" << m_pEngine;
}

EngineEffectsUnits::~EngineEffectsUnits() {

}

/* EngineEffectsUnits::getEngine()
 * Only one engine running, to get a pointer to it run
 * EngineEffectsUnits * OnlyOne = EngineEffectsUnits::getEngine()
 */
EngineEffectsUnits * EngineEffectsUnits::getEngine(){
	return m_pEngine;
}

/* This is WRONG and only done to respect pure virtual from EngineObject, see next process() */
void EngineEffectsUnits::process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize){
	qDebug() << "FXUNITS: EngineEffectsUnits: process, you're doing it wrong!";
}

/* EngineEffectsUnits::process
 * On EngineMaster, we call process(in,out,buffer, SOURCE), with samples from this Source.
 */
void EngineEffectsUnits::process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize, const QString Source){

	/* Processing all instances in this Source's queue */
	QList<EffectsUnitsInstance *> * instancesList = getInstancesBySource(Source);
	int size = instancesList->size();
	for (int i = 0; i < size; ++i) {

		m_pCurrentInstance = instancesList->at(i);
		/* Is it On? */
		if (m_pCurrentInstance->getEnabled()){
			m_pCurrentInstance->updatePorts();
			m_pCurrentInstance->process(pIn, pOut, iBufferSize);
		}

	 }
}

/* EngineEffectsUnits::addInstanceToSource
 * Every source that wants to have fx, will mantain a process queue with fx instances
 * If you want instance X to run on source Y you run:
 * addInstanceToSource(X, Y)
 *
 * This operation is called from EffectsUnitsController.
 */
void EngineEffectsUnits::addInstanceToSource(EffectsUnitsInstance * Instance, QString Source){
	if (Instance){
		QList<EffectsUnitsInstance *> * instanceList = getInstancesBySource(Source);
		instanceList->append(Instance);
	}
}

void EngineEffectsUnits::removeInstanceFromSource(int InstanceID, QString Source){
	QList<EffectsUnitsInstance *> * instanceList = getInstancesBySource(Source);
	if (instanceList){
		int size = instanceList->size();
		for (int i = 0; i < size; i++){
			if (instanceList->at(i)->getInstanceID() == InstanceID){
				instanceList->removeAt(i);
				return;
			}
		}
	}
}


QList<EffectsUnitsInstance *> * EngineEffectsUnits::getInstancesBySource(QString Source){
	if (Source == "[Channel1]"){
		return m_OnChannel1;
	} else if (Source == "[Channel2]"){
		return m_OnChannel2;
	} else {
		return NULL;
	}
}
