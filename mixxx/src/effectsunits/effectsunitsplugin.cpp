/*
 * effectsunitsplugin.cpp
 *
 *  Created on: Jun 16, 2010
 *      Author: bruno
 */
#include <QtCore>
#include <QDebug>
#include "effectsunitsplugin.h"


EffectsUnitsPlugin::EffectsUnitsPlugin(EffectsUnitsBackend * Backend, QString * Name, int PluginID) {
	m_Backend = Backend;
	m_Name = Name;
	m_PluginID = PluginID;

	qDebug() << "FXUNITS: EffectsUnitsPlugins: new plugin: " << (*m_Name) << (int) this;
}

EffectsUnitsPlugin::~EffectsUnitsPlugin() {
	// TODO Auto-generated destructor stub
}

void EffectsUnitsPlugin::process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize){
	qDebug() << "FXUNITS: EffectsUnitsPlugins: processing: " << (*m_Name);
}

QString EffectsUnitsPlugin::getName(){
	return (*m_Name);
}
