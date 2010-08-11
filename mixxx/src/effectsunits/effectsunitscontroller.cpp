#include "effectsunitscontroller.h"

/* EffectsUnitsController::EffectsUnitsController()
 * Get the engine pointer so we can append plugins and stuff.
 * Appends all our fx backends to a list,
 * loadAllPlugins.
 */
EffectsUnitsController::EffectsUnitsController(EffectsUnitsPresetManager* PresetManager) {
	m_pEngine = EngineEffectsUnits::getEngine();
	m_pPresetManager = PresetManager;

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

		// TODO - Verify if there are presets available
		m_AllPlugins.append((*backendPlugins));
	}


	// TODO - Apply restrictions so we don't load all of them
	size = m_AllPlugins.size();
	for (int i = 0; i < size; ++i) {
		m_PluginNames.append(m_AllPlugins.at(i)->getName());
	}
}

void EffectsUnitsController::addInstanceToSource(int InstanceID, QString Source){
	if (InstanceID != NULL){
		int size = m_ActiveInstances.size();
		for (int i = 0; i < size; i++){
			if (m_ActiveInstances.at(i)->getInstanceID() == InstanceID){
				m_pEngine->addInstanceToSource(m_ActiveInstances.at(i), Source);
			}
		}
	}
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

EffectsUnitsInstance * EffectsUnitsController::instantiatePluginForWidget(QString PluginName, int Widget, int KnobCount){
	EffectsUnitsPlugin * current = getPluginByName(PluginName);
	if (current != NULL){
		current->activate();
		EffectsUnitsInstance * instance = new EffectsUnitsInstance(current, Widget, KnobCount, m_pPresetManager->findPresetForPlugin(PluginName));
		m_ActiveInstances.append(instance);
		return (instance);
	}
}

void EffectsUnitsController::removeInstanceFromSource(int InstanceID, QString Source){
	if (InstanceID != NULL)
		m_pEngine->removeInstanceFromSource(InstanceID, Source);
}

void EffectsUnitsController::removeInstance(int InstanceID){
	// TODO - For all sources
	if (InstanceID != NULL){
		m_pEngine->removeInstanceFromSource(InstanceID, "[Channel1]");
		m_pEngine->removeInstanceFromSource(InstanceID, "[Channel2]");
	}
}

QStringList * EffectsUnitsController::getEffectsList(){
	return &m_PluginNames;
}
