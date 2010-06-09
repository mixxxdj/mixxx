#include "engineobject.h"

class EngineEffectsUnits : public EngineObject {

public:
	EngineEffectsUnits();
	~EngineEffectsUnits();

	static EngineEffectsUnits * getEngine();
	void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize);
	void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize, const int iChannel);

private:
	static EngineEffectsUnits * m_pEngine;
};

