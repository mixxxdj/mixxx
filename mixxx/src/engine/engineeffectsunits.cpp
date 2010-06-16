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
	//qDebug() << "FXUNITS: EngineEffectsUnits: process";
}

void EngineEffectsUnits::process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize, const QString Source){
	if (Source == "[Channel1]"){

		qDebug() << "FXUNITS: EngineEffectsUnits: processing effects for [Channel1]";
		QLinkedList<EffectsUnitsPlugin *>::iterator p1 = m_OnChannel1.begin();
		while (p1 != m_OnChannel1.end()){
			(*p1)->process(pIn, pOut, iBufferSize);
		    p1++;
		}


		} else if (Source == "[Channel2]"){

			qDebug() << "FXUNITS: EngineEffectsUnits: processing effects for [Channel2]";
			QLinkedList<EffectsUnitsPlugin *>::iterator p2 = m_OnChannel2.begin();
			while (p2 != m_OnChannel2.end()){
				(*p2)->process(pIn, pOut, iBufferSize);
			    p2++;
			}

		} else {

		}
}

void EngineEffectsUnits::addPluginToSource(EffectsUnitsPlugin * Plugin, QString Source){
	if (Source == "[Channel1]"){
		m_OnChannel1.append(Plugin);
		qDebug() << "FXUNITS: EngineEffectsUnits: Activating" << Plugin->getName() << "on [Channel1]";
	} else if (Source == "[Channel2]"){
		m_OnChannel2.append(Plugin);
		qDebug() << "FXUNITS: EngineEffectsUnits: Activating" << Plugin->getName() << "on [Channel2]";
	} else {
		qDebug() << "FXUNITS: Unrecognized source";
	}
}
