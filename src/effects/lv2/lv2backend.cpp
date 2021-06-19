#include "effects/lv2/lv2backend.h"

#include "effects/lv2/lv2manifest.h"
#include "moc_lv2backend.cpp"

LV2Backend::LV2Backend(QObject* pParent)
        : EffectsBackend(pParent, EffectBackendType::LV2) {
    m_pWorld = lilv_world_new();
    initializeProperties();
    lilv_world_load_all(m_pWorld);
    enumeratePlugins();
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
        if (lilv_plugin_is_replaced(plug)) {
            continue;
        }
        LV2Manifest* lv2Manifest = new LV2Manifest(plug, m_properties);
        lv2Manifest->getEffectManifest()->setBackendType(m_type);
        m_registeredEffects.insert(lv2Manifest->getEffectManifest()->id(),
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

const QList<QString> LV2Backend::getEffectIds() const {
    QList<QString> availableEffects;
    foreach (LV2Manifest* lv2Manifest, m_registeredEffects) {
        if (lv2Manifest->isValid()) {
            availableEffects.append(lv2Manifest->getEffectManifest()->id());
        }
    }
    return availableEffects;
}

const QSet<QString> LV2Backend::getDiscoveredPluginIds() const {
    QSet<QString> pluginIds;
    for (auto it = m_registeredEffects.constBegin();
         it != m_registeredEffects.constEnd(); ++it) {
        pluginIds.insert(it.key());
    }
    return pluginIds;
}

bool LV2Backend::canInstantiateEffect(const QString& effectId) const {
    if (m_registeredEffects.contains(effectId) &&
        m_registeredEffects[effectId]->isValid()) {
        return true;
    }
    return false;
}

EffectManifestPointer LV2Backend::getManifest(const QString& effectId) const {
    LV2Manifest* pLV2Mainfest = getLV2Manifest(effectId);
    if (pLV2Mainfest != nullptr) {
        return pLV2Mainfest->getEffectManifest();
    }
    return EffectManifestPointer();
}

LV2Manifest* LV2Backend::getLV2Manifest(const QString& effectId) const {
    return m_registeredEffects[effectId];
}

EffectPointer LV2Backend::instantiateEffect(EffectsManager* pEffectsManager,
                                            const QString& effectId) {
    if (!canInstantiateEffect(effectId)) {
        qWarning() << "WARNING: Effect" << effectId << "is not registered.";
        return EffectPointer();
    }
    LV2Manifest* lv2manifest = m_registeredEffects[effectId];

    return EffectPointer(
        new Effect(
                pEffectsManager,
                lv2manifest->getEffectManifest(),
                EffectInstantiatorPointer(
                        new LV2EffectProcessorInstantiator(
                                lv2manifest->getPlugin(),
                                lv2manifest->getAudioPortIndices(),
                                lv2manifest->getControlPortIndices()))));
}
