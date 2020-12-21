#pragma once

#include <QMultiHash>

#include "controllers/controllermappingvisitor.h"
#include "controllers/legacycontrollermapping.h"
#include "controllers/midi/midimessage.h"

/// Represents a MIDI controller mapping, containing the data elements that make
/// it up.
class LegacyMidiControllerMapping : public LegacyControllerMapping {
  public:
    LegacyMidiControllerMapping(){};
    virtual ~LegacyMidiControllerMapping(){};

    bool saveMapping(const QString& fileName) const override;

    virtual void accept(LegacyControllerMappingVisitor* visitor) override;
    virtual void accept(ConstLegacyControllerMappingVisitor* visitor) const override;
    virtual bool isMappable() const override;

    // Input mappings
    void addInputMapping(uint16_t key, const MidiInputMapping& mapping);
    void removeInputMapping(uint16_t key);
    const QMultiHash<uint16_t, MidiInputMapping>& getInputMappings() const;
    void setInputMappings(const QMultiHash<uint16_t, MidiInputMapping>& mappings);

    // Output mappings
    void addOutputMapping(const ConfigKey& key, const MidiOutputMapping& mapping);
    void removeOutputMapping(const ConfigKey& key);
    const QMultiHash<ConfigKey, MidiOutputMapping>& getOutputMappings() const;
    void setOutputMappings(const QMultiHash<ConfigKey, MidiOutputMapping>& mappings);

  private:
    // MIDI input and output mappings.
    QMultiHash<uint16_t, MidiInputMapping> m_inputMappings;
    QMultiHash<ConfigKey, MidiOutputMapping> m_outputMappings;
};
