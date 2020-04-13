/// @file midicontrollerpreset.cpp
/// @author Jan Holthuis holzhaus@mixxx.org
/// @date Wed 8 Apr 2020
/// @brief MIDI Controller Preset
///
/// This class represents a MIDI controller preset, containing the data elements
///   that make it up.

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

void MidiControllerPreset::addInputMapping(uint16_t key, MidiInputMapping mapping) {
    m_inputMappings.insertMulti(key, mapping);
    setDirty(true);
}

void MidiControllerPreset::removeInputMapping(uint16_t key) {
    m_inputMappings.remove(key);
    setDirty(true);
}

const QHash<uint16_t, MidiInputMapping>& MidiControllerPreset::getInputMappings() const {
    return m_inputMappings;
}

void MidiControllerPreset::setInputMappings(const QHash<uint16_t, MidiInputMapping>& mappings) {
    if (m_inputMappings != mappings) {
        m_inputMappings.clear();
        m_inputMappings.unite(mappings);
        setDirty(true);
    }
}

void MidiControllerPreset::addOutputMapping(ConfigKey key, MidiOutputMapping mapping) {
    m_outputMappings.insertMulti(key, mapping);
    setDirty(true);
}

void MidiControllerPreset::removeOutputMapping(ConfigKey key) {
    m_outputMappings.remove(key);
    setDirty(true);
}

const QHash<ConfigKey, MidiOutputMapping>& MidiControllerPreset::getOutputMappings() const {
    return m_outputMappings;
}

void MidiControllerPreset::setOutputMappings(const QHash<ConfigKey, MidiOutputMapping>& mappings) {
    if (m_outputMappings != mappings) {
        m_outputMappings.clear();
        m_outputMappings.unite(mappings);
        setDirty(true);
    }
}
