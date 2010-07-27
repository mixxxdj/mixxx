#ifndef DEBUGBACKEND_H
#define DEBUGBACKEND_H

#include "effectsunitsbackend.h"

class EffectsUnitsBackend;

class DEBUGBackend: public EffectsUnitsBackend {
public:
	DEBUGBackend();
	~DEBUGBackend();

	void loadPlugins();
	void connect(int PortID, float Value, int m_PluginID);
	void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize, int PluginID, double WetDry);
	void activatePlugin(int PluginID);
	void deactivatePlugin(int PluginID);
};

#endif
