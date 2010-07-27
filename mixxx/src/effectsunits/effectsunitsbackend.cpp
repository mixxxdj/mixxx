#include "effectsunitsbackend.h"

EffectsUnitsBackend::EffectsUnitsBackend() {
	// TODO Auto-generated constructor stub
}

EffectsUnitsBackend::~EffectsUnitsBackend() {
	// TODO Auto-generated destructor stub
}

QString EffectsUnitsBackend::getName(){
	return (*m_Name);
}

EffectsUnitsBackend * EffectsUnitsBackend::getBackend(){
	return (this);
}

QList<EffectsUnitsPlugin *> * EffectsUnitsBackend::getPlugins(){
	return (&m_BackendPlugins);
}

void EffectsUnitsBackend::loadPlugins(){}
void EffectsUnitsBackend::process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize, int PluginID, double WetDry){ qDebug() << "FXUNITS: EffectsUnitsBackend, you're doing it wrong.";}
void EffectsUnitsBackend::connect(int PortID, float Value, int PluginID){}
void EffectsUnitsBackend::activatePlugin(int PluginID){}
void EffectsUnitsBackend::deactivatePlugin(int PluginID){}
