#include <QtCore>
#include <QDebug>

#include "effectsunitsplugin.h"
#include "../defs.h"

class EffectsUnitsBackend {
public:
	EffectsUnitsBackend();
	~EffectsUnitsBackend();

	QString getName();
	EffectsUnitsBackend * getBackend();
	QList<EffectsUnitsPlugin *> * getPlugins();

	virtual void loadPlugins() const = 0;
	virtual void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize, int PluginID) const = 0;
	virtual void activatePlugin(int PluginID) const = 0;

protected:
	QString * m_Name;
	QList<EffectsUnitsPlugin *> m_BackendPlugins;
};


