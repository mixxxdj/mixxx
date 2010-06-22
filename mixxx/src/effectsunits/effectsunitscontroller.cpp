#include "effectsunitscontroller.h"

EffectsUnitsController::EffectsUnitsController() {
	m_pEngine = EngineEffectsUnits::getEngine();

	EffectsUnitsBackend * debug = new DEBUGBackend();
	EffectsUnitsBackend * ladspa = new LADSPABackend();

	m_pBackendsList.append(debug);
	m_pBackendsList.append(ladspa);

	debug->loadPlugins();

}

EffectsUnitsController::~EffectsUnitsController() {
	// TODO Auto-generated destructor stub
}
