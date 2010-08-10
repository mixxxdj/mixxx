/*
 * effectsunitspresetmanager.h
 *
 *  Created on: Aug 10, 2010
 *      Author: bruno
 */

#ifndef EFFECTSUNITSPRESETMANAGER_H_
#define EFFECTSUNITSPRESETMANAGER_H_

class EffectsUnitsPresetManager {
public:
	EffectsUnitsPresetManager();
	virtual ~EffectsUnitsPresetManager();

	void loadAllPresets();
	EffectsUnitsPreset * findPresetForPlugin(QString PluginName);
};

#endif /* EFFECTSUNITSPRESETMANAGER_H_ */
