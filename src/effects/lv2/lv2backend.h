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
    bool canInstantiateEffect(const QString& effectId) const;
    EffectPointer instantiateEffect(EffectsManager* pEffectsManager,
                                    const QString& effectId);
    QList<QPair<QString, bool> > getAllDiscoveredPlugins();

  private:
    void initializeProperties();
    LilvWorld* m_pWorld;
    QHash<QString, LilvNode*> m_properties;
    QHash<QString, LV2Manifest*> m_registeredEffects;
    // List used for displaying the available LV2 plugins
    QList<QPair<QString, bool> > m_allLV2Plugins;

    QString debugString() const {
        return "LV2Backend";
    }
};

#endif // LV2BACKEND_H
