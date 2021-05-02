#pragma once

#include <lilv/lilv.h>

#include "effects/defs.h"
#include "effects/effectsbackend.h"
#include "effects/lv2/lv2manifest.h"
#include "preferences/usersettings.h"

class LV2Backend : public EffectsBackend {
    Q_OBJECT
  public:
    LV2Backend(QObject* pParent);
    virtual ~LV2Backend();

    void enumeratePlugins();
    const QList<QString> getEffectIds() const;
    const QSet<QString> getDiscoveredPluginIds() const;
    EffectManifestPointer getManifest(const QString& effectId) const;
    LV2Manifest* getLV2Manifest(const QString& effectId) const;
    bool canInstantiateEffect(const QString& effectId) const;
    EffectPointer instantiateEffect(EffectsManager* pEffectsManager,
                                    const QString& effectId);

  private:
    void initializeProperties();
    LilvWorld* m_pWorld;
    QHash<QString, LilvNode*> m_properties;
    QHash<QString, LV2Manifest*> m_registeredEffects;

    QString debugString() const {
        return "LV2Backend";
    }
};
