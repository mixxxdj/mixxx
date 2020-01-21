#include "controllers/midi/midicontrollerpreset.h"

#include "controllers/defs_controllers.h"
#include "controllers/midi/midicontrollerpresetfilehandler.h"

bool MidiControllerPreset::savePreset(const QString& fileName) const {
    MidiControllerPresetFileHandler handler;
    return handler.save(*this, fileName);
}

void MidiControllerPreset::accept(ControllerPresetVisitor* visitor) {
    if (visitor) {
        visitor->visit(this);
    }
}

void MidiControllerPreset::accept(ConstControllerPresetVisitor* visitor) const {
    if (visitor) {
        visitor->visit(this);
    }
}

bool MidiControllerPreset::isMappable() const {
    return true;
}

void MidiControllerPreset::addInputMapping(uint16_t key, const MidiInputMapping& mapping) {
    m_inputMappings.insert(key, mapping);
    setDirty(true);
}

void MidiControllerPreset::removeInputMapping(uint16_t key) {
    m_inputMappings.remove(key);
    setDirty(true);
}

const QMultiHash<uint16_t, MidiInputMapping>& MidiControllerPreset::getInputMappings() const {
    return m_inputMappings;
}

void MidiControllerPreset::setInputMappings(
        const QMultiHash<uint16_t, MidiInputMapping>& mappings) {
    if (m_inputMappings != mappings) {
        m_inputMappings.clear();
        m_inputMappings.unite(mappings);
        setDirty(true);
    }
}

void MidiControllerPreset::addOutputMapping(
        const ConfigKey& key, const MidiOutputMapping& mapping) {
    m_outputMappings.insert(key, mapping);
    setDirty(true);
}

void MidiControllerPreset::removeOutputMapping(const ConfigKey& key) {
    m_outputMappings.remove(key);
    setDirty(true);
}

const QMultiHash<ConfigKey, MidiOutputMapping>&
MidiControllerPreset::getOutputMappings() const {
    return m_outputMappings;
}

void MidiControllerPreset::setOutputMappings(
        const QMultiHash<ConfigKey, MidiOutputMapping>& mappings) {
    if (m_outputMappings != mappings) {
        m_outputMappings.clear();
        m_outputMappings.unite(mappings);
        setDirty(true);
    }
}
