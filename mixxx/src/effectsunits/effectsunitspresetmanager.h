/*
 * effectsunitspresetmanager.h
 *
 *  Created on: Aug 10, 2010
 *      Author: bruno
 */

#ifndef EFFECTSUNITSPRESETMANAGER_H_
#define EFFECTSUNITSPRESETMANAGER_H_

#include "effectsunitspreset.h"
#include "../widget/wwidget.h"

class EffectsUnitsPresetManager {
public:
	EffectsUnitsPresetManager();
	virtual ~EffectsUnitsPresetManager();

	void loadAllPresets();
	EffectsUnitsPreset * findPresetForPlugin(QString PluginName);

private:

	QList<EffectsUnitsPreset *> m_Presets;
};

#endif /* EFFECTSUNITSPRESETMANAGER_H_ */
