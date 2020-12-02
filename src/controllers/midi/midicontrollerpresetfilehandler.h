#pragma once

#include "controllers/controllerpresetfilehandler.h"
#include "controllers/midi/midicontrollerpreset.h"

/// Handles loading and saving of MIDI controller presets.
class MidiControllerPresetFileHandler : public ControllerPresetFileHandler {
  public:
    MidiControllerPresetFileHandler() {};
    virtual ~MidiControllerPresetFileHandler() {};

    bool save(const MidiControllerPreset& preset, const QString& fileName) const;

  private:
    virtual ControllerPresetPointer load(const QDomElement& root,
            const QString& filePath,
            const QDir& systemPresetPath);

    void addControlsToDocument(const MidiControllerPreset& preset,
                               QDomDocument* doc) const;

    QDomElement makeTextElement(QDomDocument* doc,
                                const QString& elementName,
                                const QString& text) const;

    QDomElement inputMappingToXML(QDomDocument* doc,
                                  const MidiInputMapping& mapping) const;

    QDomElement outputMappingToXML(QDomDocument* doc,
                                   const MidiOutputMapping& mapping) const;
};
