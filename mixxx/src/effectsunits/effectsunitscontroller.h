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
	EngineEffectsUnits * m_pEngine;
	QList<EffectsUnitsBackend *> m_BackendsList;
	QList<EffectsUnitsPlugin *> m_AllPlugins;
};

#endif
