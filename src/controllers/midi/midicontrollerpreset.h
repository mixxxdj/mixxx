#pragma once
/// @file midicontrollerpreset.h
/// @author Sean Pappalardo spappalardo@mixxx.org
/// @date Mon 9 Apr 2012
/// @brief MIDI Controller preset

#include <QHash>

#include "controllers/controllerpreset.h"
#include "controllers/controllerpresetvisitor.h"
#include "controllers/midi/midimessage.h"

/// This class represents a MIDI controller preset, containing the data elements
///   that make it up.
class MidiControllerPreset : public ControllerPreset {
  public:
    MidiControllerPreset(){};
    virtual ~MidiControllerPreset(){};

    bool savePreset(const QString& fileName) const override;

    virtual void accept(ControllerPresetVisitor* visitor);
    virtual void accept(ConstControllerPresetVisitor* visitor) const;
    virtual bool isMappable() const;

    // Input mappings
    void addInputMapping(uint16_t key, MidiInputMapping mapping);
    void removeInputMapping(uint16_t key);
    const QHash<uint16_t, MidiInputMapping>& getInputMappings() const;
    void setInputMappings(const QHash<uint16_t, MidiInputMapping>& mappings);

    // Output mappings
    void addOutputMapping(ConfigKey key, MidiOutputMapping mapping);
    void removeOutputMapping(ConfigKey key);
    const QHash<ConfigKey, MidiOutputMapping>& getOutputMappings() const;
    void setOutputMappings(const QHash<ConfigKey, MidiOutputMapping>& mappings);

  private:
    // MIDI input and output mappings.
    QHash<uint16_t, MidiInputMapping> m_inputMappings;
    QHash<ConfigKey, MidiOutputMapping> m_outputMappings;
};
