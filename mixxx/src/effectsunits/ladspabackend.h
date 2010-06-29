#ifndef LADSPABACKEND_H
#define LADSPABACKEND_H

#include "effectsunitsbackend.h"
#include "../ladspa/ladspaloader.h"
#include "../ladspa/ladspacontrol.h"


class LADSPABackend: public EffectsUnitsBackend {
public:
	LADSPABackend();
	~LADSPABackend();

	void loadPlugins();
	void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize, int PluginID);
	void activatePlugin(int PluginID);
	void deactivatePlugin(int PluginID);


private:

	QList<LADSPAPlugin*> m_LADPSAPlugin;		// LADSPAPlugins to preserve instantiate process
	QList<LADSPAInstance*> m_LADSPAInstance;	// LADSPAInstance to process audio signals
	LADSPALoader * m_LADSPALoader;				// LADSPALoader loads all plugins
	QList<int> m_ActivatedPlugins;

//	LADSPAInstance * m_test;
//
//
//    LADSPAControl * zero;
//    LADSPAControl * um;
//    LADSPAControl * dois;
//    LADSPAControl * tres;
//
//	CSAMPLE * m_pBufferLeft[2];
//	CSAMPLE * m_pBufferRight[2];
};

#endif
