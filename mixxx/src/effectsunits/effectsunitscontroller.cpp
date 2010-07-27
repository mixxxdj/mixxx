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
	//activatePluginOnSource("djFlanger", "[Channel1]");
	//activatePluginOnSource("Plate2x2", "[Channel2]");

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


	// TODO - Apply restrictions so we don't load all of them
	size = m_AllPlugins.size();
	for (int i = 0; i < size; ++i) {
		m_PluginNames.append(m_AllPlugins.at(i)->getName());
	}
}

/* EffectsUnitsController::activatePluginOnSource
 * Adds the desired plugin to the processing queue of the desired source.
 * i.e. activatePluginOnSource("Delay", [Channel1])
 */
void EffectsUnitsController::addInstanceToSource(int InstanceID, QString Source){

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

EffectsUnitsInstance * EffectsUnitsController::instantiatePluginForWidget(QString PluginName, int Widget){
	EffectsUnitsPlugin * current = getPluginByName(PluginName);
	if (current != NULL){
		current->activate();
		// TODO - Implement this properly, should make one for each widget.
		// Maybe it should always create a new instance.
		// Shouldnt forget about freeing this memory when the plugin is unloaded.
		return (new EffectsUnitsInstance(current));
	}
}

QStringList * EffectsUnitsController::getEffectsList(){
	return &m_PluginNames;
}
