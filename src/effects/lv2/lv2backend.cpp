#include "effects/lv2/lv2backend.h"
#include "effects/lv2/lv2manifest.h"

LV2Backend::LV2Backend(QObject* pParent)
        : EffectsBackend(pParent, tr("LV2")) {
    m_pWorld = lilv_world_new();
    initializeProperties();
    lilv_world_load_all(m_pWorld);
}

LV2Backend::~LV2Backend() {
    foreach(LilvNode* node, m_properties) {
        lilv_node_free(node);
    }
    lilv_world_free(m_pWorld);
    foreach(LV2Manifest* lv2Manifest, m_registeredEffects) {
        delete lv2Manifest;
    }
}

void LV2Backend::enumeratePlugins() {
    const LilvPlugins *plugs = lilv_world_get_all_plugins(m_pWorld);
    LILV_FOREACH(plugins, i, plugs) {
        const LilvPlugin *plug = lilv_plugins_get(plugs, i);
        LV2Manifest* lv2Manifest = new LV2Manifest(plug, m_properties);
        m_registeredEffects.insert(lv2Manifest->getEffectManifest().id(),
                                   lv2Manifest);
    }
}

void LV2Backend::initializeProperties() {
    m_properties["audio_port"] = lilv_new_uri(m_pWorld, LV2_CORE__AudioPort);
    m_properties["input_port"] = lilv_new_uri(m_pWorld, LV2_CORE__InputPort);
    m_properties["output_port"] = lilv_new_uri(m_pWorld, LV2_CORE__OutputPort);
    m_properties["control_port"] = lilv_new_uri(m_pWorld, LV2_CORE__ControlPort);
    m_properties["button_port"] = lilv_new_uri(m_pWorld, LV2_CORE__toggled);
    m_properties["integer_port"] = lilv_new_uri(m_pWorld, LV2_CORE__integer);
    m_properties["enumeration_port"] = lilv_new_uri(m_pWorld, LV2_CORE__enumeration);
}

const QSet<QString> LV2Backend::getEffectIds() const {
    QSet<QString> availableEffects;
    foreach (LV2Manifest* lv2Manifest, m_registeredEffects) {
        if (lv2Manifest->isValid()) {
            availableEffects.insert(lv2Manifest->getEffectManifest().id());
        }
    }
    return availableEffects;
}

const QSet<QString> LV2Backend::getDiscoveredPluginIds() const {
    return m_registeredEffects.keys().toSet();
}

bool LV2Backend::canInstantiateEffect(const QString& effectId) const {
    if (m_registeredEffects.contains(effectId) &&
        m_registeredEffects[effectId]->isValid()) {
        return true;
    }
    return false;
}

EffectManifest LV2Backend::getManifest(const QString& effectId) const {
    if (m_registeredEffects.contains(effectId)) {
        return m_registeredEffects[effectId]->getEffectManifest();
    }
    return EffectManifest();
}

LV2Manifest* LV2Backend::getLV2Manifest(const QString& effectId) const {
    return m_registeredEffects[effectId];
}

EffectManifest& LV2Backend::getManifestReference(const QString& effectId) {
    return m_registeredEffects[effectId]->getEffectManifestReference();
}

EffectPointer LV2Backend::instantiateEffect(EffectsManager* pEffectsManager,
                                                const QString& effectId) {
    if (!canInstantiateEffect(effectId)) {
        qWarning() << "WARNING: Effect" << effectId << "is not registered.";
        return EffectPointer();
    }
    LV2Manifest* lv2manifest = m_registeredEffects[effectId];

    return EffectPointer(new Effect(this, pEffectsManager,
                         lv2manifest->getEffectManifest(),
                         EffectInstantiatorPointer(new
                         LV2EffectProcessorInstantiator(lv2manifest->getPlugin(),
                                                        lv2manifest->getAudioPortIndices(),
                                                        lv2manifest->getControlPortIndices()))));
}
