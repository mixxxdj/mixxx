#include <QtCore>
#include <QDebug>
#include "engineeffectsunits.h"

EngineEffectsUnits * EngineEffectsUnits::m_pEngine = NULL;

EngineEffectsUnits::EngineEffectsUnits() {
	m_pEngine = this;
	m_OnChannel1 = new QList<EffectsUnitsPlugin * >();
	m_OnChannel2 = new QList<EffectsUnitsPlugin * >();

	qDebug() << "FXUNITS: EngineEffectsUnits: new instance" << (int) m_pEngine;
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
	qDebug() << "FXUNITS: Processing plugins for Source:" << Source;
	QList<EffectsUnitsPlugin *> * pluginList = getPluginsBySource(Source);
	int size = pluginList->size();
	for (int i = 0; i < size; ++i) {
		pluginList->at(i)->process(pIn, pOut, iBufferSize);
		qDebug() << pluginList->at(i)->getName();
	 }
}

/* EngineEffectsUnits::addPluginToSource
 * Every source that wants to have fx, will mantain a process queue with fx plugins
 * If you want plugin X to run on source Y you run:
 * addPluginToSource(X, Y)
 *
 * This operation is called from EffectsUnitsController.
 */
void EngineEffectsUnits::addPluginToSource(EffectsUnitsPlugin * Plugin, QString Source){
	if (Plugin){
		QList<EffectsUnitsPlugin *> * pluginList = getPluginsBySource(Source);
		pluginList->append(Plugin);
	}
}


QList<EffectsUnitsPlugin *> * EngineEffectsUnits::getPluginsBySource(QString Source){
	if (Source == "[Channel1]"){
		return m_OnChannel1;
	} else if (Source == "[Channel2]"){
		return m_OnChannel2;
	} else {
		return NULL;
	}
}
