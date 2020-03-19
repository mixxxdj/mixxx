/**
 * @file midicontrollerpreset.h
 * @author Sean Pappalardo spappalardo@mixxx.org
 * @date Mon 9 Apr 2012
 * @brief MIDI Controller preset
 *
 * This class represents a MIDI controller preset, containing the data elements
 *   that make it up.
 *
 */

#ifndef MIDICONTROLLERPRESET_H
#define MIDICONTROLLERPRESET_H

#include <QHash>

#include "controllers/controllerpreset.h"
#include "controllers/controllerpresetvisitor.h"
#include "controllers/midi/midimessage.h"

class MidiControllerPreset : public ControllerPreset {
  public:
    MidiControllerPreset() {}
    virtual ~MidiControllerPreset() {}

    virtual void accept(ControllerPresetVisitor* visitor) {
        if (visitor) {
            visitor->visit(this);
        }
    }

    virtual void accept(ConstControllerPresetVisitor* visitor) const {
        if (visitor) {
            visitor->visit(this);
        }
    }

    virtual bool isMappable() const {
        return true;
    }

    void addInputMapping(uint16_t key, MidiInputMapping mapping) {
        m_inputMappings.insertMulti(key, mapping);
        setDirty(true);
    }

    void removeInputMapping(uint16_t key) {
        m_inputMappings.remove(key);
        setDirty(true);
    }

    const QHash<uint16_t, MidiInputMapping>& getInputMappings() const {
        return m_inputMappings;
    }

    void setInputMappings(const QHash<uint16_t, MidiInputMapping>& mappings) {
        if (m_inputMappings != mappings) {
            m_inputMappings.clear();
            m_inputMappings.unite(mappings);
            setDirty(true);
        }
    }

    void addOutputMapping(ConfigKey key, MidiOutputMapping mapping) {
        m_outputMappings.insertMulti(key, mapping);
        setDirty(true);
    }

    void removeOutputMapping(ConfigKey key) {
        m_outputMappings.remove(key);
        setDirty(true);
    }

    const QHash<ConfigKey, MidiOutputMapping>& getOutputMappings() const {
        return m_outputMappings;
    }

    void setOutputMappings(const QHash<ConfigKey, MidiOutputMapping>& mappings) {
        if (m_outputMappings != mappings) {
            m_outputMappings.clear();
            m_outputMappings.unite(mappings);
            setDirty(true);
        }
    }

  private:
    // MIDI input and output mappings.
    QHash<uint16_t, MidiInputMapping> m_inputMappings;
    QHash<ConfigKey, MidiOutputMapping> m_outputMappings;
};

#endif
