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

	/* LADSPAPlugins to preserve instantiate method */
	QList<LADSPAPlugin*> m_LADSPAPlugin;

	/* LADSPAInstance to process audio signals */
	QList<LADSPAInstance*> m_LADSPAInstance;

	/* LADSPALoader loads all plugins */
	LADSPALoader * m_LADSPALoader;

	/* List of LADSPAControls by Plugin, to use change parameter ports */
	QList<QList<LADSPAControl*> *> m_PluginLADSPAControl;

	/* process(in, out, buf) helpers: */
	LADSPAInstance * m_beingProcessed;
	QList<LADSPAControl *> * m_beingUpdated;
	QList<EffectsUnitsPort *> * m_beingRead;
	int m_monoBufferSize;
	CSAMPLE * m_pBufferLeft[2];
	CSAMPLE * m_pBufferRight[2];
};

#endif
