#ifndef EFFECTSUNITSCONTROLLER_H
#define EFFECTSUNITSCONTROLLER_H

#include "engine/engineeffectsunits.h"
#include "debugbackend.h"
#include "ladspabackend.h"
#include "effectsunitsinstance.h"
#include "effectsunitspresetmanager.h"

class EffectsUnitsController {

public:
	EffectsUnitsController(EffectsUnitsPresetManager*);
	~EffectsUnitsController();

	void addInstanceToSource(int InstanceID, QString Source);
	void removeInstanceFromSource(int InstanceID, QString Source);
	void removeInstance(int InstanceID);
	EffectsUnitsInstance * instantiatePluginForWidget(QString PluginName, int Widget, int KnobCount);

	void loadAllPlugins();
	QStringList * getEffectsList();
	EffectsUnitsPlugin * getPluginByName(QString PluginName);

private:
	EngineEffectsUnits * m_pEngine;					// Engine pointer so we can append plugins to the processing queue
	QList<EffectsUnitsBackend *> m_BackendsList;    // List of every fx backend we have
	QList<EffectsUnitsPlugin *> m_AllPlugins;		// List of every plugin from every backend
	QStringList m_PluginNames;						// List of plugin names, for dropdown and misc.
	QList<EffectsUnitsInstance *> m_ActiveInstances;// List of active instances, for instance mgmt
	EffectsUnitsPresetManager * m_pPresetManager;
};

#endif
