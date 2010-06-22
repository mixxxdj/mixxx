#ifndef EFFECTSUNITSCONTROLLER_H
#define EFFECTSUNITSCONTROLLER_H

#include "../engine/engineeffectsunits.h"
#include "debugbackend.h"
#include "ladspabackend.h"

class EffectsUnitsController {

public:
	EffectsUnitsController();
	~EffectsUnitsController();

	void activatePluginOnSource(QString PluginName, QString Source);
	void loadAllPlugins();

	EffectsUnitsPlugin * getPluginByName(QString PluginName);

private:
	EngineEffectsUnits * m_pEngine;					// Engine pointer so we can append plugins to the processing queue
	QList<EffectsUnitsBackend *> m_BackendsList;    // List of every fx backend we have
	QList<EffectsUnitsPlugin *> m_AllPlugins;		// List of every plugin from every backend
};

#endif
