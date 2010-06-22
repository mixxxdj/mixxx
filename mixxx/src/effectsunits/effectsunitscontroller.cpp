#include "effectsunitscontroller.h"

/* EffectsUnitsController::EffectsUnitsController()
 * Get the engine pointer so we can append plugins and stuff.
 * Appends all our fx backends to a list,
 * loadAllPlugins.
 */
EffectsUnitsController::EffectsUnitsController() {
	m_pEngine = EngineEffectsUnits::getEngine();

	EffectsUnitsBackend * debug = new DEBUGBackend();
	EffectsUnitsBackend * ladspa = new LADSPABackend();

	m_BackendsList.append(debug);
	m_BackendsList.append(ladspa);

	loadAllPlugins();

	//test:
	//activatePluginOnSource("Debug::Delay", "[Channel1]");
	//activatePluginOnSource("LoFiC12", "[Channel1]");

}

EffectsUnitsController::~EffectsUnitsController() {
	// TODO Auto-generated destructor stub
}

/* void EffectsUnitsController::loadAllPlugins
 * For each backend, loads all plugins available, get them into a list.
 */
void EffectsUnitsController::loadAllPlugins(){
	int size = m_BackendsList.size();
	QList<EffectsUnitsPlugin *> * backendPlugins;

	for (int i = 0; i < size; i++){
		m_BackendsList.at(i)->loadPlugins();
		backendPlugins = m_BackendsList.at(i)->getPlugins();
		m_AllPlugins.append((*backendPlugins));
	}
}

/* EffectsUnitsController::activatePluginOnSource
 * Adds the desired plugin to the processing queue of the desired source.
 * i.e. activatePluginOnSource("Delay", [Channel1])
 */
void EffectsUnitsController::activatePluginOnSource(QString PluginName, QString Source){
	m_pEngine->addPluginToSource(getPluginByName(PluginName), Source);
}

/* EffectsUnitsController::getPluginByName
 * Finds the plugin in our list of all backend plugins.
 */
EffectsUnitsPlugin * EffectsUnitsController::getPluginByName(QString PluginName){
	int size = m_AllPlugins.size();
	for (int i = 0; i < size; ++i) {
		if (m_AllPlugins.at(i)->getName() == PluginName)
			return m_AllPlugins.at(i);
	 }
	return NULL;
}
