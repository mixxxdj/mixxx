#ifndef EFFECTSUNITSBACKEND_H
#define EFFECTSUNITSBACKEND_H

#include <QtCore>
#include <QDebug>

#include "effectsunitsplugin.h"
#include "../engine/engineeffectsunits.h"
#include "../defs.h"

class EffectsUnitsPlugin;
class EngineEffectsUnits;

class EffectsUnitsBackend {
public:
	EffectsUnitsBackend();
	virtual ~EffectsUnitsBackend();

	QString getName();
	EffectsUnitsBackend * getBackend();
	QList<EffectsUnitsPlugin *> * getPlugins();

	virtual void loadPlugins();
	virtual void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize, int PluginID);
	virtual void activatePlugin(int PluginID);

protected:
	QString * m_Name;
	QList<EffectsUnitsPlugin *> m_BackendPlugins;
	EngineEffectsUnits * m_Engine;
};


#endif
