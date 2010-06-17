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
