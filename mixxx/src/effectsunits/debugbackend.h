#ifndef DEBUGBACKEND_H
#define DEBUGBACKEND_H

#include "effectsunitsbackend.h"

class EffectsUnitsBackend;

class DEBUGBackend: public EffectsUnitsBackend {
public:
	DEBUGBackend();
	~DEBUGBackend();

	void loadPlugins();
	void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize, int PluginID);
	void activatePlugin(int PluginID);
};

#endif
