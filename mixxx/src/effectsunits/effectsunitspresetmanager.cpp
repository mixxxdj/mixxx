/*
 * effectsunitspresetmanager.cpp
 *
 *  Created on: Aug 10, 2010
 *      Author: bruno
 */

#include "effectsunitspresetmanager.h"

EffectsUnitsPresetManager::EffectsUnitsPresetManager() {
	loadAllPresets();
}

EffectsUnitsPresetManager::~EffectsUnitsPresetManager() {
	// TODO Auto-generated destructor stub
}

EffectsUnitsPreset * EffectsUnitsPresetManager::findPresetForPlugin(QString PluginName){
	int size = m_Presets.size();
	for (int i = 0; i < size; i++){
		if (m_Presets.at(i)->presetFor() == PluginName){
			return m_Presets.at(i);
		}
	}
	return NULL;
}

void EffectsUnitsPresetManager::loadAllPresets(){
	QDir dir(WWidget::getPath(QString("../../effectsunits")));
	QFileInfoList files = dir.entryInfoList();
	for (QFileInfoList::iterator fileIter = files.begin(); fileIter != files.end(); fileIter++)
	{
		if ((*fileIter).isDir()) continue; // Folders can't be presets

		qDebug() << "FXUNITS: PRESET: " << (*fileIter).filePath();

		QFile file((* fileIter).filePath());
		QDomDocument document("EffectsUnitsPreset");
		file.open(QIODevice::ReadOnly);
		document.setContent(&file);
		file.close();

		m_Presets.append(new EffectsUnitsPreset(document.documentElement()));
	}
}
