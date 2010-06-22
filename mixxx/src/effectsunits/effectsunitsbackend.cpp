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
void EffectsUnitsBackend::process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize, int PluginID){ qDebug() << "FXUNITS: EffectsUnitsBackend, you're doing it wrong.";}
void EffectsUnitsBackend::activatePlugin(int PluginID){}
