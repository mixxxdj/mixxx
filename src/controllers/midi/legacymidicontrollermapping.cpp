#include "controllers/midi/legacymidicontrollermapping.h"

#include "controllers/defs_controllers.h"
#include "controllers/midi/legacymidicontrollermappingfilehandler.h"

bool LegacyMidiControllerMapping::saveMapping(const QString& fileName) const {
    LegacyMidiControllerMappingFileHandler handler;
    return handler.save(*this, fileName);
}

void LegacyMidiControllerMapping::accept(LegacyControllerMappingVisitor* visitor) {
    if (visitor) {
        visitor->visit(this);
    }
}

void LegacyMidiControllerMapping::accept(ConstLegacyControllerMappingVisitor* visitor) const {
    if (visitor) {
        visitor->visit(this);
    }
}

bool LegacyMidiControllerMapping::isMappable() const {
    return true;
}

void LegacyMidiControllerMapping::addInputMapping(uint16_t key, const MidiInputMapping& mapping) {
    m_inputMappings.insert(key, mapping);
    setDirty(true);
}

void LegacyMidiControllerMapping::removeInputMapping(uint16_t key) {
    m_inputMappings.remove(key);
    setDirty(true);
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
