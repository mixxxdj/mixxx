#include <QtCore>
#include <QDebug>
#include "engineeffectsunits.h"

EngineEffectsUnits * EngineEffectsUnits::m_pEngine = NULL;

EngineEffectsUnits::EngineEffectsUnits() {
	m_pEngine = this;
	qDebug() << "FXUNITS: EngineEffectsUnits: new instance" << (int) m_pEngine;

	EffectsUnitsPlugin * delay = new EffectsUnitsPlugin(NULL, new QString("DelayC1"), 69);
	addPluginToSource(delay, "[Channel1]");

	EffectsUnitsPlugin * reverb = new EffectsUnitsPlugin(NULL, new QString("ReverbC2"), 70);
	addPluginToSource(reverb, "[Channel2]");

	EffectsUnitsPlugin * lofi = new EffectsUnitsPlugin(NULL, new QString("LoFiC12"), 71);
	addPluginToSource(lofi, "[Channel1]");
	addPluginToSource(lofi, "[Channel2]");
}

EngineEffectsUnits::~EngineEffectsUnits() {

}

EngineEffectsUnits * EngineEffectsUnits::getEngine(){
	return m_pEngine;
}

void EngineEffectsUnits::process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize){
	qDebug() << "FXUNITS: EngineEffectsUnits: process, you're doing it wrong!";
}

void EngineEffectsUnits::process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize, const QString Source){
	QList<EffectsUnitsPlugin *> * pluginList = getPluginsBySource(Source);
	int size = pluginList->size();
	for (int i = 0; i < size; ++i) {
		pluginList->at(i)->process(pIn, pOut, iBufferSize);
	 }
}

void EngineEffectsUnits::addPluginToSource(EffectsUnitsPlugin * Plugin, QString Source){
	QList<EffectsUnitsPlugin *> * pluginList = getPluginsBySource(Source);
	pluginList->append(Plugin);
}

QList<EffectsUnitsPlugin *> * EngineEffectsUnits::getPluginsBySource(QString Source){
	if (Source == "[Channel1]"){
		return &m_OnChannel1;
	} else if (Source == "[Channel2]"){
		return &m_OnChannel2;
	} else {
		return NULL;
	}
}
