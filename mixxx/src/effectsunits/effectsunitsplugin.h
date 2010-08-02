#ifndef EFFECTSUNITSPLUGIN_H
#define EFFECTSUNITSPLUGIN_H

#include "defs.h"
#include "effectsunitsbackend.h"
#include "effectsunitsport.h"

class EffectsUnitsBackend;

class EffectsUnitsPlugin {

public:
	EffectsUnitsPlugin(EffectsUnitsBackend * Backend, QString * Name, int PluginID);
	~EffectsUnitsPlugin();

	void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize, double WetDry);
	void connect(int PortID, double Value);
	void activate();

	void addPort(EffectsUnitsPort *);

	QString getName();
	QList<EffectsUnitsPort *> * getPorts();
	QString getPortNameByIndex(int i);

private:
	QString * m_Name;					// Name of the Plugin
	int m_PluginID;						// Plugin ID (for identification inside the Backend)
	EffectsUnitsBackend * m_Backend;	// Backend pointer, we redirect process() to it
	QList<EffectsUnitsPort*> m_Ports;		// Ports info for the plugin.
};

#endif
