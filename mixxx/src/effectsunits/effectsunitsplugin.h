/*
 * effectsunitsplugin.h
 *
 *  Created on: Jun 16, 2010
 *      Author: bruno
 */

#ifndef EFFECTSUNITSPLUGIN_H_
#define EFFECTSUNITSPLUGIN_H_

#include "effectsunitsbackend.h"
#include "defs.h"

class EffectsUnitsPlugin {

public:
	EffectsUnitsPlugin(EffectsUnitsBackend * Backend, QString * Name, int PluginID);
	~EffectsUnitsPlugin();
	void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize);
	QString getName();

private:
	QString * m_Name;
	int m_PluginID;
	EffectsUnitsBackend * m_Backend;
};

#endif /* EFFECTSUNITSPLUGIN_H_ */
