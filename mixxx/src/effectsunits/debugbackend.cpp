#include "debugbackend.h"

DEBUGBackend::DEBUGBackend() {
	qDebug() << "FXUNITS: DEBUGBackend: " << this;
}

DEBUGBackend::~DEBUGBackend() {
	// TODO Auto-generated destructor stub
}

void DEBUGBackend::loadPlugins(){
	EffectsUnitsPlugin * delay = new EffectsUnitsPlugin(this, new QString("Debug::Delay"), 69);
	EffectsUnitsPlugin * reverb = new EffectsUnitsPlugin(this, new QString("Debug::Reverb"), 70);
	EffectsUnitsPlugin * lofi = new EffectsUnitsPlugin(this, new QString("Debug::LoFi"), 71);

	m_BackendPlugins.append(delay);
	m_BackendPlugins.append(reverb);
	m_BackendPlugins.append(lofi);
}

void DEBUGBackend::connect(int PortID, float Value, int m_PluginID){

}
void DEBUGBackend::process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize, int PluginID, double WetDry){
	qDebug() << "FXUNITS: DEDBUGBackend: Processing #:" << PluginID;
}

void DEBUGBackend::activatePlugin(int PluginID){
	qDebug() << "FXUNITS: DEBUGBackend: Activating: " << PluginID;
}

void DEBUGBackend::deactivatePlugin(int PluginID){
	qDebug() << "FXUNITS: DEBUGBackend: Deactivating: " << PluginID;
}
