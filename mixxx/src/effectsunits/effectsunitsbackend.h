#ifndef EFFECTSUNITSBACKEND_H
#define EFFECTSUNITSBACKEND_H

#include <QtCore>
#include <QDebug>

#include "effectsunits/effectsunitsplugin.h"
#include "engine/engineeffectsunits.h"
#include "defs.h"

class EffectsUnitsPlugin;
class EngineEffectsUnits;

/* EffectsUnitsBackend:
 * loadPlugins() -> Load all plugins into m_BackendPlugins, assign id # for each one.
 * process() -> A process call will be forwarded from plugin, along with plugin id #
 * activatePlugin() -> This will be called before process(), along with plugin id #
 */
class EffectsUnitsBackend {
  public:
    EffectsUnitsBackend();
    virtual ~EffectsUnitsBackend();

    QString getName();
    EffectsUnitsBackend* getBackend();
    QList<EffectsUnitsPlugin*>* getPlugins();

    virtual void loadPlugins() = 0;
    virtual void connect(int PortID, float Value, int PluginID) = 0;
    virtual void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize, int PluginID, double WetDry) = 0;
    virtual void activatePlugin(int PluginID) = 0;
    virtual void deactivatePlugin(int PluginID) = 0;

  protected:
    // Name of the Backend (possibly useless since every backend will have its own class)
    QString* m_Name;
    // List of available plugins on this Backend
    QList<EffectsUnitsPlugin*> m_BackendPlugins;
    // Holds the ID for the next plugin to be loaded.
    int PluginIDSequence;
};

#endif
