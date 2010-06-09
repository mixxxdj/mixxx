#include <QtCore>
#include <QDebug>
#include "engineeffectsunits.h"

EngineEffectsUnits * EngineEffectsUnits::m_pEngine = NULL;

EngineEffectsUnits::EngineEffectsUnits() {
	m_pEngine = this;
	qDebug() << "FXUNITS: EngineEffectsUnits: new instance" << (int) m_pEngine;
}

EngineEffectsUnits::~EngineEffectsUnits() {

}

EngineEffectsUnits * EngineEffectsUnits::getEngine(){
	return m_pEngine;
}

void EngineEffectsUnits::process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize){
	qDebug() << "FXUNITS: EngineEffectsUnits: process";
}

void EngineEffectsUnits::process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize, const int iChannel){
	qDebug() << "FXUNITS: EngineEffectsUnits: process at channel" << iChannel;
}
