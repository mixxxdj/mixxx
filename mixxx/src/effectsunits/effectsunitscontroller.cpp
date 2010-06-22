#include "effectsunitscontroller.h"

/* Initialization:
 * Get the engine pointer,
 * appends all our fxbackends to a list,
 * loadAllPlugins.
 */
EffectsUnitsController::EffectsUnitsController() {
	m_pEngine = EngineEffectsUnits::getEngine();

	EffectsUnitsBackend * debug = new DEBUGBackend();
	EffectsUnitsBackend * ladspa = new LADSPABackend();

	m_BackendsList.append(debug);
	m_BackendsList.append(ladspa);

	loadAllPlugins();
}

EffectsUnitsController::~EffectsUnitsController() {
	// TODO Auto-generated destructor stub
}

void EffectsUnitsController::loadAllPlugins(){
	int size_b = m_BackendsList.size();
	QList<EffectsUnitsPlugin *> * backendPlugins;

	for (int i = 0; i < size; i++){
		backendPlugins = m_BackendsList.at(i)->getPlugins();
		m_AllPlugins.append(backendPlugins);
	}
}

void EffectsUnitsController::activatePluginOnSource(QString PluginName, QString Source){
	m_pEngine->addPluginToSource(getPluginByName(PluginName), Source);
}

EffectsUnitsPlugin * EffectsUnitsController::getPluginByName(QString PluginName){
	int size = m_AllPlugins.size();
	for (int i = 0; i < size; ++i) {
		if (m_AllPlugins->at(i)->getName() == PluginName)
			return m_AllPlugins->at(i);
	 }
	return NULL;
}
