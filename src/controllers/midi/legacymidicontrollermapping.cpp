#include "controllers/midi/legacymidicontrollermapping.h"

#include "controllers/midi/legacymidicontrollermappingfilehandler.h"

bool LegacyMidiControllerMapping::saveMapping(const QString& fileName) const {
    LegacyMidiControllerMappingFileHandler handler;
    return handler.save(*this, fileName);
}

bool LegacyMidiControllerMapping::isMappable() const {
    return true;
}

void LegacyMidiControllerMapping::addInputMapping(uint16_t key, const MidiInputMapping& mapping) {
    m_inputMappings.insert(key, mapping);
    if (!std::holds_alternative<std::shared_ptr<QJSValue>>(mapping.control)) {
        // Note: JS handler are not saved to the XML file
        setDirty(true);
    }
}

void LegacyMidiControllerMapping::removeInputMapping(uint16_t key) {
    for (auto [it, end] = m_inputMappings.equal_range(key); it != end; ++it) {
        const MidiInputMapping& mapping = it.value();
        if (!std::holds_alternative<std::shared_ptr<QJSValue>>(mapping.control)) {
            setDirty(true);
        }
    }
    m_inputMappings.remove(key);
}

bool LegacyMidiControllerMapping::removeInputMapping(
        uint16_t key, const MidiInputMapping& mapping) {
    auto result = m_inputMappings.remove(key, mapping);
    setDirty(true);
    return result > 0;
}

const QMultiHash<uint16_t, MidiInputMapping>&
LegacyMidiControllerMapping::getInputMappings() const {
    return m_inputMappings;
}

void LegacyMidiControllerMapping::setInputMappings(
        const QMultiHash<uint16_t, MidiInputMapping>& mappings) {
    if (m_inputMappings != mappings) {
        m_inputMappings.clear();
        m_inputMappings.unite(mappings);
        setDirty(true);
    }
}

void LegacyMidiControllerMapping::addOutputMapping(
        const ConfigKey& key, const MidiOutputMapping& mapping) {
    m_outputMappings.insert(key, mapping);
    setDirty(true);
}

void LegacyMidiControllerMapping::removeOutputMapping(const ConfigKey& key) {
    m_outputMappings.remove(key);
    setDirty(true);
}

const QMultiHash<ConfigKey, MidiOutputMapping>&
LegacyMidiControllerMapping::getOutputMappings() const {
    return m_outputMappings;
}

void LegacyMidiControllerMapping::setOutputMappings(
        const QMultiHash<ConfigKey, MidiOutputMapping>& mappings) {
    if (m_outputMappings != mappings) {
        m_outputMappings.clear();
        m_outputMappings.unite(mappings);
        setDirty(true);
    }
}
void LegacyMidiControllerMapping::removeInputHandlerMappings() {
#if QT_VERSION >= QT_VERSION_CHECK(6, 1, 0)
    m_inputMappings.removeIf(
            [](std::pair<const uint16_t&, MidiInputMapping&> it) {
                return !std::holds_alternative<ConfigKey>(it.second.control);
            });
#else
    for (auto it = m_inputMappings.begin(); it != m_inputMappings.end();) {
        if (!std::holds_alternative<ConfigKey>(it.value().control)) {
            it = m_inputMappings.erase(it);
        } else {
            ++it;
        }
    }
#endif
}
