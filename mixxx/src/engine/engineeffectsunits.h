#include "engineobject.h"
#include "effectsunits/effectsunitsplugin.h"

class EngineEffectsUnits : public EngineObject {
public:
	EngineEffectsUnits();
	~EngineEffectsUnits();

	static EngineEffectsUnits * getEngine();
	void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize);
	void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize, const QString Source);

	void addPluginToSource(EffectsUnitsPlugin * Plugin, QString Source);

private:
	static EngineEffectsUnits * m_pEngine;
	QLinkedList<EffectsUnitsPlugin * > m_OnChannel1;
	QLinkedList<EffectsUnitsPlugin * > m_OnChannel2;

};

