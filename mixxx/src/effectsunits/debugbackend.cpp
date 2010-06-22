#include "debugbackend.h"

DEBUGBackend::DEBUGBackend() {
	m_Engine = EngineEffectsUnits::getEngine();
	qDebug() << "FXUNITS: DEBUGBackend: " << this << " using Engine: " << m_Engine;
}

DEBUGBackend::~DEBUGBackend() {
	// TODO Auto-generated destructor stub
}

void DEBUGBackend::loadPlugins(){
	EffectsUnitsPlugin * delay = new EffectsUnitsPlugin(this, new QString("DelayC1"), 69);
	EffectsUnitsPlugin * reverb = new EffectsUnitsPlugin(this, new QString("ReverbC2"), 70);
	EffectsUnitsPlugin * lofi = new EffectsUnitsPlugin(this, new QString("LoFiC12"), 71);

	m_BackendPlugins.append(delay);
	m_BackendPlugins.append(reverb);
	m_BackendPlugins.append(lofi);

	activatePlugin(69);
	m_Engine->addPluginToSource(delay, "[Channel1]");
	m_Engine->addPluginToSource(delay, "[Channel2]");
	m_Engine->addPluginToSource(reverb, "[Channel2]");
	m_Engine->addPluginToSource(lofi, "[Channel2]");
}

void DEBUGBackend::process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize, int PluginID){
	qDebug() << "FXUNITS: DEDBUGBackend: Processing: " << PluginID;
}

void DEBUGBackend::activatePlugin(int PluginID){
	qDebug() << "FXUNITS: DEBUGBackend: Activating: " << PluginID;
}
