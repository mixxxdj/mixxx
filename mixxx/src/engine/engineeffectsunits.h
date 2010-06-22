#ifndef ENGINEEFFECTSUNITS_H
#define ENGINEEFFECTSUNITS_H

#include "engineobject.h"
#include "effectsunits/effectsunitsplugin.h"

class EffectsUnitsPlugin;

class EngineEffectsUnits : public EngineObject {
public:
	EngineEffectsUnits();
	~EngineEffectsUnits();

	static EngineEffectsUnits * getEngine();
	void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize);
	void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize, const QString Source);

	void addPluginToSource(EffectsUnitsPlugin * Plugin, QString Source);
	QList<EffectsUnitsPlugin *> * getPluginsBySource(QString Source);

private:
	static EngineEffectsUnits * m_pEngine;
	QList<EffectsUnitsPlugin * > * m_OnChannel1;
	QList<EffectsUnitsPlugin * > * m_OnChannel2;

};

#endif
