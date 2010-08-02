#include <QtCore>
#include <QDebug>
#include "effectsunitsplugin.h"


EffectsUnitsPlugin::EffectsUnitsPlugin(EffectsUnitsBackend * Backend, QString * Name, int PluginID) {
	m_Backend = Backend;
	m_Name = Name;
	m_PluginID = PluginID;

	//qDebug() << "FXUNITS: EffectsUnitsPlugins: new plugin: " << (*m_Name) << (int) this;
}

EffectsUnitsPlugin::~EffectsUnitsPlugin() {
	// TODO Auto-generated destructor stub
}

/* EffectsUnitsPlugin::connect
 * Assigns a value to a plugins port.
 * Redirects to the backend to preserve abstraction.
 */
void EffectsUnitsPlugin::connect(int PortID, double Value){
	m_Backend->connect(PortID, Value, m_PluginID);
}

/* EffectsUnitsPlugin::process
 * We redirect the process call to the Backend because this
 * can be backend specific. This way we only have the Backend
 * abstraction and corresponding implementations for each fx Backend.
 */
void EffectsUnitsPlugin::process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize, double WetDry){
	m_Backend->process(pIn, pOut, iBufferSize, m_PluginID, WetDry);
}

void EffectsUnitsPlugin::activate(){
	m_Backend->activatePlugin(m_PluginID);
}

QString EffectsUnitsPlugin::getName(){
	return (*m_Name);
}

void EffectsUnitsPlugin::addPort(EffectsUnitsPort * port){
	m_Ports.push_back(port);
}

QList<EffectsUnitsPort *> * EffectsUnitsPlugin::getPorts(){
	return &m_Ports;
}

QString EffectsUnitsPlugin::getPortNameByIndex(int i){
	if (i < m_Ports.size()){
		if (!m_Ports.at(i)->isAudio){
			return *(m_Ports.at(i)->Name);
		}
	}
	return "Not Used";
}


