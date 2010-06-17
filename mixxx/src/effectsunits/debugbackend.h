#include "effectsunitsbackend.h"

class DEBUGBackend: public EffectsUnitsBackend {
public:
	DEBUGBackend();
	~DEBUGBackend();

	void loadPlugins();
	void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize, int PluginID);
	void activatePlugin(int PluginID);
};
