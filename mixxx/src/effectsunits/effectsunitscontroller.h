#ifndef EFFECTSUNITSCONTROLLER_H
#define EFFECTSUNITSCONTROLLER_H

#include "../engine/engineeffectsunits.h"
#include "debugbackend.h"
#include "ladspabackend.h"

class EffectsUnitsController {

public:
	EffectsUnitsController();
	~EffectsUnitsController();

private:
	EngineEffectsUnits * m_pEngine;
	QList<EffectsUnitsBackend *> m_pBackendsList;
};

#endif
