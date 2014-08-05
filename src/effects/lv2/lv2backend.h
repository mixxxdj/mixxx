#ifndef LV2BACKEND_H
#define LV2BACKEND_H

#include "effects/effectsbackend.h"
#include "effects/lv2/lv2manifest.h"
#include <lilv-0/lilv/lilv.h>

class LV2Backend : public EffectsBackend {
    Q_OBJECT
  public:
    LV2Backend(QObject* pParent=NULL);
    virtual ~LV2Backend();

    void enumeratePlugins();
    const QSet<QString> getEffectIds() const;
    EffectManifest getManifest(const QString& effectId) const;
    EffectManifest& getManifestReference(const QString& effectId);
    bool canInstantiateEffect(const QString& effectId) const;
    EffectPointer instantiateEffect(EffectsManager* pEffectsManager,
                                    const QString& effectId);
    QList<QPair<QPair<QString, bool>, QString> > getAllDiscoveredPlugins();

  private:
    void initializeProperties();
    LilvWorld* m_pWorld;
    QHash<QString, LilvNode*> m_properties;
    QHash<QString, LV2Manifest*> m_registeredEffects;
    // List used for displaying the available LV2 plugins
    // It stores the name of the plugin, its availability and its id
    // entry.first.first -> name
    // entry.first.second -> availability
    // entry.second -> id
    QList<QPair<QPair<QString, bool>, QString> > m_allLV2Plugins;

    QString debugString() const {
        return "LV2Backend";
    }
};

#endif // LV2BACKEND_H
