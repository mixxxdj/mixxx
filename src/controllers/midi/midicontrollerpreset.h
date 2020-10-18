#pragma once
/// @file midicontrollerpreset.h
/// @author Sean Pappalardo spappalardo@mixxx.org
/// @date Mon 9 Apr 2012
/// @brief MIDI Controller preset

#include <QMultiHash>

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

    virtual void accept(ControllerPresetVisitor* visitor) override;
    virtual void accept(ConstControllerPresetVisitor* visitor) const override;
    virtual bool isMappable() const override;

    // Input mappings
    void addInputMapping(uint16_t key, MidiInputMapping mapping);
    void removeInputMapping(uint16_t key);
    const QMultiHash<uint16_t, MidiInputMapping>& getInputMappings() const;
    void setInputMappings(const QMultiHash<uint16_t, MidiInputMapping>& mappings);

    // Output mappings
    void addOutputMapping(ConfigKey key, MidiOutputMapping mapping);
    void removeOutputMapping(ConfigKey key);
    const QMultiHash<ConfigKey, MidiOutputMapping>& getOutputMappings() const;
    void setOutputMappings(const QMultiHash<ConfigKey, MidiOutputMapping>& mappings);

  private:
    // MIDI input and output mappings.
    QMultiHash<uint16_t, MidiInputMapping> m_inputMappings;
    QMultiHash<ConfigKey, MidiOutputMapping> m_outputMappings;
};
